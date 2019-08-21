#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# script to generate FIT image source for RISC-V boards with OpenSBI
# and, optionally, multiple device trees (given on the command line).
#
# usage: $0 [<dt_name> [<dt_name] ...]

[ -z "$OPENSBI" ] && OPENSBI="fw_dynamic.bin"

if [ -z "$UBOOT_LOAD_ADDR" ]; then
	UBOOT_LOAD_ADDR="$(grep "^CONFIG_SYS_TEXT_BASE=" .config | awk 'BEGIN{FS="="} {print $2}')"
fi

if [ -z "$OPENSBI_LOAD_ADDR" ]; then
	OPENSBI_LOAD_ADDR="$(grep "^CONFIG_SPL_OPENSBI_LOAD_ADDR=" .config | awk 'BEGIN{FS="="} {print $2}')"
fi

if [ ! -f $OPENSBI ]; then
	echo "WARNING: OpenSBI binary \"$OPENSBI\" not found, resulting binary is not functional." >&2
	OPENSBI=/dev/null
fi

cat << __HEADER_EOF
/dts-v1/;

/ {
	description = "Configuration to load OpenSBI before U-Boot";

	images {
		uboot {
			description = "U-Boot";
			data = /incbin/("u-boot-nodtb.bin");
			type = "standalone";
			os = "U-Boot";
			arch = "riscv";
			compression = "none";
			load = <$UBOOT_LOAD_ADDR>;
		};
		opensbi {
			description = "RISC-V OpenSBI";
			data = /incbin/("$OPENSBI");
			type = "firmware";
			os = "opensbi";
			arch = "riscv";
			compression = "none";
			load = <$OPENSBI_LOAD_ADDR>;
			entry = <$OPENSBI_LOAD_ADDR>;
		};
__HEADER_EOF

cnt=1
for dtname in $*
do
	cat << __FDT_IMAGE_EOF
		fdt_$cnt {
			description = "$(basename $dtname .dtb)";
			data = /incbin/("$dtname");
			type = "flat_dt";
			compression = "none";
		};
__FDT_IMAGE_EOF
cnt=$((cnt+1))
done

cat << __CONF_HEADER_EOF
	};
	configurations {
		default = "config_1";

__CONF_HEADER_EOF

if [ $# -eq 0 ]; then
cat << __CONF_SECTION_EOF
		config_1 {
			description = "U-Boot FIT";
			firmware = "opensbi";
			loadables = "uboot";
		};
__CONF_SECTION_EOF
else
cnt=1
for dtname in $*
do
cat << __CONF_SECTION_EOF
		config_$cnt {
			description = "$(basename $dtname .dtb)";
			firmware = "opensbi";
			loadables = "uboot";
			fdt = "fdt_$cnt";
		};
__CONF_SECTION_EOF
cnt=$((cnt+1))
done
fi

cat << __ITS_EOF
	};
};
__ITS_EOF
