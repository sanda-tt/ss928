# YOLOv8 模型移植到 SS928 板子详细教程

> 基于官方资料 `09. 进阶功能开发/04.Yolov8移植` 整理  
> 适用场景：已有在 PC 上运行正常的 YOLOv8 模型，需要移植到 SS928 开发板运行

---

## 一、移植整体流程概览

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   PC端环境搭建   │ --> │  PT 转 ONNX     │ --> │ ONNX 转 OM      │ --> │  板端部署运行   │
│                 │     │                 │     │                 │     │                 │
│ 1. 安装Miniconda│     │ 1. 导出ONNX     │     │ 1. 配置op.cfg   │     │ 1. 拷贝OM模型   │
│ 2. 创建yolov8环境│    │ 2. 验证ONNX     │     │ 2. ATC转换      │     │ 2. 设置环境变量 │
│ 3. 创建atc环境  │     │                 │     │                 │     │ 3. 运行Sample   │
│ 4. 安装CANN工具 │     │                 │     │                 │     │                 │
└─────────────────┘     └─────────────────┘     └─────────────────┘     └─────────────────┘
```

**核心思路**：SS928 板子使用 NPU 进行推理，需要专用的 `.om` 格式模型。所以移植过程就是把 PC 上的 `.pt` 模型 → `.onnx` → `.om`。

---

## 二、第一步：PC 端环境搭建

### 2.1 安装 Miniconda（如果还没有）

```bash
# 下载 Miniconda
wget https://mirrors.tuna.tsinghua.edu.cn/anaconda/miniconda/Miniconda3-latest-Linux-x86_64.sh

# 添加执行权限并安装
chmod +x Miniconda3-latest-Linux-x86_64.sh
./Miniconda3-latest-Linux-x86_64.sh -b -p ~/miniconda3
```

### 2.2 创建 YOLOv8 Python 环境（用于模型导出）

```bash
# 创建虚拟环境（python>=3.8）
conda create -n yolov8 python=3.10

# 激活环境
conda activate yolov8

# 升级 pip
pip install --upgrade pip
```

### 2.3 安装 YOLOv8 依赖

创建 `requirements_yolov8.txt` 文件，内容如下：

```
# Base ----------------------------------------
matplotlib>=3.2.2
numpy==1.26.4
opencv-python==4.8.1.78
Pillow>=7.1.2
PyYAML>=5.3.1
requests>=2.23.0
scipy>=1.4.1
torch==2.1.0
torchvision==0.16.0
tqdm>=4.64.0

# Plotting ------------------------------------
pandas>=1.1.4
seaborn>=0.11.0

# Export --------------------------------------
onnx>=1.12.0
onnxsim>=0.4.1

# Extras --------------------------------------
psutil
thop>=0.1.1
```

安装依赖：

```bash
pip install -r requirements_yolov8.txt -i https://pypi.tuna.tsinghua.edu.cn/simple
pip install ultralytics
```

### 2.4 创建 ATC 环境（用于模型转换）

```bash
# 创建 atc 环境
conda create -n atc python=3.9.2

# 激活环境
conda activate atc
```

### 2.5 安装 ATC 相关依赖

```bash
pip3 install protobuf==3.13.0 --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install psutil==5.7.0 --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install numpy --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install scipy --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install decorator==4.4.0 --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install sympy==1.5.1 --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install cffi==1.12.3 --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install pyyaml --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install pathlib2 --user -i https://pypi.tuna.tsinghua.edu.cn/simple
```

### 2.6 安装 CANN 工具包（ATC 转换必备）

> 这是最关键的一步！ATC 工具在 CANN 工具包中。

**找到安装包**：
在官方 SDK 文件夹中找到：
- `SVP_PC/NNN_PC/Ascend-cann-toolkit_5.20.t6.2.b060_linux-x86_64.run`

或者参考：`ReleaseDoc\zh\01.software\pc\NNN\驱动和开发环境安装指南.pdf`

**安装**：

```bash
# 添加执行权限
chmod +x Ascend-cann-toolkit_5.20.t6.2.b060_linux-x86_64.run

# 执行安装
./Ascend-cann-toolkit_5.20.t6.2.b060_linux-x86_64.run --install

