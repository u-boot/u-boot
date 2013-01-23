/*
 * (C) Copyright 2010 Samsung Electronics
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
 *
 */

#ifndef __ASM_ARM_ARCH_CLK_H_
#define __ASM_ARM_ARCH_CLK_H_

#define APLL	0
#define MPLL	1
#define EPLL	2
#define HPLL	3
#define VPLL	4
#define BPLL	5

unsigned long get_pll_clk(int pllreg);
unsigned long get_arm_clk(void);
unsigned long get_i2c_clk(void);
unsigned long get_pwm_clk(void);
unsigned long get_uart_clk(int dev_index);
unsigned long get_mmc_clk(int dev_index);
void set_mmc_clk(int dev_index, unsigned int div);
unsigned long get_lcd_clk(void);
void set_lcd_clk(void);
void set_mipi_clk(void);
void set_i2s_clk_source(void);
int set_i2s_clk_prescaler(unsigned int src_frq, unsigned int dst_frq);
int set_epll_clk(unsigned long rate);
int set_spi_clk(int periph_id, unsigned int rate);

#endif
