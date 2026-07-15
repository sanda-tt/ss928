# HiSpark.AI LeNet-5 手写数字识别 Sample

## 介绍
该Sample基于MNIST数据集和LeNet-5模型，为海思智能终端芯片提供适配的量化、模型转换及端侧部署方案，可作为客户迁移部署手写数字识别模型的范式。

支持的芯片列表如下：
- **Hi3863**: 基于MSLite-Micro平台进行模型部署，依靠RISC-V CPU核进行AI推理。

## 目录结构
samples lenet5的目录结构如下所示：
```
samples
├── CMakeLists.txt
├── oh
│   ├── lenet5
│   │   ├── build.sh
│   │   ├── CMakeLists.txt
│   │   ├── data
│   │   │   └── README.md
│   │   ├── model
│   │   │   ├── mnist-12.onnx
│   │   │   └── README.md
│   │   ├── README.md
│   │   ├── scripts
│   │   │   └── preproc_mnist_data.py
│   │   └── src
│   │       └── ai_main.c
│   └── ......
└── README.md
```
- **build.sh脚本**: 用于编译Sample模型。需要配置对应的SDK_PATH 以及 ADAPTOR_PATH。
- **CMakeLists.txt**: Sample的编译框架，C代码实现。
- **model目录**: 用于存放对应的onnx原始模型。
- **scripts目录**: 用于存放对应的数据处理脚本，自动生成量化以及验证数据。
- **src文件夹**: 用于存放板端推理源文件源码。
- **README**: 此Sample的介绍。

## 预处理

- **模型准备：**
对应的原始模型 **mnist-12.onnx**已放置在model一级目录下。

- **数据准备：**
运行Scripts目录下的预处理脚本，自动下载对应的原始MNIST数据集，并处理和保存生成Sample需要的文件格式。
    ```
    python scripts/preproc_mnist_data.py --orig_path ./data --train_path ./data/train_data --test_path ./data/test_data --train_file_format bin --test_file_format all --test_data_type float32
    ```
    参数说明：
    - --orig_path：MNIST原始数据对应的目录
    - --train_path：训练集保存目录
    - --test_path：测试集保存目录
    - --train_file_format：训练集的保存格式，可选值包括bin，npy, all
    - --test_file_format：测试集的保存格式，可选值包括bin，npy, all
    - --test_data_type：测试集的bin格式文件的数据类型，可选值包括float16, float32  

Tips:
1. 网络问题导致下载出现问题，可以手动创建./data/MNIST/raw文件夹，并下载数据包 [train-images-idx3-ubyte.gz](https://ossci-datasets.s3.amazonaws.com/mnist/train-images-idx3-ubyte.gz)、[train-labels-idx1-ubyte.gz](https://ossci-datasets.s3.amazonaws.com/mnist/train-labels-idx1-ubyte.gz)、[t10k-images-idx3-ubyte.gz](https://ossci-datasets.s3.amazonaws.com/mnist/t10k-images-idx3-ubyte.gz)、[t10k-labels-idx1-ubyte.gz](https://ossci-datasets.s3.amazonaws.com/mnist/t10k-labels-idx1-ubyte.gz)到raw下，再执行脚本。

- **配置文件准备：**
新建micro_quant.cfg文件，其中train_data/bin替换为实际的绝对路径
    ```
    [micro_param]
    enable_micro=true
    target=RISCV
    support_parallel=false
    [common_quant_param]
    quant_type=FULL_QUANT
    bit_num=8
    [data_preprocess_param]
    calibrate_path=Input3:train_data/bin
    calibrate_size=60000
    input_type=BIN
    [full_quant_param]
    activation_quant_method=MAX_MIN
    bias_correction=true
    enable_all_ops=false
    ```

## 模型转换&编译&烧录调试
参考[HiSpark AI工具](https://gitee.com/HiSpark/hispark_ai/tree/master)README.md中"HiSpark.AI 平台快速入门指南"小节完成模型转换、编译、烧录调试  
其中模型转换时使用的cfg_path更改为上述准备的配置文件

## 常见问题
若出现GLIBC环境不符，或者python环境不符，依次配置gcc环境，python3.11环境，将libstdc++ / libpython的动态链接库添加到LD_LIBRARY_PATH中
```
# 1. 先链接mindspore-enterprise-lite-{version}-linux-x64/tools/converter/lib下的动态链接库
export LD_LIBRARY_PATH={PKG}/mindspore-enterprise-lite-{version}-linux-x64/tools/converter/lib:$LD_LIBRARY_PATH

# 2. 再链接 包含libpython3.11.so的同级目录，例如：
export LD_LIBRARY_PATH={Python3.11_ENV}/lib:$LD_LIBRARY_PATH

# 3. 配置gcc 6.0.30的动态链接库软链接
cd {gcc_lib}
ln -s libstdc++.so.6.0.30 libstdc++.so.6

# 3. 最后链接gcc的lib
export LD_LIBRARY_PATH={gcc_lib}:$LD_LIBRARY_PATH
```