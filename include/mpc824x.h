/*
 * Copyright Rob Taylor, Flying Pig Systems Ltd. 2000.
 * Copyright (C) 2001, James Dougherty, jfd@cs.stanford.edu
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __MPC824X_H__
#define __MPC824X_H__

#include <config.h>

/* CPU Types */
#define CPU_TYPE_601		0x01		/* PPC 601	 CPU */
#define CPU_TYPE_602		0x02		/* PPC 602	 CPU */
#define CPU_TYPE_603		0x03		/* PPC 603	 CPU */
#define CPU_TYPE_603E		0x06		/* PPC 603e	 CPU */
#define CPU_TYPE_603P		0x07		/* PPC 603p	 CPU */
#define CPU_TYPE_604		0x04		/* PPC 604	 CPU */
#define CPU_TYPE_604E		0x09		/* PPC 604e	 CPU */
#define CPU_TYPE_604R		0x0a		/* PPC 604r	 CPU */
#define CPU_TYPE_750		0x08		/* PPC 750	 CPU */
#define CPU_TYPE_8240		0x81		/* PPC 8240	 CPU */
#define CPU_TYPE_8245		0x8081		/* PPC 8245/8241 CPU */
#define _CACHE_ALIGN_SIZE	32		/* cache line size */

/* spr976 - DMISS data tlb miss address register
 * spr977 - DCMP data tlb miss compare register
 * spr978 - HASH1 PTEG1 address register
 * spr980 - HASH2 PTEG2 address register
 * IMISS  - instruction tlb miss address register
 * ICMP	  - instruction TLB mis compare register
 * RPA	  - real page address register
 * HID0	  - hardware implemntation register
 * HID2	  - instruction address breakpoint register
 */

