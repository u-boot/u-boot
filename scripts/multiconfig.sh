#!/bin/sh
#
# A wrapper script to adjust Kconfig for U-Boot
#
# Instead of touching various parts under the scripts/kconfig/ directory,
# pushing necessary adjustments into this single script would be better
# for code maintainance.  All the make targets related to the configuration
# (make %config) should be invoked via this script.
# See doc/README.kconfig for further information of Kconfig.
#
# Copyright (C) 2014, Masahiro Yamada <yamada.m@jp.panasonic.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

set -e

# Set "DEBUG" enavironment variable to show debug messages
debug () {
	if [ $DEBUG ]; then
		echo "$@"
	fi
}

# Useful shorthands
build () {
	debug $progname: $MAKE -f $srctree/scripts/Makefile.build obj="$@"
	$MAKE -f $srctree/scripts/Makefile.build obj="$@"
}

autoconf () {
	debug $progname: $MAKE -f $srctree/scripts/Makefile.autoconf obj="$@"
	$MAKE -f $srctree/scripts/Makefile.autoconf obj="$@"
}

# Make a configuration target
# Usage:
#   run_make_config <target> <objdir>
# <target>: Make target such as "config", "menuconfig", "defconfig", etc.
# <objdir>: Target directory where the make command is run.
#           Typically "", "spl", "tpl" for Normal, SPL, TPL, respectively.
run_make_config () {
	target=$1
	objdir=$2

	# Linux expects defconfig files in arch/$(SRCARCH)/configs/ directory,
	# but U-Boot has them in configs/ directory.
	# Give SRCARCH=.. to fake scripts/kconfig/Makefile.
	options="SRCARCH=.. KCONFIG_OBJDIR=$objdir"
	if [ "$objdir" ]; then
		options="$options KCONFIG_CONFIG=$objdir/$KCONFIG_CONFIG"
		mkdir -p $objdir
	fi

	build scripts/kconfig $options $target
}

# Parse .config file to detect if CONFIG_SPL, CONFIG_TPL is enabled
# and returns:
#   ""        if neither CONFIG_SPL nor CONFIG_TPL is defined
#   "spl"     if CONFIG_SPL is defined but CONFIG_TPL is not
#   "spl tpl" if both CONFIG_SPL and CONFIG_TPL are defined
get_enabled_subimages() {
	if [ ! -r "$KCONFIG_CONFIG" ]; then
		# This should never happen
		echo "$progname: $KCONFIG_CONFIG not found" >&2
		exit 1
	fi

	# CONFIG_SPL=y -> spl
	# CONFIG_TPL=y -> tpl
	sed -n -e 's/^CONFIG_SPL=y$/spl/p' -e 's/^CONFIG_TPL=y$/tpl/p' \
							 $KCONFIG_CONFIG
}

do_silentoldconfig () {
	run_make_config silentoldconfig
	subimages=$(get_enabled_subimages)

	for obj in $subimages
	do
		mkdir -p $obj/include/config $obj/include/generated
		run_make_config silentoldconfig $obj
	done

	# If the following part fails, include/config/auto.conf should be
	# deleted so "make silentoldconfig" will be re-run on the next build.
	autoconf include include/autoconf.mk include/autoconf.mk.dep || {
		rm -f include/config/auto.conf
		exit 1
	}

	# include/config.h has been updated after "make silentoldconfig".
	# We need to touch include/config/auto.conf so it gets newer
	# than include/config.h.
	# Otherwise, 'make silentoldconfig' would be invoked twice.
	touch include/config/auto.conf

	for obj in $subimages
	do
		autoconf $obj/include $obj/include/autoconf.mk || {
			rm -f include/config/auto.conf
			exit 1
		}
	done
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
	tmp_defconfig_path=configs/.tmp_defconfig

	if [ ! -r $defconfig_path ]; then
		echo >&2 "***"
		echo >&2 "*** Can't find default configuration \"configs/$1\"!"
		echo >&2 "***"
		exit 1
	fi

	mkdir -p arch configs
	# defconfig for Normal:
	#  pick lines without prefixes and lines starting '+' prefix
	#  and rip the prefixes off.
	sed -n -e '/^[+A-Z]*:/!p' -e 's/^+[A-Z]*://p' $defconfig_path \
						> configs/.tmp_defconfig

	run_make_config .tmp_defconfig || {
		cleanup_after_defconfig
		exit 1
	}

	for img in $(get_enabled_subimages)
	do
		symbol=$(echo $img | cut -c 1 | tr '[a-z]' '[A-Z]')
		# defconfig for SPL, TPL:
		#   pick lines with 'S', 'T' prefix and rip the prefixes off
		sed -n -e 's/^[+A-Z]*'$symbol'[A-Z]*://p' $defconfig_path \
						> configs/.tmp_defconfig
		run_make_config .tmp_defconfig $img || {
			cleanup_after_defconfig
			exit 1
		}
	done

	cleanup_after_defconfig
}

