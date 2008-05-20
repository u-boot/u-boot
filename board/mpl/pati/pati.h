/*
 * (C) Copyright 2003
 * Denis Peter, d.peter@mpl.ch
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
/************************************************************************
 * MACROS and register definitions for PATI Registers
 ************************************************************************/
#ifndef __PATI_H_
#define __PATI_H_	1

#define PLD_PART_ID		0x0
#define PLD_BOARD_TIMING	0x4
#define PLD_CONF_REG1		0x8
#define PLD_CONF_REG2		0xC
#define PLD_CONF_RES		0x10

#define SET_REG_BIT(y,x) (y<<(31-x))
#define GET_REG_BIT(y,x) ((y>>(31-x)) & 0x1L)

/* SDRAM Controller PLD_PART_ID */
/* 9  10 11 12 13 14 19 31 */
#define SDRAM_PART3	9
#define SDRAM_PART2	10
#define SDRAM_PART1	11
#define SDRAM_PART0	12
#define SDRAM_ID3	13
#define SDRAM_ID2	14
#define SDRAM_ID1	19
#define SDRAM_ID0	31

#define SDRAM_PART(x)	(	\
	(GET_REG_BIT(x,SDRAM_PART3)<<3) |\
	(GET_REG_BIT(x,SDRAM_PART2)<<2) |\
	(GET_REG_BIT(x,SDRAM_PART1)<<1) |\
	(GET_REG_BIT(x,SDRAM_PART0)))

#define SDRAM_ID(x)	(	\
	(GET_REG_BIT(x,SDRAM_ID3)<<3) |\
	(GET_REG_BIT(x,SDRAM_ID2)<<2) |\
	(GET_REG_BIT(x,SDRAM_ID1)<<1) |\
	(GET_REG_BIT(x,SDRAM_ID0)))

/* System Controller */
/* 0  1 3 4 5 16 20 28 29 30 */
#define SYSCNTR_PART4	0
#define SYSCNTR_PART3	1
#define SYSCNTR_PART2	3
#define SYSCNTR_PART1	4
#define SYSCNTR_PART0	5
#define SYSCNTR_ID4	16
#define SYSCNTR_ID3	20
#define SYSCNTR_ID2	28
#define SYSCNTR_ID1	29
#define SYSCNTR_ID0	30

#define SYSCNTR_PART(x)	(	\
	(GET_REG_BIT(x,SYSCNTR_PART4)<<4) |\
	(GET_REG_BIT(x,SYSCNTR_PART3)<<3) |\
	(GET_REG_BIT(x,SYSCNTR_PART2)<<2) |\
	(GET_REG_BIT(x,SYSCNTR_PART1)<<1) |\
	(GET_REG_BIT(x,SYSCNTR_PART0)))

#define SYSCNTR_ID(x)	(	\
	(GET_REG_BIT(x,SYSCNTR_ID4)<<4) |\
	(GET_REG_BIT(x,SYSCNTR_ID3)<<3) |\
	(GET_REG_BIT(x,SYSCNTR_ID2)<<2) |\
	(GET_REG_BIT(x,SYSCNTR_ID1)<<1) |\
	(GET_REG_BIT(x,SYSCNTR_ID0)))

/* SDRAM Controller PLD_BOARD_TIMING */
/* 9  10 11 12 13 14 19 31 */
#define SDRAM_CAL	9
#define SDRAM_RCD	10
#define SDRAM_WREQ	11
#define SDRAM_PR	12
#define SDRAM_RC	13
#define SDRAM_LMR	14
#define SDRAM_IIP	19
#define SDRAM_RES0	31
/* System Controller */
/* 0  1 3 4 5 16 20 28 29 30 */
#define SYSCNTR_BREV0	0
#define SYSCNTR_BREV1	1
#define SYSCNTR_BREV2	3
#define SYSCNTR_BREV3	4
#define SYSCNTR_RES0	5
#define SYSCNTR_RES1	16
#define SYSCNTR_RES2	20
#define SYSCNTR_FLWAIT2	28
#define SYSCNTR_FLWAIT1	29
#define SYSCNTR_FLWAIT0	30

