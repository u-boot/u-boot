/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
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

#ifndef __SPR_DEFS_H__
#define __SPR_DEFS_H__

extern int spear_board_init(ulong);
extern void setfreq(unsigned int, unsigned int);
extern unsigned int setfreq_sz;

struct chip_data {
	int cpufreq;
	int dramfreq;
	int dramtype;
	uchar version[32];
};

/* HW mac id in i2c memory definitions */
#define MAGIC_OFF	0x0
#define MAGIC_LEN	0x2
#define MAGIC_BYTE0	0x55
#define MAGIC_BYTE1	0xAA
#define MAC_OFF		0x2
#define MAC_LEN		0x6

#endif
