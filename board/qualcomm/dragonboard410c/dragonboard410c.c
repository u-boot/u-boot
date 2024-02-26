// SPDX-License-Identifier: GPL-2.0+
/*
 * Board init file for Dragonboard 410C
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */

#include <button.h>
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <env.h>
#include <init.h>
#include <net.h>
#include <usb.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <fdt_support.h>
#include <asm/arch/dram.h>
#include <asm/arch/misc.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define USB_HUB_RESET_GPIO 2
#define USB_SW_SELECT_GPIO 3

int board_usb_init(int index, enum usb_init_type init)
{
	struct udevice *usb;
	int ret = 0;

	/* USB device */
	ret = device_find_global_by_ofnode(ofnode_path("/soc/usb"), &usb);
	if (ret) {
		printf("Cannot find USB device\n");
		return ret;
	}

	/* Select "default" or "device" pinctrl */
	switch (init) {
	case USB_INIT_HOST:
		pinctrl_select_state(usb, "default");
		break;
	case USB_INIT_DEVICE:
		pinctrl_select_state(usb, "device");
		break;
	default:
		debug("Unknown usb_init_type %d\n", init);
		break;
	}

	return 0;
}

/* Check for vol- button - if pressed - stop autoboot */
int misc_init_r(void)
{
	struct udevice *btn;
	int ret;
	enum button_state_t state;

	ret = button_get_by_label("vol_down", &btn);
	if (ret < 0) {
		printf("Couldn't find power button!\n");
		return ret;
	}

	state = button_get_state(btn);
	if (state == BUTTON_ON) {
		env_set("preboot", "setenv preboot; fastboot 0");
		printf("vol_down pressed - Starting fastboot.\n");
	}

	return 0;
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	char serial[16];

	memset(serial, 0, 16);
	snprintf(serial, 13, "%x", msm_board_serial());
	env_set("serial#", serial);
	return 0;
}

/* Fixup of DTB for Linux Kernel
 * 1. Fixup installed DRAM.
 * 2. Fixup WLAN/BT Mac address:
 *	First, check if MAC addresses for WLAN/BT exists as environemnt
 *	variables wlanaddr,btaddr. if not, generate a unique address.
 */

int ft_board_setup(void *blob, struct bd_info *bd)
{
	u8 mac[ARP_HLEN];

	msm_fixup_memory(blob);

	if (!eth_env_get_enetaddr("wlanaddr", mac)) {
		msm_generate_mac_addr(mac);
	};

	do_fixup_by_compat(blob, "qcom,wcnss-wlan",
			   "local-mac-address", mac, ARP_HLEN, 1);


	if (!eth_env_get_enetaddr("btaddr", mac)) {
		msm_generate_mac_addr(mac);

/* The BD address is same as WLAN MAC address but with
 * least significant bit flipped.
 */
		mac[0] ^= 0x01;
	};

	do_fixup_by_compat(blob, "qcom,wcnss-bt",
			   "local-bd-address", mac, ARP_HLEN, 1);
	return 0;
}

void reset_cpu(void)
{
	psci_system_reset();
}
