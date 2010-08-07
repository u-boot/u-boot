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

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PA31 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 *ATMTXEN */
	/* PA30 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMTCA   */
	/* PA29 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMTSOC  */
	/* PA28 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 *ATMRXEN */
	/* PA27 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMRSOC */
	/* PA26 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMRCA */
	/* PA25 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMTXD[0] */
	/* PA24 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMTXD[1] */
	/* PA23 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMTXD[2] */
	/* PA22 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMTXD[3] */
	/* PA21 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMTXD[4] */
	/* PA20 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMTXD[5] */
	/* PA19 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMTXD[6] */
	/* PA18 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMTXD[7] */
	/* PA17 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMRXD[7] */
	/* PA16 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMRXD[6] */
	/* PA15 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMRXD[5] */
	/* PA14 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMRXD[4] */
	/* PA13 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMRXD[3] */
	/* PA12 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMRXD[2] */
	/* PA11 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMRXD[1] */
	/* PA10 */ {   0,   0,   0,   1,   0,   0   }, /* FCC1 ATMRXD[0] */
	/* PA9  */ {   1,   1,   0,   1,   0,   0   }, /* SMC2 TXD */
	/* PA8  */ {   1,   1,   0,   0,   0,   0   }, /* SMC2 RXD */
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
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    },

    /* Port C */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PC31 */ {   0,   0,   0,   1,   0,   0   }, /* PC31 */
	/* PC30 */ {   0,   0,   0,   1,   0,   0   }, /* PC30 */
	/* PC29 */ {   1,   1,   1,   0,   0,   0   }, /* SCC1 EN *CLSN */
	/* PC28 */ {   0,   0,   0,   1,   0,   0   }, /* PC28 */
	/* PC27 */ {   0,   0,   0,   1,   0,   0   }, /* PC27 */
	/* PC26 */ {   0,   0,   0,   1,   0,   0   }, /* PC26 */
	/* PC25 */ {   0,   0,   0,   1,   0,   0   }, /* PC25 */
	/* PC24 */ {   0,   0,   0,   1,   0,   0   }, /* PC24 */
	/* PC23 */ {   0,   1,   0,   1,   0,   0   }, /* ATMTFCLK */
	/* PC22 */ {   0,   1,   0,   0,   0,   0   }, /* ATMRFCLK */
	/* PC21 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN RXCLK */
	/* PC20 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN TXCLK */
	/* PC19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_CLK */
	/* PC18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII TX_CLK */
	/* PC17 */ {   0,   0,   0,   1,   0,   0   }, /* PC17 */
	/* PC16 */ {   0,   0,   0,   1,   0,   0   }, /* PC16 */
	/* PC15 */ {   0,   0,   0,   1,   0,   0   }, /* PC15 */
	/* PC14 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN *CD */
	/* PC13 */ {   0,   0,   0,   1,   0,   0   }, /* PC13 */
	/* PC12 */ {   0,   0,   0,   1,   0,   0   }, /* PC12 */
	/* PC11 */ {   0,   0,   0,   1,   0,   0   }, /* PC11 */
	/* PC10 */ {   0,   0,   0,   1,   0,   0   }, /* FCC2 MDC */
	/* PC9  */ {   0,   0,   0,   1,   0,   0   }, /* FCC2 MDIO */
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
    {   /*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN RxD */
	/* PD30 */ {   1,   1,   1,   1,   0,   0   }, /* SCC1 EN TxD */
	/* PD29 */ {   1,   1,   0,   1,   0,   0   }, /* SCC1 EN TENA */
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
	/* PD17 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXPRTY */
	/* PD16 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXPRTY */
#if defined(CONFIG_SOFT_I2C)
	/* PD15 */ {   1,   0,   0,   1,   1,   1   }, /* I2C SDA */
	/* PD14 */ {   1,   0,   0,   1,   1,   1   }, /* I2C SCL */
#else
#if defined(CONFIG_HARD_I2C)
	/* PD15 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SCL */
#else /* normal I/O port pins */
	/* PD15 */ {   0,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   0,   1,   1,   0,   1,   0   }, /* I2C SCL */
#endif
#endif
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

/* Check Board Identity:
 */
int checkboard (void)
{
	char str[64];
	int i = getenv_f("serial#", str, sizeof (str));

	puts ("Board: ");

	if (!i || strncmp (str, "TQM82", 5)) {
		puts ("### No HW ID - assuming TQM8260\n");
		return (0);
	}

	puts (str);
	putc ('\n');

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

	/* Since CONFIG_SYS_SDRAM_BASE is always 0 (??), we assume that
	 * we are configuring CS1 if base != 0
	 */
	sdmr_ptr = base ? &memctl->memc_lsdmr : &memctl->memc_psdmr;
	orx_ptr = base ? &memctl->memc_or2 : &memctl->memc_or1;

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
	 * get here. The SDRAM can be accessed at the address CONFIG_SYS_SDRAM_BASE.
	 */

	*sdmr_ptr = sdmr | PSDMR_OP_PREA;
	*base = c;

	*sdmr_ptr = sdmr | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++)
		*base = c;

	*sdmr_ptr = sdmr | PSDMR_OP_MRW;
	*(base + CONFIG_SYS_MRS_OFFS) = c;	/* setting MR on address lines */

	*sdmr_ptr = sdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	*base = c;

	size = get_ram_size((long *)base, maxsize);
	*orx_ptr = orx | ~(size - 1);

	return (size);
}

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;