/* Kahlua/MPC8240 defines */
#define VEN_DEV_ID		0x00021057	/* Vendor and Dev. ID for MPC106 */
#define KAHLUA_ID		0x00031057	/* Vendor & Dev Id for Kahlua's PCI */
#define KAHLUA2_ID	        0x00061057      /* 8245 is aka Kahlua-2 */
#define BMC_BASE		0x80000000	/* Kahlua ID in PCI Memory space */
#define CHRP_REG_ADDR		0xfec00000	/* MPC107 Config, Map B */
#define CHRP_REG_DATA		0xfee00000	/* MPC107 Config, Map B */
#define CHRP_ISA_MEM_PHYS	0xfd000000
#define CHRP_ISA_MEM_BUS	0x00000000
#define CHRP_ISA_MEM_SIZE	0x01000000
#define CHRP_ISA_IO_PHYS	0xfe000000
#define CHRP_ISA_IO_BUS		0x00000000
#define CHRP_ISA_IO_SIZE	0x00800000
#define CHRP_PCI_IO_PHYS	0xfe800000
#define CHRP_PCI_IO_BUS		0x00800000
#define CHRP_PCI_IO_SIZE	0x00400000
#define CHRP_PCI_MEM_PHYS	0x80000000
#define CHRP_PCI_MEM_BUS	0x80000000
#define CHRP_PCI_MEM_SIZE	0x7d000000
#define	CHRP_PCI_MEMORY_PHYS	0x00000000
#define	CHRP_PCI_MEMORY_BUS	0x00000000
#define CHRP_PCI_MEMORY_SIZE	0x40000000
#define PREP_REG_ADDR		0x80000cf8	/* MPC107 Config, Map A */
#define PREP_REG_DATA		0x80000cfc	/* MPC107 Config, Map A */
#define PREP_ISA_IO_PHYS	0x80000000
#define PREP_ISA_IO_BUS		0x00000000
#define PREP_ISA_IO_SIZE	0x00800000
#define PREP_PCI_IO_PHYS	0x81000000
#define PREP_PCI_IO_BUS		0x01000000
#define PREP_PCI_IO_SIZE	0x3e800000
#define PREP_PCI_MEM_PHYS	0xc0000000
#define PREP_PCI_MEM_BUS	0x00000000
#define PREP_PCI_MEM_SIZE	0x3f000000
#define	PREP_PCI_MEMORY_PHYS	0x00000000
#define	PREP_PCI_MEMORY_BUS	0x80000000
#define	PREP_PCI_MEMORY_SIZE	0x80000000
#define MPC107_PCI_CMD		0x80000004	/* MPC107 PCI cmd reg */
#define MPC107_PCI_STAT 	0x80000006	/* MPC107 PCI status reg */
#define PROC_INT1_ADR		0x800000a8	/* MPC107 Processor i/f cfg1 */
#define PROC_INT2_ADR		0x800000ac	/* MPC107 Processor i/f cfg2 */
#define MEM_CONT1_ADR		0x800000f0	/* MPC107 Memory control config. 1 */
#define MEM_CONT2_ADR		0x800000f4	/* MPC107 Memory control config. 2 */
#define MEM_CONT3_ADR		0x800000f8	/* MPC107 Memory control config. 3 */
#define MEM_CONT4_ADR		0x800000fc	/* MPC107 Memory control config. 4 */
#define MEM_ERREN1_ADR		0x800000c0	/* MPC107 Memory error enable 1 */
#define MEM_START1_ADR		0x80000080	/* MPC107 Memory starting addr */
#define MEM_START2_ADR		0x80000084	/* MPC107 Memory starting addr-lo */
#define XMEM_START1_ADR 	0x80000088	/* MPC107 Extended mem. start addr-hi*/
#define XMEM_START2_ADR 	0x8000008c	/* MPC107 Extended mem. start addr-lo*/
#define MEM_END1_ADR		0x80000090	/* MPC107 Memory ending address */
#define MEM_END2_ADR		0x80000094	/* MPC107 Memory ending addr-lo */
#define XMEM_END1_ADR		0x80000098	/* MPC107 Extended mem. end addrs-hi */
#define XMEM_END2_ADR		0x8000009c	/* MPC107 Extended mem. end addrs-lo*/
#define OUT_DRV_CONT		0x80000073	/* MPC107 Output Driver Control reg */
#define MEM_EN_ADR		0x800000a0	/* Memory bank enable */
#define PAGE_MODE		0x800000a3	/* MPC107 Page Mode Counter/Timer */

/*-----------------------------------------------------------------------
 * Exception offsets (PowerPC standard)
 */
#define EXC_OFF_RESERVED0	0x0000	/* Reserved */
#define EXC_OFF_SYS_RESET	0x0100	/* System reset */
#define EXC_OFF_MACH_CHCK	0x0200	/* Machine Check */
#define EXC_OFF_DATA_STOR	0x0300	/* Data Storage */
#define EXC_OFF_INS_STOR	0x0400	/* Instruction Storage */
#define EXC_OFF_EXTERNAL	0x0500	/* External */
#define EXC_OFF_ALIGN		0x0600	/* Alignment */
#define EXC_OFF_PROGRAM		0x0700	/* Program */
#define EXC_OFF_FPUNAVAIL	0x0800	/* Floating-point Unavailable */
#define EXC_OFF_DECR		0x0900	/* Decrementer */
#define EXC_OFF_RESERVED1	0x0A00	/* Reserved */
#define EXC_OFF_RESERVED2	0x0B00	/* Reserved */
#define EXC_OFF_SYS_CALL	0x0C00	/* System Call */
#define EXC_OFF_TRACE		0x0D00	/* Trace */
#define EXC_OFF_FPUNASSIST	0x0E00	/* Floating-point Assist */

	/* 0x0E10 - 0x0FFF are marked reserved in The PowerPC Architecture book */
	/* these found in DINK code  - may not apply to 8240*/
#define EXC_OFF_PMI		0x0F00	/* Performance Monitoring Interrupt */
#define EXC_OFF_VMXUI		0x0F20	/* VMX (AltiVec) Unavailable Interrupt */

	/* 0x1000 - 0x2FFF are implementation specific */
	/* these found in DINK code  - may not apply to 8240 */
