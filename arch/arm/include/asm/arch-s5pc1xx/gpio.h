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
struct s5p_gpio_bank {
	unsigned int	con;
	unsigned int	dat;
	unsigned int	pull;
	unsigned int	drv;
	unsigned int	pdn_con;
	unsigned int	pdn_pull;
	unsigned char	res1[8];
};

struct s5pc100_gpio {
	struct s5p_gpio_bank a0;
	struct s5p_gpio_bank a1;
	struct s5p_gpio_bank b;
	struct s5p_gpio_bank c;
	struct s5p_gpio_bank d;
	struct s5p_gpio_bank e0;
	struct s5p_gpio_bank e1;
	struct s5p_gpio_bank f0;
	struct s5p_gpio_bank f1;
	struct s5p_gpio_bank f2;
	struct s5p_gpio_bank f3;
	struct s5p_gpio_bank g0;
	struct s5p_gpio_bank g1;
	struct s5p_gpio_bank g2;
	struct s5p_gpio_bank g3;
	struct s5p_gpio_bank i;
	struct s5p_gpio_bank j0;
	struct s5p_gpio_bank j1;
	struct s5p_gpio_bank j2;
	struct s5p_gpio_bank j3;
	struct s5p_gpio_bank j4;
	struct s5p_gpio_bank k0;
	struct s5p_gpio_bank k1;
	struct s5p_gpio_bank k2;
	struct s5p_gpio_bank k3;
	struct s5p_gpio_bank l0;
	struct s5p_gpio_bank l1;
	struct s5p_gpio_bank l2;
	struct s5p_gpio_bank l3;
	struct s5p_gpio_bank l4;
	struct s5p_gpio_bank h0;
	struct s5p_gpio_bank h1;
	struct s5p_gpio_bank h2;
	struct s5p_gpio_bank h3;
};

struct s5pc110_gpio {
	struct s5p_gpio_bank a0;
	struct s5p_gpio_bank a1;
	struct s5p_gpio_bank b;
	struct s5p_gpio_bank c0;
	struct s5p_gpio_bank c1;
	struct s5p_gpio_bank d0;
	struct s5p_gpio_bank d1;
	struct s5p_gpio_bank e0;
	struct s5p_gpio_bank e1;
	struct s5p_gpio_bank f0;
	struct s5p_gpio_bank f1;
	struct s5p_gpio_bank f2;
	struct s5p_gpio_bank f3;
	struct s5p_gpio_bank g0;
	struct s5p_gpio_bank g1;
	struct s5p_gpio_bank g2;
	struct s5p_gpio_bank g3;
	struct s5p_gpio_bank i;
	struct s5p_gpio_bank j0;
	struct s5p_gpio_bank j1;
	struct s5p_gpio_bank j2;
	struct s5p_gpio_bank j3;
	struct s5p_gpio_bank j4;
	struct s5p_gpio_bank mp0_1;
	struct s5p_gpio_bank mp0_2;
	struct s5p_gpio_bank mp0_3;
	struct s5p_gpio_bank mp0_4;
	struct s5p_gpio_bank mp0_5;
	struct s5p_gpio_bank mp0_6;
	struct s5p_gpio_bank mp0_7;
	struct s5p_gpio_bank mp1_0;
	struct s5p_gpio_bank mp1_1;
	struct s5p_gpio_bank mp1_2;
	struct s5p_gpio_bank mp1_3;
	struct s5p_gpio_bank mp1_4;
	struct s5p_gpio_bank mp1_5;
	struct s5p_gpio_bank mp1_6;
	struct s5p_gpio_bank mp1_7;
	struct s5p_gpio_bank mp1_8;
	struct s5p_gpio_bank mp2_0;
	struct s5p_gpio_bank mp2_1;
	struct s5p_gpio_bank mp2_2;
	struct s5p_gpio_bank mp2_3;
	struct s5p_gpio_bank mp2_4;
	struct s5p_gpio_bank mp2_5;
	struct s5p_gpio_bank mp2_6;
	struct s5p_gpio_bank mp2_7;
	struct s5p_gpio_bank mp2_8;
	struct s5p_gpio_bank res1[48];
	struct s5p_gpio_bank h0;
	struct s5p_gpio_bank h1;
	struct s5p_gpio_bank h2;
	struct s5p_gpio_bank h3;
};

/* functions */
void s5p_gpio_cfg_pin(struct s5p_gpio_bank *bank, int gpio, int cfg);
void s5p_gpio_direction_output(struct s5p_gpio_bank *bank, int gpio, int en);
void s5p_gpio_direction_input(struct s5p_gpio_bank *bank, int gpio);
void s5p_gpio_set_value(struct s5p_gpio_bank *bank, int gpio, int en);
unsigned int s5p_gpio_get_value(struct s5p_gpio_bank *bank, int gpio);
void s5p_gpio_set_pull(struct s5p_gpio_bank *bank, int gpio, int mode);
void s5p_gpio_set_drv(struct s5p_gpio_bank *bank, int gpio, int mode);
void s5p_gpio_set_rate(struct s5p_gpio_bank *bank, int gpio, int mode);

/* GPIO pins per bank  */
#define GPIO_PER_BANK 8

static inline unsigned int s5p_gpio_base(int nr)
{
	return S5PC110_GPIO_BASE;
}

static inline unsigned int s5p_gpio_part_max(int nr)
{
	return 0;
}

#define s5pc110_gpio_get_nr(bank, pin)	  \
	((((((unsigned int)&(((struct s5pc110_gpio *)S5PC110_GPIO_BASE)->bank))\
	    - S5PC110_GPIO_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin)
#endif

/* Pin configurations */
#define GPIO_INPUT	0x0
#define GPIO_OUTPUT	0x1
#define GPIO_IRQ	0xf
#define GPIO_FUNC(x)	(x)

/* Pull mode */
#define GPIO_PULL_NONE	0x0
#define GPIO_PULL_DOWN	0x1
#define GPIO_PULL_UP	0x2

/* Drive Strength level */
#define GPIO_DRV_1X	0x0
#define GPIO_DRV_3X	0x1
#define GPIO_DRV_2X	0x2
#define GPIO_DRV_4X	0x3
#define GPIO_DRV_FAST	0x0
#define GPIO_DRV_SLOW	0x1

#endif
