#!/bin/sh
# ---------------------------------------------------------
# Set the platform defines
# ---------------------------------------------------------
echo -n	"/* Integrator configuration implied "	 > tmp.fil
echo	" by Makefile target */"	 	>> tmp.fil
echo -n	"#define CONFIG_INTEGRATOR"		>> tmp.fil
echo	" /* Integrator board */"		>> tmp.fil
echo -n	"#define CONFIG_ARCH_INTEGRATOR"	>> tmp.fil
echo	" 1 /* Integrator/AP	 */"		>> tmp.fil
# ---------------------------------------------------------
#	Set the core module defines according to Core Module
# ---------------------------------------------------------
cpu="arm_intcm"
variant="unknown core module"

if [ "$1" = "" ]
then
	echo "$0:: No parameters - using arm_intcm"
else
	case "$1" in
	ap7_config)
	cpu="arm_intcm"
	variant="unported core module CM7TDMI"
	;;

	ap966)
	cpu="arm_intcm"
	variant="unported core module CM966E-S"
	;;

	ap922_config)
	cpu="arm_intcm"
	variant="unported core module CM922T"
	;;

	integratorap_config	|	\
	ap_config)
	cpu="arm_intcm"
	variant="unspecified core module"
	;;

	ap720t_config)
	cpu="arm720t"
	echo -n	"#define CONFIG_CM720T"		>> tmp.fil
	echo	" 1 /* CPU core is ARM720T */ "	>> tmp.fil
	variant="Core module CM720T"
	;;

	ap922_XA10_config)
	cpu="arm_intcm"
	variant="unported core module CM922T_XA10"
	echo -n	"#define CONFIG_CM922T_XA10" 		>> tmp.fil
	echo	" 1 /* CPU core is ARM922T_XA10 */" 	>> tmp.fil
	;;

	ap920t_config)
	cpu="arm920t"
	variant="Core module CM920T"
	echo -n	"#define CONFIG_CM920T" 		>> tmp.fil
	echo	" 1 /* CPU core is ARM920T */"		>> tmp.fil
	;;

	ap926ejs_config)
	cpu="arm926ejs"
	variant="Core module CM926EJ-S"
	echo -n	"#define CONFIG_CM926EJ_S"		>> tmp.fil
	echo	" 1 /* CPU core is ARM926EJ-S */ "	>> tmp.fil
	;;

	ap946es_config)
	cpu="arm946es"
	variant="Core module CM946E-S"
	echo -n	"#define CONFIG_CM946E_S"		>> tmp.fil
	echo	" 1 /* CPU core is ARM946E-S */ "	>> tmp.fil
	;;

	*)
	echo "$0:: Unknown core module"
	variant="unknown core module"
	cpu="arm_intcm"
	;;

	esac
fi

if [ "$cpu" = "arm_intcm" ]
then
	echo "/* Core module undefined/not ported */"	>> tmp.fil
	echo "#define CONFIG_ARM_INTCM 1"		>> tmp.fil
	echo -n	"#undef CONFIG_CM_MULTIPLE_SSRAM"	>> tmp.fil
	echo -n	"	/* CM may not have " 		>> tmp.fil
	echo	"multiple SSRAM mapping */"		>> tmp.fil
	echo -n	"#undef CONFIG_CM_SPD_DETECT " 		>> tmp.fil
	echo -n	" /* CM may not support SPD " 		>> tmp.fil
	echo	"query */"				>> tmp.fil
	echo -n	"#undef CONFIG_CM_REMAP	" 		>> tmp.fil
	echo -n	" /* CM may not support "		>> tmp.fil
	echo	"remapping */"	 			>> tmp.fil
	echo -n	"#undef CONFIG_CM_INIT	" 		>> tmp.fil
	echo -n	" /* CM may not have	"		>> tmp.fil
	echo	"initialization reg */"			>> tmp.fil
	echo -n	"#undef CONFIG_CM_TCRAM	" 		>> tmp.fil
	echo	" /* CM may not have TCRAM */" 		>> tmp.fil
fi

mkdir -p ${obj}include
mkdir -p ${obj}board/integratorap
mv tmp.fil ${obj}include/config.h
# ---------------------------------------------------------
#	Ensure correct core object loaded first in U-Boot image
# ---------------------------------------------------------
sed -r 's/CPU_FILE/cpu\/'$cpu'\/start.o/; s/#.*//' ${src}board/integratorap/u-boot.lds.template > ${obj}board/integratorap/u-boot.lds
# ---------------------------------------------------------
# Complete the configuration
# ---------------------------------------------------------
$MKCONFIG -a integratorap arm $cpu integratorap;
echo "Variant:: $variant with core $cpu"