#define EXC_OFF_ITME		0x1000	/* Instruction Translation Miss Exception */
#define EXC_OFF_DLTME		0x1100	/* Data Load Translation Miss Exception */
#define EXC_OFF_DSTME		0x1200	/* Data Store Translation Miss Exception */
#define EXC_OFF_IABE		0x1300	/* Instruction Addr Breakpoint Exception */
#define EXC_OFF_SMIE		0x1400	/* System Management Interrupt Exception */
#define EXC_OFF_JMDDI		0x1600	/* Java Mode denorm detect Interr -- WTF??*/
#define EXC_OFF_RMTE		0x2000	/* Run Mode or Trace Exception */

#define _START_OFFSET		EXC_OFF_SYS_RESET

#define MAP_A_CONFIG_ADDR_HIGH	0x8000	/* Upper half of CONFIG_ADDR for Map A */
#define MAP_A_CONFIG_ADDR_LOW	0x0CF8	/* Lower half of CONFIG_ADDR for Map A */
#define MAP_A_CONFIG_DATA_HIGH	0x8000	/* Upper half of CONFIG_DAT for Map A */
#define MAP_A_CONFIG_DATA_LOW	0x0CFC	/* Lower half of CONFIG_DAT for Map A */
#define MAP_B_CONFIG_ADDR_HIGH	0xfec0	/* Upper half of CONFIG_ADDR for Map B */
#define MAP_B_CONFIG_ADDR_LOW	0x0000	/* Lower half of CONFIG_ADDR for Map B */
#define MAP_B_CONFIG_DATA_HIGH	0xfee0	/* Upper half of CONFIG_DAT for Map B */
#define MAP_B_CONFIG_DATA_LOW	0x0000	/* Lower half of CONFIG_DAT for Map B */


#if defined(CFG_ADDR_MAP_A)
#define CONFIG_ADDR_HIGH    MAP_A_CONFIG_ADDR_HIGH  /* Upper half of CONFIG_ADDR */
#define CONFIG_ADDR_LOW	    MAP_A_CONFIG_ADDR_LOW   /* Lower half of CONFIG_ADDR */
#define CONFIG_DATA_HIGH    MAP_A_CONFIG_DATA_HIGH  /* Upper half of CONFIG_DAT */
#define CONFIG_DATA_LOW	    MAP_A_CONFIG_DATA_LOW   /* Lower half of CONFIG_DAT */
#else /* Assume Map B, default */
#define CONFIG_ADDR_HIGH    MAP_B_CONFIG_ADDR_HIGH  /* Upper half of CONFIG_ADDR */
#define CONFIG_ADDR_LOW	    MAP_B_CONFIG_ADDR_LOW   /* Lower half of CONFIG_ADDR */
#define CONFIG_DATA_HIGH    MAP_B_CONFIG_DATA_HIGH  /* Upper half of CONFIG_DAT */
#define CONFIG_DATA_LOW	    MAP_B_CONFIG_DATA_LOW   /* Lower half of CONFIG_DAT */
#endif

#define CONFIG_ADDR	(CONFIG_ADDR_HIGH << 16 | CONFIG_ADDR_LOW)

#define CONFIG_DATA	(CONFIG_DATA_HIGH << 16 | CONFIG_DATA_LOW)

/* Macros to write to config registers. addr should be a constant in all cases */

#define CONFIG_WRITE_BYTE( addr, data ) \
  __asm__ __volatile__( \
  " stwbrx %1, 0, %0\n \
    sync\n \
    stb %3, %4(%2)\n \
    sync " \
  : /* no output */ \
  : "r" (CONFIG_ADDR), "r" ((addr) & ~3), \
    "b" (CONFIG_DATA), "r" (data), \
    "n" ((addr) & 3));

