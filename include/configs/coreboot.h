/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#include <asm/ibmpc.h>
/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SYS_COREBOOT
#define CONFIG_SHOW_BOOT_PROGRESS
#define CONFIG_LAST_STAGE_INIT
#define CONFIG_SYS_VSNPRINTF
#define CONFIG_ZBOOT_32
#define CONFIG_PHYSMEM
#define CONFIG_SYS_EARLY_PCI_INIT

#define CONFIG_LMB
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE
#define CONFIG_DEFAULT_DEVICE_TREE	link

#define CONFIG_BOOTSTAGE
#define CONFIG_BOOTSTAGE_REPORT
#define CONFIG_BOOTSTAGE_FDT
#define CONFIG_CMD_BOOTSTAGE
/* Place to stash bootstage data from first-stage U-Boot */
#define CONFIG_BOOTSTAGE_STASH		0x0110f000
#define CONFIG_BOOTSTAGE_STASH_SIZE	0x7fc
#define CONFIG_BOOTSTAGE_USER_COUNT	60

#define CONFIG_LZO
#undef CONFIG_ZLIB
#undef CONFIG_GZIP

/*-----------------------------------------------------------------------
 * Watchdog Configuration
 */
#undef CONFIG_WATCHDOG
#undef CONFIG_HW_WATCHDOG

/* SATA AHCI storage */

#define CONFIG_SCSI_AHCI

#ifdef CONFIG_SCSI_AHCI
#define CONFIG_LIBATA
#define CONFIG_SYS_64BIT_LBA
#define CONFIG_SATA_INTEL		1
#define CONFIG_SCSI_DEV_LIST		{PCI_VENDOR_ID_INTEL, \
			PCI_DEVICE_ID_INTEL_NM10_AHCI},	      \
	{PCI_VENDOR_ID_INTEL,		\
			PCI_DEVICE_ID_INTEL_COUGARPOINT_AHCI_MOBILE}, \
	{PCI_VENDOR_ID_INTEL, \
			PCI_DEVICE_ID_INTEL_COUGARPOINT_AHCI_SERIES6}, \
	{PCI_VENDOR_ID_INTEL,		\
			PCI_DEVICE_ID_INTEL_PANTHERPOINT_AHCI_MOBILE}

#define CONFIG_SYS_SCSI_MAX_SCSI_ID	2
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)
#endif

/* Generic TPM interfaced through LPC bus */
#define CONFIG_TPM
#define CONFIG_TPM_TIS_LPC
#define CONFIG_TPM_TIS_BASE_ADDRESS        0xfed40000

/*-----------------------------------------------------------------------
 * Real Time Clock Configuration
 */
#define CONFIG_RTC_MC146818
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS	0
#define CONFIG_SYS_ISA_IO      CONFIG_SYS_ISA_IO_BASE_ADDRESS

/*-----------------------------------------------------------------------
 * Serial Configuration
 */
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		1843200
#define CONFIG_BAUDRATE			9600
#define CONFIG_SYS_BAUDRATE_TABLE	{300, 600, 1200, 2400, 4800, \
					 9600, 19200, 38400, 115200}
#define CONFIG_SYS_NS16550_COM1	UART0_BASE
#define CONFIG_SYS_NS16550_COM2	UART1_BASE
#define CONFIG_SYS_NS16550_PORT_MAPPED

#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,vga,eserial0\0" \
					"stdout=vga,eserial0,cbmem\0" \
					"stderr=vga,eserial0,cbmem\0"

#define CONFIG_CONSOLE_MUX
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_STDIO_DEREGISTER
#define CONFIG_CBMEM_CONSOLE

#define CONFIG_CMDLINE_EDITING
#define CONFIG_COMMAND_HISTORY
#define CONFIG_AUTOCOMPLETE

#define CONFIG_SUPPORT_VFAT
/************************************************************
 * ATAPI support (experimental)
 ************************************************************/
#define CONFIG_ATAPI

/************************************************************
 * DISK Partition support
 ************************************************************/
#define CONFIG_EFI_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_MAC_PARTITION
#define CONFIG_ISO_PARTITION		/* Experimental */

#define CONFIG_CMD_PART
#define CONFIG_CMD_CBFS
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
#define CONFIG_PARTITION_UUIDS

/*-----------------------------------------------------------------------
 * Video Configuration
 */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_COREBOOT
#define CONFIG_VIDEO_SW_CURSOR
#define VIDEO_FB_16BPP_WORD_SWAP
#define CONFIG_I8042_KBD
#define CONFIG_CFB_CONSOLE
#define CONFIG_SYS_CONSOLE_INFO_QUIET

