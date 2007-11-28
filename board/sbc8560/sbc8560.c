/*
 * (C) Copyright 2003,Motorola Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
 *
 * (C) Copyright 2004 Wind River Systems Inc <www.windriver.com>.
 * Added support for Wind River SBC8560 board
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


extern long int spd_sdram (void);

#include <common.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <ioports.h>
#include <spd.h>
#include <miiphy.h>

long int fixed_sdram (void);

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {   /*            conf ppar psor pdir podr pdat */
	/* PA31 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 TxENB */
	/* PA30 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 TxClav   */
	/* PA29 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 TxSOC  */
	/* PA28 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 RxENB */
	/* PA27 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 RxSOC */
	/* PA26 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 RxClav */
	/* PA25 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[0] */
	/* PA24 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[1] */
	/* PA23 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[2] */
	/* PA22 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[3] */
	/* PA21 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[4] */
	/* PA20 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[5] */
	/* PA19 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[6] */
	/* PA18 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[7] */
	/* PA17 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[7] */
	/* PA16 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[6] */
	/* PA15 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[5] */
	/* PA14 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[4] */
	/* PA13 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[3] */
	/* PA12 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[2] */
	/* PA11 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[1] */
	/* PA10 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[0] */
	/* PA9  */ {   0,   1,   1,   1,   0,   0   }, /* FCC1 L1TXD */
	/* PA8  */ {   0,   1,   1,   0,   0,   0   }, /* FCC1 L1RXD */
	/* PA7  */ {   0,   0,   0,   1,   0,   0   }, /* PA7 */
	/* PA6  */ {   0,   1,   1,   1,   0,   0   }, /* TDM A1 L1RSYNC */
	/* PA5  */ {   0,   0,   0,   1,   0,   0   }, /* PA5 */
	/* PA4  */ {   0,   0,   0,   1,   0,   0   }, /* PA4 */
	/* PA3  */ {   0,   0,   0,   1,   0,   0   }, /* PA3 */
	/* PA2  */ {   0,   0,   0,   1,   0,   0   }, /* PA2 */
	/* PA1  */ {   1,   0,   0,   0,   0,   0   }, /* FREERUN */
	/* PA0  */ {   0,   0,   0,   1,   0,   0   }  /* PA0 */
    },

    /* Port B configuration */
    {   /*            conf ppar psor pdir podr pdat */
	/* PB31 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TX_ER */
	/* PB30 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_DV */
	/* PB29 */ {   1,   1,   1,   1,   0,   0   }, /* FCC2 MII TX_EN */
	/* PB28 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_ER */
	/* PB27 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII COL */
	/* PB26 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII CRS */
	/* PB25 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[3] */
	/* PB24 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[2] */
	/* PB23 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[1] */
	/* PB22 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[0] */
	/* PB21 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[0] */
	/* PB20 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[1] */
	/* PB19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[2] */
	/* PB18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[3] */
	/* PB17 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RX_DIV */
	/* PB16 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RX_ERR */
	/* PB15 */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TX_ERR */
	/* PB14 */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TX_EN */
	/* PB13 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:COL */
	/* PB12 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:CRS */
	/* PB11 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB10 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB9  */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB8  */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB7  */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB6  */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB5  */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB4  */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    },

    /* Port C */
    {   /*            conf ppar psor pdir podr pdat */
	/* PC31 */ {   0,   0,   0,   1,   0,   0   }, /* PC31 */
	/* PC30 */ {   0,   0,   0,   1,   0,   0   }, /* PC30 */
	/* PC29 */ {   0,   1,   1,   0,   0,   0   }, /* SCC1 EN *CLSN */
	/* PC28 */ {   0,   0,   0,   1,   0,   0   }, /* PC28 */
	/* PC27 */ {   0,   0,   0,   1,   0,   0   }, /* UART Clock in */
	/* PC26 */ {   0,   0,   0,   1,   0,   0   }, /* PC26 */
	/* PC25 */ {   0,   0,   0,   1,   0,   0   }, /* PC25 */
	/* PC24 */ {   0,   0,   0,   1,   0,   0   }, /* PC24 */
	/* PC23 */ {   0,   1,   0,   1,   0,   0   }, /* ATMTFCLK */
	/* PC22 */ {   0,   1,   0,   0,   0,   0   }, /* ATMRFCLK */
	/* PC21 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 EN RXCLK */
	/* PC20 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 EN TXCLK */
	/* PC19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_CLK CLK13 */
	/* PC18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC Tx Clock (CLK14) */
	/* PC17 */ {   0,   0,   0,   1,   0,   0   }, /* PC17 */
	/* PC16 */ {   0,   1,   0,   0,   0,   0   }, /* FCC Tx Clock (CLK16) */
	/* PC15 */ {   1,   1,   0,   0,   0,   0   }, /* PC15 */
	/* PC14 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 EN *CD */
	/* PC13 */ {   0,   0,   0,   1,   0,   0   }, /* PC13 */
	/* PC12 */ {   0,   1,   0,   1,   0,   0   }, /* PC12 */
	/* PC11 */ {   0,   0,   0,   1,   0,   0   }, /* LXT971 transmit control */
	/* PC10 */ {   1,   0,   0,   1,   0,   0   }, /* FETHMDC */
	/* PC9  */ {   1,   0,   0,   0,   0,   0   }, /* FETHMDIO */
	/* PC8  */ {   0,   0,   0,   1,   0,   0   }, /* PC8 */
	/* PC7  */ {   0,   0,   0,   1,   0,   0   }, /* PC7 */
	/* PC6  */ {   0,   0,   0,   1,   0,   0   }, /* PC6 */
	/* PC5  */ {   0,   0,   0,   1,   0,   0   }, /* PC5 */
	/* PC4  */ {   0,   0,   0,   1,   0,   0   }, /* PC4 */
	/* PC3  */ {   0,   0,   0,   1,   0,   0   }, /* PC3 */
	/* PC2  */ {   0,   0,   0,   1,   0,   1   }, /* ENET FDE */
	/* PC1  */ {   0,   0,   0,   1,   0,   0   }, /* ENET DSQE */
	/* PC0  */ {   0,   0,   0,   1,   0,   0   }, /* ENET LBK */
    },

    /* Port D */
    {   /*            conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN RxD */
	/* PD30 */ {   1,   1,   1,   1,   0,   0   }, /* SCC1 EN TxD */
	/* PD29 */ {   1,   1,   0,   1,   0,   0   }, /* SCC1 RTS */
	/* PD28 */ {   1,   1,   0,   0,   0,   0   }, /* SCC2 RxD */
	/* PD27 */ {   1,   1,   1,   1,   0,   0   }, /* SCC2 TxD */
	/* PD26 */ {   1,   1,   0,   1,   0,   0   }, /* SCC2 RTS */
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
	/* PD15 */ {   0,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   0,   0,   0,   1,   0,   0   }, /* LED */
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

int board_early_init_f (void)
{
#if defined(CONFIG_PCI)
    volatile immap_t *immr = (immap_t *)CFG_IMMR;
    volatile ccsr_pcix_t *pci = &immr->im_pcix;

    pci->peer &= 0xfffffffdf; /* disable master abort */
#endif
	return 0;
}

void reset_phy (void)
{
#if defined(CONFIG_ETHER_ON_FCC) /* avoid compile warnings for now */
	volatile unsigned char *bcsr = (unsigned char *) CFG_BCSR;
#endif
	/* reset Giga bit Ethernet port if needed here */

	/* reset the CPM FEC port */
#if (CONFIG_ETHER_INDEX == 2)
	bcsr[0] &= ~0x20;
	udelay(2);
	bcsr[0] |= 0x20;
	udelay(1000);
#elif (CONFIG_ETHER_INDEX == 3)
	bcsr[0] &= ~0x10;
	udelay(2);
	bcsr[0] |= 0x10;
	udelay(1000);
#endif
#if defined(CONFIG_MII) && defined(CONFIG_ETHER_ON_FCC)
	/* reset PHY */
	miiphy_reset("FCC1 ETHERNET", 0x0);

	/* change PHY address to 0x02 */
	bb_miiphy_write(NULL, 0, PHY_MIPSCR, 0xf028);

	bb_miiphy_write(NULL, 0x02, PHY_BMCR,
			PHY_BMCR_AUTON | PHY_BMCR_RST_NEG);
#endif /* CONFIG_MII */
}

int checkboard (void)
{
	sys_info_t sysinfo;

	get_sys_info (&sysinfo);

#ifdef CONFIG_SBC8560
	printf ("Board: Wind River SBC8560 Board\n");
#else
	printf ("Board: Wind River SBC8540 Board\n");
#endif
	printf ("\tCPU: %lu MHz\n", sysinfo.freqProcessor / 1000000);
	printf ("\tCCB: %lu MHz\n", sysinfo.freqSystemBus / 1000000);
	printf ("\tDDR: %lu MHz\n", sysinfo.freqSystemBus / 2000000);
	if((CFG_LBC_LCRR & 0x0f) == 2 || (CFG_LBC_LCRR & 0x0f) == 4 \
		|| (CFG_LBC_LCRR & 0x0f) == 8) {
		printf ("\tLBC: %lu MHz\n", sysinfo.freqSystemBus / 1000000 /(CFG_LBC_LCRR & 0x0f));
	} else {
		printf("\tLBC: unknown\n");
	}
	printf("\tCPM: %lu Mhz\n", sysinfo.freqSystemBus / 1000000);
	printf("L1 D-cache 32KB, L1 I-cache 32KB enabled.\n");
	return (0);
}


long int initdram (int board_type)
{
	long dram_size = 0;
	extern long spd_sdram (void);
#if 0
#if !defined(CONFIG_RAM_AS_FLASH)
	volatile ccsr_lbc_t *lbc= &immap->im_lbc;
	sys_info_t sysinfo;
	uint temp_lbcdll = 0;
#endif
#endif /* 0 */
#if !defined(CONFIG_RAM_AS_FLASH) || defined(CONFIG_DDR_DLL)
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
#endif
#if defined(CONFIG_DDR_DLL)
	uint temp_ddrdll = 0;

	/* Work around to stabilize DDR DLL */
	temp_ddrdll = gur->ddrdllcr;
	gur->ddrdllcr = ((temp_ddrdll & 0xff) << 16) | 0x80000000;
	asm("sync;isync;msync");
#endif

#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram ();
#else
	dram_size = fixed_sdram ();
#endif

#if 0
#if !defined(CONFIG_RAM_AS_FLASH) /* LocalBus SDRAM is not emulating flash */
	get_sys_info(&sysinfo);
	/* if localbus freq is less than 66Mhz,we use bypass mode,otherwise use DLL */
	if(sysinfo.freqSystemBus/(CFG_LBC_LCRR & 0x0f) < 66000000) {
		lbc->lcrr = (CFG_LBC_LCRR & 0x0fffffff)| 0x80000000;
	} else {
#if defined(CONFIG_MPC85xx_REV1) /* need change CLKDIV before enable DLL */
		lbc->lcrr = 0x10000004; /* default CLKDIV is 8, change it to 4 temporarily */
#endif
		lbc->lcrr = CFG_LBC_LCRR & 0x7fffffff;
		udelay(200);
		temp_lbcdll = gur->lbcdllcr;
		gur->lbcdllcr = ((temp_lbcdll & 0xff) << 16 ) | 0x80000000;
		asm("sync;isync;msync");
	}
	lbc->or2 = CFG_OR2_PRELIM; /* 64MB SDRAM */
	lbc->br2 = CFG_BR2_PRELIM;
	lbc->lbcr = CFG_LBC_LBCR;
	lbc->lsdmr = CFG_LBC_LSDMR_1;
	asm("sync");
	(unsigned int) * (ulong *)0 = 0x000000ff;
	lbc->lsdmr = CFG_LBC_LSDMR_2;
	asm("sync");
	(unsigned int) * (ulong *)0 = 0x000000ff;
	lbc->lsdmr = CFG_LBC_LSDMR_3;
	asm("sync");
	(unsigned int) * (ulong *)0 = 0x000000ff;
	lbc->lsdmr = CFG_LBC_LSDMR_4;
	asm("sync");
	(unsigned int) * (ulong *)0 = 0x000000ff;
	lbc->lsdmr = CFG_LBC_LSDMR_5;
	asm("sync");
	lbc->lsrt = CFG_LBC_LSRT;
	asm("sync");
	lbc->mrtpr = CFG_LBC_MRTPR;
	asm("sync");
#endif
#endif

#if defined(CONFIG_DDR_ECC)
	{
		/* Initialize all of memory for ECC, then
		 * enable errors */
		uint *p = 0;
		uint i = 0;
		volatile immap_t *immap = (immap_t *)CFG_IMMR;
		volatile ccsr_ddr_t *ddr= &immap->im_ddr;
		dma_init();
		for (*p = 0; p < (uint *)(8 * 1024); p++) {
			if (((unsigned int)p & 0x1f) == 0) { dcbz(p); }
			*p = (unsigned int)0xdeadbeef;
			if (((unsigned int)p & 0x1c) == 0x1c) { dcbf(p); }
		}

		/* 8K */
		dma_xfer((uint *)0x2000,0x2000,(uint *)0);
		/* 16K */
		dma_xfer((uint *)0x4000,0x4000,(uint *)0);
		/* 32K */
		dma_xfer((uint *)0x8000,0x8000,(uint *)0);
		/* 64K */
		dma_xfer((uint *)0x10000,0x10000,(uint *)0);
		/* 128k */
		dma_xfer((uint *)0x20000,0x20000,(uint *)0);
		/* 256k */
		dma_xfer((uint *)0x40000,0x40000,(uint *)0);
		/* 512k */
		dma_xfer((uint *)0x80000,0x80000,(uint *)0);
		/* 1M */
		dma_xfer((uint *)0x100000,0x100000,(uint *)0);
		/* 2M */
		dma_xfer((uint *)0x200000,0x200000,(uint *)0);
		/* 4M */
		dma_xfer((uint *)0x400000,0x400000,(uint *)0);

		for (i = 1; i < dram_size / 0x800000; i++) {
			dma_xfer((uint *)(0x800000*i),0x800000,(uint *)0);
		}

		/* Enable errors for ECC */
		ddr->err_disable = 0x00000000;
		asm("sync;isync;msync");
	}
#endif

	return dram_size;
}


#if defined(CFG_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CFG_MEMTEST_START;
	uint *pend = (uint *) CFG_MEMTEST_END;
	uint *p;

	printf("SDRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("SDRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("SDRAM test passed.\n");
	return 0;
}
#endif

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
long int fixed_sdram (void)
{

#define CFG_DDR_CONTROL 0xc2000000

  #ifndef CFG_RAMBOOT
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_ddr_t *ddr= &immap->im_ddr;

	ddr->cs0_bnds		= 0x00000007;
	ddr->cs1_bnds		= 0x0010001f;
	ddr->cs2_bnds		= 0x00000000;
	ddr->cs3_bnds		= 0x00000000;
	ddr->cs0_config		= 0x80000102;
	ddr->cs1_config		= 0x80000102;
	ddr->cs2_config		= 0x00000000;
	ddr->cs3_config		= 0x00000000;
	ddr->timing_cfg_1	= 0x37334321;
	ddr->timing_cfg_2	= 0x00000800;
	ddr->sdram_cfg		= 0x42000000;
	ddr->sdram_mode		= 0x00000022;
	ddr->sdram_interval	= 0x05200100;
	ddr->err_sbe		= 0x00ff0000;
    #if defined (CONFIG_DDR_ECC)
	ddr->err_disable = 0x0000000D;
    #endif
	asm("sync;isync;msync");
	udelay(500);
    #if defined (CONFIG_DDR_ECC)
	/* Enable ECC checking */
	ddr->sdram_cfg = (CFG_DDR_CONTROL | 0x20000000);
    #else
	ddr->sdram_cfg = CFG_DDR_CONTROL;
    #endif
	asm("sync; isync; msync");
	udelay(500);
  #endif
	return CFG_SDRAM_SIZE * 1024 * 1024;
}
#endif	/* !defined(CONFIG_SPD_EEPROM) */
