/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <asm/processor.h>
#include <ppc440.h>

/*************************************************************************
 *
 * initdram -- 440EPx's DDR controller is a DENALI Core
 *
 ************************************************************************/
long int initdram (int board_type)
{
#if !defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
	volatile ulong val;

	mtsdram(DDR0_02, 0x00000000);

	mtsdram(DDR0_00, 0x0000190A);
	mtsdram(DDR0_01, 0x01000000);
	mtsdram(DDR0_03, 0x02030602);
	mtsdram(DDR0_04, 0x13030300);
	mtsdram(DDR0_05, 0x0202050E);
	mtsdram(DDR0_06, 0x0104C823);
	mtsdram(DDR0_07, 0x000D0100);
	mtsdram(DDR0_08, 0x02360001);
	mtsdram(DDR0_09, 0x00011D5F);
	mtsdram(DDR0_10, 0x00000300);
	mtsdram(DDR0_11, 0x0027C800);
	mtsdram(DDR0_12, 0x00000003);
	mtsdram(DDR0_14, 0x00000000);
	mtsdram(DDR0_17, 0x19000000);
	mtsdram(DDR0_18, 0x19191919);
	mtsdram(DDR0_19, 0x19191919);
	mtsdram(DDR0_20, 0x0B0B0B0B);
	mtsdram(DDR0_21, 0x0B0B0B0B);
	mtsdram(DDR0_22, 0x00267F0B);
	mtsdram(DDR0_23, 0x00000000);
	mtsdram(DDR0_24, 0x01010002);
	mtsdram(DDR0_26, 0x5B260181);
	mtsdram(DDR0_27, 0x0000682B);
	mtsdram(DDR0_28, 0x00000000);
	mtsdram(DDR0_31, 0x00000000);
	mtsdram(DDR0_42, 0x01000006);
	mtsdram(DDR0_43, 0x050A0200);
	mtsdram(DDR0_44, 0x00000005);
	mtsdram(DDR0_02, 0x00000001);

	/*
	 * Wait for DCC master delay line to finish calibration
	 */
	mfsdram(DDR0_17, val);
	while (((val >> 8) & 0x000007f) == 0) {
		mfsdram(DDR0_17, val);
	}
#endif /* #ifndef CONFIG_NAND_U_BOOT */

	return (CFG_MBYTES_SDRAM << 20);
}
