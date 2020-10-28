// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Toradex
 */

#include <common.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <micrel.h>

#include "../common/tdx-cfg-block.h"

DECLARE_GLOBAL_DATA_PTR;

#define I2C_PMIC	0

enum pcb_rev_t {
	PCB_VERSION_1_0,
	PCB_VERSION_1_1
};

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

static enum pcb_rev_t get_pcb_revision(void)
{
	struct udevice *bus;
	struct udevice *i2c_dev = NULL;
	int ret;
	u8 is_bd71837 = 0;

	ret = uclass_get_device_by_seq(UCLASS_I2C, I2C_PMIC, &bus);
	if (!ret)
		ret = dm_i2c_probe(bus, 0x4b, 0, &i2c_dev);
	if (!ret)
		ret = dm_i2c_read(i2c_dev, 0x0, &is_bd71837, 1);

	/* BD71837_REV, High Nibble is major version, fix 1010 */
	is_bd71837 = !ret && ((is_bd71837 & 0xf0) == 0xa0);
	return is_bd71837 ? PCB_VERSION_1_0 : PCB_VERSION_1_1;
}

static void select_dt_from_module_version(void)
{
	char variant[32];
	char *env_variant = env_get("variant");
	int is_wifi = 0;

	if (IS_ENABLED(CONFIG_TDX_CFG_BLOCK)) {
		/*
		 * If we have a valid config block and it says we are a
		 * module with Wi-Fi/Bluetooth make sure we use the -wifi
		 * device tree.
		 */
		is_wifi = (tdx_hw_tag.prodid == VERDIN_IMX8MMQ_WIFI_BT_IT) ||
			  (tdx_hw_tag.prodid == VERDIN_IMX8MMDL_WIFI_BT_IT);
	}

	switch (get_pcb_revision()) {
	case PCB_VERSION_1_0:
		printf("Detected a V1.0 module\n");
		if (is_wifi)
			strncpy(&variant[0], "wifi", sizeof(variant));
		else
			strncpy(&variant[0], "nonwifi", sizeof(variant));
		break;
	default:
		if (is_wifi)
			strncpy(&variant[0], "wifi-v1.1", sizeof(variant));
		else
			strncpy(&variant[0], "nonwifi-v1.1", sizeof(variant));
		break;
	}

	if (strcmp(variant, env_variant)) {
		printf("Setting variant to %s\n", variant);
		env_set("variant", variant);

		if (IS_ENABLED(CONFIG_ENV_IS_NOWHERE))
			env_save();
	}
}

int board_late_init(void)
{
	select_dt_from_module_version();

	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	if (!size)
		return -EINVAL;

	*size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif
