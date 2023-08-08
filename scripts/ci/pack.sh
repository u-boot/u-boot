#!/bin/bash
# SPDX-License-Identifier:           GPL-2.0
# https://spdx.org/licenses
# Copyright (C) 2018 Marvell International Ltd.
#
###############################################################################
## This is the pack script for u-boot                                        ##
## This script is called by CI automated builds                              ##
###############################################################################
## WARNING: Do NOT MODIFY the CI wrapper code segments.                      ##
## You can only modify the config and compile commands                       ##
###############################################################################
## Prerequisites:       DESTDIR is the path to the destination directory
## Usage:               pack BUILD_NAME

## =v=v=v=v=v=v=v=v=v=v=v CI WRAPPER - Do not Modify! v=v=v=v=v=v=v=v=v=v=v= ##
set -exuo pipefail
shopt -s extglob

build_name=$1
echo "running pack.sh ${build_name}"
## =^=^=^=^=^=^=^=^=^=^=^=^  End of CI WRAPPER code -=^=^=^=^=^=^=^=^=^=^=^= ##

mkdir -p $DESTDIR/dev_images
cp u-boot.bin $DESTDIR/dev_images/
