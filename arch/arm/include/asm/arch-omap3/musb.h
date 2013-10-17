/*
 * (C) Copyright 2012
 * Ilya Yanok, <ilya.yanok@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_OMAP3_MUSB_H
#define __ASM_ARCH_OMAP3_MUSB_H
extern void am35x_musb_reset(void);
extern void am35x_musb_phy_power(u8 on);
extern void am35x_musb_clear_irq(void);
#endif
