#! /bin/bash

# Copyright (C) 2014 Samsung Electronics
# Lukasz Majewski <l.majewski@samsung.com>
#
# UMS operation test script
#
# SPDX-License-Identifier:	GPL-2.0+

clear

COLOUR_RED="\33[31m"
COLOUR_GREEN="\33[32m"
COLOUR_ORANGE="\33[33m"
COLOUR_DEFAULT="\33[0m"

DIR=./
SUFFIX=img
RCV_DIR=rcv/
LOG_FILE=./log/log-`date +%d-%m-%Y_%H-%M-%S`

cd `dirname $0`
../dfu/dfu_gadget_test_init.sh 33M 97M

cleanup () {
    rm -rf $RCV_DIR $MNT_DIR
}

control_c()
# run if user hits control-c
{
	echo -en "\n*** CTRL+C ***\n"
	umount $MNT_DIR
	cleanup
	exit 0
}

# trap keyboard interrupt (control-c)
trap control_c SIGINT

die () {
    printf "   $COLOUR_RED FAILED $COLOUR_DEFAULT \n"
    cleanup
    exit 1
}

calculate_md5sum () {
    MD5SUM=`md5sum $1`
    MD5SUM=`echo $MD5SUM | cut -d ' ' -f1`
    echo "md5sum:"$MD5SUM
}

ums_test_file () {
    printf "$COLOUR_GREEN========================================================================================= $COLOUR_DEFAULT\n"
    printf "File:$COLOUR_GREEN %s $COLOUR_DEFAULT\n" $1

    mount /dev/$MEM_DEV $MNT_DIR
    if [ -f $MNT_DIR/dat_* ]; then
	rm $MNT_DIR/dat_*
    fi

    cp ./$1 $MNT_DIR

    while true; do
	umount $MNT_DIR > /dev/null 2>&1
	if [ $? -eq 0 ]; then
	    break
	fi
	printf "$COLOUR_ORANGE\tSleeping to wait for umount...$COLOUR_DEFAULT\n"
	sleep 1
    done

    echo -n "TX: "
    calculate_md5sum $1

    MD5_TX=$MD5SUM
    sleep 1
    N_FILE=$DIR$RCV_DIR${1:2}"_rcv"

    mount /dev/$MEM_DEV $MNT_DIR
    cp $MNT_DIR/$1 $N_FILE || die $?
    rm $MNT_DIR/$1
    umount $MNT_DIR

    echo -n "RX: "
    calculate_md5sum $N_FILE
    MD5_RX=$MD5SUM

    if [ "$MD5_TX" == "$MD5_RX" ]; then
	printf "   $COLOUR_GREEN -------> OK $COLOUR_DEFAULT \n"
    else
	printf "   $COLOUR_RED -------> FAILED $COLOUR_DEFAULT \n"
	cleanup
	exit 1
    fi
}

printf "$COLOUR_GREEN========================================================================================= $COLOUR_DEFAULT\n"
echo "U-boot UMS test program"

if [ $EUID -ne 0 ]; then
   echo "You must be root to do this." 1>&2
   exit 100
fi

if [ $# -lt 3 ]; then
    echo "Wrong number of arguments"
    echo "Example:"
    echo "sudo ./ums_gadget_test.sh VID PID PART_NUM [-f ext4] [test_file]"
    die
fi

MNT_DIR="/mnt/tmp-ums-test"

VID=$1; shift
PID=$1; shift
PART_NUM=$1; shift

if [ "$1" == "-f" ]; then
    shift
    FS_TO_FORMAT=$1; shift
fi

TEST_FILE=$1

for f in `find /sys -type f -name idProduct`; do
     d=`dirname ${f}`
     if [ `cat ${d}/idVendor` != "${VID}" ]; then
	 continue
     fi
     if [ `cat ${d}/idProduct` != "${PID}" ]; then
	 continue
     fi
     USB_DEV=${d}
     break
done

if [ -z "${USB_DEV}" ]; then
     echo "Connect target"
     echo "e.g. ums 0 mmc 0"
     exit 1
fi

MEM_DEV=`find $USB_DEV -type d -name "sd[a-z]" | awk -F/ '{print $(NF)}' -`

mkdir -p $RCV_DIR
if [ ! -d $MNT_DIR ]; then
    mkdir -p $MNT_DIR
fi

if [ "$PART_NUM" == "-" ]; then
    PART_NUM=""
fi
MEM_DEV=$MEM_DEV$PART_NUM

if [ -n "$FS_TO_FORMAT" ]; then
    echo -n "Formatting partition /dev/$MEM_DEV to $FS_TO_FORMAT"
    mkfs -t $FS_TO_FORMAT /dev/$MEM_DEV > /dev/null 2>&1
    if [ $? -eq 0 ]; then
	printf " $COLOUR_GREEN DONE $COLOUR_DEFAULT \n"
    else
	die
    fi
fi

printf "Mount: /dev/$MEM_DEV \n"

if [ -n "$TEST_FILE" ]; then
    if [ ! -e $TEST_FILE ]; then
	echo "No file: $TEST_FILE"
	die
    fi
    ums_test_file $TEST_FILE
else
    for file in $DIR*.$SUFFIX
    do
	ums_test_file $file
    done
fi

cleanup

exit 0
