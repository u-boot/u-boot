/*
 *(C) Copyright 2005-2009 Netstal Maschinen AG
 *    Bruno Hars (Bruno.Hars@netstal.com)
 *    Niklaus Giger (Niklaus.Giger@netstal.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * reginfo.c - register dump of HW-configuratin register for PPC4xx based board
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx-uic.h>
#include <asm/ppc4xx-emac.h>

enum REGISTER_TYPE {
	IDCR1,	/* Indirectly Accessed DCR via SDRAM0_CFGADDR/SDRAM0_CFGDATA */
	IDCR2,	/* Indirectly Accessed DCR via EBC0_CFGADDR/EBC0_CFGDATA */
	IDCR3,	/* Indirectly Accessed DCR via EBM0_CFGADDR/EBM0_CFGDATA */
	IDCR4,	/* Indirectly Accessed DCR via PPM0_CFGADDR/PPM0_CFGDATA */
	IDCR5,	/* Indirectly Accessed DCR via CPR0_CFGADDR/CPR0_CFGDATA */
	IDCR6,	/* Indirectly Accessed DCR via SDR0_CFGADDR/SDR0_CFGDATA */
	MM	/* Directly Accessed MMIO Register */
};

struct cpu_register {
	char *name;
	enum REGISTER_TYPE type;
	u32 address;
};

/*
 * PPC440EPx registers ordered for output
 * name           type    addr            size
 * -------------------------------------------
 */

