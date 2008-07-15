/*
 * Copyright 2007 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

/*
 * MPC8610HPCD board configuration file
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_MPC86xx		1	/* MPC86xx */
#define CONFIG_MPC8610		1	/* MPC8610 specific */
#define CONFIG_MPC8610HPCD	1	/* MPC8610HPCD board specific */
#define CONFIG_NUM_CPUS		1	/* Number of CPUs in the system */
#define CONFIG_LINUX_RESET_VEC	0x100	/* Reset vector used by Linux */

#define CONFIG_FSL_DIU_FB	1	/* FSL DIU */

/* video */
#undef CONFIG_VIDEO

#if defined(CONFIG_VIDEO)
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#endif

#ifdef RUN_DIAG
#define CFG_DIAG_ADDR		0xff800000
#endif

#define CFG_RESET_ADDRESS	0xfff00100

#define CONFIG_PCI		1	/* Enable PCI/PCIE*/
#define CONFIG_PCI1		1	/* PCI controler 1 */
#define CONFIG_PCIE1		1	/* PCIe 1 connected to ULI bridge */
#define CONFIG_PCIE2		1	/* PCIe 2 connected to slot */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_FSL_LAW		1	/* Use common FSL init code */

#define CONFIG_ENV_OVERWRITE

#define CONFIG_SPD_EEPROM		/* Use SPD for DDR */
#undef CONFIG_DDR_DLL			/* possible DLL fix needed */
#define CONFIG_DDR_2T_TIMING		/* Sets the 2T timing bit */
#undef  CONFIG_DDR_ECC			/* only for ECC DDR module */
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE		0xDeadBeef
#define CONFIG_NUM_DDR_CONTROLLERS	1
#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */

#define CONFIG_HIGH_BATS	1	/* High BATs supported & enabled */
#define CONFIG_ALTIVEC		1

/*
 * L2CR setup -- make sure this is right for your board!
 */
#define CFG_L2
#define L2_INIT		0
#define L2_ENABLE	(L2CR_L2E |0x00100000 )

#ifndef CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk(0)
#endif

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */
#define CONFIG_MISC_INIT_R		1

#define CFG_MEMTEST_START	0x00200000	/* memtest region */
#define CFG_MEMTEST_END		0x00400000

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_CCSRBAR_DEFAULT	0xff700000	/* CCSRBAR Default */
#define CFG_CCSRBAR		0xe0000000	/* relocated CCSRBAR */
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR */

#define CFG_PCI1_ADDR		(CFG_CCSRBAR+0x8000)
#define CFG_PCIE1_ADDR		(CFG_CCSRBAR+0xa000)
#define CFG_PCIE2_ADDR		(CFG_CCSRBAR+0x9000)

#define CFG_DIU_ADDR		(CFG_CCSRBAR+0x2c000)

/*
 * DDR Setup
 */
#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE
#define CONFIG_VERY_BIG_RAM

#define MPC86xx_DDR_SDRAM_CLK_CNTL

#if defined(CONFIG_SPD_EEPROM)
/*
 * Determine DDR configuration from I2C interface.
 */
#define SPD_EEPROM_ADDRESS1		0x51		/* DDR DIMM */
#else
/*
 * Manually set up DDR1 parameters
 */

#define CFG_SDRAM_SIZE	256		/* DDR is 256MB */

#if 0 /* TODO */
#define CFG_DDR_CS0_BNDS	0x0000000F
#define CFG_DDR_CS0_CONFIG	0x80010202	/* Enable, no interleaving */
#define CFG_DDR_TIMING_3	0x00000000
#define CFG_DDR_TIMING_0	0x00260802
#define CFG_DDR_TIMING_1	0x3935d322
#define CFG_DDR_TIMING_2	0x14904cc8
#define CFG_DDR_MODE_1		0x00480432
#define CFG_DDR_MODE_2		0x00000000
#define CFG_DDR_INTERVAL	0x06180100
#define CFG_DDR_DATA_INIT	0xdeadbeef
#define CFG_DDR_CLK_CTRL	0x03800000
#define CFG_DDR_OCD_CTRL	0x00000000
#define CFG_DDR_OCD_STATUS	0x00000000
#define CFG_DDR_CONTROL		0xe3008000	/* Type = DDR2 */
#define CFG_DDR_CONTROL2	0x04400010

