/*----------------------------------------------------------------------------+
|       This source code is dual-licensed.  You may use it under the terms of
|       the GNU General Public License version 2, or under the license below.
|
|       This source code has been made available to you by IBM on an AS-IS
|       basis.  Anyone receiving this source is licensed under IBM
|       copyrights to use it in any way he or she deems fit, including
|       copying it, modifying it, compiling it, and redistributing it either
|       with or without modifications.  No license under IBM patents or
|       patent applications is to be implied by the copyright license.
|
|       Any user of this software should understand that IBM cannot provide
|       technical support for this software and will not be responsible for
|       any consequences resulting from the use of this software.
|
|       Any person who transfers this source code or any derivative work
|       must include the IBM copyright notice, this paragraph, and the
|       preceding two paragraphs in the transferred software.
|
|       COPYRIGHT   I B M   CORPORATION 1999
|       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
+----------------------------------------------------------------------------*/

#ifndef	__PPC4XX_H__
#define __PPC4XX_H__

/*
 * Configure which SDRAM/DDR/DDR2 controller is equipped
 */
#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_405EP) || \
	defined(CONFIG_AP1000) || defined(CONFIG_ML2)
#define CONFIG_SDRAM_PPC4xx_IBM_SDRAM	/* IBM SDRAM controller */
#endif

#if defined(CONFIG_440GP) || defined(CONFIG_440GX) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR)
#define CONFIG_SDRAM_PPC4xx_IBM_DDR	/* IBM DDR controller */
#endif

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define CONFIG_SDRAM_PPC4xx_DENALI_DDR2	/* Denali DDR(2) controller */
#endif

#if defined(CONFIG_405EX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT) || \
    defined(CONFIG_460SX)
#define CONFIG_SDRAM_PPC4xx_IBM_DDR2	/* IBM DDR(2) controller */
#endif

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) ||	\
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) ||	\
    defined(CONFIG_405EZ) || defined(CONFIG_405EX) ||	\
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define CONFIG_NAND_NDFC
#endif

/* PLB4 CrossBar Arbiter Core supported across PPC4xx families */
#if defined(CONFIG_405EX) || \
    defined(CONFIG_440EP) || defined(CONFIG_440EPX) || \
    defined(CONFIG_440GR) || defined(CONFIG_440GRX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)  || \
    defined(CONFIG_460SX)

#define PLB_ARBITER_BASE		0x80

#define plb0_revid			(PLB_ARBITER_BASE + 0x00)
#define plb0_acr			(PLB_ARBITER_BASE + 0x01)
#define plb0_acr_ppm_mask		0xF0000000
#define plb0_acr_ppm_fixed		0x00000000
#define plb0_acr_ppm_fair		0xD0000000
#define plb0_acr_hbu_mask		0x08000000
#define plb0_acr_hbu_disabled		0x00000000
#define plb0_acr_hbu_enabled		0x08000000
#define plb0_acr_rdp_mask		0x06000000
#define plb0_acr_rdp_disabled		0x00000000
#define plb0_acr_rdp_2deep		0x02000000
#define plb0_acr_rdp_3deep		0x04000000
#define plb0_acr_rdp_4deep		0x06000000
#define plb0_acr_wrp_mask		0x01000000
#define plb0_acr_wrp_disabled		0x00000000
#define plb0_acr_wrp_2deep		0x01000000

#define plb0_besrl			(PLB_ARBITER_BASE + 0x02)
#define plb0_besrh			(PLB_ARBITER_BASE + 0x03)
#define plb0_bearl			(PLB_ARBITER_BASE + 0x04)
#define plb0_bearh			(PLB_ARBITER_BASE + 0x05)
#define plb0_ccr			(PLB_ARBITER_BASE + 0x08)

#define plb1_acr			(PLB_ARBITER_BASE + 0x09)
#define plb1_acr_ppm_mask		0xF0000000
#define plb1_acr_ppm_fixed		0x00000000
#define plb1_acr_ppm_fair		0xD0000000
#define plb1_acr_hbu_mask		0x08000000
#define plb1_acr_hbu_disabled		0x00000000
#define plb1_acr_hbu_enabled		0x08000000
#define plb1_acr_rdp_mask		0x06000000
#define plb1_acr_rdp_disabled		0x00000000
#define plb1_acr_rdp_2deep		0x02000000
#define plb1_acr_rdp_3deep		0x04000000
#define plb1_acr_rdp_4deep		0x06000000
#define plb1_acr_wrp_mask		0x01000000
#define plb1_acr_wrp_disabled		0x00000000
#define plb1_acr_wrp_2deep		0x01000000

#define plb1_besrl			(PLB_ARBITER_BASE + 0x0A)
#define plb1_besrh			(PLB_ARBITER_BASE + 0x0B)
#define plb1_bearl			(PLB_ARBITER_BASE + 0x0C)
#define plb1_bearh			(PLB_ARBITER_BASE + 0x0D)

