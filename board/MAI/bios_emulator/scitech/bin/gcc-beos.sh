#! /bin/sh

# Setup for compiling with GCC/G++ for BeOS

if [ "$CHECKED" = "1" ]; then
    echo Checked debug build enabled.
else
    echo Release build enabled.
fi

export MAKESTARTUP=$SCITECH/makedefs/gcc_beos.mk
export INCLUDE="-Iinclude -I$SCITECH/include -I$PRIVATE/include"
export USE_X11=0
export USE_BEOS=1

echo GCC BeOS console compilation environment set up
