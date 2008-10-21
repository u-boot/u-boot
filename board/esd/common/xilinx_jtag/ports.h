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
/* file: ports.h                                       */
/* abstract:  This file contains extern declarations   */
/*            for providing stimulus to the JTAG ports.*/
/*******************************************************/

#ifndef ports_dot_h
#define ports_dot_h

/* these constants are used to send the appropriate ports to setPort */
/* they should be enumerated types, but some of the microcontroller  */
/* compilers don't like enumerated types */
#define TCK (short) 0
#define TMS (short) 1
#define TDI (short) 2

/*
 * Use CONFIG_SYS_FPGA_xxx defines from board include file.
 */
#define JTAG_TMS   CONFIG_SYS_FPGA_PRG     /* output */
#define JTAG_TCK   CONFIG_SYS_FPGA_CLK     /* output */
#define JTAG_TDI   CONFIG_SYS_FPGA_DATA    /* output */
#define JTAG_TDO   CONFIG_SYS_FPGA_DONE    /* input */

/* set the port "p" (TCK, TMS, or TDI) to val (0 or 1) */
void setPort(short p, short val);

/* read the TDO bit and store it in val */
unsigned char readTDOBit(void);

/* make clock go down->up->down*/
void pulseClock(void);

/* read the next byte of data from the xsvf file */
void readByte(unsigned char *data);

void waitTime(long microsec);

#endif
