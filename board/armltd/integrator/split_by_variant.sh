#!/bin/sh

mkdir -p ${obj}include
mkdir -p ${obj}board/armltd/integrator

config_file=${obj}include/config.h

if [ "$1" = "ap" ]
then
# ---------------------------------------------------------
# Set the platform defines
# ---------------------------------------------------------
cat > ${config_file} << _EOF
/* Integrator configuration implied by Makefile target */
#define CONFIG_INTEGRATOR /* Integrator board */
#define CONFIG_ARCH_INTEGRATOR 1 /* Integrator/AP */
_EOF

# ---------------------------------------------------------
#	Set the core module defines according to Core Module
# ---------------------------------------------------------
cpu="arm_intcm"
variant="unknown core module"

if [ "$2" = "" ]
then
	echo "$0:: No parameters - using arm_intcm"
else
	case "$2" in
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
	echo "#define CONFIG_CM720T 1 /* CPU core is ARM720T */" \
		>> ${config_file}
	variant="Core module CM720T"
	;;

	ap922_XA10_config)
	cpu="arm_intcm"
	variant="unported core module CM922T_XA10"
	echo "#define CONFIG_CM922T_XA10 1 /* CPU core is ARM922T_XA10 */" \
		>> ${config_file}
	;;

	ap920t_config)
	cpu="arm920t"
	variant="Core module CM920T"
	echo "#define CONFIG_CM920T 1 /* CPU core is ARM920T */" \
		>> ${config_file}
	;;

	ap926ejs_config)
	cpu="arm926ejs"
	variant="Core module CM926EJ-S"
	echo "#define CONFIG_CM926EJ_S 1 /* CPU core is ARM926EJ-S */" \
		>> ${config_file}
	;;

	ap946es_config)
	cpu="arm946es"
	variant="Core module CM946E-S"
	echo "#define CONFIG_CM946E_S 1 /* CPU core is ARM946E-S */" \
		>> ${config_file}
	;;

	*)
	echo "$0:: Unknown core module"
	variant="unknown core module"
	cpu="arm_intcm"
	;;

	esac
fi

case "$cpu" in
	arm_intcm)
	cat >> ${config_file} << _EOF
/* Core module undefined/not ported */
#define CONFIG_ARM_INTCM 1
#undef CONFIG_CM_MULTIPLE_SSRAM /* CM may not have multiple SSRAM mapping */
#undef CONFIG_CM_SPD_DETECT /* CM may not support SPD query */
#undef CONFIG_CM_REMAP /* CM may not support remapping */
#undef CONFIG_CM_INIT  /* CM may not have initialization reg */
#undef CONFIG_CM_TCRAM /* CM may not have TCRAM */
/* May not be processor without cache support */
#define CONFIG_SYS_ICACHE_OFF 1
#define CONFIG_SYS_DCACHE_OFF 1
_EOF
	;;

	arm720t)
	cat >> ${config_file} << _EOF
/* May not be processor without cache support */
#define CONFIG_SYS_ICACHE_OFF 1
#define CONFIG_SYS_DCACHE_OFF 1
_EOF
	;;
esac

else

# ---------------------------------------------------------
# Set the platform defines
# ---------------------------------------------------------
cat >> ${config_file} << _EOF
/* Integrator configuration implied by Makefile target */
#define CONFIG_INTEGRATOR /* Integrator board */
#define CONFIG_ARCH_CINTEGRATOR 1 /* Integrator/CP   */
_EOF

cpu="arm_intcm"
variant="unknown core module"

if [ "$2" = "" ]
then
	echo "$0:: No parameters - using arm_intcm"
else
	case "$2" in
	ap966)
	cpu="arm_intcm"
	variant="unported core module CM966E-S"
	;;

	ap922_config)
	cpu="arm_intcm"
	variant="unported core module CM922T"
	;;

	integratorcp_config	|	\
	cp_config)
	cpu="arm_intcm"
	variant="unspecified core module"
	;;

	cp922_XA10_config)
	cpu="arm_intcm"
	variant="unported core module CM922T_XA10"
	echo "#define CONFIG_CM922T_XA10 1 /* CPU core is ARM922T_XA10 */" \
		>> ${config_file}
	;;

	cp920t_config)
	cpu="arm920t"
	variant="Core module CM920T"
	echo "#define CONFIG_CM920T 1 /* CPU core is ARM920T */" \
		>> ${config_file}
	;;

	cp926ejs_config)
	cpu="arm926ejs"
	variant="Core module CM926EJ-S"
	echo "#define CONFIG_CM926EJ_S 1 /* CPU core is ARM926EJ-S */" \
		>> ${config_file}
	;;


	cp946es_config)
	cpu="arm946es"
	variant="Core module CM946E-S"
	echo "#define CONFIG_CM946E_S 1 /* CPU core is ARM946E-S */" \
		>> ${config_file}
	;;

	cp1136_config)
	cpu="arm1136"
	variant="Core module CM1136EJF-S"
	echo "#define CONFIG_CM1136EJF_S 1 /* CPU core is ARM1136JF-S */" \
		>> ${config_file}
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
	cat >> ${config_file} << _EOF
/* Core module undefined/not ported */
#define CONFIG_ARM_INTCM 1
#undef CONFIG_CM_MULTIPLE_SSRAM /* CM may not have multiple SSRAM mapping */
#undef CONFIG_CM_SPD_DETECT /* CM may not support SPD query */
#undef CONFIG_CM_REMAP /* CM may not support remapping */
#undef CONFIG_CM_INIT /* CM may not have initialization reg */
#undef CONFIG_CM_TCRAM /* CM may not have TCRAM */
_EOF
fi

fi # ap

# ---------------------------------------------------------
# Complete the configuration
# ---------------------------------------------------------
$MKCONFIG -a -n "${2%%_config}" integrator$1 arm $cpu integrator armltd
echo "Variant: $variant with core $cpu"
