/*
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
#include <i2c.h>

/* define to initialise the SDRAM on the local bus */
#undef INIT_LOCAL_BUS_SDRAM

/* I2C Bus adresses for PPC & Protocol board */
#define PPC8260_I2C_ADR		0x30	/*(0)011.0000 */
#define LM84_PPC_I2C_ADR	0x2A	/*(0)010.1010 */
#define LM84_SHARC_I2C_ADR	0x29	/*(0)010.1001 */
#define VIRTEX_I2C_ADR		0x25	/*(0)010.0101 */
#define X24645_PPC_I2C_ADR	0x00	/*(0)00X.XXXX  -> be careful ! No other i2c-chip should have an adress beginning with (0)00 !!! */
#define RS5C372_PPC_I2C_ADR	0x32	/*(0)011.0010  -> this adress is programmed by the manufacturer and cannot be changed !!! */

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PA31 */ {   0,   0,   0,   0,   0,   0   },
	/* PA30 */ {   0,   0,   0,   0,   0,   0   },
	/* PA29 */ {   0,   0,   0,   0,   0,   0   },
	/* PA28 */ {   0,   0,   0,   0,   0,   0   },
	/* PA27 */ {   0,   0,   0,   0,   0,   0   },
	/* PA26 */ {   0,   0,   0,   0,   0,   0   },
	/* PA25 */ {   0,   0,   0,   0,   0,   0   },
	/* PA24 */ {   0,   0,   0,   0,   0,   0   },
	/* PA23 */ {   0,   0,   0,   0,   0,   0   },
	/* PA22 */ {   0,   0,   0,   0,   0,   0   },
	/* PA21 */ {   0,   0,   0,   0,   0,   0   },
	/* PA20 */ {   0,   0,   0,   0,   0,   0   },
	/* PA19 */ {   0,   0,   0,   0,   0,   0   },
	/* PA18 */ {   0,   0,   0,   0,   0,   0   },
	/* PA17 */ {   0,   0,   0,   0,   0,   0   },
	/* PA16 */ {   0,   0,   0,   0,   0,   0   },
	/* PA15 */ {   0,   0,   0,   0,   0,   0   },
	/* PA14 */ {   0,   0,   0,   0,   0,   0   },
	/* PA13 */ {   0,   0,   0,   0,   0,   0   },
	/* PA12 */ {   0,   0,   0,   0,   0,   0   },
	/* PA11 */ {   0,   0,   0,   0,   0,   0   },
	/* PA10 */ {   0,   0,   0,   0,   0,   0   },
	/* PA9  */ {   0,   0,   0,   0,   0,   0   },
	/* PA8  */ {   0,   0,   0,   0,   0,   0   },
	/* PA7  */ {   0,   0,   0,   0,   0,   0   },
	/* PA6  */ {   0,   0,   0,   0,   0,   0   },
	/* PA5  */ {   0,   0,   0,   0,   0,   0   },
	/* PA4  */ {   0,   0,   0,   0,   0,   0   },
	/* PA3  */ {   0,   0,   0,   0,   0,   0   },
	/* PA2  */ {   0,   0,   0,   0,   0,   0   },
	/* PA1  */ {   0,   0,   0,   0,   0,   0   },
	/* PA0  */ {   0,   0,   0,   0,   0,   0   }
    },


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
	/* PB17 */ {   0,   0,   0,   0,   0,   0   },
	/* PB16 */ {   0,   0,   0,   0,   0,   0   },
	/* PB15 */ {   0,   0,   0,   0,   0,   0   },
	/* PB14 */ {   0,   0,   0,   0,   0,   0   },
	/* PB13 */ {   0,   0,   0,   0,   0,   0   },
	/* PB12 */ {   0,   0,   0,   0,   0,   0   },
	/* PB11 */ {   0,   0,   0,   0,   0,   0   },
	/* PB10 */ {   0,   0,   0,   0,   0,   0   },
	/* PB9  */ {   0,   0,   0,   0,   0,   0   },
	/* PB8  */ {   0,   0,   0,   0,   0,   0   },
	/* PB7  */ {   0,   0,   0,   0,   0,   0   },
	/* PB6  */ {   0,   0,   0,   0,   0,   0   },
	/* PB5  */ {   0,   0,   0,   0,   0,   0   },
	/* PB4  */ {   0,   0,   0,   0,   0,   0   },
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    },


    {   /*	      conf ppar psor pdir podr pdat */
	/* PC31 */ {   0,   0,   0,   0,   0,   0   },
	/* PC30 */ {   0,   0,   0,   0,   0,   0   },
	/* PC29 */ {   0,   0,   0,   0,   0,   0   },
	/* PC28 */ {   0,   0,   0,   0,   0,   0   },
	/* PC27 */ {   0,   0,   0,   0,   0,   0   },
	/* PC26 */ {   0,   0,   0,   0,   0,   0   },
	/* PC25 */ {   0,   0,   0,   0,   0,   0   },
	/* PC24 */ {   0,   0,   0,   0,   0,   0   },
	/* PC23 */ {   0,   0,   0,   0,   0,   0   },
	/* PC22 */ {   0,   0,   0,   0,   0,   0   },
	/* PC21 */ {   0,   0,   0,   0,   0,   0   },
	/* PC20 */ {   0,   0,   0,   0,   0,   0   },
	/* PC19 */ {   1,   1,   0,   0,   0,   0   },
	/* PC18 */ {   1,   1,   0,   0,   0,   0   }, /* ETHRXCLK: CLK14 */
	/* PC17 */ {   0,   0,   0,   0,   0,   0   }, /* ETHTXCLK: CLK15 */
	/* PC16 */ {   0,   0,   0,   0,   0,   0   },
	/* PC15 */ {   0,   0,   0,   0,   0,   0   },
	/* PC14 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 UART CD/ */
	/* PC13 */ {   0,   0,   0,   0,   0,   0   },
	/* PC12 */ {   0,   0,   0,   0,   0,   0   },
	/* PC11 */ {   0,   0,   0,   0,   0,   0   },
	/* PC10 */ {   1,   0,   0,   1,   0,   0   }, /* ETHMDC: GP */
	/* PC9  */ {   1,   0,   0,   1,   0,   0   }, /* ETHMDIO: GP */
	/* PC8  */ {   0,   0,   0,   0,   0,   0   },
	/* PC7  */ {   0,   0,   0,   0,   0,   0   },
	/* PC6  */ {   0,   0,   0,   0,   0,   0   },
	/* PC5  */ {   0,   0,   0,   0,   0,   0   },
	/* PC4  */ {   0,   0,   0,   0,   0,   0   },
	/* PC3  */ {   0,   0,   0,   0,   0,   0   },
	/* PC2  */ {   0,   0,   0,   0,   0,   0   },
	/* PC1  */ {   0,   0,   0,   0,   0,   0   },
	/* PC0  */ {   0,   0,   0,   0,   0,   0   }
    },


    {   /*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 UART RxD */
	/* PD30 */ {   1,   1,   1,   1,   0,   0   }, /* SCC1 UART TxD */
	/* PD29 */ {   0,   0,   0,   0,   0,   0   },
	/* PD28 */ {   0,   0,   0,   0,   0,   0   },
	/* PD27 */ {   0,   0,   0,   0,   0,   0   },
	/* PD26 */ {   0,   0,   0,   0,   0,   0   },
	/* PD25 */ {   0,   0,   0,   0,   0,   0   },
	/* PD24 */ {   0,   0,   0,   0,   0,   0   },
	/* PD23 */ {   0,   0,   0,   0,   0,   0   },
	/* PD22 */ {   0,   0,   0,   0,   0,   0   },
	/* PD21 */ {   0,   0,   0,   0,   0,   0   },
	/* PD20 */ {   0,   0,   0,   0,   0,   0   },
	/* PD19 */ {   0,   0,   0,   0,   0,   0   },
	/* PD18 */ {   0,   0,   0,   0,   0,   0   },
	/* PD17 */ {   0,   0,   0,   0,   0,   0   },
	/* PD16 */ {   0,   0,   0,   0,   0,   0   },
	/* PD15 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SCL */
	/* PD13 */ {   0,   0,   0,   0,   0,   0   },
	/* PD12 */ {   0,   0,   0,   0,   0,   0   },
	/* PD11 */ {   0,   0,   0,   0,   0,   0   },
	/* PD10 */ {   0,   0,   0,   0,   0,   0   },
	/* PD9  */ {   0,   0,   0,   0,   0,   0   },
	/* PD8  */ {   0,   0,   0,   0,   0,   0   },
	/* PD7  */ {   0,   0,   0,   0,   0,   0   },
	/* PD6  */ {   0,   0,   0,   0,   0,   0   },
	/* PD5  */ {   0,   0,   0,   0,   0,   0   },
	/* PD4  */ {   0,   0,   0,   0,   0,   0   },
	/* PD3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    }
};

/* ------------------------------------------------------------------------- */

struct tm {
	unsigned int tm_sec;
	unsigned int tm_min;
	unsigned int tm_hour;
	unsigned int tm_wday;
	unsigned int tm_mday;
	unsigned int tm_mon;
	unsigned int tm_year;
};

void read_RS5C372_time (struct tm *timedate)
{
	unsigned char buffer[8];

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

	if (! i2c_read (RS5C372_PPC_I2C_ADR, 0, 1, buffer, sizeof (buffer))) {
		timedate->tm_sec = BCD_TO_BIN (buffer[0]);
		timedate->tm_min = BCD_TO_BIN (buffer[1]);
		timedate->tm_hour = BCD_TO_BIN (buffer[2]);
		timedate->tm_wday = BCD_TO_BIN (buffer[3]);
		timedate->tm_mday = BCD_TO_BIN (buffer[4]);
		timedate->tm_mon = BCD_TO_BIN (buffer[5]);
		timedate->tm_year = BCD_TO_BIN (buffer[6]) + 2000;
	} else {
		/*printf("i2c error %02x\n", rc); */
		memset (timedate, 0, sizeof (struct tm));
	}
}

/* ------------------------------------------------------------------------- */

int read_LM84_temp (int address)
{
	unsigned char buffer[8];
	/*int rc;*/

	if (! i2c_read (address, 0, 1, buffer, 1)) {
		return (int) buffer[0];
	} else {
		/*printf("i2c error %02x\n", rc); */
		return -42;
	}
}

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	struct tm timedate;
	unsigned int ppctemp, prottemp;

	puts ("Board: Rohde & Schwarz 8260 Protocol Board\n");

	/* initialise i2c */
	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	read_RS5C372_time (&timedate);
	printf ("  Time:  %02d:%02d:%02d\n",
			timedate.tm_hour, timedate.tm_min, timedate.tm_sec);
	printf ("  Date:  %02d-%02d-%04d\n",
			timedate.tm_mday, timedate.tm_mon, timedate.tm_year);
	ppctemp = read_LM84_temp (LM84_PPC_I2C_ADR);
	prottemp = read_LM84_temp (LM84_SHARC_I2C_ADR);
	printf ("  Temp:  PPC %d C, Protocol Board %d C\n",
			ppctemp, prottemp);

	return 0;
}

