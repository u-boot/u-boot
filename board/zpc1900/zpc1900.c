/*
 * (C) Copyright 2001-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2003-2005 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <ioports.h>
#include <mpc8260.h>
#include <miiphy.h>

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PA31 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 TxENB  */
	/* PA30 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 TxClav */
	/* PA29 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 TxSOC  */
	/* PA28 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 RxENB  */
	/* PA27 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 RxSOC  */
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
	/* PA9  */ {   0,   1,   1,   1,   0,   0   }, /* SMC2 TXD */
	/* PA8  */ {   0,   1,   1,   0,   0,   0   }, /* SMC2 RXD */
	/* PA7  */ {   0,   0,   0,   0,   0,   0   }, /* PA7 */
	/* PA6  */ {   0,   0,   0,   0,   0,   0   }, /* PA6 */
	/* PA5  */ {   0,   0,   0,   0,   0,   0   }, /* PA5 */
	/* PA4  */ {   0,   0,   0,   0,   0,   0   }, /* PA4 */
	/* PA3  */ {   0,   0,   0,   0,   0,   0   }, /* PA3 */
	/* PA2  */ {   0,   0,   0,   0,   0,   0   }, /* PA2 */
	/* PA1  */ {   0,   0,   0,   0,   0,   0   }, /* PA1 */
	/* PA0  */ {   0,   0,   0,   0,   0,   0   }  /* PA0 */
    },

    /* Port B */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PB31 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TX_ER  */
	/* PB30 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_DV  */
	/* PB29 */ {   1,   1,   1,   1,   0,   0   }, /* FCC2 MII TX_EN  */
	/* PB28 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_ER  */
	/* PB27 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII COL    */
	/* PB26 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII CRS    */
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
	/* PB14 */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TX_EN  */
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
    {   /*	      conf ppar psor pdir podr pdat */
	/* PC31 */ {   0,   0,   0,   0,   0,   0   }, /* PC31 */
	/* PC30 */ {   0,   0,   0,   0,   0,   0   }, /* PC30 */
	/* PC29 */ {   0,   1,   1,   0,   0,   0   }, /* SCC1 EN CLSN */
	/* PC28 */ {   0,   0,   0,   0,   0,   0   }, /* PC28 */
	/* PC27 */ {   0,   0,   0,   0,   0,   0   }, /* PC27 */
	/* PC26 */ {   0,   0,   0,   0,   0,   0   }, /* PC26 */
	/* PC25 */ {   0,   0,   0,   0,   0,   0   }, /* PC25 */
	/* PC24 */ {   0,   0,   0,   0,   0,   0   }, /* PC24 */
	/* PC23 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 EN RXCLK */
	/* PC22 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 EN TXCLK */
	/* PC21 */ {   0,   0,   0,   0,   0,   0   }, /* PC21 */
	/* PC20 */ {   0,   0,   0,   0,   0,   0   }, /* PC20 */
	/* PC19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII Rx Clock (CLK13) */
	/* PC18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII Tx Clock (CLK14) */
	/* PC17 */ {   0,   0,   0,   0,   0,   0   }, /* PC17 */
	/* PC16 */ {   0,   0,   0,   0,   0,   0   }, /* PC16 */
	/* PC15 */ {   0,   0,   0,   0,   0,   0   }, /* PC15 */
	/* PC14 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 EN RENA */
	/* PC13 */ {   0,   0,   0,   0,   0,   0   }, /* PC13 */
	/* PC12 */ {   0,   0,   0,   0,   0,   0   }, /* PC12 */
	/* PC11 */ {   0,   0,   0,   0,   0,   0   }, /* PC11 */
	/* PC10 */ {   1,   0,   0,   1,   0,   0   }, /* LXT972 MDC */
	/* PC9  */ {   1,   0,   0,   0,   0,   0   }, /* LXT972 MDIO */
	/* PC8  */ {   0,   0,   0,   0,   0,   0   }, /* PC8 */
	/* PC7  */ {   0,   0,   0,   0,   0,   0   }, /* PC7 */
	/* PC6  */ {   0,   0,   0,   0,   0,   0   }, /* PC6 */
	/* PC5  */ {   0,   0,   0,   0,   0,   0   }, /* PC5 */
	/* PC4  */ {   0,   0,   0,   0,   0,   0   }, /* PC4 */
	/* PC3  */ {   0,   0,   0,   0,   0,   0   }, /* PC3 */
	/* PC2  */ {   0,   0,   0,   0,   0,   0   }, /* PC2 */
	/* PC1  */ {   0,   0,   0,   0,   0,   0   }, /* PC1 */
	/* PC0  */ {   0,   0,   0,   0,   0,   0   }, /* PC0 */
    },

    /* Port D */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 EN RxD  */
	/* PD30 */ {   0,   1,   1,   1,   0,   0   }, /* SCC1 EN TxD  */
	/* PD29 */ {   0,   1,   0,   1,   0,   0   }, /* SCC1 EN TENA */
	/* PD28 */ {   0,   0,   0,   0,   0,   0   }, /* PD28 */
	/* PD27 */ {   0,   0,   0,   0,   0,   0   }, /* PD27 */
	/* PD26 */ {   0,   0,   0,   0,   0,   0   }, /* PD26 */
	/* PD25 */ {   0,   0,   0,   0,   0,   0   }, /* PD25 */
	/* PD24 */ {   0,   0,   0,   0,   0,   0   }, /* PD24 */
	/* PD23 */ {   0,   0,   0,   0,   0,   0   }, /* PD23 */
	/* PD22 */ {   0,   0,   0,   0,   0,   0   }, /* PD22 */
	/* PD21 */ {   0,   0,   0,   0,   0,   0   }, /* PD21 */
	/* PD20 */ {   0,   0,   0,   0,   0,   0   }, /* PD20 */
	/* PD19 */ {   0,   0,   0,   0,   0,   0   }, /* PD19 */
	/* PD18 */ {   0,   0,   0,   0,   0,   0   }, /* PD18 */
	/* PD17 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXPRTY */
	/* PD16 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXPRTY */
	/* PD15 */ {   0,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   0,   1,   1,   0,   1,   0   }, /* I2C SCL */
	/* PD13 */ {   0,   0,   0,   0,   0,   0   }, /* PD13 */
	/* PD12 */ {   0,   0,   0,   0,   0,   0   }, /* PD12 */
	/* PD11 */ {   0,   0,   0,   0,   0,   0   }, /* PD11 */
	/* PD10 */ {   0,   0,   0,   0,   0,   0   }, /* PD10 */
	/* PD9  */ {   1,   1,   0,   1,   0,   0   }, /* SMC1 TXD */
	/* PD8  */ {   1,   1,   0,   0,   0,   0   }, /* SMC1 RXD */
	/* PD7  */ {   0,   0,   0,   0,   0,   0   }, /* PD7 */
	/* PD6  */ {   0,   0,   0,   0,   0,   0   }, /* PD6 */
	/* PD5  */ {   0,   0,   0,   0,   0,   0   }, /* PD5 */
	/* PD4  */ {   0,   0,   0,   0,   0,   0   }, /* PD4 */
	/* PD3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    }
};

#ifdef CONFIG_SYS_NVRAM_ACCESS_ROUTINE
void *nvram_read(void *dest, long src, size_t count)
{
	return memcpy(dest, (const void *)src, count);
}

void nvram_write(long dest, const void *src, size_t count)
{
	vu_char     *p1 = (vu_char *)(CONFIG_SYS_EEPROM + 0x1555);
	vu_char     *p2 = (vu_char *)(CONFIG_SYS_EEPROM + 0x0AAA);
	vu_char     *d = (vu_char *)dest;
	const uchar *s = (const uchar *)src;

	/* Unprotect the EEPROM */
	*p1 = 0xAA;
	*p2 = 0x55;
	*p1 = 0x80;
	*p1 = 0xAA;
	*p2 = 0x55;
	*p1 = 0x20;
	udelay(10000);

	/* Write the data to the EEPROM */
	while (count--) {
		*d++ = *s++;
		while (*(d - 1) != *(s - 1))
			/* wait */;
	}

	/* Protect the EEPROM */
	*p1 = 0xAA;
	*p2 = 0x55;
	*p1 = 0xA0;
	udelay(10000);
}
#endif /* CONFIG_SYS_NVRAM_ACCESS_ROUTINE */

