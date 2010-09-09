/*----------------------------------------------------------------------------+
|   This source code is dual-licensed.  You may use it under the terms of the
|   GNU General Public License version 2, or under the license below.
|
|	This source code has been made available to you by IBM on an AS-IS
|	basis.	Anyone receiving this source is licensed under IBM
|	copyrights to use it in any way he or she deems fit, including
|	copying it, modifying it, compiling it, and redistributing it either
|	with or without modifications.	No license under IBM patents or
|	patent applications is to be implied by the copyright license.
|
|	Any user of this software should understand that IBM cannot provide
|	technical support for this software and will not be responsible for
|	any consequences resulting from the use of this software.
|
|	Any person who transfers this source code or any derivative work
|	must include the IBM copyright notice, this paragraph, and the
|	preceding two paragraphs in the transferred software.
|
|	COPYRIGHT   I B M   CORPORATION 1999
|	LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
+----------------------------------------------------------------------------*/

#ifndef	__PPC405_H__
#define __PPC405_H__

/* Define bits and masks for real-mode storage attribute control registers */
#define PPC_128MB_SACR_BIT(addr)	((addr) >> 27)
#define PPC_128MB_SACR_VALUE(addr)	PPC_REG_VAL(PPC_128MB_SACR_BIT(addr),1)

#ifndef CONFIG_IOP480
#define CONFIG_SYS_DCACHE_SIZE		(16 << 10)	/* For AMCC 405 CPUs */
#else
#define CONFIG_SYS_DCACHE_SIZE		(2 << 10)	/* For PLX IOP480(403)*/
#endif

/******************************************************************************
 * Special for PPC405GP
 ******************************************************************************/

/******************************************************************************
 * DMA
 ******************************************************************************/
#define DMA_DCR_BASE 0x100
#define DMACR0	(DMA_DCR_BASE+0x00)  /* DMA channel control register 0	     */
#define DMACT0	(DMA_DCR_BASE+0x01)  /* DMA count register 0		     */
#define DMADA0	(DMA_DCR_BASE+0x02)  /* DMA destination address register 0   */
#define DMASA0	(DMA_DCR_BASE+0x03)  /* DMA source address register 0	     */
#define DMASB0	(DMA_DCR_BASE+0x04)  /* DMA scatter/gather descriptor addr 0 */
#define DMACR1	(DMA_DCR_BASE+0x08)  /* DMA channel control register 1	     */
#define DMACT1	(DMA_DCR_BASE+0x09)  /* DMA count register 1		     */
#define DMADA1	(DMA_DCR_BASE+0x0a)  /* DMA destination address register 1   */
#define DMASA1	(DMA_DCR_BASE+0x0b)  /* DMA source address register 1	     */
#define DMASB1	(DMA_DCR_BASE+0x0c)  /* DMA scatter/gather descriptor addr 1 */
#define DMACR2	(DMA_DCR_BASE+0x10)  /* DMA channel control register 2	     */
#define DMACT2	(DMA_DCR_BASE+0x11)  /* DMA count register 2		     */
#define DMADA2	(DMA_DCR_BASE+0x12)  /* DMA destination address register 2   */
#define DMASA2	(DMA_DCR_BASE+0x13)  /* DMA source address register 2	     */
#define DMASB2	(DMA_DCR_BASE+0x14)  /* DMA scatter/gather descriptor addr 2 */
#define DMACR3	(DMA_DCR_BASE+0x18)  /* DMA channel control register 3	     */
#define DMACT3	(DMA_DCR_BASE+0x19)  /* DMA count register 3		     */
#define DMADA3	(DMA_DCR_BASE+0x1a)  /* DMA destination address register 3   */
#define DMASA3	(DMA_DCR_BASE+0x1b)  /* DMA source address register 3	     */
#define DMASB3	(DMA_DCR_BASE+0x1c)  /* DMA scatter/gather descriptor addr 3 */
#define DMASR	(DMA_DCR_BASE+0x20)  /* DMA status register		     */
#define DMASGC	(DMA_DCR_BASE+0x23)  /* DMA scatter/gather command register  */
#define DMAADR	(DMA_DCR_BASE+0x24)  /* DMA address decode register	     */

#ifndef CONFIG_405EP
/******************************************************************************
 * Decompression Controller
 ******************************************************************************/
#define DECOMP_DCR_BASE 0x14
#define KIAR  (DECOMP_DCR_BASE+0x0)	/* Decompression controller addr reg */
#define KIDR  (DECOMP_DCR_BASE+0x1)	/* Decompression controller data reg */
/* values for kiar register - indirect addressing of these regs */
#define KCONF	0x40			/* decompression core config register */
#endif

/******************************************************************************
 * Power Management
 ******************************************************************************/
#ifdef CONFIG_405EX
#define POWERMAN_DCR_BASE 0xb0
#else
#define POWERMAN_DCR_BASE 0xb8
#endif
#define CPMSR	(POWERMAN_DCR_BASE+0x0) /* Power management status */
#define CPMER	(POWERMAN_DCR_BASE+0x1) /* Power management enable */
#define CPMFR	(POWERMAN_DCR_BASE+0x2) /* Power management force */

/******************************************************************************
 * Extrnal Bus Controller
 ******************************************************************************/
  /* values for EBC0_CFGADDR register - indirect addressing of these regs */
  #define PB0CR		0x00	/* periph bank 0 config reg */
  #define PB1CR		0x01	/* periph bank 1 config reg */
  #define PB2CR		0x02	/* periph bank 2 config reg */
  #define PB3CR		0x03	/* periph bank 3 config reg */
  #define PB4CR		0x04	/* periph bank 4 config reg */
#ifndef CONFIG_405EP
  #define PB5CR		0x05	/* periph bank 5 config reg */
  #define PB6CR		0x06	/* periph bank 6 config reg */
  #define PB7CR		0x07	/* periph bank 7 config reg */
#endif
  #define PB0AP		0x10	/* periph bank 0 access parameters */
  #define PB1AP		0x11	/* periph bank 1 access parameters */
  #define PB2AP		0x12	/* periph bank 2 access parameters */
  #define PB3AP		0x13	/* periph bank 3 access parameters */
  #define PB4AP		0x14	/* periph bank 4 access parameters */
