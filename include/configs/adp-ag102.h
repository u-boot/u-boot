/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/ag102.h>

/*
 * CPU and Board Configuration Options
 */
#define CONFIG_ADP_AG102

#define CONFIG_USE_INTERRUPT

#define CONFIG_SKIP_LOWLEVEL_INIT

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_MEM_REMAP
#endif

#ifdef CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_TEXT_BASE	0x04200000
#else
#define CONFIG_SYS_TEXT_BASE	0x00000000
#endif

/*
 * Timer
 */
#define CONFIG_SYS_CLK_FREQ	(66000000 * 2)
#define VERSION_CLOCK		CONFIG_SYS_CLK_FREQ

/*
 * Use Externel CLOCK or PCLK
 */
#undef CONFIG_FTRTC010_EXTCLK

#ifndef CONFIG_FTRTC010_EXTCLK
#define CONFIG_FTRTC010_PCLK
#endif

#ifdef CONFIG_FTRTC010_EXTCLK
#define TIMER_CLOCK	32768			/* CONFIG_FTRTC010_EXTCLK */
#else
#define TIMER_CLOCK	CONFIG_SYS_HZ		/* CONFIG_FTRTC010_PCLK */
#endif

#define TIMER_LOAD_VAL	0xffffffff

/*
 * Real Time Clock
 */
#define CONFIG_RTC_FTRTC010

/*
 * Real Time Clock Divider
 * RTC_DIV_COUNT			(OSC_CLK/OSC_5MHZ)
 */
#define OSC_5MHZ			(5*1000000)
#define OSC_CLK				(2*OSC_5MHZ)
#define RTC_DIV_COUNT			(OSC_CLK/OSC_5MHZ)

/*
 * Serial console configuration
 */

/* FTUART is a high speed NS 16C550A compatible UART */
#define CONFIG_BAUDRATE			38400
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_COM1		CONFIG_FTUART010_01_BASE
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_CLK		33000000	/* AG102 */

/*
 * Ethernet
 */
#define CONFIG_PHY_MAX_ADDR	32	/* this comes from <linux/phy.h> */
#define CONFIG_SYS_DISCOVER_PHY
#define CONFIG_FTGMAC100
#define CONFIG_FTGMAC100_EGIGA

#define CONFIG_BOOTDELAY	3

/*
 * SD (MMC) controller
 */
#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_FTSDC010
#define CONFIG_FTSDC010_NUMBER		1
#define CONFIG_FTSDC010_SDIO
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2

/*
 * Command line configuration.
 */
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_PING
#define CONFIG_CMD_IDE
#define CONFIG_CMD_FAT
#define CONFIG_CMD_ELF


/*
 * PCI
 */
#define CONFIG_PCI
#define CONFIG_FTPCI100
#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_FTPCI100_MEM_BASE        0xa0000000
#define CONFIG_FTPCI100_IO_SIZE         FTPCI100_BASE_IO_SIZE(256) /* 256M */
#define CONFIG_FTPCI100_MEM_SIZE        FTPCI100_MEM_SIZE(128)  /* 128M */
#define CONFIG_FTPCI100_MEM_BASE_SIZE1  0x50

#define CONFIG_PCI_MEM_BUS	0xa0000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x01000000		/* 256M */

#define CONFIG_PCI_IO_BUS	0x90000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x00100000		/* 1M */

/*
 * USB
 */
#if defined(CONFIG_CMD_PCI) || defined(CONFIG_PCI)
#if defined(CONFIG_FTPCI100)
#define __io /* enable outl & inl */
#define CONFIG_CMD_USB
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS      5
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI
#define CONFIG_PCI_EHCI_DEVICE 0
#define CONFIG_USB_EHCI_PCI
#define CONFIG_PREBOOT                          "usb start;"
#endif /* #if defiend(CONFIG_FTPCI100) */
#endif /* #if defined(CONFIG_CMD_PCI) || defined(CONFIG_PCI) */

/*
 * IDE/ATA stuff
 */
#define __io
#define CONFIG_IDE_AHB
#define CONFIG_IDE_FTIDE020

#undef	CONFIG_IDE_8xx_DIRECT		/* no pcmcia interface required */
#undef	CONFIG_IDE_LED			/* no led for ide supported	*/
#define CONFIG_IDE_RESET	1	/* reset for ide supported	*/
#define CONFIG_IDE_PREINIT	1	/* preinit for ide		*/

/* max: 2 IDE busses */
#define CONFIG_SYS_IDE_MAXBUS		1	/* origin: 2 */
/* max: 2 drives per IDE bus */
#define CONFIG_SYS_IDE_MAXDEVICE	1	/* origin: (MAXBUS * 2) */

#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_FTIDE020S_BASE
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000
#define CONFIG_SYS_ATA_IDE1_OFFSET	0x0000

#define CONFIG_SYS_ATA_DATA_OFFSET	0x0000	/* for data I/O */
#define CONFIG_SYS_ATA_REG_OFFSET	0x0000	/* for normal regs access */
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0000	/* for alternate regs */

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_PROMPT	"NDS32 # "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/* max number of command args */
#define CONFIG_SYS_MAXARGS	16

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128 * 1024)

/*
 * AHB Controller configuration
 */
#define CONFIG_FTAHBC020S

#ifdef CONFIG_FTAHBC020S
#include <faraday/ftahbc020s.h>

