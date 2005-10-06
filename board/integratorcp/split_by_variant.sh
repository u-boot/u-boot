#!/bin/sh
# ---------------------------------------------------------
# Set the platform defines
# ---------------------------------------------------------
echo -n "/* Integrator configuration implied "   > tmp.fil
echo    " by Makefile target */"   		>> tmp.fil
echo -n "#define CONFIG_INTEGRATOR"  		>> tmp.fil
echo	 " /* Integrator board */"  		>> tmp.fil
echo -n "#define CONFIG_ARCH_CINTEGRATOR"	>> tmp.fil
echo     " 1 /* Integrator/CP   */"  		>> tmp.fil
# ---------------------------------------------------------
#  Set the core module defines according to Core Module
# ---------------------------------------------------------
CC=${CROSS_COMPILE}gcc
cpu="arm_intcm"

if [ "$2" == "" ]
then
	echo "$0:: No preprocessor parameter - using ${CROSS_COMPILE}gcc"
else
	CC=$2
fi


if [ "$1" == "" ]
then
	echo "$0:: No parameters - using ${CROSS_COMPILE}gcc arm_intcm"
else
	case "$1" in
	cp966_config		|	\
	cp922_config		|	\
	cp1026_config		|	\
	integratorcp_config	|	\
	cp_config)
	cpu="arm_intcm"
	;;

	cp922_XA10_config)
	echo -n "#define CONFIG_CM922T_XA10" 		>> tmp.fil
	echo    " 1 /* CPU core is ARM922T_XA10 */" 	>> tmp.fil
	cpu="arm_intcm"
	;;

	cp920t_config)
	cpu="arm920t"
	echo -n "#define CONFIG_CM920T" 		>> tmp.fil
	echo    " 1 /* CPU core is ARM920T */"		>> tmp.fil
	;;

	cp926ejs_config)
	cpu="arm926ejs"
	echo -n "#define CONFIG_CM926EJ_S"		>> tmp.fil
	echo    " 1 /* CPU core is ARM926EJ-S */ "	>> tmp.fil
	;;


	cp946es_config)
	cpu="arm946es"
	echo -n "#define CONFIG_CM946E_S"		>> tmp.fil
	echo    " 1 /* CPU core is ARM946E-S */ "	>> tmp.fil
	;;

	cp1136_config)
	cpu="arm1136"
	echo -n "#define CONFIG_CM1136EJF_S"		>> tmp.fil
	echo    " 1 /* CPU core is ARM1136JF-S */ "	>> tmp.fil
	;;

	*)
	echo "$0:: Unrecognised target - using arm_intcm"
	cpu="arm_intcm"
	;;

	esac

fi

if [ "$cpu" == "arm_intcm" ]
then
	echo "/* Core module undefined/not ported */"	>> tmp.fil
	echo "#define CONFIG_ARM_INTCM 1"  		>> tmp.fil
	echo -n "#undef CONFIG_CM_MULTIPLE_SSRAM"	>> tmp.fil
	echo -n "  /* CM may not have " 		>> tmp.fil
	echo    "multiple SSRAM mapping */"  		>> tmp.fil
	echo -n "#undef CONFIG_CM_SPD_DETECT " 		>> tmp.fil
	echo -n " /* CM may not support SPD " 		>> tmp.fil
	echo    "query */"    				>> tmp.fil
	echo -n "#undef CONFIG_CM_REMAP  " 		>> tmp.fil
	echo -n " /* CM may not support "  		>> tmp.fil
	echo    "remapping */"   			>> tmp.fil
	echo -n "#undef CONFIG_CM_INIT  " 		>> tmp.fil
	echo -n " /* CM may not have  "  		>> tmp.fil
	echo    "initialization reg */"  		>> tmp.fil
	echo -n "#undef CONFIG_CM_TCRAM  " 		>> tmp.fil
	echo    " /* CM may not have TCRAM */" 		>> tmp.fil
fi
mv tmp.fil ./include/config.h
# ---------------------------------------------------------
#  Ensure correct core object loaded first in U-Boot image
# ---------------------------------------------------------
$CC -E -P -C -D CPU_FILE=cpu/$cpu/start.o 		\
-o board/integratorcp/u-boot.lds board/integratorcp/u-boot.lds.S
# ---------------------------------------------------------
# Complete the configuration
# ---------------------------------------------------------
./mkconfig -a integratorcp arm $cpu integratorcp;