/* x86 GPIOs are accessed through a PCI device */
#define CONFIG_INTEL_ICH6_GPIO

/*-----------------------------------------------------------------------
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_TRACE
#define CONFIG_CMD_TRACE
#define CONFIG_TRACE_BUFFER_SIZE	(16 << 20)
#define CONFIG_TRACE_EARLY_SIZE		(8 << 20)
#define CONFIG_TRACE_EARLY
#define CONFIG_TRACE_EARLY_ADDR		0x01400000

#define CONFIG_CMD_BDI
#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_ECHO
#undef CONFIG_CMD_FLASH
#define CONFIG_CMD_FPGA
#define CONFIG_CMD_GPIO
#define CONFIG_CMD_IMI
#undef CONFIG_CMD_IMLS
#define CONFIG_CMD_IO
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_ITEST
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC
#define CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_SETGETDCR
#define CONFIG_CMD_SOURCE
#define CONFIG_CMD_TIME
#define CONFIG_CMD_GETTIME
#define CONFIG_CMD_XIMG
#define CONFIG_CMD_SCSI

#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2

#define CONFIG_CMD_ZBOOT

#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTARGS		\
	"root=/dev/sdb3 init=/sbin/init rootwait ro"
#define CONFIG_BOOTCOMMAND	\
	"ext2load scsi 0:3 01000000 /boot/vmlinuz; zboot 01000000"


#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE			115200
#define CONFIG_KGDB_SER_INDEX			2
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT			"boot > "
#define CONFIG_SYS_CBSIZE			256
#define CONFIG_SYS_PBSIZE			(CONFIG_SYS_CBSIZE + \
						 sizeof(CONFIG_SYS_PROMPT) + \
						 16)
#define CONFIG_SYS_MAXARGS			16
#define CONFIG_SYS_BARGSIZE			CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START		0x00100000
#define CONFIG_SYS_MEMTEST_END			0x01000000
#define CONFIG_SYS_LOAD_ADDR			0x100000
#define CONFIG_SYS_HZ				1000

/*-----------------------------------------------------------------------
 * SDRAM Configuration
 */
#define CONFIG_NR_DRAM_BANKS			4

/* CONFIG_SYS_SDRAM_DRCTMCTL Overrides the following*/
#undef CONFIG_SYS_SDRAM_PRECHARGE_DELAY
#undef CONFIG_SYS_SDRAM_RAS_CAS_DELAY
#undef CONFIG_SYS_SDRAM_CAS_LATENCY_2T
#undef CONFIG_SYS_SDRAM_CAS_LATENCY_3T

/*-----------------------------------------------------------------------
 * CPU Features
 */

#define CONFIG_SYS_X86_TSC_TIMER
#define CONFIG_SYS_PCAT_INTERRUPTS
#define CONFIG_SYS_PCAT_TIMER
#define CONFIG_SYS_NUM_IRQS			16

/*-----------------------------------------------------------------------
 * Memory organization:
 * 32kB Stack
 * 16kB Cache-As-RAM @ 0x19200000
 * 256kB Monitor
 * (128kB + Environment Sector Size) malloc pool
 */
#define CONFIG_SYS_STACK_SIZE			(32 * 1024)
#define CONFIG_SYS_CAR_ADDR			0x19200000
#define CONFIG_SYS_CAR_SIZE			(16 * 1024)
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN			(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN			(0x20000 + 128 * 1024)


/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/*-----------------------------------------------------------------------
 * FLASH configuration
 */
#define CONFIG_ICH_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SPI_FLASH_GIGADEVICE
#define CONFIG_SYS_NO_FLASH
#define CONFIG_CMD_SF
#define CONFIG_CMD_SF_TEST
#define CONFIG_CMD_SPI
#define CONFIG_SPI

/*-----------------------------------------------------------------------
 * Environment configuration
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			0x01000

/*-----------------------------------------------------------------------
 * PCI configuration
 */
#define CONFIG_PCI

/*-----------------------------------------------------------------------
 * USB configuration
 */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_PCI
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS     12
#define CONFIG_USB_MAX_CONTROLLER_COUNT        2
#define CONFIG_USB_STORAGE
#define CONFIG_USB_KEYBOARD
#define CONFIG_SYS_USB_EVENT_POLL

#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_SMSC95XX

#define CONFIG_CMD_USB

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_STD_DEVICES_SETTINGS

#endif	/* __CONFIG_H */
