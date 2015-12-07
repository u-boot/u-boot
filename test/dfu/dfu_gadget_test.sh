#! /bin/bash

# Copyright (C) 2014 Samsung Electronics
# Lukasz Majewski <l.majewski@samsung.com>
#
# Script fixes, enhancements and testing:
# Stephen Warren <swarren@nvidia.com>
#
# DFU operation test script
#
# SPDX-License-Identifier:	GPL-2.0+

set -e # any command return if not equal to zero
clear

COLOUR_RED="\33[31m"
COLOUR_GREEN="\33[32m"
COLOUR_DEFAULT="\33[0m"

DIR=./
SUFFIX=img
RCV_DIR=rcv/
LOG_FILE=./log/log-`date +%d-%m-%Y_%H-%M-%S`

cd `dirname $0`
./dfu_gadget_test_init.sh

cleanup () {
    rm -rf $DIR$RCV_DIR
}

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

dfu_test_file () {
    printf "$COLOUR_GREEN ========================================================================================= $COLOUR_DEFAULT\n"
    printf "File:$COLOUR_GREEN %s $COLOUR_DEFAULT\n" $1

    dfu-util $USB_DEV -D $1 -a $TARGET_ALT_SETTING >> $LOG_FILE 2>&1 || die $?

    echo -n "TX: "
    calculate_md5sum $1

    MD5_TX=$MD5SUM

    dfu-util $USB_DEV -D ${DIR}/dfudummy.bin -a $TARGET_ALT_SETTING_B >> $LOG_FILE 2>&1 || die $?

    N_FILE=$DIR$RCV_DIR${1:2}"_rcv"

    dfu-util $USB_DEV -U $N_FILE -a $TARGET_ALT_SETTING >> $LOG_FILE 2>&1 || die $?

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
echo "DFU EP0 transmission test program"
echo "Trouble shoot -> disable DBG (even the KERN_DEBUG) in the UDC driver"
echo "@ -> TRATS2 # dfu 0 mmc 0"
cleanup
mkdir -p $DIR$RCV_DIR
touch $LOG_FILE

if [ $# -eq 0 ]
then
	printf "   $COLOUR_RED Please pass alt setting number!!  $COLOUR_DEFAULT \n"
	exit 0
fi

TARGET_ALT_SETTING=$1
TARGET_ALT_SETTING_B=$2

file=$3
[[ $3 == *':'* ]] && USB_DEV="-d $3" && file=""
[ $# -eq 4 ] && USB_DEV="-d $4"

if [ -n "$file" ]
then
	dfu_test_file $file
else
	for f in $DIR*.$SUFFIX
	do
	    dfu_test_file $f
	done
fi

cleanup

exit 0
