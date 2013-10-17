/*
 * SPDX-License-Identifier:	GPL-2.0	IBM-pibs
 */

#ifndef	__PPC4XX_H__
#define __PPC4XX_H__

/*
 * Include SoC specific headers
 */
#if defined(CONFIG_405EP)
#include <asm/ppc405ep.h>
#endif

#if defined(CONFIG_405EX)
#include <asm/ppc405ex.h>
#endif

#if defined(CONFIG_405EZ)
#include <asm/ppc405ez.h>
#endif

#if defined(CONFIG_405GP)
#include <asm/ppc405gp.h>
#endif

#if defined(CONFIG_440EP) || defined(CONFIG_440GR)
#include <asm/ppc440ep_gr.h>
#endif

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#include <asm/ppc440epx_grx.h>
#endif

#if defined(CONFIG_440GP)
#include <asm/ppc440gp.h>
#endif

#if defined(CONFIG_440GX)
#include <asm/ppc440gx.h>
#endif

#if defined(CONFIG_440SP)
#include <asm/ppc440sp.h>
#endif

#if defined(CONFIG_440SPE)
#include <asm/ppc440spe.h>
#endif

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
#include <asm/ppc460ex_gt.h>
#endif

#if defined(CONFIG_460SX)
#include <asm/ppc460sx.h>
#endif

#if defined(CONFIG_APM821XX)
#include <asm/apm821xx.h>
#endif

/*
 * Common registers for all SoC's
 */
/* DCR registers */
#define PLB3A0_ACR	0x0077
#define PLB4A0_ACR	0x0081
#define PLB4A1_ACR	0x0089

/* CPR register declarations */

#define PLB4Ax_ACR_PPM_MASK		0xf0000000
#define PLB4Ax_ACR_PPM_FIXED		0x00000000
#define PLB4Ax_ACR_PPM_FAIR		0xd0000000
#define PLB4Ax_ACR_HBU_MASK		0x08000000
#define PLB4Ax_ACR_HBU_DISABLED		0x00000000
#define PLB4Ax_ACR_HBU_ENABLED		0x08000000
#define PLB4Ax_ACR_RDP_MASK		0x06000000
#define PLB4Ax_ACR_RDP_DISABLED		0x00000000
#define PLB4Ax_ACR_RDP_2DEEP		0x02000000
#define PLB4Ax_ACR_RDP_3DEEP		0x04000000
#define PLB4Ax_ACR_RDP_4DEEP		0x06000000
#define PLB4Ax_ACR_WRP_MASK		0x01000000
#define PLB4Ax_ACR_WRP_DISABLED		0x00000000
#define PLB4Ax_ACR_WRP_2DEEP		0x01000000

/*
 * External Bus Controller
 */
/* Values for EBC0_CFGADDR register - indirect addressing of these regs */
#define PB0CR		0x00	/* periph bank 0 config reg		*/
#define PB1CR		0x01	/* periph bank 1 config reg		*/
#define PB2CR		0x02	/* periph bank 2 config reg		*/
#define PB3CR		0x03	/* periph bank 3 config reg		*/
#define PB4CR		0x04	/* periph bank 4 config reg		*/
#define PB5CR		0x05	/* periph bank 5 config reg		*/
#define PB6CR		0x06	/* periph bank 6 config reg		*/
#define PB7CR		0x07	/* periph bank 7 config reg		*/
#define PB0AP		0x10	/* periph bank 0 access parameters	*/
#define PB1AP		0x11	/* periph bank 1 access parameters	*/
#define PB2AP		0x12	/* periph bank 2 access parameters	*/
#define PB3AP		0x13	/* periph bank 3 access parameters	*/
#define PB4AP		0x14	/* periph bank 4 access parameters	*/
#define PB5AP		0x15	/* periph bank 5 access parameters	*/
#define PB6AP		0x16	/* periph bank 6 access parameters	*/
#define PB7AP		0x17	/* periph bank 7 access parameters	*/
#define PBEAR		0x20	/* periph bus error addr reg		*/
#define PBESR0		0x21	/* periph bus error status reg 0	*/
#define PBESR1		0x22	/* periph bus error status reg 1	*/
#define EBC0_CFG	0x23	/* external bus configuration reg	*/

/*
 * GPIO macro register defines
 */
/* todo: merge with gpio.h header */
#define GPIO_BASE		GPIO0_BASE

#define GPIO0_OR		(GPIO0_BASE + 0x0)
#define GPIO0_TCR		(GPIO0_BASE + 0x4)
#define GPIO0_OSRL		(GPIO0_BASE + 0x8)
#define GPIO0_OSRH		(GPIO0_BASE + 0xC)
#define GPIO0_TSRL		(GPIO0_BASE + 0x10)
#define GPIO0_TSRH		(GPIO0_BASE + 0x14)
#define GPIO0_ODR		(GPIO0_BASE + 0x18)
#define GPIO0_IR		(GPIO0_BASE + 0x1C)
#define GPIO0_RR1		(GPIO0_BASE + 0x20)
#define GPIO0_RR2		(GPIO0_BASE + 0x24)
#define GPIO0_RR3		(GPIO0_BASE + 0x28)
#define GPIO0_ISR1L		(GPIO0_BASE + 0x30)
#define GPIO0_ISR1H		(GPIO0_BASE + 0x34)
#define GPIO0_ISR2L		(GPIO0_BASE + 0x38)
#define GPIO0_ISR2H		(GPIO0_BASE + 0x3C)
#define GPIO0_ISR3L		(GPIO0_BASE + 0x40)
#define GPIO0_ISR3H		(GPIO0_BASE + 0x44)

