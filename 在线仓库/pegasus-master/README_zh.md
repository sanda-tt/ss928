#  Pegasus介绍

Pegasus为越影视觉解决方案代码仓，目前包含SS928V100/SS927V100解决方案。

本目录为SS928V100/SS927V100的底层处理驱动和基于OpenHarmony 5.1.0 Release补丁包，为“媒体/图形子系统”提供多媒体处理基本功能。主要功能有：音视频采集、音视频编解码、音视频输出、视频前处理、日志系统等。

本次发布仅提供基础功能，可参考docs目录下的文档、sample下的示例进行试用。如果您有任何问题，请在Gitee提交issue，我们会及时处理。



## 目录

```
pegasus/
├── docs                          # 开发文档
├── os
│   └── OpenHarmony               # OpenHarmony适配代码（海思芯片补丁、配置）
│       ├── device                # 设备相关代码
│       │   └── soc/hisilicon/patches # OpenHarmony源码补丁（按子系统分类，对原生代码的定制化修改）
│       ├── kernel                # 内核配置和补丁（linux-6.6）
│       ├── vendor                # 海思产品配置（hispark_aifly_linux、hispark_aiflylite_linux）
│       └── manifest              # 仓库清单文件
│           ├── devboard_hispark_aifly_5.1.0.xml # repo清单文件
│           └── prebuilts_setup.sh             # 预编译环境准备脚本
├── platform
│   ├── ss928v100_clang           # 支持SS928V100和SS927V100的CLANG-MUSL的SDK
│   │   ├── open_source           # 编译SDK依赖的开源软件，提供配置指导和定制补丁
│   │   ├── osdrv                 # 辅助构建SDK依赖的kernel、uboot
│   │   └── smp                   # SDK软件，自研，包括内核驱动源码、sample实例代码，闭源库
│   └── ss928v100_gcc             # 支持SS928V100和SS927V100的GCC-GLIBC的SDK
│       ├── open_source
│       ├── osdrv
│       └── smp
├── tools                         # 常用工具包
└── vendor
    ├── rkh/patches               # 润开鸿OpenHarmony源码补丁（按子系统分类，增强系统功能和驱动支持）
    └── ...                       # 其他厂商（易百纳、野火、迅为等）
```

## 代码下载

下载主仓

```shell
git clone https://gitee.com/HiSpark/pegasus.git
```

pegasus仓下有两个子仓，分别是ss928v100_clang和ss928v100_gcc，可以根据需要下载某一个或者全部下载。

例如：

- 下载ss928v100_clang子仓

  ```shell
  cd pegasus
  git submodule init
  git submodule update platform/ss928v100_clang
  ```

- 下载ss928v100_gcc子仓

  ```shell
  cd pegasus
  git submodule init
  git submodule update platform/ss928v100_gcc
  ```

- 下载全部两个子仓

  ```shell
  cd pegasus
  git submodule init
  git submodule update platform/ss928v100_clang platform/ss928v100_gcc
  ```

  

## 许可说明

- platform\ss928v100_xxx\smp\a55_linux\mpp\out\lib，os\OpenHarmony\middleware里面为上海海思的自研库，遵循上海海思的LICENSE，这个目录下有LICENSE文件，LICENSE文件中可以看到版权信息：

  ```
  End User License Agreement
  
  THIS END USER LICENSE AGREEMENT (“AGREEMENT”) IS A LEGAL AGREEMENT BETWEEN YOU (EITHER A SINGLE INDIVIDUAL, OR SINGLE LEGAL ENTITY) AND HISILICON (SHANGHAI) TECHNOLOGIES CO., LTD. ("HISILICON") FOR THE USE OF THE SOFTWARE ACCOMPANYING THIS AGREEMENT. HISILICON IS ONLY WILLING TO LICENSE THE SOFTWARE TO YOU ON CONDITION THAT YOU ACCEPT ALL OF THE TERMS IN THIS AGREEMENT. BY CLICKING “I AGREE” OR BY INSTALLING OR OTHERWISE USING OR COPYING THE SOFTWARE YOU INDICATE THAT YOU AGREE TO BE BOUND BY ALL OF THE TERMS OF THIS AGREEMENT. IF YOU DO NOT AGREE TO THE TERMS OF THIS AGREEMENT, HISILICON IS UNWILLING TO LICENSE THE SOFTWARE TO YOU AND YOU MAY NOT INSTALL, USE OR COPY THE SOFTWARE, AND YOU SHALL PROMPTLY DESTROY, DELETE, OR RETURN THE SOFTWARE TO YOUR SUPPLIER.
  ……
  ```

