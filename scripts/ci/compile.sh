#!/bin/bash
# SPDX-License-Identifier:           GPL-2.0
# https://spdx.org/licenses
# Copyright (C) 2018 Marvell International Ltd.
#
###############################################################################
## This is the compile script for u-boot                                     ##
## This script is called by CI automated builds                              ##
## It may also be used interactively by users to compile the same way as CI  ##
###############################################################################
## WARNING: Do NOT MODIFY the CI wrapper code segments.                      ##
## You can only modify the config and compile commands                       ##
###############################################################################


## =v=v=v=v=v=v=v=v=v=v=v CI WRAPPER - Do not Modify! v=v=v=v=v=v=v=v=v=v=v= ##
set -euo pipefail
shopt -s extglob
##==================================== USAGE ================================##
function usage {
	echo """
Usage: compile [--no_configure] [--echo_only] BUILD_NAME
 or:   compile --list
 or:   compile --help

Compiles u-boot similar to the given CI build

 -N, --no_configure   Skip configuration steps (mrproper, make defconfig)
 -e, --echo_only      Print out the compilation sequence but do not execute it
 -l, --list           List all supported BUILD_NAME values and exit
 -h, --help           Display this help and exit

Prerequisites:       CROSS_COMPILE must point to the cross compiler

"""
	exit 0
}
##============================ PARSE ARGUMENTS ==============================##
TEMP=`getopt -a -o Nelh --long no_configure,echo_only,list,help \
             -n 'compile' -- "$@"`

if [ $? != 0 ] ; then
	echo "Error: Failed parsing command options" >&2
	exit 1
fi
eval set -- "$TEMP"

no_configure=
echo_only=
list=

while true; do
	case "$1" in
		-N | --no_configure ) no_configure=true; shift ;;
		-e | --echo_only )    echo_only=true; shift ;;
		-l | --list ) 	      list=true; shift ;;
		-h | --help )         usage; ;;
		-- ) shift; break ;;
		* ) break ;;
	esac
done

if [[ $list ]] ; then
	DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
	echo "Supported build names:"
	grep -v '^#' "$DIR/supported_builds.txt"
	echo
	echo
	exit 0
fi

[[ $# -ne 1 ]] && usage
build_name=$1

grep ^$build_name$ ./scripts/ci/supported_builds.txt >&/dev/null ||
	( echo "Error: Unsupported build ${build_name}"; exit -1 )
echo "running compile.sh ${build_name}"
## =^=^=^=^=^=^=^=^=^=^=^=^  End of CI WRAPPER code -=^=^=^=^=^=^=^=^=^=^=^= ##


########################### U-BOOT CONFIGURATION ##############################
case $build_name in
	*_a70x0* )       defconfig="mvebu_db_armada8k_defconfig"; ;;
	*_a7020* )       defconfig="mvebu_db_armada8k_defconfig"; ;;
	*_a80x0_mcbin* ) defconfig="mvebu_mcbin-88f8040_defconfig"; ;;
	*_a80x0_ucpe* )  defconfig="mvebu_ucpe-88f8040_defconfig"; ;;
	*_a80x0* )       defconfig="mvebu_db_armada8k_defconfig"; ;;
	*_a388_gp )    defconfig="db-88f6820-gp_defconfig"; ;;
	*_a3900* )       defconfig="mvebu_db_armada8k_defconfig"; ;;
	*_cn9* )        defconfig="mvebu_db_cn91xx_defconfig"; ;;
	*_a37xx_espressobin_* )
	                 defconfig="mvebu_espressobin-88f3720_defconfig"; ;;
	*_a37xx_* )      defconfig="mvebu_db-88f3720_defconfig"; ;;
	* )	echo "Error: Could not configure defconfig." \
		" Unsupported build ${build_name}"; exit -1; ;;
esac