#define CFG_DDR_ERR_INT_EN	0x00000000
#define CFG_DDR_ERR_DIS		0x00000000
#define CFG_DDR_SBE		0x000f0000
 /* Not used in fixed_sdram function */
#define CFG_DDR_MODE		0x00000022
#define CFG_DDR_CS1_BNDS	0x00000000
#define CFG_DDR_CS2_BNDS	0x00000FFF	/* Not done */
#define CFG_DDR_CS3_BNDS	0x00000FFF	/* Not done */
#define CFG_DDR_CS4_BNDS	0x00000FFF	/* Not done */
#define CFG_DDR_CS5_BNDS	0x00000FFF	/* Not done */
#endif
#endif

#define CONFIG_ID_EEPROM
#define CFG_I2C_EEPROM_NXID
#define CFG_ID_EEPROM
#define CFG_I2C_EEPROM_ADDR     0x57
#define CFG_I2C_EEPROM_ADDR_LEN 1


#define CFG_FLASH_BASE		0xf0000000 /* start of FLASH 128M */
#define CFG_FLASH_BASE2		0xf8000000

#define CFG_FLASH_BANKS_LIST {CFG_FLASH_BASE, CFG_FLASH_BASE2}

#define CFG_BR0_PRELIM		0xf8001001 /* port size 16bit */
#define CFG_OR0_PRELIM		0xf8006e65 /* 128MB NOR Flash*/

#define CFG_BR1_PRELIM		0xf0001001 /* port size 16bit */
#define CFG_OR1_PRELIM		0xf8006e65 /* 128MB Promjet */
#if 0 /* TODO */
#define CFG_BR2_PRELIM		0xf0000000
#define CFG_OR2_PRELIM		0xf0000000 /* 256MB NAND Flash - bank 1 */
#endif
#define CFG_BR3_PRELIM		0xe8000801 /* port size 8bit */
#define CFG_OR3_PRELIM		0xfff06ff7 /* 1MB PIXIS area*/


#define CONFIG_FSL_PIXIS	1	/* use common PIXIS code */
#define PIXIS_BASE	0xe8000000	/* PIXIS registers */
#define PIXIS_ID		0x0	/* Board ID at offset 0 */
#define PIXIS_VER		0x1	/* Board version at offset 1 */
#define PIXIS_PVER		0x2	/* PIXIS FPGA version at offset 2 */
#define PIXIS_RST		0x4	/* PIXIS Reset Control register */
#define PIXIS_AUX		0x6	/* PIXIS Auxiliary register; Scratch */
#define PIXIS_SPD		0x7	/* Register for SYSCLK speed */
#define PIXIS_BRDCFG0		0x8	/* PIXIS Board Configuration Register0*/
#define PIXIS_VCTL		0x10	/* VELA Control Register */
#define PIXIS_VCFGEN0		0x12	/* VELA Config Enable 0 */
#define PIXIS_VCFGEN1		0x13	/* VELA Config Enable 1 */
#define PIXIS_VBOOT		0x16	/* VELA VBOOT Register */
#define PIXIS_VSPEED0		0x17	/* VELA VSpeed 0 */
#define PIXIS_VSPEED1		0x18	/* VELA VSpeed 1 */
#define PIXIS_VCLKH		0x19	/* VELA VCLKH register */
#define PIXIS_VCLKL		0x1A	/* VELA VCLKL register */
#define CFG_PIXIS_VBOOT_MASK	0x0C    /* Reset altbank mask*/

#define CFG_MAX_FLASH_BANKS	2		/* number of banks */
#define CFG_MAX_FLASH_SECT	1024		/* sectors per device */

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */
#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_EMPTY_INFO

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef	CFG_RAMBOOT
#endif

#if defined(CFG_RAMBOOT)
#undef CONFIG_SPD_EEPROM
#define CFG_SDRAM_SIZE	256
#endif

#undef CONFIG_CLOCKS_IN_MHZ

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK	1
#ifndef CFG_INIT_RAM_LOCK
#define CFG_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address */
#else
#define CFG_INIT_RAM_ADDR	0xe4000000	/* Initial RAM address */
#endif
#define CFG_INIT_RAM_END	0x4000		/* End of used area in RAM */

