/*
 * (C) Copyright 2000-2009
 * Vipin Kumar, ST Microelectronics, vipin.kumar@st.com
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

#include <common.h>

#if (CONFIG_DDR_HCLK)

const u32 mpmc_conf_vals[CONFIG_SPEAR_MPMCREGS] = {
	0x03030301,
	0x03030303,
	0x01000000,
	0x00000101,
	0x00000001,
	0x01000000,
	0x00010001,
	0x00000100,
	0x00010001,
	0x00000003,
	0x01000201,
	0x06000202,
	0x06060106,
	0x03050502,
	0x03040404,
	0x02020503,
	0x02010106,
	0x03000404,
	0x02020202,
	0x03000203,
	0x0707073f,
	0x07070707,
	0x06060607,
	0x06060606,
	0x05050506,
	0x05050505,
	0x04040405,
	0x04040404,
	0x03030304,
	0x03030303,
	0x02020203,
	0x02020202,
	0x01010102,
	0x01010101,
	0x08080a01,
	0x0000023f,
	0x00030600,
	0x00000000,
	0x00000a02,
	0x00001c1c,
	0x7f000000,
	0x005f0000,
	0x12030743,
	0x00640064,
	0x00640064,
	0x00640064,
	0x00000064,
	0x00200020,
	0x00200020,
	0x00200020,
	0x00200020,
	0x00200020,
	0x00200020,
	0x00200020,
	0x0000050e,
	0x00000000,
	0x2d8900c8,
	0x00c80014,
	0x00000000,
	0x00008236,
	0x00000000,
	0x03030100,
	0x03030303,
	0x03030303,
	0x03030303,
	0x00400000,
	0x003a0040,
	0x00680000,
	0x00d80120,
	0x003fffff,
	0x003fffff,
	0x00000000,
	0x00000000,
	0x003fffff,
	0x003fffff,
	0x00000000,
	0x00000000,
	0x003fffff,
	0x003fffff,
	0x00000000,
	0x00000000,
	0x003fffff,
	0x003fffff,
	0x00000000,
	0x00000000,
	0x003fffff,
	0x003fffff,
	0x00000000,
	0x00000000,
	0x003fffff,
	0x003fffff,
	0x00000000,
	0x00000000,
	0x003fffff,
	0x003fffff,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000
};
#endif
