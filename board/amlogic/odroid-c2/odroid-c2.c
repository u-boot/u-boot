/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/gxbb.h>
#include <asm/arch/sm.h>
#include <phy.h>

#define EFUSE_SN_OFFSET		20
#define EFUSE_SN_SIZE		16
#define EFUSE_MAC_OFFSET	52
#define EFUSE_MAC_SIZE		6

int board_init(void)
{
	return 0;
}

int misc_init_r(void)
{
	u8 mac_addr[EFUSE_MAC_SIZE];
	char serial[EFUSE_SN_SIZE];
	ssize_t len;

	/* Set RGMII mode */
	setbits_le32(GXBB_ETH_REG_0, GXBB_ETH_REG_0_PHY_INTF |
				     GXBB_ETH_REG_0_TX_PHASE(1) |
				     GXBB_ETH_REG_0_TX_RATIO(4) |
				     GXBB_ETH_REG_0_PHY_CLK_EN |
				     GXBB_ETH_REG_0_CLK_EN);

	/* Enable power and clock gate */
	setbits_le32(GXBB_GCLK_MPEG_1, GXBB_GCLK_MPEG_1_ETH);
	clrbits_le32(GXBB_MEM_PD_REG_0, GXBB_MEM_PD_REG_0_ETH_MASK);

	/* Reset PHY on GPIOZ_14 */
	clrbits_le32(GXBB_GPIO_EN(3), BIT(14));
	clrbits_le32(GXBB_GPIO_OUT(3), BIT(14));
	mdelay(10);
	setbits_le32(GXBB_GPIO_OUT(3), BIT(14));

	if (!eth_getenv_enetaddr("ethaddr", mac_addr)) {
		len = meson_sm_read_efuse(EFUSE_MAC_OFFSET,
					  mac_addr, EFUSE_MAC_SIZE);
		if (len == EFUSE_MAC_SIZE && is_valid_ethaddr(mac_addr))
			eth_setenv_enetaddr("ethaddr", mac_addr);
	}

	if (!getenv("serial#")) {
		len = meson_sm_read_efuse(EFUSE_SN_OFFSET, serial,
			EFUSE_SN_SIZE);
		if (len == EFUSE_SN_SIZE) 
			setenv("serial#", serial);
	}

	return 0;
}
