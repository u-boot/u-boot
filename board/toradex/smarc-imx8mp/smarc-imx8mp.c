// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (C) 2024 Toradex */

#include <init.h>
#include <asm/global_data.h>
#include <asm-generic/gpio.h>
#include <linux/errno.h>

#include "../common/tdx-cfg-block.h"

DECLARE_GLOBAL_DATA_PTR;

int board_phys_sdram_size(phys_size_t *size)
{
	if (!size)
		return -EINVAL;

	*size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE + PHYS_SDRAM_2_SIZE);

	return 0;
}

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
static bool board_has_wifi(void)
{
	struct gpio_desc *desc;

	if (!gpio_hog_lookup_name("BT_UART_RXD_GPIO", &desc))
		return !!dm_gpio_get_value(desc);

	return true;
}

/*
 * Module variants with a Wi-Fi/Bluetooth module use UART3 for Bluetooth,
 * those without use UART3 as the SMARC SER3 UART.
 * Test for a Wi-Fi module and if none found reassign UART3 interface to
 * the SMARC SER3 pins.
 */
static void ft_board_assign_uart(void *blob)
{
	const char *uart_path = "/soc@0/bus@30800000/spba-bus@30800000/serial@30880000";
	const char *pinctrl_path = "/soc@0/bus@30000000/pinctrl@30330000/uart3grp";
	int pinctrl_offset;
	int uart_offset;
	int bt_offset;
	u32 phandle;

	if (board_has_wifi())
		return;

	uart_offset = fdt_path_offset(blob, uart_path);
	if (uart_offset < 0)
		return;

	fdt_delprop(blob, uart_offset, "uart-has-rtscts");
	bt_offset = fdt_subnode_offset(blob, uart_offset, "bluetooth");
	if (bt_offset < 0)
		return;

	fdt_del_node(blob, bt_offset);

	pinctrl_offset = fdt_path_offset(blob, pinctrl_path);
	if (pinctrl_offset < 0)
		return;

	phandle = fdt_get_phandle(blob, pinctrl_offset);
	if (phandle < 0)
		return;

	fdt_setprop_u32(blob, uart_offset, "pinctrl-0", phandle);
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	ft_board_assign_uart(blob);

	return ft_common_board_setup(blob, bd);
}
#endif
