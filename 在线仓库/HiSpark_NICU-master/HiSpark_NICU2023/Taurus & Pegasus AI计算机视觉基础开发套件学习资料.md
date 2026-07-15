#  Taurus & Pegasus AI计算机视觉基础开发套件学习资料

# 前言

**1、开发环境要求**

| 序号 | 名称    | 要求                                                         |
| ---- | ------- | ------------------------------------------------------------ |
| 01   | Windows | Windows10 64位系统，系统的用户名不能含有中文                 |
| 02   | Ubuntu  | Ubuntu18.04版本，运行内存推荐8G及以上，磁盘空间推荐100G及以上 |

**2、注意事项**

* 芯片限制：由于**Hi3516DV300 NNIE（Neuron Network Inference Engine，用于进行卷积神经网络运算的加速引擎）** 配套软件及工具链仅支持**Caffe1.0 框架**，使用其他框架的网络模型需要转化为Caffe框架下的模型，参考案例提供了**Pytorch2Caffe**模型转换方案，如果开发者需要将其他框架的网络模型转化为Caffe框架下的模型，需自行搭建并验证模型转换方案。
* 框架及算法限制：参考案例只提供了**基于Resnet18的分类网和基于YOLOV2的检测网**，如果开发者需要使用其他网络，需自行搭建训练环境，并参考学习资料中提供的案例进行端到端的模型部署和功能开发。

**3、在学习嵌入式开发资料之前，请学习以下基础知识**

<font color='RedOrange'>**（注意：以下链接仅用于参考，未经允许不得转发用于商业用途）**</font>

