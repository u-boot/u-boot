#! /bin/sh

# Setup for compiling with GCC/G++ for Linux

if [ "$CHECKED" = "1" ]; then
    echo Checked debug build enabled.
else
    echo Release build enabled.
fi

export MAKESTARTUP=$SCITECH/makedefs/gcc_linux.mk
export INCLUDE="include;$SCITECH/include;$PRIVATE/include"
export USE_LINUX=1

if [ "x$LIBC" = x ]; then
	echo "GCC Linux console compilation environment set up (glib)"
else
	echo "GCC Linux console compilation environment set up (libc5)"
fi
