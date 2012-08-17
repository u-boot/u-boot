/*
 * Copyright (C) 2009 Texas Instruments Incorporated
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* Spectrum Digital TMS320DM6467 EVM board */
#define DAVINCI_DM6467EVM
#define CONFIG_SYS_USE_NAND
#define CONFIG_SYS_NAND_SMALLPAGE

#define CONFIG_SKIP_LOWLEVEL_INIT

/* SoC Configuration */
#define CONFIG_ARM926EJS				/* arm926ejs CPU */

/* Clock rates detection */
#ifndef __ASSEMBLY__
extern unsigned int davinci_arm_clk_get(void);
#endif

/* Arm Clock frequency    */
#define CONFIG_SYS_CLK_FREQ	davinci_arm_clk_get()
/* Timer Input clock freq */
#define CONFIG_SYS_HZ_CLOCK		(CONFIG_SYS_CLK_FREQ/2)
#define CONFIG_SYS_TIMERBASE		0x01c21400	/* use timer 0 */
#define CONFIG_SYS_HZ			1000
#define CONFIG_SOC_DM646X

/* EEPROM definitions for EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	6
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	20

/* Memory Info */
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)	/* 1 MiB */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x81000000	/* 16MB RAM test */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_STACKSIZE		(256 << 10)	/* 256 KiB */
#define PHYS_SDRAM_1			0x80000000	/* DDR Start */
#define PHYS_SDRAM_1_SIZE		(256 << 20)	/* DDR size 256MB */

/* Linux interfacing */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_SYS_BARGSIZE		1024		/* Bootarg Size */
#define CONFIG_SYS_LOAD_ADDR		0x80700000	/* kernel address */
#define CONFIG_REVISION_TAG

/* Serial Driver info */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	4
#define CONFIG_SYS_NS16550_COM1		0x01c20000
#define CONFIG_SYS_NS16550_CLK		24000000
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* I2C Configuration */
#define CONFIG_HARD_I2C
#define CONFIG_DRIVER_DAVINCI_I2C
#define CONFIG_SYS_I2C_SPEED		80000
#define CONFIG_SYS_I2C_SLAVE		10

/* Network & Ethernet Configuration */
#define CONFIG_DRIVER_TI_EMAC
#define CONFIG_MII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
#define CONFIG_CMD_NET

/* Flash & Environment */
#define CONFIG_SYS_NO_FLASH
#ifdef CONFIG_SYS_USE_NAND
#define CONFIG_NAND_DAVINCI
#define CONFIG_SYS_NAND_CS		2
#undef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_SIZE			(16 << 10)	/* 16 KiB */
#define CONFIG_SYS_NAND_BASE_LIST	{0x42000000, }
#define CONFIG_SYS_NAND_HW_ECC
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_ENV_OFFSET		0
#else
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			(4 << 10)	/* 4 KiB */
#endif

/* U-Boot general configuration */
#undef CONFIG_USE_IRQ				/* No IRQ/FIQ in U-Boot */
#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTFILE		"uImage"	/* Boot file name */
#define CONFIG_SYS_PROMPT	"DM6467 EVM > "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size  */
#define CONFIG_SYS_PBSIZE		\
			(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC
#define CONFIG_BOOTCOMMAND		"source 0x82080000; dhcp; bootm"
#define CONFIG_BOOTARGS			\
					"mem=120M console=ttyS0,115200n8 " \
					"root=/dev/hda1 rw noinitrd ip=dhcp"

/* U-Boot commands */
#include <config_cmd_default.h>
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_SETGETDCR
#ifdef CONFIG_SYS_USE_NAND
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#define CONFIG_CMD_NAND
#endif

#ifdef CONFIG_CMD_BDI
#define CONFIG_CLOCKS
#endif

#define CONFIG_MAX_RAM_BANK_SIZE	(256 << 20)	/* 256 MB */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

#endif /* __CONFIG_H */
