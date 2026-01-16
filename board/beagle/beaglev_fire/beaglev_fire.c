// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019-2023 Microchip Technology Inc.
 */

#include <dm.h>
#include <dm/devres.h>
#include <env.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/compat.h>
#include <mpfs-mailbox.h>

DECLARE_GLOBAL_DATA_PTR;

#define MPFS_SYSREG_SOFT_RESET	((unsigned int *)0x20002088)
#define PERIPH_RESET_VALUE		0x800001e8u

#if IS_ENABLED(CONFIG_MPFS_SYSCONTROLLER)
static unsigned char mac_addr[6];
#endif

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
#if IS_ENABLED(CONFIG_MPFS_SYSCONTROLLER)
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

	sys_serv_priv = devm_kzalloc(dev, sizeof(*sys_serv_priv), GFP_KERNEL);
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

	node = fdt_path_offset(blob, "/soc/ethernet@20110000");
	if (node >= 0) {
		ret = fdt_setprop(blob, node, "local-mac-address", mac_addr, 6);
		if (ret) {
			printf("Error setting local-mac-address property for ethernet@20110000\n");
			return -ENODEV;
		}
	}

	mpfs_syscontroller_process_dtbo(sys_serv_priv);
#endif

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
#if IS_ENABLED(CONFIG_MPFS_SYSCONTROLLER)
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
#endif

	return 0;
}