#ifndef CONFIG_405EP
  #define PB5AP		0x15	/* periph bank 5 access parameters */
  #define PB6AP		0x16	/* periph bank 6 access parameters */
  #define PB7AP		0x17	/* periph bank 7 access parameters */
#endif
  #define PBEAR		0x20	/* periph bus error addr reg */
  #define PBESR0	0x21	/* periph bus error status reg 0 */
  #define PBESR1	0x22	/* periph bus error status reg 1 */
#define EBC0_CFG	0x23	/* external bus configuration reg */

#ifdef CONFIG_405EP
/******************************************************************************
 * Control
 ******************************************************************************/
#define CNTRL_DCR_BASE 0x0f0
#define CPC0_PLLMR0   (CNTRL_DCR_BASE+0x0)  /* PLL mode  register 0	*/
#define CPC0_BOOT     (CNTRL_DCR_BASE+0x1)  /* Clock status register	*/
#define CPC0_EPCTL    (CNTRL_DCR_BASE+0x3)  /* EMAC to PHY control register */
#define CPC0_PLLMR1   (CNTRL_DCR_BASE+0x4)  /* PLL mode  register 1	*/
#define CPC0_UCR      (CNTRL_DCR_BASE+0x5)  /* UART control register	*/
#define CPC0_PCI      (CNTRL_DCR_BASE+0x9)  /* PCI control register	*/

#define CPC0_PLLMR0  (CNTRL_DCR_BASE+0x0)  /* PLL mode 0 register */
#define CPC0_BOOT    (CNTRL_DCR_BASE+0x1)  /* Chip Clock Status register */
#define CPC0_CR1     (CNTRL_DCR_BASE+0x2)  /* Chip Control 1 register */
#define CPC0_EPRCSR  (CNTRL_DCR_BASE+0x3)  /* EMAC PHY Rcv Clk Src register */
#define CPC0_PLLMR1  (CNTRL_DCR_BASE+0x4)  /* PLL mode 1 register */
#define CPC0_UCR     (CNTRL_DCR_BASE+0x5)  /* UART Control register */
#define CPC0_SRR     (CNTRL_DCR_BASE+0x6)  /* Soft Reset register */
#define CPC0_JTAGID  (CNTRL_DCR_BASE+0x7)  /* JTAG ID register */
#define CPC0_SPARE   (CNTRL_DCR_BASE+0x8)  /* Spare DCR */
#define CPC0_PCI     (CNTRL_DCR_BASE+0x9)  /* PCI Control register */

/* Bit definitions */
#define PLLMR0_CPU_DIV_MASK	 0x00300000	/* CPU clock divider */
#define PLLMR0_CPU_DIV_BYPASS	 0x00000000
#define PLLMR0_CPU_DIV_2	 0x00100000
#define PLLMR0_CPU_DIV_3	 0x00200000
#define PLLMR0_CPU_DIV_4	 0x00300000

#define PLLMR0_CPU_TO_PLB_MASK	 0x00030000	/* CPU:PLB Frequency Divisor */
#define PLLMR0_CPU_PLB_DIV_1	 0x00000000
#define PLLMR0_CPU_PLB_DIV_2	 0x00010000
#define PLLMR0_CPU_PLB_DIV_3	 0x00020000
#define PLLMR0_CPU_PLB_DIV_4	 0x00030000

#define PLLMR0_OPB_TO_PLB_MASK	 0x00003000	/* OPB:PLB Frequency Divisor */
#define PLLMR0_OPB_PLB_DIV_1	 0x00000000
#define PLLMR0_OPB_PLB_DIV_2	 0x00001000
#define PLLMR0_OPB_PLB_DIV_3	 0x00002000
#define PLLMR0_OPB_PLB_DIV_4	 0x00003000

#define PLLMR0_EXB_TO_PLB_MASK	 0x00000300	/* External Bus:PLB Divisor */
#define PLLMR0_EXB_PLB_DIV_2	 0x00000000
#define PLLMR0_EXB_PLB_DIV_3	 0x00000100
#define PLLMR0_EXB_PLB_DIV_4	 0x00000200
#define PLLMR0_EXB_PLB_DIV_5	 0x00000300

#define PLLMR0_MAL_TO_PLB_MASK	 0x00000030	/* MAL:PLB Divisor */
#define PLLMR0_MAL_PLB_DIV_1	 0x00000000
#define PLLMR0_MAL_PLB_DIV_2	 0x00000010
#define PLLMR0_MAL_PLB_DIV_3	 0x00000020
#define PLLMR0_MAL_PLB_DIV_4	 0x00000030

#define PLLMR0_PCI_TO_PLB_MASK	 0x00000003	/* PCI:PLB Frequency Divisor */
#define PLLMR0_PCI_PLB_DIV_1	 0x00000000
#define PLLMR0_PCI_PLB_DIV_2	 0x00000001
#define PLLMR0_PCI_PLB_DIV_3	 0x00000002
#define PLLMR0_PCI_PLB_DIV_4	 0x00000003

#define PLLMR1_SSCS_MASK	 0x80000000	/* Select system clock source */
#define PLLMR1_PLLR_MASK	 0x40000000	/* PLL reset */
#define PLLMR1_FBMUL_MASK	 0x00F00000 /* PLL feedback multiplier value */
#define PLLMR1_FBMUL_DIV_16	 0x00000000
#define PLLMR1_FBMUL_DIV_1	 0x00100000
#define PLLMR1_FBMUL_DIV_2	 0x00200000
#define PLLMR1_FBMUL_DIV_3	 0x00300000
#define PLLMR1_FBMUL_DIV_4	 0x00400000
#define PLLMR1_FBMUL_DIV_5	 0x00500000
#define PLLMR1_FBMUL_DIV_6	 0x00600000
#define PLLMR1_FBMUL_DIV_7	 0x00700000
#define PLLMR1_FBMUL_DIV_8	 0x00800000
#define PLLMR1_FBMUL_DIV_9	 0x00900000
#define PLLMR1_FBMUL_DIV_10	 0x00A00000
#define PLLMR1_FBMUL_DIV_11	 0x00B00000
#define PLLMR1_FBMUL_DIV_12	 0x00C00000
#define PLLMR1_FBMUL_DIV_13	 0x00D00000
#define PLLMR1_FBMUL_DIV_14	 0x00E00000
#define PLLMR1_FBMUL_DIV_15	 0x00F00000

