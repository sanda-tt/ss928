#!/bin/bash
#PATH_ENV
OS_TYPE=Linux #Linux Linux6.6 Ubuntu20.04 Ubuntu22.04 openEuler openHarmony
BOARD=Eulerpi #Eulerpi_4GB Eulerpi_8GB

OUT_PATH=$(pwd)/out
BIN_PATH=$(pwd)/bin
BOOT_PATH=$(pwd)/uboot
KERNEL_PATH=$(pwd)/kernel
ROOTFS_PATH=$(pwd)/rootfs/tmp/rootfs_glibc_arm64
ROOTFS_MUSL_PATH=$(pwd)/rootfs/tmp/rootfs_musl_arm64
OPT_PATH=$(pwd)/opt
CUR_PATH=$(pwd)

ROOTFS_SRC_FILE=$(pwd)/rootfs/$OS_TYPE/rootfs_glibc_arm64.tgz
ROOTFS_SRC_MUSL_FILE=$(pwd)/rootfs/$OS_TYPE/rootfs_musl_arm64.tar.xz

#IMG
IMG_VERSION=""
IMG_PATH=""
IMG_VERSION_FILE=""

building_firmware_init_image_path()
{
	#update OS_TYPE
	IMG_VERSION="${BOARD}_${OS_TYPE}_IMAGE_$(date +%Y%m%d%H%M%S)"
	IMG_PATH="$OUT_PATH/$IMG_VERSION"
	IMG_VERSION_FILE="$IMG_PATH/version.txt"

	echo "Step: init image path ..."
	echo "mkdir $IMG_PATH ..."

	mkdir -p $IMG_PATH
	touch $IMG_VERSION_FILE
	echo $IMG_VERSION > $IMG_VERSION_FILE
}

#LOGO
LOGO_SRC_FILE=$BOOT_PATH/logo/logo.bin
LOGO_DST_FILE=$IMG_PATH/logo.bin

building_firmware_copy_logo_file()
{
	echo "Step: copy logo file ..."
	cp -rf $LOGO_SRC_FILE $LOGO_DST_FILE
}

#UBOOT
BOOT_4GB_SRC_FILE=$BOOT_PATH/$OS_TYPE/boot_image_4G.bin
BOOT_4GB_DST_FILE=$IMG_PATH/boot_image_4G.bin
BOOT_8GB_SRC_FILE=$BOOT_PATH/$OS_TYPE/boot_image_8G.bin
BOOT_8GB_DST_FILE=$IMG_PATH/boot_image_8G.bin
BOOT_4GB_SRC_FILE_1rank=$BOOT_PATH/$OS_TYPE/boot_image_4G_1rank.bin
BOOT_4GB_DST_FILE_1rank=$IMG_PATH/boot_image_4G_1rank.bin
BOOT_8GB_SRC_FILE_1rank=$BOOT_PATH/$OS_TYPE/boot_image_8G_1rank.bin
BOOT_8GB_DST_FILE_1rank=$IMG_PATH/boot_image_8G_1rank.bin

building_firmware_copy_uboot_file()
{
	echo "Step: copy uboot file ..."
	cp $BOOT_4GB_SRC_FILE $BOOT_4GB_DST_FILE
	cp $BOOT_8GB_SRC_FILE $BOOT_8GB_DST_FILE
	cp $BOOT_4GB_SRC_FILE_1rank $BOOT_4GB_DST_FILE_1rank
	cp $BOOT_8GB_SRC_FILE_1rank $BOOT_8GB_DST_FILE_1rank
}

ENV_SIZE=0x40000
UBOOT_ENV_4GB_BIN=$IMG_PATH/uboot_env_4G.bin
UBOOT_ENV_4GB_TXT=$BOOT_PATH/$OS_TYPE/uboot_env_4G.txt
BURN_TABLE_4GB_DST=$IMG_PATH/burn_table_4G.xml
BURN_TABLE_4GB_SRC=$BOOT_PATH/$OS_TYPE/burn_table_4G.xml

UBOOT_ENV_8GB_BIN=$IMG_PATH/uboot_env_8G.bin
UBOOT_ENV_8GB_TXT=$BOOT_PATH/$OS_TYPE/uboot_env_8G.txt
BURN_TABLE_8GB_DST=$IMG_PATH/burn_table_8G.xml
BURN_TABLE_8GB_SRC=$BOOT_PATH/$OS_TYPE/burn_table_8G.xml