#define CONFIG_WRITE_HALFWORD( addr, data ) \
  __asm__ __volatile__( \
  " stwbrx %1, 0, %0\n \
    sync\n \
    sthbrx %3, %4, %2\n \
    sync " \
  : /* no output */ \
  : "r" (CONFIG_ADDR), "r" ((addr) & ~3), \
    "r" (CONFIG_DATA), "r" (data), \
    "b" ((addr) & 3));

/* this assumes it's writeing on word boundaries*/
#define CONFIG_WRITE_WORD( addr, data ) \
  __asm__ __volatile__( \
  " stwbrx %1, 0, %0\n \
    sync\n \
    stwbrx %3, 0, %2\n \
    sync " \
  : /* no output */ \
  : "r" (CONFIG_ADDR), "r" (addr), \
    "r" (CONFIG_DATA), "r" (data));

/* Configuration register reads*/

#define CONFIG_READ_BYTE( addr, reg ) \
  __asm__ ( \
  " stwbrx %1, 0, %2\n \
    sync\n \
    lbz	  %0, %4(%3)\n \
    sync " \
  : "=r" (reg) \
  : "r" ((addr) & ~3), "r" (CONFIG_ADDR), \
    "b" (CONFIG_DATA), "n" ((addr) & 3));


#define CONFIG_READ_HALFWORD( addr, reg ) \
  __asm__ ( \
  " stwbrx %1, 0, %2\n \
    sync\n \
    lhbrx %0, %4, %3\n \
    sync " \
  : "=r" (reg) \
  : "r" ((addr) & ~3), "r" (CONFIG_ADDR), \
    "r" (CONFIG_DATA), \
    "b" ((addr) & 3));

/* this assumes it's reading on word boundaries*/
#define CONFIG_READ_WORD( addr, reg ) \
  __asm__ ( \
  " stwbrx %1, 0, %2\n \
    sync\n \
    lwbrx %0, 0, %3\n \
    sync " \
  : "=r" (reg) \
  : "r" (addr), "r" (CONFIG_ADDR),\
    "r" (CONFIG_DATA));

/*
 *  configuration register 'addresses'.
 *  These are described in chaper 5 of the 8240 users manual.
 *  Where the register has an abreviation in the manual, this has
 *  been usaed here, otherwise a name in keeping with the norm has
 *  been invented.
 *  Note that some of these registers aren't documented in the manual.
 */

#define PCICR		0x80000004  /* PCI Command Register */
#define PCISR		0x80000006  /* PCI Status Register */
#define REVID		0x80000008  /* CPU revision id */
#define PIR		0x80000009  /* PCI Programming Interface Register */
#define PBCCR		0x8000000b  /* PCI Base Class Code Register */
#define PCLSR		0x8000000c  /* Processor Cache Line Size Register */
#define PLTR		0x8000000d  /* PCI Latancy Timer Register */
#define PHTR		0x8000000e  /* PCI Header Type Register */
#define BISTCTRL	0x8000000f  /* BIST Control */
#define LMBAR		0x80000010  /* Local Base Addres Register */
#define PCSRBAR		0x80000014  /* PCSR Base Address Register */
#define ILR		0x8000003c  /* PCI Interrupt Line Register */
#define IPR		0x8000003d  /* Interrupt Pin Register */
#define MINGNT		0x8000003e  /* MIN GNI */
#define MAXLAT		0x8000003f  /* MAX LAT */
#define PCIACR		0x80000046  /* PCI Arbiter Control Register */
#define PMCR1		0x80000070  /* Power management config. 1 */
#define PMCR2		0x80000072  /* Power management config. 2 */
#define ODCR		0x80000073  /* Output Driver Control Register */
#define CLKDCR		0x80000074  /* CLK Driver Control Register */
#if defined(CONFIG_MPC8245) || defined(CONFIG_MPC8241)
#define MIOCR1		0x80000076  /* Miscellaneous I/O Control Register 1 */
#define MIOCR2		0x80000077  /* Miscellaneous I/O Control Register 2 */
#endif
#define EUMBBAR		0x80000078  /* Embedded Utilities Memory Block Base Address Register */
#define EUMBBAR_VAL	0x80500000  /* PCI Relocation offset for EUMB region */
#define EUMBSIZE	0x00100000  /* Size of EUMB region */

