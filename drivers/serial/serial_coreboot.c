// SPDX-License-Identifier: GPL-2.0+
/*
 * UART support for U-Boot when launched from Coreboot
 *
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <ns16550.h>
#include <serial.h>
#include <asm/arch/sysinfo.h>

static int coreboot_ofdata_to_platdata(struct udevice *dev)
{
	struct ns16550_platdata *plat = dev_get_platdata(dev);
	struct cb_serial *cb_info = lib_sysinfo.serial;

	plat->base = cb_info->baseaddr;
	plat->reg_shift = cb_info->regwidth == 4 ? 2 : 0;
	plat->reg_width = cb_info->regwidth;
	plat->clock = cb_info->input_hertz;
	plat->fcr = UART_FCR_DEFVAL;
	plat->flags = 0;
	if (cb_info->type == CB_SERIAL_TYPE_IO_MAPPED)
		plat->flags |= NS16550_FLAG_IO;

	return 0;
}

static const struct udevice_id coreboot_serial_ids[] = {
	{ .compatible = "coreboot-serial" },
	{ },
};

U_BOOT_DRIVER(coreboot_uart) = {
	.name	= "coreboot_uart",
	.id	= UCLASS_SERIAL,
	.of_match	= coreboot_serial_ids,
	.priv_auto_alloc_size = sizeof(struct NS16550),
	.platdata_auto_alloc_size = sizeof(struct ns16550_platdata),
	.ofdata_to_platdata  = coreboot_ofdata_to_platdata,
	.probe	= ns16550_serial_probe,
	.ops	= &ns16550_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};
