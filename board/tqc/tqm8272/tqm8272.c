/*
 * (C) Copyright 2006
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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

#include <command.h>
#include <netdev.h>
#ifdef CONFIG_PCI
#include <pci.h>
#include <asm/m8260_pci.h>
#endif
#include "tqm8272.h"

#if 0
#define deb_printf(fmt,arg...) \
	printf ("TQM8272 %s %s: " fmt,__FILE__, __FUNCTION__, ##arg)
#else
#define deb_printf(fmt,arg...) \
	do { } while (0)
#endif

#if defined(CONFIG_BOARD_GET_CPU_CLK_F)
unsigned long board_get_cpu_clk_f (void);
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
	/* PA31 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 *ATMTXEN */
	/* PA30 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMTCA	*/
	/* PA29 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMTSOC	*/
	/* PA28 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 *ATMRXEN */
	/* PA27 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMRSOC */
	/* PA26 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMRCA */
	/* PA25 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMTXD[0] */
	/* PA24 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMTXD[1] */
	/* PA23 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMTXD[2] */
	/* PA22 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMTXD[3] */
	/* PA21 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMTXD[4] */
	/* PA20 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMTXD[5] */
	/* PA19 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMTXD[6] */
	/* PA18 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMTXD[7] */
	/* PA17 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMRXD[7] */
	/* PA16 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMRXD[6] */
	/* PA15 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMRXD[5] */
	/* PA14 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMRXD[4] */
	/* PA13 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMRXD[3] */
	/* PA12 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMRXD[2] */
	/* PA11 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMRXD[1] */
	/* PA10 */ {   0,   0,	 0,   1,   0,	0   }, /* FCC1 ATMRXD[0] */
	/* PA9	*/ {   1,   1,	 0,   1,   0,	0   }, /* SMC2 TXD */
	/* PA8	*/ {   1,   1,	 0,   0,   0,	0   }, /* SMC2 RXD */
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
	/* PC30 */ {   0,   0,	 0,   0,   0,	0   }, /* PC30 */
	/* PC29 */ {   1,   1,	 1,   0,   0,	0   }, /* SCC1 EN *CLSN */
	/* PC28 */ {   0,   0,	 0,   1,   0,	0   }, /* PC28 */
	/* PC27 */ {   0,   0,	 0,   1,   0,	0   }, /* PC27 */
	/* PC26 */ {   0,   0,	 0,   1,   0,	0   }, /* PC26 */
	/* PC25 */ {   0,   0,	 0,   1,   0,	0   }, /* PC25 */
	/* PC24 */ {   0,   0,	 0,   1,   0,	0   }, /* PC24 */
	/* PC23 */ {   0,   1,	 0,   1,   0,	0   }, /* ATMTFCLK */
	/* PC22 */ {   0,   1,	 0,   0,   0,	0   }, /* ATMRFCLK */
	/* PC21 */ {   1,   1,	 0,   0,   0,	0   }, /* SCC1 EN RXCLK */
	/* PC20 */ {   1,   1,	 0,   0,   0,	0   }, /* SCC1 EN TXCLK */
	/* PC19 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII RX_CLK */
	/* PC18 */ {   1,   1,	 0,   0,   0,	0   }, /* FCC2 MII TX_CLK */
	/* PC17 */ {   1,   0,	 0,   1,   0,	0   }, /* PC17 MDC */
	/* PC16 */ {   1,   0,	 0,   0,   0,	0   }, /* PC16 MDIO*/
	/* PC15 */ {   0,   0,	 0,   1,   0,	0   }, /* PC15 */
	/* PC14 */ {   1,   1,	 0,   0,   0,	0   }, /* SCC1 EN *CD */
	/* PC13 */ {   0,   0,	 0,   1,   0,	0   }, /* PC13 */
	/* PC12 */ {   0,   0,	 0,   1,   0,	0   }, /* PC12 */
	/* PC11 */ {   0,   0,	 0,   1,   0,	0   }, /* PC11 */
	/* PC10 */ {   0,   0,	 0,   1,   0,	0   }, /* PC10 */
	/* PC9	*/ {   0,   0,	 0,   1,   0,	0   }, /* PC9 */
	/* PC8	*/ {   0,   0,	 0,   1,   0,	0   }, /* PC8 */
	/* PC7	*/ {   0,   0,	 0,   1,   0,	0   }, /* PC7 */
	/* PC6	*/ {   0,   0,	 0,   1,   0,	0   }, /* PC6 */
	/* PC5	*/ {   1,   1,	 0,   1,   0,	0   }, /* PC5 SMC1 TXD */
	/* PC4	*/ {   1,   1,	 0,   0,   0,	0   }, /* PC4 SMC1 RXD */
	/* PC3	*/ {   0,   0,	 0,   1,   0,	0   }, /* PC3 */
	/* PC2	*/ {   0,   0,	 0,   1,   0,	1   }, /* ENET FDE */
	/* PC1	*/ {   0,   0,	 0,   1,   0,	0   }, /* ENET DSQE */
	/* PC0	*/ {   0,   0,	 0,   1,   0,	0   }, /* ENET LBK */
    },

    /* Port D */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   1,	 0,   0,   0,	0   }, /* SCC1 EN RxD */
	/* PD30 */ {   1,   1,	 1,   1,   0,	0   }, /* SCC1 EN TxD */
	/* PD29 */ {   1,   1,	 0,   1,   0,	0   }, /* SCC1 EN TENA */
	/* PD28 */ {   0,   0,	 0,   1,   0,	0   }, /* PD28 */
	/* PD27 */ {   0,   0,	 0,   1,   0,	0   }, /* PD27 */
	/* PD26 */ {   0,   0,	 0,   1,   0,	0   }, /* PD26 */
	/* PD25 */ {   0,   0,	 0,   1,   0,	0   }, /* PD25 */
	/* PD24 */ {   0,   0,	 0,   1,   0,	0   }, /* PD24 */
	/* PD23 */ {   0,   0,	 0,   1,   0,	0   }, /* PD23 */
	/* PD22 */ {   0,   0,	 0,   1,   0,	0   }, /* PD22 */
	/* PD21 */ {   0,   0,	 0,   1,   0,	0   }, /* PD21 */
	/* PD20 */ {   0,   0,	 0,   1,   0,	0   }, /* PD20 */
	/* PD19 */ {   0,   0,	 0,   1,   0,	0   }, /* PD19 */
	/* PD18 */ {   0,   0,	 0,   1,   0,	0   }, /* PD19 */
	/* PD17 */ {   0,   1,	 0,   0,   0,	0   }, /* FCC1 ATMRXPRTY */
	/* PD16 */ {   0,   1,	 0,   1,   0,	0   }, /* FCC1 ATMTXPRTY */
