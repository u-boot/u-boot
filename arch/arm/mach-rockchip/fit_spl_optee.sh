#!/bin/sh
# SPDX-License-Identifier:      GPL-2.0+
#
# Copyright (C) 2019 Rockchip Electronic Co.,Ltd
#
# Script to generate FIT image source for 32-bit Rockchip SoCs with
# U-Boot proper, OPTEE, and devicetree.
#
# usage: $0 <dt_name>

[ -z "$TEE" ] && TEE="tee.bin"

if [ ! -f $TEE ]; then
	echo "WARNING: TEE file $TEE NOT found, U-Boot.itb is non-functional" >&2
	echo "Please export path for TEE or copy tee.bin to U-Boot folder" >&2
	TEE=/dev/null
fi

dtname=$1

cat << __HEADER_EOF
/*
 * Copyright (C) 2017-2019 Rockchip Electronic Co.,Ltd
 *
 * Simple U-boot FIT source file containing U-Boot, dtb and optee
 */

/dts-v1/;

/ {
	description = "FIT image with OP-TEE support";
	#address-cells = <1>;

	images {
		uboot {
			description = "U-Boot";
			data = /incbin/("u-boot-nodtb.bin");
			type = "standalone";
			os = "U-Boot";
			arch = "arm";
			compression = "none";
			load = <0x61000000>;
		};
		optee {
			description = "OP-TEE";
			data = /incbin/("$TEE");
			type = "firmware";
			arch = "arm";
			os = "tee";
			compression = "none";
			load = <0x68400000>;
			entry = <0x68400000>;
		};
		fdt {
			description = "$(basename $dtname .dtb)";
			data = /incbin/("$dtname");
			type = "flat_dt";
			compression = "none";
		};
__HEADER_EOF

cat << __CONF_HEADER_EOF
	};

	configurations {
		default = "conf";
		conf {
			description = "$(basename $dtname .dtb)";
			firmware = "optee";
			loadables = "uboot";
			fdt = "fdt";
		};
__CONF_HEADER_EOF

cat << __ITS_EOF
	};
};
__ITS_EOF
