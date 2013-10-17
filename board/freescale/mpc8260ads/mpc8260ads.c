/*
 * (C) Copyright 2001-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Modified during 2001 by
 * Advanced Communications Technologies (Australia) Pty. Ltd.
 * Howard Walker, Tuong Vu-Dinh
 *
 * (C) Copyright 2001, Stuart Hughes, Lineo Inc, stuarth@lineo.com
 * Added support for the 16M dram simm on the 8260ads boards
 *
 * (C) Copyright 2003-2004 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 * Added support for SDRAM DIMMs SPD EEPROM, MII, Ethernet PHY init.
 *
 * Copyright (c) 2005 MontaVista Software, Inc.
 * Vitaly Bordug <vbordug@ru.mvista.com>
 * Added support for PCI.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <ioports.h>
#include <mpc8260.h>
#include <asm/m8260_pci.h>
#include <i2c.h>
#include <spd.h>
#include <miiphy.h>
#ifdef CONFIG_PCI
#include <pci.h>
#endif
#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#include <fdt_support.h>
#endif

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

#define CONFIG_SYS_FCC1 (CONFIG_ETHER_INDEX == 1)
#define CONFIG_SYS_FCC2 (CONFIG_ETHER_INDEX == 2)
#define CONFIG_SYS_FCC3 (CONFIG_ETHER_INDEX == 3)

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {	/*	      conf      ppar psor pdir podr pdat */
	/* PA31 */ { CONFIG_SYS_FCC1,   1,   1,   0,   0,   0 }, /* FCC1 MII COL   */
	/* PA30 */ { CONFIG_SYS_FCC1,   1,   1,   0,   0,   0 }, /* FCC1 MII CRS   */
	/* PA29 */ { CONFIG_SYS_FCC1,   1,   1,   1,   0,   0 }, /* FCC1 MII TX_ER */
	/* PA28 */ { CONFIG_SYS_FCC1,   1,   1,   1,   0,   0 }, /* FCC1 MII TX_EN */
	/* PA27 */ { CONFIG_SYS_FCC1,   1,   1,   0,   0,   0 }, /* FCC1 MII RX_DV */
	/* PA26 */ { CONFIG_SYS_FCC1,   1,   1,   0,   0,   0 }, /* FCC1 MII RX_ER */
	/* PA25 */ { 0,          0,   0,   0,   0,   0 }, /* PA25 */
	/* PA24 */ { 0,          0,   0,   0,   0,   0 }, /* PA24 */
	/* PA23 */ { 0,          0,   0,   0,   0,   0 }, /* PA23 */
	/* PA22 */ { 0,          0,   0,   0,   0,   0 }, /* PA22 */
	/* PA21 */ { CONFIG_SYS_FCC1,   1,   0,   1,   0,   0 }, /* FCC1 MII TxD[3] */
	/* PA20 */ { CONFIG_SYS_FCC1,   1,   0,   1,   0,   0 }, /* FCC1 MII TxD[2] */
	/* PA19 */ { CONFIG_SYS_FCC1,   1,   0,   1,   0,   0 }, /* FCC1 MII TxD[1] */
	/* PA18 */ { CONFIG_SYS_FCC1,   1,   0,   1,   0,   0 }, /* FCC1 MII TxD[0] */
	/* PA17 */ { CONFIG_SYS_FCC1,   1,   0,   0,   0,   0 }, /* FCC1 MII RxD[0] */
	/* PA16 */ { CONFIG_SYS_FCC1,   1,   0,   0,   0,   0 }, /* FCC1 MII RxD[1] */
	/* PA15 */ { CONFIG_SYS_FCC1,   1,   0,   0,   0,   0 }, /* FCC1 MII RxD[2] */
	/* PA14 */ { CONFIG_SYS_FCC1,   1,   0,   0,   0,   0 }, /* FCC1 MII RxD[3] */
	/* PA13 */ { 0,          0,   0,   0,   0,   0 }, /* PA13 */
	/* PA12 */ { 0,          0,   0,   0,   0,   0 }, /* PA12 */
	/* PA11 */ { 0,          0,   0,   0,   0,   0 }, /* PA11 */
	/* PA10 */ { 0,          0,   0,   0,   0,   0 }, /* PA10 */
	/* PA9  */ { 0,          0,   0,   0,   0,   0 }, /* PA9 */
	/* PA8  */ { 0,          0,   0,   0,   0,   0 }, /* PA8 */
	/* PA7  */ { 0,          0,   0,   1,   0,   0 }, /* PA7 */
	/* PA6  */ { 0,          0,   0,   0,   0,   0 }, /* PA6 */
	/* PA5  */ { 0,          0,   0,   1,   0,   0 }, /* PA5 */
	/* PA4  */ { 0,          0,   0,   1,   0,   0 }, /* PA4 */
	/* PA3  */ { 0,          0,   0,   1,   0,   0 }, /* PA3 */
	/* PA2  */ { 0,          0,   0,   1,   0,   0 }, /* PA2 */
	/* PA1  */ { 0,          0,   0,   0,   0,   0 }, /* PA1 */
	/* PA0  */ { 0,          0,   0,   1,   0,   0 }  /* PA0 */
    },

    /* Port B configuration */
    {   /*	      conf      ppar psor pdir podr pdat */
	/* PB31 */ { CONFIG_SYS_FCC2,   1,   0,   1,   0,   0 }, /* FCC2 MII TX_ER */
	/* PB30 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RX_DV */
	/* PB29 */ { CONFIG_SYS_FCC2,   1,   1,   1,   0,   0 }, /* FCC2 MII TX_EN */
	/* PB28 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RX_ER */
	/* PB27 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII COL */
	/* PB26 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII CRS */
	/* PB25 */ { CONFIG_SYS_FCC2,   1,   0,   1,   0,   0 }, /* FCC2 MII TxD[3] */
	/* PB24 */ { CONFIG_SYS_FCC2,   1,   0,   1,   0,   0 }, /* FCC2 MII TxD[2] */
	/* PB23 */ { CONFIG_SYS_FCC2,   1,   0,   1,   0,   0 }, /* FCC2 MII TxD[1] */
	/* PB22 */ { CONFIG_SYS_FCC2,   1,   0,   1,   0,   0 }, /* FCC2 MII TxD[0] */
	/* PB21 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RxD[0] */
	/* PB20 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RxD[1] */
	/* PB19 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RxD[2] */
	/* PB18 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RxD[3] */
	/* PB17 */ { CONFIG_SYS_FCC3,   1,   0,   0,   0,   0 }, /* FCC3:RX_DIV */
	/* PB16 */ { CONFIG_SYS_FCC3,   1,   0,   0,   0,   0 }, /* FCC3:RX_ERR */
	/* PB15 */ { CONFIG_SYS_FCC3,   1,   0,   1,   0,   0 }, /* FCC3:TX_ERR */
	/* PB14 */ { CONFIG_SYS_FCC3,   1,   0,   1,   0,   0 }, /* FCC3:TX_EN */
	/* PB13 */ { CONFIG_SYS_FCC3,   1,   0,   0,   0,   0 }, /* FCC3:COL */
	/* PB12 */ { CONFIG_SYS_FCC3,   1,   0,   0,   0,   0 }, /* FCC3:CRS */
	/* PB11 */ { CONFIG_SYS_FCC3,   1,   0,   0,   0,   0 }, /* FCC3:RXD */
	/* PB10 */ { CONFIG_SYS_FCC3,   1,   0,   0,   0,   0 }, /* FCC3:RXD */
	/* PB9  */ { CONFIG_SYS_FCC3,   1,   0,   0,   0,   0 }, /* FCC3:RXD */
	/* PB8  */ { CONFIG_SYS_FCC3,   1,   0,   0,   0,   0 }, /* FCC3:RXD */
	/* PB7  */ { CONFIG_SYS_FCC3,   1,   0,   1,   0,   0 }, /* FCC3:TXD */
	/* PB6  */ { CONFIG_SYS_FCC3,   1,   0,   1,   0,   0 }, /* FCC3:TXD */
	/* PB5  */ { CONFIG_SYS_FCC3,   1,   0,   1,   0,   0 }, /* FCC3:TXD */
	/* PB4  */ { CONFIG_SYS_FCC3,   1,   0,   1,   0,   0 }, /* FCC3:TXD */
	/* PB3  */ { 0,          0,   0,   0,   0,   0 }, /* pin doesn't exist */
	/* PB2  */ { 0,          0,   0,   0,   0,   0 }, /* pin doesn't exist */
	/* PB1  */ { 0,          0,   0,   0,   0,   0 }, /* pin doesn't exist */
	/* PB0  */ { 0,          0,   0,   0,   0,   0 }  /* pin doesn't exist */
    },

    /* Port C */
    {   /*	      conf      ppar psor pdir podr pdat */
	/* PC31 */ { 0,          0,   0,   0,   0,   0 }, /* PC31 */
	/* PC30 */ { 0,          0,   0,   0,   0,   0 }, /* PC30 */
	/* PC29 */ { 0,          0,   0,   0,   0,   0 }, /* PC29 */
	/* PC28 */ { 0,          0,   0,   0,   0,   0 }, /* PC28 */
	/* PC27 */ { 0,          0,   0,   0,   0,   0 }, /* PC27 */
	/* PC26 */ { 0,          0,   0,   0,   0,   0 }, /* PC26 */
	/* PC25 */ { 0,          0,   0,   0,   0,   0 }, /* PC25 */
	/* PC24 */ { 0,          0,   0,   0,   0,   0 }, /* PC24 */
	/* PC23 */ { 0,          0,   0,   0,   0,   0 }, /* PC23 */
	/* PC22 */ { CONFIG_SYS_FCC1,   1,   0,   0,   0,   0 }, /* FCC1 MII Tx Clock (CLK10) */
	/* PC21 */ { CONFIG_SYS_FCC1,   1,   0,   0,   0,   0 }, /* FCC1 MII Rx Clock (CLK11) */
	/* PC20 */ { 0,          0,   0,   0,   0,   0 }, /* PC20 */
#if CONFIG_ADSTYPE == CONFIG_SYS_8272ADS
	/* PC19 */ { 1,          0,   0,   1,   0,   0 }, /* FETHMDC  */
	/* PC18 */ { 1,          0,   0,   0,   0,   0 }, /* FETHMDIO */
	/* PC17 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII Rx Clock (CLK15) */
	/* PC16 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII Tx Clock (CLK16) */
#else
	/* PC19 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII Rx Clock (CLK13) */
	/* PC18 */ { CONFIG_SYS_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII Tx Clock (CLK14) */
	/* PC17 */ { 0,          0,   0,   0,   0,   0 }, /* PC17 */
	/* PC16 */ { 0,          0,   0,   0,   0,   0 }, /* PC16 */
#endif /* CONFIG_ADSTYPE == CONFIG_SYS_8272ADS */
	/* PC15 */ { 0,          0,   0,   0,   0,   0 }, /* PC15 */
	/* PC14 */ { 0,          0,   0,   0,   0,   0 }, /* PC14 */
	/* PC13 */ { 0,          0,   0,   0,   0,   0 }, /* PC13 */
	/* PC12 */ { 0,          0,   0,   0,   0,   0 }, /* PC12 */
	/* PC11 */ { 0,          0,   0,   0,   0,   0 }, /* PC11 */
#if CONFIG_ADSTYPE == CONFIG_SYS_8272ADS
	/* PC10 */ { 0,          0,   0,   0,   0,   0 }, /* PC10 */
	/* PC9  */ { 0,          0,   0,   0,   0,   0 }, /* PC9  */
#else
	/* PC10 */ { 1,          0,   0,   1,   0,   0 }, /* FETHMDC  */
	/* PC9  */ { 1,          0,   0,   0,   0,   0 }, /* FETHMDIO */
#endif /* CONFIG_ADSTYPE == CONFIG_SYS_8272ADS */
	/* PC8  */ { 0,          0,   0,   0,   0,   0 }, /* PC8 */
	/* PC7  */ { 0,          0,   0,   0,   0,   0 }, /* PC7 */
	/* PC6  */ { 0,          0,   0,   0,   0,   0 }, /* PC6 */
	/* PC5  */ { 0,          0,   0,   0,   0,   0 }, /* PC5 */
	/* PC4  */ { 0,          0,   0,   0,   0,   0 }, /* PC4 */
	/* PC3  */ { 0,          0,   0,   0,   0,   0 }, /* PC3 */
	/* PC2  */ { 0,          0,   0,   0,   0,   0 }, /* PC2 */
	/* PC1  */ { 0,          0,   0,   0,   0,   0 }, /* PC1 */
	/* PC0  */ { 0,          0,   0,   0,   0,   0 }, /* PC0 */
    },

    /* Port D */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 UART RxD */
	/* PD30 */ {   1,   1,   1,   1,   0,   0   }, /* SCC1 UART TxD */
	/* PD29 */ {   0,   0,   0,   0,   0,   0   }, /* PD29 */
	/* PD28 */ {   0,   1,   0,   0,   0,   0   }, /* PD28 */
	/* PD27 */ {   0,   1,   1,   1,   0,   0   }, /* PD27 */
	/* PD26 */ {   0,   0,   0,   1,   0,   0   }, /* PD26 */
	/* PD25 */ {   0,   0,   0,   1,   0,   0   }, /* PD25 */
	/* PD24 */ {   0,   0,   0,   1,   0,   0   }, /* PD24 */
	/* PD23 */ {   0,   0,   0,   1,   0,   0   }, /* PD23 */
	/* PD22 */ {   0,   0,   0,   1,   0,   0   }, /* PD22 */
	/* PD21 */ {   0,   0,   0,   1,   0,   0   }, /* PD21 */
	/* PD20 */ {   0,   0,   0,   1,   0,   0   }, /* PD20 */
	/* PD19 */ {   0,   0,   0,   1,   0,   0   }, /* PD19 */
	/* PD18 */ {   0,   0,   0,   1,   0,   0   }, /* PD18 */
	/* PD17 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXPRTY */
	/* PD16 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXPRTY */
	/* PD15 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SCL */
	/* PD13 */ {   0,   0,   0,   0,   0,   0   }, /* PD13 */
	/* PD12 */ {   0,   0,   0,   0,   0,   0   }, /* PD12 */
	/* PD11 */ {   0,   0,   0,   0,   0,   0   }, /* PD11 */
	/* PD10 */ {   0,   0,   0,   0,   0,   0   }, /* PD10 */
	/* PD9  */ {   0,   1,   0,   1,   0,   0   }, /* SMC1 TXD */
	/* PD8  */ {   0,   1,   0,   0,   0,   0   }, /* SMC1 RXD */
	/* PD7  */ {   0,   0,   0,   1,   0,   1   }, /* PD7 */
	/* PD6  */ {   0,   0,   0,   1,   0,   1   }, /* PD6 */
	/* PD5  */ {   0,   0,   0,   1,   0,   1   }, /* PD5 */
	/* PD4  */ {   0,   0,   0,   1,   0,   1   }, /* PD4 */
	/* PD3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    }
};

void reset_phy (void)
{
	vu_long *bcsr = (vu_long *)CONFIG_SYS_BCSR;

	/* Reset the PHY */
#if CONFIG_SYS_PHY_ADDR == 0
	bcsr[1] &= ~(FETHIEN1 | FETH1_RST);
	udelay(2);
	bcsr[1] |=  FETH1_RST;
#else
	bcsr[3] &= ~(FETHIEN2 | FETH2_RST);
	udelay(2);
	bcsr[3] |=  FETH2_RST;
#endif /* CONFIG_SYS_PHY_ADDR == 0 */
	udelay(1000);
#ifdef CONFIG_MII
#if CONFIG_ADSTYPE >= CONFIG_SYS_PQ2FADS
	/*
	 * Do not bypass Rx/Tx (de)scrambler (fix configuration error)
	 * Enable autonegotiation.
	 */
	bb_miiphy_write(NULL, CONFIG_SYS_PHY_ADDR, 16, 0x610);
	bb_miiphy_write(NULL, CONFIG_SYS_PHY_ADDR, MII_BMCR,
			BMCR_ANENABLE | BMCR_ANRESTART);
#else
	/*
	 * Ethernet PHY is configured (by means of configuration pins)
	 * to work at 10Mb/s only. We reconfigure it using MII
	 * to advertise all capabilities, including 100Mb/s, and
	 * restart autonegotiation.
	 */

	/* Advertise all capabilities */
	bb_miiphy_write(NULL, CONFIG_SYS_PHY_ADDR, MII_ADVERTISE, 0x01E1);

	/* Do not bypass Rx/Tx (de)scrambler */
	bb_miiphy_write(NULL, CONFIG_SYS_PHY_ADDR, MII_FCSCOUNTER,  0x0000);

	bb_miiphy_write(NULL, CONFIG_SYS_PHY_ADDR, MII_BMCR,
			BMCR_ANENABLE | BMCR_ANRESTART);
#endif /* CONFIG_ADSTYPE == CONFIG_SYS_PQ2FADS */
#endif /* CONFIG_MII */
}

