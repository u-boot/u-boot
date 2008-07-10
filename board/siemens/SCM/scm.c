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

#include "scm.h"

DECLARE_GLOBAL_DATA_PTR;

static void config_scoh_cs(void);
extern int  fpga_init(void);

#if 0
#define DEBUGF(fmt,args...)   printf (fmt ,##args)
#else
#define DEBUGF(fmt,args...)
#endif

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
	/* PA25 */ {   0,   0,   0,   1,   0,   0   },
	/* PA24 */ {   0,   0,   0,   1,   0,   0   },
	/* PA23 */ {   0,   0,   0,   1,   0,   0   },
	/* PA22 */ {   0,   0,   0,   1,   0,   0   },
	/* PA21 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[3] */
	/* PA20 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[2] */
	/* PA19 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[1] */
	/* PA18 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[0] */
	/* PA17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[0] */
	/* PA16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[1]*/
	/* PA15 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[2] */
	/* PA14 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[3] */
	/* PA13 */ {   0,   0,   0,   1,   0,   0   },
	/* PA12 */ {   0,   0,   0,   1,   0,   0   },
	/* PA11 */ {   0,   0,   0,   1,   0,   0   },
	/* PA10 */ {   0,   0,   0,   1,   0,   0   },
	/* PA9  */ {   1,   1,   1,   1,   0,   0   }, /* TDM_A1 L1TXD0 */
	/* PA8  */ {   1,   1,   1,   0,   0,   0   }, /* TDM_A1 L1RXD0 */
	/* PA7  */ {   1,   1,   1,   0,   0,   0   }, /* TDM_A1 L1TSYNC */
	/* PA6  */ {   1,   1,   1,   0,   0,   0   }, /* TDM_A1 L1RSYNC */
	/* PA5  */ {   1,   0,   0,   0,   0,   0   }, /* FIOX_FPGA_PR */
	/* PA4  */ {   1,   0,   0,   0,   0,   0   }, /* DOHM_FPGA_PR */
	/* PA3  */ {   1,   1,   0,   0,   0,   0   }, /* TDM RXCLK4 */
	/* PA2  */ {   1,   1,   0,   0,   0,   0   }, /* TDM TXCLK4 */
	/* PA1  */ {   0,   0,   0,   1,   0,   0   },
	/* PA0  */ {   1,   0,   0,   0,   0,   0   }  /* BUSY */
    },

    /* Port B configuration */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PB31 */ {   1,   0,   0,   1,   0,   0   }, /* EQ_ALARM_MIN */
	/* PB30 */ {   1,   0,   0,   1,   0,   0   }, /* EQ_ALARM_MAJ */
	/* PB29 */ {   1,   0,   0,   1,   0,   0   }, /* COM_ALARM_MIN */
	/* PB28 */ {   1,   0,   0,   1,   0,   0   }, /* COM_ALARM_MAJ */
	/* PB27 */ {   0,   1,   0,   0,   0,   0   },
	/* PB26 */ {   0,   1,   0,   0,   0,   0   },
	/* PB25 */ {   1,   0,   0,   1,   0,   0   }, /* LED_GREEN_L */
	/* PB24 */ {   1,   0,   0,   1,   0,   0   }, /* LED_RED_L */
	/* PB23 */ {   1,   1,   1,   0,   0,   0   }, /* TDM_D2 L1TXD */
	/* PB22 */ {   1,   1,   1,   0,   0,   0   }, /* TDM_D2 L1RXD */
	/* PB21 */ {   1,   1,   1,   0,   0,   0   }, /* TDM_D2 L1TSYNC */
	/* PB20 */ {   1,   1,   1,   0,   0,   0   }, /* TDM_D2 L1RSYNC */
	/* PB19 */ {   1,   0,   0,   0,   0,   0   }, /* UID */
	/* PB18 */ {   0,   1,   0,   0,   0,   0   },
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
	/* PB7  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[3] */
	/* PB6  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[2] */
	/* PB5  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[1] */
	/* PB4  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[0] */
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    },

    /* Port C configuration */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PC31 */ {   1,   1,   0,   0,   0,   0   }, /* TDM RXCLK1 */
	/* PC30 */ {   1,   1,   0,   0,   0,   0   }, /* TDM TXCLK1 */
	/* PC29 */ {   1,   1,   0,   0,   0,   0   }, /* TDM RXCLK3 */
	/* PC28 */ {   1,   1,   0,   0,   0,   0   }, /* TDM TXCLK3 */
	/* PC27 */ {   1,   1,   0,   0,   0,   0   }, /* TDM RXCLK2 */
	/* PC26 */ {   1,   1,   0,   0,   0,   0   }, /* TDM TXCLK2 */
	/* PC25 */ {   0,   0,   0,   1,   0,   0   },
	/* PC24 */ {   0,   0,   0,   1,   0,   0   },
	/* PC23 */ {   0,   1,   0,   1,   0,   0   },
	/* PC22 */ {   0,   1,   0,   0,   0,   0   },
	/* PC21 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII TX_CLK */
	/* PC20 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RX_CLK */
	/* PC19 */ {   0,   1,   0,   0,   0,   0   },
	/* PC18 */ {   0,   1,   0,   0,   0,   0   },
	/* PC17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RX_CLK */
	/* PC16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII TX_CLK */
	/* PC15 */ {   0,   0,   0,   1,   0,   0   },
	/* PC14 */ {   0,   1,   0,   0,   0,   0   },
	/* PC13 */ {   0,   0,   0,   1,   0,   0   }, /* RES_PHY_L */
	/* PC12 */ {   0,   0,   0,   1,   0,   0   },
	/* PC11 */ {   0,   0,   0,   1,   0,   0   },
	/* PC10 */ {   0,   0,   0,   1,   0,   0   },
	/* PC9  */ {   0,   1,   1,   0,   0,   0   }, /* TDM_A2 L1TSYNC */
	/* PC8  */ {   0,   0,   0,   0,   0,   0   }, /* FEP_RDY */
	/* PC7  */ {   0,   0,   0,   0,   0,   0   },
	/* PC6  */ {   0,   0,   0,   0,   0,   0   }, /* UC4_ALARM_L */
	/* PC5  */ {   0,   0,   0,   0,   0,   0   }, /* UC3_ALARM_L */
	/* PC4  */ {   0,   0,   0,   0,   0,   0   }, /* UC2_ALARM_L */
	/* PC3  */ {   0,   0,   0,   1,   0,   0   }, /* RES_MISC_L */
	/* PC2  */ {   0,   0,   0,   1,   0,   0   }, /* RES_OH_L */
	/* PC1  */ {   0,   0,   0,   1,   0,   0   }, /* RES_DOHM_L */
	/* PC0  */ {   0,   0,   0,   1,   0,   0   }, /* RES_FIOX_L */
    },

    /* Port D configuration */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN RxD */
	/* PD30 */ {   1,   1,   1,   1,   0,   0   }, /* SCC1 EN TxD */
	/* PD29 */ {   0,   0,   0,   0,   0,   0   }, /* INIT_F */
	/* PD28 */ {   0,   0,   0,   1,   0,   0   }, /* DONE_F */
	/* PD27 */ {   0,   0,   0,   0,   0,   0   }, /* INIT_D */
	/* PD26 */ {   0,   0,   0,   1,   0,   0   }, /* DONE_D */
	/* PD25 */ {   0,   0,   0,   1,   0,   0   },
	/* PD24 */ {   0,   0,   0,   1,   0,   0   },
	/* PD23 */ {   0,   0,   0,   1,   0,   0   },
	/* PD22 */ {   1,   1,   1,   0,   0,   0   }, /* TDM_A2 L1TXD */
	/* PD21 */ {   1,   1,   1,   0,   0,   0   }, /* TDM_A2 L1RXD */
	/* PD20 */ {   1,   1,   1,   0,   0,   0   }, /* TDM_A2 L1RSYNC */
	/* PD19 */ {   1,   1,   1,   0,   0,   0   }, /* SPI SPISEL */
	/* PD18 */ {   1,   1,   1,   0,   0,   0   }, /* SPI SPICLK */
	/* PD17 */ {   1,   1,   1,   0,   0,   0   }, /* SPI SPIMOSI */
	/* PD16 */ {   1,   1,   1,   0,   0,   0   }, /* SPI SPIMOSO */
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
	/* PD13 */ {   1,   1,   1,   0,   0,   0   }, /* TDM_B1 L1TXD */
	/* PD12 */ {   1,   1,   1,   0,   0,   0   }, /* TDM_B1 L1RXD */
	/* PD11 */ {   1,   1,   1,   0,   0,   0   }, /* TDM_B1 L1TSYNC */
	/* PD10 */ {   1,   1,   1,   0,   0,   0   }, /* TDM_B1 L1RSYNC */
	/* PD9  */ {   1,   1,   0,   1,   0,   0   }, /* SMC1 TXD */
	/* PD8  */ {   1,   1,   0,   0,   0,   0   }, /* SMC1 RXD */
	/* PD7  */ {   0,   0,   0,   1,   0,   1   },
	/* PD6  */ {   0,   0,   0,   1,   0,   1   },
	/* PD5  */ {   0,   0,   0,   1,   0,   0   }, /* PROG_F */
	/* PD4  */ {   0,   0,   0,   1,   0,   0   }, /* PROG_D */
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
	int i = getenv_r ("serial#", str, sizeof (str));

	puts ("Board: ");

	if (!i || strncmp (str, "TQM8260", 7)) {
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

	/* Since CFG_SDRAM_BASE is always 0 (??), we assume that
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

/*
 * Test Power-On-Reset.
 */
int power_on_reset (void)
{
	/* Test Reset Status Register */
	return gd->reset_status & RSR_CSRS ? 0 : 1;
}

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;

#ifndef CFG_RAMBOOT
	long size8, size9;
#endif
	long psize, lsize;

	psize = 16 * 1024 * 1024;
	lsize = 0;

	memctl->memc_psrt = CFG_PSRT;
	memctl->memc_mptpr = CFG_MPTPR;

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

#ifndef CFG_RAMBOOT
	/* 60x SDRAM setup:
	 */
	size8 = try_init (memctl, CFG_PSDMR_8COL, CFG_OR1_8COL,
					  (uchar *) CFG_SDRAM_BASE);
	size9 = try_init (memctl, CFG_PSDMR_9COL, CFG_OR1_9COL,
					  (uchar *) CFG_SDRAM_BASE);

	if (size8 < size9) {
		psize = size9;
		printf ("(60x:9COL - %ld MB, ", psize >> 20);
	} else {
		psize = try_init (memctl, CFG_PSDMR_8COL, CFG_OR1_8COL,
						  (uchar *) CFG_SDRAM_BASE);
		printf ("(60x:8COL - %ld MB, ", psize >> 20);
	}

	/* Local SDRAM setup:
	 */
#ifdef CFG_INIT_LOCAL_SDRAM
	memctl->memc_lsrt = CFG_LSRT;
	size8 = try_init (memctl, CFG_LSDMR_8COL, CFG_OR2_8COL,
					  (uchar *) SDRAM_BASE2_PRELIM);
	size9 = try_init (memctl, CFG_LSDMR_9COL, CFG_OR2_9COL,
					  (uchar *) SDRAM_BASE2_PRELIM);

	if (size8 < size9) {
		lsize = size9;
		printf ("Local:9COL - %ld MB) using ", lsize >> 20);
	} else {
		lsize = try_init (memctl, CFG_LSDMR_8COL, CFG_OR2_8COL,
						  (uchar *) SDRAM_BASE2_PRELIM);
		printf ("Local:8COL - %ld MB) using ", lsize >> 20);
	}

#if 0
	/* Set up BR2 so that the local SDRAM goes
	 * right after the 60x SDRAM
	 */
	memctl->memc_br2 = (CFG_BR2_PRELIM & ~BRx_BA_MSK) |
			(CFG_SDRAM_BASE + psize);
#endif
#endif /* CFG_INIT_LOCAL_SDRAM */
#endif /* CFG_RAMBOOT */

	icache_enable ();

	config_scoh_cs ();

	return (psize);
}