/* Address of PHYS_SDRAM_0 before memory remap is at 0x(100)00000 */
#define CONFIG_SYS_FTAHBC020S_SLAVE_BSR_BASE	0x100

/*
 * CONFIG_SYS_FTAHBC020S_SLAVE_BSR_6: this define is used in lowlevel_init.S,
 * hence we cannot use FTAHBC020S_BSR_SIZE(2048) since it will use ffs() wrote
 * in C language.
 */
#define CONFIG_SYS_FTAHBC020S_SLAVE_BSR_6 \
	(FTAHBC020S_SLAVE_BSR_BASE(CONFIG_SYS_FTAHBC020S_SLAVE_BSR_BASE) | \
					 FTAHBC020S_SLAVE_BSR_SIZE(0xb))
#endif

/*
 * Watchdog
 */
#define CONFIG_FTWDT010_WATCHDOG

/*
 * PCU Power Control Unit configuration
 */
#define CONFIG_ANDES_PCU

#ifdef CONFIG_ANDES_PCU
#include <andestech/andes_pcu.h>

#endif

/*
 * DDR DRAM controller configuration
 */
#define CONFIG_DWCDDR21MCTL

#ifdef CONFIG_DWCDDR21MCTL
#include <synopsys/dwcddr21mctl.h>
/* DCR:
 *   2GB: 0x000025d2, 2GB (1Gb x8 2 ranks) Micron/innoDisk/Transcend
 *   1GB: 0x000021d2, 1GB (1Gb x8 1 rank) Micron/Transcend/innoDisk
 * 512MB: 0x000025cc, Micron 512MB (512Mb x16 2 ranks)
 * 512MB: 0x000021ca, Trenscend/innoDisk 512MB (512Mb x8 1 rank)
 * 256MB: 0x000020d4, Micron 256MB (1Gb x16 1 ranks)
 */
#define CONFIG_SYS_DWCDDR21MCTL_CCR	0x00020004
#define CONFIG_SYS_DWCDDR21MCTL_CCR2	(DWCDDR21MCTL_CCR_DTT(0x1)	| \
					DWCDDR21MCTL_CCR_DFTLM(0x4)	| \
					DWCDDR21MCTL_CCR_HOSTEN(0x1))

/* 0x04: 0x000020d4 */
#define CONFIG_SYS_DWCDDR21MCTL_DCR	0x000020ca

/* 0x08: 0x0000000f */
#define CONFIG_SYS_DWCDDR21MCTL_IOCR	0x0000000f

/* 0x10: 0x00034812 */
#define CONFIG_SYS_DWCDDR21MCTL_DRR	(DWCDDR21MCTL_DRR_TRFC(0x12)	| \
					DWCDDR21MCTL_DRR_TRFPRD(0x0348))
/* 0x24 */
#define CONFIG_SYS_DWCDDR21MCTL_DLLCR0	DWCDDR21MCTL_DLLCR_PHASE(0x0)

/* 0x4c: 0x00000040 */
#define CONFIG_SYS_DWCDDR21MCTL_RSLR0	0x00000040

/* 0x5c: 0x000055CF */
#define CONFIG_SYS_DWCDDR21MCTL_RDGR0	0x000055cf

/* 0xa4: 0x00100000 */
#define CONFIG_SYS_DWCDDR21MCTL_DTAR	(DWCDDR21MCTL_DTAR_DTBANK(0x0)	| \
					DWCDDR21MCTL_DTAR_DTROW(0x0100)	| \
					DWCDDR21MCTL_DTAR_DTCOL(0x0))
/* 0x1f0: 0x00000852 */
#define CONFIG_SYS_DWCDDR21MCTL_MR	(DWCDDR21MCTL_MR_WR(0x4)	| \
					DWCDDR21MCTL_MR_CL(0x5)		| \
					DWCDDR21MCTL_MR_BL(0x2))
#endif

/*
 * Physical Memory Map
 */
#if defined(CONFIG_MEM_REMAP) || defined(CONFIG_SKIP_LOWLEVEL_INIT)
#define PHYS_SDRAM_0		0x00000000	/* SDRAM Bank #1 */
#if defined(CONFIG_MEM_REMAP)
#define PHYS_SDRAM_0_AT_INIT	0x80000000	/* SDRAM Bank #1 before remap*/
#endif
#else	/* !CONFIG_SKIP_LOWLEVEL_INIT && !CONFIG_MEM_REMAP */
#define PHYS_SDRAM_0		0x80000000	/* SDRAM Bank #1 */
#endif

#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_0_SIZE	0x10000000	/* 256 MB */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_0

#ifdef CONFIG_MEM_REMAP
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0xA0000 - \
					GENERATED_GBL_DATA_SIZE)
#else
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x1000 - \
					GENERATED_GBL_DATA_SIZE)
#endif /* CONFIG_MEM_REMAP */

/*
 * Load address and memory test area should agree with
 * board/faraday/a320/config.mk
 * Be careful not to overwrite U-boot itself.
 */
#define CONFIG_SYS_LOAD_ADDR		0x0CF00000

/* memtest works on 63 MB in DRAM */
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_0
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_0 + 0x03F00000)

/*
 * Static memory controller configuration
 */

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_NO_FLASH

/*
 * Env Storage Settings
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			4096

#endif	/* __CONFIG_H */
