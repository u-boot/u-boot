// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 *           2026 Phosh.mobi e.V.
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch-imx8/gpio.h>
#include <linux/delay.h>
#include <efi_loader.h>
#include <env.h>
#include <miiphy.h>
#include <netdev.h>

int board_early_init_f(void)
{
	return 0;
}

#if IS_ENABLED(CONFIG_FEC_MXC)
int board_phy_config(struct phy_device *phydev)
{
	/* enable rgmii rxc skew and phy mode select to RGMII copper */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}
#endif

#if IS_ENABLED(CONFIG_LOAD_ENV_FROM_MMC_BOOT_PARTITION)
uint board_mmc_get_env_part(struct mmc *mmc)
{
	uint part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);

	if (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
	return part;
}
#endif

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		char fdt_str[50];

		env_set("board_name", "librem5-devkit");
		/* Shipped boards have PCB rev. 0.1.2 */
		env_set("board_rev", "0");
		printf("Board name: %s\n", env_get("board_name"));
		printf("Board rev:  %s\n", env_get("board_rev"));

		sprintf(fdt_str, "freescale/imx8mq-librem5-devkit.dtb");
		env_set("fdtfile", fdt_str);
	}

	if (is_usb_boot()) {
		puts("USB Boot\n");
		env_set("bootcmd", "fastboot 0");
	}

	return 0;
}