* [Linux及Ubuntu的基础知识](https://www.bilibili.com/video/BV1nW411L7xm/?p=10&vd_source=96a69ef20a8f8fe17acd0c89f155ea13)（可从P10开始学起至P58，具体根据自己的需求选择性学习）
* [Docker的基础知识](https://www.bilibili.com/video/BV1wQ4y1Y7SE/?p=2&vd_source=96a69ef20a8f8fe17acd0c89f155ea13)（具体根据自己的需求选择性学习）
* [Python基础知识](https://www.runoob.com/python3/python3-tutorial.html)(具体根据自己的需求选择性学习)
* [C语言基础知识](https://www.runoob.com/cprogramming/c-tutorial.html)(具体根据自己的需求选择性学习）
* [C++基础知识](https://www.runoob.com/cplusplus/cpp-tutorial.html)(具体根据自己的需求选择性学习)

**4、嵌入式大赛赋能培训视频资料列表**

| 序列 | 参考文档                                                     | 参考视频链接                                                 |
| ---- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 01   | [《Hi3861 openharmony版本的SDK介绍》](https://gitee.com/HiSpark/HiSpark_NICU2022/blob/master/doc/09%20Hi3861%20OpenHarmony%E7%89%88%E6%9C%AC%E4%BD%BF%E7%94%A8%E4%BB%8B%E7%BB%8D.pdf) | [Hi3861社区SDK（openharmony版本）介绍](https://www.hisilicon.com/-/media/Hisilicon2020/Images/component/chip-academy/videolist/videoup/videov27.mp4) |
| 02   | [ 《WiFi-IOT芯片Hi3861 SDK使用介绍》](https://gitee.com/HiSpark/HiSpark_NICU2022/blob/master/doc/10%20WiFi-IOT%E8%8A%AF%E7%89%87Hi3861%20SDK%E4%BD%BF%E7%94%A8%E4%BB%8B%E7%BB%8D.pdf) | [WiFi-IOT芯片Hi3861 SDK使用介绍](https://www.hisilicon.com/-/media/Hisilicon2020/Images/component/chip-academy/videolist/videoup/videov28.mp4) |

| 序列 | 参考文档                                                     | 参考视频链接                                                 |
| ---- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 01   | [《OpenHarmony hi3516DV300 SDK包简介》](https://gitee.com/HiSpark/HiSpark_NICU2022/blob/master/doc/13%20OpenHarmony%20hi3516DV300%20SDK%E5%8C%85%E7%AE%80%E4%BB%8B.pdf) | [OpenHarmony Hi3516DV300 SDK包简介](https://www.hisilicon.com/-/media/Hisilicon2020/Images/component/chip-academy/videolist/videoup/videov31.mp4) |
| 02   | [《Hi3516DV300 媒体业务典型场景介绍》](https://gitee.com/HiSpark/HiSpark_NICU2022/blob/master/doc/14%20Hi3516DV300%20%E5%AA%92%E4%BD%93%E4%B8%9A%E5%8A%A1%E5%85%B8%E5%9E%8B%E5%9C%BA%E6%99%AF%E4%BB%8B%E7%BB%8D.pdf) | [Hi3516DV300媒体业务场景介绍](https://www.hisilicon.com/-/media/Hisilicon2020/Images/component/chip-academy/videolist/videoup/videov32.mp4) |
| 03   | [《Hi3516DV300 智能业务典型场景介绍》](https://gitee.com/HiSpark/HiSpark_NICU2022/blob/master/doc/15%20Hi3516DV300%20%E6%99%BA%E8%83%BD%E4%B8%9A%E5%8A%A1%E5%85%B8%E5%9E%8B%E5%9C%BA%E6%99%AF%E4%BB%8B%E7%BB%8D.pdf) | [Hi3516DV300智能业务典型场景介绍](https://www.hisilicon.com/-/media/Hisilicon2020/Images/component/chip-academy/videolist/videoup/videov33.mp4) |
| 04   | [《Hi3516DV300 SVP介绍》](https://gitee.com/HiSpark/HiSpark_NICU2022/blob/master/doc/16%20Hi3516DV300%20SVP%E4%BB%8B%E7%BB%8D.pdf) | [Hi3516DV300 SVP介绍](https://www.hisilicon.com/-/media/Hisilicon2020/Images/component/chip-academy/videolist/videoup/videov34.mp4) |
| 05   | [《IVE sdk sample介绍》](https://gitee.com/HiSpark/HiSpark_NICU2022/blob/master/doc/17%20IVE%20sdk%20sample%E4%BB%8B%E7%BB%8D.pdf) | [IVE sdk sample介绍](https://www.hisilicon.com/-/media/Hisilicon2020/Images/component/chip-academy/videolist/videoup/videov35.mp4) |
| 06   | [《Hi3516DV300 HIGV介绍》](https://gitee.com/HiSpark/HiSpark_NICU2022/blob/master/doc/18%20Hi3516DV300%20HIGV%E4%BB%8B%E7%BB%8D.pdf) | [Hi3516DV300 HIGV介绍](https://www.hisilicon.com/-/media/Hisilicon2020/Images/component/chip-academy/videolist/videoup/videov36.mp4) |
| 07   | [《NNIE开发工具RuyiStudio介绍》](https://gitee.com/HiSpark/HiSpark_NICU2022/blob/master/doc/19%20NNIE%E5%BC%80%E5%8F%91%E5%B7%A5%E5%85%B7RuyiStudio%E4%BB%8B%E7%BB%8D.pdf) | [NNIE开发工具RuyiStudio介绍](https://www.hisilicon.com/-/media/Hisilicon2020/Images/component/chip-academy/videolist/videoup/videov37.mp4) |
| 08   | [《NNIE精度损失问题解决方法》](https://gitee.com/HiSpark/HiSpark_NICU2022/blob/master/doc/20%20NNIE%E7%B2%BE%E5%BA%A6%E6%8D%9F%E5%A4%B1%E9%97%AE%E9%A2%98%E8%A7%A3%E5%86%B3%E6%96%B9%E6%B3%95.pdf) | [NNIE精度损失问题解决方法](https://www.hisilicon.com/-/media/Hisilicon2020/Images/component/chip-academy/videolist/videoup/videov38.mp4) |



# 第一章：硬件准备

## [1.1、Pegasus套件](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/1%20%E7%A1%AC%E4%BB%B6%E5%87%86%E5%A4%87.md#11pegasus%E5%A5%97%E4%BB%B6)

## [1.2、Taurus套件](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/1%20%E7%A1%AC%E4%BB%B6%E5%87%86%E5%A4%87.md#12taurus%E5%A5%97%E4%BB%B6)

## [1.3、硬件验证](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/1%20%E7%A1%AC%E4%BB%B6%E5%87%86%E5%A4%87.md#13%E7%A1%AC%E4%BB%B6%E9%AA%8C%E8%AF%81)



# 第二章：开发环境搭建

## [2.1、Pegasus开发环境搭建](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/2.1%20Pegasus%E5%BC%80%E5%8F%91%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA.md)

## [2.2、Taurus开发环境搭建](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/2.2%20Taurus%E5%BC%80%E5%8F%91%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA.md)



# 第三章：运行第一个程序

## [3.1、Pegasus（HelloWorld）](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/3.1%20Pegasus_HelloWorld.md#31pegasushelloworld)

## [3.2、Taurus（HelloWorld）](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/3.2%20Taurus_HelloWorld.md#32taurushelloworld)



# 第四章、Pegasus套件的课程及案例

## [4.1、Pegasus芯片平台介绍](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/4.1%20Pegasus%E8%8A%AF%E7%89%87%E5%B9%B3%E5%8F%B0%E4%BB%8B%E7%BB%8D.md#%E7%AC%AC%E5%9B%9B%E7%AB%A0pegasus%E5%A5%97%E4%BB%B6%E7%9A%84%E8%AF%BE%E7%A8%8B%E5%8F%8A%E6%A1%88%E4%BE%8B)

## [4.2、Pegasus操作系统原理](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/4.2%20Pegasus%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F%E5%8E%9F%E7%90%86.md#42pegasus%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F%E5%8E%9F%E7%90%86)

## [4.3、基础外设开发](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/4.3%20%E5%9F%BA%E7%A1%80%E5%A4%96%E8%AE%BE%E5%BC%80%E5%8F%91.md#43%E5%9F%BA%E7%A1%80%E5%A4%96%E8%AE%BE%E5%BC%80%E5%8F%91)

## [4.4、网络通信](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/4.4%20%E7%BD%91%E7%BB%9C%E9%80%9A%E4%BF%A1.md#44%E7%BD%91%E7%BB%9C%E9%80%9A%E4%BF%A1)

## [4.5、物联网应用开发](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/4.5%20%E7%89%A9%E8%81%94%E7%BD%91%E5%BA%94%E7%94%A8%E5%BC%80%E5%8F%91.md#45%E7%89%A9%E8%81%94%E7%BD%91%E5%BA%94%E7%94%A8%E5%BC%80%E5%8F%91)

## [4.6、综合应用案例](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/4.6%20%E7%BB%BC%E5%90%88%E5%BA%94%E7%94%A8%E6%A1%88%E4%BE%8B.md#46%E7%BB%BC%E5%90%88%E5%BA%94%E7%94%A8%E6%A1%88%E4%BE%8B)



# 第五章：Taurus套件的课程及案例

##  [5.1、Taurus芯片平台介绍](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/5.1%20Taurus%E8%8A%AF%E7%89%87%E5%B9%B3%E5%8F%B0%E4%BB%8B%E7%BB%8D.md#51-taurus%E8%8A%AF%E7%89%87%E5%B9%B3%E5%8F%B0%E4%BB%8B%E7%BB%8D)

##  [5.2、Camera视频预处理和后处理实验](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/5.2%20Camera%E8%A7%86%E9%A2%91%E9%A2%84%E5%A4%84%E7%90%86%E5%92%8C%E5%90%8E%E5%A4%84%E7%90%86%E5%AE%9E%E9%AA%8C.md#52camera%E8%A7%86%E9%A2%91%E9%A2%84%E5%A4%84%E7%90%86%E5%92%8C%E5%90%8E%E5%A4%84%E7%90%86%E5%AE%9E%E9%AA%8C)

##  [5.3、AI视觉实验](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/5.3%20AI%E8%A7%86%E8%A7%89%E5%AE%9E%E9%AA%8C.md#53ai%E8%A7%86%E8%A7%89%E5%AE%9E%E9%AA%8C)

##  [5.4、综合实验](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/5.4%20%E7%BB%BC%E5%90%88%E5%AE%9E%E9%AA%8C.md#541ai-sample%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA)

##  [5.5、附录](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/5.5%20%E9%99%84%E5%BD%95.md#55%E9%99%84%E5%BD%95)



# 第六章：Taurus & Pegasus套件互联

##  [6.1、串口互联方式](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/6.1%20%E4%B8%B2%E5%8F%A3%E4%BA%92%E8%81%94%E6%96%B9%E5%BC%8F.md#61%E4%B8%B2%E5%8F%A3%E4%BA%92%E8%81%94%E6%96%B9%E5%BC%8F)

##  [6.2、WiFi互联方式](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/6.2%20WiFi%E4%BA%92%E8%81%94%E6%96%B9%E5%BC%8F.md#62wifi%E4%BA%92%E8%81%94%E6%96%B9%E5%BC%8F)



# [第七章：FAQ](https://gitee.com/HiSpark/HiSpark_NICU2023/blob/master/7%20FAQ.md#%E7%AC%AC%E4%B8%83%E7%AB%A0faq)