# 设置环境变量（每次使用ATC前都要执行）
source /usr/local/Ascend/ascend-toolkit/5.20.t6.2.b060/x86_64-linux/bin/setenv.bash
```

> 注意：`/usr/local/Ascend/ascend-toolkit/` 是默认安装路径，如果你安装在其他位置，请替换为实际路径。

---

## 三、第二步：PT 模型转 ONNX

### 3.1 确认你的模型文件

确保你有一个 `.pt` 模型文件，例如 `yolov8n.pt` 或你自己训练好的模型 `best.pt`。

### 3.2 导出 ONNX 模型

**方法：使用 Python 脚本导出**

创建 `export_onnx.py` 文件：

```python
from ultralytics import YOLO

# 加载你的模型（替换为你的模型路径）
model = YOLO("yolov8n.pt")

# 导出 ONNX 模型
# opset=12 是 SS928 兼容的版本
success = model.export(format="onnx", opset=12)

print("ONNX 导出成功！")
```

**运行导出**：

```bash
# 确保在 yolov8 环境下
conda activate yolov8

python export_onnx.py
```

导出成功后，会在模型同级目录生成 `yolov8n.onnx` 文件。

### 3.3 验证 ONNX 模型（可选）

可以用以下代码简单测试 ONNX 模型是否正常：

```python
import onnx

# 加载并检查 ONNX 模型
model = onnx.load("yolov8n.onnx")
onnx.checker.check_model(model)
print("ONNX 模型验证通过！")
```

---

## 四、第三步：ONNX 转 OM（核心转换步骤）

### 4.1 配置 AIPP 预处理文件 `op.cfg`

> **什么是 AIPP？**  
> AIPP（Artificial Intelligence Pre-Processing）是昇腾 NPU 的图像预处理模块。在模型转换时配置好，板端推理时就不需要做预处理了，直接输入 YUV420 图像即可。

创建 `op.cfg` 文件，内容如下：

```
aipp_op {
    aipp_mode : static
    related_input_rank : 0
    max_src_image_size : 1228800
    support_rotation : false
    input_format : YUV420SP_U8
    src_image_size_w : 640
    src_image_size_h : 640
    cpadding_value: 0.0
    csc_switch : true
    rbuv_swap_switch : false
    ax_swap_switch : false
    matrix_r0c0 : 256
    matrix_r0c1 : 0
    matrix_r0c2 : 359
    matrix_r1c0 : 256
    matrix_r1c1 : -88
    matrix_r1c2 : -183
    matrix_r2c0 : 256
    matrix_r2c1 : 454
    matrix_r2c2 : 0
    input_bias_0 : 0
    input_bias_1 : 128
    input_bias_2 : 128
    crop : false
    load_start_pos_h : 0
    load_start_pos_w : 0
    crop_size_w : 640
    crop_size_h : 640
    min_chn_0 : 0.0
    min_chn_1 : 0.0
    min_chn_2 : 0.0
    var_reci_chn_0 : 0.0039215686274509803921568627451
    var_reci_chn_1 : 0.0039215686274509803921568627451
    var_reci_chn_2 : 0.0039215686274509803921568627451
}
```

**参数说明**：
- `input_format: YUV420SP_U8`：输入图像格式为 YUV420SP（板端摄像头常见输出格式）
- `src_image_size_w/h: 640`：输入图像宽高（YOLOv8 默认 640x640）
- `csc_switch: true`：开启色域转换（YUV → RGB）
- `var_reci_chn: 0.00392...`：归一化系数（即 1/255）

### 4.2 执行 ATC 转换

**方法 1：直接命令行转换**

```bash
# 切换到 atc 环境
conda activate atc

# 设置 CANN 环境
source /usr/local/Ascend/ascend-toolkit/5.20.t6.2.b060/x86_64-linux/bin/setenv.bash

# 执行转换
atc --model=yolov8n.onnx \
    --framework=5 \
    --output=yolov8n \
    --soc_version="OPTG" \
    --output_type=FP32 \
    --insert_op_conf=./op.cfg
```

**参数说明**：
- `--model`：输入的 ONNX 模型路径
- `--framework=5`：5 代表 ONNX 格式
- `--output`：输出的 OM 模型文件名（不需要加 .om 后缀）
- `--soc_version="OPTG"`：SS928 芯片版本号
- `--output_type=FP32`：输出数据类型
- `--insert_op_conf`：AIPP 配置文件路径

**方法 2：使用脚本转换（推荐）**

创建 `onnx2om.sh` 脚本：

```bash
#!/bin/bash

# 设置 CANN 环境
source /usr/local/Ascend/ascend-toolkit/latest/x86_64-linux/bin/setenv.bash

# 执行转换
atc --model=$1 \
    --framework=5 \
    --output=$2 \
    --soc_version="OPTG" \
    --output_type=FP32 \
    --insert_op_conf=./op.cfg