case $build_name in
	uboot_2018_a70x0 )   device_tree="armada-7040-db"; ;;
	uboot_2018_a70x0_B ) device_tree="armada-7040-db-B"; ;;
	uboot_2018_a70x0_C ) device_tree="armada-7040-db-C"; ;;
	uboot_2018_a70x0_D ) device_tree="armada-7040-db-D"; ;;
	uboot_2018_a70x0_E ) device_tree="armada-7040-db-E"; ;;
	uboot_2018_a70x0_pcac ) device_tree="armada-7040-pcac"; ;;
	uboot_2018_a7020_amc ) device_tree="armada-7020-amc"; ;;
	uboot_2018_a7020_comexp ) device_tree="armada-7020-comexp"; ;;
	uboot_2018_a70x0_kr ) device_tree="armada-7040-db"; ;;

	uboot_2018_a80x0 )   device_tree="armada-8040-db"; ;;
	uboot_2018_a80x0_B ) device_tree="armada-8040-db-B"; ;;
	uboot_2018_a80x0_C ) device_tree="armada-8040-db-C"; ;;
	uboot_2018_a80x0_D ) device_tree="armada-8040-db-D"; ;;
	uboot_2018_a80x0_E ) device_tree="armada-8040-db-E"; ;;
	uboot_2018_a80x0_G ) device_tree="armada-8040-db-G"; ;;
	uboot_2018_a80x0_H ) device_tree="armada-8040-db-H"; ;;
	uboot_2018_a80x0_kr ) device_tree="armada-8040-db"; ;;
	uboot_2018_a80x0_pm ) device_tree="armada-8040-db"; ;;
	uboot_2018_a80x0_ddr32 ) device_tree="armada-8040-db"; ;;
	uboot_2018_a80x0_mcbin_single_shot* )
	device_tree="armada-8040-mcbin-single-shot"; ;;
	uboot_2018_a80x0_mcbin* ) device_tree="armada-8040-mcbin"; ;;
	uboot_2018_a80x0_ucpe* ) device_tree="armada-8040-ucpe"; ;;

	uboot_2018_a388_gp ) device_tree="armada-388-gp"; ;;
	uboot_2018_a3900_A ) device_tree="armada-3900-vd-A"; ;;
	uboot_2018_a3900_B ) device_tree="armada-3900-vd-B"; ;;
	uboot_2018_cn9130_A ) device_tree="cn9130-db-A"; ;;
	uboot_2018_cn9130_B ) device_tree="cn9130-db-B"; ;;
	uboot_2018_cn9130_ddr32_A ) device_tree="cn9130-db-A"; ;;
	uboot_2018_cn9130_emmc_A ) device_tree="cn9130-db-A"; ;;
	uboot_2018_cn9130_crb_A ) device_tree="cn9130-crb-A"; ;;
	uboot_2018_cn9130_crb_B ) device_tree="cn9130-crb-B"; ;;
	uboot_2018_cn9131_A ) device_tree="cn9131-db-A"; ;;
	uboot_2018_cn9131_B ) device_tree="cn9131-db-B"; ;;
	uboot_2018_cn9131_ddr32_A ) device_tree="cn9131-db-A"; ;;
	uboot_2018_cn9131_emmc_A ) device_tree="cn9131-db-A"; ;;
	uboot_2018_cn9132_A ) device_tree="cn9132-db-A"; ;;
	uboot_2018_cn9132_B ) device_tree="cn9132-db-B"; ;;
	uboot_2018_cn9132_ddr32_A ) device_tree="cn9132-db-A"; ;;
	uboot_2018_cn9132_emmc_A ) device_tree="cn9132-db-A"; ;;

	uboot_2018*_a37xx_ddr3_v2_A_* )
		device_tree="armada-3720-db"; ;;
	uboot_2018_a37xx_ddr3_v2_B_* )
		device_tree="armada-3720-ddr3-db-v2-B"; ;;
	uboot_2018_a37xx_ddr3_v2_C_* )
		device_tree="armada-3720-ddr3-db-v2-C"; ;;
	uboot_2018_a37xx_ddr4_v1_A_* )
		device_tree="armada-3720-ddr4-db-v1-A"; ;;
	uboot_2018_a37xx_ddr4_v3_A_* )
		device_tree="armada-3720-ddr4-db-v3-A"; ;;
	uboot_2018_a37xx_ddr4_v3_B_* )
		device_tree="armada-3720-ddr4-db-v3-B"; ;;
	uboot_2018_a37xx_ddr4_v3_C_* )
		device_tree="armada-3720-ddr4-db-v3-C"; ;;
	uboot_2018_a37xx_espressobin_ddr3_* )
		device_tree="armada-3720-espressobin"; ;;
	uboot_2018_a37xx_espressobin_ddr4_v7_1G_emmc* )
		device_tree="armada-3720-espressobin-emmc"; ;;
	uboot_2018_a37xx_espressobin_ddr4_v7_2G_emmc* )
		device_tree="armada-3720-espressobin-emmc"; ;;
	uboot_2018_a37xx_espressobin_ddr4_v7_1G* )
		device_tree="armada-3720-espressobin"; ;;
	uboot_2018_a37xx_espressobin_ddr4_v7_2G* )
		device_tree="armada-3720-espressobin"; ;;
	* ) echo "Error: Could not configure device_tree." \
		" Unsupported build ${build_name}"; exit -1;	;;
