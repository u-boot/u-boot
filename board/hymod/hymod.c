/*
 * (C) Copyright 2000
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
 *
 * Hacked for the Hymod board by Murray.Jensen@cmst.csiro.au, 20-Oct-00
 */

#include <common.h>
#include <mpc8260.h>
#include <ioports.h>
#include <i2c.h>
#include <asm/iopin_8260.h>

/* ------------------------------------------------------------------------- */

/* imports from eeprom.c */
extern int eeprom_load (unsigned, hymod_eeprom_t *);
extern int eeprom_fetch (unsigned, char *, ulong);
extern void eeprom_print (hymod_eeprom_t *);

/* imports from fetch.c */
extern int fetch_and_parse (char *, ulong, int (*)(uchar *, uchar *));

/* imports from common/main.c */
extern char console_buffer[CFG_CBSIZE];

/* ------------------------------------------------------------------------- */

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

	/* Port A configuration */
	{							/*        conf ppar psor pdir podr pdat */
									/* PA31 */ {1, 1, 1, 0, 0, 0},
									/* FCC1 MII COL */
									/* PA30 */ {1, 1, 1, 0, 0, 0},
									/* FCC1 MII CRS */
									/* PA29 */ {1, 1, 1, 1, 0, 0},
									/* FCC1 MII TX_ER */
									/* PA28 */ {1, 1, 1, 1, 0, 0},
									/* FCC1 MII TX_EN */
									/* PA27 */ {1, 1, 1, 0, 0, 0},
									/* FCC1 MII RX_DV */
									/* PA26 */ {1, 1, 1, 0, 0, 0},
									/* FCC1 MII RX_ER */
									/* PA25 */ {1, 0, 0, 1, 0, 0},
									/* FCC2 MII MDIO */
									/* PA24 */ {1, 0, 0, 1, 0, 0},
									/* FCC2 MII MDC */
									/* PA23 */ {1, 0, 0, 1, 0, 0},
									/* FCC3 MII MDIO */
									/* PA22 */ {1, 0, 0, 1, 0, 0},
									/* FCC3 MII MDC */
									/* PA21 */ {1, 1, 0, 1, 0, 0},
									/* FCC1 MII TxD[3] */
									/* PA20 */ {1, 1, 0, 1, 0, 0},
									/* FCC1 MII TxD[2] */
									/* PA19 */ {1, 1, 0, 1, 0, 0},
									/* FCC1 MII TxD[1] */
									/* PA18 */ {1, 1, 0, 1, 0, 0},
									/* FCC1 MII TxD[0] */
									/* PA17 */ {1, 1, 0, 0, 0, 0},
									/* FCC1 MII RxD[3] */
									/* PA16 */ {1, 1, 0, 0, 0, 0},
									/* FCC1 MII RxD[2] */
									/* PA15 */ {1, 1, 0, 0, 0, 0},
									/* FCC1 MII RxD[1] */
									/* PA14 */ {1, 1, 0, 0, 0, 0},
									/* FCC1 MII RxD[0] */
									/* PA13 */ {1, 0, 0, 1, 0, 0},
									/* FCC1 MII MDIO */
									/* PA12 */ {1, 0, 0, 1, 0, 0},
									/* FCC1 MII MDC */
									/* PA11 */ {1, 0, 0, 1, 0, 0},
									/* SEL_CD */
									/* PA10 */ {1, 0, 0, 0, 0, 0},
									/* FLASH STS1 */
									/* PA9  */ {1, 0, 0, 0, 0, 0},
									/* FLASH STS0 */
									/* PA8  */ {1, 0, 0, 0, 0, 0},
									/* FLASH ~PE */
									/* PA7  */ {1, 0, 0, 0, 0, 0},
									/* WATCH ~HRESET */
									/* PA6  */ {1, 0, 0, 0, 1, 0},
									/* VC DONE */
									/* PA5  */ {1, 0, 0, 1, 1, 0},
									/* VC INIT */
									/* PA4  */ {1, 0, 0, 1, 0, 0},
									/* VC ~PROG */
									/* PA3  */ {1, 0, 0, 1, 0, 0},
									/* VM ENABLE */
									/* PA2  */ {1, 0, 0, 0, 1, 0},
									/* VM DONE */
									/* PA1  */ {1, 0, 0, 1, 1, 0},
									/* VM INIT */
									/* PA0  */ {1, 0, 0, 1, 0, 0}
									/* VM ~PROG */
	 },

	/* Port B configuration */
	{							/*        conf ppar psor pdir podr pdat */
									/* PB31 */ {1, 1, 0, 1, 0, 0},
									/* FCC2 MII TX_ER */
									/* PB30 */ {1, 1, 0, 0, 0, 0},
									/* FCC2 MII RX_DV */
									/* PB29 */ {1, 1, 1, 1, 0, 0},
									/* FCC2 MII TX_EN */
									/* PB28 */ {1, 1, 0, 0, 0, 0},
									/* FCC2 MII RX_ER */
									/* PB27 */ {1, 1, 0, 0, 0, 0},
									/* FCC2 MII COL */
									/* PB26 */ {1, 1, 0, 0, 0, 0},
									/* FCC2 MII CRS */
									/* PB25 */ {1, 1, 0, 1, 0, 0},
									/* FCC2 MII TxD[3] */
									/* PB24 */ {1, 1, 0, 1, 0, 0},
									/* FCC2 MII TxD[2] */
									/* PB23 */ {1, 1, 0, 1, 0, 0},
									/* FCC2 MII TxD[1] */
									/* PB22 */ {1, 1, 0, 1, 0, 0},
									/* FCC2 MII TxD[0] */
									/* PB21 */ {1, 1, 0, 0, 0, 0},
									/* FCC2 MII RxD[0] */
									/* PB20 */ {1, 1, 0, 0, 0, 0},
									/* FCC2 MII RxD[1] */
									/* PB19 */ {1, 1, 0, 0, 0, 0},
									/* FCC2 MII RxD[2] */
									/* PB18 */ {1, 1, 0, 0, 0, 0},
									/* FCC2 MII RxD[3] */
									/* PB17 */ {1, 1, 0, 0, 0, 0},
									/* FCC3 MII RX_DV */
									/* PB16 */ {1, 1, 0, 0, 0, 0},
									/* FCC3 MII RX_ER */
									/* PB15 */ {1, 1, 0, 1, 0, 0},
									/* FCC3 MII TX_ER */
									/* PB14 */ {1, 1, 0, 1, 0, 0},
									/* FCC3 MII TX_EN */
									/* PB13 */ {1, 1, 0, 0, 0, 0},
									/* FCC3 MII COL */
									/* PB12 */ {1, 1, 0, 0, 0, 0},
									/* FCC3 MII CRS */
									/* PB11 */ {1, 1, 0, 0, 0, 0},
									/* FCC3 MII RxD[3] */
									/* PB10 */ {1, 1, 0, 0, 0, 0},
									/* FCC3 MII RxD[2] */
									/* PB9  */ {1, 1, 0, 0, 0, 0},
									/* FCC3 MII RxD[1] */
									/* PB8  */ {1, 1, 0, 0, 0, 0},
									/* FCC3 MII RxD[0] */
									/* PB7  */ {1, 1, 0, 1, 0, 0},
									/* FCC3 MII TxD[3] */
									/* PB6  */ {1, 1, 0, 1, 0, 0},
									/* FCC3 MII TxD[2] */
									/* PB5  */ {1, 1, 0, 1, 0, 0},
									/* FCC3 MII TxD[1] */
									/* PB4  */ {1, 1, 0, 1, 0, 0},
									/* FCC3 MII TxD[0] */
									/* PB3  */ {0, 0, 0, 0, 0, 0},
									/* pin doesn't exist */
									/* PB2  */ {0, 0, 0, 0, 0, 0},
									/* pin doesn't exist */
									/* PB1  */ {0, 0, 0, 0, 0, 0},
									/* pin doesn't exist */
									/* PB0  */ {0, 0, 0, 0, 0, 0}
									/* pin doesn't exist */
	 },

	/* Port C */
	{							/*        conf ppar psor pdir podr pdat */
									/* PC31 */ {1, 0, 0, 0, 0, 0},
									/* MEZ ~IACK */
	 /* PC30 */ {0, 0, 0, 0, 0, 0},
									/* PC29 */ {1, 1, 0, 0, 0, 0},
									/* CLK SCCx */
									/* PC28 */ {1, 1, 0, 0, 0, 0},
									/* CLK4 */
									/* PC27 */ {1, 1, 0, 0, 0, 0},
									/* CLK SCCF */
									/* PC26 */ {1, 1, 0, 0, 0, 0},
									/* CLK 32K */
									/* PC25 */ {1, 1, 0, 0, 0, 0},
									/* BRG4/CLK7 */
	 /* PC24 */ {0, 0, 0, 0, 0, 0},
									/* PC23 */ {1, 1, 0, 0, 0, 0},
									/* CLK SCCx */
									/* PC22 */ {1, 1, 0, 0, 0, 0},
									/* FCC1 MII RX_CLK */
									/* PC21 */ {1, 1, 0, 0, 0, 0},
									/* FCC1 MII TX_CLK */
									/* PC20 */ {1, 1, 0, 0, 0, 0},
									/* CLK SCCF */
									/* PC19 */ {1, 1, 0, 0, 0, 0},
									/* FCC2 MII RX_CLK */
									/* PC18 */ {1, 1, 0, 0, 0, 0},
									/* FCC2 MII TX_CLK */
									/* PC17 */ {1, 1, 0, 0, 0, 0},
									/* FCC3 MII RX_CLK */
									/* PC16 */ {1, 1, 0, 0, 0, 0},
									/* FCC3 MII TX_CLK */
									/* PC15 */ {1, 0, 0, 0, 0, 0},
									/* SCC1 UART ~CTS */
									/* PC14 */ {1, 0, 0, 0, 0, 0},
									/* SCC1 UART ~CD */
									/* PC13 */ {1, 0, 0, 0, 0, 0},
									/* SCC2 UART ~CTS */
									/* PC12 */ {1, 0, 0, 0, 0, 0},
									/* SCC2 UART ~CD */
									/* PC11 */ {1, 0, 0, 1, 0, 0},
									/* SCC1 UART ~DTR */
									/* PC10 */ {1, 0, 0, 1, 0, 0},
									/* SCC1 UART ~DSR */
									/* PC9  */ {1, 0, 0, 1, 0, 0},
									/* SCC2 UART ~DTR */
									/* PC8  */ {1, 0, 0, 1, 0, 0},
									/* SCC2 UART ~DSR */
									/* PC7  */ {1, 0, 0, 0, 0, 0},
									/* TEMP ~ALERT */
									/* PC6  */ {1, 0, 0, 0, 0, 0},
									/* FCC3 INT */
									/* PC5  */ {1, 0, 0, 0, 0, 0},
									/* FCC2 INT */
									/* PC4  */ {1, 0, 0, 0, 0, 0},
									/* FCC1 INT */
									/* PC3  */ {1, 1, 1, 1, 0, 0},
									/* SDMA IDMA2 ~DACK */
									/* PC2  */ {1, 1, 1, 0, 0, 0},
									/* SDMA IDMA2 ~DONE */
									/* PC1  */ {1, 1, 0, 0, 0, 0},
									/* SDMA IDMA2 ~DREQ */
									/* PC0  */ {1, 1, 0, 1, 0, 0}
									/* BRG7 */
	 },

	/* Port D */
	{							/*        conf ppar psor pdir podr pdat */
									/* PD31 */ {1, 1, 0, 0, 0, 0},
									/* SCC1 UART RxD */
									/* PD30 */ {1, 1, 1, 1, 0, 0},
									/* SCC1 UART TxD */
									/* PD29 */ {1, 0, 0, 1, 0, 0},
									/* SCC1 UART ~RTS */
									/* PD28 */ {1, 1, 0, 0, 0, 0},
									/* SCC2 UART RxD */
									/* PD27 */ {1, 1, 0, 1, 0, 0},
									/* SCC2 UART TxD */
									/* PD26 */ {1, 0, 0, 1, 0, 0},
									/* SCC2 UART ~RTS */
									/* PD25 */ {1, 0, 0, 0, 0, 0},
									/* SCC1 UART ~RI */
									/* PD24 */ {1, 0, 0, 0, 0, 0},
									/* SCC2 UART ~RI */
									/* PD23 */ {1, 0, 0, 1, 0, 0},
									/* CLKGEN PD */
									/* PD22 */ {1, 0, 0, 0, 0, 0},
									/* USER3 */
									/* PD21 */ {1, 0, 0, 0, 0, 0},
									/* USER2 */
									/* PD20 */ {1, 0, 0, 0, 0, 0},
									/* USER1 */
									/* PD19 */ {1, 1, 1, 0, 0, 0},
									/* SPI ~SEL */
									/* PD18 */ {1, 1, 1, 0, 0, 0},
									/* SPI CLK */
									/* PD17 */ {1, 1, 1, 0, 0, 0},
									/* SPI MOSI */
									/* PD16 */ {1, 1, 1, 0, 0, 0},
									/* SPI MISO */
									/* PD15 */ {1, 1, 1, 0, 1, 0},
									/* I2C SDA */
									/* PD14 */ {1, 1, 1, 0, 1, 0},
									/* I2C SCL */
									/* PD13 */ {1, 0, 0, 1, 0, 1},
									/* TEMP ~STDBY */
									/* PD12 */ {1, 0, 0, 1, 0, 1},
									/* FCC3 ~RESET */
									/* PD11 */ {1, 0, 0, 1, 0, 1},
									/* FCC2 ~RESET */
									/* PD10 */ {1, 0, 0, 1, 0, 1},
									/* FCC1 ~RESET */
									/* PD9  */ {1, 0, 0, 0, 0, 0},
									/* PD9 */
									/* PD8  */ {1, 0, 0, 0, 0, 0},
									/* PD8 */
									/* PD7  */ {1, 0, 0, 1, 0, 1},
									/* PD7 */
									/* PD6  */ {1, 0, 0, 1, 0, 1},
									/* PD6 */
									/* PD5  */ {1, 0, 0, 1, 0, 1},
									/* PD5 */
									/* PD4  */ {1, 0, 0, 1, 0, 1},
									/* PD4 */
									/* PD3  */ {0, 0, 0, 0, 0, 0},
									/* pin doesn't exist */
									/* PD2  */ {0, 0, 0, 0, 0, 0},
									/* pin doesn't exist */
									/* PD1  */ {0, 0, 0, 0, 0, 0},
									/* pin doesn't exist */
									/* PD0  */ {0, 0, 0, 0, 0, 0}
									/* pin doesn't exist */
	 }
};

