/*
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2001
 * Advent Networks, Inc. <http://www.adventnetworks.com>
 * Jay Monkman <jtm@smoothsmoothie.com>
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

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {


    /* Port A configuration */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PA31 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 *ATMTXEN */
	/* PA30 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMTCA   */
	/* PA29 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMTSOC  */
	/* PA28 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 *ATMRXEN */
	/* PA27 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMRSOC */
	/* PA26 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMRCA */
	/* PA25 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[0] */
	/* PA24 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[1] */
	/* PA23 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[2] */
	/* PA22 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[3] */
	/* PA21 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[4] */
	/* PA20 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[5] */
	/* PA19 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[6] */
	/* PA18 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[7] */
	/* PA17 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[7] */
	/* PA16 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[6] */
	/* PA15 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[5] */
	/* PA14 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[4] */
	/* PA13 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[3] */
	/* PA12 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[2] */
	/* PA11 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[1] */
	/* PA10 */ {   0,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[0] */
	/* PA9  */ {   1,   1,   0,   1,   0,   0   }, /* SMC2 TXD */
	/* PA8  */ {   1,   1,   0,   0,   0,   0   }, /* SMC2 RXD */
	/* PA7  */ {   1,   0,   0,   1,   0,   0   }, /* TDM_A1:L1TSYNC */
	/* PA6  */ {   1,   0,   0,   1,   0,   0   }, /* TDN_A1:L1RSYNC */
	/* PA5  */ {   0,   0,   0,   0,   0,   0   }, /* PA5 */
	/* PA4  */ {   0,   0,   0,   0,   0,   0   }, /* PA4 */
	/* PA3  */ {   0,   0,   0,   0,   0,   0   }, /* PA3 */
	/* PA2  */ {   0,   0,   0,   0,   0,   0   }, /* PA2 */
	/* PA1  */ {   0,   0,   0,   0,   0,   0   }, /* PA1 */
	/* PA0  */ {   0,   0,   0,   0,   0,   0   }  /* PA0 */
    },

    /* Port B configuration */
    {   /*	      conf ppar psor pdir podr pdat */
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
	/* PB17 */ {   0,   0,   0,   0,   0,   0   }, /* PB17 */
	/* PB16 */ {   1,   0,   0,   0,   0,   0   }, /* TDM_A1:L1CLK0 */
	/* PB15 */ {   1,   0,   0,   1,   0,   1   }, /* /FETHRST */
	/* PB14 */ {   1,   0,   0,   1,   0,   0   }, /* FETHDIS */
	/* PB13 */ {   0,   0,   0,   0,   0,   0   }, /* PB13 */
	/* PB12 */ {   1,   0,   0,   1,   0,   0   }, /* TDM_B1:L1CLK0 */
	/* PB11 */ {   1,   0,   0,   1,   0,   0   }, /* TDM_D1:L1TXD */
	/* PB10 */ {   1,   0,   0,   1,   0,   0   }, /* TDM_D1:L1RXD */
	/* PB9  */ {   1,   0,   0,   1,   0,   0   }, /* TDM_D1:L1TSYNC */
	/* PB8  */ {   1,   0,   0,   1,   0,   0   }, /* TDM_D1:L1RSYNC */
	/* PB7  */ {   0,   0,   0,   0,   0,   0   }, /* PB7 */
	/* PB6  */ {   0,   0,   0,   0,   0,   0   }, /* PB6 */
	/* PB5  */ {   0,   0,   0,   0,   0,   0   }, /* PB5 */
	/* PB4  */ {   0,   0,   0,   0,   0,   0   }, /* PB4 */
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    },

    /* Port C */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PC31 */ {   0,   0,   0,   0,   0,   0   }, /* PC31 */
	/* PC30 */ {   0,   0,   0,   0,   0,   0   }, /* PC30 */
	/* PC29 */ {   0,   0,   0,   0,   0,   0   }, /* PC28 */
	/* PC28 */ {   1,   1,   0,   0,   0,   0   }, /* CLK4 */
	/* PC27 */ {   0,   0,   0,   0,   0,   0   }, /* PC27 */
	/* PC26 */ {   0,   0,   0,   0,   0,   0   }, /* PC26 */
	/* PC25 */ {   1,   1,   0,   0,   0,   0   }, /* CLK7 */
	/* PC24 */ {   0,   0,   0,   0,   0,   0   }, /* PC24 */
	/* PC23 */ {   1,   0,   0,   1,   0,   0   }, /* ATMTFCLK */
	/* PC22 */ {   0,   0,   0,   0,   0,   0   }, /* PC22 */
	/* PC21 */ {   0,   0,   0,   0,   0,   0   }, /* PC23 */
	/* PC20 */ {   0,   0,   0,   0,   0,   0   }, /* PC24 */
	/* PC19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_CLK */
	/* PC18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII TX_CLK */
	/* PC17 */ {   0,   0,   0,   0,   0,   0   }, /* PC17 */
	/* PC16 */ {   0,   0,   0,   0,   0,   0   }, /* PC16 */
	/* PC15 */ {   1,   0,   0,   1,   0,   0   }, /* FCC1:TxAddr[0] */
	/* PC14 */ {   1,   0,   0,   1,   0,   0   }, /* FCC1:RxAddr[0] */
	/* PC13 */ {   1,   0,   0,   1,   0,   0   }, /* FCC1:TxAddr[1] */
	/* PC12 */ {   1,   0,   0,   1,   0,   0   }, /* FCC1:RxAddr[1] */
	/* PC11 */ {   1,   1,   0,   1,   0,   0   }, /* TDM_D1:L1CLK0 */
	/* PC10 */ {   1,   0,   0,   1,   0,   0   }, /* FCC2 MDC */
	/* PC9  */ {   1,   0,   0,   1,   0,   0   }, /* FCC2 MDIO */
	/* PC8  */ {   0,   0,   0,   0,   0,   0   }, /* PC8 */
	/* PC7  */ {   1,   0,   0,   1,   0,   0   }, /* FCC1:TxAddr[2]*/
	/* PC6  */ {   1,   0,   0,   1,   0,   0   }, /* FCC1:RxAddr[2] */
	/* PC5  */ {   0,   0,   0,   0,   0,   0   }, /* PC5 */
	/* PC4  */ {   0,   0,   0,   0,   0,   0   }, /* PC4 */
	/* PC3  */ {   1,   0,   0,   1,   0,   0   }, /* IDMA2:DACK */
	/* PC2  */ {   1,   0,   0,   1,   0,   0   }, /* IDMA2:DONE */
	/* PC1  */ {   1,   0,   0,   1,   0,   0   }, /* IDMA2:DREQ */
	/* PC0  */ {   1,   0,   0,   1,   0,   0   }, /* IDMA1:DREQ */
    },

    /* Port D */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   0,   0,   0,   0,   0,   0   }, /* PD31 */
	/* PD30 */ {   0,   0,   0,   0,   0,   0   }, /* PD30 */
	/* PD29 */ {   1,   0,   0,   1,   0,   0   }, /* FCC1:RxAddr[3] */
	/* PD28 */ {   0,   0,   0,   0,   0,   0   }, /* PD28 */
	/* PD27 */ {   0,   0,   0,   0,   0,   0   }, /* PD27 */
	/* PD26 */ {   1,   0,   0,   1,   0,   0   }, /* TDM_C1:L1RSYNC */
	/* PD25 */ {   0,   0,   0,   0,   0,   0   }, /* PD25 */
	/* PD24 */ {   0,   0,   0,   0,   0,   0   }, /* PD24 */
	/* PD23 */ {   0,   0,   0,   0,   0,   0   }, /* PD23 */
	/* PD22 */ {   0,   0,   0,   0,   0,   0   }, /* PD22 */
	/* PD21 */ {   0,   0,   0,   0,   0,   0   }, /* PD21 */
	/* PD20 */ {   0,   0,   0,   0,   0,   0   }, /* PD20 */
	/* PD19 */ {   0,   0,   0,   0,   0,   0   }, /* PD19 */
	/* PD18 */ {   0,   0,   0,   0,   0,   0   }, /* PD19 */
	/* PD17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXPRTY */
	/* PD16 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXPRTY */
	/* PD15 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SCL */
	/* PD13 */ {   1,   0,   0,   0,   0,   0   }, /* TDM_B1:L1TXD */
	/* PD12 */ {   1,   0,   0,   0,   0,   0   }, /* TDM_B1:L1RXD */
	/* PD11 */ {   1,   0,   0,   0,   0,   0   }, /* TDM_B1:L1TSYNC */
	/* PD10 */ {   1,   0,   0,   0,   0,   0   }, /* TDM_B1:L1RSYNC*/
	/* PD9  */ {   1,   1,   0,   1,   0,   0   }, /* SMC1:TXD */
	/* PD8  */ {   1,   1,   0,   0,   0,   0   }, /* SMC1:RXD */
	/* PD7  */ {   1,   1,   0,   0,   0,   0   }, /* SMC1:SMSYN */
	/* PD6  */ {   1,   0,   0,   1,   0,   0   }, /* IDMA1:DACK */
	/* PD5  */ {   1,   0,   0,   1,   0,   0   }, /* IDMA1:DONE */
	/* PD4  */ {   0,   0,   0,   0,   0,   0   }, /* PD4 */
	/* PD3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    }
};

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	puts ("Board: Wind River PPMC8260\n");
	return 0;
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	volatile uchar c = 0xff;
	volatile uchar *ramaddr0 = (uchar *) (CONFIG_SYS_SDRAM0_BASE);
	volatile uchar *ramaddr1 = (uchar *) (CONFIG_SYS_SDRAM1_BASE);
	ulong psdmr = CONFIG_SYS_PSDMR;
	volatile uchar *ramaddr2 = (uchar *) (CONFIG_SYS_SDRAM2_BASE);
	ulong lsdmr = CONFIG_SYS_LSDMR;
	int i;

	/*
	 * Quote from 8260 UM (10.4.2 SDRAM Power-On Initialization, 10-35):
	 *
	 * "At system reset, initialization software must set up the
	 *  programmable parameters in the memory controller banks registers
	 *  (ORx, BRx, P/LSDMR). After all memory parameters are configured,
	 *  system software should execute the following initialization sequence
	 *  for each SDRAM device.
	 *
	 *  1. Issue a PRECHARGE-ALL-BANKS command
	 *  2. Issue eight CBR REFRESH commands
	 *  3. Issue a MODE-SET command to initialize the mode register
	 *
	 *  The initial commands are executed by setting P/LSDMR[OP] and
	 *  accessing the SDRAM with a single-byte transaction."
	 *
	 * The appropriate BRx/ORx registers have already been set when we
	 * get here. The SDRAM can be accessed at the address CONFIG_SYS_SDRAM_BASE.
	 */

	memctl->memc_psrt = CONFIG_SYS_PSRT;
	memctl->memc_mptpr = CONFIG_SYS_MPTPR;