#define PLLMR1_FWDVA_MASK	 0x00070000 /* PLL forward divider A value */
#define PLLMR1_FWDVA_DIV_8	 0x00000000
#define PLLMR1_FWDVA_DIV_7	 0x00010000
#define PLLMR1_FWDVA_DIV_6	 0x00020000
#define PLLMR1_FWDVA_DIV_5	 0x00030000
#define PLLMR1_FWDVA_DIV_4	 0x00040000
#define PLLMR1_FWDVA_DIV_3	 0x00050000
#define PLLMR1_FWDVA_DIV_2	 0x00060000
#define PLLMR1_FWDVA_DIV_1	 0x00070000
#define PLLMR1_FWDVB_MASK	 0x00007000 /* PLL forward divider B value */
#define PLLMR1_TUNING_MASK	 0x000003FF /* PLL tune bits */

/* Defines for CPC0_EPRCSR register */
#define CPC0_EPRCSR_E0NFE	0x80000000
#define CPC0_EPRCSR_E1NFE	0x40000000
#define CPC0_EPRCSR_E1RPP	0x00000080
#define CPC0_EPRCSR_E0RPP	0x00000040
#define CPC0_EPRCSR_E1ERP	0x00000020
#define CPC0_EPRCSR_E0ERP	0x00000010
#define CPC0_EPRCSR_E1PCI	0x00000002
#define CPC0_EPRCSR_E0PCI	0x00000001

/* Defines for CPC0_PCI Register */
#define CPC0_PCI_SPE		0x00000010 /* PCIINT/WE select	 */
#define CPC0_PCI_HOST_CFG_EN	0x00000008 /* PCI host config Enable */
#define CPC0_PCI_ARBIT_EN	0x00000001 /* PCI Internal Arb Enabled */

/* Defines for CPC0_BOOR Register */
#define CPC0_BOOT_SEP		0x00000002 /* serial EEPROM present */

/* Defines for CPC0_PLLMR1 Register fields */
#define PLL_ACTIVE		0x80000000
#define CPC0_PLLMR1_SSCS	0x80000000
#define PLL_RESET		0x40000000
#define CPC0_PLLMR1_PLLR	0x40000000
	/* Feedback multiplier */
#define PLL_FBKDIV		0x00F00000
#define CPC0_PLLMR1_FBDV	0x00F00000
#define PLL_FBKDIV_16		0x00000000
#define PLL_FBKDIV_1		0x00100000
#define PLL_FBKDIV_2		0x00200000
#define PLL_FBKDIV_3		0x00300000
#define PLL_FBKDIV_4		0x00400000
#define PLL_FBKDIV_5		0x00500000
#define PLL_FBKDIV_6		0x00600000
#define PLL_FBKDIV_7		0x00700000
#define PLL_FBKDIV_8		0x00800000
#define PLL_FBKDIV_9		0x00900000
#define PLL_FBKDIV_10		0x00A00000
#define PLL_FBKDIV_11		0x00B00000
#define PLL_FBKDIV_12		0x00C00000
#define PLL_FBKDIV_13		0x00D00000
#define PLL_FBKDIV_14		0x00E00000
#define PLL_FBKDIV_15		0x00F00000
	/* Forward A divisor */
#define PLL_FWDDIVA		0x00070000
#define CPC0_PLLMR1_FWDVA	0x00070000
#define PLL_FWDDIVA_8		0x00000000
#define PLL_FWDDIVA_7		0x00010000
#define PLL_FWDDIVA_6		0x00020000
#define PLL_FWDDIVA_5		0x00030000
#define PLL_FWDDIVA_4		0x00040000
#define PLL_FWDDIVA_3		0x00050000
#define PLL_FWDDIVA_2		0x00060000
#define PLL_FWDDIVA_1		0x00070000
	/* Forward B divisor */
#define PLL_FWDDIVB		0x00007000
#define CPC0_PLLMR1_FWDVB	0x00007000
#define PLL_FWDDIVB_8		0x00000000
#define PLL_FWDDIVB_7		0x00001000
#define PLL_FWDDIVB_6		0x00002000
#define PLL_FWDDIVB_5		0x00003000
#define PLL_FWDDIVB_4		0x00004000
#define PLL_FWDDIVB_3		0x00005000
#define PLL_FWDDIVB_2		0x00006000
#define PLL_FWDDIVB_1		0x00007000
	/* PLL tune bits */
#define PLL_TUNE_MASK		 0x000003FF
#define PLL_TUNE_2_M_3		 0x00000133	/*  2 <= M <= 3 */
#define PLL_TUNE_4_M_6		 0x00000134	/*  3 <  M <= 6 */
#define PLL_TUNE_7_M_10		 0x00000138	/*  6 <  M <= 10 */
#define PLL_TUNE_11_M_14	 0x0000013C	/* 10 <  M <= 14 */
#define PLL_TUNE_15_M_40	 0x0000023E	/* 14 <  M <= 40 */
#define PLL_TUNE_VCO_LOW	 0x00000000	/* 500MHz <= VCO <=  800MHz */
#define PLL_TUNE_VCO_HI		 0x00000080	/* 800MHz <  VCO <= 1000MHz */

/* Defines for CPC0_PLLMR0 Register fields */
	/* CPU divisor */
#define PLL_CPUDIV		0x00300000
#define CPC0_PLLMR0_CCDV	0x00300000
#define PLL_CPUDIV_1		0x00000000
#define PLL_CPUDIV_2		0x00100000
#define PLL_CPUDIV_3		0x00200000
#define PLL_CPUDIV_4		0x00300000
	/* PLB divisor */
#define PLL_PLBDIV		0x00030000
#define CPC0_PLLMR0_CBDV	0x00030000
#define PLL_PLBDIV_1		0x00000000
#define PLL_PLBDIV_2		0x00010000
#define PLL_PLBDIV_3		0x00020000
#define PLL_PLBDIV_4		0x00030000
	/* OPB divisor */
#define PLL_OPBDIV		0x00003000
#define CPC0_PLLMR0_OPDV	0x00003000
#define PLL_OPBDIV_1		0x00000000
#define PLL_OPBDIV_2		0x00001000
#define PLL_OPBDIV_3		0x00002000
#define PLL_OPBDIV_4		0x00003000
	/* EBC divisor */
