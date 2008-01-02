#!/bin/sh
# ---------------------------------------------------------
#  Set the core module defines according to Core Module
# ---------------------------------------------------------
# ---------------------------------------------------------
# Set up the Versatile type define
# ---------------------------------------------------------

mkdir -p ${obj}include
variant=PB926EJ-S
if [ "$1" = "" ]
then
	echo "$0:: No parameters - using versatilepb_config"
	echo "#define CONFIG_ARCH_VERSATILE_PB" > ${obj}include/config.h
	variant=PB926EJ-S
else
	case "$1" in
	versatilepb_config	|	\
	versatile_config)
	echo "#define CONFIG_ARCH_VERSATILE_PB" > ${obj}include/config.h
	;;

	versatileab_config)
	echo "#define CONFIG_ARCH_VERSATILE_AB" > ${obj}include/config.h
	variant=AB926EJ-S
	;;


	*)
	echo "$0:: Unrecognised config - using versatilepb_config"
	echo "#define CONFIG_ARCH_VERSATILE_PB" > ${obj}include/config.h
	variant=PB926EJ-S
	;;

	esac

fi
# ---------------------------------------------------------
# Complete the configuration
# ---------------------------------------------------------
$MKCONFIG -a versatile arm arm926ejs versatile NULL versatile
echo "Variant:: $variant"
