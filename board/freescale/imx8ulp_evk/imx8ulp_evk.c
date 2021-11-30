// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/imx8ulp-pins.h>
#include <asm/arch/clock.h>
#include <asm/arch/pcc.h>
#include <asm/arch/sys_proto.h>
#include <miiphy.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_FEC_MXC)
#define ENET_CLK_PAD_CTRL	(PAD_CTL_PUS_UP | PAD_CTL_DSE | PAD_CTL_IBE_ENABLE)
static iomux_cfg_t const enet_clk_pads[] = {
	IMX8ULP_PAD_PTE19__ENET0_REFCLK | MUX_PAD_CTRL(ENET_CLK_PAD_CTRL),
	IMX8ULP_PAD_PTF10__ENET0_1588_CLKIN | MUX_PAD_CTRL(ENET_CLK_PAD_CTRL),
};

static int setup_fec(void)
{
	/*
	 * Since ref clock and timestamp clock are from external,
	 * set the iomux prior the clock enablement
	 */
	imx8ulp_iomux_setup_multiple_pads(enet_clk_pads, ARRAY_SIZE(enet_clk_pads));

	/* Select enet time stamp clock: 001 - External Timestamp Clock */
	cgc1_enet_stamp_sel(1);

	/* enable FEC PCC */
	pcc_clock_enable(4, ENET_PCC4_SLOT, true);
	pcc_reset_peripheral(4, ENET_PCC4_SLOT, false);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}
#endif

int board_init(void)
{
	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();

	return 0;
}

int board_early_init_f(void)
{
	return 0;
}

int board_late_init(void)
{
	return 0;
}