#if defined(CONFIG_SOFT_I2C)
	/* PD15 */ {   1,   0,	 0,   1,   1,	1   }, /* I2C SDA */
	/* PD14 */ {   1,   0,	 0,   1,   1,	1   }, /* I2C SCL */
#else
#if defined(CONFIG_HARD_I2C)
	/* PD15 */ {   1,   1,	 1,   0,   1,	0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,	 1,   0,   1,	0   }, /* I2C SCL */
#else /* normal I/O port pins */
	/* PD15 */ {   0,   1,	 1,   0,   1,	0   }, /* I2C SDA */
	/* PD14 */ {   0,   1,	 1,   0,   1,	0   }, /* I2C SCL */
#endif
#endif
	/* PD13 */ {   0,   0,	 0,   0,   0,	0   }, /* PD13 */
	/* PD12 */ {   0,   0,	 0,   0,   0,	0   }, /* PD12 */
	/* PD11 */ {   0,   0,	 0,   0,   0,	0   }, /* PD11 */
	/* PD10 */ {   0,   0,	 0,   0,   0,	0   }, /* PD10 */
	/* PD9	*/ {   1,   1,	 0,   1,   0,	0   }, /* SMC1 TXD */
	/* PD8	*/ {   1,   1,	 0,   0,   0,	0   }, /* SMC1 RXD */
	/* PD7	*/ {   0,   0,	 0,   1,   0,	1   }, /* PD7 */
	/* PD6	*/ {   0,   0,	 0,   1,   0,	1   }, /* PD6 */
	/* PD5	*/ {   0,   0,	 0,   1,   0,	0   }, /* PD5 */
	/* PD4	*/ {   0,   0,	 0,   1,   0,	1   }, /* PD4 */
	/* PD3	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PD2	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PD1	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PD0	*/ {   0,   0,	 0,   0,   0,	0   }  /* pin doesn't exist */
    }
};

