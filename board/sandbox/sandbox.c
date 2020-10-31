// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <cpu_func.h>
#include <cros_ec.h>
#include <dm.h>
#include <env_internal.h>
#include <init.h>
#include <led.h>
#include <os.h>
#include <asm/global_data.h>
#include <asm/test.h>
#include <asm/u-boot-sandbox.h>

/*
 * Pointer to initial global data area
 *
 * Here we initialize it.
 */
gd_t *gd;

#if !CONFIG_IS_ENABLED(OF_PLATDATA)
/* Add a simple GPIO device */
U_BOOT_DRVINFO(gpio_sandbox) = {
	.name = "sandbox_gpio",
};
#endif

#ifndef CONFIG_TIMER
/* system timer offset in ms */
static unsigned long sandbox_timer_offset;

void timer_test_add_offset(unsigned long offset)
{
	sandbox_timer_offset += offset;
}

unsigned long timer_read_counter(void)
{
	return os_get_nsec() / 1000 + sandbox_timer_offset * 1000;
}
#endif

/* specific order for sandbox: nowhere is the first value, used by default */
static enum env_location env_locations[] = {
	ENVL_NOWHERE,
	ENVL_EXT4,
};

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio >= ARRAY_SIZE(env_locations))
		return ENVL_UNKNOWN;

	return env_locations[prio];
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

int board_init(void)
{
	if (IS_ENABLED(CONFIG_LED))
		led_default_state();

	return 0;
}

int ft_board_setup(void *fdt, struct bd_info *bd)
{
	/* Create an arbitrary reservation to allow testing OF_BOARD_SETUP.*/
	return fdt_add_mem_rsv(fdt, 0x00d02000, 0x4000);
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_CROS_EC, &dev);
	if (ret && ret != -ENODEV) {
		/* Force console on */
		gd->flags &= ~GD_FLG_SILENT;

		printf("cros-ec communications failure %d\n", ret);
		puts("\nPlease reset with Power+Refresh\n\n");
		panic("Cannot init cros-ec device");
		return -1;
	}
	return 0;
}
#endif
