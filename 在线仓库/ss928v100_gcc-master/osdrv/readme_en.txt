1.How to use Makefile of directory osdrv:
Note: a. This readme describes ss928v100.
      b. The Bootloaders are compiled with aarch64-openeuler-linux-gnu 64bit toolchain. 

The specific commands are as follows
(1)Compile the entire osdrv directory:
    make all
    Default Compilation:
    make LLVM=0 BOOT_MEDIA=emmc CHIP=ss928v100 all

parameter explain:
CHIP: Can be ss928v100 or ss927v100, default is ss928v100.
BOOT_MEDIA:include spi and nand. Use 'spi' when you want to use spinor flash/spinand flash. Use 'nand' when you want to use nand flash only.Use 'emmc' when you want to use emmc only.

(2)Clear all compiled files under osdrv directory:
        make clean
(3)Completely remove all compiled files under osdrv directory, and the generated images:
        make distclean

The following compilation command takes ss928v100 as an example.
(4)Separately compile kernel:
    make LLVM=0 BOOT_MEDIA=spi atf -j 20
    (Notes: BOOT_MEDIA can be spi, nand or emmc)

(5)Separately compile uboot for quick boot solution and "Boot Image" for non-sercure boot solution:
        make BOOT_MEDIA=spi gslboot_build -j 20
        (Notes: BOOT_MEDIA can be spi, nand or emmc)

All images are generated in osdrv/pub/, including:
        a. boot_image.bin is the bootloader
        b. u-boot-ss928v100.bin is the u-boot image for "quick_boot" (if OTP is enabled)
        c. uImage_ss928v100 is the uImage of Linux Kernel
2.Note
(1)Executable files in Linux may become non-executable after copying them to somewhere else under Windows, and result in souce code cannot be compiled. Many symbolic link files will be generated in the souce package after compiling the u-boot or the kernel. The volume of the source package will become very big, because all the symbolic link files are turning out to be real files in Windows. So, DO NOT copy source package in Windows.
(2)The SS928V100 has floating-point unit and neon. The libraries in the file system are compiled with har
d floating point and neon compatible with the soft floating point call interface. Therefore, you need to ad
d the "-mcpu=cortex-a55" options to Makefile during compilation of all board codes.