/* UPM pattern for slow init */
static const uint upmTableSlow[] =
{
    /* Offset	UPM Read Single RAM array entry */
    /* 0x00 */	0xffffee00, 0x00ffcc80, 0x00ffcf00, 0x00ffdc00,
    /* 0x04 */	0x00ffce80, 0x00ffcc00, 0x00ffee00, 0x3fffcc07,

		/* UPM Read Burst RAM array entry -> unused */
    /* 0x08 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x0C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

		/* UPM Read Burst RAM array entry -> unused */
    /* 0x10 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x14 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

		/* UPM Write Single RAM array entry */
    /* 0x18 */	0xffffee00, 0x00ffec80, 0x00ffef00, 0x00fffc80,
    /* 0x1C */	0x00fffe00, 0x00ffec00, 0x0fffef00, 0x3fffec05,

		/* UPM Write Burst RAM array entry -> unused */
    /* 0x20 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x24 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x28 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x2C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

		/* UPM Refresh Timer RAM array entry -> unused */
    /* 0x30 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x34 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x38 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

		/* UPM Exception RAM array entry -> unused */
    /* 0x3C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* UPM pattern for fast init */
static const uint upmTableFast[] =
{
    /* Offset	UPM Read Single RAM array entry */
    /* 0x00 */	0xffffee00, 0x00ffcc80, 0x00ffcd80, 0x00ffdc00,
    /* 0x04 */	0x00ffdc00, 0x00ffcf00, 0x00ffec00, 0x3fffcc07,

		/* UPM Read Burst RAM array entry -> unused */
    /* 0x08 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x0C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

		/* UPM Read Burst RAM array entry -> unused */
    /* 0x10 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x14 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

		/* UPM Write Single RAM array entry */
    /* 0x18 */	0xffffee00, 0x00ffec80, 0x00ffee80, 0x00fffc00,
    /* 0x1C */	0x00fffc00, 0x00ffec00, 0x0fffef00, 0x3fffec05,

		/* UPM Write Burst RAM array entry -> unused */
    /* 0x20 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x24 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x28 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x2C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

		/* UPM Refresh Timer RAM array entry -> unused */
    /* 0x30 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x34 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x38 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

		/* UPM Exception RAM array entry -> unused */
    /* 0x3C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};


/* ------------------------------------------------------------------------- */

/* Check Board Identity:
 */
int checkboard (void)
{
	char *p = (char *) HWIB_INFO_START_ADDR;

	puts ("Board: ");
	if (*((unsigned long *)p) == (unsigned long)CONFIG_SYS_HWINFO_MAGIC) {
		puts (p);
	} else {
		puts ("No HWIB assuming TQM8272");
	}
	putc ('\n');

	return 0;
}

/* ------------------------------------------------------------------------- */
#if defined(CONFIG_BOARD_GET_CPU_CLK_F)
static int get_cas_latency (void)
{
	/* get it from the option -ts in CIB */
	/* default is 3 */
	int	ret = 3;
	int	pos = 0;
	char	*p = (char *) CIB_INFO_START_ADDR;

	while ((*p != '\0') && (pos < CIB_INFO_LEN)) {
		if (*p < ' ' || *p > '~') { /* ASCII strings! */
			return ret;
		}
		if (*p == '-') {
			if ((p[1] == 't') && (p[2] == 's')) {
				return (p[4] - '0');
			}
		}
		p++;
		pos++;
	}
	return ret;
}
#endif

static ulong set_sdram_timing (volatile uint *sdmr_ptr, ulong sdmr, int col)
{
#if defined(CONFIG_BOARD_GET_CPU_CLK_F)
	int	clk = board_get_cpu_clk_f ();
	volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	int	busmode = (immr->im_siu_conf.sc_bcr & BCR_EBM ? 1 : 0);
	int	cas;

	sdmr = sdmr & ~(PSDMR_RFRC_MSK | PSDMR_PRETOACT_MSK | PSDMR_WRC_MSK | \
			 PSDMR_BUFCMD);
	if (busmode) {
		switch (clk) {
			case 66666666:
				sdmr |= (PSDMR_RFRC_66MHZ_60X | \
					PSDMR_PRETOACT_66MHZ_60X | \
					PSDMR_WRC_66MHZ_60X | \
					PSDMR_BUFCMD_66MHZ_60X);
				break;
			case 100000000:
				sdmr |= (PSDMR_RFRC_100MHZ_60X | \
					PSDMR_PRETOACT_100MHZ_60X | \
					PSDMR_WRC_100MHZ_60X | \
					PSDMR_BUFCMD_100MHZ_60X);
				break;

		}
	} else {
		switch (clk) {
			case 66666666:
				sdmr |= (PSDMR_RFRC_66MHZ_SINGLE | \
					PSDMR_PRETOACT_66MHZ_SINGLE | \
					PSDMR_WRC_66MHZ_SINGLE | \
					PSDMR_BUFCMD_66MHZ_SINGLE);
				break;
			case 100000000:
				sdmr |= (PSDMR_RFRC_100MHZ_SINGLE | \
					PSDMR_PRETOACT_100MHZ_SINGLE | \
					PSDMR_WRC_100MHZ_SINGLE | \
					PSDMR_BUFCMD_100MHZ_SINGLE);
				break;
			case 133333333:
				sdmr |= (PSDMR_RFRC_133MHZ_SINGLE | \
					PSDMR_PRETOACT_133MHZ_SINGLE | \
					PSDMR_WRC_133MHZ_SINGLE | \
					PSDMR_BUFCMD_133MHZ_SINGLE);
				break;
		}
	}
	cas = get_cas_latency();
	sdmr &=~ (PSDMR_CL_MSK | PSDMR_LDOTOPRE_MSK);
	sdmr |= cas;
	sdmr |= ((cas - 1) << 6);
	return sdmr;
#else
	return sdmr;
#endif
}

/* Try SDRAM initialization with P/LSDMR=sdmr and ORx=orx
 *
 * This routine performs standard 8260 initialization sequence
 * and calculates the available memory size. It may be called
 * several times to try different SDRAM configurations on both
 * 60x and local buses.
 */
static long int try_init (volatile memctl8260_t * memctl, ulong sdmr,
						  ulong orx, volatile uchar * base, int col)
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
	sdmr = set_sdram_timing (sdmr_ptr, sdmr, col);
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

#ifndef CONFIG_SYS_RAMBOOT
	/* 60x SDRAM setup:
	 */
	size8 = try_init (memctl, CONFIG_SYS_PSDMR_8COL, CONFIG_SYS_OR1_8COL,
					  (uchar *) CONFIG_SYS_SDRAM_BASE, 8);
	size9 = try_init (memctl, CONFIG_SYS_PSDMR_9COL, CONFIG_SYS_OR1_9COL,
					  (uchar *) CONFIG_SYS_SDRAM_BASE, 9);

	if (size8 < size9) {
		psize = size9;
		printf ("(60x:9COL - %ld MB, ", psize >> 20);
	} else {
		psize = try_init (memctl, CONFIG_SYS_PSDMR_8COL, CONFIG_SYS_OR1_8COL,
						  (uchar *) CONFIG_SYS_SDRAM_BASE, 8);
		printf ("(60x:8COL - %ld MB, ", psize >> 20);
	}

#endif /* CONFIG_SYS_RAMBOOT */

	icache_enable ();

	return (psize);
}


static inline int scanChar (char *p, int len, unsigned long *number)
{
	int	akt = 0;

	*number = 0;
	while (akt < len) {
		if ((*p >= '0') && (*p <= '9')) {
			*number *= 10;
			*number += *p - '0';
			p += 1;
		} else {
			if (*p == '-')	return akt;
			return -1;
		}
		akt ++;
	}
	return akt;
}

static int dump_hwib(void)
{
	HWIB_INFO	*hw = &hwinf;
	volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	char *s = getenv("serial#");

	if (hw->OK) {
		printf ("HWIB on %x\n", HWIB_INFO_START_ADDR);
		printf ("serial : %s\n", s);
		printf ("ethaddr: %s\n", hw->ethaddr);
		printf ("FLASH	: %x nr:%d\n", hw->flash, hw->flash_nr);
		printf ("RAM	: %x cs:%d\n", hw->ram, hw->ram_cs);
		printf ("CPU	: %lu\n", hw->cpunr);
		printf ("CAN	: %d\n", hw->can);
		if (hw->eeprom) printf ("EEprom : %x\n", hw->eeprom);
		else printf ("No EEprom\n");
		if (hw->nand) {
			printf ("NAND	: %x\n", hw->nand);
			printf ("NAND CS: %d\n", hw->nand_cs);
		} else { printf ("No NAND\n");}
		printf ("Bus %s mode.\n", (hw->Bus ? "60x" : "Single PQII"));
		printf ("  real : %s\n", (immr->im_siu_conf.sc_bcr & BCR_EBM ? \
				 "60x" : "Single PQII"));
		printf ("Option : %lx\n", hw->option);
		printf ("%s Security Engine\n", (hw->SecEng ? "with" : "no"));
		printf ("CPM Clk: %d\n", hw->cpmcl);
		printf ("CPU Clk: %d\n", hw->cpucl);
		printf ("Bus Clk: %d\n", hw->buscl);
		if (hw->busclk_real_ok) {
			printf ("  real Clk: %d\n", hw->busclk_real);
		}
		printf ("CAS	: %d\n", get_cas_latency());
	} else {
		printf("HWIB @%x not OK\n", HWIB_INFO_START_ADDR);
	}
	return 0;
}

static inline int search_real_busclk (int *clk)
{
	int	part = 0, pos = 0;
	char *p = (char *) CIB_INFO_START_ADDR;
	int	ok = 0;

	while ((*p != '\0') && (pos < CIB_INFO_LEN)) {
		if (*p < ' ' || *p > '~') { /* ASCII strings! */
			return 0;
		}
		switch (part) {
		default:
			if (*p == '-') {
				++part;
			}
			break;
		case 3:
			if (*p == '-') {
				++part;
				break;
			}
			if (*p == 'b') {
				ok = 1;
				p++;
				break;
			}
			if (ok) {
				switch (*p) {
				case '6':
					*clk = 66666666;
					return 1;
					break;
				case '1':
					if (p[1] == '3') {
						*clk = 133333333;
					} else {
						*clk = 100000000;
					}
					return 1;
					break;
				}
			}
			break;
		}
		p++;
	}
	return 0;
}

int analyse_hwib (void)
{
	char	*p = (char *) HWIB_INFO_START_ADDR;
	int	anz;
	int	part = 1, i = 0, pos = 0;
	HWIB_INFO	*hw = &hwinf;

	deb_printf(" %s pointer: %p\n", __FUNCTION__, p);
	/* Head = TQM */
	if (*((unsigned long *)p) != (unsigned long)CONFIG_SYS_HWINFO_MAGIC) {
		deb_printf("No HWIB\n");
		return -1;
	}
	p += 3;
	if (scanChar (p, 4, &hw->cpunr) < 0) {
		deb_printf("No CPU\n");
		return -2;
	}
	p +=4;

	hw->flash = 0x200000 << (*p - 'A');
	p++;
	hw->flash_nr = *p - '0';
	p++;

	hw->ram = 0x2000000 << (*p - 'A');
	p++;
	if (*p == '2') {
		hw->ram_cs = 2;
		p++;
	}

	if (*p == 'A') hw->can = 1;
	if (*p == 'B') hw->can = 2;
	p +=1;
	p +=1;	/* connector */
	if (*p != '0') {
		hw->eeprom = 0x1000 << (*p - 'A');
	}
	p++;

	if ((*p < '0') || (*p > '9')) {
		/* NAND before z-option */
		hw->nand = 0x8000000 << (*p - 'A');
		p++;
		hw->nand_cs = *p - '0';
		p += 2;
	}
	/* z-option */
	anz = scanChar (p, 4, &hw->option);
	if (anz < 0) {
		deb_printf("No option\n");
		return -3;
	}
	if (hw->option & 0x8) hw->Bus = 1;
	p += anz;
	if (*p != '-') {
		deb_printf("No -\n");
		return -4;
	}
	p++;
	/* C option */
	if (*p == 'E') {
		hw->SecEng = 1;
		p++;
	}
	switch (*p) {
		case 'M': hw->cpucl = 266666666;
			break;
		case 'P': hw->cpucl = 300000000;
			break;
		case 'T': hw->cpucl = 400000000;
			break;
		default:
			deb_printf("No CPU Clk: %c\n", *p);
			return -5;
			break;
	}
	p++;
	switch (*p) {
		case 'I': hw->cpmcl = 200000000;
			break;
		case 'M': hw->cpmcl = 300000000;
			break;
		default:
			deb_printf("No CPM Clk\n");
			return -6;
			break;
	}
	p++;
	switch (*p) {
		case 'B': hw->buscl = 66666666;
			break;
		case 'E': hw->buscl = 100000000;
			break;
		case 'F': hw->buscl = 133333333;
			break;
		default:
			deb_printf("No BUS Clk\n");
			return -7;
			break;
	}
	p++;

	hw->OK = 1;
	/* search MAC Address */
	while ((*p != '\0') && (pos < CONFIG_SYS_HWINFO_SIZE)) {
		if (*p < ' ' || *p > '~') { /* ASCII strings! */
			return 0;
		}
		switch (part) {
		default:
			if (*p == ' ') {
				++part;
				i = 0;
			}
			break;
		case 3:			/* Copy MAC address */
			if (*p == ' ') {
				++part;
				i = 0;
				break;
			}
			hw->ethaddr[i++] = *p;
			if ((i % 3) == 2)
				hw->ethaddr[i++] = ':';
			break;

		}
		p++;
	}

	hw->busclk_real_ok = search_real_busclk (&hw->busclk_real);
	return 0;
}

#if defined(CONFIG_GET_CPU_STR_F)
/* !! This routine runs from Flash */
char get_cpu_str_f (char *buf)
{
	char *p = (char *) HWIB_INFO_START_ADDR;
	int	i = 0;

	buf[i++] = 'M';
	buf[i++] = 'P';
	buf[i++] = 'C';
	if (*((unsigned long *)p) == (unsigned long)CONFIG_SYS_HWINFO_MAGIC) {
		buf[i++] = *&p[3];
		buf[i++] = *&p[4];
		buf[i++] = *&p[5];
		buf[i++] = *&p[6];
	} else {
		buf[i++] = '8';
		buf[i++] = '2';
		buf[i++] = '7';
		buf[i++] = 'x';
	}
	buf[i++] = 0;
	return 0;
}
#endif

#if defined(CONFIG_BOARD_GET_CPU_CLK_F)
/* !! This routine runs from Flash */
unsigned long board_get_cpu_clk_f (void)
{
	char *p = (char *) HWIB_INFO_START_ADDR;
	int i = 0;

	if (*((unsigned long *)p) == (unsigned long)CONFIG_SYS_HWINFO_MAGIC) {
		if (search_real_busclk (&i))
			return i;
	}
	return CONFIG_8260_CLKIN;
}
#endif

#if CONFIG_BOARD_EARLY_INIT_R

static int can_test (unsigned long off)
{
	volatile unsigned char	*base	= (unsigned char *) (CONFIG_SYS_CAN_BASE + off);

	*(base + 0x17) = 'T';
	*(base + 0x18) = 'Q';
	*(base + 0x19) = 'M';
	if ((*(base + 0x17) != 'T') ||
	    (*(base + 0x18) != 'Q') ||
	    (*(base + 0x19) != 'M')) {
		return 0;
	}
	return 1;
}

static int can_config_one (unsigned long off)
{
	volatile unsigned char	*ctrl	= (unsigned char *) (CONFIG_SYS_CAN_BASE + off);
	volatile unsigned char	*cpu_if = (unsigned char *) (CONFIG_SYS_CAN_BASE + off + 0x02);
	volatile unsigned char	*clkout = (unsigned char *) (CONFIG_SYS_CAN_BASE + off + 0x1f);
	unsigned char temp;

	*cpu_if = 0x45;
	temp = *ctrl;
	temp |= 0x40;
	*ctrl	= temp;
	*clkout = 0x20;
	temp = *ctrl;
	temp &= ~0x40;
	*ctrl	= temp;
	return 0;
}

static int can_config (void)
{
	int	ret = 0;
	can_config_one (0);
	if (hwinf.can == 2) {
		can_config_one (0x100);
	}
	/* make Test if they really there */
	ret += can_test (0);
	ret += can_test (0x100);
	return ret;
}

static int init_can (void)
{
	volatile immap_t * immr = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immr->im_memctl;
	int	count = 0;

	if ((hwinf.OK) && (hwinf.can)) {
		memctl->memc_or4 = CONFIG_SYS_CAN_OR;
		memctl->memc_br4 = CONFIG_SYS_CAN_BR;
		/* upm Init */
		upmconfig (UPMC, (uint *) upmTableFast,
			   sizeof (upmTableFast) / sizeof (uint));
		memctl->memc_mcmr =	(MxMR_DSx_3_CYCL |
					MxMR_GPL_x4DIS |
					MxMR_RLFx_2X |
					MxMR_WLFx_2X |
					MxMR_OP_NORM);
		/* can configure */
		count = can_config ();
		printf ("CAN:	%d @ %x\n", count, CONFIG_SYS_CAN_BASE);
		if (hwinf.can != count) printf("!!! difference to HWIB\n");
	} else {
		printf ("CAN:	No\n");
	}
	return 0;
}

int board_early_init_r(void)
{
	analyse_hwib ();
	init_can ();
	return 0;
}
#endif

int do_hwib_dump (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	dump_hwib ();
	return 0;
}

U_BOOT_CMD(
	  hwib, 1,	1,	do_hwib_dump,
	  "dump HWIB'",
	  ""
);

#ifdef CONFIG_SYS_UPDATE_FLASH_SIZE
static int get_flash_timing (void)
{
	/* get it from the option -tf in CIB */
	/* default is 0x00000c84 */
	int	ret = 0x00000c84;
	int	pos = 0;
	int	nr = 0;
	char	*p = (char *) CIB_INFO_START_ADDR;

	while ((*p != '\0') && (pos < CIB_INFO_LEN)) {
		if (*p < ' ' || *p > '~') { /* ASCII strings! */
			return ret;
		}
		if (*p == '-') {
			if ((p[1] == 't') && (p[2] == 'f')) {
				p += 6;
				ret = 0;
				while (nr < 8) {
				if ((*p >= '0') && (*p <= '9')) {
					ret *= 0x10;
					ret += *p - '0';
					p += 1;
					nr ++;
				} else if ((*p >= 'A') && (*p <= 'F')) {
					ret *= 10;
					ret += *p - '7';
					p += 1;
					nr ++;
				} else {
					if (nr < 8) return 0x00000c84;
					return ret;
				}
				}
			}
		}
		p++;
		pos++;
	}
	return ret;
}

/* Update the Flash_Size and the Flash Timing */
int update_flash_size (int flash_size)
{
	volatile immap_t * immr = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immr->im_memctl;
	unsigned long reg;
	unsigned long tim;

	/* I must use reg, otherwise the board hang */
	reg = memctl->memc_or0;
	reg &= ~ORxU_AM_MSK;
	reg |= MEG_TO_AM(flash_size >> 20);
	tim = get_flash_timing ();
	reg &= ~0xfff;
	reg |= (tim & 0xfff);
	memctl->memc_or0 = reg;
	return 0;
}
#endif

#ifdef CONFIG_PCI
struct pci_controller hose;

int board_early_init_f (void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	immap->im_clkrst.car_sccr |= M826X_SCCR_PCI_MODE_EN;
	return 0;
}

extern void pci_mpc8250_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc8250_init(&hose);
}
#endif

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
