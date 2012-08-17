/*
 * Freescale MX28EVK board
 *
 * (C) Copyright 2011 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * Based on m28evk.c:
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx28.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <linux/mii.h>
#include <miiphy.h>
#include <netdev.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Functions
 */
int board_early_init_f(void)
{
	/* IO0 clock at 480MHz */
	mx28_set_ioclk(MXC_IOCLK0, 480000);
	/* IO1 clock at 480MHz */
	mx28_set_ioclk(MXC_IOCLK1, 480000);

	/* SSP0 clock at 96MHz */
	mx28_set_sspclk(MXC_SSPCLK0, 96000, 0);
	/* SSP2 clock at 96MHz */
	mx28_set_sspclk(MXC_SSPCLK2, 96000, 0);

#ifdef	CONFIG_CMD_USB
	mxs_iomux_setup_pad(MX28_PAD_SSP2_SS1__USB1_OVERCURRENT);
	mxs_iomux_setup_pad(MX28_PAD_AUART2_RX__GPIO_3_8 |
			MXS_PAD_4MA | MXS_PAD_3V3 | MXS_PAD_NOPULL);
	gpio_direction_output(MX28_PAD_AUART2_RX__GPIO_3_8, 1);
#endif

	return 0;
}

int dram_init(void)
{
	return mx28_dram_init();
}

int board_init(void)
{
	/* Adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

#ifdef	CONFIG_CMD_MMC
static int mx28evk_mmc_wp(int id)
{
	if (id != 0) {
		printf("MXS MMC: Invalid card selected (card id = %d)\n", id);
		return 1;
	}

	return gpio_get_value(MX28_PAD_SSP1_SCK__GPIO_2_12);
}

int board_mmc_init(bd_t *bis)
{
	/* Configure WP as input */
	gpio_direction_input(MX28_PAD_SSP1_SCK__GPIO_2_12);

	/* Configure MMC0 Power Enable */
	gpio_direction_output(MX28_PAD_PWM3__GPIO_3_28, 0);

	return mxsmmc_initialize(bis, 0, mx28evk_mmc_wp);
}
#endif

#ifdef	CONFIG_CMD_NET

#define	MII_OPMODE_STRAP_OVERRIDE	0x16
#define	MII_PHY_CTRL1			0x1e
#define	MII_PHY_CTRL2			0x1f

int fecmxc_mii_postcall(int phy)
{
	miiphy_write("FEC1", phy, MII_BMCR, 0x9000);
	miiphy_write("FEC1", phy, MII_OPMODE_STRAP_OVERRIDE, 0x0202);
	if (phy == 3)
		miiphy_write("FEC1", 3, MII_PHY_CTRL2, 0x8180);
	return 0;
}

int board_eth_init(bd_t *bis)
{
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;
	struct eth_device *dev;
	int ret;

	ret = cpu_eth_init(bis);

	/* MX28EVK uses ENET_CLK PAD to drive FEC clock */
	writel(CLKCTRL_ENET_TIME_SEL_RMII_CLK | CLKCTRL_ENET_CLK_OUT_EN,
					&clkctrl_regs->hw_clkctrl_enet);

	/* Power-on FECs */
	gpio_direction_output(MX28_PAD_SSP1_DATA3__GPIO_2_15, 0);

	/* Reset FEC PHYs */
	gpio_direction_output(MX28_PAD_ENET0_RX_CLK__GPIO_4_13, 0);
	udelay(200);
	gpio_set_value(MX28_PAD_ENET0_RX_CLK__GPIO_4_13, 1);

	ret = fecmxc_initialize_multi(bis, 0, 0, MXS_ENET0_BASE);
	if (ret) {
		puts("FEC MXS: Unable to init FEC0\n");
		return ret;
	}

	ret = fecmxc_initialize_multi(bis, 1, 3, MXS_ENET1_BASE);
	if (ret) {
		puts("FEC MXS: Unable to init FEC1\n");
		return ret;
	}

	dev = eth_get_dev_by_name("FEC0");
	if (!dev) {
		puts("FEC MXS: Unable to get FEC0 device entry\n");
		return -EINVAL;
	}

	ret = fecmxc_register_mii_postcall(dev, fecmxc_mii_postcall);
	if (ret) {
		puts("FEC MXS: Unable to register FEC0 mii postcall\n");
		return ret;
	}

	dev = eth_get_dev_by_name("FEC1");
	if (!dev) {
		puts("FEC MXS: Unable to get FEC1 device entry\n");
		return -EINVAL;
	}

	ret = fecmxc_register_mii_postcall(dev, fecmxc_mii_postcall);
	if (ret) {
		puts("FEC MXS: Unable to register FEC1 mii postcall\n");
		return ret;
	}

	return ret;
}

#endif
