/*
 * (C) Copyright 2004
 * Elmeg Communications Systems GmbH, Juergen Selent (j.selent@elmeg.de)
 *
 * Support for the Elmeg VoVPN Gateway Module
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* define cpu used */
#define	CONFIG_MPC8272			1

/* define busmode: 8260 */
#undef	CONFIG_BUSMODE_60x

#define	CONFIG_SYS_TEXT_BASE		0xfff00000

/* system clock rate (CLKIN) - equal to the 60x and local bus speed */
#ifdef	CONFIG_CLKIN_66MHz
#define	CONFIG_8260_CLKIN		66666666	/* in Hz */
#else
#define	CONFIG_8260_CLKIN		100000000	/* in Hz */
#endif

/* call board_early_init_f */
#define	CONFIG_BOARD_EARLY_INIT_F	1

/* have misc_init_r() function */
#define CONFIG_MISC_INIT_R		1

/* have reset_phy_r() function */
#define CONFIG_RESET_PHY_R		1

/* have special reset function */
#define	CONFIG_HAVE_OWN_RESET		1

/* allow serial and ethaddr to be overwritten */
#define	CONFIG_ENV_OVERWRITE

/* watchdog disabled */
#undef	CONFIG_WATCHDOG

/* include support for bzip2 compressed images */
#undef	CONFIG_BZIP2

/* status led */
#undef	CONFIG_STATUS_LED		/* XXX jse */

/* vendor parameter protection */
#define CONFIG_ENV_OVERWRITE

/*
 * select serial console configuration
 *
 * if either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 */
#define	CONFIG_CONS_ON_SMC
#undef	CONFIG_CONS_ON_SCC
#undef	CONFIG_CONS_NONE
#define	CONFIG_CONS_INDEX		1

/* serial port default baudrate */
#define CONFIG_BAUDRATE			115200

/* echo on for serial download	*/
#define CONFIG_LOADS_ECHO		1

/* don't allow baudrate change	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE

/*
 * select ethernet configuration
 *
 * if either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected, then
 * CONFIG_ETHER_INDEX must be set to the channel number (1-4 for SCC, 1-3
 * for FCC)
 *
 * if CONFIG_ETHER_NONE is defined, then either the ethernet routines must be
 * defined elsewhere (as for the console), or CONFIG_CMD_NET must be unset.
 */
#undef	CONFIG_ETHER_ON_SCC
#define	CONFIG_ETHER_ON_FCC
#undef	CONFIG_ETHER_NONE

#ifdef	CONFIG_ETHER_ON_FCC

/* which SCC/FCC channel for ethernet */
#define	CONFIG_ETHER_INDEX		1

/* Marvell Switch SMI base addr */
#define CONFIG_SYS_PHY_ADDR			0x10

/* FCC1 RMII REFCLK is CLK10 */
#define CONFIG_SYS_CMXFCR_VALUE		CMXFCR_TF1CS_CLK10
#define CONFIG_SYS_CMXFCR_MASK			(CMXFCR_FC1|CMXFCR_TF1CS_MSK)

/* BDs and buffers on 60x bus */
#define CONFIG_SYS_CPMFCR_RAMTYPE		0

/* Local Protect, Full duplex, Flowcontrol, RMII */
#define CONFIG_SYS_FCC_PSMR			(FCC_PSMR_LPB|FCC_PSMR_FDE|\
					 FCC_PSMR_FCE|FCC_PSMR_RMII)

/* bit-bang MII PHY management	*/
#define CONFIG_BITBANGMII

#define MDIO_PORT			1		/* Port B */

#define MDIO_DECLARE		volatile ioport_t *iop = ioport_addr ( \
					(immap_t *) CONFIG_SYS_IMMR, MDIO_PORT )
#define MDC_DECLARE		MDIO_DECLARE