#define PLL_EXTBUSDIV		0x00000300
#define CPC0_PLLMR0_EPDV	0x00000300
#define PLL_EXTBUSDIV_2		0x00000000
#define PLL_EXTBUSDIV_3		0x00000100
#define PLL_EXTBUSDIV_4		0x00000200
#define PLL_EXTBUSDIV_5		0x00000300
	/* MAL divisor */
#define PLL_MALDIV		0x00000030
#define CPC0_PLLMR0_MPDV	0x00000030
#define PLL_MALDIV_1		0x00000000
#define PLL_MALDIV_2		0x00000010
#define PLL_MALDIV_3		0x00000020
#define PLL_MALDIV_4		0x00000030
	/* PCI divisor */
#define PLL_PCIDIV		0x00000003
#define CPC0_PLLMR0_PPFD	0x00000003
#define PLL_PCIDIV_1		0x00000000
#define PLL_PCIDIV_2		0x00000001
#define PLL_PCIDIV_3		0x00000002
#define PLL_PCIDIV_4		0x00000003

/*
 *------------------------------------------------------------------------------
 * PLL settings for 266MHz CPU, 133MHz PLB/SDRAM, 66MHz EBC, 33MHz PCI,
 * assuming a 33.3MHz input clock to the 405EP.
 *------------------------------------------------------------------------------
 */
#define PLLMR0_266_133_66  (PLL_CPUDIV_1 | PLL_PLBDIV_2 |  \
			    PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |  \
			    PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PLLMR1_266_133_66  (PLL_FBKDIV_8  |  \
			    PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			    PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)

#define PLLMR0_133_66_66_33  (PLL_CPUDIV_1 | PLL_PLBDIV_1 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_4 |	\
			      PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PLLMR1_133_66_66_33  (PLL_FBKDIV_4  |  \
			      PLL_FWDDIVA_6 | PLL_FWDDIVB_6 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)
#define PLLMR0_200_100_50_33 (PLL_CPUDIV_1 | PLL_PLBDIV_2 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_3 |	\
			      PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PLLMR1_200_100_50_33 (PLL_FBKDIV_6  |  \
			      PLL_FWDDIVA_4 | PLL_FWDDIVB_4 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)
#define PLLMR0_266_133_66_33 (PLL_CPUDIV_1 | PLL_PLBDIV_2 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_4 |	\
			      PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PLLMR1_266_133_66_33 (PLL_FBKDIV_8  |  \
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)
#define PLLMR0_266_66_33_33 (PLL_CPUDIV_1 | PLL_PLBDIV_4 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |	\
			      PLL_MALDIV_1 | PLL_PCIDIV_2)
#define PLLMR1_266_66_33_33 (PLL_FBKDIV_8  |  \
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)
#define PLLMR0_333_111_55_37 (PLL_CPUDIV_1 | PLL_PLBDIV_3 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |	\
			      PLL_MALDIV_1 | PLL_PCIDIV_3)
#define PLLMR1_333_111_55_37 (PLL_FBKDIV_10  |	\
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_HI)
#define PLLMR0_333_111_55_111 (PLL_CPUDIV_1 | PLL_PLBDIV_3 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |	\
			      PLL_MALDIV_1 | PLL_PCIDIV_1)
#define PLLMR1_333_111_55_111 (PLL_FBKDIV_10  |  \
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_HI)

/*
 * PLL Voltage Controlled Oscillator (VCO) definitions
 * Maximum and minimum values (in MHz) for correct PLL operation.
 */
#define VCO_MIN     500
#define VCO_MAX     1000
#elif defined(CONFIG_405EZ)
#define SDR0_NAND0	0x4000
#define SDR0_ULTRA0	0x4040
#define SDR0_ULTRA1	0x4050
#define SDR0_ICINTSTAT	0x4510

#define SDR_NAND0_NDEN		0x80000000
#define SDR_NAND0_NDBTEN	0x40000000
#define SDR_NAND0_NDBADR_MASK	0x30000000
#define SDR_NAND0_NDBPG_MASK	0x0f000000
#define SDR_NAND0_NDAREN	0x00800000
#define SDR_NAND0_NDRBEN	0x00400000

#define SDR_ULTRA0_NDGPIOBP	0x80000000
#define SDR_ULTRA0_CSN_MASK	0x78000000
#define SDR_ULTRA0_CSNSEL0	0x40000000
#define SDR_ULTRA0_CSNSEL1	0x20000000
#define SDR_ULTRA0_CSNSEL2	0x10000000
#define SDR_ULTRA0_CSNSEL3	0x08000000
#define SDR_ULTRA0_EBCRDYEN	0x04000000
#define SDR_ULTRA0_SPISSINEN	0x02000000
#define SDR_ULTRA0_NFSRSTEN	0x01000000

#define SDR_ULTRA1_LEDNENABLE	0x40000000

#define SDR_ICRX_STAT	0x80000000
#define SDR_ICTX0_STAT	0x40000000
#define SDR_ICTX1_STAT	0x20000000

#define SDR0_PINSTP	0x40

/******************************************************************************
 * Control
 ******************************************************************************/
/* CPR Registers */
#define CPR0_CLKUP	0x020		/* CPR_CLKUPD */
#define CPR0_PLLC		0x040		/* CPR_PLLC */
#define CPR0_PLLD		0x060		/* CPR_PLLD */
#define CPC0_PRIMAD	0x080		/* CPR_PRIMAD */
#define CPC0_PERD0	0x0e0		/* CPR_PERD0 */
#define CPC0_PERD1	0x0e1		/* CPR_PERD1 */
#define CPC0_PERC0	0x180		/* CPR_PERC0 */

#define CPR_CLKUPD_ENPLLCH_EN  0x40000000 /* Enable CPR PLL Changes */
#define CPR_CLKUPD_ENDVCH_EN   0x20000000 /* Enable CPR Sys. Div. Changes */
#define CPR_PERD0_SPIDV_MASK   0x000F0000 /* SPI Clock Divider */

#define PLLC_SRC_MASK	       0x20000000 /* PLL feedback source */

#define PLLD_FBDV_MASK	       0x1F000000 /* PLL feedback divider value */
#define PLLD_FWDVA_MASK        0x000F0000 /* PLL forward divider A value */
#define PLLD_FWDVB_MASK        0x00000700 /* PLL forward divider B value */

#define PRIMAD_CPUDV_MASK      0x0F000000 /* CPU Clock Divisor Mask */
#define PRIMAD_PLBDV_MASK      0x000F0000 /* PLB Clock Divisor Mask */
#define PRIMAD_OPBDV_MASK      0x00000F00 /* OPB Clock Divisor Mask */
#define PRIMAD_EBCDV_MASK      0x0000000F /* EBC Clock Divisor Mask */

#define PERD0_PWMDV_MASK       0xFF000000 /* PWM Divider Mask */
#define PERD0_SPIDV_MASK       0x000F0000 /* SPI Divider Mask */
#define PERD0_U0DV_MASK        0x0000FF00 /* UART 0 Divider Mask */
#define PERD0_U1DV_MASK        0x000000FF /* UART 1 Divider Mask */

#else /* #ifdef CONFIG_405EP */
/******************************************************************************
 * Control
 ******************************************************************************/
#define CNTRL_DCR_BASE 0x0b0
#define CPC0_PLLMR	(CNTRL_DCR_BASE + 0x0)	/* PLL mode  register */
#define CPC0_CR0	(CNTRL_DCR_BASE + 0x1)	/* chip control register 0 */
#define CPC0_CR1	(CNTRL_DCR_BASE + 0x2)	/* chip control register 1 */
#define CPC0_PSR	(CNTRL_DCR_BASE + 0x4)	/* chip pin strapping reg */

/* CPC0_ECR/CPC0_EIRR: PPC405GPr only */
#define CPC0_EIRR	(CNTRL_DCR_BASE + 0x6)	/* ext interrupt routing reg */
#define CPC0_ECR	0xaa			/* edge conditioner register */

/* Bit definitions */
#define PLLMR_FWD_DIV_MASK	0xE0000000	/* Forward Divisor */
#define PLLMR_FWD_DIV_BYPASS	0xE0000000
#define PLLMR_FWD_DIV_3		0xA0000000
#define PLLMR_FWD_DIV_4		0x80000000
#define PLLMR_FWD_DIV_6		0x40000000

#define PLLMR_FB_DIV_MASK	0x1E000000	/* Feedback Divisor */
#define PLLMR_FB_DIV_1		0x02000000
#define PLLMR_FB_DIV_2		0x04000000
#define PLLMR_FB_DIV_3		0x06000000
#define PLLMR_FB_DIV_4		0x08000000

#define PLLMR_TUNING_MASK	0x01F80000

#define PLLMR_CPU_TO_PLB_MASK	0x00060000	/* CPU:PLB Frequency Divisor */
#define PLLMR_CPU_PLB_DIV_1	0x00000000
#define PLLMR_CPU_PLB_DIV_2	0x00020000
#define PLLMR_CPU_PLB_DIV_3	0x00040000
#define PLLMR_CPU_PLB_DIV_4	0x00060000

#define PLLMR_OPB_TO_PLB_MASK	0x00018000	/* OPB:PLB Frequency Divisor */
#define PLLMR_OPB_PLB_DIV_1	0x00000000
#define PLLMR_OPB_PLB_DIV_2	0x00008000
#define PLLMR_OPB_PLB_DIV_3	0x00010000
#define PLLMR_OPB_PLB_DIV_4	0x00018000

#define PLLMR_PCI_TO_PLB_MASK	0x00006000	/* PCI:PLB Frequency Divisor */
#define PLLMR_PCI_PLB_DIV_1	0x00000000
#define PLLMR_PCI_PLB_DIV_2	0x00002000
#define PLLMR_PCI_PLB_DIV_3	0x00004000
#define PLLMR_PCI_PLB_DIV_4	0x00006000

#define PLLMR_EXB_TO_PLB_MASK	0x00001800	/* External Bus:PLB Divisor */
#define PLLMR_EXB_PLB_DIV_2	0x00000000
#define PLLMR_EXB_PLB_DIV_3	0x00000800
#define PLLMR_EXB_PLB_DIV_4	0x00001000
#define PLLMR_EXB_PLB_DIV_5	0x00001800

/* definitions for PPC405GPr (new mode strapping) */
#define PLLMR_FWDB_DIV_MASK	0x00000007	/* Forward Divisor B */

#define PSR_PLL_FWD_MASK	0xC0000000
#define PSR_PLL_FDBACK_MASK	0x30000000
#define PSR_PLL_TUNING_MASK	0x0E000000
#define PSR_PLB_CPU_MASK	0x01800000
#define PSR_OPB_PLB_MASK	0x00600000
#define PSR_PCI_PLB_MASK	0x00180000
#define PSR_EB_PLB_MASK		0x00060000
#define PSR_ROM_WIDTH_MASK	0x00018000
#define PSR_ROM_LOC		0x00004000
#define PSR_PCI_ASYNC_EN	0x00001000
#define PSR_PERCLK_SYNC_MODE_EN 0x00000800	/* PPC405GPr only */
#define PSR_PCI_ARBIT_EN	0x00000400
#define PSR_NEW_MODE_EN		0x00000020	/* PPC405GPr only */

#ifndef CONFIG_IOP480
/*
 * PLL Voltage Controlled Oscillator (VCO) definitions
 * Maximum and minimum values (in MHz) for correct PLL operation.
*/
#define VCO_MIN     400
#define VCO_MAX     800
#endif /* #ifndef CONFIG_IOP480 */
#endif /* #ifdef CONFIG_405EP */

/******************************************************************************
 * Memory Access Layer
 ******************************************************************************/
