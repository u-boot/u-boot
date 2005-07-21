#! /bin/sh

# Setup for compiling with GCC/G++ for FreeBSD

if [ "$CHECKED" = "1" ]; then
    echo Checked debug build enabled.
else
    echo Release build enabled.
fi

export MAKESTARTUP=$SCITECH/makedefs/gcc_freebsd.mk
export INCLUDE="-Iinclude -I$SCITECH/include -I$PRIVATE/include"
export USE_X11=1
export USE_FREEBSD=1

echo GCC FreeBSD console compilation environment set up
