/*
 * Board data structure for musb gadget on OMAPs
 *
 * Copyright (C) 2012, Ilya Yanok <ilya.yanok@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARM_OMAP_MUSB_H
#define __ASM_ARM_OMAP_MUSB_H

extern struct musb_platform_ops musb_dsps_ops;
extern const struct musb_platform_ops am35x_ops;
extern const struct musb_platform_ops omap2430_ops;

struct omap_musb_board_data {
	u8 interface_type;
	void (*set_phy_power)(u8 on);
	void (*clear_irq)(void);
	void (*reset)(void);
};

enum musb_interface    {MUSB_INTERFACE_ULPI, MUSB_INTERFACE_UTMI};
#endif /* __ASM_ARM_OMAP_MUSB_H */
