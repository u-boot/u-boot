#!/bin/sh
#
# A wrapper script to adjust Kconfig for U-Boot
#
# This file will be removed after cleaning up defconfig files
#
# Copyright (C) 2014, Masahiro Yamada <yamada.m@jp.panasonic.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

set -e

# Make a configuration target
# Usage:
#   run_make_config <target> <objdir>
# <target>: Make target such as "config", "menuconfig", "defconfig", etc.
run_make_config () {
	# Linux expects defconfig files in arch/$(SRCARCH)/configs/ directory,
	# but U-Boot has them in configs/ directory.
	# Give SRCARCH=.. to fake scripts/kconfig/Makefile.
	$MAKE -f $srctree/scripts/Makefile.build obj=scripts/kconfig SRCARCH=.. $1
}

do_silentoldconfig () {
	run_make_config silentoldconfig

	# If the following part fails, include/config/auto.conf should be
	# deleted so "make silentoldconfig" will be re-run on the next build.
	$MAKE -f $srctree/scripts/Makefile.autoconf || {
		rm -f include/config/auto.conf
		exit 1
	}

	# include/config.h has been updated after "make silentoldconfig".
	# We need to touch include/config/auto.conf so it gets newer
	# than include/config.h.
	# Otherwise, 'make silentoldconfig' would be invoked twice.
	touch include/config/auto.conf
}

cleanup_after_defconfig () {
	rm -f configs/.tmp_defconfig
	# ignore 'Directory not empty' error
	# without using non-POSIX option '--ignore-fail-on-non-empty'
	rmdir arch configs 2>/dev/null || true
}

# Usage:
#  do_board_defconfig <board>_defconfig
do_board_defconfig () {
	defconfig_path=$srctree/configs/$1

	if [ ! -r $defconfig_path ]; then
		echo >&2 "***"
		echo >&2 "*** Can't find default configuration \"configs/$1\"!"
		echo >&2 "***"
		exit 1
	fi

	mkdir -p arch configs
	# prefix "*:" is deprecated.  Drop it simply.
	sed -e 's/^[+A-Z]*://' $defconfig_path > configs/.tmp_defconfig

	run_make_config .tmp_defconfig || {
		cleanup_after_defconfig
		exit 1
	}

	cleanup_after_defconfig
}

do_board_felconfig () {
    do_board_defconfig ${1%%_felconfig}_defconfig
    if ! grep -q CONFIG_ARCH_SUNXI=y .config || ! grep -q CONFIG_SPL=y .config ; then
	echo "$progname: Cannot felconfig a non-sunxi or non-SPL platform" >&2
	exit 1
    fi
    sed -i -e 's/\# CONFIG_SPL_FEL is not set/CONFIG_SPL_FEL=y\nCONFIG_UART0_PORT_F=n/g' \
	.config
}

do_others () {
	run_make_config $1
}

progname=$(basename $0)
target=$1

case $target in
*_defconfig)
	do_board_defconfig $target;;
*_felconfig)
	do_board_felconfig $target;;
*_config)
	# backward compatibility
	do_board_defconfig ${target%_config}_defconfig;;
silentoldconfig)
	do_silentoldconfig;;
*)
	do_others $target;;
esac
