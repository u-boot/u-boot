#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# script to generate FIT image source for K3 Family boards with
# ATF, OPTEE, SPL and multiple device trees (given on the command line).
# Inspired from board/sunxi/mksunxi_fit_atf.sh
#
# usage: $0 <atf_load_addr> <dt_name> [<dt_name> [<dt_name] ...]

[ -z "$ATF" ] && ATF="bl31.bin"

if [ ! -f $ATF ]; then
	echo "WARNING ATF file $ATF NOT found, resulting binary is non-functional" >&2
	ATF=/dev/null
fi

[ -z "$TEE" ] && TEE="bl32.bin"

if [ ! -f $TEE ]; then
	echo "WARNING OPTEE file $TEE NOT found, resulting might be non-functional" >&2
	TEE=/dev/null
fi

[ -z "$DM" ] && DM="dm.bin"

if [ ! -e $DM ]; then
	echo "WARNING DM file $DM NOT found, resulting might be non-functional" >&2
	DM=/dev/null
fi

if [ ! -z "$IS_HS" ]; then
	HS_APPEND=_HS
fi

cat << __HEADER_EOF
/dts-v1/;

/ {
	description = "Configuration to load ATF and SPL";
	#address-cells = <1>;

	images {
		atf {
			description = "ARM Trusted Firmware";
			data = /incbin/("$ATF");
			type = "firmware";
			arch = "arm64";
			compression = "none";
			os = "arm-trusted-firmware";
			load = <$1>;
			entry = <$1>;
		};
		tee {
			description = "OPTEE";
			data = /incbin/("$TEE");
			type = "tee";
			arch = "arm64";
			compression = "none";
			os = "tee";
			load = <0x9e800000>;
			entry = <0x9e800000>;
		};
		dm {
			description = "DM binary";
			data = /incbin/("$DM");
			type = "firmware";
			arch = "arm32";
			compression = "none";
			os = "DM";
			load = <0x89000000>;
			entry = <0x89000000>;
		};
		spl {
			description = "SPL (64-bit)";
			data = /incbin/("spl/u-boot-spl-nodtb.bin$HS_APPEND");
			type = "standalone";
			os = "U-Boot";
			arch = "arm64";
			compression = "none";
			load = <0x80080000>;
			entry = <0x80080000>;
		};
__HEADER_EOF

# shift through ATF load address in the command line arguments
shift

for dtname in $*
do
	cat << __FDT_IMAGE_EOF
		$(basename $dtname) {
			description = "$(basename $dtname .dtb)";
			data = /incbin/("$dtname$HS_APPEND");
			type = "flat_dt";
			arch = "arm";
			compression = "none";
		};
__FDT_IMAGE_EOF
done

cat << __CONF_HEADER_EOF
	};
	configurations {
		default = "$(basename $1)";

__CONF_HEADER_EOF

for dtname in $*
do
	cat << __CONF_SECTION_EOF
		$(basename $dtname) {
			description = "$(basename $dtname .dtb)";
			firmware = "atf";
			loadables = "tee", "dm", "spl";
			fdt = "$(basename $dtname)";
		};
__CONF_SECTION_EOF
done

cat << __ITS_EOF
	};
};
__ITS_EOF