const struct cpu_register ppc4xx_reg[] = {
	{"PB0CR",		IDCR2,	PB0CR},
	{"PB0AP",		IDCR2,	PB0AP},
	{"PB1CR",		IDCR2,	PB1CR},
	{"PB1AP",		IDCR2,	PB1AP},
	{"PB2CR",		IDCR2,	PB2CR},
	{"PB2AP",		IDCR2,	PB2AP},
	{"PB3CR",		IDCR2,	PB3CR},
	{"PB3AP",		IDCR2,	PB3AP},

	{"PB4CR",		IDCR2,	PB4CR},
	{"PB4AP",		IDCR2,	PB4AP},
#if !defined(CONFIG_405EP)
	{"PB5CR",		IDCR2,	PB5CR},
	{"PB5AP",		IDCR2,	PB5AP},
	{"PB6CR",		IDCR2,	PB6CR},
	{"PB6AP",		IDCR2,	PB6AP},
	{"PB7CR",		IDCR2,	PB7CR},
	{"PB7AP",		IDCR2,	PB7AP},
#endif

	{"PBEAR",		IDCR2,	PBEAR},
#if defined(CONFIG_405EP) || defined (CONFIG_405GP)
	{"PBESR0",		IDCR2,	PBESR0},
	{"PBESR1",		IDCR2,	PBESR1},
#endif
	{"EBC0_CFG",		IDCR2,	EBC0_CFG},

#ifdef CONFIG_405GP
	{"SDRAM0_BESR0",	IDCR1,	SDRAM0_BESR0},
	{"SDRAM0_BESRS0",	IDCR1,	SDRAM0_BESRS0},
	{"SDRAM0_BESR1",	IDCR1,	SDRAM0_BESR1},
	{"SDRAM0_BESRS1",	IDCR1,	SDRAM0_BESRS1},
	{"SDRAM0_BEAR",		IDCR1,	SDRAM0_BEAR},
	{"SDRAM0_CFG",		IDCR1,	SDRAM0_CFG},
	{"SDRAM0_RTR",		IDCR1,	SDRAM0_RTR},
	{"SDRAM0_PMIT",		IDCR1,	SDRAM0_PMIT},

	{"SDRAM0_B0CR",		IDCR1,	SDRAM0_B0CR},
	{"SDRAM0_B1CR",		IDCR1,	SDRAM0_B1CR},
	{"SDRAM0_B2CR",		IDCR1,	SDRAM0_B2CR},
	{"SDRAM0_B3CR",		IDCR1,	SDRAM0_B1CR},
	{"SDRAM0_TR",		IDCR1,	SDRAM0_TR},
	{"SDRAM0_ECCCFG",	IDCR1,	SDRAM0_B1CR},
	{"SDRAM0_ECCESR",	IDCR1,	SDRAM0_ECCESR},


#endif

#ifdef CONFIG_440EPX
	{"SDR0_SDSTP0",		IDCR6,	SDR0_SDSTP0},
	{"SDR0_SDSTP1",		IDCR6,	SDR0_SDSTP1},
	{"SDR0_SDSTP2",		IDCR6,	SDR0_SDSTP2},
	{"SDR0_SDSTP3",		IDCR6,	SDR0_SDSTP3},
	{"SDR0_CUST0",		IDCR6,	SDR0_CUST0},
	{"SDR0_CUST1",		IDCR6,	SDR0_CUST1},
	{"SDR0_EBC",		IDCR6,	SDR0_EBC},
	{"SDR0_AMP0",		IDCR6,	SDR0_AMP0},
	{"SDR0_AMP1",		IDCR6,	SDR0_AMP1},
	{"SDR0_CP440",		IDCR6,	SDR0_CP440},
	{"SDR0_CRYP0",		IDCR6,	SDR0_CRYP0},
	{"SDR0_DDRCFG",		IDCR6,	SDR0_DDRCFG},
	{"SDR0_EMAC0RXST",	IDCR6,	SDR0_EMAC0RXST},
	{"SDR0_EMAC0TXST",	IDCR6,	SDR0_EMAC0TXST},
	{"SDR0_MFR",		IDCR6,	SDR0_MFR},
	{"SDR0_PCI0",		IDCR6,	SDR0_PCI0},
	{"SDR0_PFC0",		IDCR6,	SDR0_PFC0},
	{"SDR0_PFC1",		IDCR6,	SDR0_PFC1},
	{"SDR0_PFC2",		IDCR6,	SDR0_PFC2},
	{"SDR0_PFC4",		IDCR6,	SDR0_PFC4},
	{"SDR0_UART0",		IDCR6,	SDR0_UART0},
	{"SDR0_UART1",		IDCR6,	SDR0_UART1},
	{"SDR0_UART2",		IDCR6,	SDR0_UART2},
	{"SDR0_UART3",		IDCR6,	SDR0_UART3},
	{"DDR0_02",		IDCR1,	DDR0_02},
	{"DDR0_00",		IDCR1,	DDR0_00},
	{"DDR0_01",		IDCR1,	DDR0_01},
	{"DDR0_03",		IDCR1,	DDR0_03},
	{"DDR0_04",		IDCR1,	DDR0_04},
	{"DDR0_05",		IDCR1,	DDR0_05},
	{"DDR0_06",		IDCR1,	DDR0_06},
	{"DDR0_07",		IDCR1,	DDR0_07},
	{"DDR0_08",		IDCR1,	DDR0_08},
	{"DDR0_09",		IDCR1,	DDR0_09},
	{"DDR0_10",		IDCR1,	DDR0_10},
	{"DDR0_11",		IDCR1,	DDR0_11},
	{"DDR0_12",		IDCR1,	DDR0_12},
	{"DDR0_14",		IDCR1,	DDR0_14},
	{"DDR0_17",		IDCR1,	DDR0_17},
	{"DDR0_18",		IDCR1,	DDR0_18},
	{"DDR0_19",		IDCR1,	DDR0_19},
	{"DDR0_20",		IDCR1,	DDR0_20},
	{"DDR0_21",		IDCR1,	DDR0_21},
	{"DDR0_22",		IDCR1,	DDR0_22},
	{"DDR0_23",		IDCR1,	DDR0_23},
	{"DDR0_24",		IDCR1,	DDR0_24},
	{"DDR0_25",		IDCR1,	DDR0_25},
	{"DDR0_26",		IDCR1,	DDR0_26},
	{"DDR0_27",		IDCR1,	DDR0_27},
	{"DDR0_28",		IDCR1,	DDR0_28},
	{"DDR0_31",		IDCR1,	DDR0_31},
	{"DDR0_32",		IDCR1,	DDR0_32},
	{"DDR0_33",		IDCR1,	DDR0_33},
	{"DDR0_34",		IDCR1,	DDR0_34},
	{"DDR0_35",		IDCR1,	DDR0_35},
	{"DDR0_36",		IDCR1,	DDR0_36},
	{"DDR0_37",		IDCR1,	DDR0_37},
	{"DDR0_38",		IDCR1,	DDR0_38},
	{"DDR0_39",		IDCR1,	DDR0_39},
	{"DDR0_40",		IDCR1,	DDR0_40},
	{"DDR0_41",		IDCR1,	DDR0_41},
	{"DDR0_42",		IDCR1,	DDR0_42},
	{"DDR0_43",		IDCR1,	DDR0_43},
	{"DDR0_44",		IDCR1,	DDR0_44},
	{"CPR0_ICFG",		IDCR5,	CPR0_ICFG},
	{"CPR0_MALD",		IDCR5,	CPR0_MALD},
	{"CPR0_OPBD00",		IDCR5,	CPR0_OPBD0},
	{"CPR0_PERD0",		IDCR5,	CPR0_PERD},
	{"CPR0_PLLC0",		IDCR5,	CPR0_PLLC},
	{"CPR0_PLLD0",		IDCR5,	CPR0_PLLD},
	{"CPR0_PRIMAD0",	IDCR5,	CPR0_PRIMAD0},
	{"CPR0_PRIMBD0",	IDCR5,	CPR0_PRIMBD0},
	{"CPR0_SPCID",		IDCR5,	CPR0_SPCID},
	{"SPI0_MODE",		MM,	SPI0_MODE},
	{"IIC0_CLKDIV",		MM,	PCIL0_PMM1MA},
	{"PCIL0_PMM0MA",	MM,	PCIL0_PMM0MA},
	{"PCIL0_PMM1MA",	MM,	PCIL0_PMM1MA},
	{"PCIL0_PTM1LA",	MM,	PCIL0_PMM1MA},
	{"PCIL0_PTM1MS",	MM,	PCIL0_PTM1MS},
	{"PCIL0_PTM2LA",	MM,	PCIL0_PMM1MA},
	{"PCIL0_PTM2MS",	MM,	PCIL0_PTM2MS},
	{"ZMII0_FER",		MM,	ZMII0_FER},
	{"ZMII0_SSR",		MM,	ZMII0_SSR},
	{"EMAC0_IPGVR",		MM,	EMAC0_IPGVR},
	{"EMAC0_MR1",		MM,	EMAC0_MR1},
	{"EMAC0_PTR",		MM,	EMAC0_PTR},
	{"EMAC0_RWMR",		MM,	EMAC0_RWMR},
	{"EMAC0_STACR",		MM,	EMAC0_STACR},
	{"EMAC0_TMR0",		MM,	EMAC0_TMR0},
	{"EMAC0_TMR1",		MM,	EMAC0_TMR1},
	{"EMAC0_TRTR",		MM,	EMAC0_TRTR},
	{"EMAC1_MR1",		MM,	EMAC1_MR1},
	{"GPIO0_OR",		MM,	GPIO0_OR},
	{"GPIO1_OR",		MM,	GPIO1_OR},
	{"GPIO0_TCR",		MM,	GPIO0_TCR},
	{"GPIO1_TCR",		MM,	GPIO1_TCR},
	{"GPIO0_ODR",		MM,	GPIO0_ODR},
	{"GPIO1_ODR",		MM,	GPIO1_ODR},
	{"GPIO0_OSRL",		MM,	GPIO0_OSRL},
	{"GPIO0_OSRH",		MM,	GPIO0_OSRH},
	{"GPIO1_OSRL",		MM,	GPIO1_OSRL},
	{"GPIO1_OSRH",		MM,	GPIO1_OSRH},
	{"GPIO0_TSRL",		MM,	GPIO0_TSRL},
	{"GPIO0_TSRH",		MM,	GPIO0_TSRH},
	{"GPIO1_TSRL",		MM,	GPIO1_TSRL},
	{"GPIO1_TSRH",		MM,	GPIO1_TSRH},
	{"GPIO0_IR",		MM,	GPIO0_IR},
	{"GPIO1_IR",		MM,	GPIO1_IR},
	{"GPIO0_ISR1L",		MM,	GPIO0_ISR1L},
	{"GPIO0_ISR1H",		MM,	GPIO0_ISR1H},
	{"GPIO1_ISR1L",		MM,	GPIO1_ISR1L},
	{"GPIO1_ISR1H",		MM,	GPIO1_ISR1H},
	{"GPIO0_ISR2L",		MM,	GPIO0_ISR2L},
	{"GPIO0_ISR2H",		MM,	GPIO0_ISR2H},
	{"GPIO1_ISR2L",		MM,	GPIO1_ISR2L},
	{"GPIO1_ISR2H",		MM,	GPIO1_ISR2H},
	{"GPIO0_ISR3L",		MM,	GPIO0_ISR3L},
	{"GPIO0_ISR3H",		MM,	GPIO0_ISR3H},
	{"GPIO1_ISR3L",		MM,	GPIO1_ISR3L},
	{"GPIO1_ISR3H",		MM,	GPIO1_ISR3H},
	{"SDR0_USB2PHY0CR",	IDCR6,	SDR0_USB2PHY0CR},
	{"SDR0_USB2H0CR",	IDCR6,	SDR0_USB2H0CR},
	{"SDR0_USB2D0CR",	IDCR6,	SDR0_USB2D0CR},
#endif
};

