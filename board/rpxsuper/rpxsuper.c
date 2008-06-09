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
#include "rpxsuper.h"

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PA31 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 *ATMTXEN */
	/* PA30 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMTCA   */
	/* PA29 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMTSOC  */
	/* PA28 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 *ATMRXEN */
	/* PA27 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMRSOC */
	/* PA26 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMRCA */
	/* PA25 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[0] */
	/* PA24 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[1] */
	/* PA23 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[2] */
	/* PA22 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[3] */
	/* PA21 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[4] */
	/* PA20 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[5] */
	/* PA19 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[6] */
	/* PA18 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXD[7] */
	/* PA17 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[7] */
	/* PA16 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[6] */
	/* PA15 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[5] */
	/* PA14 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[4] */
	/* PA13 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[3] */
	/* PA12 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[2] */
	/* PA11 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[1] */
	/* PA10 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXD[0] */
	/* PA9  */ {   1,   1,   0,   1,   0,   0   }, /* SMC2 TXD */
	/* PA8  */ {   1,   1,   0,   0,   0,   0   }, /* SMC2 RXD */
	/* PA7  */ {   1,   0,   0,   0,   0,   0   }, /* PA7 */
	/* PA6  */ {   1,   0,   0,   0,   0,   0   }, /* PA6 */
	/* PA5  */ {   1,   0,   0,   0,   0,   0   }, /* PA5 */
	/* PA4  */ {   1,   0,   0,   0,   0,   0   }, /* PA4 */
	/* PA3  */ {   1,   0,   0,   0,   0,   0   }, /* PA3 */
	/* PA2  */ {   1,   0,   0,   0,   0,   0   }, /* PA2 */
	/* PA1  */ {   1,   0,   0,   0,   0,   0   }, /* PA1 */
	/* PA0  */ {   1,   0,   0,   0,   0,   0   }  /* PA0 */
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
	/* PC31 */ {   1,   0,   0,   1,   0,   0   }, /* PC31 */
	/* PC30 */ {   1,   0,   0,   1,   0,   0   }, /* PC30 */
	/* PC29 */ {   1,   1,   1,   0,   0,   0   }, /* SCC1 EN *CLSN */
	/* PC28 */ {   1,   0,   0,   1,   0,   0   }, /* PC28 */
	/* PC27 */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[0] */
	/* PC26 */ {   1,   0,   0,   1,   0,   0   }, /* PC26 */
	/* PC25 */ {   1,   0,   0,   1,   0,   0   }, /* PC25 */
	/* PC24 */ {   1,   0,   0,   1,   0,   0   }, /* PC24 */
	/* PC23 */ {   1,   1,   0,   1,   0,   0   }, /* ATMTFCLK */
	/* PC22 */ {   1,   1,   0,   0,   0,   0   }, /* ATMRFCLK */
	/* PC21 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN RXCLK */
	/* PC20 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN TXCLK */
	/* PC19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_CLK */
	/* PC18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII TX_CLK */
	/* PC17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RX_CLK */
	/* PC16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII TX_CLK */
	/* PC15 */ {   1,   0,   0,   0,   0,   0   }, /* PC15 */
	/* PC14 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN *CD */
	/* PC13 */ {   1,   0,   0,   1,   0,   0   }, /* PC13 */
	/* PC12 */ {   1,   0,   0,   1,   0,   0   }, /* PC12 */
	/* PC11 */ {   1,   0,   0,   1,   0,   0   }, /* PC11 */
	/* PC10 */ {   1,   0,   0,   1,   0,   0   }, /* FCC2 MDC */
	/* PC9  */ {   1,   0,   0,   1,   0,   0   }, /* FCC2 MDIO */
	/* PC8  */ {   1,   0,   0,   1,   0,   0   }, /* PC8 */
	/* PC7  */ {   1,   0,   0,   1,   0,   0   }, /* PC7 */
	/* PC6  */ {   1,   0,   0,   1,   0,   0   }, /* PC6 */
	/* PC5  */ {   1,   0,   0,   1,   0,   0   }, /* PC5 */
	/* PC4  */ {   1,   0,   0,   1,   0,   0   }, /* PC4 */
	/* PC3  */ {   1,   0,   0,   1,   0,   0   }, /* PC3 */
	/* PC2  */ {   1,   0,   0,   1,   0,   1   }, /* ENET FDE */
	/* PC1  */ {   1,   0,   0,   1,   0,   0   }, /* ENET DSQE */
	/* PC0  */ {   1,   0,   0,   1,   0,   0   }, /* ENET LBK */
    },

    /* Port D */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   0,   0,   0,   0,   0   }, /* SCC1 EN RxD */
	/* PD30 */ {   1,   0,   0,   0,   0,   0   }, /* SCC1 EN TxD */
	/* PD29 */ {   1,   0,   0,   0,   0,   0   }, /* SCC1 EN TENA */
	/* PD28 */ {   1,   0,   0,   0,   0,   0   }, /* PD28 */
	/* PD27 */ {   1,   0,   0,   0,   0,   0   }, /* PD27 */
	/* PD26 */ {   1,   0,   0,   0,   0,   0   }, /* PD26 */
	/* PD25 */ {   1,   0,   0,   0,   0,   0   }, /* PD25 */
	/* PD24 */ {   1,   0,   0,   0,   0,   0   }, /* PD24 */
	/* PD23 */ {   1,   0,   0,   0,   0,   0   }, /* PD23 */
	/* PD22 */ {   1,   0,   0,   0,   0,   0   }, /* PD22 */
	/* PD21 */ {   1,   0,   0,   0,   0,   0   }, /* PD21 */
	/* PD20 */ {   1,   0,   0,   0,   0,   0   }, /* PD20 */
	/* PD19 */ {   1,   0,   0,   0,   0,   0   }, /* PD19 */
	/* PD18 */ {   1,   0,   0,   0,   0,   0   }, /* PD19 */
	/* PD17 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMRXPRTY */
	/* PD16 */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 ATMTXPRTY */
	/* PD15 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SCL */
	/* PD13 */ {   1,   0,   0,   0,   0,   0   }, /* PD13 */
	/* PD12 */ {   1,   0,   0,   0,   0,   0   }, /* PD12 */
	/* PD11 */ {   1,   0,   0,   0,   0,   0   }, /* PD11 */
	/* PD10 */ {   1,   0,   0,   0,   0,   0   }, /* PD10 */
	/* PD9  */ {   1,   1,   0,   1,   0,   0   }, /* SMC1 TXD */
	/* PD8  */ {   1,   1,   0,   0,   0,   0   }, /* SMC1 RXD */
	/* PD7  */ {   1,   0,   0,   0,   0,   0   }, /* PD7 */
	/* PD6  */ {   1,   0,   0,   0,   0,   0   }, /* PD6 */
	/* PD5  */ {   1,   0,   0,   0,   0,   0   }, /* PD5 */
	/* PD4  */ {   1,   0,   0,   0,   0,   0   }, /* PD4 */
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
    volatile t_rpx_regs *regs = (t_rpx_regs*)CFG_REGS_BASE;
    volatile immap_t *immap  = (immap_t *)CFG_IMMR;
    volatile memctl8260_t *memctl = &immap->im_memctl;
    memctl->memc_br4 = CFG_BR4_PRELIM;
    memctl->memc_or4 = CFG_OR4_PRELIM;
    regs->bcsr1 = 0x70; /* to enable terminal no SMC1 */
    regs->bcsr2 = 0x20;	/* mut be written to enable writing FLASH */
    return 0;
}