/* ------------------------------------------------------------------------- */

/*
 * Miscelaneous platform dependent initialisations while still
 * running in flash
 */

int misc_init_f (void)
{
	return 0;
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;

#ifdef INIT_LOCAL_BUS_SDRAM
	volatile uchar *ramaddr8;
#endif
	volatile ulong *ramaddr32;
	ulong sdmr;
	int i;

	/*
	 * Only initialize SDRAM when running from FLASH.
	 * When running from RAM, don't touch it.
	 */
	if ((ulong) initdram & 0xff000000) {
		immap->im_siu_conf.sc_ppc_acr = 0x02;
		immap->im_siu_conf.sc_ppc_alrh = 0x01267893;
		immap->im_siu_conf.sc_ppc_alrl = 0x89ABCDEF;
		immap->im_siu_conf.sc_lcl_acr = 0x02;
		immap->im_siu_conf.sc_lcl_alrh = 0x01234567;
		immap->im_siu_conf.sc_lcl_alrl = 0x89ABCDEF;
		/*
		 * Program local/60x bus Transfer Error Status and Control Regs:
		 * Disable parity errors
		 */
		immap->im_siu_conf.sc_tescr1 = 0x00040000;
		immap->im_siu_conf.sc_ltescr1 = 0x00040000;

		/*
		 * Perform Power-Up Initialisation of SDRAM (see 8260 UM, 10.4.2)
		 *
		 * The appropriate BRx/ORx registers have already
		 * been set when we get here (see cpu_init_f). The
		 * SDRAM can be accessed at the address CONFIG_SYS_SDRAM_BASE.
		 */
		memctl->memc_mptpr = 0x2000;
		memctl->memc_mar = 0x0200;
#ifdef INIT_LOCAL_BUS_SDRAM
		/* initialise local bus ram
		 *
		 * (using the PSRMR_ definitions is NOT an error here
		 * - the LSDMR has the same fields as the PSDMR!)
		 */
		memctl->memc_lsrt = 0x0b;
		memctl->memc_lurt = 0x00;
		ramaddr = (uchar *) PHYS_SDRAM_LOCAL;
		sdmr = CONFIG_SYS_LSDMR & ~(PSDMR_OP_MSK | PSDMR_RFEN | PSDMR_PBI);
		memctl->memc_lsdmr = sdmr | PSDMR_OP_PREA;
		*ramaddr = 0xff;
		for (i = 0; i < 8; i++) {
			memctl->memc_lsdmr = sdmr | PSDMR_OP_CBRR;
			*ramaddr = 0xff;
		}
		memctl->memc_lsdmr = sdmr | PSDMR_OP_MRW;
		*ramaddr = 0xff;
		memctl->memc_lsdmr = CONFIG_SYS_LSDMR | PSDMR_OP_NORM;
#endif
		/* initialise 60x bus ram */
		memctl->memc_psrt = 0x0b;
		memctl->memc_purt = 0x08;
		ramaddr32 = (ulong *) PHYS_SDRAM_60X;
		sdmr = CONFIG_SYS_PSDMR & ~(PSDMR_OP_MSK | PSDMR_RFEN | PSDMR_PBI);
		memctl->memc_psdmr = sdmr | PSDMR_OP_PREA;
		ramaddr32[0] = 0x00ff00ff;
		ramaddr32[1] = 0x00ff00ff;
		memctl->memc_psdmr = sdmr | PSDMR_OP_CBRR;
		for (i = 0; i < 8; i++) {
			ramaddr32[0] = 0x00ff00ff;
			ramaddr32[1] = 0x00ff00ff;
		}
		memctl->memc_psdmr = sdmr | PSDMR_OP_MRW;
		ramaddr32[0] = 0x00ff00ff;
		ramaddr32[1] = 0x00ff00ff;
		memctl->memc_psdmr = sdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	}

	/* return the size of the 60x bus ram */
	return PHYS_SDRAM_60X_SIZE;
}

/* ------------------------------------------------------------------------- */

/*
 * Miscelaneous platform dependent initialisations after monitor
 * has been relocated into ram
 */

int misc_init_r (void)
{
	printf ("misc_init_r\n");
	return (0);
}
