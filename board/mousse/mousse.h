/*
 * MOUSSE/MPC8240 Board definitions.
 * For more info, see http://www.vooha.com/
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001
 * James Dougherty (jfd@cs.stanford.edu)
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef __MOUSSE_H
#define __MOUSSE_H

/* System addresses */

#define PCI_SPECIAL_BASE	0xfe000000
#define PCI_SPECIAL_SIZE	0x01000000

/* PORTX Device Addresses for Mousse */

#define PORTX_DEV_BASE		0xff000000
#define PORTX_DEV_SIZE		0x01000000

#define ENET_DEV_BASE		0x80000000

#define PLD_REG_BASE		(PORTX_DEV_BASE | 0xe09000)
#define PLD_REG(off)		(*(volatile unsigned char *) \
				 (PLD_REG_BASE + (off)))

#define PLD_REVID_B1		0x7f
#define PLD_REVID_B2		0x01

/* MPLD */
#define SYS_HARD_RESET()	{ for (;;) PLD_REG(0) = 0; } /* clr 0x80 bit */
#define SYS_REVID_GET()		((int) PLD_REG(0) & 0x7f)
#define SYS_LED_OFF()		(PLD_REG(1) |= 0x80)
#define SYS_LED_ON()		(PLD_REG(1) &= ~0x80)
#define SYS_WATCHDOG_IRQ3()	(PLD_REG(2) |= 0x80)
#define SYS_WATCHDOG_RESET()	(PLD_REG(2) &= ~0x80)
#define SYS_TOD_PROTECT()	(PLD_REG(3) |= 0x80)
#define SYS_TOD_UNPROTECT()	(PLD_REG(3) &= ~0x80)

/* SGS M48T59Y */
#define TOD_BASE		(PORTX_DEV_BASE | 0xe0a000)
#define TOD_REG_BASE		(TOD_BASE | 0x1ff0)
#define TOD_NVRAM_BASE		TOD_BASE
#define TOD_NVRAM_SIZE		0x1ff0
#define TOD_NVRAM_LIMIT		(TOD_NVRAM_BASE + TOD_NVRAM_SIZE)

/* NS16552 SIO */
#define SERIAL_BASE(_x)		(PORTX_DEV_BASE | 0xe08000 | ((_x) ? 0 : 0x80))
#define N_SIO_CHANNELS		2
#define N_COM_PORTS		N_SIO_CHANNELS

/*
 * On-board Dec21143 PCI Ethernet
 * Note: The PCI MBAR chosen here was used from MPC8240UM which states
 * that PCI memory is at: 0x80000 - 0xFDFFFFFF, if AMBOR[CPU_FD_ALIAS]
 * is set, then PCI memory maps 1-1 with this address range in the
 * correct byte order.
 */
#define PCI_ENET_IOADDR		0x80000000
#define PCI_ENET_MEMADDR	0x80000000

/*
 * Flash Memory Layout
 *
 *    2 MB Flash Bank 0 runs in 8-bit mode.  In Flash Bank 0, the 32 kB
 *    sector SA3 is obscured by the 32 kB serial/TOD access space, and
 *    the 64 kB sectors SA19-SA26 are obscured by the 512 kB PLCC
 *    containing the fixed boot ROM.  (If the 512 kB PLCC is
 *    deconfigured by jumper, this window to Flash Bank 0 becomes
 *    visible, but it still contains the fixed boot code and should be
 *    considered read-only).  Flash Bank 0 sectors SA0 (16 kB), SA1 (8
 *    kB), and SA2 (8 kB) are currently unused.
 *
 *    2 MB Flash Bank 1 runs in 16-bit mode.  Flash Bank 1 is fully
 *    usable, but it's a 16-bit wide device on a 64-bit bus.  Therefore
 *    16-bit words only exist at addresses that are multiples of 8.  All
 *    PROM data and control addresses must be multiplied by 8.
 *
 *    See flashMap.c for description of flash filesystem layout.
 */

/*
 * FLASH memory address space: 8-bit wide FLASH memory spaces.
 */
#define FLASH0_SEG0_START	0xffe00000	 /* Baby 32Kb segment */
#define FLASH0_SEG0_END		0xffe07fff	 /* 16 kB + 8 kB + 8 kB */
#define FLASH0_SEG0_SIZE	0x00008000	 /*   (sectors SA0-SA2) */