#define CONFIG_SYS_MDIO_PIN			0x00002000	/* PB18 */
#define CONFIG_SYS_MDC_PIN			0x00001000	/* PB19 */
#define MDIO_ACTIVE			(iop->pdir |=  CONFIG_SYS_MDIO_PIN)
#define MDIO_TRISTATE			(iop->pdir &= ~CONFIG_SYS_MDIO_PIN)
#define MDIO_READ			((iop->pdat &  CONFIG_SYS_MDIO_PIN) != 0)
#define MDIO(bit)			if(bit) iop->pdat |=  CONFIG_SYS_MDIO_PIN; \
					else	iop->pdat &= ~CONFIG_SYS_MDIO_PIN
#define MDC(bit)			if(bit) iop->pdat |=  CONFIG_SYS_MDC_PIN; \
					else	iop->pdat &= ~CONFIG_SYS_MDC_PIN
#define MIIDELAY			udelay(1)

#endif

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

#define CONFIG_CMD_BDI
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
#define CONFIG_CMD_IMLS
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_SOURCE


/*
 * boot options & environment
 */
#define CONFIG_BOOTDELAY		3
#define CONFIG_BOOTCOMMAND		"run flash_self"
#undef  CONFIG_BOOTARGS
#define	CONFIG_EXTRA_ENV_SETTINGS	\
"clean_nv=erase fff20000 ffffffff\0" \
"update_boss=tftp 100000 PPC/logic157.bin; protect off fff00000 ffffffff; erase fff00000 ffffffff; cp.b 100000 fff00000 ${filesize}; tftp 100000 PPC/bootmon157.bin; cp.b 100000 fff20000 ${filesize}\0" \
"update_lx=tftp 100000 ${kernel}; erase ${kernel_addr} ffefffff; cp.b 100000 ${kernel_addr} ${filesize}\0" \
"update_fs=tftp 100000 ${fs}.${fstype}; erase ff840000 ffdfffff; cp.b 100000 ff840000 ${filesize}\0" \
"update_ub=tftp 100000 ${uboot}; protect off fff00000 fff1ffff; erase fff00000 fff1ffff; cp.b 100000 fff00000 ${filesize}; protect off ff820000 ff83ffff; erase ff820000 ff83ffff\0" \
"flashargs=setenv bootargs root=${rootdev} rw rootfstype=${fstype}\0" \
"nfsargs=setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath}\0" \
"addip=setenv bootargs ${bootargs} ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}:${netdev}:off\0" \
"addmisc=setenv bootargs ${bootargs} console=${console},${baudrate} ethaddr=${ethaddr} panic=1\0" \
"net_nfs=tftpboot 400000 ${kernel}; run nfsargs addip addmisc; bootm\0" \
"net_self=tftpboot 400000 ${kernel}; run flashargs addmisc; bootm\0" \
"flash_self=run flashargs addmisc; bootm ${kernel_addr}\0" \
"flash_nfs=run nfsargs addip addmisc; bootm ${kernel_addr}\0" \
"fstype=cramfs\0" \
"rootpath=/root_fs\0" \
"uboot=PPC/u-boot.bin\0" \
"kernel=PPC/uImage\0" \
"kernel_addr=ffe00000\0" \
"fs=PPC/root_fs\0" \
"console=ttyS0\0" \
"netdev=eth0\0" \
"rootdev=31:3\0" \
"ethaddr=00:09:4f:01:02:03\0" \
"ipaddr=10.0.0.201\0" \
"netmask=255.255.255.0\0" \
"serverip=10.0.0.136\0" \
"gatewayip=10.0.0.10\0" \
"hostname=bastard\0" \
""


/*
 * miscellaneous configurable options
 */

/* undef to save memory */
#define	CONFIG_SYS_LONGHELP

/* monitor command prompt */

/* console i/o buffer size */
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE			1024
#else
#define	CONFIG_SYS_CBSIZE			256
#endif

/* print buffer size */
#define	CONFIG_SYS_PBSIZE			(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/* max number of command args */
#define	CONFIG_SYS_MAXARGS			16

