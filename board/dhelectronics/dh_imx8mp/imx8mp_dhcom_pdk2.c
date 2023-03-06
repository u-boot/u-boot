// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <dm.h>
#include <dt-bindings/clock/imx8mp-clock.h>
#include <env.h>
#include <env_internal.h>
#include <i2c_eeprom.h>
#include <linux/bitfield.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>

#include "lpddr4_timing.h"
#include "../common/dh_common.h"
#include "../common/dh_imx.h"

DECLARE_GLOBAL_DATA_PTR;

int mach_cpu_init(void)
{
	icache_enable();
	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	const u16 memsz[] = { 512, 1024, 1536, 2048, 3072, 4096, 6144, 8192 };
	u8 memcfg = dh_get_memcfg();

	*size = (u64)memsz[memcfg] << 20ULL;

	return 0;
}

static int dh_imx8_setup_ethaddr(void)
{
	unsigned char enetaddr[6];

	if (dh_mac_is_in_env("ethaddr"))
		return 0;

	if (!dh_imx_get_mac_from_fuse(enetaddr))
		goto out;

	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom0"))
		goto out;

	return -ENXIO;

out:
	return eth_env_set_enetaddr("ethaddr", enetaddr);
}

static int dh_imx8_setup_eth1addr(void)
{
	unsigned char enetaddr[6];

	if (dh_mac_is_in_env("eth1addr"))
		return 0;

	if (!dh_imx_get_mac_from_fuse(enetaddr))
		goto increment_out;

	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom1"))
		goto out;

	/*
	 * Populate second ethernet MAC from first ethernet EEPROM with MAC
	 * address LSByte incremented by 1. This is only used on SoMs without
	 * second ethernet EEPROM, i.e. early prototypes.
	 */
	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom0"))
		goto increment_out;

	return -ENXIO;

increment_out:
	enetaddr[5]++;

out:
	return eth_env_set_enetaddr("eth1addr", enetaddr);
}

int dh_setup_mac_address(void)
{
	int ret;

	ret = dh_imx8_setup_ethaddr();
	if (ret)
		printf("%s: Unable to setup ethaddr! ret = %d\n", __func__, ret);

	ret = dh_imx8_setup_eth1addr();
	if (ret)
		printf("%s: Unable to setup eth1addr! ret = %d\n", __func__, ret);

	return ret;
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	dh_setup_mac_address();
	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	return prio ? ENVL_UNKNOWN : ENVL_SPI_FLASH;
}

static const char *iomuxc_compat = "fsl,imx8mp-iomuxc";
static const char *lan_compat = "ethernet-phy-id0007.c110";
static const char *ksz_compat = "ethernet-phy-id0022.1642";