/* ------------------------------------------------------------------------- */

/*
 * AMI FS6377 Clock Generator configuration table
 *
 * the "fs6377_regs[]" table entries correspond to FS6377 registers
 * 0 - 15 (total of 16 bytes).
 *
 * the data is written to the FS6377 via the i2c bus using address in
 * "fs6377_addr" (address is 7 bits - R/W bit not included).
 */

uchar fs6377_addr = 0x5c;

uchar fs6377_regs[16] = {
	12, 75, 64, 25, 144, 128, 25, 192,
	0, 16, 135, 192, 224, 64, 64, 192
};

iopin_t pa11 = { IOPIN_PORTA, 11, 0 };

/* ------------------------------------------------------------------------- */

/*
 * special board initialisation, after clocks and timebase have been
 * set up but before environment and serial are initialised.
 *
 * added so that very early initialisations can be done using the i2c
 * driver (which requires the clocks, to calculate the dividers, and
 * the timebase, for udelay())
 */

int board_postclk_init (void)
{
	i2c_init (CFG_I2C_SPEED, CFG_I2C_SLAVE);

	/*
	 * Initialise the FS6377 clock chip
	 *
	 * the secondary address is the register number from where to
	 * start the write - I want to write all the registers
	 *
	 * don't bother checking return status - we have no console yet
	 * to print it on, nor any RAM to store it in - it will be obvious
	 * if this doesn't work
	 */
	(void) i2c_write (fs6377_addr, 0, 1, fs6377_regs,
					  sizeof (fs6377_regs));

	return (0);
}

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity: Hardwired to HYMOD
 */

