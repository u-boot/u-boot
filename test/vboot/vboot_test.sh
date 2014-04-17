#!/bin/sh
#
# Copyright (c) 2013, Google Inc.
#
# Simple Verified Boot Test Script
#
# SPDX-License-Identifier:	GPL-2.0+

set -e

# Run U-Boot and report the result
# Args:
#	$1:	Test message
run_uboot() {
	echo -n "Test Verified Boot Run: $1: "
	${uboot} -d sandbox-u-boot.dtb >${tmp} -c '
sb load host 0 100 test.fit;
fdt addr 100;
bootm 100;
reset'
	if ! grep -q "$2" ${tmp}; then
		echo
		echo "Verified boot key check failed, output follows:"
		cat ${tmp}
		false
	else
		echo "OK"
	fi
}

echo "Simple Verified Boot Test"
echo "========================="
echo
echo "Please see doc/uImage.FIT/verified-boot.txt for more information"
echo

err=0
tmp=/tmp/vboot_test.$$

dir=$(dirname $0)

if [ -z ${O} ]; then
	O=.
fi
O=$(readlink -f ${O})

dtc="-I dts -O dtb -p 2000"
uboot="${O}/u-boot"
mkimage="${O}/tools/mkimage"
fit_check_sign="${O}/tools/fit_check_sign"
keys="${dir}/dev-keys"
echo ${mkimage} -D "${dtc}"

echo "Build keys"
mkdir -p ${keys}

# Create an RSA key pair
openssl genrsa -F4 -out ${keys}/dev.key 2048 2>/dev/null

# Create a certificate containing the public key
openssl req -batch -new -x509 -key ${keys}/dev.key -out ${keys}/dev.crt

pushd ${dir} >/dev/null

function do_test {
	echo do $sha test
	# Compile our device tree files for kernel and U-Boot
	dtc -p 0x1000 sandbox-kernel.dts -O dtb -o sandbox-kernel.dtb
	dtc -p 0x1000 sandbox-u-boot.dts -O dtb -o sandbox-u-boot.dtb

	# Create a number kernel image with zeroes
	head -c 5000 /dev/zero >test-kernel.bin

	# Build the FIT, but don't sign anything yet
	echo Build FIT with signed images
	${mkimage} -D "${dtc}" -f sign-images-$sha.its test.fit >${tmp}

	run_uboot "unsigned signatures:" "dev-"

	# Sign images with our dev keys
	echo Sign images
	${mkimage} -D "${dtc}" -F -k dev-keys -K sandbox-u-boot.dtb \
		-r test.fit >${tmp}

	run_uboot "signed images" "dev+"


	# Create a fresh .dtb without the public keys
	dtc -p 0x1000 sandbox-u-boot.dts -O dtb -o sandbox-u-boot.dtb

	echo Build FIT with signed configuration
	${mkimage} -D "${dtc}" -f sign-configs-$sha.its test.fit >${tmp}

	run_uboot "unsigned config" $sha"+ OK"

	# Sign images with our dev keys
	echo Sign images
	${mkimage} -D "${dtc}" -F -k dev-keys -K sandbox-u-boot.dtb \
		-r test.fit >${tmp}

	run_uboot "signed config" "dev+"

	echo check signed config on the host
	if ! ${fit_check_sign} -f test.fit -k sandbox-u-boot.dtb >${tmp}; then
		echo
		echo "Verified boot key check on host failed, output follows:"
		cat ${tmp}
		false
	else
		if ! grep -q "dev+" ${tmp}; then
			echo
			echo "Verified boot key check failed, output follows:"
			cat ${tmp}
			false
		else
			echo "OK"
		fi
	fi

	run_uboot "signed config" "dev+"

	# Increment the first byte of the signature, which should cause failure
	sig=$(fdtget -t bx test.fit /configurations/conf@1/signature@1 value)
	newbyte=$(printf %x $((0x${sig:0:2} + 1)))
	sig="${newbyte} ${sig:2}"
	fdtput -t bx test.fit /configurations/conf@1/signature@1 value ${sig}

	run_uboot "signed config with bad hash" "Bad Data Hash"
}

sha=sha1
do_test
sha=sha256
do_test

popd >/dev/null

echo
if ${ok}; then
	echo "Test passed"
else
	echo "Test failed"
fi
