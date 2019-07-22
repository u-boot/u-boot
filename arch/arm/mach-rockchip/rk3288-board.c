// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <ram.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3288.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/pmu_rk3288.h>
#include <asm/arch-rockchip/boot_mode.h>
#include <asm/gpio.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int rk_board_late_init(void)
{
	return 0;
}

static void rk3288_detect_reset_reason(void)
{
	struct rk3288_cru *cru = rockchip_get_cru();
	const char *reason;

	if (IS_ERR(cru))
		return;

	switch (cru->cru_glb_rst_st) {
	case GLB_POR_RST:
		reason = "POR";
		break;
	case FST_GLB_RST_ST:
	case SND_GLB_RST_ST:
		reason = "RST";
		break;
	case FST_GLB_TSADC_RST_ST:
	case SND_GLB_TSADC_RST_ST:
		reason = "THERMAL";
		break;
	case FST_GLB_WDT_RST_ST:
	case SND_GLB_WDT_RST_ST:
		reason = "WDOG";
		break;
	default:
		reason = "unknown reset";
	}

	env_set("reset_reason", reason);

	/*
	 * Clear cru_glb_rst_st, so we can determine the last reset cause
	 * for following resets.
	 */
	rk_clrreg(&cru->cru_glb_rst_st, GLB_RST_ST_MASK);
}

int board_late_init(void)
{
	setup_boot_mode();
	rk3288_detect_reset_reason();

	return rk_board_late_init();
}

int board_init(void)
{
	return 0;
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#if defined(CONFIG_USB_GADGET) && defined(CONFIG_USB_GADGET_DWC2_OTG)
#include <usb.h>
#include <usb/dwc2_udc.h>

static struct dwc2_plat_otg_data otg_data = {
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
					"snps,dwc2");

	while (node > 0) {
		mode = fdt_getprop(blob, node, "dr_mode", NULL);
		if (mode && strcmp(mode, "otg") == 0) {
			matched = true;
			break;
		}

		node = fdt_node_offset_by_compatible(blob, node,
					"snps,dwc2");
	}
	if (!matched) {
		debug("Not found usb_otg device\n");
		return -ENODEV;
	}
	otg_data.regs_otg = fdtdec_get_addr(blob, node, "reg");

	return dwc2_udc_probe(&otg_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}
#endif