#define FLASH0_SEG1_START	0xffe10000	 /* 1MB - 64Kb FLASH0 seg */
#define FLASH0_SEG1_END		0xffefffff	 /* 960 kB */
#define FLASH0_SEG1_SIZE	0x000f0000

#define FLASH0_SEG2_START	0xfff00000	 /* Boot Loader stored here */
#define FLASH0_SEG2_END		0xfff7ffff	 /* 512 kB FLASH0/PLCC seg */
#define FLASH0_SEG2_SIZE	0x00080000

#define FLASH0_SEG3_START	0xfff80000	 /* 512 kB FLASH0 seg */
#define FLASH0_SEG3_END		0xffffffff
#define FLASH0_SEG3_SIZE	0x00080000

/* Where Kahlua starts */
#define FLASH_RESET_VECT	0xfff00100

/*
 * CHRP / PREP (MAP A/B) definitions.
 */

#define PREP_REG_ADDR		0x80000cf8	/* MPC107 Config, Map A */
#define PREP_REG_DATA		0x80000cfc	/* MPC107 Config, Map A */
/* MPC107 (MPC8240 internal EUMBBAR mapped) */
#define CHRP_REG_ADDR		0xfec00000	/* MPC106 Config, Map B */
#define CHRP_REG_DATA		0xfee00000	/* MPC106 Config, Map B */

/*
 * Mousse PCI IDSEL Assignments (Device Number)
 */
#define MOUSSE_IDSEL_ENET	13		/* On-board 21143 Ethernet */
#define MOUSSE_IDSEL_LPCI	14		/* On-board PCI slot */
#define MOUSSE_IDSEL_82371	15		/* That other thing */
#define MOUSSE_IDSEL_CPCI2	31		/* CPCI slot 2 */
#define MOUSSE_IDSEL_CPCI3	30		/* CPCI slot 3 */
#define MOUSSE_IDSEL_CPCI4	29		/* CPCI slot 4 */
#define MOUSSE_IDSEL_CPCI5	28		/* CPCI slot 5 */
#define MOUSSE_IDSEL_CPCI6	27		/* CPCI slot 6 */

/*
 * Mousse Interrupt Mapping:
 *
 *	IRQ1	Enet (intA|intB|intC|intD)
 *	IRQ2	CPCI intA (See below)
 *	IRQ3	Local PCI slot intA|intB|intC|intD
 *	IRQ4	COM1 Serial port (Actually higher addressed port on duart)
 *
 * PCI Interrupt Mapping in CPCI chassis:
 *
 *		   |	       CPCI Slot
 *		   | 1 (CPU)	2	3	4	5	6
 *	-----------+--------+-------+-------+-------+-------+-------+
 *	  intA	   |	X		X		X
 *	  intB	   |		X		X		X
 *	  intC	   |	X		X		X
 *	  intD	   |		X		X		X
 */


#define EPIC_VECTOR_EXT0	0
#define EPIC_VECTOR_EXT1	1
#define EPIC_VECTOR_EXT2	2
#define EPIC_VECTOR_EXT3	3
#define EPIC_VECTOR_EXT4	4
#define EPIC_VECTOR_TM0		16
#define EPIC_VECTOR_TM1		17
#define EPIC_VECTOR_TM2		18
#define EPIC_VECTOR_TM3		19
#define EPIC_VECTOR_I2C		20
#define EPIC_VECTOR_DMA0	21
#define EPIC_VECTOR_DMA1	22
#define EPIC_VECTOR_I2O		23


#define INT_VEC_IRQ0		0
#define INT_NUM_IRQ0		INT_VEC_IRQ0
#define MOUSSE_IRQ_ENET		EPIC_VECTOR_EXT1	/* Hardwired */
#define MOUSSE_IRQ_CPCI		EPIC_VECTOR_EXT2	/* Hardwired */
#define MOUSSE_IRQ_LPCI		EPIC_VECTOR_EXT3	/* Hardwired */
#define MOUSSE_IRQ_DUART	EPIC_VECTOR_EXT4	/* Hardwired */

/* Onboard DEC 21143 Ethernet */
#define PCI_ENET_MEMADDR	0x80000000
#define PCI_ENET_IOADDR		0x80000000

