/*
 * SchulerControl GmbH, SC_SPS_1 module config
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 * on behalf of DENX Software Engineering GmbH
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __SC_SPS_1_H__
#define __SC_SPS_1_H__

/*
 * SoC configurations
 */
#define CONFIG_MX28				/* i.MX28 SoC */
#define CONFIG_MXS_GPIO				/* GPIO control */
#define CONFIG_SYS_HZ		1000		/* Ticks per second */

/*
 * Define SC_SPS_1 machine type by hand until it lands in mach-types
 */
#define MACH_TYPE_SC_SPS_1	4172

#define CONFIG_MACH_TYPE	MACH_TYPE_SC_SPS_1

#include <asm/arch/regs-base.h>

#define CONFIG_SYS_NO_FLASH
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_ARCH_MISC_INIT

#define CONFIG_ENV_IS_IN_MMC

#define CONFIG_OF_LIBFDT

/*
 * SPL
 */
#define CONFIG_SPL
#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"arch/arm/cpu/arm926ejs/mxs"
#define CONFIG_SPL_LDSCRIPT	"arch/arm/cpu/arm926ejs/mxs/u-boot-spl.lds"
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT

/*
 * U-Boot Commands
 */
#include <config_cmd_default.h>
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DOS_PARTITION

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_GPIO
#define CONFIG_CMD_MII
#define CONFIG_CMD_MMC
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_USB

/*
 * Memory configurations
 */
#define CONFIG_NR_DRAM_BANKS		1		/* 1 bank of DRAM */
#define PHYS_SDRAM_1			0x40000000	/* Base address */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* Max 1 GB RAM */
#define CONFIG_STACKSIZE		0x00010000	/* 128 KB stack */
#define CONFIG_SYS_MALLOC_LEN		0x00400000	/* 4 MB for malloc */
#define CONFIG_SYS_GBL_DATA_SIZE	128		/* Initial data */
#define CONFIG_SYS_MEMTEST_START	0x40000000	/* Memtest start adr */
#define CONFIG_SYS_MEMTEST_END		0x40400000	/* 4 MB RAM test */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Point initial SP in SRAM so SPL can use it too. */
#define CONFIG_SYS_INIT_RAM_ADDR	0x00000000
#define CONFIG_SYS_INIT_RAM_SIZE	(128 * 1024)

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)
/*
 * We need to sacrifice first 4 bytes of RAM here to avoid triggering some
 * strange BUG in ROM corrupting first 4 bytes of RAM when loading U-Boot
 * binary. In case there was more of this mess, 0x100 bytes are skipped.
 */
#define CONFIG_SYS_TEXT_BASE		0x40000100

/*
 * U-Boot general configurations
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT	"=> "
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O buffer size */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
						/* Print buffer size */
#define CONFIG_SYS_MAXARGS	32		/* Max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
						/* Boot argument buffer size */
#define CONFIG_VERSION_VARIABLE			/* U-BOOT version */
#define CONFIG_AUTO_COMPLETE			/* Command auto complete */
#define CONFIG_CMDLINE_EDITING			/* Command history etc */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

/*
 * Serial Driver
 */
#define CONFIG_PL011_SERIAL
#define CONFIG_PL011_CLOCK		24000000
#define CONFIG_PL01x_PORTS		{ (void *)MXS_UARTDBG_BASE }
#define CONFIG_CONS_INDEX		0
#define CONFIG_BAUDRATE			115200	/* Default baud rate */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * MMC Driver
 */
#ifdef CONFIG_CMD_MMC
#define CONFIG_APBH_DMA
#define CONFIG_MMC
#define CONFIG_MMC_BOUNCE_BUFFER
#define CONFIG_GENERIC_MMC
#define CONFIG_MXS_MMC
#endif
#define CONFIG_ENV_SIZE			(16 * 1024)
#ifdef CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET		(256 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0
#else
#define CONFIG_ENV_IS_NOWHERE
#endif

/*
 * Ethernet on SOC (FEC)
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_ETHPRIME			"FEC0"
#define CONFIG_FEC_MXC
#define CONFIG_FEC_MXC_MULTI
#define CONFIG_MII
#define CONFIG_DISCOVER_PHY
#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_PHYLIB
#define CONFIG_PHY_SMSC
#endif

/*
 * USB
 */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MXS
#define CONFIG_EHCI_MXS_PORT		0
#define CONFIG_EHCI_IS_TDI
#define CONFIG_USB_STORAGE
#endif

/*
 * Boot Linux
 */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_BOOTARGS		"console=ttyAMA0,115200"
#define CONFIG_BOOTCOMMAND	"bootm "
#define CONFIG_LOADADDR		0x42000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/*
 * Extra Environments
 */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"update_sd_firmware_filename=u-boot.sd\0"			\
	"update_sd_firmware="		/* Update the SD firmware partition */ \
		"if mmc rescan ; then "					\
		"if tftp ${update_sd_firmware_filename} ; then "	\
		"setexpr fw_sz ${filesize} / 0x200 ; "	/* SD block size */ \
		"setexpr fw_sz ${fw_sz} + 1 ; "				\
		"mmc write ${loadaddr} 0x800 ${fw_sz} ; "		\
		"fi ; "							\
		"fi\0"

#endif /* __SC_SPS_1_H__ */
