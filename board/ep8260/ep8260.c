/*
 * (C) Copyright 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Frank Panno <fpanno@delphintech.com>, Delphin Technology AG
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
#include "ep8260.h"
/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PA31 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA30 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA29 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA28 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA27 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA26 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA25 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA24 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA23 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA22 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA21 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA20 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA19 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA18 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA17 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA16 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA15 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA14 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA13 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA12 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA11 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA10 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PA9  */ {   0,   1,   0,   1,   0,   0   }, /*  */
	/* PA8  */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PA7  */ {   0,   0,   0,   1,   0,   0   }, /* PA7 */
	/* PA6  */ {   0,   0,   0,   1,   0,   0   }, /* PA6 */
	/* PA5  */ {   0,   0,   0,   1,   0,   0   }, /* PA5 */
	/* PA4  */ {   0,   0,   0,   1,   0,   0   }, /* PA4 */
	/* PA3  */ {   0,   0,   0,   1,   0,   0   }, /* PA3 */
	/* PA2  */ {   0,   0,   0,   1,   0,   0   }, /* PA2 */
	/* PA1  */ {   0,   0,   0,   1,   0,   0   }, /* PA1 */
	/* PA0  */ {   0,   0,   0,   1,   0,   0   }  /* PA0 */
    },

    /* Port B configuration */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PB31 */ {   0,   1,   0,   1,   0,   0   }, /*  */
	/* PB30 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PB29 */ {   0,   1,   1,   1,   0,   0   }, /*  */
	/* PB28 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PB27 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PB26 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PB25 */ {   0,   1,   0,   1,   0,   0   }, /*  */
	/* PB24 */ {   0,   1,   0,   1,   0,   0   }, /*  */
	/* PB23 */ {   0,   1,   0,   1,   0,   0   }, /*  */
	/* PB22 */ {   0,   1,   0,   1,   0,   0   }, /*  */
	/* PB21 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PB20 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PB19 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PB18 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PB17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RX_DV */
	/* PB16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RX_ER */
	/* PB15 */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TX_ER */
	/* PB14 */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TX_EN */
	/* PB13 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII COL */
	/* PB12 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII CRS */
	/* PB11 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RxD[3] */
	/* PB10 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RxD[2] */
	/* PB9  */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RxD[1] */
	/* PB8  */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RxD[0] */
	/* PB7  */ {   0,   0,   0,   0,   0,   0   }, /* PB7 */
	/* PB6  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[1] */
	/* PB5  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[2] */
	/* PB4  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[3] */
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    },

    /* Port C */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PC31 */ {   0,   0,   0,   1,   0,   0   }, /* PC31 */
	/* PC30 */ {   0,   0,   0,   1,   0,   0   }, /* PC30 */
	/* PC29 */ {   0,   1,   1,   0,   0,   0   }, /*  */
	/* PC28 */ {   0,   0,   0,   1,   0,   0   }, /* PC28 */
	/* PC27 */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[0] */
	/* PC26 */ {   0,   0,   0,   1,   0,   0   }, /* PC26 */
	/* PC25 */ {   0,   0,   0,   1,   0,   0   }, /* PC25 */
	/* PC24 */ {   0,   0,   0,   1,   0,   0   }, /* PC24 */
	/* PC23 */ {   0,   1,   0,   1,   0,   0   }, /*  */
	/* PC22 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PC21 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PC20 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PC19 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PC18 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PC17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII CLK15 */
	/* PC16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII CLK16 */
	/* PC15 */ {   0,   0,   0,   1,   0,   0   }, /* PC15 */
	/* PC14 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PC13 */ {   0,   0,   0,   1,   0,   0   }, /* PC13 */
	/* PC12 */ {   0,   0,   0,   1,   0,   0   }, /* PC12 */
	/* PC11 */ {   0,   0,   0,   1,   0,   0   }, /* PC11 */
	/* PC10 */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PC9  */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PC8  */ {   0,   0,   0,   1,   0,   0   }, /* PC8 */
	/* PC7  */ {   0,   0,   0,   1,   0,   0   }, /* PC7 */
	/* PC6  */ {   0,   0,   0,   1,   0,   0   }, /* PC6 */
	/* PC5  */ {   0,   0,   0,   1,   0,   0   }, /* PC5 */
	/* PC4  */ {   0,   0,   0,   1,   0,   0   }, /* PC4 */
	/* PC3  */ {   0,   0,   0,   1,   0,   0   }, /* PC3 */
	/* PC2  */ {   0,   0,   0,   1,   0,   1   }, /*  */
	/* PC1  */ {   0,   0,   0,   1,   0,   0   }, /*  */
	/* PC0  */ {   0,   0,   0,   1,   0,   0   }, /*  */
    },

    /* Port D */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PD30 */ {   0,   1,   1,   1,   0,   0   }, /*  */
	/* PD29 */ {   0,   1,   0,   1,   0,   0   }, /*  */
	/* PD28 */ {   0,   0,   0,   1,   0,   0   }, /* PD28 */
	/* PD27 */ {   0,   0,   0,   1,   0,   0   }, /* PD27 */
	/* PD26 */ {   0,   0,   0,   1,   0,   0   }, /* PD26 */
	/* PD25 */ {   0,   0,   0,   1,   0,   0   }, /* PD25 */
	/* PD24 */ {   0,   0,   0,   1,   0,   0   }, /* PD24 */
	/* PD23 */ {   0,   0,   0,   1,   0,   0   }, /* PD23 */
	/* PD22 */ {   0,   0,   0,   1,   0,   0   }, /* PD22 */
	/* PD21 */ {   0,   0,   0,   1,   0,   0   }, /* PD21 */
	/* PD20 */ {   0,   0,   0,   1,   0,   0   }, /* PD20 */
	/* PD19 */ {   0,   0,   0,   1,   0,   0   }, /* PD19 */
	/* PD18 */ {   0,   0,   0,   1,   0,   0   }, /* PD19 */
	/* PD17 */ {   0,   1,   0,   0,   0,   0   }, /*  */
	/* PD16 */ {   0,   1,   0,   1,   0,   0   }, /*  */
	/* PD15 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SCL */
	/* PD13 */ {   0,   0,   0,   0,   0,   0   }, /* PD13 */
	/* PD12 */ {   0,   0,   0,   0,   0,   0   }, /* PD12 */
	/* PD11 */ {   0,   0,   0,   0,   0,   0   }, /* PD11 */
	/* PD10 */ {   0,   0,   0,   0,   0,   0   }, /* PD10 */
	/* PD9  */ {   1,   1,   0,   1,   0,   0   }, /* SMC1 TXD */
	/* PD8  */ {   1,   1,   0,   0,   0,   0   }, /* SMC1 RXD */
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

