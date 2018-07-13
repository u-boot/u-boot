// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2005-2007 by Texas Instruments
 * Some code has been taken from tusb6010.c
 * Copyrights for that are attributable to:
 * Copyright (C) 2006 Nokia Corporation
 * Tony Lindgren <tony@atomide.com>
 *
 * This file is part of the Inventra Controller Driver for Linux.
 */
#include <common.h>
#include <asm/omap_common.h>
#include <asm/omap_musb.h>
#include <twl4030.h>
#include <twl6030.h>
#include "linux-compat.h"

#include "musb_core.h"
#include "omap2430.h"

static inline void omap2430_low_level_exit(struct musb *musb)
{
	u32 l;

	/* in any role */
	l = musb_readl(musb->mregs, OTG_FORCESTDBY);
	l |= ENABLEFORCE;	/* enable MSTANDBY */
	musb_writel(musb->mregs, OTG_FORCESTDBY, l);
}

static inline void omap2430_low_level_init(struct musb *musb)
{
	u32 l;

	l = musb_readl(musb->mregs, OTG_FORCESTDBY);
	l &= ~ENABLEFORCE;	/* disable MSTANDBY */
	musb_writel(musb->mregs, OTG_FORCESTDBY, l);
}


static int omap2430_musb_init(struct musb *musb)
{
	u32 l;
	int status = 0;
	unsigned long int start;
	struct omap_musb_board_data *data =
		(struct omap_musb_board_data *)musb->controller;

	/* Reset the controller */
	musb_writel(musb->mregs, OTG_SYSCONFIG, SOFTRST);

	start = get_timer(0);

	while (1) {
		l = musb_readl(musb->mregs, OTG_SYSCONFIG);
		if ((l & SOFTRST) == 0)
			break;

		if (get_timer(start) > (CONFIG_SYS_HZ / 1000)) {
			dev_err(musb->controller, "MUSB reset is taking too long\n");
			return -ENODEV;
		}
	}

	l = musb_readl(musb->mregs, OTG_INTERFSEL);

	if (data->interface_type == MUSB_INTERFACE_UTMI) {
		/* OMAP4 uses Internal PHY GS70 which uses UTMI interface */
		l &= ~ULPI_12PIN;       /* Disable ULPI */
		l |= UTMI_8BIT;         /* Enable UTMI  */
	} else {
		l |= ULPI_12PIN;
	}

	musb_writel(musb->mregs, OTG_INTERFSEL, l);

	pr_debug("HS USB OTG: revision 0x%x, sysconfig 0x%02x, "
			"sysstatus 0x%x, intrfsel 0x%x, simenable  0x%x\n",
			musb_readl(musb->mregs, OTG_REVISION),
			musb_readl(musb->mregs, OTG_SYSCONFIG),
			musb_readl(musb->mregs, OTG_SYSSTATUS),
			musb_readl(musb->mregs, OTG_INTERFSEL),
			musb_readl(musb->mregs, OTG_SIMENABLE));
	return 0;

err1:
	return status;
}

static int omap2430_musb_enable(struct musb *musb)
{
#ifdef CONFIG_TWL4030_USB
	if (twl4030_usb_ulpi_init()) {
		serial_printf("ERROR: %s Could not initialize PHY\n",
				__PRETTY_FUNCTION__);
	}
#endif

#ifdef CONFIG_TWL6030_POWER
	twl6030_usb_device_settings();
#endif

#ifdef CONFIG_OMAP44XX
	u32 *usbotghs_control = (u32 *)((*ctrl)->control_usbotghs_ctrl);
	*usbotghs_control = USBOTGHS_CONTROL_AVALID |
		USBOTGHS_CONTROL_VBUSVALID | USBOTGHS_CONTROL_IDDIG;
#endif

	return 0;
}

static void omap2430_musb_disable(struct musb *musb)
{

}

static int omap2430_musb_exit(struct musb *musb)
{
	del_timer_sync(&musb_idle_timer);

	omap2430_low_level_exit(musb);

	return 0;
}

const struct musb_platform_ops omap2430_ops = {
	.init		= omap2430_musb_init,
	.exit		= omap2430_musb_exit,
	.enable		= omap2430_musb_enable,
	.disable	= omap2430_musb_disable,
};