#define SYSCNTR_BREV(x)	(	\
	(GET_REG_BIT(x,SYSCNTR_BREV3)<<3) |\
	(GET_REG_BIT(x,SYSCNTR_BREV2)<<2) |\
	(GET_REG_BIT(x,SYSCNTR_BREV1)<<1) |\
	(GET_REG_BIT(x,SYSCNTR_BREV0)))

#define GET_SYSCNTR_FLWAIT(x)	(	\
	(GET_REG_BIT(x,SYSCNTR_FLWAIT2)<<2) |\
	(GET_REG_BIT(x,SYSCNTR_FLWAIT1)<<1) |\
	(GET_REG_BIT(x,SYSCNTR_FLWAIT0)))

#define SET_SYSCNTR_FLWAIT(x)	(	\
	(SET_REG_BIT(((x & 0x04)!=0),SYSCNTR_FLWAIT2)) |\
	(SET_REG_BIT(((x & 0x02)!=0)x,SYSCNTR_FLWAIT1)) |\
	(SET_REG_BIT(((x & 0x01)!=0)x,SYSCNTR_FLWAIT0)))

/* SDRAM Controller REG 2*/
/* 9  10 11 12 13 14 19 31 */
#define SDRAM_MUX0	9
#define SDRAM_MUX1	10
#define SDRAM_PDIS	11
#define SDRAM_RES1	12
#define SDRAM_RES2	13
#define SDRAM_RES3	14
#define SDRAM_RES4	19
#define SDRAM_RIP	31

#define GET_SDRAM_MUX(x)	(	\
	(GET_REG_BIT(x,SDRAM_MUX1)<<1)| \
	(GET_REG_BIT(x,SDRAM_MUX0)))


/* System Controller */
/* 0  1 3 4 5 16 20 28 29 30 */
#define SYSCNTR_FLAG	0
#define SYSCNTR_IP	1
#define SYSCNTR_BIND2	3
#define SYSCNTR_BIND1	4
#define SYSCNTR_BIND0	5
#define SYSCNTR_PRM	16
#define SYSCNTR_ICW	20
#define SYSCNTR_ISB2	28
#define SYSCNTR_ISB1	29
#define SYSCNTR_ISB0	30

#define GET_SYSCNTR_BOOTIND(x)	(	\
	(GET_REG_BIT(x,SYSCNTR_BIND2)<<2) |\
	(GET_REG_BIT(x,SYSCNTR_BIND1)<<1) |\
	(GET_REG_BIT(x,SYSCNTR_BIND0)))

#define SET_SYSCNTR_BOOTIND(x)	(	\
	(SET_REG_BIT(((x & 0x04)!=0),SYSCNTR_BIND2)) |\
	(SET_REG_BIT(((x & 0x02)!=0)x,SYSCNTR_BIND1))| \
	(SET_REG_BIT(((x & 0x01)!=0)x,SYSCNTR_BIND0)))

#define GET_SYSCNTR_ISB(x)	(	\
	(GET_REG_BIT(x,SYSCNTR_ISB2)<<2)| \
	(GET_REG_BIT(x,SYSCNTR_ISB1)<<1)| \
	(GET_REG_BIT(x,SYSCNTR_ISB0)))

#define SET_SYSCNTR_ISB(x)	(	\
	(SET_REG_BIT(((x & 0x04)!=0),SYSCNTR_ISB2))| \
	(SET_REG_BIT(((x & 0x02)!=0)x,SYSCNTR_ISB))| \
	(SET_REG_BIT(((x & 0x01)!=0)x,SYSCNTR_ISB0)))

/* SDRAM Controller REG 3*/
/* 9  10 11 12 13 14 19 31 */
#define SDRAM_RES5	9
#define SDRAM_CFG1	10
#define SDRAM_CFG2	11
#define SDRAM_CFG3	12
#define SDRAM_RES6	13
#define SDRAM_CFG5	14
#define SDRAM_CFG6	19
#define SDRAM_RES7	31

#define GET_SDRAM_CFG(x)	(	\
	(GET_REG_BIT(x,SDRAM_CFG6)<<4) |\
	(GET_REG_BIT(x,SDRAM_CFG5)<<3) |\
	(GET_REG_BIT(x,SDRAM_CFG3)<<2) |\
	(GET_REG_BIT(x,SDRAM_CFG2)<<1) |\
	(GET_REG_BIT(x,SDRAM_CFG1)))

