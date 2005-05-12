/*
 * (C) Copyright 2005
 * Heiko Schocher, DENX Software Engineering, <hs@denx.de>
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
	/* PA31 */ {   0,   1,	 1,   0,   0,	0   }, /* FCC1 COL */
	/* PA30 */ {   0,   1,	 1,   0,   0,	0   }, /* FCC1 CRS */
	/* PA29 */ {   0,   1,	 1,   1,   0,	0   }, /* FCC1 TXER */
	/* PA28 */ {   0,   1,	 1,   1,   0,	0   }, /* FCC1 TXEN */
	/* PA27 */ {   0,   1,	 1,   0,   0,	0   }, /* FCC1 RXDV */
	/* PA26 */ {   0,   1,	 1,   0,   0,	0   }, /* FCC1 RXER */
	/* PA25 */ {   0,   0,	 0,   0,   1,	0   }, /* 8247_P0 */
#if defined(CONFIG_SOFT_I2C)
	/* PA24 */ {   1,   0,	 0,   0,   1,	1   }, /* I2C_SDA2 */
	/* PA23 */ {   1,   0,	 0,   1,   1,	1   }, /* I2C_SCL2 */
#else /* normal I/O port pins */
	/* PA24 */ {   0,   0,	 0,   1,   0,	0   }, /* PA24 */
	/* PA23 */ {   0,   0,	 0,   1,   0,	0   }, /* PA23 */