esac

if [ $device_tree ]; then
	device_tree="DEVICE_TREE=${device_tree}"
fi

config_cmd=""
if [[ $build_name == "uboot_2018_a70x0_D" ||
	$build_name == "uboot_2018_a7020_amc" ||
	$build_name == "uboot_2018_a7020_comexp" ||
	$build_name == "uboot_2018_a80x0_D" ||
	$build_name == "uboot_2018_cn9130_B" ||
	$build_name == "uboot_2018_cn9131_B" ||
	$build_name == "uboot_2018_cn9132_B" ]]; then
	config_cmd="""
	./scripts/config -d CONFIG_MVEBU_SPI_BOOT
	./scripts/config -e CONFIG_MVEBU_NAND_BOOT
	./scripts/config -d CONFIG_ENV_IS_IN_SPI_FLASH
	./scripts/config -e CONFIG_ENV_IS_IN_NAND
	"""
elif [[ $build_name == "uboot_2018_a37xx_espressobin_ddr4_v7_1G_emmc" ||
	$build_name == "uboot_2018_a37xx_espressobin_ddr4_v7_2G_emmc" ||
	$build_name == "uboot_2018_cn9130_emmc_A" ||
	$build_name == "uboot_2018_cn9131_emmc_A" ||
	$build_name == "uboot_2018_cn9132_emmc_A" ]]; then
	config_cmd="""
	./scripts/config -d CONFIG_MVEBU_SPI_BOOT
	./scripts/config -e CONFIG_MVEBU_MMC_BOOT
	./scripts/config -d CONFIG_ENV_IS_IN_SPI_FLASH
	./scripts/config -e CONFIG_ENV_IS_IN_MMC
	./scripts/config --set-val MVEBU_BOOT_PART 1
	"""
	if [[ $build_name == "uboot_2018_a37xx_espressobin_ddr4_v7_1G_emmc" ||
	$build_name == "uboot_2018_a37xx_espressobin_ddr4_v7_2G_emmc" ]]; then
	config_cmd=$config_cmd"""
		./scripts/config --set-val MVEBU_BOOT_DEVICE 1
	"""
	fi
fi

build_flags="" # not used in u-boot-2018
logfile="$$.make.log"
###############################################################################


## =v=v=v=v=v=v=v=v=v=v=v CI WRAPPER - Do not Modify! v=v=v=v=v=v=v=v=v=v=v= ##
cmd="""
set -x
pwd"""
## =^=^=^=^=^=^=^=^=^=^=^=^  End of CI WRAPPER code -=^=^=^=^=^=^=^=^=^=^=^= ##


##################################### CONFIG ##################################
[[ $no_configure ]] || cmd=$cmd"""
make mrproper
make ${defconfig}
${config_cmd}"""

#################################### COMPILE ##################################
cmd=$cmd"""
make ${device_tree} ${build_flags} -j4"""
###############################################################################


## =v=v=v=v=v=v=v=v=v=v=v CI WRAPPER - Do not Modify! v=v=v=v=v=v=v=v=v=v=v= ##
if [[ $echo_only ]]; then
	echo "$cmd"
	exit 0
fi

(eval "$cmd") 2>&1 | tee ${logfile}
cat $logfile
if grep -i "warning:" $logfile; then
	echo "Error: Build has warnings. Aborted"
	exit -1
fi
## =^=^=^=^=^=^=^=^=^=^=^=^  End of CI WRAPPER code -=^=^=^=^=^=^=^=^=^=^=^= ##