#ifdef CONFIG_PCI
typedef struct pci_ic_s {
	unsigned long pci_int_stat;
	unsigned long pci_int_mask;
}pci_ic_t;
#endif

int board_early_init_f (void)
{
	vu_long *bcsr = (vu_long *)CONFIG_SYS_BCSR;

#ifdef CONFIG_PCI
	volatile pci_ic_t* pci_ic = (pci_ic_t *) CONFIG_SYS_PCI_INT;

	/* mask alll the PCI interrupts */
	pci_ic->pci_int_mask |= 0xfff00000;
#endif
#if (CONFIG_CONS_INDEX == 1) || (CONFIG_KGDB_INDEX == 1)
	bcsr[1] &= ~RS232EN_1;
#endif
#if (CONFIG_CONS_INDEX > 1) || (CONFIG_KGDB_INDEX > 1)
	bcsr[1] &= ~RS232EN_2;
#endif

#if CONFIG_ADSTYPE != CONFIG_SYS_8260ADS /* PCI mode can be selected */
#if CONFIG_ADSTYPE == CONFIG_SYS_PQ2FADS
	if ((bcsr[3] & BCSR_PCI_MODE) == 0) /* PCI mode selected by JP9 */
#endif /* CONFIG_ADSTYPE == CONFIG_SYS_PQ2FADS */
	{
		volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

		immap->im_clkrst.car_sccr |= M826X_SCCR_PCI_MODE_EN;
		immap->im_siu_conf.sc_siumcr =
			(immap->im_siu_conf.sc_siumcr & ~SIUMCR_LBPC11)
			| SIUMCR_LBPC01;
	}
#endif /* CONFIG_ADSTYPE != CONFIG_SYS_8260ADS */

	return 0;
}

