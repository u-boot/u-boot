/********************************************************************
 *
 * Unless otherwise specified, Copyright (C) 2004-2005 Barco Control Rooms
 *
 * $Source: /home/services/cvs/firmware/ppc/u-boot-1.1.2/board/barco/speed.h,v $
 * $Revision: 1.2 $
 * $Author: mleeman $
 * $Date: 2005/02/21 12:48:58 $
 *
 * Last ChangeLog Entry
 * $Log: speed.h,v $
 * Revision 1.2  2005/02/21 12:48:58  mleeman
 * update of copyright years (feedback wd)
 *
 * Revision 1.1  2005/02/14 09:23:46  mleeman
 * - moved 'barcohydra' directory to a more generic barco; since we will be
 *   supporting and adding multiple boards
 *
 * Revision 1.2  2005/02/09 12:56:23  mleeman
 * add generic header to track changes in sources
 *
 *
 *******************************************************************/

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
 */

/*-----------------------------------------------------------------------
 * Timer value for timer 2, ICLK = 10
 *
 * SPEED_FCOUNT2 =  GCLK / (16 * (TIMER_TMR_PS + 1))
 * SPEED_TMR3_PS = (GCLK / (16 * SPEED_FCOUNT3)) - 1
 *
 * SPEED_FCOUNT2	timer 2 counting frequency
 * GCLK	      		CPU clock
 * SPEED_TMR2_PS	prescaler
 */
#define SPEED_TMR2_PS  	(250 - 1)	/* divide by 250	*/

/*-----------------------------------------------------------------------
 * Timer value for PIT
 *
 * PIT_TIME = SPEED_PITC / PITRTCLK
 * PITRTCLK = 8192
 */
#define SPEED_PITC	(82 << 16)	/* start counting from 82	*/

/*
 * The new value for PTA is calculated from
 *
 *	PTA = (gclk * Trefresh) / (2 ^ (2 * DFBRG) * PTP * NCS)
 *
 * gclk		CPU clock (not bus clock !)
 * Trefresh	Refresh cycle * 4 (four word bursts used)
 * DFBRG	For normal mode (no clock reduction) always 0
 * PTP		Prescaler (already adjusted for no. of banks and 4K / 8K refresh)
 * NCS		Number of SDRAM banks (chip selects) on this UPM.
 */
