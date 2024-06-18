// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <dm.h>
#include <cpu.h>
#include <log.h>
#include <init.h>
#include <usb.h>
#include <virtio_types.h>
#include <virtio.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

unsigned long get_board_sys_clk(void)
{
	return gd->cpu_clk ? gd->cpu_clk : 40000000;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int board_early_init_f(void)
{
	struct cpu_plat *cpu_plat;
	struct udevice *cpu = cpu_get_current_dev();

	if (!cpu)
		return -ENODEV;

	cpu_plat = dev_get_parent_plat(cpu);
	if (!cpu_plat)
		return -ENODEV;

	gd->cpu_clk = cpu_plat->timebase_freq;
	return 0;
}

int board_late_init(void)
{
	/* start usb so that usb keyboard can be used as input device */
	if (CONFIG_IS_ENABLED(USB_KEYBOARD))
		usb_init();

	/*
	 * Make sure virtio bus is enumerated so that peripherals
	 * on the virtio bus can be discovered by their drivers
	 */
	virtio_init();

	return 0;
}