#define GPIO1_OR		(GPIO1_BASE + 0x0)
#define GPIO1_TCR		(GPIO1_BASE + 0x4)
#define GPIO1_OSRL		(GPIO1_BASE + 0x8)
#define GPIO1_OSRH		(GPIO1_BASE + 0xC)
#define GPIO1_TSRL		(GPIO1_BASE + 0x10)
#define GPIO1_TSRH		(GPIO1_BASE + 0x14)
#define GPIO1_ODR		(GPIO1_BASE + 0x18)
#define GPIO1_IR		(GPIO1_BASE + 0x1C)
#define GPIO1_RR1		(GPIO1_BASE + 0x20)
#define GPIO1_RR2		(GPIO1_BASE + 0x24)
#define GPIO1_RR3		(GPIO1_BASE + 0x28)
#define GPIO1_ISR1L		(GPIO1_BASE + 0x30)
#define GPIO1_ISR1H		(GPIO1_BASE + 0x34)
#define GPIO1_ISR2L		(GPIO1_BASE + 0x38)
#define GPIO1_ISR2H		(GPIO1_BASE + 0x3C)
#define GPIO1_ISR3L		(GPIO1_BASE + 0x40)
#define GPIO1_ISR3H		(GPIO1_BASE + 0x44)

/* General Purpose Timer (GPT) Register Offsets */
#define GPT0_TBC		0x00000000
#define GPT0_IM			0x00000018
#define GPT0_ISS		0x0000001C
#define GPT0_ISC		0x00000020
#define GPT0_IE			0x00000024
#define GPT0_COMP0		0x00000080
#define GPT0_COMP1		0x00000084
#define GPT0_COMP2		0x00000088
#define GPT0_COMP3		0x0000008C
#define GPT0_COMP4		0x00000090
#define GPT0_COMP5		0x00000094
#define GPT0_COMP6		0x00000098
#define GPT0_MASK0		0x000000C0
#define GPT0_MASK1		0x000000C4
#define GPT0_MASK2		0x000000C8
#define GPT0_MASK3		0x000000CC
#define GPT0_MASK4		0x000000D0
#define GPT0_MASK5		0x000000D4
#define GPT0_MASK6		0x000000D8
#define GPT0_DCT0		0x00000110
#define GPT0_DCIS		0x0000011C

#if defined(CONFIG_440)
#include <asm/ppc440.h>
#else
#include <asm/ppc405.h>
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

#define EXC_OFF_SYS_RESET	0x0100	/* System reset			*/
#define _START_OFFSET		(EXC_OFF_SYS_RESET + 0x2000)

#define RESET_VECTOR	0xfffffffc
#define CACHELINE_MASK	(CONFIG_SYS_CACHELINE_SIZE - 1) /* Address mask for
						cache line aligned data. */

#define CPR0_DCR_BASE	0x0C
#define CPR0_CFGADDR	(CPR0_DCR_BASE + 0x0)
#define CPR0_CFGDATA	(CPR0_DCR_BASE + 0x1)

#define SDR_DCR_BASE	0x0E
#define SDR0_CFGADDR	(SDR_DCR_BASE + 0x0)
#define SDR0_CFGDATA	(SDR_DCR_BASE + 0x1)

#define SDRAM_DCR_BASE	0x10
#define SDRAM0_CFGADDR	(SDRAM_DCR_BASE + 0x0)
#define SDRAM0_CFGDATA	(SDRAM_DCR_BASE + 0x1)

#define EBC_DCR_BASE	0x12
#define EBC0_CFGADDR	(EBC_DCR_BASE + 0x0)
#define EBC0_CFGDATA	(EBC_DCR_BASE + 0x1)

/*
 * Macros for indirect DCR access
 */
#define mtcpr(reg, d)	\
  do { mtdcr(CPR0_CFGADDR, reg); mtdcr(CPR0_CFGDATA, d); } while (0)
#define mfcpr(reg, d)	\
  do { mtdcr(CPR0_CFGADDR, reg); d = mfdcr(CPR0_CFGDATA); } while (0)

#define mtebc(reg, d)	\
  do { mtdcr(EBC0_CFGADDR, reg); mtdcr(EBC0_CFGDATA, d); } while (0)
#define mfebc(reg, d)	\
  do { mtdcr(EBC0_CFGADDR, reg); d = mfdcr(EBC0_CFGDATA); } while (0)

#define mtsdram(reg, d)	\
  do { mtdcr(SDRAM0_CFGADDR, reg); mtdcr(SDRAM0_CFGDATA, d); } while (0)
#define mfsdram(reg, d)	\
  do { mtdcr(SDRAM0_CFGADDR, reg); d = mfdcr(SDRAM0_CFGDATA); } while (0)

#define mtsdr(reg, d)	\
  do { mtdcr(SDR0_CFGADDR, reg); mtdcr(SDR0_CFGDATA, d); } while (0)
#define mfsdr(reg, d)	\
  do { mtdcr(SDR0_CFGADDR, reg); d = mfdcr(SDR0_CFGDATA); } while (0)

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

int ppc4xx_pci_sync_clock_config(u32 async);

#endif	/* __ASSEMBLY__ */

/* for multi-cpu support */
#define NA_OR_UNKNOWN_CPU	-1

#endif	/* __PPC4XX_H__ */