static int dh_dt_patch_som_eqos(const void *fdt_blob)
{
	const void __iomem *mux = (void __iomem *)IOMUXC_BASE_ADDR +
		FIELD_GET(MUX_CTRL_OFS_MASK, MX8MP_PAD_ENET_RX_CTL__GPIO1_IO24);
	int mac_node, mdio_node, iomuxc_node, ksz_node, lan_node, subnode;
	const char *mac_compat = "nxp,imx8mp-dwmac-eqos";
	void *blob = (void *)fdt_blob;
	const fdt32_t *clk_prop;
	bool is_gigabit;
	u32 handle;
	u32 clk[6];

	setbits_le32(mux, IOMUX_CONFIG_SION);
	is_gigabit = !(readl(GPIO1_BASE_ADDR) & BIT(24));
	clrbits_le32(mux, IOMUX_CONFIG_SION);

	/* Adjust EQoS node for Gigabit KSZ9131RNXI or Fast LAN8740Ai PHY */
	mac_node = fdt_node_offset_by_compatible(blob, -1, mac_compat);
	if (mac_node < 0)
		return 0;

	mdio_node = fdt_first_subnode(blob, mac_node);
	if (mdio_node < 0)
		return 0;

	/* KSZ9131RNXI */
	ksz_node = fdt_node_offset_by_compatible(blob, mdio_node, ksz_compat);
	if (ksz_node < 0)
		return 0;

	/* LAN8740Ai */
	lan_node = fdt_node_offset_by_compatible(blob, mdio_node, lan_compat);
	if (lan_node < 0)
		return 0;

	iomuxc_node = fdt_node_offset_by_compatible(blob, -1, iomuxc_compat);
	if (iomuxc_node < 0)
		return 0;

	/*
	 * The code below adjusts the following DT properties:
	 * - assigned-clock-parents .. 125 MHz RGMII / 50 MHz RMII ref clock
	 * - assigned-clock-rates .... 125 MHz RGMII / 50 MHz RMII ref clock
	 * - phy-handle .............. KSZ9131RNXI RGMII / LAN8740Ai RMII
	 * - phy-mode ................ RGMII / RMII
	 * - pinctrl-0 ............... RGMII / RMII
	 * - PHY subnode status ...... "disabled"/"okay" per RGMII / RMII
	 */

	/* Perform all inplace changes first, string changes last. */
	clk_prop = fdt_getprop(blob, mac_node, "assigned-clock-parents", NULL);
	if (!clk_prop)
		return 0;
	clk[0] = clk_prop[0];
	clk[1] = cpu_to_fdt32(IMX8MP_SYS_PLL1_266M);
	clk[2] = clk_prop[2];
	clk[3] = cpu_to_fdt32(IMX8MP_SYS_PLL2_100M);
	clk[4] = clk_prop[4];
	clk[5] = is_gigabit ? cpu_to_fdt32(IMX8MP_SYS_PLL2_125M) :
			      cpu_to_fdt32(IMX8MP_SYS_PLL2_50M);
	fdt_setprop_inplace(blob, mac_node, "assigned-clock-parents",
			    clk, 6 * sizeof(u32));

	clk[0] = cpu_to_fdt32(0);
	clk[1] = cpu_to_fdt32(100000000);
	clk[2] = is_gigabit ? cpu_to_fdt32(125000000) :
			      cpu_to_fdt32(50000000);
	fdt_setprop_inplace(blob, mac_node, "assigned-clock-rates",
			    clk, 3 * sizeof(u32));

	handle = fdt_get_phandle(blob, is_gigabit ? ksz_node : lan_node);
	fdt_setprop_inplace_u32(blob, mac_node, "phy-handle", handle);

	fdt_for_each_subnode(subnode, blob, iomuxc_node) {
		if (!strstr(fdt_get_name(blob, subnode, NULL),
			    is_gigabit ? "eqos-rgmii" : "eqos-rmii"))
			continue;

		handle = fdt_get_phandle(blob, subnode);
		fdt_setprop_inplace_u32(blob, mac_node, "pinctrl-0", handle);
		break;
	}

	fdt_setprop_string(blob, mac_node, "phy-mode",
			   is_gigabit ? "rgmii-id" : "rmii");

	mac_node = fdt_node_offset_by_compatible(blob, -1, mac_compat);
	mdio_node = fdt_first_subnode(blob, mac_node);
	ksz_node = fdt_node_offset_by_compatible(blob, mdio_node, ksz_compat);
	fdt_setprop_string(blob, ksz_node, "status",
			   is_gigabit ? "okay" : "disabled");

	mac_node = fdt_node_offset_by_compatible(blob, -1, mac_compat);
	mdio_node = fdt_first_subnode(blob, mac_node);
	lan_node = fdt_node_offset_by_compatible(blob, mdio_node, lan_compat);
	fdt_setprop_string(blob, lan_node, "status",
			   is_gigabit ? "disabled" : "okay");

	return 0;
}

