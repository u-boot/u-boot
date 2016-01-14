/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/uart.h>
#include <asm/arch-rockchip/grf_rk3036.h>
#include <asm/arch/sdram_rk3036.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

#define GRF_BASE	0x20008000

void get_ddr_config(struct rk3036_ddr_config *config)
{
	/* K4B4G1646Q config */
	config->ddr_type = 3;
	config->rank = 1;
	config->cs0_row = 15;
	config->cs1_row = 15;

	/* 8bank */
	config->bank = 3;
	config->col = 10;

	/* 16bit bw */
	config->bw = 1;
}

#define FASTBOOT_KEY_GPIO 93

int fastboot_key_pressed(void)
{
	gpio_request(FASTBOOT_KEY_GPIO, "fastboot_key");
	gpio_direction_input(FASTBOOT_KEY_GPIO);
	return !gpio_get_value(FASTBOOT_KEY_GPIO);
}

#define ROCKCHIP_BOOT_MODE_FASTBOOT	0x5242C309

int board_late_init(void)
{
	struct rk3036_grf * const grf = (void *)GRF_BASE;
	int boot_mode = readl(&grf->os_reg[4]);

	/* Clear boot mode */
	writel(0, &grf->os_reg[4]);

	if (boot_mode == ROCKCHIP_BOOT_MODE_FASTBOOT ||
	    fastboot_key_pressed()) {
		printf("enter fastboot!\n");
		setenv("preboot", "setenv preboot; fastboot usb0");
	}

	return 0;
}

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = sdram_size();

	return 0;
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