int checkboard (void)
{
	puts ("Board: HYMOD\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

/*
 * miscellaneous (early - while running in flash) initialisations.
 */

#define _NOT_USED_	0xFFFFFFFF

uint upmb_table[] = {
	/* Read Single Beat (RSS) - offset 0x00 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* Read Burst (RBS) - offset 0x08 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* Write Single Beat (WSS) - offset 0x18 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* Write Burst (WSS) - offset 0x20 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* Refresh Timer (PTS) - offset 0x30 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* Exception Condition (EXS) - offset 0x3c */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_
};

uint upmc_table[] = {
	/* Read Single Beat (RSS) - offset 0x00 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* Read Burst (RBS) - offset 0x08 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* Write Single Beat (WSS) - offset 0x18 */
	0xF0E00000, 0xF0A00000, 0x00A00000, 0x30A00000,
	0xF0F40007, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* Write Burst (WSS) - offset 0x20 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* Refresh Timer (PTS) - offset 0x30 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* Exception Condition (EXS) - offset 0x3c */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_
};

int misc_init_f (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;

	printf ("UPMs:  ");

	upmconfig (UPMB, upmb_table, sizeof upmb_table / sizeof upmb_table[0]);
	memctl->memc_mbmr = CFG_MBMR;

	upmconfig (UPMC, upmc_table, sizeof upmc_table / sizeof upmc_table[0]);
	memctl->memc_mcmr = CFG_MCMR;

	printf ("configured\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

long initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	volatile uchar c = 0, *ramaddr = (uchar *) (CFG_SDRAM_BASE + 0x8);
	ulong psdmr = CFG_PSDMR;
	int i;

	/*
	 * Quote from 8260 UM (10.4.2 SDRAM Power-On Initialization, 10-35):
	 *
	 * "At system reset, initialization software must set up the
	 *  programmable parameters in the memory controller banks registers
	 *  (ORx, BRx, P/LSDMR). After all memory parameters are conÞgured,
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

	memctl->memc_psdmr = psdmr | PSDMR_OP_PREA;
	*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++)
		*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_MRW;
	*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	*ramaddr = c;

	return (CFG_SDRAM_SIZE << 20);
}

/* ------------------------------------------------------------------------- */
/* miscellaneous initialisations after relocation into ram (misc_init_r)     */
/* 									     */
/* loads the data in the main board and mezzanine board eeproms into	     */
/* the hymod configuration struct stored in the board information area.	     */
/* 									     */
/* if the contents of either eeprom is invalid, prompts for a serial	     */
/* number (and an ethernet address if required) then fetches a file	     */
/* containing information to be stored in the eeprom from the tftp server    */
/* (the file name is based on the serial number and a built-in path)	     */

/* these are relative to the root of the server's tftp directory */
static char *bddb_cfgdir = "/hymod/bddb";
static char *global_env_path = "/hymod/global_env";

static ulong get_serno (const char *prompt)
{
	for (;;) {
		int n;
		char *p;
		ulong serno;

		n = readline (prompt);

		if (n < 0)
			return (0);

		if (n == 0)
			continue;

		serno = simple_strtol (console_buffer, &p, 10);

		if (p > console_buffer && *p == '\0')
			return (serno);

		printf ("Invalid number (%s) - please re-enter\n", console_buffer);
	}
}

static int read_eeprom (char *label, unsigned offset, hymod_eeprom_t * ep)
{
	char filename[50], prompt[50];
	ulong serno;
	int count = 0;

	sprintf (prompt, "Enter %s board serial number: ", label);

	for (;;) {

		if (eeprom_load (offset, ep))
			return (1);

		printf ("*** %s board EEPROM contents are %sinvalid\n",
				label, count == 0 ? "" : "STILL ");

		puts ("*** will attempt to fetch from server (Ctrl-C to abort)\n");

		if ((serno = get_serno (prompt)) == 0) {
			puts ("\n*** interrupted! - ignoring eeprom contents\n");
			return (0);
		}

		sprintf (filename, "%s/%010lu.cfg", bddb_cfgdir, serno);

		printf ("*** fetching %s board EEPROM contents from server\n",
				label);

		if (eeprom_fetch (offset, filename, 0x100000) == 0) {
			puts ("*** fetch failed - ignoring eeprom contents\n");
			return (0);
		}

		count++;
	}
}

static ulong main_serno;

static int env_fetch_callback (uchar * name, uchar * value)
{
	char *ov, nv[CFG_CBSIZE], *p, *q, *nn;
	int override = 1, append = 0, nl;

	nn = name;
	if (*nn == '-') {
		override = 0;
		nn++;
	}

	if ((nl = strlen (nn)) > 0 && nn[nl - 1] == '+') {
		append = 1;
		nn[--nl] = '\0';
	}

	p = value;
	q = nv;

	while ((*q = *p++) != '\0')
		if (*q == '%') {
			switch (*p++) {

			case '\0':			/* whoops - back up */
				p--;
				break;

			case '%':			/* a single percent character */
				q++;
				break;

			case 's':			/* main board serial number as string */
				q += sprintf (q, "%010lu", main_serno);
				break;

			case 'S':			/* main board serial number as number */
				q += sprintf (q, "%lu", main_serno);
				break;

			default:			/* ignore any others */
				break;
			}
		} else
			q++;

	if ((ov = getenv (nn)) != NULL) {

		if (append) {

			if (strstr (ov, nv) == NULL) {
				int ovl, nvl;

				printf ("Appending '%s' to env cmd '%s'\n", nv, nn);

				ovl = strlen (ov);
				nvl = strlen (nv);

				while (nvl >= 0) {
					nv[ovl + 1 + nvl] = nv[nvl];
					nvl--;
				}

				nv[ovl] = ' ';

				while (--ovl >= 0)
					nv[ovl] = ov[ovl];

				setenv (nn, nv);
			}

			return (1);
		}

		if (!override || strcmp (ov, nv) == 0)
			return (1);

		printf ("Re-setting env cmd '%s' from '%s' to '%s'\n", nn, ov, nv);
	} else
		printf ("Setting env cmd '%s' to '%s'\n", nn, nv);

	setenv (nn, nv);
	return (1);
}

int misc_init_r (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	hymod_conf_t *cp = &gd->bd->bi_hymod_conf;
	int rc;

	memset ((void *) cp, 0, sizeof (*cp));

	/* set up main board config info */

	if (i2c_probe (CFG_I2C_EEPROM_ADDR | HYMOD_EEOFF_MAIN)) {

		if (read_eeprom
			("main", HYMOD_EEOFF_MAIN << 8, &cp->main.eeprom))
			cp->main.eeprom_valid = 1;

		puts ("EEPROM:main...");

		if (cp->main.eeprom_valid) {
			printf ("OK (ver %u)\n", cp->main.eeprom.ver);
			eeprom_print (&cp->main.eeprom);
			main_serno = cp->main.eeprom.serno;
		} else
			puts ("BAD\n");

		cp->main.mmap[0].prog.exists = 1;
		cp->main.mmap[0].prog.size = FPGA_MAIN_CFG_SIZE;
		cp->main.mmap[0].prog.base = FPGA_MAIN_CFG_BASE;

		cp->main.mmap[0].reg.exists = 1;
		cp->main.mmap[0].reg.size = FPGA_MAIN_REG_SIZE;
		cp->main.mmap[0].reg.base = FPGA_MAIN_REG_BASE;

		cp->main.mmap[0].port.exists = 1;
		cp->main.mmap[0].port.size = FPGA_MAIN_PORT_SIZE;
		cp->main.mmap[0].port.base = FPGA_MAIN_PORT_BASE;

		cp->main.iopins[0].prog_pin.port = FPGA_MAIN_PROG_PORT;
		cp->main.iopins[0].prog_pin.pin = FPGA_MAIN_PROG_PIN;
		cp->main.iopins[0].prog_pin.flag = 1;
		cp->main.iopins[0].init_pin.port = FPGA_MAIN_INIT_PORT;
		cp->main.iopins[0].init_pin.pin = FPGA_MAIN_INIT_PIN;
		cp->main.iopins[0].init_pin.flag = 1;
		cp->main.iopins[0].done_pin.port = FPGA_MAIN_DONE_PORT;
		cp->main.iopins[0].done_pin.pin = FPGA_MAIN_DONE_PIN;
		cp->main.iopins[0].done_pin.flag = 1;
#ifdef FPGA_MAIN_ENABLE_PORT
		cp->main.iopins[0].enable_pin.port = FPGA_MAIN_ENABLE_PORT;
		cp->main.iopins[0].enable_pin.pin = FPGA_MAIN_ENABLE_PIN;
		cp->main.iopins[0].enable_pin.flag = 1;
#endif
	} else
		puts ("EEPROM:main...NOT PRESENT\n");

	/* set up mezzanine board config info */

	if (i2c_probe (CFG_I2C_EEPROM_ADDR | HYMOD_EEOFF_MEZZ)) {

		if (read_eeprom
			("mezz", HYMOD_EEOFF_MEZZ << 8, &cp->mezz.eeprom))
			cp->mezz.eeprom_valid = 1;

		puts ("EEPROM:mezz...");

		if (cp->mezz.eeprom_valid) {
			printf ("OK (ver %u)\n", cp->mezz.eeprom.ver);
			eeprom_print (&cp->mezz.eeprom);
		} else
			puts ("BAD\n");

		cp->mezz.mmap[0].prog.exists = 1;
		cp->mezz.mmap[0].prog.size = FPGA_MEZZ_CFG_SIZE;
		cp->mezz.mmap[0].prog.base = FPGA_MEZZ_CFG_BASE;

		cp->mezz.mmap[0].reg.exists = 0;

		cp->mezz.mmap[0].port.exists = 0;

		cp->mezz.iopins[0].prog_pin.port = FPGA_MEZZ_PROG_PORT;
		cp->mezz.iopins[0].prog_pin.pin = FPGA_MEZZ_PROG_PIN;
		cp->mezz.iopins[0].prog_pin.flag = 1;
		cp->mezz.iopins[0].init_pin.port = FPGA_MEZZ_INIT_PORT;
		cp->mezz.iopins[0].init_pin.pin = FPGA_MEZZ_INIT_PIN;
		cp->mezz.iopins[0].init_pin.flag = 1;
		cp->mezz.iopins[0].done_pin.port = FPGA_MEZZ_DONE_PORT;
		cp->mezz.iopins[0].done_pin.pin = FPGA_MEZZ_DONE_PIN;
		cp->mezz.iopins[0].done_pin.flag = 1;
#ifdef FPGA_MEZZ_ENABLE_PORT
		cp->mezz.iopins[0].enable_pin.port = FPGA_MEZZ_ENABLE_PORT;
		cp->mezz.iopins[0].enable_pin.pin = FPGA_MEZZ_ENABLE_PIN;
		cp->mezz.iopins[0].enable_pin.flag = 1;
#endif

		if (cp->mezz.eeprom_valid &&
			cp->mezz.eeprom.bdtype == HYMOD_BDTYPE_DISPLAY) {
			/*
			 * mezzanine board is a display board - switch the SEL_CD
			 * input of the FS6377 clock generator (via I/O Port Pin PA11) to
			 * high (or 1) to select the 27MHz required by the display board
			 */
			iopin_set_high (&pa11);

			puts ("SEL_CD:toggled for display board\n");
		}
	} else
		puts ("EEPROM:mezz...NOT PRESENT\n");

	cp->crc =
			crc32 (0, (unsigned char *) cp, offsetof (hymod_conf_t, crc));

	if (getenv ("global_env_loaded") == NULL) {

		puts ("*** global environment has not been loaded\n");
		puts ("*** fetching from server (Control-C to Abort)\n");

		rc = fetch_and_parse (global_env_path, 0x100000,
							  env_fetch_callback);

		if (rc == 0)
			puts ("*** Fetch of environment failed!\n");
		else
			setenv ("global_env_loaded", "yes");
	}
	return (0);
}
