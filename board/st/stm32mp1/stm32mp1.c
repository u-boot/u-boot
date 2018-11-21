// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */
#include <config.h>
#include <common.h>
#include <led.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <phy.h>
#include <reset.h>
#include <usb.h>
#include <asm/arch/stm32.h>
#include <asm/io.h>
#include <power/regulator.h>
#include <usb/dwc2_udc.h>

/*
 * Get a global data pointer
 */
DECLARE_GLOBAL_DATA_PTR;

#define STM32MP_GUSBCFG 0x40002407

#define STM32MP_GGPIO 0x38
#define STM32MP_GGPIO_VBUS_SENSING BIT(21)

static struct dwc2_plat_otg_data stm32mp_otg_data = {
	.usb_gusbcfg = STM32MP_GUSBCFG,
};

static struct reset_ctl usbotg_reset;

int board_usb_init(int index, enum usb_init_type init)
{
	struct fdtdec_phandle_args args;
	struct udevice *dev;
	const void *blob = gd->fdt_blob;
	struct clk clk;
	struct phy phy;
	int node;
	int phy_provider;
	int ret;

	/* find the usb otg node */
	node = fdt_node_offset_by_compatible(blob, -1, "snps,dwc2");
	if (node < 0) {
		debug("Not found usb_otg device\n");
		return -ENODEV;
	}

	if (!fdtdec_get_is_enabled(blob, node)) {
		debug("stm32 usbotg is disabled in the device tree\n");
		return -ENODEV;
	}

	/* Enable clock */
	ret = fdtdec_parse_phandle_with_args(blob, node, "clocks",
					     "#clock-cells", 0, 0, &args);
	if (ret) {
		debug("usbotg has no clocks defined in the device tree\n");
		return ret;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_CLK, args.node, &dev);
	if (ret)
		return ret;

	if (args.args_count != 1) {
		debug("Can't find clock ID in the device tree\n");
		return -ENODATA;
	}

	clk.dev = dev;
	clk.id = args.args[0];

	ret = clk_enable(&clk);
	if (ret) {
		debug("Failed to enable usbotg clock\n");
		return ret;
	}

	/* Reset */
	ret = fdtdec_parse_phandle_with_args(blob, node, "resets",
					     "#reset-cells", 0, 0, &args);
	if (ret) {
		debug("usbotg has no resets defined in the device tree\n");
		goto clk_err;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_RESET, args.node, &dev);
	if (ret || args.args_count != 1)
		goto clk_err;

	usbotg_reset.dev = dev;
	usbotg_reset.id = args.args[0];

	reset_assert(&usbotg_reset);
	udelay(2);
	reset_deassert(&usbotg_reset);

	/* Get USB PHY */
	ret = fdtdec_parse_phandle_with_args(blob, node, "phys",
					     "#phy-cells", 0, 0, &args);
	if (!ret) {
		phy_provider = fdt_parent_offset(blob, args.node);
		ret = uclass_get_device_by_of_offset(UCLASS_PHY,
						     phy_provider, &dev);
		if (ret)
			goto clk_err;

		phy.dev = dev;
		phy.id = fdtdec_get_uint(blob, args.node, "reg", -1);

		ret = generic_phy_power_on(&phy);
		if (ret) {
			debug("unable to power on the phy\n");
			goto clk_err;
		}

		ret = generic_phy_init(&phy);
		if (ret) {
			debug("failed to init usb phy\n");
			goto phy_power_err;
		}
	}

	/* Parse and store data needed for gadget */
	stm32mp_otg_data.regs_otg = fdtdec_get_addr(blob, node, "reg");
	if (stm32mp_otg_data.regs_otg == FDT_ADDR_T_NONE) {
		debug("usbotg: can't get base address\n");
		ret = -ENODATA;
		goto phy_init_err;
	}

	stm32mp_otg_data.rx_fifo_sz = fdtdec_get_int(blob, node,
						     "g-rx-fifo-size", 0);
	stm32mp_otg_data.np_tx_fifo_sz = fdtdec_get_int(blob, node,
							"g-np-tx-fifo-size", 0);
	stm32mp_otg_data.tx_fifo_sz = fdtdec_get_int(blob, node,
						     "g-tx-fifo-size", 0);
	/* Enable voltage level detector */
	if (!(fdtdec_parse_phandle_with_args(blob, node, "usb33d-supply",
					     NULL, 0, 0, &args))) {
		if (!uclass_get_device_by_of_offset(UCLASS_REGULATOR,
						    args.node, &dev)) {
			ret = regulator_set_enable(dev, true);
			if (ret) {
				debug("Failed to enable usb33d\n");
				goto phy_init_err;
			}
		}
	}
		/* Enable vbus sensing */
	setbits_le32(stm32mp_otg_data.regs_otg + STM32MP_GGPIO,
		     STM32MP_GGPIO_VBUS_SENSING);

	return dwc2_udc_probe(&stm32mp_otg_data);

phy_init_err:
	generic_phy_exit(&phy);

phy_power_err:
	generic_phy_power_off(&phy);

clk_err:
	clk_disable(&clk);

	return ret;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	/* Reset usbotg */
	reset_assert(&usbotg_reset);
	udelay(2);
	reset_deassert(&usbotg_reset);

	return 0;
}

int board_late_init(void)
{
	return 0;
}

/* board dependent setup after realloc */
int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = STM32_DDR_BASE + 0x100;

	if (IS_ENABLED(CONFIG_LED))
		led_default_state();

	return 0;
}
