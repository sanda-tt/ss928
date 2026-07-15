# OpenHarmony通过打patch方式实现产品特性化修改

   -   [简介](#section01)
   -   [patch文件配置](#section02)
   -   [patch制作](#section03)

## 简介<a name="section01"></a>
基于OpenHarmony 5.1.0 Release版本，在Hi3403V100和Hi3519AV200芯片平台上适配OpenHarmony，对OH原生代码的定制化修改由于无法上社区，因此采取打patch形式来处理。

>![](public_sys-resources/icon-note.gif) **说明：**
>对开源鸿蒙组件的定制化修改均已制作成patch，存放于`os/OpenHarmony/device/soc/hisilicon/patches/`目录下。海思集成的补丁文件放置在该路径，润开鸿集成的补丁文件放置在`vendor/rkh/patches`路径下。
>
>初次编译时使用`--patch`参数自动应用patches目录下的补丁文件，详情请参考 [OpenHarmony编译](../../docs/zh-CN/OpenHarmony%20Small版本使用指南/OpenHarmony%20Small版本使用指南.md#openharmony编译) 章节。

## patch文件配置<a name="section02"></a>
OpenHarmony的编译构建子系统提供先打入patch、再开始编译构建的方案，产品编译打入patch，详情可参考`os/OpenHarmony/build/hb/util/prebuild/patch_process.py`，只需配置好产品的patch.yml文件。

添加`os/OpenHarmony/vendor/hisilicon/hispark_aifly_linux/patch.yml`和`os/OpenHarmony/vendor/hisilicon/hispark_aiflylite_linux/patch.yml`，文件中配置需要打入的patch，编译时会在vendor下加载读取该文件。配置格式如下：
```
OpenHarmony源码代码仓相对路径：
  - 对OpenHarmony源码定制化修改patch相对路径

applications/sample/camera/media:
  - device/soc/hisilicon/patches/applications/sample/camera/media/camera_media_001.patch
  - device/soc/hisilicon/patches/applications/sample/camera/media/camera_media_002.patch
  - ../../vendor/rkh/patches/applications/sample/camera/media/camera_media_003.patch
  - ../../vendor/rkh/patches/applications/sample/camera/media/camera_media_004.patch
......
```
>一个OpenHarmony源码代码仓下可以添加多个patch文件，不需要的patch可以注释或者删除掉

## patch制作<a name="section03"></a>
使用git比较OpenHarmony源码仓与Hispark_aifly产品定制化修改源码仓得到差异patch文件。

提供`os/OpenHarmony/device/soc/hisilicon/patches/make_linux_patch.sh`脚本文件

执行下述命令，即可完成对applications/sample/camera源代码仓对比差异，生成camera_media_001.patch文件
```
sh -x ./make_linux_patch.sh /pathto/OpenHarmony-v5.1.0-Release/OpenHarmony/applications/sample/camera/media /pathto/sig/master/test/applications/sample/camera/media  camera_media_001.patch
```
参数说明：
```
参数1：/pathto/OpenHarmony-v5.1.0-Release/OpenHarmony/applications/sample/camera  OH原生代码仓绝对路径
参数2：/pathto/sig/master/applications/sample/camera  产品化特性基于OH原生代码仓定制化修改的绝对路径
参数3：git对比生成差异后的patch文件名称
```

>单个patch文件不宜过大、若涉及改动量大，可以按照目录拆分多个patch文件；建议二进制和源码文件拆分开