#define ns2clk(ns) (ns / (1000000000 / CONFIG_8260_CLKIN) + 1)

phys_size_t initdram (int board_type)
{
#if   CONFIG_ADSTYPE == CONFIG_SYS_PQ2FADS
	long int msize = 32;
#elif CONFIG_ADSTYPE == CONFIG_SYS_8272ADS
	long int msize = 64;
#else
	long int msize = 16;
#endif

#ifndef CONFIG_SYS_RAMBOOT
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	volatile uchar *ramaddr, c = 0xff;
	uint or;
	uint psdmr;
	uint psrt;

	int i;

	immap->im_siu_conf.sc_ppc_acr  = 0x00000002;
	immap->im_siu_conf.sc_ppc_alrh = 0x01267893;
	immap->im_siu_conf.sc_tescr1   = 0x00004000;

	memctl->memc_mptpr = CONFIG_SYS_MPTPR;
#ifdef CONFIG_SYS_LSDRAM_BASE
	/*
	  Initialise local bus SDRAM only if the pins
	  are configured as local bus pins and not as PCI.
	  The configuration is determined by the HRCW.
	*/
	if ((immap->im_siu_conf.sc_siumcr & SIUMCR_LBPC11) == SIUMCR_LBPC00) {
		memctl->memc_lsrt  = CONFIG_SYS_LSRT;
#if CONFIG_ADSTYPE == CONFIG_SYS_PQ2FADS /* CS3 */
		memctl->memc_or3   = 0xFF803280;
		memctl->memc_br3   = CONFIG_SYS_LSDRAM_BASE | 0x00001861;
#else				  /* CS4 */
		memctl->memc_or4   = 0xFFC01480;
		memctl->memc_br4   = CONFIG_SYS_LSDRAM_BASE | 0x00001861;
#endif /* CONFIG_ADSTYPE == CONFIG_SYS_PQ2FADS */
		memctl->memc_lsdmr = CONFIG_SYS_LSDMR | 0x28000000;
		ramaddr = (uchar *) CONFIG_SYS_LSDRAM_BASE;
		*ramaddr = c;
		memctl->memc_lsdmr = CONFIG_SYS_LSDMR | 0x08000000;
		for (i = 0; i < 8; i++)
			*ramaddr = c;
		memctl->memc_lsdmr = CONFIG_SYS_LSDMR | 0x18000000;
		*ramaddr = c;
		memctl->memc_lsdmr = CONFIG_SYS_LSDMR | 0x40000000;
	}
#endif /* CONFIG_SYS_LSDRAM_BASE */

	/* Init 60x bus SDRAM */
#ifdef CONFIG_SPD_EEPROM
	{
		spd_eeprom_t spd;
		uint pbi, bsel, rowst, lsb, tmp;

		i2c_read (CONFIG_SPD_ADDR, 0, 1, (uchar *) & spd, sizeof (spd));

		/* Bank-based interleaving is not supported for physical bank
		   sizes greater than 128MB which is encoded as 0x20 in SPD
		 */
		pbi = (spd.row_dens > 32) ? 1 : CONFIG_SDRAM_PBI;
		msize = spd.nrows * (4 * spd.row_dens);	/* Mixed size not supported */
		or = ~(msize - 1) << 20;	/* SDAM */
		switch (spd.nbanks) {	/* BPD */
		case 2:
			bsel = 1;
			break;
		case 4:
			bsel = 2;
			or |= 0x00002000;
			break;
		case 8:
			bsel = 3;
			or |= 0x00004000;
			break;
		}
		lsb = 3;	/* For 64-bit port, lsb is 3 bits */

		if (pbi) {	/* Bus partition depends on interleaving */
			rowst = 32 - (spd.nrow_addr + spd.ncol_addr + bsel + lsb);
			or |= (rowst << 9);	/* ROWST */
		} else {
			rowst = 32 - (spd.nrow_addr + spd.ncol_addr + lsb);
			or |= ((rowst * 2 - 12) << 9);	/* ROWST */
		}
		or |= ((spd.nrow_addr - 9) << 6);	/* NUMR */

		psdmr = (pbi << 31);	/* PBI */
		/* Bus multiplexing parameters */
		tmp = 32 - (lsb + spd.nrow_addr);	/* Tables 10-19 and 10-20 */
		psdmr |= ((tmp - (rowst - 5) - 13) << 24);	/* SDAM */
		psdmr |= ((tmp - 3 - 12) << 21);	/* BSMA */

		tmp = (31 - lsb - 10) - tmp;
		/* Pin connected to SDA10 is (31 - lsb - 10).
		   rowst is multiplexed over (32 - (lsb + spd.nrow_addr)),
		   so (rowst + tmp) alternates with AP.
		 */
		if (pbi)				/* Table 10-7 */
			psdmr |= ((10 - (rowst + tmp)) << 18);	/* SDA10 */
		else
			psdmr |= ((12 - (rowst + tmp)) << 18);	/* SDA10 */

		/* SDRAM device-specific parameters */
		tmp = ns2clk (70);	/* Refresh recovery is not in SPD, so assume 70ns */
		switch (tmp) {		/* RFRC */
		case 1:
		case 2:
			psdmr |= (1 << 15);
			break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			psdmr |= ((tmp - 2) << 15);
			break;
		default:
			psdmr |= (7 << 15);
		}
		psdmr |= (ns2clk (spd.trp) % 8 << 12);	/* PRETOACT */
		psdmr |= (ns2clk (spd.trcd) % 8 << 9);	/* ACTTORW */
		/* BL=0 because for 64-bit SDRAM burst length must be 4 */
		/* LDOTOPRE ??? */
		for (i = 0, tmp = spd.write_lat; (i < 4) && ((tmp & 1) == 0); i++)
			tmp >>= 1;
		switch (i) {			/* WRC */
		case 0:
		case 1:
			psdmr |= (1 << 4);
			break;
		case 2:
		case 3:
			psdmr |= (i << 4);
			break;
		}
		/* EAMUX=0 - no external address multiplexing */
		/* BUFCMD=0 - no external buffers */
		for (i = 1, tmp = spd.cas_lat; (i < 3) && ((tmp & 1) == 0); i++)
			tmp >>= 1;
		psdmr |= i;				/* CL */

		switch (spd.refresh & 0x7F) {
		case 1:
			tmp = 3900;
			break;
		case 2:
			tmp = 7800;
			break;
		case 3:
			tmp = 31300;
			break;
		case 4:
			tmp = 62500;
			break;
		case 5:
			tmp = 125000;
			break;
		default:
			tmp = 15625;
		}
		psrt = tmp / (1000000000 / CONFIG_8260_CLKIN *
				  ((memctl->memc_mptpr >> 8) + 1)) - 1;
#ifdef SPD_DEBUG
		printf ("\nDIMM type:       %-18.18s\n", spd.mpart);
		printf ("SPD size:        %d\n", spd.info_size);
		printf ("EEPROM size:     %d\n", 1 << spd.chip_size);
		printf ("Memory type:     %d\n", spd.mem_type);
		printf ("Row addr:        %d\n", spd.nrow_addr);
		printf ("Column addr:     %d\n", spd.ncol_addr);
		printf ("# of rows:       %d\n", spd.nrows);
		printf ("Row density:     %d\n", spd.row_dens);
		printf ("# of banks:      %d\n", spd.nbanks);
		printf ("Data width:      %d\n",
				256 * spd.dataw_msb + spd.dataw_lsb);
		printf ("Chip width:      %d\n", spd.primw);
		printf ("Refresh rate:    %02X\n", spd.refresh);
		printf ("CAS latencies:   %02X\n", spd.cas_lat);
		printf ("Write latencies: %02X\n", spd.write_lat);
		printf ("tRP:             %d\n", spd.trp);
		printf ("tRCD:            %d\n", spd.trcd);

		printf ("OR=%X, PSDMR=%08X, PSRT=%0X\n", or, psdmr, psrt);
#endif /* SPD_DEBUG */
	}
#else  /* !CONFIG_SPD_EEPROM */
	or    = CONFIG_SYS_OR2;
	psdmr = CONFIG_SYS_PSDMR;
	psrt  = CONFIG_SYS_PSRT;
#endif /* CONFIG_SPD_EEPROM */
	memctl->memc_psrt = psrt;
	memctl->memc_or2 = or;
	memctl->memc_br2 = CONFIG_SYS_SDRAM_BASE | 0x00000041;
	ramaddr = (uchar *) CONFIG_SYS_SDRAM_BASE;
	memctl->memc_psdmr = psdmr | 0x28000000;	/* Precharge all banks */
	*ramaddr = c;
	memctl->memc_psdmr = psdmr | 0x08000000;	/* CBR refresh */
	for (i = 0; i < 8; i++)
		*ramaddr = c;

	memctl->memc_psdmr = psdmr | 0x18000000;	/* Mode Register write */
	*ramaddr = c;
	memctl->memc_psdmr = psdmr | 0x40000000;	/* Refresh enable */
	*ramaddr = c;
#endif /* CONFIG_SYS_RAMBOOT */

	/* return total 60x bus SDRAM size */
	return (msize * 1024 * 1024);
}

int checkboard (void)
{
#if   CONFIG_ADSTYPE == CONFIG_SYS_8260ADS
	puts ("Board: Motorola MPC8260ADS\n");
#elif CONFIG_ADSTYPE == CONFIG_SYS_8266ADS
	puts ("Board: Motorola MPC8266ADS\n");
#elif CONFIG_ADSTYPE == CONFIG_SYS_PQ2FADS
	puts ("Board: Motorola PQ2FADS-ZU\n");
#elif CONFIG_ADSTYPE == CONFIG_SYS_8272ADS
	puts ("Board: Motorola MPC8272ADS\n");
#else
	puts ("Board: unknown\n");
#endif
	return 0;
}

#ifdef CONFIG_PCI
struct pci_controller hose;

extern void pci_mpc8250_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc8250_init(&hose);
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
}
#endif