#endif
	/* PA22 */ {   0,   0,	 0,   0,   1,	0   }, /* SMC2_DCD */
	/* PA21 */ {   0,   1,	 0,   1,   0,	0   }, /* FCC1 TXD3 */
	/* PA20 */ {   0,   1,	 0,   1,   0,	0   }, /* FCC1 TXD2 */
	/* PA19 */ {   0,   1,	 0,   1,   0,	0   }, /* FCC1 TXD1 */
	/* PA18 */ {   0,   1,	 0,   1,   0,	0   }, /* FCC1 TXD0 */
	/* PA17 */ {   0,   1,	 0,   0,   0,	0   }, /* FCC1 RXD0 */
	/* PA16 */ {   0,   1,	 0,   0,   0,	0   }, /* FCC1 RXD1 */
	/* PA15 */ {   0,   1,	 0,   0,   0,	0   }, /* FCC1 RXD2 */
	/* PA14 */ {   0,   1,	 0,   0,   0,	0   }, /* FCC1 RXD3 */
	/* PA13 */ {   0,   0,	 0,   1,   1,	0   }, /* SMC2_RTS */
	/* PA12 */ {   0,   0,	 0,   0,   1,	0   }, /* SMC2_CTS */
	/* PA11 */ {   0,   0,	 0,   1,   1,	0   }, /* SMC2_DTR */
	/* PA10 */ {   0,   0,	 0,   0,   1,	0   }, /* SMC2_DSR */
	/* PA9	*/ {   0,   1,	 0,   1,   0,	0   }, /* SMC2 TXD */
	/* PA8	*/ {   0,   1,	 0,   0,   0,	0   }, /* SMC2 RXD */
	/* PA7	*/ {   0,   0,	 0,   1,   0,	0   }, /* PA7 */
	/* PA6	*/ {   0,   0,	 0,   1,   0,	0   }, /* PA6 */
	/* PA5	*/ {   0,   0,	 0,   1,   0,	0   }, /* PA5 */
	/* PA4	*/ {   0,   0,	 0,   1,   0,	0   }, /* PA4 */
	/* PA3	*/ {   0,   0,	 0,   1,   0,	0   }, /* PA3 */
	/* PA2	*/ {   0,   0,	 0,   1,   0,	0   }, /* PA2 */
	/* PA1	*/ {   0,   0,	 0,   1,   0,	0   }, /* PA1 */
	/* PA0	*/ {   0,   0,	 0,   1,   0,	0   }  /* PA0 */
    },

    /* Port B configuration */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PB31 */ {   1,   1,	 0,   1,   0,	0   }, /* FCC2 MII TX_ER */
	/* PB30 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII RX_DV */
	/* PB29 */ {   1,   1,	 1,   1,   0,	0   }, /* FCC2 MII TX_EN */
	/* PB28 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII RX_ER */
	/* PB27 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII COL */
	/* PB26 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII CRS */
	/* PB25 */ {   1,   1,	 0,   1,   0,	0   }, /* FCC2 MII TxD[3] */
	/* PB24 */ {   1,   1,	 0,   1,   0,	0   }, /* FCC2 MII TxD[2] */
	/* PB23 */ {   1,   1,	 0,   1,   0,	0   }, /* FCC2 MII TxD[1] */
	/* PB22 */ {   1,   1,	 0,   1,   0,	0   }, /* FCC2 MII TxD[0] */
	/* PB21 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII RxD[0] */
	/* PB20 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII RxD[1] */
	/* PB19 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII RxD[2] */
	/* PB18 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII RxD[3] */
	/* PB17 */ {   0,   0,	 0,   0,   0,	0   }, /* PB17 */
	/* PB16 */ {   0,   0,	 0,   0,   0,	0   }, /* PB16 */
	/* PB15 */ {   0,   0,	 0,   0,   0,	0   }, /* PB15 */
	/* PB14 */ {   0,   0,	 0,   0,   0,	0   }, /* PB14 */
	/* PB13 */ {   0,   0,	 0,   0,   0,	0   }, /* PB13 */
	/* PB12 */ {   0,   0,	 0,   0,   0,	0   }, /* PB12 */
	/* PB11 */ {   0,   0,	 0,   0,   0,	0   }, /* PB11 */
	/* PB10 */ {   0,   0,	 0,   0,   0,	0   }, /* PB10 */
	/* PB9	*/ {   0,   0,	 0,   0,   0,	0   }, /* PB9 */
	/* PB8	*/ {   0,   0,	 0,   0,   0,	0   }, /* PB8 */
	/* PB7	*/ {   0,   0,	 0,   0,   0,	0   }, /* PB7 */
	/* PB6	*/ {   0,   0,	 0,   0,   0,	0   }, /* PB6 */
	/* PB5	*/ {   0,   0,	 0,   0,   0,	0   }, /* PB5 */
	/* PB4	*/ {   0,   0,	 0,   0,   0,	0   }, /* PB4 */
	/* PB3	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PB2	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PB1	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PB0	*/ {   0,   0,	 0,   0,   0,	0   }  /* pin doesn't exist */
    },

    /* Port C */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PC31 */ {   0,   0,	 0,   1,   0,	0   }, /* PC31 */
	/* PC30 */ {   0,   0,	 0,   1,   0,	0   }, /* PC30 */
	/* PC29 */ {   0,   1,	 0,   0,   0,	0   }, /* SCC1 EN *CLSN */
	/* PC28 */ {   0,   1,	 1,   0,   0,	0   }, /* SYNC_OUT */
	/* PC27 */ {   0,   0,	 0,   1,   0,	0   }, /* PC27 */
	/* PC26 */ {   0,   0,	 0,   1,   0,	0   }, /* PC26 */
	/* PC25 */ {   0,   1,	 1,   0,   0,	0   }, /* SYNC_IN */
	/* PC24 */ {   0,   0,	 0,   1,   0,	0   }, /* PC24 */
	/* PC23 */ {   0,   1,	 0,   1,   0,	0   }, /* ATMTFCLK */
	/* PC22 */ {   0,   1,	 0,   0,   0,	0   }, /* ATMRFCLK */
	/* PC21 */ {   0,   1,	 0,   0,   0,	0   }, /* SCC1 EN RXCLK */
	/* PC20 */ {   0,   1,	 0,   0,   0,	0   }, /* SCC1 EN TXCLK */
	/* PC19 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII RX_CLK */
	/* PC18 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII TX_CLK */
	/* PC17 */ {   0,   0,	 0,   1,   0,	0   }, /* PC17 */
	/* PC16 */ {   0,   0,	 0,   1,   0,	0   }, /* PC16 */
	/* PC15 */ {   0,   0,	 0,   1,   0,	0   }, /* PC15 */
	/* PC14 */ {   0,   1,	 0,   0,   0,	0   }, /* SCC1 EN *CD */
	/* PC13 */ {   0,   0,	 0,   1,   0,	0   }, /* PC13 */
	/* PC12 */ {   0,   0,	 0,   1,   0,	0   }, /* PC12 */
	/* PC11 */ {   0,   0,	 0,   1,   0,	0   }, /* PC11 */
	/* PC10 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC2 MDC */
	/* PC9	*/ {   0,   0,	 0,   1,   0,	0   }, /* FCC2 MDIO */
	/* PC8	*/ {   0,   0,	 0,   1,   0,	0   }, /* PC8 */
	/* PC7	*/ {   0,   0,	 0,   1,   0,	0   }, /* PC7 */
	/* PC6	*/ {   0,   0,	 0,   1,   0,	0   }, /* PC6 */
	/* PC5	*/ {   0,   0,	 0,   1,   0,	0   }, /* PC5 */
	/* PC4	*/ {   0,   0,	 0,   1,   0,	0   }, /* PC4 */
	/* PC3	*/ {   0,   0,	 0,   1,   0,	0   }, /* PC3 */
	/* PC2	*/ {   0,   0,	 0,   1,   0,	1   }, /* ENET FDE */
	/* PC1	*/ {   0,   0,	 0,   1,   0,	0   }, /* ENET DSQE */
	/* PC0	*/ {   0,   0,	 0,   1,   0,	0   }, /* ENET LBK */
    },

    /* Port D */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   0,   1,	 0,   0,   0,	0   }, /* SCC1 EN RxD */
	/* PD30 */ {   0,   1,	 1,   1,   0,	0   }, /* SCC1 EN TxD */
	/* PD29 */ {   0,   1,	 0,   1,   0,	0   }, /* SCC1 EN TENA */
	/* PD28 */ {   0,   0,	 0,   1,   0,	0   }, /* PD28 */
	/* PD27 */ {   0,   0,	 0,   1,   0,	0   }, /* PD27 */
	/* PD26 */ {   0,   0,	 0,   1,   0,	0   }, /* PD26 */
	/* PD25 */ {   0,   1,	 0,   0,   0,	0   }, /* SCC3_RX */
	/* PD24 */ {   0,   1,	 0,   1,   0,	0   }, /* SCC3_TX */
	/* PD23 */ {   0,   1,	 0,   1,   0,	0   }, /* SCC3_RTS */
	/* PD22 */ {   0,   1,	 0,   0,   0,	0   }, /* SCC4_RXD */
	/* PD21 */ {   0,   1,	 0,   1,   0,	0   }, /* SCC4_TXD */
	/* PD20 */ {   0,   1,	 0,   1,   0,	0   }, /* SCC4_RTS */
	/* PD19 */ {   0,   1,	 1,   0,   0,	0   }, /* SPI_SEL */
	/* PD18 */ {   0,   1,	 1,   0,   0,	0   }, /* SPI_CLK */
	/* PD17 */ {   0,   1,	 1,   0,   0,	0   }, /* SPI_MOSI */
	/* PD16 */ {   0,   1,	 1,   0,   0,	0   }, /* SPI_MISO */
