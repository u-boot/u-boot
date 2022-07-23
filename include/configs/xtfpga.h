/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007-2013 Tensilica, Inc.
 * Copyright (C) 2014 - 2016 Cadence Design Systems Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/core.h>
#include <asm/addrspace.h>
#include <asm/config.h>

/*
 * The 'xtfpga' board describes a set of very similar boards with only minimal
 * differences.
 */

/*===================*/
/* RAM Layout        */
/*===================*/

#if XCHAL_HAVE_PTP_MMU
#define CONFIG_SYS_MEMORY_BASE		\
	(XCHAL_VECBASE_RESET_VADDR - XCHAL_VECBASE_RESET_PADDR)
#define CONFIG_SYS_IO_BASE		0xf0000000
#else
#define CONFIG_SYS_MEMORY_BASE		0x60000000
#define CONFIG_SYS_IO_BASE		0x90000000
#define CONFIG_MAX_MEM_MAPPED		0x10000000
#endif

/* Onboard RAM sizes:
 *
 * LX60		0x04000000		  64 MB
 * LX110	0x03000000		  48 MB
 * LX200	0x06000000		  96 MB
 * ML605	0x18000000		 384 MB
 * KC705	0x38000000		 896 MB
 *
 * noMMU configurations can only see first 256MB of onboard memory.
 */

#if XCHAL_HAVE_PTP_MMU || CONFIG_BOARD_SDRAM_SIZE < 0x10000000
#define CONFIG_SYS_SDRAM_SIZE		CONFIG_BOARD_SDRAM_SIZE
#else
#define CONFIG_SYS_SDRAM_SIZE		0x10000000
#endif

#define CONFIG_SYS_SDRAM_BASE		MEMADDR(0x00000000)

/* Lx60 can only map 128kb memory (instead of 256kb) when running under OCD */
#ifdef CONFIG_XTFPGA_LX60
# define CONFIG_SYS_MONITOR_LEN		0x00020000	/* 128KB */
#else
# define CONFIG_SYS_MONITOR_LEN		0x00040000	/* 256KB */
#endif

/* Memory test is destructive so default must not overlap vectors or U-Boot*/

/* Load address for stand-alone applications.
 * MEMADDR cannot be used here, because the definition needs to be
 * a plain number as it's used as -Ttext argument for ld in standalone
 * example makefile.
 * Handle noMMU vs MMUv2 vs MMUv3 distinction here manually.
 */
#if XCHAL_HAVE_PTP_MMU
#if XCHAL_VECBASE_RESET_VADDR == XCHAL_VECBASE_RESET_PADDR
#define CONFIG_STANDALONE_LOAD_ADDR	0x00800000
#else
#define CONFIG_STANDALONE_LOAD_ADDR	0xd0800000
#endif
#else
#define CONFIG_STANDALONE_LOAD_ADDR	0x60800000
#endif

#if defined(CONFIG_MAX_MEM_MAPPED) && \
	CONFIG_MAX_MEM_MAPPED < CONFIG_SYS_SDRAM_SIZE
#define XTENSA_SYS_TEXT_ADDR		\
	(MEMADDR(CONFIG_MAX_MEM_MAPPED) - CONFIG_SYS_MONITOR_LEN)
#else
#define XTENSA_SYS_TEXT_ADDR		\
	(MEMADDR(CONFIG_SYS_SDRAM_SIZE) - CONFIG_SYS_MONITOR_LEN)
#endif

/*==============================*/
/* U-Boot general configuration */
/*==============================*/

	/* Console I/O Buffer Size  */
/*==============================*/
/* U-Boot autoboot configuration */
/*==============================*/


/*=========================================*/
/* FPGA Registers (board info and control) */
/*=========================================*/

/*
 * These assume FPGA bitstreams from Tensilica release RB and up. Earlier
 * releases may not provide any/all of these registers or at these offsets.
 * Some of the FPGA registers are broken down into bitfields described by
 * SHIFT left amount and field WIDTH (bits), and also by a bitMASK.
 */

/* FPGA core clock frequency in Hz (also input to UART) */
#define CONFIG_SYS_FPGAREG_FREQ	IOADDR(0x0D020004)	/* CPU clock frequency*/

/*
 * DIP switch (left=sw1=lsb=bit0, right=sw8=msb=bit7; off=0, on=1):
 *   Bits 0..5 set the lower 6 bits of the default ethernet MAC.
 *   Bit 6 is reserved for future use by Tensilica.
 *   Bit 7 maps the first 128KB of ROM address space at CONFIG_SYS_ROM_BASE to
 *   the base of flash * (when on/1) or to the base of RAM (when off/0).
 */
#define CONFIG_SYS_FPGAREG_DIPSW	IOADDR(0x0D02000C)
#define FPGAREG_MAC_SHIFT		0	/* Ethernet MAC bits 0..5 */
#define FPGAREG_MAC_WIDTH		6
#define FPGAREG_MAC_MASK		0x3f
#define FPGAREG_BOOT_SHIFT		7	/* Boot ROM addr mapping */
#define FPGAREG_BOOT_WIDTH		1
#define FPGAREG_BOOT_MASK		0x80
#define FPGAREG_BOOT_RAM		0
#define FPGAREG_BOOT_FLASH		(1<<FPGAREG_BOOT_SHIFT)

/* Force hard reset of board by writing a code to this register */
#define CONFIG_SYS_FPGAREG_RESET	IOADDR(0x0D020010) /* Reset board .. */
#define CONFIG_SYS_FPGAREG_RESET_CODE	0x0000DEAD   /*  by writing this code */

/*====================*/
/* Serial Driver Info */
/*====================*/

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_COM1		IOADDR(0x0D050020) /* Base address */

/* Input clk to NS16550 (in Hz; the SYS_CLK_FREQ is in kHz) */
#define CONFIG_SYS_NS16550_CLK		get_board_sys_clk()

/*======================*/
/* Ethernet Driver Info */
/*======================*/

#define CONFIG_ETHBASE			00:50:C2:13:6f:00
#define CONFIG_SYS_ETHOC_BASE		IOADDR(0x0d030000)
#define CONFIG_SYS_ETHOC_BUFFER_ADDR	IOADDR(0x0D800000)

/*=====================*/
/* Flash & Environment */
/*=====================*/

#ifdef CONFIG_XTFPGA_LX60
# define CONFIG_SYS_FLASH_SIZE		0x0040000	/* 4MB */
# define CONFIG_SYS_FLASH_PARMSECT_SZ	0x2000		/* param size  8KB */
# define CONFIG_SYS_FLASH_BASE		IOADDR(0x08000000)
#elif defined(CONFIG_XTFPGA_KC705)
# define CONFIG_SYS_FLASH_SIZE		0x8000000	/* 128MB */
# define CONFIG_SYS_FLASH_PARMSECT_SZ	0x8000		/* param size 32KB */
# define CONFIG_SYS_FLASH_BASE		IOADDR(0x00000000)
#else
# define CONFIG_SYS_FLASH_SIZE		0x1000000	/* 16MB */
# define CONFIG_SYS_FLASH_PARMSECT_SZ	0x8000		/* param size 32KB */
# define CONFIG_SYS_FLASH_BASE		IOADDR(0x08000000)
#endif

/*
 * Put environment in top block (64kB)
 * Another option would be to put env. in 2nd param block offs 8KB, size 8KB
 */

/* print 'E' for empty sector on flinfo */

#endif /* __CONFIG_H */
