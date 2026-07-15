# Introduction of Google Open-speech

## 数据介绍
此数据集来源于Google Open-speech数据集，包含各类场景语音以及口语发音。

## 数据来源 && License
原始数据下载链接：[speech_commands_v0.02.tar.gz](http://download.tensorflow.org/data/speech_commands_v0.02.tar.gz)。Label主要涉及口语唤醒词的语音。
- **_Silence_**
- **_unkown_**
- **Yes**
- **No**
- **Up**
- **Down**
- **Left**
- **Right**
- **On**
- **Off**
- **Stop**
- **Go**


## 数据划分方法
数据划分按照 10%数据作为校准数据集，10%的数据作为测试数据集。

## 预处理流程
预处理包含DecodeWav，AudioSpectrogram 以及 MFCC三个步骤。