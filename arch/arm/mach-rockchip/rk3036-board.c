/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <ram.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/periph.h>
#include <asm/arch/grf_rk3036.h>
#include <asm/arch/boot_mode.h>
#include <asm/arch/sdram_rk3036.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

#define GRF_BASE	0x20008000

static void setup_boot_mode(void)
{
	struct rk3036_grf *const grf = (void *)GRF_BASE;
	int boot_mode = readl(&grf->os_reg[4]);

	debug("boot mode %x.\n", boot_mode);

	/* Clear boot mode */
	writel(BOOT_NORMAL, &grf->os_reg[4]);

	switch (boot_mode) {
	case BOOT_FASTBOOT:
		printf("enter fastboot!\n");
		setenv("preboot", "setenv preboot; fastboot usb0");
		break;
	case BOOT_UMS:
		printf("enter UMS!\n");
		setenv("preboot", "setenv preboot; ums mmc 0");
		break;
	}
}

__weak int rk_board_late_init(void)
{
	return 0;
}

int board_late_init(void)
{
	setup_boot_mode();

	return rk_board_late_init();
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
