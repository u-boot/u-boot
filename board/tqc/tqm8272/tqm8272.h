/*
 * (C) Copyright 2008
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

#ifndef	_TQM8272_HEADER_H
#define	_TQM8272_HEADER_H

#define _NOT_USED_	0xFFFFFFFF

typedef struct{
	int	Bus;
	int	flash;
	int	flash_nr;
	int	ram;
	int	ram_cs;
	int	nand;
	int	nand_cs;
	int	eeprom;
	int	can;
	unsigned long	cpunr;
	unsigned long	option;
	int	SecEng;
	int	cpucl;
	int	cpmcl;
	int	buscl;
	int	busclk_real_ok;
	int	busclk_real;
	unsigned char	OK;
	unsigned char  ethaddr[20];
} HWIB_INFO;

static HWIB_INFO	hwinf = {0, 0, 1, 0, 1, 0, 0, 0, 0, 8272, 0 ,0,
			 0, 0, 0, 0, 0, 0};
#endif