/* Some other PCI device */
#define PCI_SLOT_MEMADDR	0x81000000
#define PCI_SLOT_IOADDR		0x81000000

/* Promise ATA66 PCI Device (ATA controller) */
#define PROMISE_MBAR0  0xa0000000
#define PROMISE_MBAR1  (PROMISE_MBAR0 + 0x1000)
#define PROMISE_MBAR2  (PROMISE_MBAR0 + 0x2000)
#define PROMISE_MBAR3  (PROMISE_MBAR0 + 0x3000)
#define PROMISE_MBAR4  (PROMISE_MBAR0 + 0x4000)
#define PROMISE_MBAR5  (PROMISE_MBAR0 + 0x5000)

/* ATA/66 Controller offsets */
#define CONFIG_SYS_ATA_BASE_ADDR     PROMISE_MBAR0
#define CONFIG_SYS_IDE_MAXBUS	       2 /* ide0/ide1 */
#define CONFIG_SYS_IDE_MAXDEVICE      2 /* 2 drives per controller */
#define CONFIG_SYS_ATA_IDE0_OFFSET    0
#define CONFIG_SYS_ATA_IDE1_OFFSET    0x3000
/*
 * Definitions for accessing IDE controller registers
 */
#define CONFIG_SYS_ATA_DATA_OFFSET    0
#define CONFIG_SYS_ATA_REG_OFFSET     0
#define CONFIG_SYS_ATA_ALT_OFFSET    (0x1000)

/*
 * The constants ROM_TEXT_ADRS, ROM_SIZE, RAM_HIGH_ADRS, and RAM_LOW_ADRS
 * are defined in config.h and Makefile.
 * All definitions for these constants must be identical.
 */
#define ROM_BASE_ADRS		0xfff00000	/* base address of ROM */
#define ROM_TEXT_ADRS		(ROM_BASE_ADRS+0x0100) /* with PC & SP */
#define ROM_WARM_ADRS		(ROM_TEXT_ADRS+0x0004) /* warm reboot entry */
#define ROM_SIZE		0x00080000	/* 512KB ROM space */
#define RAM_LOW_ADRS		0x00010000   /* RAM address for vxWorks */
#define RAM_HIGH_ADRS		0x00c00000   /* RAM address for bootrom */

/*
 *  NVRAM configuration
 *  NVRAM is implemented via the SGS Thomson M48T59Y
 *  64Kbit (8Kbx8) Timekeeper SRAM.
 *  This 8KB NVRAM also has a TOD. See m48t59y.{h,c} for more information.
 */

#define NV_RAM_ADRS		TOD_NVRAM_BASE
#define NV_RAM_INTRVL		1
#define NV_RAM_WR_ENBL		SYS_TOD_UNPROTECT()
#define NV_RAM_WR_DSBL		SYS_TOD_PROTECT()

#define NV_OFF_BOOT0		0x0000	/* Boot string 0 (256b) */
#define NV_OFF_BOOT1		0x0100	/* Boot string 1 (256b) */
#define NV_OFF_BOOT2		0x0200	/* Boot string 2 (256b)*/
#define NV_OFF_MACADDR		0x0400	/* 21143 MAC address (6b) */
#define NV_OFF_ACTIVEBOOT	0x0406	/* Active boot string, 0 to 2 (1b) */
#define NV_OFF_UNUSED1		0x0407	/* Unused (1b) */
#define NV_OFF_BINDFIX		0x0408	/* See sysLib.c:sysBindFix() (1b) */
#define NV_OFF_UNUSED2		0x0409	/* Unused (7b) */
#define NV_OFF_TIMEZONE		0x0410	/* TIMEZONE env var (64b) */
#define NV_OFF_VXWORKS_END	0x07FF	/* 2047 VxWorks Total */
#define NV_OFF_U_BOOT		0x0800	/* 2048 U-Boot boot-loader */
#define NV_OFF_U_BOOT_ADDR	(TOD_BASE + NV_OFF_U_BOOT) /* sysaddr*/
#define NV_U_BOOT_ENV_SIZE	2048	/* 2K - U-Boot Total */
#define NV_OFF__next_free	(NV_U_BOOT_ENVSIZE +1)
#define NV_RAM_SIZE		8176	/* NVRAM End */

#endif /* __MOUSSE_H */
