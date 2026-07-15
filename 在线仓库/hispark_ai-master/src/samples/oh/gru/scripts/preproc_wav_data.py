#!/usr/bin/python3.11
# -*- coding: utf-8 -*-
# Copyright (c) 2025-2025 HiSilicon (Shanghai) Technologies Co., Ltd. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License
import os
import struct
import sys
import urllib
import tarfile
import argparse
import random
import logging

import onnx
import onnxruntime as ort
import tensorflow as tf
import numpy as np
if tf.__version__.split(".")[0] == '1':
    from tensorflow.contrib.framework.python.ops import audio_ops


LABELS = ["_silence_", "_unknown_", "yes", "no", "up", "down", "left", "right", "on", "off", "stop", "go"]


def write_txt(data, txt_file, sep_len=12):
    idx = 1
    with open(txt_file, "w") as fw:
        for f in np.nditer(data):
            f = float(f)
            s = "{:.06f}".format(f)
            if idx % sep_len == 0:
                fw.write("  ")
            fw.write(s + ", ")
            if idx % sep_len == (sep_len - 1):
                fw.write("\n")
            idx += 1


def onnx_infer(data, feature_name, onnx_model_path, output_root_dir):
    if not os.path.exists(os.path.join(output_root_dir, "quant_mfcc_input")):
        os.makedirs(os.path.join(output_root_dir, "quant_mfcc_input"))
    if not os.path.exists(os.path.join(output_root_dir, "quant_hidden_states")):
        os.makedirs(os.path.join(output_root_dir, "quant_hidden_states"))
    init_hidden_states = np.zeros((1, 154), dtype=np.float32)
    for time in range(25):
        session = ort.InferenceSession(onnx_model_path, providers=['CPUExecutionProvider'])
        data[:, time, :].tofile(os.path.join(output_root_dir, "quant_mfcc_input", "timestamp" + str(time) + "_" +
            feature_name.split(".")[0] + ".bin"))
        init_hidden_states.tofile(os.path.join(output_root_dir, "quant_hidden_states", "timestamp" + str(time) + "_"
            + feature_name.split(".")[0] + ".bin"))
        outputs = session.run(["output_hidden_states", "label_softmax"], {"mfcc_input": data[:, time, :],
            "hidden_states": init_hidden_states})
        init_hidden_states = outputs[0]
    logits = outputs[1][0]
    return logits


def preprocess(path):
    tf_version = tf.__version__
    with open(path, 'rb') as wav_file:
        wav_data = wav_file.read()
    waveform, sample_rate = tf.audio.decode_wav(
        contents=wav_data,
        desired_channels=1,
        desired_samples=16000
    )
    if tf_version.split(".")[0] == '2':
        spectrogram = tf.raw_ops.AudioSpectrogram(
                    input=waveform,
                    window_size=640,
                    stride=640,
                    magnitude_squared=True
        )
    else:
        spectrogram = audio_ops.audio_spectrogram(
                    waveform,
                    window_size=640,
                    stride=640,
                    magnitude_squared=True
        )
    if tf_version.split(".")[0] == '2':
        mfcc_input = tf.raw_ops.Mfcc(
                    spectrogram=spectrogram,
                    sample_rate=sample_rate,
                    dct_coefficient_count=10,
                    filterbank_channel_count=40,
                    lower_frequency_limit=20,
                    upper_frequency_limit=4000
        )
    else:
        mfcc_input = audio_ops.mfcc(
                    spectrogram=spectrogram,
                    sample_rate=sample_rate,
                    dct_coefficient_count=10,
                    filterbank_channel_count=40,
                    lower_frequency_limit=20,
                    upper_frequency_limit=4000
        )
    if tf_version.split(".")[0] == '1':
        with tf.Session() as sess:
            mfcc_input_npy = mfcc_input.eval(session=sess)
    else:
        mfcc_input_npy = mfcc_input.numpy()
    return mfcc_input_npy