/* boot argument buffer size */
#define CONFIG_SYS_BARGSIZE			CONFIG_SYS_CBSIZE

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START		0x00100000
/* 1 ... 15 MB in DRAM */
#define CONFIG_SYS_MEMTEST_END			0x00f00000
/* full featured memtest */
#define CONFIG_SYS_ALT_MEMTEST

/* default load address */
#define	CONFIG_SYS_LOAD_ADDR			0x00100000

/* decrementer freq: 1 ms ticks	*/

/* configure flash */
#define CONFIG_SYS_FLASH_BASE			0xff800000
#define CONFIG_SYS_MAX_FLASH_BANKS		1
#define CONFIG_SYS_MAX_FLASH_SECT		64
#define CONFIG_SYS_FLASH_SIZE			8
#undef	CONFIG_SYS_FLASH_16BIT
#define CONFIG_SYS_FLASH_ERASE_TOUT		240000
#define CONFIG_SYS_FLASH_WRITE_TOUT		500
#define CONFIG_SYS_FLASH_LOCK_TOUT		500
#define CONFIG_SYS_FLASH_UNLOCK_TOUT		10000
#define CONFIG_SYS_FLASH_PROTECTION

/* monitor in flash */
#define CONFIG_SYS_MONITOR_OFFSET		0x00700000

/* environment in flash */
#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + 0x00020000)
#define CONFIG_ENV_SIZE			0x00020000
#define CONFIG_ENV_SECT_SIZE		0x00020000

/*
 * Initial memory map for linux
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ			(8 << 20)

/* hard reset configuration words */
#ifdef	CONFIG_CLKIN_66MHz
#define CONFIG_SYS_HRCW_MASTER			0x04643050
#else
#error NO HRCW FOR 100MHZ SPECIFIED !!!
#endif
#define CONFIG_SYS_HRCW_SLAVE1			0x00000000
#define CONFIG_SYS_HRCW_SLAVE2			0x00000000
#define CONFIG_SYS_HRCW_SLAVE3			0x00000000
#define CONFIG_SYS_HRCW_SLAVE4			0x00000000
#define CONFIG_SYS_HRCW_SLAVE5			0x00000000
#define CONFIG_SYS_HRCW_SLAVE6			0x00000000
#define CONFIG_SYS_HRCW_SLAVE7			0x00000000

/* internal memory mapped register */
#define CONFIG_SYS_IMMR			0xF0000000

/* definitions for initial stack pointer and data area (in DPRAM) */
#define CONFIG_SYS_INIT_RAM_ADDR		CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE		0x2000
#define CONFIG_SYS_GBL_DATA_OFFSET		(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET		CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE			0x00000000
#define CONFIG_SYS_SDRAM_SIZE			(32*1024*1024)
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_FLASH		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_OFFSET)
#define CONFIG_SYS_MONITOR_LEN			0x00020000
#define CONFIG_SYS_MALLOC_LEN			0x00020000

/* cache configuration */
#define CONFIG_SYS_CACHELINE_SIZE		32      /* for MPC8260 */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT		5	/* log base 2 of above */
#endif

/*
 * HIDx - Hardware Implementation-dependent Registers
 *-----------------------------------------------------------------------
 * HID0 also contains cache control - initially enable both caches and
 * invalidate contents, then the final state leaves only the instruction
 * cache enabled. Note that Power-On and Hard reset invalidate the caches,
 * but Soft reset does not.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CONFIG_SYS_HID0_INIT			(HID0_ICE|HID0_DCE|\
					 HID0_ICFI|HID0_DCI|HID0_IFEM|HID0_ABE)
#define CONFIG_SYS_HID0_FINAL			(HID0_IFEM|HID0_ABE)
#define CONFIG_SYS_HID2			0

/* RMR - reset mode register - turn on checkstop reset enable */
#define CONFIG_SYS_RMR				RMR_CSRE

/* BCR - bus configuration */
#define CONFIG_SYS_BCR				0x00000000

