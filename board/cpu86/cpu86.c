/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include "cpu86.h"

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PA31 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII COL */
	/* PA30 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII CRS */
	/* PA29 */ {   1,   1,   1,   1,   0,   0   }, /* FCC1 MII TX_ER */
	/* PA28 */ {   1,   1,   1,   1,   0,   0   }, /* FCC1 MII TX_EN */
	/* PA27 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII RX_DV */
	/* PA26 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII RX_ER */
	/* PA25 */ {   1,   0,   0,   1,   0,   0   }, /* FCC2 MII MDIO */
	/* PA24 */ {   1,   0,   0,   1,   0,   0   }, /* FCC2 MII MDC */
	/* PA23 */ {   1,   0,   0,   1,   0,   0   }, /* FCC1 MII MDIO */
	/* PA22 */ {   1,   0,   0,   1,   0,   0   }, /* FCC1 MII MDC */
	/* PA21 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[3] */
	/* PA20 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[2] */
	/* PA19 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[1] */
	/* PA18 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[0] */
	/* PA17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[0] */
	/* PA16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[1] */
	/* PA15 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[2] */
	/* PA14 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[3] */
	/* PA13 */ {   1,   0,   0,   1,   0,   0   }, /* FCC2 MII TXSL1 */
	/* PA12 */ {   1,   0,   0,   1,   0,   1   }, /* FCC2 MII TXSL0 */
	/* PA11 */ {   1,   0,   0,   1,   0,   0   }, /* FCC1 MII TXSL1 */
	/* PA10 */ {   1,   0,   0,   1,   0,   1   }, /* FCC1 MII TXSL0 */
	/* PA9  */ {   0,   1,   0,   1,   0,   0   }, /* SMC2 TXD */
	/* PA8  */ {   0,   1,   0,   0,   0,   0   }, /* SMC2 RXD */
	/* PA7  */ {   0,   0,   0,   0,   0,   0   }, /* PA7 */
	/* PA6  */ {   1,   0,   0,   1,   0,   1   }, /* FCC2 MII PAUSE */
	/* PA5  */ {   1,   0,   0,   1,   0,   1   }, /* FCC1 MII PAUSE */
	/* PA4  */ {   1,   0,   0,   1,   0,   0   }, /* FCC2 MII PWRDN */
	/* PA3  */ {   1,   0,   0,   1,   0,   0   }, /* FCC1 MII PWRDN */
	/* PA2  */ {   0,   0,   0,   0,   0,   0   }, /* PA2 */
	/* PA1  */ {   1,   0,   0,   0,   0,   0   }, /* FCC2 MII MDINT */
	/* PA0  */ {   1,   0,   0,   1,   0,   0   }  /* FCC1 MII MDINT */
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
	/* PB16 */ {   0,   0,   0,   0,   0,   0   }, /* PB16 */
	/* PB15 */ {   0,   0,   0,   0,   0,   0   }, /* PB15 */
	/* PB14 */ {   0,   0,   0,   0,   0,   0   }, /* PB14 */
	/* PB13 */ {   0,   0,   0,   0,   0,   0   }, /* PB13 */
	/* PB12 */ {   0,   0,   0,   0,   0,   0   }, /* PB12 */
	/* PB11 */ {   0,   0,   0,   0,   0,   0   }, /* PB11 */
	/* PB10 */ {   0,   0,   0,   0,   0,   0   }, /* PB10 */
	/* PB9  */ {   0,   0,   0,   0,   0,   0   }, /* PB9 */
	/* PB8  */ {   0,   0,   0,   0,   0,   0   }, /* PB8 */
	/* PB7  */ {   0,   0,   0,   0,   0,   0   }, /* PB7 */
	/* PB6  */ {   0,   0,   0,   0,   0,   0   }, /* PB6 */
	/* PB5  */ {   0,   0,   0,   0,   0,   0   }, /* PB5 */
	/* PB4  */ {   0,   0,   0,   0,   0,   0   }, /* PB4 */
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* PB3 */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* PB2 */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* PB1 */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* PB0 */
    },

    /* Port C */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PC31 */ {   0,   0,   0,   0,   0,   0   }, /* PC31 */
	/* PC30 */ {   0,   0,   0,   0,   0,   0   }, /* PC30 */
	/* PC29 */ {   1,   0,   0,   0,   0,   0   }, /* SCC1 CTS */
	/* PC28 */ {   1,   0,   0,   0,   0,   0   }, /* SCC2 CTS */
	/* PC27 */ {   0,   0,   0,   0,   0,   0   }, /* PC27 */
	/* PC26 */ {   0,   0,   0,   0,   0,   0   }, /* PC26 */
	/* PC25 */ {   0,   0,   0,   0,   0,   0   }, /* PC25 */
	/* PC24 */ {   0,   0,   0,   0,   0,   0   }, /* PC24 */
	/* PC23 */ {   0,   0,   0,   0,   0,   0   }, /* FDC37C78 DACFD */
	/* PC22 */ {   0,   0,   0,   0,   0,   0   }, /* FDC37C78 DNFD */
	/* PC21 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RX_CLK */
	/* PC20 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII TX_CLK */
	/* PC19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_CLK */
	/* PC18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII TX_CLK */
	/* PC17 */ {   0,   0,   0,   0,   0,   0   }, /* PC17 */
	/* PC16 */ {   0,   0,   0,   0,   0,   0   }, /* PC16 */
	/* PC15 */ {   0,   0,   0,   0,   0,   0   }, /* PC15 */
	/* PC14 */ {   0,   0,   0,   0,   0,   0   }, /* PC14 */
	/* PC13 */ {   0,   0,   0,   0,   0,   0   }, /* PC13 */
	/* PC12 */ {   0,   0,   0,   0,   0,   0   }, /* PC12 */
	/* PC11 */ {   0,   0,   0,   0,   0,   0   }, /* PC11 */
	/* PC10 */ {   0,   0,   0,   0,   0,   0   }, /* PC10 */
	/* PC9  */ {   0,   0,   0,   0,   0,   0   }, /* FC9 */
	/* PC8  */ {   0,   0,   0,   0,   0,   0   }, /* PC8 */
	/* PC7  */ {   0,   0,   0,   0,   0,   0   }, /* PC7 */
	/* PC6  */ {   0,   0,   0,   0,   0,   0   }, /* PC6 */
	/* PC5  */ {   0,   0,   0,   0,   0,   0   }, /* PC5 */
	/* PC4  */ {   0,   0,   0,   0,   0,   0   }, /* PC4 */
	/* PC3  */ {   0,   0,   0,   0,   0,   0   }, /* PC3 */
	/* PC2  */ {   0,   0,   0,   0,   0,   0   }, /* PC2 */
	/* PC1  */ {   0,   0,   0,   0,   0,   0   }, /* PC1 */
	/* PC0  */ {   0,   0,   0,   0,   0,   0   }, /* FDC37C78 DRQFD */
    },

    /* Port D */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 RXD */
	/* PD30 */ {   1,   1,   1,   1,   0,   0   }, /* SCC1 TXD */
	/* PD29 */ {   1,   0,   0,   1,   0,   0   }, /* SCC1 RTS */
	/* PD28 */ {   1,   1,   0,   0,   0,   0   }, /* SCC2 RXD */
	/* PD27 */ {   1,   1,   0,   1,   0,   0   }, /* SCC2 TXD */
	/* PD26 */ {   1,   0,   0,   1,   0,   0   }, /* SCC2 RTS */
	/* PD25 */ {   0,   0,   0,   0,   0,   0   }, /* PD25 */
	/* PD24 */ {   0,   0,   0,   0,   0,   0   }, /* PD24 */
	/* PD23 */ {   0,   0,   0,   0,   0,   0   }, /* PD23 */
	/* PD22 */ {   0,   0,   0,   0,   0,   0   }, /* PD22 */
	/* PD21 */ {   0,   0,   0,   0,   0,   0   }, /* PD21 */
	/* PD20 */ {   0,   0,   0,   0,   0,   0   }, /* PD20 */
	/* PD19 */ {   0,   0,   0,   0,   0,   0   }, /* PD19 */
	/* PD18 */ {   0,   0,   0,   0,   0,   0   }, /* PD18 */
	/* PD17 */ {   0,   0,   0,   0,   0,   0   }, /* PD17 */
	/* PD16 */ {   0,   0,   0,   0,   0,   0   }, /* PD16 */
