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
"""Data Preprocessing Script

This script downloads the MNIST dataset and processes it into the data format required by the HiSpark.AI Studio
"""
import argparse
import csv
import os
import logging

import numpy as np
from torchvision.datasets import MNIST


SUPPORT_DATATYPE = {
    "float16": np.float16,
    "float32": np.float32
}

SUPPORT_FILE_FORMAT = ['bin', 'npy', 'all']


# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
    handlers=[logging.StreamHandler()]
)
logger = logging.getLogger(__name__)


def make_dir_if_not_exists(path: str) -> None:
    """Create the directory if it does not exist.

    Args:
        path: Name of the directory to create
    """
    if not os.path.exists(path):
        os.makedirs(path)
        logger.info("INFO: make dir {}".format(path))


def download_dataset(
    orig_path: str, save_path: str, train: bool, download_bin: bool, download_npy: bool, bin_datatype: str):
    """Download and preprocess MNIST dataset. Data will be normalized to 0~1

    Args:
        orig_path: Path to save the original data
        save_path: Path to save the preprocessed data
        download_bin: Whether to save MNIST dataset as bin format
        download_npy: Whether to save MNIST dataset as npy format
        bin_datatype: Indicates data type of the data stored in the bin file
    """
    dataset = MNIST(root=orig_path, train=train, download=True)

    npy_path = os.path.join(save_path, "npy")
    bin_path = os.path.join(save_path, "bin")
    if download_bin:
        make_dir_if_not_exists(bin_path)
    if download_npy:
        make_dir_if_not_exists(npy_path)


    labels = [["sample", "label"]]
    for idx, (image, label) in enumerate(dataset):
        tensor = np.array(image).astype(np.float32) / 255

        sample_name = "sample_{:0>5d}_{}".format(idx, label)
        if download_bin:
            sample_path = "{}.bin".format(sample_name)
            tensor.astype(SUPPORT_DATATYPE[bin_datatype]).tofile(os.path.join(bin_path, sample_path))
        if download_npy:
            sample_path = "{}.npy".format(sample_name)
            np.save(os.path.join(npy_path, sample_path), tensor)

        labels.append([sample_name, label])

    with open(os.path.join(save_path, "label.csv"), 'w') as f:
        writer = csv.writer(f)
        writer.writerows(labels)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='download and preprocess MNIST dataset')
    parser.add_argument('--orig_path', help='Path to save the original data', required=True)
    parser.add_argument('--train_path', help='Path to save the training set', required=True)
    parser.add_argument('--test_path', help='Path to save the test set', required=True)
    parser.add_argument('--train_file_format',
        help='The storage format of the training set shall be one of {}.'.format(SUPPORT_FILE_FORMAT),
        default="bin",
        required=False,
        choices=SUPPORT_FILE_FORMAT
    )
    parser.add_argument('--test_file_format',
        help='The storage format of the test set shall be one of {}.'.format(SUPPORT_FILE_FORMAT),
        default="npy",
        required=False,
        choices=SUPPORT_FILE_FORMAT
    )
    parser.add_argument('--test_data_type',
        help='The data type of the test set shall be one of {}.'.format(SUPPORT_DATATYPE.keys()),
        default="float16",
        required=False,
        choices=SUPPORT_DATATYPE.keys()
    )
    args = parser.parse_args()

    download_dataset(
        orig_path=args.orig_path,
        save_path=args.train_path,
        train=True,
        download_bin=args.train_file_format == "bin" or args.train_file_format == "all",
        download_npy=args.train_file_format == "npy" or args.train_file_format == "all",
        bin_datatype="float32"
    )
    logger.info("INFO: Training set downloaded.")

    download_dataset(
        orig_path=args.orig_path,
        save_path=args.test_path,
        train=False,
        download_bin=args.test_file_format == "bin" or args.test_file_format == "all",
        download_npy=args.test_file_format == "npy" or args.test_file_format == "all",
        bin_datatype=args.test_data_type
    )
    logger.info("INFO: Test set downloaded.")