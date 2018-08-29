/***************************************************************
 * Project:
 *	  CPLD SlaveSerial Configuration via embedded microprocessor.
 *
 * Copyright info:
 *
 *	  This is free software; you can redistribute it and/or modify
 *	  it as you like.
 *
 *	  This program is distributed in the hope that it will be useful,
 *	  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Description:
 *
 *      This is the main source file that will allow a microprocessor
 *      to configure Xilinx Virtex, Virtex-E, Virtex-EM, Virtex-II,
 *      and Spartan-II devices via the SlaveSerial Configuration Mode.
 *      This code is discussed in Xilinx Application Note, XAPP502.
 *
 * History:
 *	  3-October-2001  MN/MP  - Created
 *	  20-August-2008  Renesas Solutions - Modified to SH7723
 ****************************************************************/

#include <common.h>

/* Serial */
#define SCIF_BASE 0xffe00000 /* SCIF0 */
#define SCSMR	(vu_short *)(SCIF_BASE + 0x00)
#define SCBRR	(vu_char *)(SCIF_BASE + 0x04)
#define SCSCR	(vu_short *)(SCIF_BASE + 0x08)
#define SC_TDR	(vu_char *)(SCIF_BASE + 0x0C)
#define SC_SR	(vu_short *)(SCIF_BASE + 0x10)
#define SCFCR	(vu_short *)(SCIF_BASE + 0x18)
#define	RFCR	(vu_long *)0xFE400020

#define SCSCR_INIT		0x0038
#define SCSCR_CLR		0x0000
#define SCFCR_INIT		0x0006
#define SCSMR_INIT		0x0080
#define RFCR_CLR		0xA400
#define SCI_TD_E		0x0020
#define SCI_TDRE_CLEAR	0x00df

#define BPS_SETTING_VALUE	1 /* 12.5MHz */
#define WAIT_RFCR_COUNTER	500

/* CPLD data size */
#define CPLD_DATA_SIZE	169216

/* out */
#define CPLD_PFC_ADR	((vu_short *)0xA4050112)

#define CPLD_PROG_ADR	((vu_char *)0xA4050132)
#define CPLD_PROG_DAT	0x80

/* in */
#define CPLD_INIT_ADR	((vu_char *)0xA4050132)
#define CPLD_INIT_DAT	0x40
#define CPLD_DONE_ADR	((vu_char *)0xA4050132)
#define CPLD_DONE_DAT	0x20

/* data */
#define CPLD_NOMAL_START	0xA0A80000
#define CPLD_SAFE_START		0xA0AC0000
#define MODE_SW				(vu_char *)0xA405012A

static void init_cpld_loader(void)
{

	*SCSCR = SCSCR_CLR;
	*SCFCR = SCFCR_INIT;
	*SCSMR = SCSMR_INIT;

	*SCBRR = BPS_SETTING_VALUE;

	*RFCR = RFCR_CLR; /* Refresh counter clear */

	while (*RFCR < WAIT_RFCR_COUNTER)
		;

	*SCFCR = 0x0; /* RTRG=00, TTRG=00 */
				  /* MCE=0,TFRST=0,RFRST=0,LOOP=0 */
	*SCSCR = SCSCR_INIT;
}

static int check_write_ready(void)
{
	u16 status = *SC_SR;
	return status & SCI_TD_E;
}

static void write_cpld_data(char ch)
{
	while (!check_write_ready())
		;

	*SC_TDR = ch;
	*SC_SR;
	*SC_SR = SCI_TDRE_CLEAR;
}

static int delay(void)
{
	int i;
	int c = 0;
	for (i = 0; i < 200; i++) {
		c = *(volatile int *)0xa0000000;
	}
	return c;
}

/***********************************************************************
 *
 * Function:     slave_serial
 *
 * Description:  Initiates SlaveSerial Configuration.
 *               Calls ShiftDataOut() to output serial data
 *
 ***********************************************************************/
static void slave_serial(void)
{
	int i;
	unsigned char *flash;

	*CPLD_PROG_ADR |= CPLD_PROG_DAT; /* PROGRAM_OE HIGH */
	delay();

	/*
	 * Toggle Program Pin by Toggling Program_OE bit
	 * This is accomplished by writing to the Program Register in the CPLD
	 *
	 * NOTE: The Program_OE bit should be driven high to bring the Virtex
	 *      Program Pin low. Likewise, it should be driven low
	 *      to bring the Virtex Program Pin to High-Z
	 */

	*CPLD_PROG_ADR &= ~CPLD_PROG_DAT; /* PROGRAM_OE LOW */
	delay();

	/*
	 * Bring Program High-Z
	 * (Drive Program_OE bit low to bring Virtex Program Pin to High-Z
	 */

	/* Program_OE bit Low brings the Virtex Program Pin to High Z: */
	*CPLD_PROG_ADR |= CPLD_PROG_DAT; /* PROGRAM_OE HIGH */

	while ((*CPLD_INIT_ADR & CPLD_INIT_DAT) == 0)
		delay();

	/* Begin Slave-Serial Configuration */
	flash = (unsigned char *)CPLD_NOMAL_START;

	for (i = 0; i < CPLD_DATA_SIZE; i++)
		write_cpld_data(*flash++);
}

/***********************************************************************
 *
 * Function: check_done_bit
 *
 * Description: This function takes monitors the CPLD Input Register
 * 		   by checking the status of the DONE bit in that Register.
 *               By doing so, it monitors the Xilinx Virtex device's DONE
 *               Pin to see if configuration bitstream has been properly
 *               loaded
 *
 ***********************************************************************/
static void check_done_bit(void)
{
	while (!(*CPLD_DONE_ADR & CPLD_DONE_DAT))
		;
}

/***********************************************************************
 *
 * Function: init_cpld
 *
 * Description: Begins Slave Serial configuration of Xilinx FPGA
 *
 ***********************************************************************/
void init_cpld(void)
{
	/* Init serial device */
	init_cpld_loader();

	if (*CPLD_DONE_ADR & CPLD_DONE_DAT)	/* Already DONE */
		return;

	*((vu_short *)HIZCRB) = 0x0000;
	*CPLD_PFC_ADR = 0x7c00;			/* FPGA PROG = OUTPUT */

	/* write CPLD data from NOR flash to device */
	slave_serial();

	/*
	 * Monitor the DONE bit in the CPLD Input Register to see if
	 * configuration successful
	 */

	check_done_bit();
}