/* ------------------------------------------------------------------------- */

static void config_scoh_cs (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile memctl8260_t *memctl = &immr->im_memctl;
	volatile can_reg_t *can = (volatile can_reg_t *) CFG_CAN0_BASE;
	volatile uint tmp, i;

	/* Initialize OR3 / BR3 for CAN Bus Controller 0 */
	memctl->memc_or3 = CFG_CAN0_OR3;
	memctl->memc_br3 = CFG_CAN0_BR3;
	/* Initialize OR4 / BR4 for CAN Bus Controller 1 */
	memctl->memc_or4 = CFG_CAN1_OR4;
	memctl->memc_br4 = CFG_CAN1_BR4;

	/* Initialize MAMR to write in the array at address 0x0 */
	memctl->memc_mamr = 0x00 | MxMR_OP_WARR | MxMR_GPL_x4DIS;

	/* Initialize UPMA for CAN: single read */
	memctl->memc_mdr = 0xcffeec00;
	udelay (1);					/* Necessary to have the data correct in the UPM array!!!! */
	/* The read on the CAN controller write the data of mdr in UPMA array. */
	/* The index to the array will be incremented automatically
	   through this read */
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x0ffcec00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x0ffcec00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x0ffcec00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x0ffcec00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x0ffcfc00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x0ffcfc00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0xfffdec07;
	udelay (1);
	tmp = can->cpu_interface;


	/* Initialize MAMR to write in the array at address 0x18 */
	memctl->memc_mamr = 0x18 | MxMR_OP_WARR | MxMR_GPL_x4DIS;

	/* Initialize UPMA for CAN: single write */
	memctl->memc_mdr = 0xfcffec00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x00ffec00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x00ffec00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x00ffec00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x00ffec00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x00fffc00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x00fffc00;
	udelay (1);
	tmp = can->cpu_interface;

	memctl->memc_mdr = 0x30ffec07;
	udelay (1);
	tmp = can->cpu_interface;

	/* Initialize MAMR */
	memctl->memc_mamr = MxMR_GPL_x4DIS;	/* GPL_B4 ouput line Disable */


	/* Initialize OR5 / BR5 for the extended EEPROM Bank0 */
	memctl->memc_or5 = CFG_EXTPROM_OR5;
	memctl->memc_br5 = CFG_EXTPROM_BR5;
	/* Initialize OR6 / BR6 for the extended EEPROM Bank1 */
	memctl->memc_or6 = CFG_EXTPROM_OR6;
	memctl->memc_br6 = CFG_EXTPROM_BR6;

	/* Initialize OR7 / BR7 for the Glue Logic */
	memctl->memc_or7 = CFG_FIOX_OR7;
	memctl->memc_br7 = CFG_FIOX_BR7;

	/* Initialize OR8 / BR8 for the DOH Logic */
	memctl->memc_or8 = CFG_FDOHM_OR8;
	memctl->memc_br8 = CFG_FDOHM_BR8;

	DEBUGF ("OR0 %08x   BR0 %08x\n", memctl->memc_or0, memctl->memc_br0);
	DEBUGF ("OR1 %08x   BR1 %08x\n", memctl->memc_or1, memctl->memc_br1);
	DEBUGF ("OR2 %08x   BR2 %08x\n", memctl->memc_or2, memctl->memc_br2);
	DEBUGF ("OR3 %08x   BR3 %08x\n", memctl->memc_or3, memctl->memc_br3);
	DEBUGF ("OR4 %08x   BR4 %08x\n", memctl->memc_or4, memctl->memc_br4);
	DEBUGF ("OR5 %08x   BR5 %08x\n", memctl->memc_or5, memctl->memc_br5);
	DEBUGF ("OR6 %08x   BR6 %08x\n", memctl->memc_or6, memctl->memc_br6);
	DEBUGF ("OR7 %08x   BR7 %08x\n", memctl->memc_or7, memctl->memc_br7);
	DEBUGF ("OR8 %08x   BR8 %08x\n", memctl->memc_or8, memctl->memc_br8);

	DEBUGF ("UPMA  addr 0x0\n");
	memctl->memc_mamr = 0x00 | MxMR_OP_RARR | MxMR_GPL_x4DIS;
	for (i = 0; i < 0x8; i++) {
		tmp = can->cpu_interface;
		udelay (1);
		DEBUGF (" %08x ", memctl->memc_mdr);
	}
	DEBUGF ("\nUPMA  addr 0x18\n");
	memctl->memc_mamr = 0x18 | MxMR_OP_RARR | MxMR_GPL_x4DIS;
	for (i = 0; i < 0x8; i++) {
		tmp = can->cpu_interface;
		udelay (1);
		DEBUGF (" %08x ", memctl->memc_mdr);
	}
	DEBUGF ("\n");
	memctl->memc_mamr = MxMR_GPL_x4DIS;
}

/* ------------------------------------------------------------------------- */

int misc_init_r (void)
{
	fpga_init ();
	return (0);
}

/* ------------------------------------------------------------------------- */