#if defined(CONFIG_405EZ)
#define	MAL_DCR_BASE	0x380
#else
#define MAL_DCR_BASE	0x180
#endif
#define	MAL0_CFG	(MAL_DCR_BASE + 0x00) /* MAL Config reg */
#define	MAL0_ESR	(MAL_DCR_BASE + 0x01) /* Err Status (Read/Clear) */
#define	MAL0_IER	(MAL_DCR_BASE + 0x02) /* Interrupt enable */
#define	MAL0_TXCASR	(MAL_DCR_BASE + 0x04) /* TX Channel active (set) */
#define	MAL0_TXCARR	(MAL_DCR_BASE + 0x05) /* TX Channel active (reset) */
#define	MAL0_TXEOBISR	(MAL_DCR_BASE + 0x06) /* TX End of buffer int status */
#define	MAL0_TXDEIR	(MAL_DCR_BASE + 0x07) /* TX Descr. Error Int reg */
#define	MAL0_RXCASR	(MAL_DCR_BASE + 0x10) /* RX Channel active (set) */
#define	MAL0_RXCARR	(MAL_DCR_BASE + 0x11) /* RX Channel active (reset) */
#define	MAL0_RXEOBISR	(MAL_DCR_BASE + 0x12) /* RX End of buffer int status */
#define	MAL0_RXDEIR	(MAL_DCR_BASE + 0x13) /* RX Descr. Error Int reg */
#define	MAL0_TXCTP0R	(MAL_DCR_BASE + 0x20) /* TX 0 Channel table ptr */
#define	MAL0_TXCTP1R	(MAL_DCR_BASE + 0x21) /* TX 1 Channel table ptr */
#define	MAL0_TXCTP2R	(MAL_DCR_BASE + 0x22) /* TX 2 Channel table ptr */
#define	MAL0_TXCTP3R	(MAL_DCR_BASE + 0x23) /* TX 3 Channel table ptr */
#define	MAL0_RXCTP0R	(MAL_DCR_BASE + 0x40) /* RX 0 Channel table ptr */
#define	MAL0_RXCTP1R	(MAL_DCR_BASE + 0x41) /* RX 1 Channel table ptr */
#define	MAL0_RXCTP2R	(MAL_DCR_BASE + 0x42) /* RX 2 Channel table ptr */
#define	MAL0_RXCTP3R	(MAL_DCR_BASE + 0x43) /* RX 3 Channel table ptr */
#define	MAL0_RXCTP8R	(MAL_DCR_BASE + 0x48) /* RX 8 Channel table ptr */
#define	MAL0_RXCTP16R	(MAL_DCR_BASE + 0x50) /* RX 16 Channel table ptr */
#define	MAL0_RXCTP24R	(MAL_DCR_BASE + 0x58) /* RX 24 Channel table ptr */
#define	MAL0_RCBS0	(MAL_DCR_BASE + 0x60) /* RX 0 Channel buffer size */
#define	MAL0_RCBS1	(MAL_DCR_BASE + 0x61) /* RX 1 Channel buffer size */
#define	MAL0_RCBS2	(MAL_DCR_BASE + 0x62) /* RX 2 Channel buffer size */
#define	MAL0_RCBS3	(MAL_DCR_BASE + 0x63) /* RX 3 Channel buffer size */
#define	MAL0_RCBS8	(MAL_DCR_BASE + 0x68) /* RX 8 Channel buffer size */
#define	MAL0_RCBS16	(MAL_DCR_BASE + 0x70) /* RX 16 Channel buffer size */
#define	MAL0_RCBS24	(MAL_DCR_BASE + 0x78) /* RX 24 Channel buffer size */

/*-----------------------------------------------------------------------------
| UART Register Offsets
'----------------------------------------------------------------------------*/
#define		DATA_REG	0x00
#define		DL_LSB		0x00
#define		DL_MSB		0x01
#define		INT_ENABLE	0x01
#define		FIFO_CONTROL	0x02
#define		LINE_CONTROL	0x03
#define		MODEM_CONTROL	0x04
#define		LINE_STATUS	0x05
#define		MODEM_STATUS	0x06
#define		SCRATCH		0x07

/******************************************************************************
 * On Chip Memory
 ******************************************************************************/
#if defined(CONFIG_405EZ)
#define OCM_DCR_BASE 0x020
#define OCM0_PLBCR1	(OCM_DCR_BASE + 0x00)	/* OCM PLB3 Bank 1 Config */
#define OCM0_PLBCR2	(OCM_DCR_BASE + 0x01)	/* OCM PLB3 Bank 2 Config */
#define OCM0_PLBBEAR	(OCM_DCR_BASE + 0x02)	/* OCM PLB3 Bus Error Add */
#define OCM0_DSRC1	(OCM_DCR_BASE + 0x08)	/* OCM D-side Bank 1 Config */
#define OCM0_DSRC2	(OCM_DCR_BASE + 0x09)	/* OCM D-side Bank 2 Config */
#define OCM0_ISRC1	(OCM_DCR_BASE + 0x0A)	/* OCM I-side Bank 1Config */
#define OCM0_ISRC2	(OCM_DCR_BASE + 0x0B)	/* OCM I-side Bank 2 Config */
#define OCM0_DISDPC	(OCM_DCR_BASE + 0x0C)	/* OCM D-/I-side Data Par Chk */
#else
#define OCM_DCR_BASE 0x018
#define OCM0_ISCNTL	(OCM_DCR_BASE+0x01)	/* OCM I-side control reg */
#define OCM0_DSARC	(OCM_DCR_BASE+0x02)	/* OCM D-side address compare */
#define OCM0_DSCNTL	(OCM_DCR_BASE+0x03)	/* OCM D-side control */
#endif /* CONFIG_405EZ */

/******************************************************************************
 * GPIO macro register defines
 ******************************************************************************/
#if defined(CONFIG_405EZ)
/* Only the 405EZ has 2 GPIOs */
#define GPIO_BASE  0xEF600700
#define GPIO0_OR		(GPIO_BASE+0x0)
#define GPIO0_TCR		(GPIO_BASE+0x4)
#define GPIO0_OSRL		(GPIO_BASE+0x8)
#define GPIO0_OSRH		(GPIO_BASE+0xC)
#define GPIO0_TSRL		(GPIO_BASE+0x10)
#define GPIO0_TSRH		(GPIO_BASE+0x14)
#define GPIO0_ODR		(GPIO_BASE+0x18)
#define GPIO0_IR		(GPIO_BASE+0x1C)
#define GPIO0_RR1		(GPIO_BASE+0x20)
#define GPIO0_RR2		(GPIO_BASE+0x24)
#define GPIO0_RR3		(GPIO_BASE+0x28)
#define GPIO0_ISR1L		(GPIO_BASE+0x30)
#define GPIO0_ISR1H		(GPIO_BASE+0x34)
#define GPIO0_ISR2L		(GPIO_BASE+0x38)
#define GPIO0_ISR2H		(GPIO_BASE+0x3C)
#define GPIO0_ISR3L		(GPIO_BASE+0x40)
#define GPIO0_ISR3H		(GPIO_BASE+0x44)