#define MSAR1		0x80000080  /* Memory Starting Address Register 1 */
#define MSAR2		0x80000084  /* Memory Starting Address Register 2 */
#define EMSAR1		0x80000088  /* Extended Memory Starting Address Register 1*/
#define EMSAR2		0x8000008c  /* Extended Memory Starting Address Register 2*/
#define MEAR1		0x80000090  /* Memory Ending Address Register 1 */
#define MEAR2		0x80000094  /* Memory Ending Address Register 2 */
#define EMEAR1		0x80000098  /* Extended Memory Ending Address Register 1 */
#define EMEAR2		0x8000009c  /* Extended Memory Ending Address Register 2 */
#define MBER		0x800000a0  /* Memory bank Enable Register*/
#define MPMR		0x800000a3  /* Memory Page Mode Register (stores PGMAX) */
#define PICR1		0x800000a8  /* Processor Interface Configuration Register 1 */
#define PICR2		0x800000ac  /* Processor Interface Configuration Register 2 */
#define ECCSBECR	0x800000b8  /* ECC Single-Bit Error Counter Register */
#define ECCSBETR	0x800000b8  /* ECC Single-Bit Error Trigger Register */
#define ERRENR1		0x800000c0  /* Error Enableing Register 1 */
#define ERRENR2		0x800000c4  /* Error Enableing Register 2 */
#define ERRDR1		0x800000c1  /* Error Detection Register 1 */
#define IPBESR		0x800000c3  /* Internal Processor Error Status Register */
#define ERRDR2		0x800000c5  /* Error Detection Register 2 */
#define PBESR		0x800000c7  /* PCI Bus Error Status Register */
#define PBEAR		0x800000c8  /* Processor/PCI Bus Error Status Register */
#define AMBOR		0x800000e0  /* Address Map B Options Register */
#define PCMBCR		0x800000e1  /* PCI/Memory Buffer Configuration */
#define MCCR1		0x800000f0  /* Memory Control Configuration Register 1 */
#define MCCR2		0x800000f4  /* Memory Control Configuration Register 2 */
#define MCCR3		0x800000f8  /* Memory Control Configuration Register 3 */
#define MCCR4		0x800000fc  /* Memory Control Configuration Register 4 */

/* some values for some of the above */

#define PICR1_CF_APARK		0x00000008
#define PICR1_LE_MODE		0x00000020
#define PICR1_ST_GATH_EN	0x00000040
#if defined(CONFIG_MPC8240)
#define PICR1_EN_PCS		0x00000080 /* according to dink code, sets the 8240 to handle pci config space */
#elif defined(CONFIG_MPC8245) || defined(CONFIG_MPC8241)
#define PICR1_NO_BUSW_CK	0x00000080 /* no bus width check for flash writes */
#define PICR1_DEC		0x00000100 /* Time Base enable on 8245/8241 */
#define ERCR1		        0x800000d0  /* Extended ROM Configuration Register 1 */
#define ERCR2		        0x800000d4  /* Extended ROM Configuration Register 2 */
#define ERCR3		        0x800000d8  /* Extended ROM Configuration Register 3 */
#define ERCR4		        0x800000dc  /* Extended ROM Configuration Register 4 */
#define MIOCR1		        0x80000076  /* Miscellaneous I/O Control Register 1 */
#define MIOCR1_ADR_X	        0x80000074  /* Miscellaneous I/O Control Register 1 */
#define MIOCR1_SHIFT	        2
#define MIOCR2		        0x80000077  /* Miscellaneous I/O Control Register 2 */
#define MIOCR2_ADR_X	        0x80000074  /* Miscellaneous I/O Control Register 1 */
#define MIOCR2_SHIFT	        3
#define ODCR_ADR_X	        0x80000070	/* Output Driver Control register */
#define ODCR_SHIFT              3
#define PMCR2_ADR	        0x80000072	/* Power Mgmnt Cfg 2 register */
#define PMCR2_ADR_X	        0x80000070
#define PMCR2_SHIFT             3
#define PMCR1_ADR	        0x80000070	/* Power Mgmnt Cfg 1 reister */
#else
#error Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif
#define PICR1_CF_DPARK		0x00000200
#define PICR1_MCP_EN		0x00000800
#define PICR1_FLASH_WR_EN	0x00001000
#ifdef CONFIG_MPC8240
#define PICR1_ADDRESS_MAP	0x00010000
#define PIRC1_MSK		0xff000000
#endif
#define PICR1_PROC_TYPE_MSK	0x00060000
#define PICR1_PROC_TYPE_603E	0x00040000
#define PICR1_RCS0		0x00100000