def maybe_download_and_extract_dataset(dest_directory,
    data_url="http://download.tensorflow.org/data/speech_commands_v0.02.tar.gz"):
    """Download and extract data set tar file.

    If the data set we're using doesn't already exist, this function
    downloads it from the TensorFlow.org website and unpacks it into a
    directory.
    If the data_url is none, don't download anything and expect the data
    directory to contain the correct files already.

    Args:
      data_url: Web location of the tar file containing the data set.
      dest_directory: File path to extract data to.
    """
    if not data_url:
        return
    if not os.path.exists(dest_directory):
        os.makedirs(dest_directory)
    filename = data_url.split('/')[-1]
    filepath = os.path.join(dest_directory, filename)
    if not os.path.exists(filepath):

        def _progress(count, block_size, total_size):
            logging.info(
                '\r>> Downloading %s %.1f%%' %
                (filename, float(count * block_size) / float(total_size) * 100.0))

        try:
            filepath, _ = urllib.request.urlretrieve(data_url, filepath, _progress)
        except:
            logging.error('Failed to download URL: %s to folder: %s', data_url,
                            filepath)
            logging.error('Please make sure you have enough free space and'
                            ' an internet connection')
            raise

        statinfo = os.stat(filepath)
        logging.info('Successfully downloaded %s (%d bytes)', filename,
                        statinfo.st_size)
    tarfile.open(filepath, 'r:gz').extractall(dest_directory)


def preprocess_calibrate_data(data_root_dir, output_root_dir, onnx_model, sample_num):
    calibrate_list = []
    with open(os.path.join(data_root_dir, "validation_list.txt"), "r") as fr:
        lines = fr.readlines()
        for line in lines:
            calibrate_list.append(line[:-1])
    random.shuffle(calibrate_list)
    if sample_num != -1 or sample_num < len(calibrate_list):
        calibrate_list = calibrate_list[0:sample_num]
    label_dirs = os.listdir(data_root_dir)
    for label in label_dirs:
        if os.path.isfile(os.path.join(data_root_dir, label)):
            continue
        if label not in LABELS:
            continue
        waves = os.listdir(os.path.join(data_root_dir, label))
        for wav in waves:
            if label + "/" + wav not in calibrate_list:
                continue
            data = preprocess(os.path.join(data_root_dir, label, wav))
            _ = onnx_infer(data, label + "_" + wav, onnx_model, output_root_dir)


def preprocess_validation_data(data_root_dir, output_root_dir, is_fp16=False):
    calibrate_list = []
    with open(os.path.join(data_root_dir, "testing_list.txt"), "r") as fr:
        lines = fr.readlines()
        for line in lines:
            calibrate_list.append(line[:-1])
    label_dirs = os.listdir(data_root_dir)
    for label in label_dirs:
        if os.path.isfile(os.path.join(data_root_dir, label)):
            continue
        if label not in LABELS:
            continue
        waves = os.listdir(os.path.join(data_root_dir, label))
        for wav in waves:
            if label + "/" + wav not in calibrate_list:
                continue
            p = os.path.join(output_root_dir, label, wav.split(".")[0])
            if not os.path.exists(os.path.dirname(p)):
                os.makedirs(os.path.dirname(p))
            data = preprocess(os.path.join(data_root_dir, label, wav))
            if is_fp16:
                data_fp16 = data.astype(np.float16)
                data_fp16.tofile(p + ".bin")
            else:
                data.tofile(p + ".bin")
            write_txt(data, p + ".txt")
            np.save(p + ".npy", data)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='preproc wav data')
    parser.add_argument('--data_root_dir', help='data file download', default="./data", required=False)
    parser.add_argument('--quant_data_dir', help='quant data', default="./calib_data", required=False)
    parser.add_argument('--validation_data_dir', help='validation data', default="./valid_data", required=False)
    parser.add_argument('--onnx_model_path', help='onnx model path', default="../model/GRU_S_STREAM.onnx",
        required=False)
    parser.add_argument('--sample_num', help='sample number', default=-1, required=False)
    parser.add_argument('--fp16', help='fp16 validation data', default=True, required=False)
    args = parser.parse_args()
    maybe_download_and_extract_dataset(dest_directory=args.data_root_dir)
    preprocess_calibrate_data(data_root_dir=args.data_root_dir, output_root_dir=args.quant_data_dir,
        onnx_model=args.onnx_model_path, sample_num=int(args.sample_num))
    preprocess_validation_data(data_root_dir=args.data_root_dir, output_root_dir=args.validation_data_dir,
        is_fp16=args.fp16)
