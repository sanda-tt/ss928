# HiSpark.AI GRU 音频固定词识别 Sample

## 介绍
Gru固定词识别Sample基于Google开源的Open-Speech数据集 以及 ARM Softmax开源的KWS-MCU Benchmark的开源GRU模型，为海思智能终端芯片提供适配的量化，模型转换 以及 端侧部署的Sample。客户可以基于此Sample为范式迁移部署相应的固定词识别模型。

支持的芯片列表如下：
- **Hi3863**: 基于MSLite-Micro平台进行模型部署，依靠RISC-V CPU核进行AI推理。

## 数据处理 & 量化指南
### 预处理

- **准备模型**
对应的Gru原始模型 **GRU_S_STREAM.onnx**已放置在model一级目录下。

- **preprocess_wave_data**
运行Scripts目录下的预处理脚本，在samples一级目录下自动下载对应的原始Google Open-speech数据集。
```
python scripts/preproc_wav_data.py --data_root_dir ./data/origin_data --quant_data_dir ./data/quant_data --validation_data_dir ./data/validation_data --onnx_model_path ./model/GRU_S_STREAM.onnx --sample_num 50 [--fp16 true]
```
Tips:
1. 网络问题导致下载出现问题，可以手动创建./data/origin_data文件夹，并下载数据包 [speech_commands_v0.02.tar.gz](http://download.tensorflow.org/data/speech_commands_v0.02.tar.gz)到./data/origin_data下，再执行脚本。

### 模型量化

- **RISC-V平台量化指南**
1. 准备MindSpore资源包
2. 准备micro_quant.cfg文件：
```
[micro_param]
enable_micro=true
target=RISCV
support_parallel=false
[common_quant_param]
quant_type=FULL_QUANT
bit_num=8
[data_preprocess_param]
calibrate_path=mfcc_input:data/quant_data/quant_mfcc_input,hidden_states:data/quant_data/quant_hidden_states
calibrate_size=750
input_type=BIN
[full_quant_param]
activation_quant_method=MAX_MIN
bias_correction=true
enable_all_ops=false
```
Tips:
1. 这一步完成了量化转换前的配置工作，模型转换操作参考 RISC-V平台模型转换指南。

## RISC-V平台模型转换指南
- **模型代码生成**
将mindspore lib库的路径添加到LD_LIBRARY_PATH中
```
export LD_LIBRARY_PATH={PKG}/mindspore-enterprise-lite-{version}-linux-x64/tools/converter/lib:$LD_LIBRARY_PATH
```
接着开始进行模型的编译：
```
./{PKG}/mindspore-enterprise-lite-{version}-linux-x64/tools/converter/converter/converter_lite --fmk=ONNX --modelFile={MODEL_PATH}/xxx.onnx --outputFile={OUTPUT_PATH} --configFile={MODEL_PATH}/micro_qunat.cfg --encryption=false --inputDataFormat=NCHW --outputDataFormat=NCHW --inputDataType=FLOAT --outputDataType=FLOAT
```
以Onnx为例，{MODEL_PATH}模型用例路径，{PKG}表示整包路径，{OUTPUT_PATH}表示自动生成项目的输出路径。

- **链接库编译（二次编译）**
到达生成库同级路径，并且解压WS63中SDK包，找到编译链路径，默认推荐：
```
COMPILER_PATH={SDK}/tools/bin/compiler/riscv/cc_riscv32_musl_{version}/cc_riscv32_musl/bin
```
之后进行链接库编译：
```
cd {OUTPUT_PATH}
rm -rf build
cmake -S . -B build \
        -D OP_LIB="{PKG}/mindspore-enterprise-lite-{version}-linux-x64/tools/codegen/lib/riscv/libnnacl.a" \
        -D WRAPPER_LIB="{PKG}/mindspore-enterprise-lite-{version}-linux-x64/tools/codegen/lib/riscv/libwrapper.a" \
        -D RISCV_TOOLCHAIN_PATH={COMPILER_PATH} \
        -D PKG_PATH="{PKG}/mindspore-enterprise-lite-{version}-linux-x64"
cd build
make -j4
```
编译完成后，将build文件夹生成的libnet.a以及libmicro_runtime.a拷贝到SDK对应路径下：
```
cp {OUTPUT_PATH}/build/libmicro_runtime.a {SDK}/middleware/utils/ai_mcu/lib
cp {OUTPUT_PATH}/build/src/libnet.a {SDK}/middleware/utils/ai_mcu/lib
```
之后参考运行指南，完成samples的修改，以及samples的编译：


## RISC-V平台编译指南
1. 获取Hi3863 SDK的代码，保存在用户指定路径，其路径为{SDK_PATH}。
    路径如下表示解压成功，且目录正确：
    ```
    {SDK_PATH}
        |---- application
        |---- drivers
        |---- build
        |---- protocol
        |---- ....
        |---- build.py
    ```
2. 获取HiSpark.AI Adaptor包，并进行解压,其以及路径为{ADAPTOR_PATH}。
    解压命令为；tar -zxvf HiSpark.AI_{version}-adaptor.tar.gz -C ${ADAPTOR_PATH}
    路径如下表示解压成功，且目录正确：
    ```
    {ADAPTOR_PATH}
        |---- adaptor
        |---- include
    ```
3. 获取此HiSpark.AI Samples包
4. 根据业务修改Sample包，根据下方新版本Sample的说明
5. 将HiSpark.AI MSLite Micro工具链编译出的libmicro_runtime.a libnet.a复制到SDK的路径下
        即${SDK_PATH}/middleware/utils/ai_mcu/lib目录下，若目录不存在则需要创建此目录
6. 在命令行输入：
```
export SDK_PATH=${SDK_PATH}
export ADAPTOR_PATH=${ADAPTOR_PATH}
./build.sh
```
7. 获取编译成功的fwpkg文件，在${SDK_PATH}/output/ws63/fwpkg/ws63-liteos-app/ws63-liteos-app_all.fwpkg路径下

## 目录结构
Gru Samples的目录结构如下所示：
```
samples
├── CMakeLists.txt
├── OH
│   ├── Gru
│   │   ├── build.sh
│   │   ├── CMakeLists.txt
│   │   ├── model
│   │   │   ├── GRU_S_STREAM.onnx
│   │   │   └── README.md
│   │   ├── README.md
│   │   ├── scripts
│   │   │   ├── preproc_wav_data.py
│   │   │   └── README.md
│   │   └── src
│   │       └── ai_audio_main.c
│   │       └── ai_audio_main_npu.c
│   │       └── ai_main.h
│   │       └── CMakeLists.txt
│   └── ......
└── README.md
```
- **build.sh脚本**: 用于编译Sample模型。需要配置对应的SDK_PATH 以及 ADAPTOR_PATH。
- **CMakeLists.txt**: Sample的编译框架，C代码实现。
- **model目录**: 用于存放对应的onnx原始模型。
- **scripts目录**: 用于存放对应的数据处理脚本，自动生成量化以及验证数据。
- **src文件夹**: 用于存放板端推理源文件源码。
- **README**: 此Sample的介绍。

## 资源下载链接

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