#define PICR2_CF_SNOOP_WS_MASK	0x000c0000
#define PICR2_CF_SNOOP_WS_0WS	0x00000000
#define PICR2_CF_SNOOP_WS_1WS	0x00040000
#define PICR2_CF_SNOOP_WS_2WS	0x00080000
#define PICR2_CF_SNOOP_WS_3WS	0x000c0000
#define PICR2_CF_APHASE_WS_MASK 0x0000000c
#define PICR2_CF_APHASE_WS_0WS	0x00000000
#define PICR2_CF_APHASE_WS_1WS	0x00000004
#define PICR2_CF_APHASE_WS_2WS	0x00000008
#define PICR2_CF_APHASE_WS_3WS	0x0000000c

#define MCCR1_ROMNAL_SHIFT	28
#define MCCR1_ROMNAL_MSK	0xf0000000
#define MCCR1_ROMFAL_SHIFT	23
#define MCCR1_ROMFAL_MSK	0x0f800000
#define MCCR1_DBUS_SIZE0        0x00400000
#define MCCR1_BURST		0x00100000
#define MCCR1_MEMGO		0x00080000
#define MCCR1_SREN		0x00040000
#if defined(CONFIG_MPC8240)
#define MCCR1_RAM_TYPE		0x00020000
#elif defined(CONFIG_MPC8245) || defined(CONFIG_MPC8241)
#define MCCR1_SDRAM_EN		0x00020000
#else
#error Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif
#define MCCR1_PCKEN		0x00010000
#define MCCR1_BANK1ROW_SHIFT	2
#define MCCR1_BANK2ROW_SHIFT	4
#define MCCR1_BANK3ROW_SHIFT	6
#define MCCR1_BANK4ROW_SHIFT	8
#define MCCR1_BANK5ROW_SHIFT	10
#define MCCR1_BANK6ROW_SHIFT	12
#define MCCR1_BANK7ROW_SHIFT	14

#define MCCR2_TS_WAIT_TIMER_MSK 0xe0000000
#define MCCR2_TS_WAIT_TIMER_SHIFT 29
#define MCCR2_ASRISE_MSK	0x1e000000
#define MCCR2_ASRISE_SHIFT	25
#define MCCR2_ASFALL_MSK	0x01e00000
#define MCCR2_ASFALL_SHIFT	21

#define MCCR2_INLINE_PAR_NOT_ECC    0x00100000
#define MCCR2_WRITE_PARITY_CHK	0x00080000
#define MCCR2_INLFRD_PARECC_CHK_EN  0x00040000
#ifdef CONFIG_MPC8240
#define MCCR2_ECC_EN		0x00020000
#define MCCR2_EDO		0x00010000
#endif
#define MCCR2_REFINT_MSK	0x0000fffc
#define MCCR2_REFINT_SHIFT	2
#define MCCR2_RSV_PG		0x00000002
#define MCCR2_PMW_PAR		0x00000001