#ifndef CONFIG_SYS_RAMBOOT
	long size8, size9;
#endif
	long psize, lsize;

	psize = 16 * 1024 * 1024;
	lsize = 0;

	memctl->memc_psrt = CONFIG_SYS_PSRT;
	memctl->memc_mptpr = CONFIG_SYS_MPTPR;

#if 0							/* Just for debugging */
#define	prt_br_or(brX,orX) do {				\
    ulong start =  memctl->memc_ ## brX & 0xFFFF8000;	\
    ulong sizem = ~memctl->memc_ ## orX | 0x00007FFF;	\
    printf ("\n"					\
	    #brX " 0x%08x  " #orX " 0x%08x "		\
	    "==> 0x%08lx ... 0x%08lx = %ld MB\n",	\
	memctl->memc_ ## brX, memctl->memc_ ## orX,	\
	start, start+sizem, (sizem+1)>>20);		\
    } while (0)
	prt_br_or (br0, or0);
	prt_br_or (br1, or1);
	prt_br_or (br2, or2);
	prt_br_or (br3, or3);
#endif

#ifndef CONFIG_SYS_RAMBOOT
	/* 60x SDRAM setup:
	 */
	size8 = try_init (memctl, CONFIG_SYS_PSDMR_8COL, CONFIG_SYS_OR1_8COL,
					  (uchar *) CONFIG_SYS_SDRAM_BASE);
	size9 = try_init (memctl, CONFIG_SYS_PSDMR_9COL, CONFIG_SYS_OR1_9COL,
					  (uchar *) CONFIG_SYS_SDRAM_BASE);

	if (size8 < size9) {
		psize = size9;
		printf ("(60x:9COL - %ld MB, ", psize >> 20);
	} else {
		psize = try_init (memctl, CONFIG_SYS_PSDMR_8COL, CONFIG_SYS_OR1_8COL,
						  (uchar *) CONFIG_SYS_SDRAM_BASE);
		printf ("(60x:8COL - %ld MB, ", psize >> 20);
	}

	/* Local SDRAM setup:
	 */
#ifdef CONFIG_SYS_INIT_LOCAL_SDRAM
	memctl->memc_lsrt = CONFIG_SYS_LSRT;
	size8 = try_init (memctl, CONFIG_SYS_LSDMR_8COL, CONFIG_SYS_OR2_8COL,
					  (uchar *) SDRAM_BASE2_PRELIM);
	size9 = try_init (memctl, CONFIG_SYS_LSDMR_9COL, CONFIG_SYS_OR2_9COL,
					  (uchar *) SDRAM_BASE2_PRELIM);

	if (size8 < size9) {
		lsize = size9;
		printf ("Local:9COL - %ld MB) using ", lsize >> 20);
	} else {
		lsize = try_init (memctl, CONFIG_SYS_LSDMR_8COL, CONFIG_SYS_OR2_8COL,
						  (uchar *) SDRAM_BASE2_PRELIM);
		printf ("Local:8COL - %ld MB) using ", lsize >> 20);
	}

#if 0
	/* Set up BR2 so that the local SDRAM goes
	 * right after the 60x SDRAM
	 */
	memctl->memc_br2 = (CONFIG_SYS_BR2_PRELIM & ~BRx_BA_MSK) |
			(CONFIG_SYS_SDRAM_BASE + psize);
#endif
#endif /* CONFIG_SYS_INIT_LOCAL_SDRAM */
#endif /* CONFIG_SYS_RAMBOOT */

	icache_enable ();

	return (psize);
}

/* ------------------------------------------------------------------------- */
