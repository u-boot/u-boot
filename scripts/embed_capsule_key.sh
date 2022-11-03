#! /bin/bash
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2023, Linaro Limited
#

gen_capsule_signature_file() {
cat >> $1 << EOF
/dts-v1/;
/plugin/;

&{/} {
	signature {
		capsule-key = /incbin/(CONFIG_EFI_CAPSULE_ESL_FILE);
	};
};
EOF
}

gen_capsule_signature_file signature.$$.dts > /dev/null 2>&1
$CPP $dtc_cpp_flags -x assembler-with-cpp -o signature.$$.tmp signature.$$.dts > /dev/null 2>&1
dtc -@ -O dtb -o signature.$$.dtbo signature.$$.tmp > /dev/null 2>&1
fdtoverlay -i $1 -o temp.$$.dtb -v signature.$$.dtbo > /dev/null 2>&1
mv temp.$$.dtb $1 > /dev/null 2>&1
rm -f signature.$$.* > /dev/null 2>&1