#define GPIO1_BASE  0xEF600800
#define GPIO1_OR		(GPIO1_BASE+0x0)
#define GPIO1_TCR		(GPIO1_BASE+0x4)
#define GPIO1_OSRL		(GPIO1_BASE+0x8)
#define GPIO1_OSRH		(GPIO1_BASE+0xC)
#define GPIO1_TSRL		(GPIO1_BASE+0x10)
#define GPIO1_TSRH		(GPIO1_BASE+0x14)
#define GPIO1_ODR		(GPIO1_BASE+0x18)
#define GPIO1_IR		(GPIO1_BASE+0x1C)
#define GPIO1_RR1		(GPIO1_BASE+0x20)
#define GPIO1_RR2		(GPIO1_BASE+0x24)
#define GPIO1_RR3		(GPIO1_BASE+0x28)
#define GPIO1_ISR1L		(GPIO1_BASE+0x30)
#define GPIO1_ISR1H		(GPIO1_BASE+0x34)
#define GPIO1_ISR2L		(GPIO1_BASE+0x38)
#define GPIO1_ISR2H		(GPIO1_BASE+0x3C)
#define GPIO1_ISR3L		(GPIO1_BASE+0x40)
#define GPIO1_ISR3H		(GPIO1_BASE+0x44)

#elif defined(CONFIG_405EX)
#define GPIO_BASE  0xEF600800
#define GPIO0_OR	       (GPIO_BASE+0x0)
#define GPIO0_TCR	       (GPIO_BASE+0x4)
#define GPIO0_OSRL	       (GPIO_BASE+0x8)
#define GPIO0_OSRH	       (GPIO_BASE+0xC)
#define GPIO0_TSRL	       (GPIO_BASE+0x10)
#define GPIO0_TSRH	       (GPIO_BASE+0x14)
#define GPIO0_ODR	       (GPIO_BASE+0x18)
#define GPIO0_IR	       (GPIO_BASE+0x1C)
#define GPIO0_RR1	       (GPIO_BASE+0x20)
#define GPIO0_RR2	       (GPIO_BASE+0x24)
#define GPIO0_ISR1L	       (GPIO_BASE+0x30)
#define GPIO0_ISR1H	       (GPIO_BASE+0x34)
#define GPIO0_ISR2L	       (GPIO_BASE+0x38)
#define GPIO0_ISR2H	       (GPIO_BASE+0x3C)
#define GPIO0_ISR3L	       (GPIO_BASE+0x40)
#define GPIO0_ISR3H	       (GPIO_BASE+0x44)

#else	/* !405EZ */

#define GPIO_BASE  0xEF600700
#define GPIO0_OR	       (GPIO_BASE+0x0)
#define GPIO0_TCR	       (GPIO_BASE+0x4)
#define GPIO0_OSRH	       (GPIO_BASE+0x8)
#define GPIO0_OSRL	       (GPIO_BASE+0xC)
#define GPIO0_TSRH	       (GPIO_BASE+0x10)
#define GPIO0_TSRL	       (GPIO_BASE+0x14)
#define GPIO0_ODR	       (GPIO_BASE+0x18)
#define GPIO0_IR	       (GPIO_BASE+0x1C)
#define GPIO0_RR1	       (GPIO_BASE+0x20)
#define GPIO0_RR2	       (GPIO_BASE+0x24)
#define GPIO0_ISR1H	       (GPIO_BASE+0x30)
#define GPIO0_ISR1L	       (GPIO_BASE+0x34)
#define GPIO0_ISR2H	       (GPIO_BASE+0x38)
#define GPIO0_ISR2L	       (GPIO_BASE+0x3C)

#endif /* CONFIG_405EZ */

#define GPIO0_BASE		GPIO_BASE

#if defined(CONFIG_405EX)
#define SDR0_SRST		0x0200

/*
 * Software Reset Register
 */
#define SDR0_SRST_BGO		PPC_REG_VAL(0, 1)
#define SDR0_SRST_PLB4		PPC_REG_VAL(1, 1)
#define SDR0_SRST_EBC		PPC_REG_VAL(2, 1)
#define SDR0_SRST_OPB		PPC_REG_VAL(3, 1)
#define SDR0_SRST_UART0		PPC_REG_VAL(4, 1)
#define SDR0_SRST_UART1		PPC_REG_VAL(5, 1)
#define SDR0_SRST_IIC0		PPC_REG_VAL(6, 1)
#define SDR0_SRST_BGI		PPC_REG_VAL(7, 1)
#define SDR0_SRST_GPIO		PPC_REG_VAL(8, 1)
#define SDR0_SRST_GPT		PPC_REG_VAL(9, 1)
#define SDR0_SRST_DMC		PPC_REG_VAL(10, 1)
#define SDR0_SRST_RGMII		PPC_REG_VAL(11, 1)
#define SDR0_SRST_EMAC0		PPC_REG_VAL(12, 1)
#define SDR0_SRST_EMAC1		PPC_REG_VAL(13, 1)
#define SDR0_SRST_CPM		PPC_REG_VAL(14, 1)
#define SDR0_SRST_EPLL		PPC_REG_VAL(15, 1)
#define SDR0_SRST_UIC		PPC_REG_VAL(16, 1)
#define SDR0_SRST_UPRST		PPC_REG_VAL(17, 1)
#define SDR0_SRST_IIC1		PPC_REG_VAL(18, 1)
#define SDR0_SRST_SCP		PPC_REG_VAL(19, 1)
#define SDR0_SRST_UHRST		PPC_REG_VAL(20, 1)
#define SDR0_SRST_DMA		PPC_REG_VAL(21, 1)
#define SDR0_SRST_DMAC		PPC_REG_VAL(22, 1)
#define SDR0_SRST_MAL		PPC_REG_VAL(23, 1)
#define SDR0_SRST_EBM		PPC_REG_VAL(24, 1)
#define SDR0_SRST_GPTR		PPC_REG_VAL(25, 1)
#define SDR0_SRST_PE0		PPC_REG_VAL(26, 1)
#define SDR0_SRST_PE1		PPC_REG_VAL(27, 1)
#define SDR0_SRST_CRYP		PPC_REG_VAL(28, 1)
#define SDR0_SRST_PKP		PPC_REG_VAL(29, 1)
#define SDR0_SRST_AHB		PPC_REG_VAL(30, 1)
#define SDR0_SRST_NDFC		PPC_REG_VAL(31, 1)

