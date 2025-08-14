// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <dm.h>
#include <dm/devres.h>
#include <env.h>
#include <linux/compat.h>
#include <mpfs-mailbox.h>

DECLARE_GLOBAL_DATA_PTR;

#define MPFS_SYSREG_SOFT_RESET	((unsigned int *)0x20002088)
#define PERIPH_RESET_VALUE		0x1e8u

static unsigned char mac_addr[6];

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	const void *fdt;
	int list_len;

	/*
	 * If there's not a HSS provided dtb, there's no point re-selecting
	 * since we'd just end up re-selecting the same dtb again.
	 */
	if (!gd->arch.firmware_fdt_addr)
		return -EINVAL;

	fdt = (void *)gd->arch.firmware_fdt_addr;

	list_len = fdt_stringlist_count(fdt, 0, "compatible");
	if (list_len < 1)
		return -EINVAL;

	for (int i = 0; i < list_len; i++) {
		int len, match;
		const char *compat;
		char copy[64];
		char *devendored;

		compat = fdt_stringlist_get(fdt, 0, "compatible", i, &len);
		if (!compat)
			return -EINVAL;

		/*
		 * The naming scheme for compatibles doesn't produce anything
		 * close to this long.
		 */
		if (len >= 64)
			return -EINVAL;

		strncpy(copy, compat, 64);
		strtok(copy, ",");

		devendored = strtok(NULL, ",");
		if (!devendored)
			return -EINVAL;

		match = strcmp(devendored, name);
		if (!match)
			return 0;
	}

	return -EINVAL;
}
#endif

int board_fdt_blob_setup(void **fdtp)
{
	fdtp = (void *)_end;

	/*
	 * The devicetree provided by the previous stage is very minimal due to
	 * severe space constraints. The firmware performs no fixups etc.
	 * U-Boot, if providing a devicetree, almost certainly has a better
	 * more complete one than the firmware so that provided by the firmware
	 * is ignored for OF_SEPARATE.
	 */
	if (IS_ENABLED(CONFIG_OF_BOARD) && !IS_ENABLED(CONFIG_MULTI_DTB_FIT)) {
		if (gd->arch.firmware_fdt_addr)
			fdtp = (void *)(uintptr_t)gd->arch.firmware_fdt_addr;
	}

	return 0;
}

int board_init(void)
{
	/* For now nothing to do here. */

	return 0;
}

int board_early_init_f(void)
{
	unsigned int val;

	/* Reset uart, mmc peripheral */
	val = readl(MPFS_SYSREG_SOFT_RESET);
	val = (val & ~(PERIPH_RESET_VALUE));
	writel(val, MPFS_SYSREG_SOFT_RESET);

	return 0;
}

int board_late_init(void)
{
	u32 ret;
	int node;
	u8 device_serial_number[16] = {0};
	void *blob = (void *)gd->fdt_blob;
	struct udevice *dev;
	struct mpfs_sys_serv *sys_serv_priv;

	ret = uclass_get_device_by_name(UCLASS_MISC, "syscontroller", &dev);
	if (ret) {
		debug("%s: system controller setup failed\n", __func__);
		return ret;
	}

	sys_serv_priv = kzalloc(sizeof(*sys_serv_priv), GFP_KERNEL);
	if (!sys_serv_priv)
		return -ENOMEM;

	sys_serv_priv->dev = dev;

	sys_serv_priv->sys_controller = mpfs_syscontroller_get(dev);
	ret = IS_ERR(sys_serv_priv->sys_controller);
	if (ret) {
		debug("%s:  Failed to register system controller sub device ret=%d\n", __func__, ret);
		return -ENODEV;
	}

	ret = mpfs_syscontroller_read_sernum(sys_serv_priv, device_serial_number);
	if (ret) {
		printf("Cannot read device serial number\n");
		return -EINVAL;
	}

	/* Update MAC address with device serial number */
	mac_addr[0] = 0x00;
	mac_addr[1] = 0x04;
	mac_addr[2] = 0xA3;
	mac_addr[3] = device_serial_number[2];
	mac_addr[4] = device_serial_number[1];
	mac_addr[5] = device_serial_number[0];

	node = fdt_path_offset(blob, "/soc/ethernet@20112000");
	if (node >= 0) {
		ret = fdt_setprop(blob, node, "local-mac-address", mac_addr, 6);
		if (ret) {
			printf("Error setting local-mac-address property for ethernet@20112000\n");
			return -ENODEV;
		}
	}

	mac_addr[5] = device_serial_number[0] + 1;

	node = fdt_path_offset(blob, "/soc/ethernet@20110000");
	if (node >= 0) {
		ret = fdt_setprop(blob, node, "local-mac-address", mac_addr, 6);
		if (ret) {
			printf("Error setting local-mac-address property for ethernet@20110000\n");
			return -ENODEV;
		}
	}

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	u32 ret;
	int node;

	node = fdt_path_offset(blob, "/soc/ethernet@20110000");
	if (node >= 0) {
		ret = fdt_setprop(blob, node, "local-mac-address", mac_addr, 6);
		if (ret) {
			printf("Error setting local-mac-address property for ethernet@20110000\n");
			return -ENODEV;
		}
	}

	mac_addr[5] -= 1;

	node = fdt_path_offset(blob, "/soc/ethernet@20112000");
	if (node >= 0) {
		ret = fdt_setprop(blob, node, "local-mac-address", mac_addr, 6);
		if (ret) {
			printf("Error setting local-mac-address property for ethernet@20112000\n");
			return -ENODEV;
		}
	}

	return 0;
}
