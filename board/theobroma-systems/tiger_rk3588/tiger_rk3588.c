// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023 Theobroma Systems Design und Consulting GmbH
 */

#include <phy.h>
#include <eth_phy.h>

#include <asm/types.h>
#include <asm/arch-rockchip/cru_rk3588.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/ioc_rk3588.h>
#include <asm-generic/u-boot.h>
#include <dm/device.h>
#include <dm/uclass-id.h>
#include <linux/bitfield.h>

#include "../common/common.h"

#define GPIO2C3_SEL_MASK		GENMASK(15, 12)
#define GPIO2C3_ETH0_REFCLKO_25M	FIELD_PREP(GPIO2C3_SEL_MASK, 1)

#define REFCLKO25M_ETH0_OUT_SEL_MASK	BIT(15)
#define REFCLKO25M_ETH0_OUT_SEL_CPLL	FIELD_PREP(REFCLKO25M_ETH0_OUT_SEL_MASK, 1)
#define REFCLKO25M_ETH0_OUT_DIV_MASK	GENMASK(14, 8)
#define REFCLKO25M_ETH0_OUT_DIV(x)	FIELD_PREP(REFCLKO25M_ETH0_OUT_DIV_MASK, (x) - 1)

#define REFCLKO25M_ETH0_OUT_EN		BIT(4)

void setup_eth0refclko(void)
{
	/* Configure and enable ETH0_REFCLKO_25MHz */
	static struct rk3588_bus_ioc * const bus_ioc = (void *)BUS_IOC_BASE;
	static struct rk3588_cru * const cru = (void *)CRU_BASE;

	/* 1. Pinmux */
	rk_clrsetreg(&bus_ioc->gpio2c_iomux_sel_l, GPIO2C3_SEL_MASK, GPIO2C3_ETH0_REFCLKO_25M);
	/* 2. Parent clock selection + divider => CPLL (1.5GHz) / 60 => 25MHz */
	rk_clrsetreg(&cru->clksel_con[15],
		     REFCLKO25M_ETH0_OUT_SEL_MASK | REFCLKO25M_ETH0_OUT_DIV_MASK,
		     REFCLKO25M_ETH0_OUT_SEL_CPLL | REFCLKO25M_ETH0_OUT_DIV(60));
	/* 3. Enable clock */
	rk_clrreg(&cru->clkgate_con[5], REFCLKO25M_ETH0_OUT_EN);
}

int rockchip_early_misc_init_r(void)
{
	setup_boottargets();

	setup_eth0refclko();

	return 0;
}
