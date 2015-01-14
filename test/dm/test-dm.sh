#!/bin/sh

NUM_CPUS=$(cat /proc/cpuinfo |grep -c processor)
dtc -I dts -O dtb test/dm/test.dts -o test/dm/test.dtb
make O=sandbox sandbox_config
make O=sandbox -s -j${NUM_CPUS}
dd if=/dev/zero of=spi.bin bs=1M count=2
./sandbox/u-boot -d test/dm/test.dtb -c "dm test"
rm spi.bin
