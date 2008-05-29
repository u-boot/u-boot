/*
 * (C) Copyright 2008
 * Martha J Marx, Silicon Turnkey Express, mmarx@silicontkx.com
 * mpc512x I/O pin/pad initialization for the ADS5121 board
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

#include <common.h>
#include <linux/types.h>
#include "iopin.h"

/*
 * IO PAD TYPES
 *	for all types   fmux is used to select the funtion
 *			ds sets the slew rate
 *	STD pins  nothing extra (can set ds & fmux only)
 *	STD_PU	  pue=1 to enable pull & pud sets whether up or down resistors
 *	STD_ST	  st sets the Schmitt trigger
 *	STD_PU_ST pue & pud sets pull-up/down resistors as in STD_PU
 *		  st sets the Schmitt trigger
 *	PCI	  hold sets output delay
 *	PCI_ST	  hold sets output delay and st sets the Schmitt trigger
 */

static struct iopin_t {
	u_short p_offset; /* offset from IOCTL_MEM_OFFSET 		*/
	u_short p_no;	  /* number of pins to set this way		*/
	u_short bit_or:7; /* Do bitwise OR instead of setting		*/
	u_short fmux:2;	  /* pad function select 0-3			*/
	u_short hold:2;   /* PCI pad types only; 			*/
	u_short pud:1; 	  /* pull resistor; PU types only;		*/
			  /* if pue=1 then 0=pull-down, 1=pull-up	*/
	u_short	pue:1;	  /* Pull resistor enable; _PU types only	*/
	u_short st:1;	  /* Schmitt trigger enable; _ST types only	*/
	u_short	ds:2;	  /* Slew rate class, 0=class1, ..., 3=class4	*/
} ioregs_init[] = {
/* FUNC1=FEC_RX_DV Sets Next 3 to FEC pads 	*/
	{IOCTL_SPDIF_TXCLK, 	3,  0, 1, 0, 0, 0, 0, 3},
/* Set highest Slew on 9 PATA pins		*/
	{IOCTL_PATA_CE1, 	9,  1, 0, 0, 0, 0, 0, 3},
/* FUNC1=FEC_COL Sets Next 15 to FEC pads 	*/
	{IOCTL_PSC0_0, 		15, 0, 1, 0, 0, 0, 0, 3},
/* FUNC1=SPDIF_TXCLK				*/
	{IOCTL_LPC_CS1, 	1,  0, 1, 0, 0, 0, 1, 3},
/* FUNC2=SPDIF_TX and sets Next pin to SPDIF_RX	*/
	{IOCTL_I2C1_SCL, 	2,  0, 2, 0, 0, 0, 1, 3},
/* FUNC2=DIU CLK				*/
	{IOCTL_PSC6_0, 		1,  0, 2, 0, 0, 0, 1, 3},
/* FUNC2=DIU_HSYNC 				*/
	{IOCTL_PSC6_1, 		1,  0, 2, 0, 0, 0, 0, 3},
/* FUNC2=DIUVSYNC Sets Next 26 to DIU Pads	*/
	{IOCTL_PSC6_4, 		26, 0, 2, 0, 0, 0, 0, 3}
};

void iopin_initialize(void)
{
	short i, j, n, p;
	u_long *reg;

	if (sizeof(ioregs_init) == 0)
		return;

	immap_t *im = (immap_t *)CFG_IMMR;
	reg = (u_long *)&(im->io_ctrl.regs[0]);
	n = sizeof(ioregs_init) / sizeof(ioregs_init[0]);

	for (i = 0; i < n; i++) {
		for (p = 0, j = ioregs_init[i].p_offset / sizeof(u_long);
			p < ioregs_init[i].p_no; p++, j++) {
		/* lowest 9 bits sets the register */
			if (ioregs_init[i].bit_or)
				reg[j] |= *((u_long *) &ioregs_init[i].p_no)
						& 0x000001ff;
			else
				reg[j] = *((u_long *) &ioregs_init[i].p_no)
						& 0x000001ff;
		}
	}
	return;
}
