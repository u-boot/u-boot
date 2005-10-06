#!/bin/sh
# ---------------------------------------------------------
#  Set the core module defines according to Core Module
# ---------------------------------------------------------
CC=${CROSS_COMPILE}gcc
config="versatilepb_config"

if [ "$2" == "" ]
then
	echo "$0:: No preprocessor parameter - using ${CROSS_COMPILE}gcc"
else
	CC=$2
fi


# ---------------------------------------------------------
# Set up the Versatile type define
# ---------------------------------------------------------
if [ "$1" == "" ]
then
	echo "$0:: No parameters - using ${CROSS_COMPILE}gcc versatilepb_config"

else
	case "$config" in
	versatilepb_config	|	\
	versatile_config)
	echo "#define CONFIG_ARCH_VERSATILE_PB" > ./include/config.h
	;;

	versatileab_config)
	echo "#define CONFIG_ARCH_VERSATILE_AB" > ./include/config.h
	;;


	*)
	echo "$0:: Unrecognised config - using versatilepb_config"
	;;

	esac

fi
# ---------------------------------------------------------
# Complete the configuration
# ---------------------------------------------------------
./mkconfig -a versatile arm arm926ejs versatile