#if defined(CONFIG_SOFT_I2C)
	/* PD15 */ {   1,   0,   0,   1,   1,   1   }, /* I2C SDA */
	/* PD14 */ {   1,   0,   0,   1,   1,   1   }, /* I2C SCL */
#else
#if defined(CONFIG_HARD_I2C)
	/* PD15 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SCL */
#else /* normal I/O port pins */
	/* PD15 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SCL */
#endif
#endif
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
	/* PD3  */ {   0,   0,   0,   0,   0,   0   }, /* PD3 */
	/* PD2  */ {   0,   0,   0,   0,   0,   0   }, /* PD2 */
	/* PD1  */ {   0,   0,   0,   0,   0,   0   }, /* PD1 */
	/* PD0  */ {   0,   0,   0,   0,   0,   0   }  /* PD0 */
    }
};

/* ------------------------------------------------------------------------- */

/* Check Board Identity:
 */
int checkboard (void)
{
	printf ("Board: CPU86 (Rev %02x)\n", CPU86_REV);
	return 0;
}

/* ------------------------------------------------------------------------- */

/* Try SDRAM initialization with P/LSDMR=sdmr and ORx=orx
 *
 * This routine performs standard 8260 initialization sequence
 * and calculates the available memory size. It may be called
 * several times to try different SDRAM configurations on both
 * 60x and local buses.
 */
