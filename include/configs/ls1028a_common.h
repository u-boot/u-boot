/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2020 NXP
 */

#ifndef __L1028A_COMMON_H
#define __L1028A_COMMON_H

#define CONFIG_REMAKE_ELF
#define CONFIG_FSL_LAYERSCAPE
#define CONFIG_MP

#include <asm/arch/stream_id_lsch3.h>
#include <asm/arch/config.h>
#include <asm/arch/soc.h>

/* Link Definitions */
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE

#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000UL
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_DDR_BLOCK2_BASE	0x2080000000ULL
#define CONFIG_SYS_FSL_DDR_MAIN_NUM_CTRLS	1

#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START        0x80000000
#define CONFIG_SYS_MEMTEST_END          0x9fffffff

/*
 * SMP Definitinos
 */
#define CPU_RELEASE_ADDR		secondary_boot_func

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		25000000	/* 25MHz */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2048 * 1024)

/* I2C */
#ifndef CONFIG_DM_I2C
#define CONFIG_SYS_I2C
#endif

/* Serial Port */
#define CONFIG_CONS_INDEX       1
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE     1
#define CONFIG_SYS_NS16550_CLK          (get_bus_freq(0) / 2)

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_DDR_SDRAM_BASE + 0x10000000)

/* Physical Memory Map */
#define CONFIG_CHIP_SELECTS_PER_CTRL	4

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		128

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

#undef CONFIG_BOOTCOMMAND

#define XSPI_NOR_BOOTCOMMAND	\
	"run xspi_hdploadcmd; run distro_bootcmd; run xspi_bootcmd; " \
	"env exists secureboot && esbc_halt;;"
#define SD_BOOTCOMMAND	\
	"run sd_hdploadcmd; run distro_bootcmd;run sd_bootcmd; " \
	"env exists secureboot && esbc_halt;"
#define SD2_BOOTCOMMAND	\
	"run emmc_hdploadcmd; run distro_bootcmd;run emmc_bootcmd; " \
	"env exists secureboot && esbc_halt;"

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Boot args buffer */

#ifndef CONFIG_CMDLINE_EDITING
#define CONFIG_CMDLINE_EDITING		1
#endif

#define CONFIG_SYS_MAXARGS		64	/* max command args */

#define CONFIG_SYS_BOOTM_LEN   (64 << 20)      /* Increase max gunzip size */

/*  MMC  */
#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
#endif

#define CONFIG_SYS_MMC_ENV_DEV         0
#define OCRAM_NONSECURE_SIZE		0x00010000
#define CONFIG_SYS_FSL_QSPI_BASE	0x20000000

#define CONFIG_SYS_MONITOR_BASE CONFIG_SYS_TEXT_BASE

/* I2C bus multiplexer */
#define I2C_MUX_PCA_ADDR_PRI            0x77 /* Primary Mux*/
#define I2C_MUX_CH_DEFAULT              0x8

/* EEPROM */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM		0
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5

/* DisplayPort */
#define DP_PWD_EN_DEFAULT_MASK          0x8

#ifdef CONFIG_NXP_ESBC
#include <asm/fsl_secure_boot.h>
#endif

/* Ethernet */
/* smallest ENETC BD ring has 8 entries */
#define CONFIG_SYS_RX_ETH_BUFFER		8

#endif /* __L1028A_COMMON_H */
