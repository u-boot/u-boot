/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * (C) Copyright 2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

/* for linking errors see http://lists.denx.de/pipermail/u-boot/2009-July/057350.html */

#ifndef _CONFIG_KM_ARM_H
#define _CONFIG_KM_ARM_H

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_MARVELL
#define CONFIG_ARM926EJS		/* Basic Architecture */
#define CONFIG_FEROCEON_88FR131		/* CPU Core subversion */
#define CONFIG_KIRKWOOD			/* SOC Family Name */
#define CONFIG_KW88F6281		/* SOC Name */
#define CONFIG_MACH_SUEN3		/* Machine type */

/* include common defines/options for all Keymile boards */
#include "keymile-common.h"
#undef CONFIG_CMD_DTT
#undef CONFIG_BOOTCOUNT_LIMIT

#define CONFIG_MD5	/* get_random_hex on krikwood needs MD5 support */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */
#define CONFIG_KIRKWOOD_EGIGA_INIT	/* Enable GbePort0/1 for kernel */
#undef  CONFIG_KIRKWOOD_PCIE_INIT	/* Disable PCIE Port0 for kernel */
#define CONFIG_KIRKWOOD_RGMII_PAD_1V8	/* Set RGMII Pad voltage to 1.8V */

#define CONFIG_MISC_INIT_R

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_TCLK
#define CONFIG_SYS_NS16550_COM1		KW_UART0_BASE

/*
 * Serial Port configuration
 * The following definitions let you select what serial you want to use
 * for your console driver.
 */

#define CONFIG_CONS_INDEX	1	/* Console on UART0 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_BOOTMAPSZ	(8 << 20)	/* Initial Memmap for Linux */
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs  */
#define CONFIG_INITRD_TAG		/* enable INITRD tag */
#define CONFIG_SETUP_MEMORY_TAGS 	/* enable memory tag */

/*
 * Commands configuration
 */
#define CONFIG_CMD_ELF
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NFS

/*
 * Without NOR FLASH we need this
 */
#define CONFIG_SYS_NO_FLASH
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS

/*
 * NAND Flash configuration
 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define NAND_MAX_CHIPS			1
#define CONFIG_NAND_KIRKWOOD
#define CONFIG_SYS_NAND_BASE		0xd8000000

#define BOOTFLASH_START		0x0

#define CONFIG_KM_CONSOLE_TTY	"ttyS0"

/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128

/*
 * Other required minimal configurations
 */
#define CONFIG_CONSOLE_INFO_QUIET	/* some code reduction */
#define CONFIG_ARCH_CPU_INIT		/* call arch_cpu_init() */
#define CONFIG_ARCH_MISC_INIT		/* call arch_misc_init() */
#define CONFIG_DISPLAY_CPUINFO		/* Display cpu info */
#define CONFIG_NR_DRAM_BANKS	4
#define CONFIG_STACKSIZE	0x00100000	/* regular stack- 1M */
#define CONFIG_SYS_RESET_ADDRESS 0xffff0000	/* Rst Vector Adr */

/*
 * Ethernet Driver configuration
 */
#define CONFIG_NETCONSOLE	/* include NetConsole support   */
#define CONFIG_NET_MULTI	/* specify more that one ports available */
#define CONFIG_MII		/* expose smi ove miiphy interface */
#define CONFIG_MVGBE		/* Enable Marvell Gbe Controller Driver */
#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN	/* detect link using phy */
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable port 0 only */
#define CONFIG_PHY_BASE_ADR	0
#define CONFIG_ENV_OVERWRITE	/* ethaddr can be reprogrammed */
#define CONFIG_RESET_PHY_R	/* use reset_phy() to init 88E1118 PHY */

/*
 * UBI related stuff
 */
#define CONFIG_SYS_USE_UBI

/*
 * I2C related stuff
 */
#undef	CONFIG_HARD_I2C		/* I2C with hardware support */
#define	CONFIG_SOFT_I2C		/* I2C bit-banged	*/

#if defined(CONFIG_HARD_I2C)
#define	CONFIG_I2C_KIRKWOOD
#define	CONFIG_I2C_KW_REG_BASE		KW_TWSI_BASE
#define	CONFIG_SYS_I2C_SLAVE		0x0
#define	CONFIG_SYS_I2C_SPEED		100000
#endif

#define	CONFIG_KIRKWOOD_GPIO		/* Enable GPIO Support */
#if defined(CONFIG_SOFT_I2C)
#ifndef __ASSEMBLY__
#include <asm/arch-kirkwood/gpio.h>
extern void __set_direction(unsigned pin, int high);
void set_sda (int state);
void set_scl (int state);
int get_sda (void);
int get_scl (void);
#define SUEN3_SDA_PIN	8
#define SUEN3_SCL_PIN	9
#define SUEN3_ENV_WP	38

#define I2C_ACTIVE	__set_direction(SUEN3_SDA_PIN, 0)
#define I2C_TRISTATE	__set_direction(SUEN3_SDA_PIN, 1)
#define I2C_READ	(kw_gpio_get_value(SUEN3_SDA_PIN) ? 1 : 0)
#define I2C_SDA(bit)	kw_gpio_set_value(SUEN3_SDA_PIN, bit);
#define I2C_SCL(bit)	kw_gpio_set_value(SUEN3_SCL_PIN, bit);
#endif

#define I2C_DELAY	udelay(3)	/* 1/4 I2C clock duration */
#define I2C_SOFT_DECLARATIONS

#define	CONFIG_SYS_I2C_SLAVE		0x0
#define	CONFIG_SYS_I2C_SPEED		100000
#endif

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2

#if defined(CONFIG_SYS_NO_FLASH)
#define CONFIG_KM_UBI_PARTITION_NAME   "ubi0"
#undef	CONFIG_FLASH_CFI_MTD
#undef	CONFIG_JFFS2_CMDLINE
#endif

#endif /* _CONFIG_KM_ARM_H */