do_defconfig () {
	if [ "$KBUILD_DEFCONFIG" ]; then
		do_board_defconfig $KBUILD_DEFCONFIG
		echo "*** Default configuration is based on '$KBUILD_DEFCONFIG'"
	else
		run_make_config defconfig
	fi
}

do_board_felconfig () {
    do_board_defconfig ${1%%_felconfig}_defconfig
    if ! grep -q CONFIG_ARCH_SUNXI=y .config || ! grep -q CONFIG_SPL=y .config ; then
	echo "$progname: Cannot felconfig a non-sunxi or non-SPL platform" >&2
	exit 1
    fi
    sed -i -e 's/\# CONFIG_SPL_FEL is not set/CONFIG_SPL_FEL=y/g' \
	.config spl/.config
}

do_savedefconfig () {
	if [ -r "$KCONFIG_CONFIG" ]; then
		subimages=$(get_enabled_subimages)
	else
		subimages=
	fi

	run_make_config savedefconfig

	output_lines=

	# -r option is necessay because some string-type configs may include
	# backslashes as an escape character
	while read -r line
	do
		output_lines="$output_lines%$line"
	done < defconfig

	for img in $subimages
	do
		run_make_config savedefconfig $img

		symbol=$(echo $img | cut -c 1 | tr '[a-z]' '[A-Z]')
		unmatched=

		while read -r line
		do
			tmp=
			match=

			# "# CONFIG_FOO is not set" should not be divided.
			# Use "%" as a separator, instead of a whitespace.
			# "%" is unlikely to appear in defconfig context.
			save_IFS=$IFS
			IFS=%
			# coalesce common lines together
			for i in $output_lines
			do
				case "$i" in
				[+A-Z]*:$line)
					tmp="$tmp%$unmatched"
					i=$(echo "$i" | \
					    sed -e "s/^\([^:]*\)/\1$symbol/")
					tmp="$tmp%$i"
					match=1
					;;
				$line)
					tmp="$tmp%$unmatched"
					tmp="$tmp%+$symbol:$i"
					match=1
					;;
				*)
					tmp="$tmp%$i"
					;;
				esac
			done

			# Restore the default separator for the outer for loop.
			IFS=$save_IFS

			if [ "$match" ]; then
				output_lines="$tmp"
				unmatched=
			else
				unmatched="$unmatched%$symbol:$line"
			fi
		done < defconfig

		output_lines="$output_lines%$unmatched"
	done

	rm -f defconfig
	touch defconfig

	save_IFS=$IFS
	IFS=%

	for line in $output_lines
	do
		case "$line" in
		"")
			# do not output blank lines
			;;
		*)
			echo $line >> defconfig
			;;
		esac
	done

	IFS=$save_IFS
}

# Some sanity checks before running "make <objdir>/<target>",
# where <objdir> should be either "spl" or "tpl".
# Doing "make spl/menuconfig" etc. on a non-SPL board makes no sense.
# It should be allowed only when ".config" exists and "CONFIG_SPL" is enabled.
#
# Usage:
#   check_enabled_sumbimage <objdir>/<target> <objdir>
check_enabled_subimage () {

	case $2 in
	spl|tpl) ;;
	*)
		echo >&2 "***"
		echo >&2 "*** \"make $1\" is not supported."
		echo >&2 "***"
		exit 1
		;;
	esac
	test -r "$KCONFIG_CONFIG" && get_enabled_subimages | grep -q $2 || {
		config=CONFIG_$(echo $2 | tr '[a-z]' '[A-Z]')

		echo >&2 "***"
		echo >&2 "*** Create \"$KCONFIG_CONFIG\" with \"$config\" enabled"
		echo >&2 "*** before \"make $1\"."
		echo >&2 "***"
		exit 1
	}
}

# Usage:
#   do_others <objdir>/<target>
# The field "<objdir>/" is typically empy, "spl/", "tpl/" for Normal, SPL, TPL,
# respectively.
# The field "<target>" is a configuration target such as "config",
# "menuconfig", etc.
do_others () {
	target=${1##*/}

	if [ "$target" = "$1" ]; then
		objdir=
	else
		objdir=${1%/*}
		check_enabled_subimage $1 $objdir

		if [ -f "$objdir/$KCONFIG_CONFIG" ]; then
			timestamp_before=$(stat --printf="%Y" \
						$objdir/$KCONFIG_CONFIG)
		fi
	fi

	run_make_config $target $objdir

	if [ "$timestamp_before" -a -f "$objdir/$KCONFIG_CONFIG" ]; then
		timestamp_after=$(stat --printf="%Y" $objdir/$KCONFIG_CONFIG)

		if [ "$timestamp_after" -gt "$timestamp_before" ]; then
			# $objdir/.config has been updated.
			# touch .config to invoke "make silentoldconfig"
			touch $KCONFIG_CONFIG
		fi
	fi
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
defconfig)
	do_defconfig;;
savedefconfig)
	do_savedefconfig;;
*)
	do_others $target;;
esac
