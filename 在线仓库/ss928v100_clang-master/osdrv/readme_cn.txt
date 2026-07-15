1.osdrv 顶层 Makefile 使用说明
注意：a. 本 readme 针对 ss928v100 进行说明;
      	b. bootloader 使用 aarch64-openeuler-linux-gnu 64bit工具链进行编译，需要从https://gitee.com/openeuler/yocto-meta-openeuler/releases下载，使用前需要将其添加到环境变量中	

编译具体命令如下：

(1)编译整个osdrv目录：
    make all
    默认编译:
    make LLVM=1 BOOT_MEDIA=emmc CHIP=ss928v100 all

参数说明：
CHIP：可以是ss928v100或ss927v100,默认为ss928v100。
BOOT_MEDIA：spi nor或spi nand启动选择spi；并口nand启动选择nand；emmc启动选择emmc。

(2)清除整个osdrv目录的编译文件：
    make clean

(3)彻底清除整个osdrv目录的编译中间文件：
    make distclean

(4)单独编译kernel image，在 osdrv 目录执行以下命令：
    make LLVM=1 BOOT_MEDIA=spi atf -j 20
    说明：根据实际情况，BOOT_MEDIA 可选 spi、nand 或 emmc

(5)单独编译快速启动的 uboot 或非安全启动的 Boot Image，在 osdrv 目录执行以下命令：
        make BOOT_MEDIA=spi gslboot_build -j 20
        说明：根据实际情况，BOOT_MEDIA 可选 spi、nand 或 emmc

编译生成的镜像位于 osdrv/pub/ 目录，包括：
        a. boot_image.bin 为 bootloader 镜像
        b. u-boot-ss928v100.bin 为 u-boot 镜像（OTP 使能快速启动时使用）
        c. uImage_ss928v100 为 Linux 内核镜像

2.注意事项
(1)在windows下复制源码包时，linux下的可执行文件可能变为非可执行文件，导致无法编译使用；u-boot或内核下编译后，会有很多符号链接文件，

在windows下复制这些源码包, 会使源码包变的巨大，因为linux下的符号链接文件变为windows下实实在在的文件，导致源码包膨胀。
因此使用时请注意不要在windows下复制源代码包。
(2)SS928V100具有浮点运算单元和neon。文件系统中的库是采用软浮点和neon编译而成，因此请用户注意，所有SS928V100板端代码编译时需要在Makefile里面添加选项-mcpu=cortex-a55。