/* System Controller */
/* 0  1 3 4 5 16 20 28 29 30 */
#define SYSCNTR_BDIS	0
#define SYSCNTR_PCIM	1
#define SYSCNTR_CFG0	3
#define SYSCNTR_CFG1	4
#define SYSCNTR_CFG2	5
#define SYSCNTR_CFG3	16
#define SYSCNTR_BOOTEN	20
#define SYSCNTR_CPU_VPP	28
#define SYSCNTR_FL_VPP	29
#define SYSCNTR_FL_WP	30

#define GET_SYSCNTR_CFG(x)	(	\
	(GET_REG_BIT(x,SYSCNTR_CFG3)<<3)| \
	(GET_REG_BIT(x,SYSCNTR_CFG2)<<2)| \
	(GET_REG_BIT(x,SYSCNTR_CFG1)<<1)| \
	(GET_REG_BIT(x,SYSCNTR_CFG0)))


/***************************************************************
 * MISC Defines
 ***************************************************************/

#define PCI_VENDOR_ID_MPL	0x18E6
#define PCI_DEVICE_ID_PATI	0x00DA

#if defined(CONFIG_MIP405)
#define PATI_FIRMWARE_START_OFFSET	0x00300000
#define PATI_ISO_STRING  "MEV-10084-001"
#endif

#define PATI_ENDIAN_MODE	0x3E

/*******************************************
 * PATI Mapping:
 * -------------
 * PCI Map:
 * -------
 * All addreses are mapped into the memory area
 * (IO Area on some areas may also be possible)
 * - pci_cfg_mem_base: fixed address to the PLX config area size 512Bytes
 * - pci_space0_addr:  configurable
 * - pci_space1_addr	  configurable
 *
 * Local Map:
 * ----------
 * Local addresses (Remap)
 * - SDRAM      0x06000000 Size 16MByte mask 0xff000000
 * - EPLD CFG   0x07000000 Size 512Bytes
 * - FLASH      0x03000000 Size up to 8MByte
 * - CPU        0x01000000 Size 4MByte (only accessable if special configured)
 *
 * Implemention:
 * -------------
 * To prevent using large resources reservation on the host following
 * PCI mapping is choosed:
 * - pci_cfg_mem_base: fixed address to the PLX config area size 512Bytes
 * - pci_space0_addr:  configured to the EPLD Config Area size 256Bytes
 * - pci_space1_addr:  configured to the SDRAM Area size 1MBytes, this
 *                     space is used to switch between SDRAM, Flash and CPU
 *
 */

/* Attribute definitions */
#define PATI_BUS_SIZE_8		0
#define PATI_BUS_SIZE_16	1
#define PATI_BUS_SIZE_32	3

#define PATI_SPACE0_MASK	(0xFEFFFE00)  /* Mask Attributes */
#define PATI_SPACE1_MASK	(0x00000000)  /* Mask Attributes */

#define PATI_EXTRA_LONG_EEPROM	1

#define SPACE0_TA_ENABLE (1<<6)
#define SPACE1_TA_ENABLE (1<<6)

/* Config Area */
#define PATI_LOC_CFG_ADDR		0x07000000		/* Local Address */
#define PATI_LOC_CFG_MASK		0xFFFFFF00		/* 256 Bytes */
/* Attributes */
#define PATI_LOC_CFG_BUS_SIZE		PATI_BUS_SIZE_32	/* 32 Bit */
#define PATI_LOC_CFG_BURST		0			/* No Burst */
#define PATI_LOC_CFG_NO_PREFETCH	1			/* No Prefetch */
#define PATI_LOC_CFG_TA_ENABLE		1			/* Enable TA */

#define PATI_LOC_CFG_SPACE0_ATTR  ( \
		PATI_LOC_CFG_BUS_SIZE | \
		(PATI_LOC_CFG_TA_ENABLE << 6) | \
		(PATI_LOC_CFG_NO_PREFETCH << 8) | \
		(PATI_LOC_CFG_BURST << 24) | \
		(PATI_EXTRA_LONG_EEPROM << 25))

/* should never be used */
#define PATI_LOC_CFG_SPACE1_ATTR  ( \
		PATI_LOC_CFG_BUS_SIZE | \
		(PATI_LOC_CFG_TA_ENABLE << 6) | \
		(PATI_LOC_CFG_NO_PREFETCH << 9) | \
		(PATI_LOC_CFG_BURST << 8))