#ifndef CONFIG_SYS_RAMBOOT
	memctl->memc_psdmr = psdmr | PSDMR_OP_PREA;
	*ramaddr0++ = c;
	*ramaddr1++ = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++) {
		*ramaddr0++ = c;
		*ramaddr1++ = c;
	}

	memctl->memc_psdmr = psdmr | PSDMR_OP_MRW;
	ramaddr0 = (uchar *) (CONFIG_SYS_SDRAM0_BASE + 0x110);
	ramaddr1 = (uchar *) (CONFIG_SYS_SDRAM1_BASE + 0x110);
	*ramaddr0 = c;
	*ramaddr1 = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	*ramaddr0 = c;
	*ramaddr1 = c;

	memctl->memc_lsdmr = lsdmr | PSDMR_OP_PREA;
	*ramaddr2++ = c;

	memctl->memc_lsdmr = lsdmr | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++) {
		*ramaddr2++ = c;
	}

	memctl->memc_lsdmr = lsdmr | PSDMR_OP_MRW;
	*ramaddr2++ = c;

	memctl->memc_lsdmr = lsdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	*ramaddr2 = c;
#endif

	/* return total ram size */
	return ((CONFIG_SYS_SDRAM0_SIZE + CONFIG_SYS_SDRAM1_SIZE) * 1024 * 1024);
}

#ifdef CONFIG_MISC_INIT_R
/* ------------------------------------------------------------------------- */
int misc_init_r (void)
{
#ifdef CONFIG_SYS_LED_BASE
	uchar ds = *(unsigned char *) (CONFIG_SYS_LED_BASE + 1);
	uchar ss;
	uchar tmp[64];
	int res;

	if ((ds != 0) && (ds != 0xff)) {
		res = getenv_f("ethaddr", (char *)tmp, sizeof (tmp));
		if (res > 0) {
			ss = ((ds >> 4) & 0x0f);
			ss += ss < 0x0a ? '0' : ('a' - 10);
			tmp[15] = ss;

			ss = (ds & 0x0f);
			ss += ss < 0x0a ? '0' : ('a' - 10);
			tmp[16] = ss;

			tmp[17] = '\0';
			setenv ("ethaddr", (char *)tmp);
			/* set the led to show the address */
			*((unsigned char *) (CONFIG_SYS_LED_BASE + 1)) = ds;
		}
	}
#endif /* CONFIG_SYS_LED_BASE */
	return (0);
}
#endif /* CONFIG_MISC_INIT_R */
