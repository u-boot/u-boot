/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 */

#ifndef __LS1012AQDS_H__
#define __LS1012AQDS_H__

#include "ls1012a_common.h"

/* DDR */
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	1
#define CONFIG_SYS_SDRAM_SIZE		0x40000000

/*
 * QIXIS Definitions
 */
#define CONFIG_FSL_QIXIS

#ifdef CONFIG_FSL_QIXIS
#define CONFIG_QIXIS_I2C_ACCESS
#define CONFIG_SYS_I2C_FPGA_ADDR	0x66
#define QIXIS_LBMAP_BRDCFG_REG		0x04
#define QIXIS_LBMAP_SWITCH		6
#define QIXIS_LBMAP_MASK		0x08
#define QIXIS_LBMAP_SHIFT		0
#define QIXIS_LBMAP_DFLTBANK		0x00
#define QIXIS_LBMAP_ALTBANK		0x08
#define QIXIS_RST_CTL_RESET		0x31
#define QIXIS_RCFG_CTL_RECONFIG_IDLE	0x20
#define QIXIS_RCFG_CTL_RECONFIG_START	0x21
#define QIXIS_RCFG_CTL_WATCHDOG_ENBLE	0x08
#endif

/*
 * I2C bus multiplexer
 */
#define I2C_MUX_PCA_ADDR_PRI		0x77
#define I2C_MUX_PCA_ADDR_SEC		0x76 /* Secondary multiplexer */
#define I2C_RETIMER_ADDR		0x18
#define I2C_MUX_CH_DEFAULT		0x8
#define I2C_MUX_CH_CH7301		0xC
#define I2C_MUX_CH5			0xD
#define I2C_MUX_CH7			0xF

#define I2C_MUX_CH_VOL_MONITOR 0xa

/*
* RTC configuration
*/
#define RTC
#define CONFIG_SYS_I2C_RTC_ADDR         0x51  /* Channel 3*/

/* EEPROM */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM    0
#define CONFIG_SYS_I2C_EEPROM_ADDR   0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN     1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 5


/* Voltage monitor on channel 2*/
#define I2C_VOL_MONITOR_ADDR           0x40
#define I2C_VOL_MONITOR_BUS_V_OFFSET   0x2
#define I2C_VOL_MONITOR_BUS_V_OVF      0x1
#define I2C_VOL_MONITOR_BUS_V_SHIFT    3

/* DSPI */
#define CONFIG_FSL_DSPI1

#define MMAP_DSPI          DSPI1_BASE_ADDR

#define CONFIG_SYS_DSPI_CTAR0   1

#define CONFIG_SYS_DSPI_CTAR1	(DSPI_CTAR_TRSZ(7) | DSPI_CTAR_PCSSCK_1CLK |\
				DSPI_CTAR_PASC(0) | DSPI_CTAR_PDT(0) | \
				DSPI_CTAR_CSSCK(2) | DSPI_CTAR_ASC(0) | \
				DSPI_CTAR_DT(0))
#define CONFIG_SPI_FLASH_SST /* cs1 */

#define CONFIG_SYS_DSPI_CTAR2	(DSPI_CTAR_TRSZ(7) | DSPI_CTAR_PCSSCK_1CLK |\
				DSPI_CTAR_PASC(0) | DSPI_CTAR_PDT(0) | \
				DSPI_CTAR_CSSCK(0) | DSPI_CTAR_ASC(0) | \
				DSPI_CTAR_DT(0))
#define CONFIG_SPI_FLASH_STMICRO /* cs2 */

#define CONFIG_SYS_DSPI_CTAR3	(DSPI_CTAR_TRSZ(7) | DSPI_CTAR_PCSSCK_1CLK |\
				DSPI_CTAR_PASC(0) | DSPI_CTAR_PDT(0) | \
				DSPI_CTAR_CSSCK(2) | DSPI_CTAR_ASC(0) | \
				DSPI_CTAR_DT(0))
#define CONFIG_SPI_FLASH_EON /* cs3 */

/*  MMC  */
#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
#endif

#define CONFIG_PCIE1		/* PCIE controller 1 */

#define CONFIG_PCI_SCAN_SHOW

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"verify=no\0"				\
	"fdt_addr=0x00f00000\0"			\
	"kernel_addr=0x01000000\0"		\
	"kernelheader_addr=0x600000\0"		\
	"scriptaddr=0x80000000\0"		\
	"scripthdraddr=0x80080000\0"		\
	"fdtheader_addr_r=0x80100000\0"		\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernel_addr_r=0x96000000\0"		\
	"fdt_addr_r=0x90000000\0"		\
	"load_addr=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"kernelheader_size=0x40000\0"		\
	"console=ttyS0,115200\0"		\
	BOOTENV					\
	"boot_scripts=ls1012aqds_boot.scr\0"	\
	"boot_script_hdr=hdr_ls1012aqds_bs.out\0"	\
	"scan_dev_for_boot_part="		\
	     "part list ${devtype} ${devnum} devplist; "	\
	     "env exists devplist || setenv devplist 1; "	\
	     "for distro_bootpart in ${devplist}; do "		\
		  "if fstype ${devtype} "			\
		      "${devnum}:${distro_bootpart} "		\
		      "bootfstype; then "			\
		      "run scan_dev_for_boot; "	\
		  "fi; "			\
	      "done\0"				\
	"scan_dev_for_boot="				  \
		"echo Scanning ${devtype} "		  \
				"${devnum}:${distro_bootpart}...; "  \
		"for prefix in ${boot_prefixes}; do "	  \
			"run scan_dev_for_scripts; "	  \
		"done;"					  \
		"\0"					  \
	"boot_a_script="				  \
		"load ${devtype} ${devnum}:${distro_bootpart} "  \
			"${scriptaddr} ${prefix}${script}; "    \
		"env exists secureboot && load ${devtype} "     \
			"${devnum}:${distro_bootpart} "		\
			"${scripthdraddr} ${prefix}${boot_script_hdr}; " \
			"env exists secureboot "	\
			"&& esbc_validate ${scripthdraddr};"    \
		"source ${scriptaddr}\0"	  \
	"qspi_bootcmd=pfe stop; echo Trying load from qspi..;"	\
		"sf probe 0:0 && sf read $load_addr "	\
		"$kernel_addr $kernel_size; env exists secureboot "	\
		"&& sf read $kernelheader_addr_r $kernelheader_addr "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0"

#undef CONFIG_BOOTCOMMAND
#ifdef CONFIG_TFABOOT
#undef QSPI_NOR_BOOTCOMMAND
#define QSPI_NOR_BOOTCOMMAND "pfe stop; run distro_bootcmd; run qspi_bootcmd; "\
			     "env exists secureboot && esbc_halt;"
#else
#define CONFIG_BOOTCOMMAND "pfe stop; run distro_bootcmd; run qspi_bootcmd; "\
			   "env exists secureboot && esbc_halt;"
#endif

#include <asm/fsl_secure_boot.h>
#endif /* __LS1012AQDS_H__ */