#define MCCR3_BSTOPRE2TO5_MSK	0xf0000000 /*BSTOPRE[2-5]*/
#define MCCR3_BSTOPRE2TO5_SHIFT 28
#define MCCR3_REFREC_MSK	0x0f000000
#define MCCR3_REFREC_SHIFT	24
#ifdef CONFIG_MPC8240
#define MCCR3_RDLAT_MSK		0x00f00000
#define MCCR3_RDLAT_SHIFT	20
#define MCCR3_CPX		0x00010000
#define MCCR3_RAS6P_MSK		0x00078000
#define MCCR3_RAS6P_SHIFT	15
#define MCCR3_CAS5_MSK		0x00007000
#define MCCR3_CAS5_SHIFT	12
#define MCCR3_CP4_MSK		0x00000e00
#define MCCR3_CP4_SHIFT		9
#define MCCR3_CAS3_MSK		0x000001c0
#define MCCR3_CAS3_SHIFT	6
#define MCCR3_RCD2_MSK		0x00000038
#define MCCR3_RCD2_SHIFT	3
#define MCCR3_RP1_MSK		0x00000007
#define MCCR3_RP1_SHIFT		0
#endif

#define MCCR4_PRETOACT_MSK	0xf0000000
#define MCCR4_PRETOACT_SHIFT	28
#define MCCR4_ACTTOPRE_MSK	0x0f000000
#define MCCR4_ACTTOPRE_SHIFT	24
#define MCCR4_WMODE		0x00800000
#define MCCR4_INLINE		0x00400000
#if defined(CONFIG_MPC8240)
#define MCCR4_BIT21		0x00200000 /* this include cos DINK code sets it- unknown function*/
#elif defined(CONFIG_MPC8245) || defined(CONFIG_MPC8241)
#define MCCR4_EXTROM		0x00200000 /* enables Extended ROM space */
#else
#error Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif
#define MCCR4_REGISTERED	0x00100000
#define MCCR4_BSTOPRE0TO1_MSK	0x000c0000 /*BSTOPRE[0-1]*/
#define MCCR4_BSTOPRE0TO1_SHIFT 18
#define MCCR4_REGDIMM		0x00008000
#define MCCR4_SDMODE_MSK	0x00007f00
#define MCCR4_SDMODE_SHIFT	8
#define MCCR4_ACTTORW_MSK	0x000000f0
#define MCCR4_ACTTORW_SHIFT	4
#define MCCR4_BSTOPRE6TO9_MSK	0x0000000f /*BSTOPRE[6-9]*/
#define MCCR4_BSTOPRE6TO9_SHIFT 0
#define MCCR4_DBUS_SIZE2_SHIFT	17

#define MICR_ADDR_MASK		0x0ff00000
#define MICR_ADDR_SHIFT		20
#define MICR_EADDR_MASK		0x30000000
#define MICR_EADDR_SHIFT	28

#define BATU_BEPI_MSK		0xfffe0000
#define BATU_BL_MSK		0x00001ffc

#define BATU_BL_128K		0x00000000
#define BATU_BL_256K		0x00000004
#define BATU_BL_512K		0x0000000c
#define BATU_BL_1M		0x0000001c
#define BATU_BL_2M		0x0000003c
#define BATU_BL_4M		0x0000007c
#define BATU_BL_8M		0x000000fc
#define BATU_BL_16M		0x000001fc
#define BATU_BL_32M		0x000003fc
#define BATU_BL_64M		0x000007fc
#define BATU_BL_128M		0x00000ffc
#define BATU_BL_256M		0x00001ffc

#define BATU_VS			0x00000002
#define BATU_VP			0x00000001

#define BATL_BRPN_MSK		0xfffe0000
#define BATL_WIMG_MSK		0x00000078

#define BATL_WRITETHROUGH	0x00000040
#define BATL_CACHEINHIBIT	0x00000020
#define BATL_MEMCOHERENCE	0x00000010
#define BATL_GUARDEDSTORAGE	0x00000008

#define BATL_PP_MSK		0x00000003
#define BATL_PP_00		0x00000000 /* No access */
#define BATL_PP_01		0x00000001 /* Read-only */
#define BATL_PP_10		0x00000002 /* Read-write */
#define BATL_PP_11		0x00000003

