// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025, Kuan-Wei Chiu <visitorckw@gmail.com>
 *
 * QEMU Virtual System Controller Driver
 */

#include <dm.h>
#include <qemu_virt_ctrl.h>
#include <sysreset.h>
#include <asm/io.h>
#include <linux/err.h>

/* Register offsets */
#define VIRT_CTRL_REG_FEATURES  0x00
#define VIRT_CTRL_REG_CMD       0x04

/* Commands */
#define VIRT_CTRL_CMD_NOOP      0x00
#define VIRT_CTRL_CMD_RESET     0x01
#define VIRT_CTRL_CMD_HALT      0x02
#define VIRT_CTRL_CMD_PANIC     0x03

static int qemu_virt_ctrl_request(struct udevice *dev, enum sysreset_t type)
{
	struct qemu_virt_ctrl_plat *plat = dev_get_plat(dev);
	u32 val;

	switch (type) {
	case SYSRESET_WARM:
	case SYSRESET_COLD:
		val = VIRT_CTRL_CMD_RESET;
		break;
	case SYSRESET_POWER_OFF:
		val = VIRT_CTRL_CMD_HALT;
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	writel(val, plat->reg + VIRT_CTRL_REG_CMD);

	return -EINPROGRESS;
}

static struct sysreset_ops qemu_virt_ctrl_ops = {
	.request = qemu_virt_ctrl_request,
};

U_BOOT_DRIVER(sysreset_qemu_virt_ctrl) = {
	.name = "sysreset_qemu_virt_ctrl",
	.id = UCLASS_SYSRESET,
	.ops = &qemu_virt_ctrl_ops,
	.plat_auto = sizeof(struct qemu_virt_ctrl_plat),
};
