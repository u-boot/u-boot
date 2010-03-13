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

#ifndef _SPR_XLOADER_TABLE_H
#define _SPR_XLOADER_TABLE_H

#define XLOADER_TABLE_VERSION_1_1	2
#define XLOADER_TABLE_VERSION_1_2	3

#define XLOADER_TABLE_ADDRESS		0xD2801FF0

#define DDRMOBILE	1
#define DDR2		2

#define REV_BA		1
#define REV_AA		2
#define REV_AB		3

struct xloader_table_1_1 {
	unsigned short ddrfreq;
	unsigned char ddrsize;
	unsigned char ddrtype;

	unsigned char soc_rev;
} __attribute__ ((packed));

struct xloader_table_1_2 {
	unsigned const char *version;

	unsigned short ddrfreq;
	unsigned char ddrsize;
	unsigned char ddrtype;

	unsigned char soc_rev;
} __attribute__ ((packed));

union table_contents {
	struct xloader_table_1_1 table_1_1;
	struct xloader_table_1_2 table_1_2;
};

struct xloader_table {
	unsigned char table_version;
	union table_contents table;
} __attribute__ ((packed));

#endif