#define CFG_GBL_DATA_SIZE	128		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN		(512 * 1024)	/* Reserve 512 KB for Mon */
#define CFG_MALLOC_LEN		(6 * 1024 * 1024)	/* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CFG_NS16550_COM1	(CFG_CCSRBAR+0x4500)
#define CFG_NS16550_COM2	(CFG_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/*
 * Pass open firmware flat tree to kernel
 */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1


/* maximum size of the flat tree (8K) */
#define OF_FLAT_TREE_MAX_SIZE	8192

#define CFG_64BIT_VSPRINTF	1
#define CFG_64BIT_STRTOUL	1

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES	{0x69}	/* Don't probe these addrs */
#define CFG_I2C_OFFSET		0x3000

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CFG_PCI1_MEM_BASE	0x80000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CFG_PCI1_IO_BASE	0x00000000
#define CFG_PCI1_IO_PHYS	0xe1000000
#define CFG_PCI1_IO_SIZE	0x00100000	/* 1M */

/* PCI view of System Memory */
#define CFG_PCI_MEMORY_BUS	0x00000000
#define CFG_PCI_MEMORY_PHYS	0x00000000
#define CFG_PCI_MEMORY_SIZE	0x80000000

/* For RTL8139 */
#define KSEG1ADDR(x)	({u32 _x = le32_to_cpu(*(u32 *)(x)); (&_x); })
#define _IO_BASE		0x00000000

/* controller 1, Base address 0xa000 */
#define CFG_PCIE1_MEM_BASE	0xa0000000
#define CFG_PCIE1_MEM_PHYS	CFG_PCIE1_MEM_BASE
#define CFG_PCIE1_MEM_SIZE	0x10000000	/* 256M */
#define CFG_PCIE1_IO_BASE	0x00000000
#define CFG_PCIE1_IO_PHYS	0xe3000000
#define CFG_PCIE1_IO_SIZE	0x00100000	/* 1M */

/* controller 2, Base Address 0x9000 */
#define CFG_PCIE2_MEM_BASE	0x90000000
#define CFG_PCIE2_MEM_PHYS	CFG_PCIE2_MEM_BASE
#define CFG_PCIE2_MEM_SIZE	0x10000000	/* 256M */
#define CFG_PCIE2_IO_BASE	0x00000000	/* reuse mem LAW */
#define CFG_PCIE2_IO_PHYS	0xe2000000
#define CFG_PCIE2_IO_SIZE	0x00100000	/* 1M */


#if defined(CONFIG_PCI)

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#define CONFIG_NET_MULTI
#define CONFIG_CMD_NET
#define CONFIG_PCI_PNP		/* do pci plug-and-play */
#define CONFIG_CMD_REGINFO

#define CONFIG_ULI526X
#ifdef CONFIG_ULI526X
#define CONFIG_ETHADDR   00:E0:0C:00:00:01
#endif

/************************************************************
 * USB support
 ************************************************************/
#define CONFIG_PCI_OHCI		1
#define CONFIG_USB_OHCI_NEW		1
#define CONFIG_USB_KEYBOARD	1
#define CFG_DEVICE_DEREGISTER
#define CFG_USB_EVENT_POLL	1
#define CFG_USB_OHCI_SLOT_NAME	"ohci_pci"
#define CFG_USB_OHCI_MAX_ROOT_PORTS 15
#define CFG_OHCI_SWAP_REG_ACCESS	1

#if !defined(CONFIG_PCI_PNP)
#define PCI_ENET0_IOADDR	0xe0000000
#define PCI_ENET0_MEMADDR	0xe0000000
#define PCI_IDSEL_NUMBER	0x0c	/* slot0->3(IDSEL)=12->15 */
#endif

#define CONFIG_DOS_PARTITION
#define CONFIG_SCSI_AHCI

#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SATA_ULI5288
#define CFG_SCSI_MAX_SCSI_ID	4
#define CFG_SCSI_MAX_LUN	1
#define CFG_SCSI_MAX_DEVICE	(CFG_SCSI_MAX_SCSI_ID * CFG_SCSI_MAX_LUN)
#define CFG_SCSI_MAXDEVICE	CFG_SCSI_MAX_DEVICE
#endif

#endif	/* CONFIG_PCI */

/*
 * BAT0		2G	Cacheable, non-guarded
 * 0x0000_0000	2G	DDR
 */