/*
 * CPU Register dump of PPC4xx HW configuration registers
 * Output: first all DCR-registers, then in order of struct ppc4xx_reg
 */
#define PRINT_DCR(dcr) 	printf("0x%08x %-16s: 0x%08x\n", dcr,#dcr, mfdcr(dcr));

void ppc4xx_reginfo(void)
{
	unsigned int i;
	unsigned int n;
	u32 value;
	enum REGISTER_TYPE type;
#if defined (CONFIG_405EP)
	printf("Dump PPC405EP HW configuration registers\n\n");
#elif CONFIG_405GP
	printf ("Dump 405GP HW configuration registers\n\n");
#elif CONFIG_440EPX
	printf("Dump PPC440EPx HW configuration registers\n\n");
#endif
	printf("MSR: 0x%08x\n", mfmsr());

	printf ("\nUniversal Interrupt Controller Regs\n");
	PRINT_DCR(UIC0SR);
	PRINT_DCR(UIC0ER);
	PRINT_DCR(UIC0CR);
	PRINT_DCR(UIC0PR);
	PRINT_DCR(UIC0TR);
	PRINT_DCR(UIC0MSR);
	PRINT_DCR(UIC0VR);
	PRINT_DCR(UIC0VCR);

#if (UIC_MAX > 1)
	PRINT_DCR(UIC2SR);
	PRINT_DCR(UIC2ER);
	PRINT_DCR(UIC2CR);
	PRINT_DCR(UIC2PR);
	PRINT_DCR(UIC2TR);
	PRINT_DCR(UIC2MSR);
	PRINT_DCR(UIC2VR);
	PRINT_DCR(UIC2VCR);
#endif

#if (UIC_MAX > 2)
	PRINT_DCR(UIC2SR);
	PRINT_DCR(UIC2ER);
	PRINT_DCR(UIC2CR);
	PRINT_DCR(UIC2PR);
	PRINT_DCR(UIC2TR);
	PRINT_DCR(UIC2MSR);
	PRINT_DCR(UIC2VR);
	PRINT_DCR(UIC2VCR);
#endif

#if (UIC_MAX > 3)
	PRINT_DCR(UIC3SR);
	PRINT_DCR(UIC3ER);
	PRINT_DCR(UIC3CR);
	PRINT_DCR(UIC3PR);
	PRINT_DCR(UIC3TR);
	PRINT_DCR(UIC3MSR);
	PRINT_DCR(UIC3VR);
	PRINT_DCR(UIC3VCR);
#endif

#if defined (CONFIG_405EP) || defined (CONFIG_405GP)
	printf ("\n\nDMA Channels\n");
	PRINT_DCR(DMASR);
	PRINT_DCR(DMASGC);
	PRINT_DCR(DMAADR);

	PRINT_DCR(DMACR0);
	PRINT_DCR(DMACT0);
	PRINT_DCR(DMADA0);
	PRINT_DCR(DMASA0);
	PRINT_DCR(DMASB0);

	PRINT_DCR(DMACR1);
	PRINT_DCR(DMACT1);
	PRINT_DCR(DMADA1);
	PRINT_DCR(DMASA1);
	PRINT_DCR(DMASB1);

	PRINT_DCR(DMACR2);
	PRINT_DCR(DMACT2);
	PRINT_DCR(DMADA2);
	PRINT_DCR(DMASA2);
	PRINT_DCR(DMASB2);

	PRINT_DCR(DMACR3);
	PRINT_DCR(DMACT3);
	PRINT_DCR(DMADA3);
	PRINT_DCR(DMASA3);
	PRINT_DCR(DMASB3);
#endif

	printf ("\n\nVarious HW-Configuration registers\n");
#if defined (CONFIG_440EPX)
	PRINT_DCR(MAL0_CFG);
	PRINT_DCR(CPM0_ER);
	PRINT_DCR(CPM1_ER);
	PRINT_DCR(PLB4A0_ACR);
	PRINT_DCR(PLB4A1_ACR);
	PRINT_DCR(PLB3A0_ACR);
	PRINT_DCR(OPB2PLB40_BCTRL);
	PRINT_DCR(P4P3BO0_CFG);
#endif
	n = sizeof(ppc4xx_reg) / sizeof(ppc4xx_reg[0]);
	for (i = 0; i < n; i++) {
		value = 0;
		type = ppc4xx_reg[i].type;
		switch (type) {
		case IDCR1:	/* Indirect via SDRAM0_CFGADDR/DDR0_CFGDATA */
			mtdcr(SDRAM0_CFGADDR, ppc4xx_reg[i].address);
			value = mfdcr(SDRAM0_CFGDATA);
			break;
		case IDCR2:	/* Indirect via EBC0_CFGADDR/EBC0_CFGDATA */
			mtdcr(EBC0_CFGADDR, ppc4xx_reg[i].address);
			value = mfdcr(EBC0_CFGDATA);
			break;
		case IDCR5:	/* Indirect via CPR0_CFGADDR/CPR0_CFGDATA */
			mtdcr(CPR0_CFGADDR, ppc4xx_reg[i].address);
			value = mfdcr(CPR0_CFGDATA);
			break;
		case IDCR6:	/* Indirect via SDR0_CFGADDR/SDR0_CFGDATA */
			mtdcr(SDR0_CFGADDR, ppc4xx_reg[i].address);
			value = mfdcr(SDR0_CFGDATA);
			break;
		case MM:	/* Directly Accessed MMIO Register */
			value = in_be32((const volatile unsigned __iomem *)
				ppc4xx_reg[i].address);
			break;
		default:
			printf("\nERROR: struct entry %d: unknown register"
				"type\n", i);
			break;
		}
		printf("0x%08x %-16s: 0x%08x\n",ppc4xx_reg[i].address,
			ppc4xx_reg[i].name, value);
	}
}