static int dh_dt_patch_som_fec(const void *fdt_blob)
{
	const void __iomem *mux = (void __iomem *)IOMUXC_BASE_ADDR +
		FIELD_GET(MUX_CTRL_OFS_MASK, MX8MP_PAD_SAI1_TXFS__GPIO4_IO10);
	int mac_node, mdio_node, iomuxc_node, lan_node, phy_node, subnode;
	const char *mac_compat = "fsl,imx8mp-fec";
	void *blob = (void *)fdt_blob;
	const fdt32_t *clk_prop;
	bool is_gigabit;
	u32 handle;
	u32 clk[8];

	setbits_le32(mux, IOMUX_CONFIG_SION);
	is_gigabit = !(readl(GPIO4_BASE_ADDR) & BIT(10));
	clrbits_le32(mux, IOMUX_CONFIG_SION);

	/* Test for non-default SoM with 100/Full PHY attached to FEC */
	if (is_gigabit)
		return 0;

	/* Adjust FEC node for Fast LAN8740Ai PHY */
	mac_node = fdt_node_offset_by_compatible(blob, -1, mac_compat);
	if (mac_node < 0)
		return 0;

	/* Optional PHY pointed to by phy-handle, possibly on carrier board */
	phy_node = fdtdec_lookup_phandle(blob, mac_node, "phy-handle");
	if (phy_node > 0) {
		fdt_setprop_string(blob, phy_node, "status", "disabled");
		mac_node = fdt_node_offset_by_compatible(blob, -1, mac_compat);
	}

	mdio_node = fdt_first_subnode(blob, mac_node);
	if (mdio_node < 0)
		return 0;

	/* LAN8740Ai */
	lan_node = fdt_node_offset_by_compatible(blob, mdio_node, lan_compat);
	if (lan_node < 0)
		return 0;

	iomuxc_node = fdt_node_offset_by_compatible(blob, -1, iomuxc_compat);
	if (iomuxc_node < 0)
		return 0;

	/*
	 * The code below adjusts the following DT properties:
	 * - assigned-clock-parents .. 50 MHz RMII ref clock
	 * - assigned-clock-rates .... 50 MHz RMII ref clock
	 * - phy-handle .............. LAN8740Ai RMII
	 * - phy-mode ................ RMII
	 * - pinctrl-0 ............... RMII
	 * - PHY subnode status ...... "okay" for RMII PHY
	 */

	/* Perform all inplace changes first, string changes last. */
	clk_prop = fdt_getprop(blob, mac_node, "assigned-clock-parents", NULL);
	if (!clk_prop)
		return 0;
	clk[0] = clk_prop[0];
	clk[1] = cpu_to_fdt32(IMX8MP_SYS_PLL1_266M);
	clk[2] = clk_prop[2];
	clk[3] = cpu_to_fdt32(IMX8MP_SYS_PLL2_100M);
	clk[4] = clk_prop[4];
	clk[5] = cpu_to_fdt32(IMX8MP_SYS_PLL2_50M);
	clk[6] = clk_prop[6];
	clk[7] = cpu_to_fdt32(IMX8MP_SYS_PLL2_50M);
	fdt_setprop_inplace(blob, mac_node, "assigned-clock-parents",
			    clk, 8 * sizeof(u32));

	clk[0] = cpu_to_fdt32(0);
	clk[1] = cpu_to_fdt32(100000000);
	clk[2] = cpu_to_fdt32(50000000);
	clk[3] = cpu_to_fdt32(0);
	fdt_setprop_inplace(blob, mac_node, "assigned-clock-rates",
			    clk, 4 * sizeof(u32));

	handle = fdt_get_phandle(blob, lan_node);
	fdt_setprop_inplace_u32(blob, mac_node, "phy-handle", handle);

	fdt_for_each_subnode(subnode, blob, iomuxc_node) {
		if (!strstr(fdt_get_name(blob, subnode, NULL), "fec-rmii"))
			continue;

		handle = fdt_get_phandle(blob, subnode);
		fdt_setprop_inplace_u32(blob, mac_node, "pinctrl-0", handle);
		break;
	}

	fdt_setprop_string(blob, mac_node, "phy-mode", "rmii");
	mac_node = fdt_node_offset_by_compatible(blob, -1, mac_compat);
	mdio_node = fdt_first_subnode(blob, mac_node);
	lan_node = fdt_node_offset_by_compatible(blob, mdio_node, lan_compat);
	fdt_setprop_string(blob, lan_node, "status", "okay");

	return 0;
}

static int dh_dt_patch_som(const void *fdt_blob)
{
	int ret;

	/* Do nothing if not i.MX8MP DHCOM SoM */
	ret = fdt_node_check_compatible(fdt_blob, 0, "dh,imx8mp-dhcom-som");
	if (ret)
		return 0;

	ret = dh_dt_patch_som_eqos(fdt_blob);
	if (ret)
		return ret;

	return dh_dt_patch_som_fec(fdt_blob);
}

int fdtdec_board_setup(const void *fdt_blob)
{
	return dh_dt_patch_som(fdt_blob);
}
