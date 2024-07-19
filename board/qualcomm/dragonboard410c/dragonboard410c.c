// SPDX-License-Identifier: GPL-2.0+
/*
 * Board init file for Dragonboard 410C
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */

#include <button.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <env.h>
#include <init.h>
#include <mmc.h>
#include <net.h>
#include <usb.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <fdt_support.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

/* UNSTUFF_BITS macro taken from Linux Kernel: drivers/mmc/core/sd.c */
#define UNSTUFF_BITS(resp, start, size) \
	({ \
		const int __size = size; \
		const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1;	\
		const int __off = 3 - ((start) / 32); \
		const int __shft = (start) & 31; \
		u32 __res; \
					\
		__res = resp[__off] >> __shft; \
		if (__size + __shft > 32) \
			__res |= resp[__off - 1] << ((32 - __shft) % 32); \
		__res & __mask;	\
	})

static u32 msm_board_serial(void)
{
	struct mmc *mmc_dev;

	mmc_dev = find_mmc_device(0);
	if (!mmc_dev)
		return 0;

	if (mmc_init(mmc_dev))
		return 0;

	return UNSTUFF_BITS(mmc_dev->cid, 16, 32);
}

static void msm_generate_mac_addr(u8 *mac)
{
	/* use locally adminstrated pool */
	mac[0] = 0x02;
	mac[1] = 0x00;

	/*
	 * Put the 32-bit serial number in the last 32-bit of the MAC address.
	 * Use big endian order so it is consistent with the serial number
	 * written as a hexadecimal string, e.g. 0x1234abcd -> 02:00:12:34:ab:cd
	 */
	put_unaligned_be32(msm_board_serial(), &mac[2]);
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

int qcom_late_init(void)
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
