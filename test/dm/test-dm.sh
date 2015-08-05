#!/bin/sh

die() {
	echo $1
	exit 1
}

NUM_CPUS=$(cat /proc/cpuinfo |grep -c processor)
make O=sandbox sandbox_config || die "Cannot configure U-Boot"
make O=sandbox -s -j${NUM_CPUS} || die "Cannot build U-Boot"
dd if=/dev/zero of=spi.bin bs=1M count=2
echo -n "this is a test" > testflash.bin
dd if=/dev/zero bs=1M count=4 >>testflash.bin
./sandbox/u-boot -d ./sandbox/arch/sandbox/dts/test.dtb -c "ut dm"
rm spi.bin
rm testflash.bin
