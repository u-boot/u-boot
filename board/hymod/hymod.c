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
 * Hacked for the Hymod board by Murray.Jensen@csiro.au, 20-Oct-00
 */

#include <common.h>
#include <mpc8260.h>
#include <mpc8260_irq.h>
#include <ioports.h>
#include <i2c.h>
#include <asm/iopin_8260.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */

/* imports from eeprom.c */
extern int hymod_eeprom_read (int, hymod_eeprom_t *);
extern void hymod_eeprom_print (hymod_eeprom_t *);

/* imports from env.c */
extern void hymod_check_env (void);

/* ------------------------------------------------------------------------- */

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

	/* Port A configuration */
	{
		/* cnf par sor dir odr dat */
		{   1,  1,  1,  0,  0,  0   },	/* PA31: FCC1 MII COL */
		{   1,  1,  1,  0,  0,  0   },	/* PA30: FCC1 MII CRS */
		{   1,  1,  1,  1,  0,  0   },	/* PA29: FCC1 MII TX_ER */
		{   1,  1,  1,  1,  0,  0   },	/* PA28: FCC1 MII TX_EN */
		{   1,  1,  1,  0,  0,  0   },	/* PA27: FCC1 MII RX_DV */
		{   1,  1,  1,  0,  0,  0   },	/* PA26: FCC1 MII RX_ER */
		{   1,  0,  0,  1,  0,  0   },	/* PA25: FCC2 MII MDIO */
		{   1,  0,  0,  1,  0,  0   },	/* PA24: FCC2 MII MDC */
		{   1,  0,  0,  1,  0,  0   },	/* PA23: FCC3 MII MDIO */
		{   1,  0,  0,  1,  0,  0   },	/* PA22: FCC3 MII MDC */
		{   1,  1,  0,  1,  0,  0   },	/* PA21: FCC1 MII TxD[3] */
		{   1,  1,  0,  1,  0,  0   },	/* PA20: FCC1 MII TxD[2] */
		{   1,  1,  0,  1,  0,  0   },	/* PA19: FCC1 MII TxD[1] */
		{   1,  1,  0,  1,  0,  0   },	/* PA18: FCC1 MII TxD[0] */
		{   1,  1,  0,  0,  0,  0   },	/* PA17: FCC1 MII RxD[3] */
		{   1,  1,  0,  0,  0,  0   },	/* PA16: FCC1 MII RxD[2] */
		{   1,  1,  0,  0,  0,  0   },	/* PA15: FCC1 MII RxD[1] */
		{   1,  1,  0,  0,  0,  0   },	/* PA14: FCC1 MII RxD[0] */
		{   1,  0,  0,  1,  0,  0   },	/* PA13: FCC1 MII MDIO */
		{   1,  0,  0,  1,  0,  0   },	/* PA12: FCC1 MII MDC */
		{   1,  0,  0,  1,  0,  0   },	/* PA11: SEL_CD */
		{   1,  0,  0,  0,  0,  0   },	/* PA10: FLASH STS1 */
		{   1,  0,  0,  0,  0,  0   },	/* PA09: FLASH STS0 */
		{   1,  0,  0,  0,  0,  0   },	/* PA08: FLASH ~PE */
		{   1,  0,  0,  0,  0,  0   },	/* PA07: WATCH ~HRESET */
		{   1,  0,  0,  0,  1,  0   },	/* PA06: VC DONE */
		{   1,  0,  0,  1,  1,  0   },	/* PA05: VC INIT */
		{   1,  0,  0,  1,  0,  0   },	/* PA04: VC ~PROG */
		{   1,  0,  0,  1,  0,  0   },	/* PA03: VM ENABLE */
		{   1,  0,  0,  0,  1,  0   },	/* PA02: VM DONE */
		{   1,  0,  0,  1,  1,  0   },	/* PA01: VM INIT */
		{   1,  0,  0,  1,  0,  0   }	/* PA00: VM ~PROG */
	},

	/* Port B configuration */
	{
		/* cnf par sor dir odr dat */
		{   1,  1,  0,  1,  0,  0   },	/* PB31: FCC2 MII TX_ER */
		{   1,  1,  0,  0,  0,  0   },	/* PB30: FCC2 MII RX_DV */
		{   1,  1,  1,  1,  0,  0   },	/* PB29: FCC2 MII TX_EN */
		{   1,  1,  0,  0,  0,  0   },	/* PB28: FCC2 MII RX_ER */
		{   1,  1,  0,  0,  0,  0   },	/* PB27: FCC2 MII COL */
		{   1,  1,  0,  0,  0,  0   },	/* PB26: FCC2 MII CRS */
		{   1,  1,  0,  1,  0,  0   },	/* PB25: FCC2 MII TxD[3] */
		{   1,  1,  0,  1,  0,  0   },	/* PB24: FCC2 MII TxD[2] */
		{   1,  1,  0,  1,  0,  0   },	/* PB23: FCC2 MII TxD[1] */
		{   1,  1,  0,  1,  0,  0   },	/* PB22: FCC2 MII TxD[0] */
		{   1,  1,  0,  0,  0,  0   },	/* PB21: FCC2 MII RxD[0] */
		{   1,  1,  0,  0,  0,  0   },	/* PB20: FCC2 MII RxD[1] */
		{   1,  1,  0,  0,  0,  0   },	/* PB19: FCC2 MII RxD[2] */
		{   1,  1,  0,  0,  0,  0   },	/* PB18: FCC2 MII RxD[3] */
		{   1,  1,  0,  0,  0,  0   },	/* PB17: FCC3 MII RX_DV */
		{   1,  1,  0,  0,  0,  0   },	/* PB16: FCC3 MII RX_ER */
		{   1,  1,  0,  1,  0,  0   },	/* PB15: FCC3 MII TX_ER */
		{   1,  1,  0,  1,  0,  0   },	/* PB14: FCC3 MII TX_EN */
		{   1,  1,  0,  0,  0,  0   },	/* PB13: FCC3 MII COL */
		{   1,  1,  0,  0,  0,  0   },	/* PB12: FCC3 MII CRS */
		{   1,  1,  0,  0,  0,  0   },	/* PB11: FCC3 MII RxD[3] */
		{   1,  1,  0,  0,  0,  0   },	/* PB10: FCC3 MII RxD[2] */
		{   1,  1,  0,  0,  0,  0   },	/* PB09: FCC3 MII RxD[1] */
		{   1,  1,  0,  0,  0,  0   },	/* PB08: FCC3 MII RxD[0] */
		{   1,  1,  0,  1,  0,  0   },	/* PB07: FCC3 MII TxD[3] */
		{   1,  1,  0,  1,  0,  0   },	/* PB06: FCC3 MII TxD[2] */
		{   1,  1,  0,  1,  0,  0   },	/* PB05: FCC3 MII TxD[1] */
		{   1,  1,  0,  1,  0,  0   },	/* PB04: FCC3 MII TxD[0] */
		{   0,  0,  0,  0,  0,  0   },	/* PB03: pin doesn't exist */
		{   0,  0,  0,  0,  0,  0   },	/* PB02: pin doesn't exist */
		{   0,  0,  0,  0,  0,  0   },	/* PB01: pin doesn't exist */
		{   0,  0,  0,  0,  0,  0   }	/* PB00: pin doesn't exist */
	},

	/* Port C configuration */
	{
		/* cnf par sor dir odr dat */
		{   1,  0,  0,  0,  0,  0   },	/* PC31: MEZ ~IACK */
		{   0,  0,  0,  0,  0,  0   },	/* PC30: ? */
		{   1,  1,  0,  0,  0,  0   },	/* PC29: CLK SCCx */
		{   1,  1,  0,  0,  0,  0   },	/* PC28: CLK4 */
		{   1,  1,  0,  0,  0,  0   },	/* PC27: CLK SCCF */
		{   1,  1,  0,  0,  0,  0   },	/* PC26: CLK 32K */
		{   1,  1,  0,  0,  0,  0   },	/* PC25: BRG4/CLK7 */
		{   0,  0,  0,  0,  0,  0   },	/* PC24: ? */
		{   1,  1,  0,  0,  0,  0   },	/* PC23: CLK SCCx */
		{   1,  1,  0,  0,  0,  0   },	/* PC22: FCC1 MII RX_CLK */
		{   1,  1,  0,  0,  0,  0   },	/* PC21: FCC1 MII TX_CLK */
		{   1,  1,  0,  0,  0,  0   },	/* PC20: CLK SCCF */
		{   1,  1,  0,  0,  0,  0   },	/* PC19: FCC2 MII RX_CLK */
		{   1,  1,  0,  0,  0,  0   },	/* PC18: FCC2 MII TX_CLK */
		{   1,  1,  0,  0,  0,  0   },	/* PC17: FCC3 MII RX_CLK */
		{   1,  1,  0,  0,  0,  0   },	/* PC16: FCC3 MII TX_CLK */
		{   1,  0,  0,  0,  0,  0   },	/* PC15: SCC1 UART ~CTS */
		{   1,  0,  0,  0,  0,  0   },	/* PC14: SCC1 UART ~CD */
		{   1,  0,  0,  0,  0,  0   },	/* PC13: SCC2 UART ~CTS */
		{   1,  0,  0,  0,  0,  0   },	/* PC12: SCC2 UART ~CD */
		{   1,  0,  0,  1,  0,  0   },	/* PC11: SCC1 UART ~DTR */
		{   1,  0,  0,  1,  0,  0   },	/* PC10: SCC1 UART ~DSR */
		{   1,  0,  0,  1,  0,  0   },	/* PC09: SCC2 UART ~DTR */
		{   1,  0,  0,  1,  0,  0   },	/* PC08: SCC2 UART ~DSR */
		{   1,  0,  0,  0,  0,  0   },	/* PC07: TEMP ~ALERT */
		{   1,  0,  0,  0,  0,  0   },	/* PC06: FCC3 INT */
		{   1,  0,  0,  0,  0,  0   },	/* PC05: FCC2 INT */
		{   1,  0,  0,  0,  0,  0   },	/* PC04: FCC1 INT */
		{   0,  1,  1,  1,  0,  0   },	/* PC03: SDMA IDMA2 ~DACK */
		{   0,  1,  1,  0,  0,  0   },	/* PC02: SDMA IDMA2 ~DONE */
		{   0,  1,  0,  0,  0,  0   },	/* PC01: SDMA IDMA2 ~DREQ */
		{   1,  1,  0,  1,  0,  0   }	/* PC00: BRG7 */
	},

	/* Port D configuration */
	{
		/* cnf par sor dir odr dat */
		{   1,  1,  0,  0,  0,  0   },	/* PD31: SCC1 UART RxD */
		{   1,  1,  1,  1,  0,  0   },	/* PD30: SCC1 UART TxD */
		{   1,  0,  0,  1,  0,  0   },	/* PD29: SCC1 UART ~RTS */
		{   1,  1,  0,  0,  0,  0   },	/* PD28: SCC2 UART RxD */
		{   1,  1,  0,  1,  0,  0   },	/* PD27: SCC2 UART TxD */
		{   1,  0,  0,  1,  0,  0   },	/* PD26: SCC2 UART ~RTS */
		{   1,  0,  0,  0,  0,  0   },	/* PD25: SCC1 UART ~RI */
		{   1,  0,  0,  0,  0,  0   },	/* PD24: SCC2 UART ~RI */
		{   1,  0,  0,  1,  0,  0   },	/* PD23: CLKGEN PD */
		{   1,  0,  0,  0,  0,  0   },	/* PD22: USER3 */
		{   1,  0,  0,  0,  0,  0   },	/* PD21: USER2 */
		{   1,  0,  0,  0,  0,  0   },	/* PD20: USER1 */
		{   1,  1,  1,  0,  0,  0   },	/* PD19: SPI ~SEL */
		{   1,  1,  1,  0,  0,  0   },	/* PD18: SPI CLK */
		{   1,  1,  1,  0,  0,  0   },	/* PD17: SPI MOSI */
		{   1,  1,  1,  0,  0,  0   },	/* PD16: SPI MISO */
		{   1,  1,  1,  0,  1,  0   },	/* PD15: I2C SDA */
		{   1,  1,  1,  0,  1,  0   },	/* PD14: I2C SCL */
		{   1,  0,  0,  1,  0,  1   },	/* PD13: TEMP ~STDBY */
		{   1,  0,  0,  1,  0,  1   },	/* PD12: FCC3 ~RESET */
		{   1,  0,  0,  1,  0,  1   },	/* PD11: FCC2 ~RESET */
		{   1,  0,  0,  1,  0,  1   },	/* PD10: FCC1 ~RESET */
		{   1,  0,  0,  0,  0,  0   },	/* PD09: PD9 */
		{   1,  0,  0,  0,  0,  0   },	/* PD08: PD8 */
		{   1,  0,  0,  1,  0,  1   },	/* PD07: PD7 */
		{   1,  0,  0,  1,  0,  1   },	/* PD06: PD6 */
		{   1,  0,  0,  1,  0,  1   },	/* PD05: PD5 */
		{   1,  0,  0,  1,  0,  1   },	/* PD04: PD4 */
		{   0,  0,  0,  0,  0,  0   },	/* PD03: pin doesn't exist */
		{   0,  0,  0,  0,  0,  0   },	/* PD02: pin doesn't exist */
		{   0,  0,  0,  0,  0,  0   },	/* PD01: pin doesn't exist */
		{   0,  0,  0,  0,  0,  0   }	/* PD00: pin doesn't exist */
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
 *
 * The fs6377 has four clock outputs: A, B, C and D.
 *
 * Outputs C and D can each provide two different clock outputs C1/D1 or
 * C2/D2 depending on the state of the SEL_CD input which is connected to
 * the MPC8260 I/O port pin PA11. PA11 output (SEL_CD input) low (or 0)
 * selects C1/D1 and PA11 output (SEL_CD input) high (or 1) selects C2/D2.
 *
 * PA11 defaults to output low (or 0) in the i/o port config table above.
 *
 * Output A provides a 100MHz for the High Speed Serial chips. Output B
 * provides a 3.6864MHz clock for more accurate asynchronous serial bit
 * rates. Output C is routed to the mezzanine connector but is currently
 * unused - both C1 and C2 are set to 16MHz. Output D is used by both the
 * alt-input and display mezzanine boards for their video chips. The
 * alt-input board requires a clock of 24.576MHz and this is available on
 * D1 (PA11=SEL_CD=0). The display board requires a clock of 27MHz and this
 * is available on D2 (PA11=SEL_CD=1).
 *
 * So the default is a clock suitable for the alt-input board. PA11 is toggled
 * later in misc_init_r(), if a display board is detected.
 */

uchar fs6377_addr = 0x5c;

uchar fs6377_regs[16] = {
	 12,  75,  64,  25, 144, 128,  25, 192,
	  0,  16, 135, 192, 224,  64,  64, 192
};

/* ------------------------------------------------------------------------- */

/*
 * special board initialisation, after clocks and timebase have been
 * set up but before environment and serial are initialised.
 *
 * added so that very early initialisations can be done using the i2c
 * driver (which requires the clocks, to calculate the dividers, and
 * the timebase, for udelay())
 */

int
board_postclk_init (void)
{
	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

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

int
checkboard (void)
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

int
misc_init_f (void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;

	printf ("UPMs:  ");

	upmconfig (UPMB, upmb_table, sizeof upmb_table / sizeof upmb_table[0]);
	memctl->memc_mbmr = CONFIG_SYS_MBMR;

	upmconfig (UPMC, upmc_table, sizeof upmc_table / sizeof upmc_table[0]);
	memctl->memc_mcmr = CONFIG_SYS_MCMR;

	printf ("configured\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

phys_size_t
initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	volatile uchar c = 0, *ramaddr = (uchar *) (CONFIG_SYS_SDRAM_BASE + 0x8);
	ulong psdmr = CONFIG_SYS_PSDMR;
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
	 * get here. The SDRAM can be accessed at the address CONFIG_SYS_SDRAM_BASE.
	 */

	memctl->memc_psrt = CONFIG_SYS_PSRT;
	memctl->memc_mptpr = CONFIG_SYS_MPTPR;

	memctl->memc_psdmr = psdmr | PSDMR_OP_PREA;
	*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++)
		*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_MRW;
	*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	*ramaddr = c;

	return (CONFIG_SYS_SDRAM_SIZE << 20);
}

/* ------------------------------------------------------------------------- */
/* miscellaneous initialisations after relocation into ram (misc_init_r)     */
/*									     */
/* loads the data in the main board and mezzanine board eeproms into	     */
/* the hymod configuration struct stored in the board information area.	     */
/*									     */
/* if the contents of either eeprom is invalid, prompts for a serial	     */
/* number (and an ethernet address if required) then fetches a file	     */
/* containing information to be stored in the eeprom from the tftp server    */
/* (the file name is based on the serial number and a built-in path)	     */

int
last_stage_init (void)
{
	hymod_conf_t *cp = &gd->bd->bi_hymod_conf;
	int rc;

#ifdef CONFIG_BOOT_RETRY_TIME
	/*
	 * we use the readline () function, but we also want
	 * command timeout enabled
	 */
	init_cmd_timeout ();
#endif

	memset ((void *) cp, 0, sizeof (*cp));

	/* set up main board config info */

	rc = hymod_eeprom_read (0, &cp->main.eeprom);

	puts ("EEPROM:main...");
	if (rc < 0)
		puts ("NOT PRESENT\n");
	else if (rc == 0)
		puts ("INVALID\n");
	else {
		cp->main.eeprom.valid = 1;

		printf ("OK (ver %u)\n", cp->main.eeprom.ver);
		hymod_eeprom_print (&cp->main.eeprom);

		/*
		 * hard-wired assumption here: all hymod main boards will have
		 * one xilinx fpga, with the interrupt line connected to IRQ2
		 *
		 * One day, this might be based on the board type
		 */

		cp->main.xlx[0].mmap.prog.exists = 1;
		cp->main.xlx[0].mmap.prog.size = FPGA_MAIN_CFG_SIZE;
		cp->main.xlx[0].mmap.prog.base = FPGA_MAIN_CFG_BASE;

		cp->main.xlx[0].mmap.reg.exists = 1;
		cp->main.xlx[0].mmap.reg.size = FPGA_MAIN_REG_SIZE;
		cp->main.xlx[0].mmap.reg.base = FPGA_MAIN_REG_BASE;

		cp->main.xlx[0].mmap.port.exists = 1;
		cp->main.xlx[0].mmap.port.size = FPGA_MAIN_PORT_SIZE;
		cp->main.xlx[0].mmap.port.base = FPGA_MAIN_PORT_BASE;

		cp->main.xlx[0].iopins.prog_pin.port = FPGA_MAIN_PROG_PORT;
		cp->main.xlx[0].iopins.prog_pin.pin = FPGA_MAIN_PROG_PIN;
		cp->main.xlx[0].iopins.prog_pin.flag = 1;
		cp->main.xlx[0].iopins.init_pin.port = FPGA_MAIN_INIT_PORT;
		cp->main.xlx[0].iopins.init_pin.pin = FPGA_MAIN_INIT_PIN;
		cp->main.xlx[0].iopins.init_pin.flag = 1;
		cp->main.xlx[0].iopins.done_pin.port = FPGA_MAIN_DONE_PORT;
		cp->main.xlx[0].iopins.done_pin.pin = FPGA_MAIN_DONE_PIN;
		cp->main.xlx[0].iopins.done_pin.flag = 1;
#ifdef FPGA_MAIN_ENABLE_PORT
		cp->main.xlx[0].iopins.enable_pin.port = FPGA_MAIN_ENABLE_PORT;
		cp->main.xlx[0].iopins.enable_pin.pin = FPGA_MAIN_ENABLE_PIN;
		cp->main.xlx[0].iopins.enable_pin.flag = 1;
#endif

		cp->main.xlx[0].irq = FPGA_MAIN_IRQ;
	}

	/* set up mezzanine board config info */

	rc = hymod_eeprom_read (1, &cp->mezz.eeprom);

	puts ("EEPROM:mezz...");
	if (rc < 0)
		puts ("NOT PRESENT\n");
	else if (rc == 0)
		puts ("INVALID\n");
	else {
		cp->main.eeprom.valid = 1;

		printf ("OK (ver %u)\n", cp->mezz.eeprom.ver);
		hymod_eeprom_print (&cp->mezz.eeprom);
	}

	cp->crc = crc32 (0, (unsigned char *)cp, offsetof (hymod_conf_t, crc));

	hymod_check_env ();

	return (0);
}

#ifdef CONFIG_SHOW_ACTIVITY
void board_show_activity (ulong timebase)
{
#ifdef CONFIG_SYS_HYMOD_DBLEDS
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile iop8260_t *iop = &immr->im_ioport;
	static int shift = 0;

	if ((timestamp % CONFIG_SYS_HZ) == 0) {
		if (++shift > 3)
			shift = 0;
		iop->iop_pdatd =
				(iop->iop_pdatd & ~0x0f000000) | (1 << (24 + shift));
	}
#endif /* CONFIG_SYS_HYMOD_DBLEDS */
}

void show_activity(int arg)
{
}
#endif /* CONFIG_SHOW_ACTIVITY */
