/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cros_ec.h>
#include <dm.h>
#include <os.h>
#include <asm/u-boot-sandbox.h>

/*
 * Pointer to initial global data area
 *
 * Here we initialize it.
 */
gd_t *gd;

/* Add a simple GPIO device */
U_BOOT_DEVICE(gpio_sandbox) = {
	.name = "gpio_sandbox",
};

void flush_cache(unsigned long start, unsigned long size)
{
}

unsigned long timer_read_counter(void)
{
	return os_get_nsec() / 1000;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifdef CONFIG_VIDEO_SANDBOX_SDL
	int ret;

	ret = sandbox_lcd_sdl_early_init();
	if (ret) {
		puts("Could not init sandbox LCD emulation\n");
		return ret;
	}
#endif

	return 0;
}
#endif

int arch_early_init_r(void)
{
#ifdef CONFIG_CROS_EC
	if (cros_ec_board_init()) {
		printf("%s: Failed to init EC\n", __func__);
		return 0;
	}
#endif

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	if (cros_ec_get_error()) {
		/* Force console on */
		gd->flags &= ~GD_FLG_SILENT;

		printf("cros-ec communications failure %d\n",
		       cros_ec_get_error());
		puts("\nPlease reset with Power+Refresh\n\n");
		panic("Cannot init cros-ec device");
		return -1;
	}
	return 0;
}
#endif