/* SDRAM Area */
#define PATI_LOC_SDRAM_ADDR		0x06000000		/* Local Address */
#define PATI_LOC_SDRAM_MASK		0xFFF00000		/* 1MByte */
/* Attributes */
#define PATI_LOC_SDRAM_BUS_SIZE		PATI_BUS_SIZE_32	/* 32 Bit */
#define PATI_LOC_SDRAM_BURST		0			/* No Burst */
#define PATI_LOC_SDRAM_NO_PREFETCH	0			/* Prefetch */
#define PATI_LOC_SDRAM_TA_ENABLE	1			/* Enable TA */

/* should never be used */
#define PATI_LOC_SDRAM_SPACE0_ATTR  ( \
		PATI_LOC_SDRAM_BUS_SIZE | \
		(PATI_LOC_SDRAM_TA_ENABLE << 6) | \
		(PATI_LOC_SDRAM_NO_PREFETCH << 8) | \
		(PATI_LOC_SDRAM_BURST << 24) | \
		(PATI_EXTRA_LONG_EEPROM << 25))

#define PATI_LOC_SDRAM_SPACE1_ATTR  ( \
		PATI_LOC_SDRAM_BUS_SIZE | \
		(PATI_LOC_SDRAM_TA_ENABLE << 6) | \
		(PATI_LOC_SDRAM_NO_PREFETCH << 9) | \
		(PATI_LOC_SDRAM_BURST << 8))


/* Flash Area */
#define PATI_LOC_FLASH_ADDR		0x03000000		/* Local Address */
#define PATI_LOC_FLASH_MASK		0xFFF00000		/* 1MByte */
/* Attributes */
#define PATI_LOC_FLASH_BUS_SIZE		PATI_BUS_SIZE_16	/* 16 Bit */
#define PATI_LOC_FLASH_BURST		0			/* No Burst */
#define PATI_LOC_FLASH_NO_PREFETCH	1			/* No Prefetch */
#define PATI_LOC_FLASH_TA_ENABLE	1			/* Enable TA */

/* should never be used */
#define PATI_LOC_FLASH_SPACE0_ATTR  ( \
		PATI_LOC_FLASH_BUS_SIZE | \
		(PATI_LOC_FLASH_TA_ENABLE << 6) | \
		(PATI_LOC_FLASH_NO_PREFETCH << 8) | \
		(PATI_LOC_FLASH_BURST << 24) | \
		(PATI_EXTRA_LONG_EEPROM << 25))

#define PATI_LOC_FLASH_SPACE1_ATTR  ( \
		PATI_LOC_FLASH_BUS_SIZE | \
		(PATI_LOC_FLASH_TA_ENABLE << 6) | \
		(PATI_LOC_FLASH_NO_PREFETCH << 9) | \
		(PATI_LOC_FLASH_BURST << 8))


/* CPU Area */
#define PATI_LOC_CPU_ADDR		0x01000000		/* Local Address */
#define PATI_LOC_CPU_MASK		0xFFF00000		/* 1Mbyte */
/* Attributes */
#define PATI_LOC_CPU_BUS_SIZE		PATI_BUS_SIZE_32	/* 32 Bit */
#define PATI_LOC_CPU_BURST		0			/* No Burst */
#define PATI_LOC_CPU_NO_PREFETCH	1			/* No Prefetch */
#define PATI_LOC_CPU_TA_ENABLE		1			/* Enable TA */

/* should never be used */
#define PATI_LOC_CPU_SPACE0_ATTR  ( \
		PATI_LOC_CPU_BUS_SIZE | \
		(PATI_LOC_CPU_TA_ENABLE << 6) | \
		(PATI_LOC_CPU_NO_PREFETCH << 8) | \
		(PATI_LOC_CPU_BURST << 24) | \
		(PATI_EXTRA_CPU_EEPROM << 25))

#define PATI_LOC_CPU_SPACE1_ATTR  ( \
		PATI_LOC_CPU_BUS_SIZE | \
		(PATI_LOC_CPU_TA_ENABLE << 6) | \
		(PATI_LOC_CPU_NO_PREFETCH << 9) | \
		(PATI_LOC_CPU_BURST << 8))

