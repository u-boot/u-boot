/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <os.h>

/* system timer offset in ms */
static unsigned long sandbox_timer_offset;

void sandbox_timer_add_offset(unsigned long offset)
{
	sandbox_timer_offset += offset;
}

static int sandbox_timer_get_count(struct udevice *dev, u64 *count)
{
	*count = os_get_nsec() / 1000 + sandbox_timer_offset * 1000;

	return 0;
}

static int sandbox_timer_probe(struct udevice *dev)
{
	return 0;
}

static const struct timer_ops sandbox_timer_ops = {
	.get_count = sandbox_timer_get_count,
};

static const struct udevice_id sandbox_timer_ids[] = {
	{ .compatible = "sandbox,timer" },
	{ }
};

U_BOOT_DRIVER(sandbox_timer) = {
	.name	= "sandbox_timer",
	.id	= UCLASS_TIMER,
	.of_match = sandbox_timer_ids,
	.probe = sandbox_timer_probe,
	.ops	= &sandbox_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