phys_size_t initdram(int board_type)
{
	vu_char *bcsr = (vu_char *)CONFIG_SYS_BCSR;
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	vu_char *ramaddr;
	uchar c = 0xFF;
	long int msize = CONFIG_SYS_SDRAM_SIZE;
	int i;

	if (bcsr[4] & BCSR_PCI_MODE) { /* PCI mode selected by JP9 */
		immap->im_clkrst.car_sccr |= SCCR_PCI_MODE;
		immap->im_siu_conf.sc_siumcr =
			(immap->im_siu_conf.sc_siumcr & ~SIUMCR_LBPC11)
			| SIUMCR_LBPC01;
	}

#ifndef CONFIG_SYS_RAMBOOT
	immap->im_siu_conf.sc_ppc_acr  = 0x03;
	immap->im_siu_conf.sc_ppc_alrh = 0x30126745;
	immap->im_siu_conf.sc_tescr1   = 0x00004000;

	memctl->memc_mptpr = CONFIG_SYS_MPTPR;

#ifdef CONFIG_SYS_LSDRAM_BASE
	/*
	  Initialise local bus SDRAM only if the pins
	  are configured as local bus pins and not as PCI.
	*/
	if ((immap->im_siu_conf.sc_siumcr & SIUMCR_LBPC11) == SIUMCR_LBPC00) {
		memctl->memc_lsrt  = CONFIG_SYS_LSRT;
		memctl->memc_or4   = CONFIG_SYS_LSDRAM_OR;
		memctl->memc_br4   = CONFIG_SYS_LSDRAM_BR;
		ramaddr = (vu_char *)CONFIG_SYS_LSDRAM_BASE;
		memctl->memc_lsdmr = CONFIG_SYS_LSDMR | PSDMR_OP_PREA;
		*ramaddr = c;
		memctl->memc_lsdmr = CONFIG_SYS_LSDMR | PSDMR_OP_CBRR;
		for (i = 0; i < 8; i++)
			*ramaddr = c;
		memctl->memc_lsdmr = CONFIG_SYS_LSDMR | PSDMR_OP_MRW;
		*ramaddr = c;
		memctl->memc_lsdmr = CONFIG_SYS_LSDMR | PSDMR_RFEN;
	}
#endif /* CONFIG_SYS_LSDRAM_BASE */

	/* Initialise 60x bus SDRAM */
	memctl->memc_psrt = CONFIG_SYS_PSRT;
	memctl->memc_or2  = CONFIG_SYS_PSDRAM_OR;
	memctl->memc_br2  = CONFIG_SYS_PSDRAM_BR;
	/*
	 * The mode data for Mode Register Write command must appear on
	 * the address lines during a mode-set cycle. It is driven by
	 * the memory controller, in single PowerQUICC II mode,
	 * according to PSDMR[CL] and PSDMR[BL] fields. In
	 * 60x-compatible mode, software must drive the correct value on
	 * the address lines. BL=0 because for 64-bit port size burst
	 * length must be 4.
	 */
	ramaddr = (vu_char *)(CONFIG_SYS_SDRAM_BASE |
			      ((CONFIG_SYS_PSDMR & PSDMR_CL_MSK) << 7) | 0x10);
	memctl->memc_psdmr = CONFIG_SYS_PSDMR | PSDMR_OP_PREA; /* Precharge all banks */
	*ramaddr = c;
	memctl->memc_psdmr = CONFIG_SYS_PSDMR | PSDMR_OP_CBRR; /* CBR refresh */
	for (i = 0; i < 8; i++)
		*ramaddr = c;
	memctl->memc_psdmr = CONFIG_SYS_PSDMR | PSDMR_OP_MRW;  /* Mode Register write */
	*ramaddr = c;
	memctl->memc_psdmr = CONFIG_SYS_PSDMR | PSDMR_RFEN;    /* Refresh enable */
	*ramaddr = c;
#endif /* CONFIG_SYS_RAMBOOT */

	/* Return total 60x bus SDRAM size */
	return msize * 1024 * 1024;
}

int checkboard(void)
{
	vu_char *bcsr = (vu_char *)CONFIG_SYS_BCSR;

	printf("Board: Zephyr ZPC.1900 Rev. %c\n", bcsr[2] + 0x40);
	return 0;
}