#define CFG_DBAT0L	(BATL_PP_RW | BATL_MEMCOHERENCE)
#define CFG_DBAT0U	(BATU_BL_2G | BATU_VS | BATU_VP)
#define CFG_IBAT0L	(BATL_PP_RW | BATL_MEMCOHERENCE )
#define CFG_IBAT0U	CFG_DBAT0U

/*
 * BAT1		1G	Cache-inhibited, guarded
 * 0x8000_0000	256M	PCI-1 Memory
 * 0xa000_0000	256M	PCI-Express 1 Memory
 * 0x9000_0000	256M	PCI-Express 2 Memory
 */

#define CFG_DBAT1L	(CFG_PCI1_MEM_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CFG_DBAT1U	(CFG_PCI1_MEM_PHYS | BATU_BL_1G | BATU_VS | BATU_VP)
#define CFG_IBAT1L	(CFG_PCI1_MEM_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT1U	CFG_DBAT1U

/*
 * BAT2		16M	Cache-inhibited, guarded
 * 0xe100_0000	1M	PCI-1 I/O
 */

#define CFG_DBAT2L	(CFG_PCI1_IO_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CFG_DBAT2U	(CFG_PCI1_IO_PHYS | BATU_BL_16M | BATU_VS | BATU_VP)
#define CFG_IBAT2L	(CFG_PCI1_IO_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT2U	CFG_DBAT2U

/*
 * BAT3		32M	Cache-inhibited, guarded
 * 0xe200_0000	1M	PCI-Express 2 I/O
 * 0xe300_0000	1M	PCI-Express 1 I/O
 */

#define CFG_DBAT3L	(CFG_PCIE2_IO_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CFG_DBAT3U	(CFG_PCIE2_IO_PHYS | BATU_BL_32M | BATU_VS | BATU_VP)
#define CFG_IBAT3L	(CFG_PCIE2_IO_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT3U	CFG_DBAT3U

/*
 * BAT4		4M	Cache-inhibited, guarded
 * 0xe000_0000	4M	CCSR
 */
#define CFG_DBAT4L	(CFG_CCSRBAR | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CFG_DBAT4U	(CFG_CCSRBAR | BATU_BL_1M | BATU_VS | BATU_VP)
#define CFG_IBAT4L	(CFG_CCSRBAR | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT4U	CFG_DBAT4U

/*
 * BAT5		128K	Cacheable, non-guarded
 * 0xe400_0000	128K	Init RAM for stack in the CPU DCache (no backing memory)
 */
#define CFG_DBAT5L	(CFG_INIT_RAM_ADDR | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CFG_DBAT5U	(CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CFG_IBAT5L	CFG_DBAT5L
#define CFG_IBAT5U	CFG_DBAT5U

/*
 * BAT6		256M	Cache-inhibited, guarded
 * 0xf000_0000	256M	FLASH
 */
#define CFG_DBAT6L	(CFG_FLASH_BASE	 | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CFG_DBAT6U	(CFG_FLASH_BASE	 | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_IBAT6L	(CFG_FLASH_BASE | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CFG_IBAT6U	CFG_DBAT6U

/*
 * BAT7		4M	Cache-inhibited, guarded
 * 0xe800_0000	4M	PIXIS
 */
#define CFG_DBAT7L	(PIXIS_BASE | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CFG_DBAT7U	(PIXIS_BASE | BATU_BL_1M | BATU_VS | BATU_VP)
#define CFG_IBAT7L	(PIXIS_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT7U	CFG_DBAT7U


/*
 * Environment
 */
#ifndef CFG_RAMBOOT
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define CFG_ENV_SECT_SIZE	0x20000	/* 126k (one sector) for env */
#define CFG_ENV_SIZE		0x2000
#else
#define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
#define CFG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change */


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII

#if defined(CFG_RAMBOOT)
#undef CONFIG_CMD_ENV
#endif

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SCSI
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_USB
#endif


#define CONFIG_WATCHDOG			/* watchdog enabled */
#define CFG_WATCHDOG_FREQ	5000	/* Feed interval, 5s */

/*DIU Configuration*/
#define DIU_CONNECT_TO_DVI		/* DIU controller connects to DVI encoder*/

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	*/
#define CONFIG_CMDLINE_EDITING          /* Command-line editing */
#define CFG_LOAD_ADDR	0x2000000	/* default load address */
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt */

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size */
#endif

#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_IPADDR		192.168.1.100