/***************************************************
 * Hardware Config word definition
 ***************************************************/
#define BOOT_EXT_FLASH		0x00000000
#define BOOT_INT_FLASH		0x00000004
#define BOOT_FROM_PCI		0x00000006
#define BOOT_FROM_SDRAM		0x00000005

#define ENABLE_INT_ARB		0x00000008

#define INITIAL_IRQ_PREF	0x00000010

#define INITIAL_MEM_0M		0x00000000
#define INITIAL_MEM_4M		0x00000080
#define INITIAL_MEM_8M		0x00000040
#define INITIAL_MEM_12M		0x000000C0
#define INITIAL_MEM_16M		0x00000020
#define INITIAL_MEM_20M		0x000000A0
#define INITIAL_MEM_24M		0x00000060
#define INITIAL_MEM_28M		0x000000E0
/* CONF */
#define INTERNAL_HWCONF		0x00000100
/* PRPM */
#define LOCAL_CPU_SLAVE		0x00000200
/* BDIS */
#define DISABLE_MEM_CNTR	0x00000400
/* PCIM */
#define PCI_MASTER_ONLY		0x00000800


#define PATI_HW_START		((BOOT_EXT_FLASH | INITIAL_MEM_28M | INITIAL_IRQ_PREF))
#define PATI_HW_PCI_ONLY	((BOOT_EXT_FLASH | INITIAL_MEM_28M | INITIAL_IRQ_PREF | PCI_MASTER_ONLY))
#define PATI_HW_CPU_ACC		((BOOT_EXT_FLASH | INITIAL_MEM_12M | INITIAL_IRQ_PREF | PCI_MASTER_ONLY))
#define PATI_HW_CPU_SLAVE	((BOOT_EXT_FLASH | INITIAL_MEM_12M | INITIAL_IRQ_PREF | PCI_MASTER_ONLY | LOCAL_CPU_SLAVE))

/***************************************************
 * Direct Master Config
 ***************************************************/
#define PATI_DMASTER_PCI_ADDR		0x01000000
#define PATI_BUS_MASTER 1


#define PATI_DMASTER_MASK		0xFFF00000  /* 1MByte */
#define PATI_DMASTER_ADDR		0x01000000  /* Local Address */

#define PATI_DMASTER_MEMORY_EN		0x00000001 /* 0x00000001 */
#define PATI_DMASTER_READ_AHEAD		0x00000004 /* 0x00000004 */
#define PATI_DMASTER_READ_NOT_AHEAD	0x00000000 /* 0x00000004 */
#define PATI_DMASTER_PRE_SIZE_CNTRL_0	0x00000000
#define PATI_DMASTER_PRE_SIZE_CNTRL_4	0x00000008
#define PATI_DMASTER_PRE_SIZE_CNTRL_8	0x00001000
#define PATI_DMASTER_PRE_SIZE_CNTRL_16	0x00001008
#define PATI_DMASTER_REL_PCI		0x00000000
#define PATI_DMASTER_NOT_REL_PCI	0x00000010
#define PATI_DMASTER_WR_INVAL		0x00000200
#define PATI_DMASTER_NOT_WR_INVAL	0x00000000
#define PATI_DMASTER_PRE_LIMIT		0x00000800
#define PATI_DMASTER_PRE_CONT		0x00000000
#define PATI_DMASTER_DELAY_WR_0		0x00000000
#define PATI_DMASTER_DELAY_WR_4		0x00004000
#define PATI_DMASTER_DELAY_WR_8		0x00008000
#define PATI_DMASTER_DELAY_WR_16	0x0000C000

#define PATI_DMASTER_PCI_ADDR_MASK	0xFFFF0000

#define PATI_DMASTER_ATTR	\
	PATI_DMASTER_MEMORY_EN | \
	PATI_DMASTER_READ_AHEAD | \
	PATI_DMASTER_PRE_SIZE_CNTRL_4 | \
	PATI_DMASTER_REL_PCI | \
	PATI_DMASTER_NOT_WR_INVAL | \
	PATI_DMASTER_PRE_LIMIT | \
	PATI_DMASTER_DELAY_WR_0


#endif /* #ifndef __PATI_H_ */