void
reset_phy(void)
{
    volatile t_rpx_regs *regs = (t_rpx_regs*)CFG_REGS_BASE;
    regs->bcsr4 = 0xC3;
}

/*
 * Check Board Identity:
 */

int checkboard(void)
{
    volatile t_rpx_regs *regs = (t_rpx_regs*)CFG_REGS_BASE;
    printf ("Board: Embedded Planet RPX Super, Revision %d\n",
	regs->bcsr0 >> 4);

    return 0;
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram(int board_type)
{
    volatile immap_t *immap  = (immap_t *)CFG_IMMR;
    volatile memctl8260_t *memctl = &immap->im_memctl;
    volatile uchar c = 0, *ramaddr;
    ulong psdmr, lsdmr, bcr;
    long size = 0;
    int i;

    psdmr = CFG_PSDMR;
    lsdmr = CFG_LSDMR;

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

    size = CFG_SDRAM0_SIZE;
    bcr = immap->im_siu_conf.sc_bcr;
    immap->im_siu_conf.sc_bcr = (bcr & ~BCR_EBM);

    memctl->memc_mptpr = CFG_MPTPR;

    ramaddr = (uchar *)(CFG_SDRAM0_BASE);
    memctl->memc_psrt = CFG_PSRT;

    memctl->memc_psdmr = psdmr | PSDMR_OP_PREA;
    *ramaddr = c;

    memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR;
    for (i = 0; i < 8; i++)
	*ramaddr = c;

    memctl->memc_psdmr = psdmr | PSDMR_OP_MRW;
    *ramaddr = c;

    memctl->memc_psdmr = psdmr | PSDMR_OP_NORM | PSDMR_RFEN;
    *ramaddr = c;

    immap->im_siu_conf.sc_bcr = bcr;

#ifndef CFG_RAMBOOT
/*    size += CFG_SDRAM1_SIZE; */
    ramaddr = (uchar *)(CFG_SDRAM1_BASE);
    memctl->memc_lsrt = CFG_LSRT;

    memctl->memc_lsdmr = lsdmr | PSDMR_OP_PREA;
    *ramaddr = c;

    memctl->memc_lsdmr = lsdmr | PSDMR_OP_CBRR;
    for (i = 0; i < 8; i++)
	*ramaddr = c;

    memctl->memc_lsdmr = lsdmr | PSDMR_OP_MRW;
    *ramaddr = c;

    memctl->memc_lsdmr = lsdmr | PSDMR_OP_NORM | PSDMR_RFEN;
    *ramaddr = c;
#endif

    /* return total ram size */
    return (size * 1024 * 1024);
}