#define CONFIG_HOSTNAME		unknown
#define CONFIG_ROOTPATH		/opt/nfsroot
#define CONFIG_BOOTFILE		uImage
#define CONFIG_UBOOTPATH	8610hpcd/u-boot.bin

#define CONFIG_SERVERIP		192.168.1.1
#define CONFIG_GATEWAYIP	192.168.1.1
#define CONFIG_NETMASK		255.255.255.0

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#define CONFIG_BOOTDELAY 10	/* -1 disables auto-boot */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_BAUDRATE	115200

#if defined(CONFIG_PCI1)
#define PCI_ENV \
 "pcireg=md ${a}000 3; echo o;md ${a}c00 25; echo i; md ${a}da0 15;" \
	"echo e;md ${a}e00 9\0" \
 "pci1regs=setenv a e0008; run pcireg\0" \
 "pcierr=md ${a}e00 8; pci d.b $b.0 7 1; pci d.w $b.0 1e 1;" \
	"pci d.w $b.0 56 1\0" \
 "pcierrc=mw ${a}e00 ffffffff; pci w.b $b.0 7 ff; pci w.w $b.0 1e ffff;" \
	"pci w.w $b.0 56 ffff\0"	\
 "pci1err=setenv a e0008; run pcierr\0"	\
 "pci1errc=setenv a e0008; run pcierrc\0"
#else
#define	PCI_ENV ""
#endif

#if defined(CONFIG_PCIE1) || defined(CONFIG_PCIE2)
#define PCIE_ENV \
 "pciereg=md ${a}000 6; md ${a}020 4; md ${a}bf8 2; echo o;md ${a}c00 25;" \
	"echo i; md ${a}da0 15; echo e;md ${a}e00 e; echo d; md ${a}f00 c\0" \
 "pcie1regs=setenv a e000a; run pciereg\0"	\
 "pcie2regs=setenv a e0009; run pciereg\0"	\
 "pcieerr=md ${a}020 1; md ${a}e00; pci d.b $b.0 7 1; pci d.w $b.0 1e 1;"\
	"pci d.w $b.0 56 1; pci d $b.0 104 1; pci d $b.0 110 1;"	\
	"pci d $b.0 130 1\0" \
 "pcieerrc=mw ${a}020 ffffffff; mw ${a}e00 ffffffff; pci w.b $b.0 7 ff;"\
	"pci w.w $b.0 1e ffff; pci w.w $b.0 56 ffff; pci w $b.0 104 ffffffff;" \
	"pci w $b.0 110 ffffffff; pci w $b.0 130 ffffffff\0"		\
 "pciecfg=pci d $b.0 0 20; pci d $b.0 100 e; pci d $b.0 400 69\0"	\
 "pcie1err=setenv a e000a; run pcieerr\0"	\
 "pcie2err=setenv a e0009; run pcieerr\0"	\
 "pcie1errc=setenv a e000a; run pcieerrc\0"	\
 "pcie2errc=setenv a e0009; run pcieerrc\0"
#else
#define	PCIE_ENV ""
#endif

#define DMA_ENV \
 "dma0=mw ${d}104 ffffffff;mw ${d}110 50000;mw ${d}114 $sad0;mw ${d}118 50000;"\
	"mw ${d}120 $bc0;mw ${d}100 f03c404; mw ${d}11c $dad0; md ${d}100 9\0" \
 "dma1=mw ${d}184 ffffffff;mw ${d}190 50000;mw ${d}194 $sad1;mw ${d}198 50000;"\
	"mw ${d}1a0 $bc1;mw ${d}180 f03c404; mw ${d}19c $dad1; md ${d}180 9\0" \
 "dma2=mw ${d}204 ffffffff;mw ${d}210 50000;mw ${d}214 $sad2;mw ${d}218 50000;"\
	"mw ${d}220 $bc2;mw ${d}200 f03c404; mw ${d}21c $dad2; md ${d}200 9\0" \
 "dma3=mw ${d}284 ffffffff;mw ${d}290 50000;mw ${d}294 $sad3;mw ${d}298 50000;"\
	"mw ${d}2a0 $bc3;mw ${d}280 f03c404; mw ${d}29c $dad3; md ${d}280 9\0"

