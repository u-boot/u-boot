/*
 * (C) Copyright 2012
 * Ilya Yanok, <ilya.yanok@gmail.com>
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
 * Foundation, Inc.
 */

#ifndef __ASM_ARCH_OMAP3_MUSB_H
#define __ASM_ARCH_OMAP3_MUSB_H
extern void am35x_musb_reset(void);
extern void am35x_musb_phy_power(u8 on);
extern void am35x_musb_clear_irq(void);
#endif