/* SIUMCR - siu module configuration */
#define CONFIG_SYS_SIUMCR			0x4905c000

/* SYPCR - system protection control */
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR			0xffffff87
#else
#define CONFIG_SYS_SYPCR			0xffffff83
#endif

/* TMCNTSC - time counter status and control */
/* clear interrupts XXX jse */
/*#define CONFIG_SYS_TMCNTSC			(TMCNTSC_SEC|TMCNTSC_ALR) */
#define CONFIG_SYS_TMCNTSC			(TMCNTSC_SEC|TMCNTSC_ALR|\
					 TMCNTSC_TCF|TMCNTSC_TCE)

/* PISCR - periodic interrupt status and control */
/* clear interrupts XXX jse */
/*#define CONFIG_SYS_PISCR			(PISCR_PS) */
#define CONFIG_SYS_PISCR			(PISCR_PS|PISCR_PTF|PISCR_PTE)

/* SCCR - system clock control */
#define CONFIG_SYS_SCCR			0x000001a9

/* RCCR - risc controller configuration */
#define CONFIG_SYS_RCCR			0

/*
 * MEMORY MAP
 * ----------
 * CS0 - FLASH    8MB/8Bit	base=0xff800000 (boot: 0xfe000000, 8x mirrored)
 * CS1 - SDRAM   32MB/64Bit	base=0x00000000
 * CS2 - DSP/SL1  1MB/16Bit	base=0xf0100000
 * CS3 - DSP/SL2  1MB/16Bit	base=0xf0200000
 * CS4 - DSP/SL3  1MB/16Bit	base=0xf0300000
 * CS5 - DSP/SL4  1MB/16Bit	base=0xf0400000
 * CS7 - DPRAM    1KB/8Bit	base=0xf0500000, size=32KB (32x mirrored)
 *  x  - IMMR     384KB		base=0xf0000000
 */
/* XXX jse 100MHz TODO */
#define CONFIG_SYS_BR0_PRELIM			0xff800801
#define CONFIG_SYS_OR0_PRELIM			0xff801e44
#define CONFIG_SYS_BR1_PRELIM			0x00000041
#define CONFIG_SYS_OR1_PRELIM			0xfe002ec0
#if 1
#define CONFIG_SYS_BR2_PRELIM			0xf0101001
#define CONFIG_SYS_OR2_PRELIM			0xfff00ef4
#define CONFIG_SYS_BR3_PRELIM			0xf0201001
#define CONFIG_SYS_OR3_PRELIM			0xfff00ef4
#define CONFIG_SYS_BR4_PRELIM			0xf0301001
#define CONFIG_SYS_OR4_PRELIM			0xfff00ef4
#define CONFIG_SYS_BR5_PRELIM			0xf0401001
#define CONFIG_SYS_OR5_PRELIM			0xfff00ef4
#else
#define CONFIG_SYS_BR2_PRELIM			0xf0101081
#define CONFIG_SYS_OR2_PRELIM			0xfff00104
#define CONFIG_SYS_BR3_PRELIM			0xf0201081
#define CONFIG_SYS_OR3_PRELIM			0xfff00104
#define CONFIG_SYS_BR4_PRELIM			0xf0301081
#define CONFIG_SYS_OR4_PRELIM			0xfff00104
#define CONFIG_SYS_BR5_PRELIM			0xf0401081
#define CONFIG_SYS_OR5_PRELIM			0xfff00104
#endif
#define CONFIG_SYS_BR7_PRELIM			0xf0500881
#define CONFIG_SYS_OR7_PRELIM			0xffff8104
#define CONFIG_SYS_MPTPR			0x2700
#define CONFIG_SYS_PSDMR			0x822a2452	/* optimal */
/*#define CONFIG_SYS_PSDMR			0x822a48a3 */	/* relaxed */
#define CONFIG_SYS_PSRT			0x1a

/* "bad" address */
#define	CONFIG_SYS_RESET_ADDRESS		0x40000000

#endif	/* __CONFIG_H */
