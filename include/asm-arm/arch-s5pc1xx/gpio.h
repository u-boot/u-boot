/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
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

#ifndef __ASM_ARCH_GPIO_H
#define __ASM_ARCH_GPIO_H

#ifndef __ASSEMBLY__
struct s5pc1xx_gpio_bank {
	unsigned int	con;
	unsigned int	dat;
	unsigned int	pull;
	unsigned int	drv;
	unsigned int	pdn_con;
	unsigned int	pdn_pull;
	unsigned char	res1[8];
};

struct s5pc100_gpio {
	struct s5pc1xx_gpio_bank gpio_a0;
	struct s5pc1xx_gpio_bank gpio_a1;
	struct s5pc1xx_gpio_bank gpio_b;
	struct s5pc1xx_gpio_bank gpio_c;
	struct s5pc1xx_gpio_bank gpio_d;
	struct s5pc1xx_gpio_bank gpio_e0;
	struct s5pc1xx_gpio_bank gpio_e1;
	struct s5pc1xx_gpio_bank gpio_f0;
	struct s5pc1xx_gpio_bank gpio_f1;
	struct s5pc1xx_gpio_bank gpio_f2;
	struct s5pc1xx_gpio_bank gpio_f3;
	struct s5pc1xx_gpio_bank gpio_g0;
	struct s5pc1xx_gpio_bank gpio_g1;
	struct s5pc1xx_gpio_bank gpio_g2;
	struct s5pc1xx_gpio_bank gpio_g3;
	struct s5pc1xx_gpio_bank gpio_i;
	struct s5pc1xx_gpio_bank gpio_j0;
	struct s5pc1xx_gpio_bank gpio_j1;
	struct s5pc1xx_gpio_bank gpio_j2;
	struct s5pc1xx_gpio_bank gpio_j3;
	struct s5pc1xx_gpio_bank gpio_j4;
	struct s5pc1xx_gpio_bank gpio_k0;
	struct s5pc1xx_gpio_bank gpio_k1;
	struct s5pc1xx_gpio_bank gpio_k2;
	struct s5pc1xx_gpio_bank gpio_k3;
	struct s5pc1xx_gpio_bank gpio_l0;
	struct s5pc1xx_gpio_bank gpio_l1;
	struct s5pc1xx_gpio_bank gpio_l2;
	struct s5pc1xx_gpio_bank gpio_l3;
	struct s5pc1xx_gpio_bank gpio_l4;
	struct s5pc1xx_gpio_bank gpio_h0;
	struct s5pc1xx_gpio_bank gpio_h1;
	struct s5pc1xx_gpio_bank gpio_h2;
	struct s5pc1xx_gpio_bank gpio_h3;
};

struct s5pc110_gpio {
	struct s5pc1xx_gpio_bank gpio_a0;
	struct s5pc1xx_gpio_bank gpio_a1;
	struct s5pc1xx_gpio_bank gpio_b;
	struct s5pc1xx_gpio_bank gpio_c0;
	struct s5pc1xx_gpio_bank gpio_c1;
	struct s5pc1xx_gpio_bank gpio_d0;
	struct s5pc1xx_gpio_bank gpio_d1;
	struct s5pc1xx_gpio_bank gpio_e0;
	struct s5pc1xx_gpio_bank gpio_e1;
	struct s5pc1xx_gpio_bank gpio_f0;
	struct s5pc1xx_gpio_bank gpio_f1;
	struct s5pc1xx_gpio_bank gpio_f2;
	struct s5pc1xx_gpio_bank gpio_f3;
	struct s5pc1xx_gpio_bank gpio_g0;
	struct s5pc1xx_gpio_bank gpio_g1;
	struct s5pc1xx_gpio_bank gpio_g2;
	struct s5pc1xx_gpio_bank gpio_g3;
	struct s5pc1xx_gpio_bank gpio_i;
	struct s5pc1xx_gpio_bank gpio_j0;
	struct s5pc1xx_gpio_bank gpio_j1;
	struct s5pc1xx_gpio_bank gpio_j2;
	struct s5pc1xx_gpio_bank gpio_j3;
	struct s5pc1xx_gpio_bank gpio_j4;
	struct s5pc1xx_gpio_bank gpio_mp0_1;
	struct s5pc1xx_gpio_bank gpio_mp0_2;
	struct s5pc1xx_gpio_bank gpio_mp0_3;
	struct s5pc1xx_gpio_bank gpio_mp0_4;
	struct s5pc1xx_gpio_bank gpio_mp0_5;
	struct s5pc1xx_gpio_bank gpio_mp0_6;
	struct s5pc1xx_gpio_bank gpio_mp0_7;
	struct s5pc1xx_gpio_bank gpio_mp1_0;
	struct s5pc1xx_gpio_bank gpio_mp1_1;
	struct s5pc1xx_gpio_bank gpio_mp1_2;
	struct s5pc1xx_gpio_bank gpio_mp1_3;
	struct s5pc1xx_gpio_bank gpio_mp1_4;
	struct s5pc1xx_gpio_bank gpio_mp1_5;
	struct s5pc1xx_gpio_bank gpio_mp1_6;
	struct s5pc1xx_gpio_bank gpio_mp1_7;
	struct s5pc1xx_gpio_bank gpio_mp1_8;
	struct s5pc1xx_gpio_bank gpio_mp2_0;
	struct s5pc1xx_gpio_bank gpio_mp2_1;
	struct s5pc1xx_gpio_bank gpio_mp2_2;
	struct s5pc1xx_gpio_bank gpio_mp2_3;
	struct s5pc1xx_gpio_bank gpio_mp2_4;
	struct s5pc1xx_gpio_bank gpio_mp2_5;
	struct s5pc1xx_gpio_bank gpio_mp2_6;
	struct s5pc1xx_gpio_bank gpio_mp2_7;
	struct s5pc1xx_gpio_bank gpio_mp2_8;
	struct s5pc1xx_gpio_bank res1[48];
	struct s5pc1xx_gpio_bank gpio_h0;
	struct s5pc1xx_gpio_bank gpio_h1;
	struct s5pc1xx_gpio_bank gpio_h2;
	struct s5pc1xx_gpio_bank gpio_h3;
};
#endif

#endif