static long int try_init (volatile memctl8260_t * memctl, ulong sdmr,
			  ulong orx, volatile uchar * base)
{
	volatile uchar c = 0xff;
	volatile uint *sdmr_ptr;
	volatile uint *orx_ptr;
	ulong maxsize, size;
	int i;

	/* We must be able to test a location outsize the maximum legal size
	 * to find out THAT we are outside; but this address still has to be
	 * mapped by the controller. That means, that the initial mapping has
	 * to be (at least) twice as large as the maximum expected size.
	 */
	maxsize = (1 + (~orx | 0x7fff)) / 2;

	/* Since CFG_SDRAM_BASE is always 0 (??), we assume that
	 * we are configuring CS1 if base != 0
	 */
	sdmr_ptr = &memctl->memc_psdmr;
	orx_ptr = &memctl->memc_or2;

	*orx_ptr = orx;

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

	*sdmr_ptr = sdmr | PSDMR_OP_PREA;
	*base = c;

	*sdmr_ptr = sdmr | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++)
		*base = c;

	*sdmr_ptr = sdmr | PSDMR_OP_MRW;
	*(base + CFG_MRS_OFFS) = c;	/* setting MR on address lines */

	*sdmr_ptr = sdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	*base = c;

	size = get_ram_size((long *)base, maxsize);

	*orx_ptr = orx | ~(size - 1);

	return (size);
}

long int initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;

#ifndef CFG_RAMBOOT
	ulong size8, size9;
#endif
	long psize;

	psize = 32 * 1024 * 1024;

	memctl->memc_mptpr = CFG_MPTPR;
	memctl->memc_psrt = CFG_PSRT;

#ifndef CFG_RAMBOOT
	/* 60x SDRAM setup:
	 */
	size8 = try_init (memctl, CFG_PSDMR_8COL, CFG_OR2_8COL,
			  (uchar *) CFG_SDRAM_BASE);
	size9 = try_init (memctl, CFG_PSDMR_9COL, CFG_OR2_9COL,
			  (uchar *) CFG_SDRAM_BASE);

	if (size8 < size9) {
		psize = size9;
		printf ("(60x:9COL) ");
	} else {
		psize = try_init (memctl, CFG_PSDMR_8COL, CFG_OR2_8COL,
				  (uchar *) CFG_SDRAM_BASE);
		printf ("(60x:8COL) ");
	}

#endif	/* CFG_RAMBOOT */

	icache_enable ();

	return (psize);
}

#if (CONFIG_COMMANDS & CFG_CMD_DOC)
extern void doc_probe (ulong physadr);
void doc_init (void)
{
	doc_probe (CFG_DOC_BASE);
}
#endif