- platform\ss928v100_xxx\smp\a55_linux\interdrv，platform\ss928v100_xxx\smp\a55_linux\osal，platform\ss928v100_xxx\smp\a55_linux\mpp\cbb，platform\ss928v100_xxx\smp\a55_linux\mpp\component等目录下为上海海思自研代码，使用基于GPL许可的Hisilicon (Shanghai) 版权声明，在该目录下有License目录，许可信息和版权信息通常可以在文件开头看到：

  ```
  /*
   * Copyright (c) 2025 HiSilicon (Shanghai) Technologies Co., Ltd.
   *
   * This program is free software; you can redistribute it and/or
   * modify it under the terms of the GNU General Public License
   * as published by the Free Software Foundation; either version 2
   * of the License, or (at your option) any later version.
     * ... * /
  ```

- platform\ss928v100_xxx\smp\a55_linux\mpp\sample，os\OpenHarmony\patch目录下为上海海思自研代码，使用基于Apache License Version 2.0许可的Hisilicon (Shanghai) 版权声明，在该目录下有Apache License Version 2.0的LICENSE文件，许可信息和版权信息通常可以在文件开头看到：

  ```
  /*
   * Copyright (c) 2025 HiSilicon (Shanghai) Technologies Co., Ltd.
   *
   * Licensed under the Apache License, Version 2.0 (the "License");
   * you may not use this file except in compliance with the License.
   * You may obtain a copy of the License at
   * .../
  ```



## 软件资料介绍

| 名称                                                         | 介绍                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [开发环境搭建指南](docs/zh-CN/Hi3403V100环境搭建指南/Hi3403V100环境搭建指南.md) | 主要包括Linux系统编译环境搭建、SDK包下载、编译、交叉编译工具安装、OS系统移植。 |
| <a href="docs/zh-CN/OpenHarmony Small版本使用指南/OpenHarmony Small版本使用指南.md">OpenHarmony Small版本使用指南</a> | 本文档基于OpenHarmony 5.1.0 Release版本适配SS928V100/SS927V100芯片，支持OpenHarmony Small型系统运行媒体、图形基本功能，支持XTS认证。 |
| [润开鸿OpenHarmony移植指南](vendor/rkh/README_zh.md)         | 本文档主要提供了基于OpenHarmony桌面系统移植。                |
| <a href="vendor/topeet/docs/SS928V100创建Ubuntu rootfs.pdf">迅为Ubuntu移植指南</a> | 本文档主要提供了基于Ubuntu桌面系统移植。                     |
| [OpenEuler移植指南](./os/OpenEuler/README_zh.md)             | 本文档主要提供了基于OpenEuler桌面系统移植。                  |
| [工具包介绍](./tools/README_zh.md)                           | 本文档主要提供了常用工具包下载、介绍以及匹配文档。           |



## 支持的开发板

| 开发板介绍               | 购买链接                                                     | 开发板图片                                                   |
| ------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 海鸥派Euler Pi（易百纳） | [易百纳海鸥派Euler Pi开发板（淘宝）](https://item.taobao.com/item.htm?abbucket=13&id=755989596567&skuId=5948917988054) | <img src="vendor/ebaina/docs/media/Euler Pi.png" alt="Euler Pi" style="zoom:25%;"/> |
| LubanCat-Hi3403（野火）  | **暂不提供**                                                 | <img src="vendor/LubanCat-Hi3403/doc/media/lbc-hi3403.jpg" alt="lbc-hi3403" style="zoom:25%;" /> |

开发板产品均由相应厂家自行销售，由其对产品质量负责，如侵犯他人知识产权的由其自行承担全部责任及赔偿。海思不提供任何保证及担保，亦不承担责任及赔偿。



## 开发板资料

|        开发板名称        |                         案例开发指南                         |                          硬件原理图                          |
| :----------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| 海鸥派Euler Pi（易百纳） | [海鸥派Euler Pi开发板案例开发指南](vendor/ebaina/README.md)  | [海鸥派Euler Pi开发板硬件原理图](vendor/ebaina/docs/hardware) |
| LubanCat-Hi3403（野火）  | [LubanCat-Hi3403开发板案例开发指南](vendor/LubanCat-Hi3403/README.md) | [LubanCat-Hi3403开发板硬件原理图](vendor/LubanCat-Hi3403/doc/hardware) |



## 参与贡献

- 参考[社区参与贡献指南](https://gitee.com/HiSpark/docs/blob/master/contribute/%E7%A4%BE%E5%8C%BA%E5%8F%82%E4%B8%8E%E8%B4%A1%E7%8C%AE%E6%8C%87%E5%8D%97.md)
