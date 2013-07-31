/*
 * (C) Copyright 2008
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
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
#endif	/* __CONFIG_H */