#endif /* 440EP/EPX 440GR/GRX 440SP/SPE 460EX/GT/SX 405EX*/

#if defined(CONFIG_440)
/*
 * Enable long long (%ll ...) printf format on 440 PPC's since most of
 * them support 36bit physical addressing
 */
#define CONFIG_SYS_64BIT_VSPRINTF
#define CONFIG_SYS_64BIT_STRTOUL
#include <ppc440.h>
#else
#include <ppc405.h>
#endif

#include <asm/ppc4xx-sdram.h>
#include <asm/ppc4xx-ebc.h>
#if !defined(CONFIG_XILINX_440)
#include <asm/ppc4xx-uic.h>
#endif

/*
 * Macro for generating register field mnemonics
 */
#define	PPC_REG_BITS		32
#define	PPC_REG_VAL(bit, value)	((value) << ((PPC_REG_BITS - 1) - (bit)))

/*
 * Elide casts when assembling register mnemonics
 */
#ifndef __ASSEMBLY__
#define	static_cast(type, val)	(type)(val)
#else
#define	static_cast(type, val)	(val)
#endif

/*
 * Common stuff for 4xx (405 and 440)
 */

#define EXC_OFF_SYS_RESET	0x0100	/* System reset				*/
#define _START_OFFSET		(EXC_OFF_SYS_RESET + 0x2000)

#define RESET_VECTOR	0xfffffffc
#define CACHELINE_MASK	(CONFIG_SYS_CACHELINE_SIZE - 1) /* Address mask for cache
						     line aligned data. */

#define CPR0_DCR_BASE	0x0C
#define cprcfga		(CPR0_DCR_BASE+0x0)
#define cprcfgd		(CPR0_DCR_BASE+0x1)

#define SDR_DCR_BASE	0x0E
#define sdrcfga		(SDR_DCR_BASE+0x0)
#define sdrcfgd		(SDR_DCR_BASE+0x1)

#define SDRAM_DCR_BASE	0x10
#define memcfga		(SDRAM_DCR_BASE+0x0)
#define memcfgd		(SDRAM_DCR_BASE+0x1)

#define EBC_DCR_BASE	0x12
#define ebccfga		(EBC_DCR_BASE+0x0)
#define ebccfgd		(EBC_DCR_BASE+0x1)

/*
 * Macros for indirect DCR access
 */
#define mtcpr(reg, d)	do { mtdcr(cprcfga,reg);mtdcr(cprcfgd,d); } while (0)
#define mfcpr(reg, d)	do { mtdcr(cprcfga,reg);d = mfdcr(cprcfgd); } while (0)

#define mtebc(reg, d)	do { mtdcr(ebccfga,reg);mtdcr(ebccfgd,d); } while (0)
#define mfebc(reg, d)	do { mtdcr(ebccfga,reg);d = mfdcr(ebccfgd); } while (0)

#define mtsdram(reg, d)	do { mtdcr(memcfga,reg);mtdcr(memcfgd,d); } while (0)
#define mfsdram(reg, d)	do { mtdcr(memcfga,reg);d = mfdcr(memcfgd); } while (0)

#define mtsdr(reg, d)	do { mtdcr(sdrcfga,reg);mtdcr(sdrcfgd,d); } while (0)
#define mfsdr(reg, d)	do { mtdcr(sdrcfga,reg);d = mfdcr(sdrcfgd); } while (0)

#ifndef __ASSEMBLY__

typedef struct
{
	unsigned long freqDDR;
	unsigned long freqEBC;
	unsigned long freqOPB;
	unsigned long freqPCI;
	unsigned long freqPLB;
	unsigned long freqTmrClk;
	unsigned long freqUART;
	unsigned long freqProcessor;
	unsigned long freqVCOHz;
	unsigned long freqVCOMhz;	/* in MHz                          */
	unsigned long pciClkSync;	/* PCI clock is synchronous        */
	unsigned long pciIntArbEn;	/* Internal PCI arbiter is enabled */
	unsigned long pllExtBusDiv;
	unsigned long pllFbkDiv;
	unsigned long pllFwdDiv;
	unsigned long pllFwdDivA;
	unsigned long pllFwdDivB;
	unsigned long pllOpbDiv;
	unsigned long pllPciDiv;
	unsigned long pllPlbDiv;
} PPC4xx_SYS_INFO;

static inline u32 get_mcsr(void)
{
	u32 val;

	asm volatile("mfspr %0, 0x23c" : "=r" (val) :);
	return val;
}

static inline void set_mcsr(u32 val)
{
	asm volatile("mtspr 0x23c, %0" : "=r" (val) :);
}

#endif	/* __ASSEMBLY__ */

/* for multi-cpu support */
#define NA_OR_UNKNOWN_CPU	-1

#endif	/* __PPC4XX_H__ */