UBOOT_ENV_4GB_BIN_1rank=$IMG_PATH/uboot_env_4G_1rank.bin
UBOOT_ENV_4GB_TXT_1rank=$BOOT_PATH/$OS_TYPE/uboot_env_4G_1rank.txt
BURN_TABLE_4GB_DST_1rank=$IMG_PATH/burn_table_4G_1rank.xml
BURN_TABLE_4GB_SRC_1rank=$BOOT_PATH/$OS_TYPE/burn_table_4G_1rank.xml

UBOOT_ENV_8GB_BIN_1rank=$IMG_PATH/uboot_env_8G_1rank.bin
UBOOT_ENV_8GB_TXT_1rank=$BOOT_PATH/$OS_TYPE/uboot_env_8G_1rank.txt
BURN_TABLE_8GB_DST_1rank=$IMG_PATH/burn_table_8G_1rank.xml
BURN_TABLE_8GB_SRC_1rank=$BOOT_PATH/$OS_TYPE/burn_table_8G_1rank.xml

ENV_APPEND_FILE_DST=$IMG_PATH/env_append.txt
ENV_APPEND_FILE_SRC=$BOOT_PATH/$OS_TYPE/env_append.txt

ENV_APPEND_FILE_DST_1rank=$IMG_PATH/env_append_1rank.txt
ENV_APPEND_FILE_SRC_1rank=$BOOT_PATH/$OS_TYPE/env_append_1rank.txt

building_firmware_generate_uboot_env()
{
	echo "Step: generate uboot env ..."
	mkenvimage -s $ENV_SIZE -o $UBOOT_ENV_4GB_BIN $UBOOT_ENV_4GB_TXT
	echo "Generate $UBOOT_ENV_4GB_BIN"

	mkenvimage -s $ENV_SIZE -o $UBOOT_ENV_8GB_BIN $UBOOT_ENV_8GB_TXT
	echo "Generate $UBOOT_ENV_8GB_BIN"

	mkenvimage -s $ENV_SIZE -o $UBOOT_ENV_4GB_BIN_1rank $UBOOT_ENV_4GB_TXT_1rank
	echo "Generate $UBOOT_ENV_4GB_BIN_1rank"

	mkenvimage -s $ENV_SIZE -o $UBOOT_ENV_8GB_BIN_1rank $UBOOT_ENV_8GB_TXT_1rank
	echo "Generate $UBOOT_ENV_8GB_BIN_1rank"

	cp $BURN_TABLE_4GB_SRC $BURN_TABLE_4GB_DST
	cp $BURN_TABLE_8GB_SRC $BURN_TABLE_8GB_DST

	cp $BURN_TABLE_4GB_SRC_1rank $BURN_TABLE_4GB_DST_1rank
	cp $BURN_TABLE_8GB_SRC_1rank $BURN_TABLE_8GB_DST_1rank

	cp $ENV_APPEND_FILE_SRC $ENV_APPEND_FILE_DST
	cp $ENV_APPEND_FILE_SRC_1rank $ENV_APPEND_FILE_DST_1rank
}

#kernel
KERNEL_SRC_FILE=$KERNEL_PATH/$OS_TYPE/uImage_ss928v100
KERNEL_DST_FILE=$IMG_PATH/uImage_ss928v100

building_firmware_copy_kernel_file()
{
	echo "Step: copy kernel file ..."
	cp $KERNEL_SRC_FILE $KERNEL_DST_FILE
}

#rootfs
ROOTFS_SRC_PATH=$(pwd)/rootfs/tmp

building_firmware_clean_rootfs()
{
	echo "Step: clean rootfs ..."
	rm -rf $ROOTFS_SRC_PATH
}

building_firmware_decompression_rootfs()
{
	echo "Step: decompression rootfs ..."
	mkdir -p $ROOTFS_SRC_PATH
	tar -xpvf $ROOTFS_SRC_FILE -C $ROOTFS_SRC_PATH > /dev/null

	if [[ "$OS_TYPE" == "Linux6.6" ]]; then
		tar -xpvf $ROOTFS_SRC_MUSL_FILE -C $ROOTFS_SRC_PATH > /dev/null
	fi

	rm -rf $ROOTFS_PATH/rootfs_base
	mkdir -p $ROOTFS_PATH/nfsroot
}

