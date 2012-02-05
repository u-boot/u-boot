/*
 * (C) Copyright 2012 Samsung Electronics
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
 *
 */

#ifndef __TZPC_H_
#define __TZPC_H_

#ifndef __ASSEMBLY__
struct exynos5_tzpc {
	unsigned int r0size;
	char res1[0x7FC];
	unsigned int decprot0stat;
	unsigned int decprot0set;
	unsigned int decprot0clr;
	unsigned int decprot1stat;
	unsigned int decprot1set;
	unsigned int decprot1clr;
	unsigned int decprot2stat;
	unsigned int decprot2set;
	unsigned int decprot2clr;
	unsigned int decprot3stat;
	unsigned int decprot3set;
	unsigned int decprot3clr;
	char res2[0x7B0];
	unsigned int periphid0;
	unsigned int periphid1;
	unsigned int periphid2;
	unsigned int periphid3;
	unsigned int pcellid0;
	unsigned int pcellid1;
	unsigned int pcellid2;
	unsigned int pcellid3;
};
#endif

#endif
