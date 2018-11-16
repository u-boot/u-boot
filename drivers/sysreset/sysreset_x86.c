// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * Generic reset driver for x86 processor
 */

#include <common.h>
#include <dm.h>
#include <sysreset.h>
#include <asm/io.h>
#include <asm/processor.h>

static int x86_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	int value;

	switch (type) {
	case SYSRESET_WARM:
		value = SYS_RST | RST_CPU;
		break;
	case SYSRESET_COLD:
		value = SYS_RST | RST_CPU | FULL_RST;
		break;
	default:
		return -ENOSYS;
	}

	outb(value, IO_PORT_RESET);

	return -EINPROGRESS;
}

static const struct udevice_id x86_sysreset_ids[] = {
	{ .compatible = "x86,reset" },
	{ }
};

static struct sysreset_ops x86_sysreset_ops = {
	.request = x86_sysreset_request,
};

U_BOOT_DRIVER(x86_sysreset) = {
	.name = "x86-sysreset",
	.id = UCLASS_SYSRESET,
	.of_match = x86_sysreset_ids,
	.ops = &x86_sysreset_ops,
};
