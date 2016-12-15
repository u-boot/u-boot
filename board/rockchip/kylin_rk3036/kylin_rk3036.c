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

#if defined(CONFIG_USB_GADGET) && defined(CONFIG_USB_GADGET_DWC2_OTG)
#include <usb.h>
#include <usb/dwc2_udc.h>

static struct dwc2_plat_otg_data rk3036_otg_data = {
	.rx_fifo_sz	= 512,
	.np_tx_fifo_sz	= 16,
	.tx_fifo_sz	= 128,
};

int board_usb_init(int index, enum usb_init_type init)
{
	int node;
	const char *mode;
	bool matched = false;
	const void *blob = gd->fdt_blob;

	/* find the usb_otg node */
	node = fdt_node_offset_by_compatible(blob, -1,
					"rockchip,rk3288-usb");

	while (node > 0) {
		mode = fdt_getprop(blob, node, "dr_mode", NULL);
		if (mode && strcmp(mode, "otg") == 0) {
			matched = true;
			break;
		}

		node = fdt_node_offset_by_compatible(blob, node,
					"rockchip,rk3288-usb");
	}
	if (!matched) {
		debug("Not found usb_otg device\n");
		return -ENODEV;
	}
	rk3036_otg_data.regs_otg = fdtdec_get_addr(blob, node, "reg");

	return dwc2_udc_probe(&rk3036_otg_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}
#endif
