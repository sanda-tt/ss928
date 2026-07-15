# HiSpark AI 开源项目

## 项目介绍 && 资源
HiSaprk AI是海思嵌入式AI应用开发解决方案，提供模型压缩、转换、推理等功能，可以结合社区已开源的WS63 SDK集成，开发AI应用。Sample目前已支持LeNet-5手写数字识别 及 Gru-S固定词语音识别相关应用。

## HiSpark AI目录介绍

| 目录   | 二级目录 | 介绍                                                             |
| ------ | ------  | ------------------------------------------------------------     |
| docs   |         |存放AI工具链使用指南，以及AI应用开发指南等文档，帮助客户快速熟悉HiSpark.AI解决方案 |
| src    | adaptor |AI平台所配套的适配层源码                                            |
| src    | samples |HiSpark.AI提供的Samples，用于指导用户基于基于HiSpark各平台部署AI应用                                                                   |
| src    | mindspore-lite | 基于RISC-V平台的AI推理框架，用于自动生成AI推理模块代码并 提供对应的RISC-V算子库                  |

## 社区版本介绍（外部链接）

## 生态板介绍链接
- **WS63系列单板**:  ws63系列是2.4GHz Wi-Fi 6 星闪多模解决方案，其中ws63E支持2.4GHz的雷达人体活动检测功能，适用于大小家电、电工照明及对人体出没检测有需求的常电类物联网智能场景，项目介绍如下：[WS63项目介绍](https://gitee.com/HiSpark/fbb_ws63)。 

  购买链接请参考WS63项目介绍的**购买渠道**页面。

## Sample案例说明
HiSpark.AI提供了一下Sample供开发参考：
| 平台 | 应用 | AI功能 |
| ---- | ---- | ---- |
| ws63 | LeNet-5手写数字图像识别 | MindSpore Lite Micro工具链量化，转换，编译，SDK集成 |
| ws63 | Gru-S音频固定词识别 | MindSpore Lite Micro工具链量化，转换，编译，SDK集成 |

### **HiSpark.AI CPU系列平台介绍**
超轻量的模型部署平台，支持KB级RAM嵌入式设备。

## 源码编译
### 环境依赖
| 软件名称   | 版本 | 作用                                                             |
| ------ | ------  | ------------------------------------------------------------     |
| Ubuntu |   22.04  |编译和运行MindSpore的操作系统                                      |
| GCC    | 11.3.0-12.3.0 |用于编译MindSpore的C++编译器                                  |
| CMake  | 3.22.2及以上	 |编译构建MindSpore的工具                                       |
| Python | 3.11 | MindSpore的使用依赖Python环境                                         |
| PyYAML | 6.0及以上 | MindSpore里的算子编译功能依赖PyYAML模块                           |
| Numpy  | 1.19.3及以上 | MindSpore里的Numpy相关功能依赖Numpy模块                        |

### 获取毕昇编译器
- 点击[毕昇编译器官方下载链接](https://developers.hisilicon.com/cn/developerTool)并登录华为开发者账号。
- 在资源下载页面，选择 Toolchain 分类下的 Linux 系统版本。
- 查找并下载适用于 RISC-V 架构 的编译器软件包，其名称为：	BiSheng-llvm-15.0.4-riscv-x86-linux（或最新版本）。
- 下载完成后，使用以下命令解压（请确保命令实际文件名与下载文件一致）。
    ```
    tar -xzvf BiSheng-llvm-15.0.4-riscv-x86-linux-25.09.1.tar.gz
    ```

### 编译MindSpore
- **拉取mindspore-lite仓submodule**  
运行以下命令：
    ```
    git submodule update --init --remote --progress src/mindspore-lite
    ```

- **编译mindspore-lite**
进入mindspore-lite目录,设置环境变量，/path替换为毕昇编译器解压后对应的目录
    ```
    # bisheng-compiler-bin-path为bisheng编译器的二进制文件目录，如/path/BiSheng-llvm-binary-release-musl/bin
    cd src/mindspore-lite
    export MSLITE_ENABLE_MICRO=ON
    export MSLITE_ENABLE_INT8=ON
    export MSLITE_ENABLE_TRAIN=OFF
    export MSLITE_ENABLE_TESTCASES=OFF
    export MSLITE_TARGET_RISCV=ON
    export HISPARK_RISCV_TOOLCHAIN_PATH=${bisheng-compiler-bin-path}/BiSheng-llvm-binary-release-musl/
    bash build.sh -I x86_64 -j32 # 执行编译脚本，可在执行中修改-j{线程数}来修改线程数量
    # 若需增量编译，使用bash build.sh -I x86_64 -j32 -i命令
    ```

## **HiSpark.AI 平台快速入门指南**

- **准备hispark_ai工具链**  
获取MSLite工具链，或根据上述源码编译指南进行编译。MSLite安装包目录结构如下：
    ```
    ├── runtime
    │   ├── include
    │   │   ├── api
    │   │   ├── c_api
    │   │   └── ...
    │   ├── lib
    │   │   ├── libmindspore-lite.so
    │   │   └── ...
    │   └── third_party
    └── tools
        ├── benchmark
        ├── codegen
        │   ├── include
        │   │   ├── nnacl_c
        │   │   └── wrapper
        │   └── lib
        │       ├── cpu
        │       └── riscv
        └── converter
            ├── converter
            │   └── converter_lite
            ├── include
            │   ├── api
            │   └── ...
            ├── lib
            │   ├── libmindspore_converter.so
            │   ├── libmindspore_core.so
            │   └── ...
            └── third_party
                └── proto
    ```

- **准备待部署模型与数据**
  - 准备好待部署模型。可直接使用 HiSpark.AI LeNet-5以及Gru Sample中的mnist-12.onnx以及GRU_S_STREAM.onnx。
  - 准备好量化数据。**无需量化可跳过此步骤。** 准备一个文件夹，将float32格式的量化数据存储为.bin格式，可直接使用 HiSpark.AI LeNet-5以及Gru Sample中的 运行数据预处理脚本之后的npy_data文件夹。

- **准备SDK**
  - 从开源社区下载fbb_ws63的源码
    ```
      git clone https://gitee.com/HiSpark/fbb_ws63.git
    ```

- **准备Samples**
  进入sample一级目录，如LeNet-5就进入{hispark_ai_root}/src/samples/OH/Lenet5目录，而Gru就进入{hispark_ai_root}/src/samples/OH/Gru目录。Sample目录结构如下：
    ```
    {sample_path}
    ├── build.sh
    ├── CMakeLists.txt
    ├── model
    │   ├── xxx.onnx
    │   └── README.md
    ├── README.md
    ├── scripts
    │   ├── preproc_xxx_data.py
    │   └── README.md
    └── src
        ├── ai_main.c
        ├── ai_main.h
        └── CMakeLists.txt
    ```

- **模型编译**
  - 使用MSLite包中带的converter_lite工具进行模型转换，生成目标代码
    ```
     # mslite_pkg_path变量为解压的HiSpark.AI MSLite压缩包路径，一级文件夹名称为mindspore-{package_item}-lite-{version}-linux-64
     # model_path为原始模型路径，如mnist-12.onnx
     # generate_code_path为生成代码路径
     # cfg_path为配置文件路径
     export PATH=${mslite_pkg_path}/tools/converter/converter:$PATH
     export LD_LIBRARY_PATH=${mslite_pkg_path}/tools/converter/lib:$LD_LIBRARY_PATH
     converter_lite --fmk=ONNX --modelFile={model_path} --outputFile={generate_code_path} --configFile={cfg_path} --inputDataFormat=NCHW --outputDataFormat=NCHW
    ```
    其中cfg_path所配置的文件内容如下：
    ```
    [micro_param]
    enable_micro=true
    target=RISCV
    support_parallel=false
    ```
  - 自动代码生成的目录如下
    ```
    {generate_code_path}
    ├── benchmark
    ├── CMakeLists.txt
    ├── include
    │   ├── model_handle.h
    │   └── ...
    └── src
        ├── allocator.c
        ├── allocator.h
        ├── CMakeLists.txt
        ├── context.c
        ├── context.h
        ├── model0
        │   ├── model0.c
        │   ├── net0.c
        │   ├── net0.h
        │   ├── weight0.c
        │   └── weight0.h
        ├── model.c
        ├── model.h
        ├── net.cmake
        ├── tensor.c
        └── tensor.h
    ```
  - 静态链接库编译
    ```
    # sdk_path为下载的SDK路径
    # mslite_pkg_path为HiSpark.AI的工具链路径
    # generate_code_path为生成代码路径
    # hcc_version为SDK编译器版本，如cc_riscv32_musl_105
    cd {generate_code_path}
    rm -rf build
    cmake -S . -B build -D OP_LIB="${mslite_pkg_path}/tools/codegen/lib/riscv/libnnacl.a" -D WRAPPER_LIB="${mslite_pkg_path}/tools/codegen/lib/riscv/libwrapper.a" -D RISCV_TOOLCHAIN_PATH="${sdk_path}/src/tools/bin/compiler/riscv/${hcc_version}/cc_riscv32_musl/bin" -D PKG_PATH="${mslite_pkg_path}"
    cd build
    make -j4
    ```
    编译产物存放于build文件夹下，目录结构如下。libnet.a以及libmicro_runtime.a分别放置在build/src路径以及build路径下：
    ```
    {generate_code_path}/build
    ├── CMakeCache.txt
    ├── CMakeFiles
    │   ├── x.xx.x
    │   ├── Makefile2
    │   └── ...
    ├── cmake_install.cmake
    ├── libmicro_runtime.a
    ├── Makefile
    └── src
        ├── CMakeFiles
        ├── cmake_install.cmake
        ├── libnet.a
        └── Makefile
    ```
    将libnet.a以及libmicro_runtime.a拷贝到${sdk_path}/src/middleware/utils/ai_mcu/lib目录下。
    ```
    # sdk_path为SDK的源码目录 (https://gitee.com/HiSpark/fbb_ws63)
    mkdir -p ${sdk_path}/src/middleware/utils/ai_mcu/lib
    cp -rf {generate_code_path}/build/libmicro_runtime.a ${sdk_path}/src/middleware/utils/ai_mcu/lib
    cp -rf {generate_code_path}/build/src/libnet.a ${sdk_path}/src/middleware/utils/ai_mcu/lib
    ```

- **SDK编译**
    配置对应环境变量，在samples下运行build.sh脚本，即可完成编译
    ```
    cd ${sample_path}
    export SDK_PATH=${sdk_path}/src
    export ADAPTOR_PATH=${adaptor_path}
    ./build.sh
    ```
    编译成功后，ws63-ai-liteos-sample.fwpkg镜像文件会生成在${sample_path}/output目录下

- **烧录调试**
    使用BurnTool工具进行ws63-ai-liteos-sample.fwpkg的烧录。
    烧录成功运行后，会看到串口打印的运行成功信息，如Gru下：
    ```
    [AI_MCU] Get Tcxo Time 115 ms
    [AI_MCU] Data size: [48]
    Shape: [1 12 ]
    DataType: 43
    [AI_MCU] Data: [0.95731][0.00266][0.00294][0.00590][0.00286][0.00374][0.00285][0.00685][0.00231][0.00307][0.00654][0.00292]
    [AI_MCU] ai_mcu_sample_process
    ```

## 参与贡献

- 参考[社区参与贡献指南](https://gitee.com/HiSpark/docs/blob/master/contribute/%E7%A4%BE%E5%8C%BA%E5%8F%82%E4%B8%8E%E8%B4%A1%E7%8C%AE%E6%8C%87%E5%8D%97.md)