echo "转换完成！输出文件: $2.om"
```

使用方法：

```bash
chmod +x onnx2om.sh
./onnx2om.sh yolov8n.onnx yolov8n
```

### 4.3 转换成功标志

转换成功后，会生成 `yolov8n.om` 文件。如果看到类似以下输出，说明成功：

```
ATC start working now, please wait for a moment.
...
ATC run success, welcome to the next use.
```

---

## 五、第四步：板端部署与运行

### 5.1 将 OM 模型拷贝到板端

使用 `scp` 或 MobaXterm 等工具，将生成的 `yolov8n.om` 拷贝到 SS928 板子上。

```bash
# 示例：使用 scp
scp yolov8n.om root@192.168.1.10:/root/
```

### 5.2 板端运行

**步骤 1：设置环境变量**

在板端终端执行：

```bash
export LD_LIBRARY_PATH=/opt/lib/npu:$LD_LIBRARY_PATH
```

**步骤 2：运行 Sample 程序**

官方提供了 `sample_yolov8_imx347` 示例程序（在 V2.0.0 版本固件中）。

```bash
# 确保模型文件名为 yolov8n.om（或在代码中修改路径）
./sample_yolov8_imx347
```

> **注意**：代码中默认只在置信度 ≥ 0.85 时绘制检测框。如果测试时看不到框，可以将阈值调低至 0.50。

---

## 六、常见问题与解决方案

### Q1: ATC 转换报错 "soc_version not found"

**解决**：确认 `--soc_version="OPTG"` 参数正确，且 CANN 工具包安装完整。

### Q2: 板端运行提示找不到 NPU 库

**解决**：确保执行了环境变量设置：
```bash
export LD_LIBRARY_PATH=/opt/lib/npu:$LD_LIBRARY_PATH
```

### Q3: 检测框置信度阈值太高，看不到结果

**解决**：修改代码中的置信度阈值，从 0.85 改为 0.50 或更低。

### Q4: ONNX 导出失败

**解决**：
1. 确认 `opset=12`
2. 确认 torch 版本兼容（推荐 torch==2.1.0 或 torch==1.12.0）
3. 尝试简化 ONNX：`onnxsim input.onnx output.onnx`

### Q5: 模型输入尺寸不是 640x640？

**解决**：修改 `op.cfg` 中的 `src_image_size_w` 和 `src_image_size_h` 为你模型的实际输入尺寸。

---

## 七、如果你是自定义训练模型

如果你需要**训练自己的数据集**再移植，完整流程是：

```
1. 准备数据集（YOLO格式）
   --> 2. 标注数据（LabelImg工具）
      --> 3. 训练模型（yolo detect train）
         --> 4. 导出 ONNX（model.export）
            --> 5. ATC 转 OM
               --> 6. 板端部署
```

训练命令示例：

```bash
yolo detect mode=train data=./test.yaml model=yolov8n.pt epochs=150 batch=16 imgsz=640 device=0
```

---

## 八、关键文件清单

| 文件 | 说明 | 位置 |
|------|------|------|
| `yolov8n.pt` | 原始 PyTorch 模型 | 你自己准备 |
| `yolov8n.onnx` | ONNX 中间模型 | 导出生成 |
| `yolov8n.om` | 板端运行模型 | ATC 转换生成 |
| `op.cfg` | AIPP 预处理配置 | 手动创建 |
| `export_onnx.py` | ONNX 导出脚本 | 手动创建 |
| `onnx2om.sh` | ATC 转换脚本 | 手动创建 |

---

## 九、总结

移植的核心就是 **"PT → ONNX → OM"** 三步：

1. **环境准备**：装 Miniconda → 建 yolov8 环境 → 建 atc 环境 → 装 CANN
2. **导出 ONNX**：用 ultralytics 的 `model.export(format="onnx", opset=12)`
3. **ATC 转换**：用 `atc` 命令 + `op.cfg` 配置文件生成 `.om`
4. **板端运行**：拷贝 `.om` → 设环境变量 → 执行 sample

整个流程在 PC（Ubuntu 虚拟机或物理机）上完成模型转换，然后把 `.om` 文件丢到板子上就能跑。

---

> 参考文档：
> - `09. 进阶功能开发/04.Yolov8移植/01.Yolov8模型转换与部署.pdf`
> - `09. 进阶功能开发/04.Yolov8移植/02.Yolov8模型训练与部署（完整版）.pdf`