#if defined(CONFIG_HARD_I2C)
	/* PD15 */ {   1,   1,	 1,   0,   1,	0   }, /* I2C SDA1 */
	/* PD14 */ {   1,   1,	 1,   0,   1,	0   }, /* I2C SCL1 */
#else /* normal I/O port pins */
	/* PD15 */ {   0,   1,	 1,   0,   1,	0   }, /* PD15 */
	/* PD14 */ {   0,   1,	 1,   0,   1,	0   }, /* PD14 */
#endif
	/* PD13 */ {   0,   0,	 0,   0,   0,	0   }, /* PD13 */
	/* PD12 */ {   0,   0,	 0,   0,   0,	0   }, /* PD12 */
	/* PD11 */ {   0,   0,	 0,   0,   0,	0   }, /* PD11 */
	/* PD10 */ {   0,   0,	 0,   0,   0,	0   }, /* PD10 */
	/* PD9	*/ {   0,   0,	 0,   0,   0,	0   }, /* PD9 */
	/* PD8	*/ {   0,   0,	 0,   0,   0,	0   }, /* PD8 */
	/* PD7	*/ {   0,   0,	 0,   1,   0,	1   }, /* MII_MDIO */
	/* PD6	*/ {   0,   0,	 0,   1,   0,	1   }, /* PD6 */
	/* PD5	*/ {   0,   0,	 0,   1,   0,	1   }, /* PD5 */
	/* PD4	*/ {   0,   0,	 0,   1,   0,	1   }, /* PD4 */
	/* PD3	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PD2	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PD1	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PD0	*/ {   0,   0,	 0,   0,   0,	0   }  /* pin doesn't exist */
    }
};

/* ------------------------------------------------------------------------- */

/* Check Board Identity:
 */
int checkboard (void)
{
	puts ("Board: IDS 8247\n");
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

	long psize, lsize;

	psize = 16 * 1024 * 1024;
	lsize = 0;

	memctl->memc_psrt = CFG_PSRT;
	memctl->memc_mptpr = CFG_MPTPR;

#ifndef CFG_RAMBOOT
	/* 60x SDRAM setup:
	 */
	psize = try_init (memctl, CFG_PSDMR, CFG_OR2,
						  (uchar *) CFG_SDRAM_BASE);
#endif /* CFG_RAMBOOT */

	icache_enable ();

	return (psize);
}

int misc_init_r (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_flashstart = 0xff800000;
}

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
extern ulong
nand_probe (ulong physadr);

void
nand_init (void)
{
	ulong totlen = 0;

	debug ("Probing at 0x%.8x\n", CFG_NAND0_BASE);
	totlen += nand_probe (CFG_NAND0_BASE);

	printf ("%4lu MB\n", totlen >>20);
}

#endif	/* CFG_CMD_NAND */