/* ------------------------------------------------------------------------- */

/*
 * Setup CS4 to enable the Board Control/Status registers.
 * Otherwise the smcs won't work.
*/
int board_early_init_f (void)
{
	volatile t_ep_regs *regs = (t_ep_regs *) CFG_REGS_BASE;
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;

	memctl->memc_br4 = CFG_BR4_PRELIM;
	memctl->memc_or4 = CFG_OR4_PRELIM;
	regs->bcsr1 = 0x62;	/* to enable terminal on SMC1 */
	regs->bcsr2 = 0x30;	/* enable NVRAM and writing FLASH */
	return 0;
}

void reset_phy (void)
{
	volatile t_ep_regs *regs = (t_ep_regs *) CFG_REGS_BASE;

	regs->bcsr4 = 0xC0;
}

/*
 * Check Board Identity:
 * I don' know, how the next board revisions will be coded.
 * Thats why its a static interpretation ...
*/

int checkboard (void)
{
	volatile t_ep_regs *regs = (t_ep_regs *) CFG_REGS_BASE;
	uint major = 0, minor = 0;

	switch (regs->bcsr0) {
	case 0x02:
		major = 1;
		break;
	case 0x03:
		major = 1;
		minor = 1;
		break;
	case 0x06:
		major = 1;
		minor = 3;
		break;
	default:
		break;
	}
	printf ("Board: Embedded Planet EP8260, Revision %d.%d\n",
		major, minor);
	return 0;
}


/* ------------------------------------------------------------------------- */


long int initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	volatile uchar c = 0;
	volatile uchar *ramaddr = (uchar *) (CFG_SDRAM_BASE) + 0x110;

/*
	ulong psdmr = CFG_PSDMR;
#ifdef CFG_LSDRAM
	ulong lsdmr = CFG_LSDMR;
#endif
*/
	long size = CFG_SDRAM0_SIZE;
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
* get here. The SDRAM can be accessed at the address CFG_SDRAM_BASE.
*/

	memctl->memc_psrt = CFG_PSRT;
	memctl->memc_mptpr = CFG_MPTPR;

	memctl->memc_psdmr = (ulong) CFG_PSDMR | PSDMR_OP_PREA;
	*ramaddr = c;

	memctl->memc_psdmr = (ulong) CFG_PSDMR | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++)
		*ramaddr = c;

	memctl->memc_psdmr = (ulong) CFG_PSDMR | PSDMR_OP_MRW;
	*ramaddr = c;

	memctl->memc_psdmr = (ulong) CFG_PSDMR | PSDMR_OP_NORM | PSDMR_RFEN;
	*ramaddr = c;

#ifndef CFG_RAMBOOT
#ifdef CFG_LSDRAM
	size += CFG_SDRAM1_SIZE;
	ramaddr = (uchar *) (CFG_SDRAM1_BASE) + 0x8c;
	memctl->memc_lsrt = CFG_LSRT;

	memctl->memc_lsdmr = (ulong) CFG_LSDMR | PSDMR_OP_PREA;
	*ramaddr = c;

	memctl->memc_lsdmr = (ulong) CFG_LSDMR | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++)
		*ramaddr = c;

	memctl->memc_lsdmr = (ulong) CFG_LSDMR | PSDMR_OP_MRW;
	*ramaddr = c;

	memctl->memc_lsdmr = (ulong) CFG_LSDMR | PSDMR_OP_NORM | PSDMR_RFEN;
	*ramaddr = c;
#endif /* CFG_LSDRAM */
#endif /* CFG_RAMBOOT */
	return (size * 1024 * 1024);
}