#define SDR0_UART0		0x0120	/* UART0 Config */
#define SDR0_UART1		0x0121	/* UART1 Config */
#define SDR0_MFR		0x4300	/* SDR0_MFR reg */

/* Defines for CPC0_EPRCSR register */
#define CPC0_EPRCSR_E0NFE	0x80000000
#define CPC0_EPRCSR_E1NFE	0x40000000
#define CPC0_EPRCSR_E1RPP	0x00000080
#define CPC0_EPRCSR_E0RPP	0x00000040
#define CPC0_EPRCSR_E1ERP	0x00000020
#define CPC0_EPRCSR_E0ERP	0x00000010
#define CPC0_EPRCSR_E1PCI	0x00000002
#define CPC0_EPRCSR_E0PCI	0x00000001

#define CPR0_CLKUPD	0x020
#define CPR0_PLLC	0x040
#define CPR0_PLLD	0x060
#define CPR0_CPUD	0x080
#define CPR0_PLBD	0x0a0
#define CPR0_OPBD0	0x0c0
#define CPR0_PERD	0x0e0

#define SDR0_PINSTP	0x0040
#define SDR0_SDCS0	0x0060

#define SDR0_SDCS_SDD			(0x80000000 >> 31)

/* CUST0 Customer Configuration Register0 */
#define SDR0_CUST0		     0x4000
#define SDR0_CUST0_MUX_E_N_G_MASK	0xC0000000 /* Mux_Emac_NDFC_GPIO */
#define SDR0_CUST0_MUX_EMAC_SEL		0x40000000 /* Emac Selection */
#define SDR0_CUST0_MUX_NDFC_SEL		0x80000000 /* NDFC Selection */
#define SDR0_CUST0_MUX_GPIO_SEL		0xC0000000 /* GPIO Selection */

#define SDR0_CUST0_NDFC_EN_MASK		0x20000000 /* NDFC Enable Mask */
#define SDR0_CUST0_NDFC_ENABLE		0x20000000 /* NDFC Enable */
#define SDR0_CUST0_NDFC_DISABLE		0x00000000 /* NDFC Disable */

#define SDR0_CUST0_NDFC_BW_MASK	  	0x10000000 /* NDFC Boot Width */
#define SDR0_CUST0_NDFC_BW_16_BIT 	0x10000000 /* NDFC Boot Width= 16 Bit */
#define SDR0_CUST0_NDFC_BW_8_BIT  	0x00000000 /* NDFC Boot Width=  8 Bit */

#define SDR0_CUST0_NDFC_BP_MASK		0x0F000000 /* NDFC Boot Page */
#define SDR0_CUST0_NDFC_BP_ENCODE(n)	((((unsigned long)(n))&0xF)<<24)
#define SDR0_CUST0_NDFC_BP_DECODE(n)	((((unsigned long)(n))>>24)&0x0F)

#define SDR0_CUST0_NDFC_BAC_MASK	0x00C00000 /* NDFC Boot Address Cycle */
#define SDR0_CUST0_NDFC_BAC_ENCODE(n)	((((unsigned long)(n))&0x3)<<22)
#define SDR0_CUST0_NDFC_BAC_DECODE(n)	((((unsigned long)(n))>>22)&0x03)

#define SDR0_CUST0_NDFC_ARE_MASK	0x00200000 /* NDFC Auto Read Enable */
#define SDR0_CUST0_NDFC_ARE_ENABLE	0x00200000 /* NDFC Auto Read Enable */
#define SDR0_CUST0_NDFC_ARE_DISABLE	0x00000000 /* NDFC Auto Read Disable */

#define SDR0_CUST0_NRB_MASK		0x00100000 /* NDFC Ready / Busy */
#define SDR0_CUST0_NRB_BUSY		0x00100000 /* Busy */
#define SDR0_CUST0_NRB_READY		0x00000000 /* Ready */

#define SDR0_CUST0_NDRSC_MASK	0x0000FFF0 /* NDFC Device Reset Count Mask */
#define SDR0_CUST0_NDRSC_ENCODE(n)	((((unsigned long)(n))&0xFFF)<<4)
#define SDR0_CUST0_NDRSC_DECODE(n)	((((unsigned long)(n))>>4)&0xFFF)

#define SDR0_CUST0_CHIPSELGAT_MASK	0x0000000F /* Chip Sel Gating Mask */
#define SDR0_CUST0_CHIPSELGAT_DIS	0x00000000 /* Chip Sel Gating Disable */
#define SDR0_CUST0_CHIPSELGAT_ENALL  0x0000000F /* All Chip Sel Gating Enable */
#define SDR0_CUST0_CHIPSELGAT_EN0	0x00000008 /* Chip Sel0 Gating Enable */
#define SDR0_CUST0_CHIPSELGAT_EN1	0x00000004 /* Chip Sel1 Gating Enable */
#define SDR0_CUST0_CHIPSELGAT_EN2	0x00000002 /* Chip Sel2 Gating Enable */
#define SDR0_CUST0_CHIPSELGAT_EN3	0x00000001 /* Chip Sel3 Gating Enable */

#define SDR0_PFC0		0x4100
#define SDR0_PFC1		0x4101
#define SDR0_PFC1_U1ME		0x02000000
#define SDR0_PFC1_U0ME		0x00080000
#define SDR0_PFC1_U0IM		0x00040000
#define SDR0_PFC1_SIS		0x00020000
#define SDR0_PFC1_DMAAEN	0x00010000
#define SDR0_PFC1_DMADEN	0x00008000
#define SDR0_PFC1_USBEN		0x00004000
#define SDR0_PFC1_AHBSWAP	0x00000020
#define SDR0_PFC1_USBBIGEN	0x00000010
#define SDR0_PFC1_GPT_FREQ	0x0000000f
#endif

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

#endif	/* __PPC405_H__ */
