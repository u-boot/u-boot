/*
 * Copyright 2007 Freescale Semiconductor.
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

#ifndef __BCSR_H_
#define __BCSR_H_

#include <common.h>

/* BCSR Bit definitions
	* BCSR 0 *
	0:3	ccb sys pll
	4:6	cfg core pll
	7	cfg boot seq

	* BCSR 1 *
	0:2 	cfg rom lock
	3:5 	cfg host agent
	6	PCI IO
	7	cfg RIO size

	* BCSR 2 *
	0:4	QE PLL
	5	QE clock
	6	cfg PCI arbiter

	* BCSR 3 *
	0	TSEC1 reduce
	1	TSEC2 reduce
	2:3	TSEC1 protocol
	4:5 	TSEC2 protocol
	6	PHY1 slave
	7	PHY2 slave

	* BCSR 4 *
	4	clock enable
	5	boot EPROM
	6	GETH transactive reset
	7	BRD write potect

	* BCSR 5 *
	1:3	Leds 1-3
	4	UPC1 enable
	5	UPC2 enable
	6	UPC2 pos
	7	RS232 enable

	* BCSR 6 *
	0	CFG ver 0
	1	CFG ver 1
	6	Register config led
	7	Power on reset

	* BCSR 7 *
	2 	board host mode indication
	5 	enable TSEC1 PHY
	6 	enable TSEC2 PHY

	* BCSR 8 *
	0	UCC GETH1 enable
	1	UCC GMII enable
	3	UCC TBI enable
	5	UCC MII enable
	7	Real time clock reset

	* BCSR 9 *
	0	UCC2 GETH enable
	1	UCC2 GMII enable
	3	UCC2 TBI enable
	5	UCC2 MII enable
	6	Ready only - indicate flash ready after burning
	7	Flash write protect
*/

/*BCSR Utils functions*/

void enable_8568mds_duart(void);
void enable_8568mds_flash_write(void);
void disable_8568mds_flash_write(void);

#endif	/* __BCSR_H_ */
