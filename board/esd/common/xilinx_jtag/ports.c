/*
 * (C) Copyright 2003
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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

/*******************************************************/
/* file: ports.c                                       */
/* abstract:  This file contains the routines to       */
/*            output values on the JTAG ports, to read */
/*            the TDO bit, and to read a byte of data  */
/*            from the prom                            */
/*                                                     */
/*******************************************************/

#include <common.h>
#include <asm/processor.h>

#include "ports.h"

static unsigned long output = 0;
static int filepos = 0;
static int oldstate = 0;
static int newstate = 0;
static int readptr = 0;

extern long filesize;
extern const unsigned char fpgadata[];


/* if in debugging mode, then just set the variables */
void setPort(short p,short val)
{
	if (p==TMS) {
		if (val) {
			output |= JTAG_TMS;
		} else {
			output &= ~JTAG_TMS;
		}
	}
	if (p==TDI) {
		if (val) {
			output |= JTAG_TDI;
		} else {
			output &= ~JTAG_TDI;
		}
	}
	if (p==TCK) {
		if (val) {
			output |= JTAG_TCK;
		} else {
			output &= ~JTAG_TCK;
		}
		out32(GPIO0_OR, output);
	}
}


/* toggle tck LH */
void pulseClock(void)
{
	setPort(TCK,0);  /* set the TCK port to low  */
	setPort(TCK,1);  /* set the TCK port to high */
}


/* read in a byte of data from the prom */
void readByte(unsigned char *data)
{
	/* pretend reading using a file */
	*data = fpgadata[readptr++];
	newstate = (100 * filepos++) / filesize;
	if (newstate != oldstate) {
		printf("%4d\r\r\r\r", newstate);
		oldstate = newstate;
	}
}

/* read the TDO bit from port */
unsigned char readTDOBit(void)
{
	unsigned long inputs;

	inputs = in32(GPIO0_IR);
	if (inputs & JTAG_TDO)
		return 1;
	else
		return 0;
}


/* Wait at least the specified number of microsec.                           */
/* Use a timer if possible; otherwise estimate the number of instructions    */
/* necessary to be run based on the microcontroller speed.  For this example */
/* we pulse the TCK port a number of times based on the processor speed.     */
void waitTime(long microsec)
{
	udelay(microsec); /* esd */
}
