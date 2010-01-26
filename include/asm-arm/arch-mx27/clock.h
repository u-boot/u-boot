/*
 *
 * (c) 2009 Ilya Yanok, Emcraft Systems <yanok@emcraft.com>
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

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H
unsigned int imx_decode_pll(unsigned int pll, unsigned int f_ref);

ulong imx_get_mpllclk(void);
ulong imx_get_armclk(void);
ulong imx_get_spllclk(void);
ulong imx_get_fclk(void);
ulong imx_get_hclk(void);
ulong imx_get_bclk(void);
ulong imx_get_perclk1(void);
ulong imx_get_perclk2(void);
ulong imx_get_perclk3(void);
ulong imx_get_ahbclk(void);

#define imx_get_uartclk imx_get_perclk1
#define imx_get_fecclk imx_get_ahbclk

#endif /* __ASM_ARCH_CLOCK_H */
