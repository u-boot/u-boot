#! /bin/sh

# Setup for compiling with Watcom C/C++ for QNX4

if [ "$CHECKED" = "1" ]; then
    echo Checked debug build enabled.
else
    echo Release build enabled.
fi

export MAKESTARTUP=$SCITECH/makedefs/qnx4.mk
export INCLUDE="-I$SCITECH/include -I$PRIVATE/include -I/usr/include"
export USE_QNX=1
export USE_QNX4=1
export WC_LIBBASE=wc10

echo Qnx 4 console compilation environment set up

