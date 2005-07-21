#! /bin/sh

# LINUX VERSION
# Set the place where SciTech Software is installed, and where each
# of the supported compilers is installed. These environment variables
# are used by the batch files in the SCITECH\BIN directory.
#
# Modify the as appropriate for your compiler configuration (you should
# only need to change things in this batch file).
#
# This version is for a normal Linux installation.

# The SCITECH variable points to where batch files, makefile startups,
# include files and source files will be found when compiling.

export SCITECH=$MGL_ROOT 

# The SCITECH_LIB variable points to where the SciTech libraries live
# for installation and linking. This allows you to have the source and
# include files on local machines for compiling and have the libraries
# located on a common network machine (for network builds).

export SCITECH_LIB=$SCITECH

# The PRIVATE variable points to where private source files reside that
# do not live in the public source tree

export PRIVATE=$HOME/private

# The following define the locations of all the compilers that you may
# be using. Change them to reflect where you have installed your
# compilers.

export GCC_PATH=/usr/bin
export TEMP=/tmp TMP=/tmp

# Add the Scitech bin path to the current PATH
export PATH=$SCITECH/bin:$SCITECH/bin-linux:$PATH
if [ "x$LIBC" = x ]; then
	export PATH=$SCITECH/bin-linux/glibc:$PATH
else
	export PATH=$SCITECH/bin-linux/libc:$PATH
fi
