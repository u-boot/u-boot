#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# script to generate FIT image source for Xilinx ZynqMP boards with
# ARM Trusted Firmware and multiple device trees (given on the command line)
#
# usage: $0 <dt_name> [<dt_name> [<dt_name] ...]

BL33="u-boot-nodtb.bin"
[ -z "$BL31" ] && BL31="bl31.bin"
# Can be also done as ${CROSS_COMPILE}readelf -l bl31.elf | awk '/Entry point/ { print $3 }'
[ -z "$ATF_LOAD_ADDR" ] && ATF_LOAD_ADDR="0xfffea000"

if [ -z "$BL33_LOAD_ADDR" ];then
	BL33_LOAD_ADDR=`awk '/CONFIG_SYS_TEXT_BASE/ { print $3 }' include/generated/autoconf.h`
fi

DTB_LOAD_ADDR=`awk '/CONFIG_XILINX_OF_BOARD_DTB_ADDR/ { print $3 }' include/generated/autoconf.h`
if [ ! -z "$DTB_LOAD_ADDR" ]; then
	DTB_LOAD="load = <$DTB_LOAD_ADDR>;"
else
	DTB_LOAD=""
fi

if [ -z "$*" ]; then
	DT=arch/arm/dts/${DEVICE_TREE}.dtb
else
	DT=$*
fi

if [ ! -f $BL31 ]; then
	echo "WARNING: BL31 file $BL31 NOT found, resulting binary is non-functional" >&2
	BL31=/dev/null
	# But U-Boot proper could be loaded in EL3 by specifying
	# firmware = "uboot";
	# instead of "atf" in config node
fi

cat << __HEADER_EOF
// SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)

/dts-v1/;

/ {
	description = "Configuration to load ATF before U-Boot";

	images {
		uboot {
			description = "U-Boot (64-bit)";
			data = /incbin/("$BL33");
			type = "firmware";
			os = "u-boot";
			arch = "arm64";
			compression = "none";
			load = <$BL33_LOAD_ADDR>;
			entry = <$BL33_LOAD_ADDR>;
			hash {
				algo = "md5";
			};
		};
		atf {
			description = "ARM Trusted Firmware";
			data = /incbin/("$BL31");
			type = "firmware";
			os = "arm-trusted-firmware";
			arch = "arm64";
			compression = "none";
			load = <$ATF_LOAD_ADDR>;
			entry = <$ATF_LOAD_ADDR>;
			hash {
				algo = "md5";
			};
		};
__HEADER_EOF

DEFAULT=1
cnt=1
for dtname in $DT
do
	cat << __FDT_IMAGE_EOF
		fdt_$cnt {
			description = "$(basename $dtname .dtb)";
			data = /incbin/("$dtname");
			type = "flat_dt";
			arch = "arm64";
			compression = "none";
			$DTB_LOAD
			hash {
				algo = "md5";
			};
		};
__FDT_IMAGE_EOF

[ "x$(basename $dtname .dtb)" = "x${DEVICE_TREE}" ] && DEFAULT=$cnt

cnt=$((cnt+1))
done

cat << __CONF_HEADER_EOF
	};
	configurations {
		default = "config_$DEFAULT";

__CONF_HEADER_EOF

cnt=1
for dtname in $DT
do
cat << __CONF_SECTION1_EOF
		config_$cnt {
			description = "$(basename $dtname .dtb)";
			firmware = "atf";
			loadables = "uboot";
			fdt = "fdt_$cnt";
		};
__CONF_SECTION1_EOF
cnt=$((cnt+1))
done

cat << __ITS_EOF
	};
};
__ITS_EOF
