#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
######################################################
# Copyright (C) 2016 Marvell International Ltd.
#
# https://spdx.org/licenses
#
# Author: Konstantin Porotchkin kostap@marvell.com
#
# Version 0.4
#
# UART recovery downloader for Armada SoCs
#
######################################################

port=$1
file=$2

pattern_repeat=1500
default_baudrate=115200
tmpfile=/tmp/xmodem.pattern
tools=( dd stty sx minicom )

if [[ -z "$port" || -z "$file" ]]
then
    echo -e "\nMarvell recovery image downloader for Armada SoC family."
    echo -e "Command syntax:"
    echo -e "\t$(basename $0) <port> <file>"
    echo -e "\tport  - serial port the target board is connected to"
    echo -e "\tfile  - recovery boot image for target download"
    echo -e "For example - load the image over ttyUSB0:"
    echo -e "$(basename $0) /dev/ttyUSB0 /tmp/flash-image.bin\n"
fi

# Sanity checks
if [ -c "$port" ]
then
   echo -e "Using device connected on serial port \"$port\""
else
   echo "Wrong serial port name!"
   exit 1
fi

if [ -f "$file" ]
then
   echo -e "Loading flash image file \"$file\""
else
   echo "File $file does not exist!"
   exit 1
fi

# Verify required tools installation
for tool in ${tools[@]}
do
    toolname=`which $tool`
    if [ -z "$toolname" ]
    then
        echo -e "Missing installation of \"$tool\" --> Exiting"
        exit 1
    fi
done

if [ -f "$tmpfile" ]
then
    rm -f $tmpfile
fi

# Send the escape sequence to target board using default debug port speed
stty -F $port raw ignbrk time 5 $default_baudrate
counter=0
while [ $counter -lt $pattern_repeat ]; do
    echo -n -e "\xBB\x11\x22\x33\x44\x55\x66\x77" >> $tmpfile
    let counter=counter+1
done

echo -en "Press the \"Reset\" button on the target board and "
echo -en "the \"Enter\" key on the host keyboard simultaneously"
read
dd if=$tmpfile of=$port &>/dev/null

# Speed up the binary image transfer
stty -F $port raw ignbrk time 5 $default_baudrate
sx -vv $file > $port < $port
#sx-at91 $port $file

# Optional - fire up Minicom
minicom -D $port -b $default_baudrate