BOARDTOOLS_SRC_PATH=$(pwd)/boardtools/$OS_TYPE
BOARDTOOLS_DST_PATH=$ROOTFS_PATH/usr/local/bin/
building_firmware_copy_boardtools_file()
{
	echo "Step: copy boardtools file ..."
	cp -rf $BOARDTOOLS_SRC_PATH/* $BOARDTOOLS_DST_PATH
}

OVERLAY_SRC_PATH=$(pwd)/overlay/$OS_TYPE
OVERLAY_DST_PATH=$ROOTFS_PATH/
building_firmware_patch_overlay_file()
{
	echo "Step: patch overlay file ..."
	cp -rf $OVERLAY_SRC_PATH/* $OVERLAY_DST_PATH
}

OPT_SRC_PATH=$(pwd)/opt/$OS_TYPE
OPT_DST_PATH=$ROOTFS_PATH/opt/
building_firmware_patch_opt_file()
{
	echo "Step: patch opt file ..."
	cp -rf $OPT_SRC_PATH/* $OPT_DST_PATH
}

EMMC_ROOTFS_SIZE=320
EXT4_TOOL=$BIN_PATH/mkfs.ext4
MKEXT4=$BIN_PATH/make_ext4fs
POPULATE_EXTFS=$BIN_PATH/populate-extfs.sh
EXT4_IMAGE_BIN=$IMG_PATH/rootfs.ext4
EXT4_IMAGE_MUSL_BIN=$IMG_PATH/rootfs_musl.ext4
COUNT_SIZE=$((EMMC_ROOTFS_SIZE*1024*2 ))

building_firmware_generate_ext4fs_debugfs()
{
	echo "Step: generate ext4fs debugfs ..."

	dd if=/dev/zero of=$EXT4_IMAGE_BIN bs=512 count=$COUNT_SIZE
	$EXT4_TOOL $EXT4_IMAGE_BIN
	fakeroot $POPULATE_EXTFS $ROOTFS_PATH $EXT4_IMAGE_BIN  > /dev/null

	if [[ "$OS_TYPE" == "Linux6.6" ]]; then
		dd if=/dev/zero of=$EXT4_IMAGE_MUSL_BIN bs=512 count=$COUNT_SIZE
		$EXT4_TOOL $EXT4_IMAGE_MUSL_BIN
		$POPULATE_EXTFS $ROOTFS_MUSL_PATH $EXT4_IMAGE_MUSL_BIN  > /dev/null
	fi
}

EMMC_OPT_SIZE=128
COUNT_OPT_SIZE=$((EMMC_OPT_SIZE*1024*2 ))
OPT_IMAGE_BIN=$IMG_PATH/opt_${EMMC_OPT_SIZE}M.ext4
building_firmware_generate_ext4fs_opt()
{
	echo "Step: generate ext4fs opt ..."
	dd if=/dev/zero of=$OPT_IMAGE_BIN  bs=512 count=$COUNT_OPT_SIZE
	$EXT4_TOOL $OPT_IMAGE_BIN
	$POPULATE_EXTFS $OPT_PATH/$BOARD $OPT_IMAGE_BIN  > /dev/null
}

building_firmware_generate_emmc_filesystem()
{
	echo "Step: emmc filesystem ..."
	building_firmware_generate_ext4fs_debugfs;
}

building_firmware_copy_ddr_description()
{
	cp -f "$CUR_PATH/README.assets/不同型号DDR说明.png" $IMG_PATH/
}

building_firmware_check_os_support()
{
	local -r supported="Linux Linux6.6 Ubuntu20.04 Ubuntu22.04 openEuler openHarmony"

	case " $supported " in
		*" $OS_TYPE "*) ;;
		*)
			echo "Error: Unsupported system type '$OS_TYPE'."
			echo "Supported types: ${supported[*]}"
			exit 1
			;;
	esac

	#update all param by OS_TYPE
	ROOTFS_PATH="$(pwd)/rootfs/tmp/rootfs_glibc_arm64"
	ROOTFS_SRC_FILE="$(pwd)/rootfs/$OS_TYPE/rootfs_glibc_arm64.tgz"
	ROOTFS_MUSL_PATH="$(pwd)/rootfs/tmp/rootfs_musl_arm64"
	ROOTFS_SRC_MUSL_FILE="$(pwd)/rootfs/$OS_TYPE/rootfs_musl_arm64.tar.xz"

	if [[ "$OS_TYPE" == Ubuntu* ]]; then
		echo "Ubuntu detected: $OS_TYPE"
		ORIG_OS_TYPE="$OS_TYPE"
		OS_TYPE="Ubuntu"
		ROOTFS_PATH="$(pwd)/rootfs/tmp/${ORIG_OS_TYPE}"
		ROOTFS_SRC_FILE="$(pwd)/rootfs/${OS_TYPE}/${ORIG_OS_TYPE}_rootfs.tar.gz"
		EMMC_ROOTFS_SIZE=1024
		COUNT_SIZE=$((EMMC_ROOTFS_SIZE*1024*2 ))

		if [[ ! -f "$ROOTFS_SRC_FILE" ]]; then
			RED='\033[0;31m'
    		NC='\033[0m'

			echo -e "${RED}Rootfs file not found: $ROOTFS_SRC_FILE${NC}"
			echo -e "${RED}Downloading from README provided Baidu link..."
			BAIDU_LINK="https://pan.baidu.com/s/13QQiYs-54qRPZKOZnEpfgg?pwd=x53p"
			echo -e "${RED}Please download manually from: $BAIDU_LINK${NC}"
			exit 1
    	fi
	fi

	building_firmware_init_image_path;

	LOGO_SRC_FILE="$BOOT_PATH/logo/logo.bin"
	LOGO_DST_FILE="$IMG_PATH/logo.bin"

	BOOT_4GB_SRC_FILE="$BOOT_PATH/$OS_TYPE/boot_image_4G.bin"
	BOOT_4GB_DST_FILE="$IMG_PATH/boot_image_4G.bin"
	BOOT_8GB_SRC_FILE="$BOOT_PATH/$OS_TYPE/boot_image_8G.bin"
	BOOT_8GB_DST_FILE="$IMG_PATH/boot_image_8G.bin"

	BOOT_4GB_SRC_FILE_1rank="$BOOT_PATH/$OS_TYPE/boot_image_4G_1rank.bin"
	BOOT_4GB_DST_FILE_1rank="$IMG_PATH/boot_image_4G_1rank.bin"
	BOOT_8GB_SRC_FILE_1rank="$BOOT_PATH/$OS_TYPE/boot_image_8G_1rank.bin"
	BOOT_8GB_DST_FILE_1rank="$IMG_PATH/boot_image_8G_1rank.bin"

	UBOOT_ENV_4GB_BIN="$IMG_PATH/uboot_env_4G.bin"
	UBOOT_ENV_4GB_TXT="$BOOT_PATH/$OS_TYPE/uboot_env_4G.txt"
	BURN_TABLE_4GB_DST="$IMG_PATH/burn_table_4G.xml"
	BURN_TABLE_4GB_SRC="$BOOT_PATH/$OS_TYPE/burn_table_4G.xml"

	UBOOT_ENV_8GB_BIN="$IMG_PATH/uboot_env_8G.bin"
	UBOOT_ENV_8GB_TXT="$BOOT_PATH/$OS_TYPE/uboot_env_8G.txt"
	BURN_TABLE_8GB_DST="$IMG_PATH/burn_table_8G.xml"
	BURN_TABLE_8GB_SRC="$BOOT_PATH/$OS_TYPE/burn_table_8G.xml"

	UBOOT_ENV_4GB_BIN_1rank="$IMG_PATH/uboot_env_4G_1rank.bin"
	UBOOT_ENV_4GB_TXT_1rank="$BOOT_PATH/$OS_TYPE/uboot_env_4G_1rank.txt"
	BURN_TABLE_4GB_DST_1rank="$IMG_PATH/burn_table_4G_1rank.xml"
	BURN_TABLE_4GB_SRC_1rank="$BOOT_PATH/$OS_TYPE/burn_table_4G_1rank.xml"

	UBOOT_ENV_8GB_BIN_1rank="$IMG_PATH/uboot_env_8G_1rank.bin"
	UBOOT_ENV_8GB_TXT_1rank="$BOOT_PATH/$OS_TYPE/uboot_env_8G_1rank.txt"
	BURN_TABLE_8GB_DST_1rank="$IMG_PATH/burn_table_8G_1rank.xml"
	BURN_TABLE_8GB_SRC_1rank="$BOOT_PATH/$OS_TYPE/burn_table_8G_1rank.xml"

	ENV_APPEND_FILE_DST="$IMG_PATH/env_append.txt"
	ENV_APPEND_FILE_SRC="$BOOT_PATH/$OS_TYPE/env_append.txt"

	ENV_APPEND_FILE_DST_1rank="$IMG_PATH/env_append_1rank.txt"
	ENV_APPEND_FILE_SRC_1rank="$BOOT_PATH/$OS_TYPE/env_append_1rank.txt"

	KERNEL_SRC_FILE="$KERNEL_PATH/$OS_TYPE/uImage_ss928v100"
	KERNEL_DST_FILE="$IMG_PATH/uImage_ss928v100"

	BOARDTOOLS_SRC_PATH="$(pwd)/boardtools/$OS_TYPE"
	BOARDTOOLS_DST_PATH="$ROOTFS_PATH/usr/local/bin/"

	OVERLAY_SRC_PATH="$(pwd)/overlay/$OS_TYPE"
	OVERLAY_DST_PATH="$ROOTFS_PATH/"

	OPT_SRC_PATH="$(pwd)/opt/$OS_TYPE"
	OPT_DST_PATH="$ROOTFS_PATH/opt/"

	EXT4_IMAGE_BIN="$IMG_PATH/rootfs.ext4"
	EXT4_IMAGE_MUSL_BIN="$IMG_PATH/rootfs_musl.ext4"

	OPT_IMAGE_BIN="$IMG_PATH/opt_${EMMC_OPT_SIZE}M.ext4"

}

building_firmware_generate_new_image()
{
	building_firmware_check_os_support;
	building_firmware_copy_logo_file;
	building_firmware_copy_uboot_file;
	building_firmware_generate_uboot_env;
	building_firmware_copy_kernel_file;
	building_firmware_clean_rootfs;
	building_firmware_decompression_rootfs;
	building_firmware_copy_boardtools_file;
	building_firmware_patch_overlay_file;
	building_firmware_patch_opt_file;
	building_firmware_generate_emmc_filesystem;
	building_firmware_copy_ddr_description;
}

building_firmware_usage()
{
	echo "Usage:  $0 [-option]"
	echo "options:"
	echo "    -c                       remove all intermediate files"
	echo "    -r                       remove all intermediate file and images"
	echo "    -g                       generate a new images"
	echo "    -os <system>             set system type (Linux/Linux6.6/Ubuntu20.04/Ubuntu22.04/openEuler/openHarmony)"
	echo "    -h                       help information"
}

b_arg_generate=0
b_arg_remove=0
b_arg_clean=0

function building_firmware_parse_arg()
{
	while [[ $# -gt 0 ]]; do
		case "$1" in
			-r)
				b_arg_remove=1
				shift
				;;
			-g)
				b_arg_generate=1
				shift
				;;
			-os)
				OS_TYPE="$2"
				[[ -z "$OS_TYPE" ]] && {
					echo "Error: -os requires a system type"
					exit 1
				}
				shift 2
				;;
			-c)
				b_arg_clean=1
				shift
				;;
			-h)
				building_firmware_usage
				exit 0
				;;
			*)
				echo "Unknown option: $1"
				building_firmware_usage
				exit 1
				;;
		esac
	done
}

if [ $# -lt 1 ]; then
	building_firmware_usage;
	exit 0;
fi

building_firmware_parse_arg $@

#######################Action###############################
if [ $b_arg_remove -eq 1 ]; then
	echo "remove all intermediate files and images"
	building_firmware_clean_rootfs;
	rm -rf $OUT_PATH/*;
fi

if [ $b_arg_clean -eq 1 ]; then
	echo "remove all intermediate files"
	building_firmware_clean_rootfs;
fi

if [ $b_arg_generate -eq 1 ]; then
	echo "generate a new image"
	building_firmware_generate_new_image;
fi
