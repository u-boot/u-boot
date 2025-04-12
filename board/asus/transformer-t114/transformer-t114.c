// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010-2013
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2023
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

/* T114 Transformers derive from Macallan board */

#include <dm.h>
#include <fdt_support.h>
#include <i2c.h>
#include <log.h>

#ifdef CONFIG_MMC_SDHCI_TEGRA

#define TPS65913_I2C_ADDRESS			0x58
#define TPS65913_PRIMARY_SECONDARY_PAD2		0xfb
#define   GPIO_4				BIT(0)
#define TPS65913_PRIMARY_SECONDARY_PAD3		0xfe
#define   DVFS2					BIT(1)
#define   DVFS1					BIT(0)

/* We are using this function only till palmas pinctrl driver is available */
void pin_mux_mmc(void)
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(0, TPS65913_I2C_ADDRESS, 1, &dev);
	if (ret) {
		log_debug("%s: cannot find PMIC I2C chip\n", __func__);
		return;
	}

	/* GPIO4 function has to be GPIO */
	dm_i2c_reg_clrset(dev, TPS65913_PRIMARY_SECONDARY_PAD2,
			  GPIO_4, 0);

	/* DVFS1 is enabled, DVFS2 is disabled */
	dm_i2c_reg_clrset(dev, TPS65913_PRIMARY_SECONDARY_PAD3,
			  DVFS2 | DVFS1, DVFS1);
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	/* Remove TrustZone nodes */
	fdt_del_node_and_alias(blob, "/firmware");
	fdt_del_node_and_alias(blob, "/reserved-memory/trustzone@bfe00000");

	return 0;
}
#endif
