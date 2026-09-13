// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 - 2009 Windriver, <www.windriver.com>
 * Author: Tom Rix <Tom.Rix@windriver.com>
 *
 * (C) Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 */

#include <console.h>
#include <fastboot.h>
#include <g_dnl.h>
#include <stdio.h>
#include <linux/errno.h>
#include <linux/printk.h>
#include <linux/usb/gadget.h>
#include <u-boot/schedule.h>

int fastboot_usb_run(int controller_index, void *buf_addr, u32 buf_size)
{
	struct udevice *udc;
	int ret;

	ret = udc_device_get_by_index(controller_index, &udc);
	if (ret) {
		pr_err("USB init failed: %d\n", ret);
		return ret;
	}

	fastboot_init(buf_addr, buf_size);
	g_dnl_clear_detach();

	ret = g_dnl_register("usb_dnl_fastboot");
	if (ret)
		goto out_put;

	if (!g_dnl_board_usb_cable_connected()) {
		puts("\rUSB cable not detected.\n");
		ret = -ENODEV;
		goto out_unregister;
	}

	while (!g_dnl_detach()) {
#ifndef CONFIG_XPL_BUILD
		/* SPL callers own the session lifetime and may have no console. */
		if (CONFIG_IS_ENABLED(CMD_FASTBOOT_ABORT_KEYED)) {
			if (tstc()) {
				getchar();
				puts("\rOperation aborted.\n");
				break;
			}
		} else if (ctrlc()) {
			break;
		}
#endif
		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}

out_unregister:
	g_dnl_unregister();
	g_dnl_clear_detach();
out_put:
	udc_device_put(udc);

	return ret;
}
