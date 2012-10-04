/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/socfpga_base_addrs.h>

/*
 * High level configuration
 */

#define CONFIG_ARMV7
#define CONFIG_L2_OFF
#define CONFIG_SYS_DCACHE_OFF
#undef CONFIG_USE_IRQ

#define CONFIG_MISC_INIT_R
#define CONFIG_SINGLE_BOOTLOADER
#define CONFIG_SOCFPGA

#define CONFIG_SYS_TEXT_BASE		0x08000040
#define V_NS16550_CLK			1000000
#define CONFIG_BAUDRATE			57600
#define CONFIG_SYS_HZ			1000
#define CONFIG_TIMER_CLOCK_KHZ		2400
#define CONFIG_SYS_LOAD_ADDR		0x7fc0

/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		256
/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT		"SOCFPGA_CYCLONE5 # "
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

/*
 * Display CPU and Board Info
 */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/*
 * Enable early stage initialization at C environment
 */
#define CONFIG_BOARD_EARLY_INIT_F

/* flat device tree */
#define CONFIG_OF_LIBFDT
/* skip updating the FDT blob */
#define CONFIG_FDT_BLOB_SKIP_UPDATE
/* Initial Memory map size for Linux, minus 4k alignment for DFT blob */
#define CONFIG_SYS_BOOTMAPSZ		((256*1024*1024) - (4*1024))

#define CONFIG_SPL_RAM_DEVICE
#define CONFIG_SPL_STACK (&__stack_start)
#define CONFIG_SYS_SPL_MALLOC_START ((unsigned long) (&__malloc_start))
#define CONFIG_SYS_SPL_MALLOC_SIZE (&__malloc_end - &__malloc_start)

/*
 * Memory allocation (MALLOC)
 */
/* Room required on the stack for the environment data */
#define CONFIG_ENV_SIZE			1024
/* Size of DRAM reserved for malloc() use */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)

/* SP location before relocation, must use scratch RAM */
#define CONFIG_SYS_INIT_RAM_ADDR	0xFFFF0000
/* Reserving 0x100 space at back of scratch RAM for debug info */
#define CONFIG_SYS_INIT_RAM_SIZE	(0x10000 - 0x100)
/* Stack pointer prior relocation, must situated at on-chip RAM */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)


/*
 * Command line configuration.
 */
#define CONFIG_SYS_NO_FLASH
#include <config_cmd_default.h>
/* FAT file system support */
#define CONFIG_CMD_FAT


/*
 * Misc
 */
#define CONFIG_DOS_PARTITION            1

#ifdef CONFIG_SPL_BUILD
#undef CONFIG_PARTITIONS
#endif

/*
 * Environment setup
 */

/* Delay before automatically booting the default image */
#define CONFIG_BOOTDELAY		3
/* Enable auto completion of commands using TAB */
#define CONFIG_AUTO_COMPLETE
/* use "hush" command parser */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMD_RUN

#define CONFIG_BOOTCOMMAND "run ramboot"

/*
 * arguments passed to the bootm command. The value of
 * CONFIG_BOOTARGS goes into the environment value "bootargs".
 * Do note the value will overide also the chosen node in FDT blob.
 */
#define CONFIG_BOOTARGS "console=ttyS0,57600,mem=256M@0x0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=n\0" \
	"loadaddr= " MK_STR(CONFIG_SYS_LOAD_ADDR) "\0" \
	"ramboot=setenv bootargs " CONFIG_BOOTARGS ";" \
		"bootm ${loadaddr} - ${fdt_addr}\0" \
	"bootimage=uImage\0" \
	"fdt_addr=100\0" \
	"fsloadcmd=ext2load\0" \
		"bootm ${loadaddr} - ${fdt_addr}\0" \
	"qspiroot=/dev/mtdblock0\0" \
	"qspirootfstype=jffs2\0" \
	"qspiboot=setenv bootargs " CONFIG_BOOTARGS \
		" root=${qspiroot} rw rootfstype=${qspirootfstype};"\
		"bootm ${loadaddr} - ${fdt_addr}\0"

/* using environment setting for stdin, stdout, stderr */
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
/* Enable the call to overwrite_console() */
#define CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
/* Enable overwrite of previous console environment settings */
#define CONFIG_SYS_CONSOLE_ENV_OVERWRITE

/* max number of command args	 */
#define CONFIG_SYS_MAXARGS		16


/*
 * Hardware drivers
 */

/*
 * SDRAM Memory Map
 */
/* We have 1 bank of DRAM */
#define CONFIG_NR_DRAM_BANKS		1
/* SDRAM Bank #1 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
/* SDRAM memory size */
#define PHYS_SDRAM_1_SIZE		0x80000000

#define PHYS_SDRAM_1			CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_START	0x00000000
#define CONFIG_SYS_MEMTEST_END		PHYS_SDRAM_1_SIZE

/*
 * NS16550 Configuration
 */
#define UART0_BASE			SOCFPGA_UART0_ADDRESS
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_CLK          V_NS16550_CLK
#define CONFIG_CONS_INDEX               1
#define CONFIG_SYS_NS16550_COM1		UART0_BASE

#define CONFIG_SYS_BAUDRATE_TABLE {4800, 9600, 19200, 38400, 57600, 115200}

/*
 * FLASH
 */
#define CONFIG_SYS_NO_FLASH

/*
 * L4 OSC1 Timer 0
 */
/* This timer use eosc1 where the clock frequency is fixed
 * throughout any condition */
#define CONFIG_SYS_TIMERBASE		SOCFPGA_OSC1TIMER0_ADDRESS

/* reload value when timer count to zero */
#define TIMER_LOAD_VAL			0xFFFFFFFF

#define CONFIG_ENV_IS_NOWHERE

/*
 * SPL "Second Program Loader" aka Initial Software
 */

/* Enable building of SPL globally */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK

/* TEXT_BASE for linking the SPL binary */
#define CONFIG_SPL_TEXT_BASE		0xFFFF0000

/* Stack size for SPL */
#define CONFIG_SPL_STACK_SIZE		(4 * 1024)

/* MALLOC size for SPL */
#define CONFIG_SPL_MALLOC_SIZE		(5 * 1024)

#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_BOARD_INIT

#define CHUNKSZ_CRC32			(1 * 1024)

#define CONFIG_CRC32_VERIFY

/* Linker script for SPL */
#define CONFIG_SPL_LDSCRIPT "arch/arm/cpu/armv7/socfpga/u-boot-spl.lds"

/* Support for common/libcommon.o in SPL binary */
#define CONFIG_SPL_LIBCOMMON_SUPPORT
/* Support for lib/libgeneric.o in SPL binary */
#define CONFIG_SPL_LIBGENERIC_SUPPORT

#endif	/* __CONFIG_H */
