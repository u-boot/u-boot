#! /bin/sh
#
# This script generates a single object file from a set of libraries (*.a files)
# Usage: meltobjs.sh target.o library1.a library2.a ...
#
# (C) SciTech Software, Inc. 1998
#

TMPDIR=/tmp/melt$$
TARGET=$1
TARGETDIR=$PWD
shift
mkdir $TMPDIR

cd $TMPDIR

for a in $*
do
    ar x $a
done
ld -r -o $TARGETDIR/$TARGET *.o

rm -fr $TMPDIR