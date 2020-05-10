// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Toradex
 */

#include <common.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <miiphy.h>
#include <netdev.h>
#include <micrel.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	/* rom_pointer[1] contains the size of TEE occupies */
	if (rom_pointer[1])
		gd->ram_size = PHYS_SDRAM_SIZE - rom_pointer[1];
	else
		gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

#if IS_ENABLED(CONFIG_FEC_MXC)
static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	int tmp;

	switch (ksz9xx1_phy_get_id(phydev) & MII_KSZ9x31_SILICON_REV_MASK) {
	case PHY_ID_KSZ9031:
		/*
		 * The PHY adds 1.2ns for the RXC and 0ns for TXC clock by
		 * default. The MAC and the layout don't add a skew between
		 * clock and data.
		 * Add 0.3ns for the RXC path and 0.96 + 0.42 ns (1.38 ns) for
		 * the TXC path to get the required clock skews.
		 */
		/* control data pad skew - devaddr = 0x02, register = 0x04 */
		ksz9031_phy_extended_write(phydev, 0x02,
					   MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW,
					   MII_KSZ9031_MOD_DATA_NO_POST_INC,
					   0x0070);
		/* rx data pad skew - devaddr = 0x02, register = 0x05 */
		ksz9031_phy_extended_write(phydev, 0x02,
					   MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW,
					   MII_KSZ9031_MOD_DATA_NO_POST_INC,
					   0x7777);
		/* tx data pad skew - devaddr = 0x02, register = 0x06 */
		ksz9031_phy_extended_write(phydev, 0x02,
					   MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
					   MII_KSZ9031_MOD_DATA_NO_POST_INC,
					   0x0000);
		/* gtx and rx clock pad skew - devaddr = 0x02,register = 0x08 */
		ksz9031_phy_extended_write(phydev, 0x02,
					   MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
					   MII_KSZ9031_MOD_DATA_NO_POST_INC,
					   0x03f4);
		break;
	case PHY_ID_KSZ9131:
	default:
		/* read rxc dll control - devaddr = 0x2, register = 0x4c */
		tmp = ksz9031_phy_extended_read(phydev, 0x02,
					MII_KSZ9131_EXT_RGMII_2NS_SKEW_RXDLL,
					MII_KSZ9031_MOD_DATA_NO_POST_INC);
		/* disable rxdll bypass (enable 2ns skew delay on RXC) */
		tmp &= ~MII_KSZ9131_RXTXDLL_BYPASS;
		/* rxc data pad skew 2ns - devaddr = 0x02, register = 0x4c */
		tmp = ksz9031_phy_extended_write(phydev, 0x02,
					MII_KSZ9131_EXT_RGMII_2NS_SKEW_RXDLL,
					MII_KSZ9031_MOD_DATA_NO_POST_INC, tmp);
		/* read txc dll control - devaddr = 0x02, register = 0x4d */
		tmp = ksz9031_phy_extended_read(phydev, 0x02,
					MII_KSZ9131_EXT_RGMII_2NS_SKEW_TXDLL,
					MII_KSZ9031_MOD_DATA_NO_POST_INC);
		/* disable txdll bypass (enable 2ns skew delay on TXC) */
		tmp &= ~MII_KSZ9131_RXTXDLL_BYPASS;
		/* rxc data pad skew 2ns - devaddr = 0x02, register = 0x4d */
		tmp = ksz9031_phy_extended_write(phydev, 0x02,
					MII_KSZ9131_EXT_RGMII_2NS_SKEW_TXDLL,
					MII_KSZ9031_MOD_DATA_NO_POST_INC, tmp);
		break;
	}

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

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int board_late_init(void)
{
	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif
