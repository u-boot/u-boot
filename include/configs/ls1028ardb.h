/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019, 2021 NXP
 */

#ifndef __LS1028A_RDB_H
#define __LS1028A_RDB_H

#include "ls1028a_common.h"

#define COUNTER_FREQUENCY_REAL		(get_board_sys_clk() / 4)

/* Store environment at top of flash */

/*
 * QIXIS Definitions
 */

#ifdef CONFIG_FSL_QIXIS
#define QIXIS_BASE			0x7fb00000
#define QIXIS_BASE_PHYS			QIXIS_BASE
#define CFG_SYS_I2C_FPGA_ADDR	0x66
#define QIXIS_LBMAP_SWITCH		2
#define QIXIS_LBMAP_MASK		0xe0
#define QIXIS_LBMAP_SHIFT		0x5
#define QIXIS_LBMAP_DFLTBANK		0x00
#define QIXIS_LBMAP_ALTBANK		0x00
#define QIXIS_LBMAP_SD			0x00
#define QIXIS_LBMAP_EMMC		0x00
#define QIXIS_LBMAP_XSPI		0x00
#define QIXIS_RCW_SRC_SD		0xf8
#define QIXIS_RCW_SRC_EMMC		0xf9
#define QIXIS_RCW_SRC_XSPI		0xff
#define QIXIS_RST_CTL_RESET		0x31
#define QIXIS_RCFG_CTL_RECONFIG_IDLE	0x10
#define QIXIS_RCFG_CTL_RECONFIG_START	0x11
#define QIXIS_RCFG_CTL_WATCHDOG_ENBLE	0x08
#define QIXIS_RST_FORCE_MEM		0x01

#define CFG_SYS_FPGA_CSPR_EXT	(0x0)
#define CFG_SYS_FPGA_CSPR		(CSPR_PHYS_ADDR(QIXIS_BASE_PHYS) | \
					CSPR_PORT_SIZE_8 | \
					CSPR_MSEL_GPCM | \
					CSPR_V)
#define CFG_SYS_FPGA_CSOR		(CSOR_NOR_ADM_SHIFT(4) | \
					CSOR_NOR_NOR_MODE_AVD_NOR | \
					CSOR_NOR_TRHZ_80)
#endif

/* Initial environment variables */
#ifndef SPL_NO_ENV
#undef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	"boot_scripts=ls1028ardb_boot.scr\0" \
	"scan_dev_for_boot_part=" \
		"part list ${devtype} ${devnum} devplist; " \
		"env exists devplist || setenv devplist 1; " \
		"for distro_bootpart in ${devplist}; do " \
		  "if fstype ${devtype} " \
			"${devnum}:${distro_bootpart} " \
			"bootfstype; then " \
			"run scan_dev_for_boot; " \
		  "fi; " \
		"done\0" \
	"boot_a_script=" \
		"load ${devtype} ${devnum}:${distro_bootpart} " \
			"${scriptaddr} ${prefix}${script}; " \
		"env exists secureboot && load ${devtype} " \
			"${devnum}:${distro_bootpart} " \
			"${scripthdraddr} ${prefix}${boot_script_hdr} " \
			"&& esbc_validate ${scripthdraddr};" \
		"source ${scriptaddr}\0"
#endif
#endif /* __LS1028A_RDB_H */