/*
 * I'd attempt to do defines for the PP bits, but it's use is a bit
 * too complex, see the PowerPC Operating Environment Architecture
 * section in the PowerPc arch book, chapter 4.
 */

/*eumb and epic config*/

#define EPIC_FPR		0x00041000
#define EPIC_GCR		0x00041020
#define EPIC_EICR		0x00041030
#define EPIC_EVI		0x00041080
#define EPIC_PI			0x00041090
#define EPIC_SVR		0x000410E0
#define EPIC_TFRR		0x000410F0

/*
 * Note the information for these is rather mangled in the 8240 manual.
 * These are guesses.
 */

#define EPIC_GTCCR0		0x00041100
#define EPIC_GTCCR1		0x00041140
#define EPIC_GTCCR2		0x00041180
#define EPIC_GTCCR3		0x000411C0
#define EPIC_GTBCR0		0x00041110
#define EPIC_GTBCR1		0x00041150
#define EPIC_GTBCR2		0x00041190
#define EPIC_GTBCR3		0x000411D0
#define EPIC_GTVPR0		0x00041120
#define EPIC_GTVPR1		0x00041160
#define EPIC_GTVPR2		0x000411a0
#define EPIC_GTVPR3		0x000411e0
#define EPIC_GTDR0		0x00041130
#define EPIC_GTDR1		0x00041170
#define EPIC_GTDR2		0x000411b0
#define EPIC_GTDR3		0x000411f0

#define EPIC_IVPR0		0x00050200
#define EPIC_IVPR1		0x00050220
#define EPIC_IVPR2		0x00050240
#define EPIC_IVPR3		0x00050260
#define EPIC_IVPR4		0x00050280

#define EPIC_SVPR0		0x00050200
#define EPIC_SVPR1		0x00050220
#define EPIC_SVPR2		0x00050240
#define EPIC_SVPR3		0x00050260
#define EPIC_SVPR4		0x00050280
#define EPIC_SVPR5		0x000502A0
#define EPIC_SVPR6		0x000502C0
#define EPIC_SVPR7		0x000502E0
#define EPIC_SVPR8		0x00050300
#define EPIC_SVPR9		0x00050320
#define EPIC_SVPRa		0x00050340
#define EPIC_SVPRb		0x00050360
#define EPIC_SVPRc		0x00050380
#define EPIC_SVPRd		0x000503A0
#define EPIC_SVPRe		0x000503C0
#define EPIC_SVPRf		0x000503E0

/* MPC8240 Byte Swap/PCI Support Macros */
#define BYTE_SWAP_16_BIT(x)    ( (((x) & 0x00ff) << 8) | ( (x) >> 8) )
#define LONGSWAP(x) ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8)|\
		     (((x) & 0x00ff0000) >>  8) | (((x) & 0xff000000) >> 24) )
#define PCISWAP(x)   LONGSWAP(x)

#ifndef __ASSEMBLY__

/*
 * MPC107 Support
 *
 */
unsigned int mpc824x_mpc107_getreg(unsigned int regNum);
void mpc824x_mpc107_setreg(unsigned int regNum, unsigned int regVal);
void mpc824x_mpc107_write8(unsigned int address, unsigned char data);
void mpc824x_mpc107_write16(unsigned int address, unsigned short data);
void mpc824x_mpc107_write32(unsigned int address, unsigned int data);
unsigned char mpc824x_mpc107_read8(unsigned int address);
unsigned short mpc824x_mpc107_read16(unsigned int address);
unsigned int mpc824x_mpc107_read32(unsigned int address);
unsigned int mpc824x_eummbar_read(unsigned int regNum);
void mpc824x_eummbar_write(unsigned int regNum, unsigned int regVal);

#ifdef CONFIG_PCI
struct pci_controller;
void pci_cpm824x_init(struct pci_controller* hose);
#endif

#endif /* __ASSEMBLY__ */

#endif /* __MPC824X_H__ */
