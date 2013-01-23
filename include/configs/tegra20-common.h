/*
 *  (C) Copyright 2010-2012
 *  NVIDIA Corporation <www.nvidia.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __TEGRA20_COMMON_H
#define __TEGRA20_COMMON_H
#include <asm/sizes.h>
#include <linux/stringify.h>

/*
 * High Level Configuration Options
 */
#define CONFIG_ARMCORTEXA9		/* This is an ARM V7 CPU core */
#define CONFIG_TEGRA20			/* in a NVidia Tegra20 core */
#define CONFIG_TEGRA			/* which is a Tegra generic machine */
#define CONFIG_SYS_L2CACHE_OFF		/* No L2 cache */

#define CONFIG_SYS_CACHELINE_SIZE	32

#include <asm/arch/tegra.h>		/* get chip and board defs */

/* Align LCD to 1MB boundary */
#define CONFIG_LCD_ALIGNMENT	MMU_SECTION_SIZE

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_OF_LIBFDT		/* enable passing of devicetree */

#ifdef CONFIG_TEGRA_LP0
#define TEGRA_LP0_ADDR			0x1C406000
#define TEGRA_LP0_SIZE			0x2000
#define TEGRA_LP0_VEC \
	"lp0_vec=" __stringify(TEGRA_LP0_SIZE)	\
	"@" __stringify(TEGRA_LP0_ADDR) " "
#else
#define TEGRA_LP0_VEC
#endif

/* Environment */
#define CONFIG_ENV_VARS_UBOOT_CONFIG
#define CONFIG_ENV_SIZE			0x2000	/* Total Size Environment */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(4 << 20)	/* 4MB  */

/*
 * PllX Configuration
 */
#define CONFIG_SYS_CPU_OSC_FREQUENCY	1000000	/* Set CPU clock to 1GHz */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			216000000	/* 216MHz (pllp_out0) */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX	1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

/*
 * This parameter affects a TXFILLTUNING field that controls how much data is
 * sent to the latency fifo before it is sent to the wire. Without this
 * parameter, the default (2) causes occasional Data Buffer Errors in OUT
 * packets depending on the buffer address and size.
 */
#define CONFIG_USB_EHCI_TXFIFO_THRESH	10
#define CONFIG_EHCI_IS_TDI

/* Total I2C ports on Tegra20 */
#define TEGRA_I2C_NUM_CONTROLLERS	4

/* include default commands */
#include <config_cmd_default.h>
#define CONFIG_PARTITION_UUIDS
#define CONFIG_CMD_PART

/* remove unused commands */
#undef CONFIG_CMD_FLASH		/* flinfo, erase, protect */
#undef CONFIG_CMD_FPGA		/* FPGA configuration support */
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_NFS		/* NFS support */
#undef CONFIG_CMD_NET		/* network support */

/* turn on command-line edit/hist/auto */
#define CONFIG_CMDLINE_EDITING
#define CONFIG_COMMAND_HISTORY
#define CONFIG_AUTO_COMPLETE

#define CONFIG_SYS_NO_FLASH

#define CONFIG_CONSOLE_MUX
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define CONFIG_LOADADDR		0x408000	/* def. location for kernel */
#define CONFIG_BOOTDELAY	2		/* -1 to disable auto boot */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		V_PROMPT
/*
 * Increasing the size of the IO buffer as default nfsargs size is more
 *  than 256 and so it is not possible to edit it
 */
#define CONFIG_SYS_CBSIZE		(256 * 2) /* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_SYS_MEMTEST_START	(NV_PA_SDRC_CS0 + 0x600000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x100000)

#define CONFIG_SYS_LOAD_ADDR		(0xA00800)	/* default */
#define CONFIG_SYS_HZ			1000

#define CONFIG_STACKBASE	0x2800000	/* 40MB */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		NV_PA_SDRC_CS0
#define PHYS_SDRAM_1_SIZE	0x20000000	/* 512M */

#define CONFIG_SYS_TEXT_BASE	0x0010c000
#define CONFIG_SYS_UBOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#define CONFIG_SYS_BOOTMAPSZ	(256 << 20) /* 256M */

#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_STACKBASE
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
						CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)

#define CONFIG_TEGRA_GPIO
#define CONFIG_CMD_GPIO
#define CONFIG_CMD_ENTERRCM
#define CONFIG_CMD_BOOTZ

/* Defines for SPL */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_RAM_DEVICE
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_NAND_SIMPLE
#define CONFIG_SPL_TEXT_BASE		0x00108000
#define CONFIG_SPL_MAX_SIZE		(CONFIG_SYS_TEXT_BASE - \
						CONFIG_SPL_TEXT_BASE)
#define CONFIG_SYS_SPL_MALLOC_START	0x00090000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x00010000
#define CONFIG_SPL_STACK		0x000ffffc

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/tegra20/u-boot-spl.lds"

#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* Misc utility code */
#define CONFIG_BOUNCE_BUFFER

#endif /* __TEGRA20_COMMON_H */
