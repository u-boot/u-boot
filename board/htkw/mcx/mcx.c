/*
 * Copyright (C) 2011 Ilya Yanok, Emcraft Systems
 *
 * Based on ti/evm/evm.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/omap_gpio.h>
#include "errno.h"
#include <i2c.h>
#ifdef CONFIG_USB_EHCI
#include <usb.h>
#include <asm/ehci-omap.h>
#endif
#include "mcx.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_USB_EHCI
static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
};

int ehci_hcd_init(void)
{
	return omap_ehci_hcd_init(&usbhs_bdata);
}

int ehci_hcd_stop(void)
{
	return omap_ehci_hcd_stop();
}
#endif

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_MCX();
}

#if defined(CONFIG_OMAP_HSMMC) && !defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0);
}
#endif

#ifdef CONFIG_USB_EHCI_OMAP
#define USB_HOST_PWR_EN		132
int board_usb_init(void)
{
	if (gpio_request(USB_HOST_PWR_EN, "USB_HOST_PWR_EN") < 0) {
		puts("Failed to get USB_HOST_PWR_EN pin\n");
		return -ENODEV;
	}
	gpio_direction_output(USB_HOST_PWR_EN, 1);

	return 0;
}
#endif
