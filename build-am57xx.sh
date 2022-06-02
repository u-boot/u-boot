#!/bin/bash

sys=$(uname -m)

#add: bc bison flex libssl-dev u-boot-tools binutils gcc

DIR="$PWD"

config="am57xx_evm_defconfig"

if [ ! -f ./load.menuconfig ] ; then
	echo "Developers: too make changes: [touch load.menuconfig]"
fi

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- distclean

if [ "x${sys}" = "xarmv7l" ] ; then
	if [ -f ./load.menuconfig ] ; then
		make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- ${config}
		make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
		make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- savedefconfig
		cp -v defconfig ./configs/${config}
		make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- distclean
	fi

	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- ${config}
	make ARCH=arm -j2 CROSS_COMPILE=arm-linux-gnueabihf-
else
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- ${config}

	if [ -f ./load.menuconfig ] ; then
		make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
	fi

	make ARCH=arm -j2 CROSS_COMPILE=arm-linux-gnueabihf-

	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- savedefconfig
	cp -v defconfig ./configs/${config}
fi