#ifdef ENV_DEBUG
#define	CONFIG_EXTRA_ENV_SETTINGS				\
 "netdev=eth0\0"						\
 "uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"				\
 "tftpflash=tftpboot $loadaddr $uboot; "			\
	"protect off " MK_STR(TEXT_BASE) " +$filesize; "	\
	"erase " MK_STR(TEXT_BASE) " +$filesize; "		\
	"cp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize; "	\
	"protect on " MK_STR(TEXT_BASE) " +$filesize; "	\
	"cmp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize\0"	\
 "consoledev=ttyS0\0"						\
 "ramdiskaddr=2000000\0"					\
 "ramdiskfile=8610hpcd/ramdisk.uboot\0"				\
 "fdtaddr=c00000\0"						\
 "fdtfile=8610hpcd/mpc8610_hpcd.dtb\0"				\
 "bdev=sda3\0"					\
 "en-wd=mw.b f8100010 0x08; echo -expect:- 08; md.b f8100010 1\0" \
 "dis-wd=mw.b f8100010 0x00; echo -expect:- 00; md.b f8100010 1\0" \
 "maxcpus=1"	\
 "eoi=mw e00400b0 0\0"						\
 "iack=md e00400a0 1\0"						\
 "ddrreg=md ${a}000 8; md ${a}080 8;md ${a}100 d; md ${a}140 4;" \
	"md ${a}bf0 4; md ${a}e00 3; md ${a}e20 3; md ${a}e40 7;" \
	"md ${a}f00 5\0" \
 "ddr1regs=setenv a e0002; run ddrreg\0" \
 "gureg=md ${a}000 2c; md ${a}0b0 1; md ${a}0c0 1; md ${a}800 1;" \
	"md ${a}900 6; md ${a}a00 1; md ${a}b20 3; md ${a}e00 1;" \
	"md ${a}e60 1; md ${a}ef0 1d\0" \
 "guregs=setenv a e00e0; run gureg\0" \
 "mcmreg=md ${a}000 1b; md ${a}bf8 2; md ${a}e00 5\0" \
 "mcmregs=setenv a e0001; run mcmreg\0" \
 "diuregs=md e002c000 1d\0" \
 "dium=mw e002c01c\0" \
 "diuerr=md e002c014 1\0" \
 "othbootargs=diufb=15M video=fslfb:1280x1024-32@60,monitor=0 debug\0" \
 "monitor=0-DVI\0" \
 "pmregs=md e00e1000 2b\0" \
 "lawregs=md e0000c08 4b\0" \
 "lbcregs=md e0005000 36\0" \
 "dma0regs=md e0021100 12\0" \
 "dma1regs=md e0021180 12\0" \
 "dma2regs=md e0021200 12\0" \
 "dma3regs=md e0021280 12\0" \
 PCI_ENV \
 PCIE_ENV \
 DMA_ENV
#else
#define CONFIG_EXTRA_ENV_SETTINGS                               \
 "netdev=eth0\0"                                                \
 "uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"                         \
 "consoledev=ttyS0\0"                                           \
 "ramdiskaddr=2000000\0"                                        \
 "ramdiskfile=8610hpcd/ramdisk.uboot\0"                         \
 "fdtaddr=c00000\0"                                             \
 "fdtfile=8610hpcd/mpc8610_hpcd.dtb\0"                          \
 "bdev=sda3\0"							\
 "othbootargs=diufb=15M video=fslfb:1280x1024-32@60,monitor=0\0"\
 "monitor=0-DVI\0"
#endif

#define CONFIG_NFSBOOTCOMMAND					\
 "setenv bootargs root=/dev/nfs rw "				\
	"nfsroot=$serverip:$rootpath "				\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs;"		\
 "tftp $loadaddr $bootfile;"					\
 "tftp $fdtaddr $fdtfile;"					\
 "bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND \
 "setenv bootargs root=/dev/ram rw "				\
	"console=$consoledev,$baudrate $othbootargs;"		\
 "tftp $ramdiskaddr $ramdiskfile;"				\
 "tftp $loadaddr $bootfile;"					\
 "tftp $fdtaddr $fdtfile;"					\
 "bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		\
 "setenv bootargs root=/dev/$bdev rw "	\
	"console=$consoledev,$baudrate $othbootargs;"	\
 "tftp $loadaddr $bootfile;"		\
 "tftp $fdtaddr $fdtfile;"		\
 "bootm $loadaddr - $fdtaddr"

#endif	/* __CONFIG_H */
