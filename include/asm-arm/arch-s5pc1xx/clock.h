/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
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

#ifndef __ASM_ARM_ARCH_CLOCK_H_
#define __ASM_ARM_ARCH_CLOCK_H_

#ifndef __ASSEMBLY__
struct s5pc100_clock {
	unsigned long	apll_lock;
	unsigned long	mpll_lock;
	unsigned long	epll_lock;
	unsigned long	hpll_lock;
	unsigned char	res1[0xf0];
	unsigned long	apll_con;
	unsigned long	mpll_con;
	unsigned long	epll_con;
	unsigned long	hpll_con;
	unsigned char	res2[0xf0];
	unsigned long	src0;
	unsigned long	src1;
	unsigned long	src2;
	unsigned long	src3;
	unsigned char	res3[0xf0];
	unsigned long	div0;
	unsigned long	div1;
	unsigned long	div2;
	unsigned long	div3;
	unsigned long	div4;
	unsigned char	res4[0x1ec];
	unsigned long	gate_d00;
	unsigned long	gate_d01;
	unsigned long	gate_d02;
	unsigned char	res5[0x54];
	unsigned long	gate_sclk0;
	unsigned long	gate_sclk1;
};

struct s5pc110_clock {
	unsigned long	apll_lock;
	unsigned char	res1[0x4];
	unsigned long	mpll_lock;
	unsigned char	res2[0x4];
	unsigned long	epll_lock;
	unsigned char	res3[0xc];
	unsigned long	vpll_lock;
	unsigned char	res4[0xdc];
	unsigned long	apll_con;
	unsigned char	res5[0x4];
	unsigned long	mpll_con;
	unsigned char	res6[0x4];
	unsigned long	epll_con;
	unsigned char	res7[0xc];
	unsigned long	vpll_con;
	unsigned char	res8[0xdc];
	unsigned long	src0;
	unsigned long	src1;
	unsigned long	src2;
	unsigned long	src3;
	unsigned char	res9[0xf0];
	unsigned long	div0;
	unsigned long	div1;
	unsigned long	div2;
	unsigned long	div3;
	unsigned long	div4;
	unsigned char	res10[0x1ec];
	unsigned long	gate_d00;
	unsigned long	gate_d01;
	unsigned long	gate_d02;
	unsigned char	res11[0x54];
	unsigned long	gate_sclk0;
	unsigned long	gate_sclk1;
};
#endif

#endif
