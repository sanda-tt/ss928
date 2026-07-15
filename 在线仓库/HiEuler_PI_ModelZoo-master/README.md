# HiEuler_PI_ModelZoo

## 介绍
海鸥派基于HiSpark ModelZoo的应用仓库，在海鸥派上对HiSpark ModelZoo模型库进行验证并优化应用过程。通过本仓库可以在海鸥派上快速上手体验HiSpark ModelZoo上的各大开源模型。HiSpark ModelZoo代码仓及网站：

- [HiSpark ModelZoo代码仓](https://gitee.com/HiSpark/modelzoo)
- [HiSpark ModelZoo网站](https://modelzoo.hispark.hisilicon.com/#/ModelZoo)

## 声明

目前HiEuler_PI_ModelZoo仅是基于HiSpark ModelZoo上模型的应用，暂不涉及模型算法方面。

## 目录

```shell
├── docs			# 各种模型的应用文档
├── LICENSE
├── README.md
├── scripts			# 部分模型应用过程所需脚本
└── src				# HiSpark ModelZoo源码目录
```

## 代码仓下载

下载主仓。

```shell
git clone https://gitee.com/hieulerpi/HiEuler_PI_ModelZoo.git
```

下载子仓。

```shell
cd HiEuler_PI_ModelZoo
git submodule init
git submodule update src
```

## 使用说明

- 根据[《环境准备》](./docs/环境准备.md)文档，搭建开发环境和开发板环境。
- 按照下表选择已验证好的模型即可开始体验。

| 序号 | 类型                           | 模型            | 介绍                                                         | 应用文档                                                     |
| ---- | ------------------------------ | --------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 00   | classification<br/>图像分类    | ResNet50        | ResNet是残差网络(Residual Network)的缩写,该系列网络广泛用于目标分类等领域以及作为计算机视觉任务主干经典神经网络的一部分，典型的网络有ResNet50, ResNet101等。ResNet证明网络能够向更深（包含更多隐藏层）的方向发展 | [ResNet50应用指南](./docs/classification/ResNet50应用指南.md) |
| 01   | detection<br/>图像检测         | YOLO11s         | YOLO系列网络模型是最为经典的one-stage算法，也是目前工业领域使用最多的目标检测网络，YOLO11网络模型是YOLO系列的最新版本，在继承了原有YOLO网络模型优点的基础上，在架构和训练方法上进行了重大改进，具有更高的检测精度、速度和效率 | [YOLO11s应用指南](./docs/detection/YOLO11s应用指南.md)       |
| 02   | segmentation<br/>图像分割      | UNet            | UNet是由FCN改进而来的图像分割模型，其网络结构像U型，分为特征提取部分和上采样特征融合部分。 | [UNet应用指南](./docs/segmentation/UNet应用指南.md)          |
| 03   | segmentation<br/>图像分割      | YOLO11s-seg     | YOLO系列网络模型是最为经典的one-stage算法，也是目前工业领域使用最多的目标检测网络，YOLO11网络模型是YOLO系列的最新版本，在继承了原有YOLO网络模型优点的基础上，在架构和训练方法上进行了重大改进，具有更高的检测精度、速度和效率。YOLO11s-seg作为实例分割的模型，比检测模型更进一步，包括识别图像中的各个对象并将它们与图像的其余部分分割开来。 | [YOLO11s-seg应用指南](./docs/segmentation/YOLO11s-seg应用指南.md) |
| 04   | recognition<br />人脸识别      | FaceNet         | FaceNet 是一种基于深度卷积神经网络的端到端人脸识别与特征嵌入模型。相比传统基于手工特征或分阶段匹配的方法，它通过将人脸图像直接映射为固定维度的紧凑特征向量（Embedding），并采用三元组损失（Triplet Loss）优化特征相似度度量，能有效缩小类内差异、扩大类间距离，兼顾识别精度与推理效率，适用于身份验证、人脸检索、监控安防等大规模人脸识别场景。 | [FaceNet应用指南](./docs/recognition/FaceNet应用指南.md)     |
| 05   | ocr<br />文字识别              | PaddleOCRv4-rec | PP-OCRv4识别模型在PP-OCRv3的基础上进一步升级。整体的框架保持了与PP-OCRv3识别模型相同的pipeline，分别进行了数据、网络结构、训练策略等方面的优化。 | [PaddleOCRv4_rec应用指南](./docs/ocr/PaddleOCRv4_rec应用指南.md) |
| 06   | pose<br />姿态估计             | HRNet           | HigherHRNet 是一种新型的自下而上人体姿态估计算法，它在训练阶段引入多分辨率监督机制，在推理阶段采用多分辨率聚合策略，不仅能有效应对自下而上多人姿态估计任务中的尺度变化难题，还可实现关键点的高精度定位，尤其在小尺寸人体目标的处理上表现突出。 | [HRNet应用指南](./docs/pose/HRNet应用指南.md)                |
| 07   | pose<br />姿态估计             | YOLO11s-pose    | YOLO系列网络模型是最为经典的one-stage算法，也是目前工业领域使用最多的目标检测网络，YOLO11网络模型是YOLO系列的最新版本，在继承了原有YOLO网络模型优点的基础上，在架构和训练方法上进行了重大改进，具有更高的检测精度、速度和效率。YOLO11s-pose作为YOLO11的姿态估计的模型，能检测出代表人体不同部位的17个关键点。 | [YOLO11s-pose应用指南](./docs/pose/YOLO11s-pose应用指南.md)  |
| 08   | depth<br />双目深度            | LRStereo-B      | LRStereo-B是一个轻量且鲁棒的双目立体匹配模型。它在开源模型(Raft-Stereo)的基础上做了大量的模型结构改进和重训。具体功能为输入标定好的左右目图像以及相关的相机参数，获得左目图像对应的深度图。 | [LRStereo-B应用指南](./docs/depth/LRStereo-B应用指南.md)     |
| 09   | point<br />特征点检测          | SuperPoint      | SuperPoint模型的全卷积神经网络架构对全尺寸图像进行操作，并在单次前向传递中产生伴随固定长度描述符的兴趣点检测。该模型有一个单一的共享编码器来处理和减少输入图像的维数。在编码器之后，该架构分成两个解码器“头”，它们学习任务特定权重——一个用于兴趣点检测，另一个用于感兴趣点描述。大多数网络参数在两个任务之间共享，这与传统系统不同，传统系统首先检测兴趣点，然后计算描述符，并且缺乏跨两个任务共享计算和表示的能力。 | [SuperPoint应用指南](./docs/point/SuperPoint应用指南.md)     |
| 10   | count<br />人群计数            | CrowdCount      | CrowdCount是一种基于多尺度卷积神经网络（MSCNN）的高精度人群计数模型。相比传统多列 / 多网络方法，它通过单列网络中的多尺度特征块（MSB）与尺度自适应密度图回归技术，能有效应对透视畸变导致的人物尺度差异问题，兼顾计数精度与模型轻量化，适用于监控图像、公共场所等密集人群计数场景。 | [CrowdCount应用指南](./docs/count/CrowdCount应用指南.md)     |
| 11   | super_resolution<br />图像超分 | VDSR            | VDSR（Very Deep Super-Resolution Network）是一种20层深度卷积神经网络，通过残差学习实现图像超分辨率重建。 | [VDSR应用指南](./docs/super_resolution/VDSR应用指南.md)      |
| 12   | vlm<br />图文交互              | MiniCPM-4v-0.5B | MiniCPM-4v-0.5B: 小参数, 大智慧——端侧多模态模型 由面壁智能(OpenBMB)打造的MiniCPM-4v-0.5B, 以0.53B的精简参数量，在端侧设备上实现了卓越的图文理解与交互能力。专为边缘计算场景设计，让每一分算力都充分发挥价值。 三大核心优势：    1. 创新架构，轻装上阵 通过创新的稀疏感知训练和视觉压缩技术，实现 16:1 视觉特征压缩比，在保证高精度的同时，极大降低了推理算力消耗与内存占用。    2. 高能数据，以小博大 依托高密度数据体系，数据准备成本下降90%。通过汇聚全球高质量语料进行精细化对齐，用优质的数据训练模型，实现越级性能表现。    3. 高效训练，成本锐减 采用原创 WSD 调度策略与“模型风洞”技术，相比传统方案，搜索算力节省超99%，整体训练成本节省约60%，为端侧模型的持续迭代提供高效路径。 海思平台技术支撑： 现已适配 Hi3403V100 平台，持续生成速度达 21 tokens/s，为边缘侧设备提供流程、敏捷的智能视觉体验。 | [MiniCPM-4v-0.5B应用指南](./docs/vlm/MiniCPM-4v-0.5B应用指南.md) |

