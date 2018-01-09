/*
 * Copyright (c) 2016 Toradex, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch-tegra/ap.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <power/as3722.h>

#include "../common/tdx-common.h"
#include "pinmux-config-apalis-tk1.h"

#define LAN_RESET_N TEGRA_GPIO(S, 2)

int arch_misc_init(void)
{
	if (readl(NV_PA_BASE_SRAM + NVBOOTINFOTABLE_BOOTTYPE) ==
	    NVBOOTTYPE_RECOVERY)
		printf("USB recovery mode\n");

	return 0;
}

int checkboard(void)
{
	puts("Model: Toradex Apalis TK1 2GB\n");

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_clear_tristate_input_clamping();

	gpio_config_table(apalis_tk1_gpio_inits,
			  ARRAY_SIZE(apalis_tk1_gpio_inits));

	pinmux_config_pingrp_table(apalis_tk1_pingrps,
				   ARRAY_SIZE(apalis_tk1_pingrps));

	pinmux_config_drvgrp_table(apalis_tk1_drvgrps,
				   ARRAY_SIZE(apalis_tk1_drvgrps));
}

#ifdef CONFIG_PCI_TEGRA
int tegra_pcie_board_init(void)
{
	/* TODO: Convert to driver model
	struct udevice *pmic;
	int err;

	err = as3722_init(&pmic);
	if (err) {
		error("failed to initialize AS3722 PMIC: %d\n", err);
		return err;
	}

	err = as3722_sd_enable(pmic, 4);
	if (err < 0) {
		error("failed to enable SD4: %d\n", err);
		return err;
	}

	err = as3722_sd_set_voltage(pmic, 4, 0x24);
	if (err < 0) {
		error("failed to set SD4 voltage: %d\n", err);
		return err;
	}

	err = as3722_gpio_configure(pmic, 1, AS3722_GPIO_OUTPUT_VDDH |
					     AS3722_GPIO_INVERT);
	if (err < 0) {
		error("failed to configure GPIO#1 as output: %d\n", err);
		return err;
	}

	err = as3722_gpio_direction_output(pmic, 2, 1);
	if (err < 0) {
		error("failed to set GPIO#2 high: %d\n", err);
		return err;
	}
	*/

	/* Reset I210 Gigabit Ethernet Controller */
	gpio_request(LAN_RESET_N, "LAN_RESET_N");
	gpio_direction_output(LAN_RESET_N, 0);

	/*
	 * Make sure we don't get any back feeding from LAN_WAKE_N resp.
	 * DEV_OFF_N
	 */
	gpio_request(TEGRA_GPIO(O, 5), "LAN_WAKE_N");
	gpio_direction_output(TEGRA_GPIO(O, 5), 0);

	gpio_request(TEGRA_GPIO(O, 6), "LAN_DEV_OFF_N");
	gpio_direction_output(TEGRA_GPIO(O, 6), 0);

	/* Make sure LDO9 and LDO10 are initially enabled @ 0V */
	/* TODO: Convert to driver model
	err = as3722_ldo_enable(pmic, 9);
	if (err < 0) {
		error("failed to enable LDO9: %d\n", err);
		return err;
	}
	err = as3722_ldo_enable(pmic, 10);
	if (err < 0) {
		error("failed to enable LDO10: %d\n", err);
		return err;
	}
	err = as3722_ldo_set_voltage(pmic, 9, 0x80);
	if (err < 0) {
		error("failed to set LDO9 voltage: %d\n", err);
		return err;
	}
	err = as3722_ldo_set_voltage(pmic, 10, 0x80);
	if (err < 0) {
		error("failed to set LDO10 voltage: %d\n", err);
		return err;
	}
	*/

	mdelay(100);

	/* Make sure controller gets enabled by disabling DEV_OFF_N */
	gpio_set_value(TEGRA_GPIO(O, 6), 1);

	/* Enable LDO9 and LDO10 for +V3.3_ETH on patched prototypes */
	/* TODO: Convert to driver model
	err = as3722_ldo_set_voltage(pmic, 9, 0xff);
	if (err < 0) {
		error("failed to set LDO9 voltage: %d\n", err);
		return err;
	}
	err = as3722_ldo_set_voltage(pmic, 10, 0xff);
	if (err < 0) {
		error("failed to set LDO10 voltage: %d\n", err);
		return err;
	}
	*/

	mdelay(100);
	gpio_set_value(LAN_RESET_N, 1);

#ifdef APALIS_TK1_PCIE_EVALBOARD_INIT
#define PEX_PERST_N	TEGRA_GPIO(DD, 1) /* Apalis GPIO7 */
#define RESET_MOCI_CTRL	TEGRA_GPIO(U, 4)

	/* Reset PLX PEX 8605 PCIe Switch plus PCIe devices on Apalis Evaluation
	   Board */
	gpio_request(PEX_PERST_N, "PEX_PERST_N");
	gpio_request(RESET_MOCI_CTRL, "RESET_MOCI_CTRL");
	gpio_direction_output(PEX_PERST_N, 0);
	gpio_direction_output(RESET_MOCI_CTRL, 0);
	/* Must be asserted for 100 ms after power and clocks are stable */
	mdelay(100);
	gpio_set_value(PEX_PERST_N, 1);
	/* Err_5: PEX_REFCLK_OUTpx/nx Clock Outputs is not Guaranteed Until
	   900 us After PEX_PERST# De-assertion */
	mdelay(1);
	gpio_set_value(RESET_MOCI_CTRL, 1);
#endif /* APALIS_T30_PCIE_EVALBOARD_INIT */

	return 0;
}
#endif /* CONFIG_PCI_TEGRA */
