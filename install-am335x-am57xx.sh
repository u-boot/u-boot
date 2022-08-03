#!/bin/bash

if ! id | grep -q root; then
	echo "must be run as root"
	exit
fi

sys=$(uname -m)

wdir="$PWD"
disk="/dev/sdb"

if [ "x${sys}" = "xarmv7l" ] ; then
	if [ -f ${wdir}/MLO ] && [ -f ${wdir}/u-boot-dtb.img ] ; then
		if [ -b /dev/mmcblk0 ] ; then
			echo "dd if=${wdir}/MLO of=/dev/mmcblk0 count=2 seek=1 bs=128k"
			dd if=${wdir}/MLO of=/dev/mmcblk0 count=2 seek=1 bs=128k
			echo "dd if=${wdir}/u-boot-dtb.img of=/dev/mmcblk0 count=4 seek=1 bs=384k"
			dd if=${wdir}/u-boot-dtb.img of=/dev/mmcblk0 count=4 seek=1 bs=384k
		fi

		if [ -b /dev/mmcblk1 ] ; then
			echo "dd if=${wdir}/MLO of=/dev/mmcblk1 count=2 seek=1 bs=128k"
			dd if=${wdir}/MLO of=/dev/mmcblk1 count=2 seek=1 bs=128k
			echo "dd if=${wdir}/u-boot-dtb.img of=/dev/mmcblk1 count=4 seek=1 bs=384k"
			dd if=${wdir}/u-boot-dtb.img of=/dev/mmcblk1 count=4 seek=1 bs=384k
		fi
	else
		echo "Run build-am335x.sh or build-am57xx.sh first"
	fi
else
	echo "Using [${disk}]"
	if [ -f ${wdir}/MLO ] && [ -f ${wdir}/u-boot-dtb.img ] ; then
		if [ -b ${disk} ] ; then
			echo "dd if=${wdir}/MLO of=${disk} count=2 seek=1 bs=128k"
			dd if=${wdir}/MLO of=${disk} count=2 seek=1 bs=128k
			echo "dd if=${wdir}/u-boot-dtb.img of=${disk} count=4 seek=1 bs=384k"
			dd if=${wdir}/u-boot-dtb.img of=${disk} count=4 seek=1 bs=384k
		fi
	else
		echo "Run build-am335x.sh or build-am57xx.sh first"
	fi
fi
