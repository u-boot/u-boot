// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 MediaTek Inc. All Rights Reserved.
 *
 * Author: Sky Huang <SkyLake.Huang@mediatek.com>
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/of_access.h>
#include <dm/pinctrl.h>
#include <dm/ofnode.h>
#include <fw_loader.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <phy.h>
#include "mtk.h"

#define MTK_2P5GPHY_ID_MT7987			0x00339c91
#define MTK_2P5GPHY_ID_MT7988			0x00339c11

#define PBUS_BASE				0x0f000000
#define PBUS_SIZE				0x1f0024

#define MTK_2P5GPHY_PMD_REG			0x010000
#define DO_NOT_RESET				(MTK_2P5GPHY_PMD_REG + 0x28)
#define   DO_NOT_RESET_XBZ			BIT(0)
#define   DO_NOT_RESET_PMA			BIT(3)
#define   DO_NOT_RESET_RX			BIT(5)
#define FNPLL_PWR_CTRL1				(MTK_2P5GPHY_PMD_REG + 0x208)
#define   RG_SPEED_MASK				GENMASK(3, 0)
#define   RG_SPEED_2500				BIT(3)
#define   RG_SPEED_100				BIT(0)
#define FNPLL_PWR_CTRL_STATUS			(MTK_2P5GPHY_PMD_REG + 0x20c)
#define   RG_STABLE_MASK			GENMASK(3, 0)
#define   RG_SPEED_2500_STABLE			BIT(3)
#define   RG_SPEED_100_STABLE			BIT(0)

#define MTK_2P5GPHY_XBZ_PCS			0x030000
#define PHY_CTRL_CONFIG				(MTK_2P5GPHY_XBZ_PCS + 0x200)
#define PMU_WP					(MTK_2P5GPHY_XBZ_PCS + 0x800)
#define   WRITE_PROTECT_KEY			0xCAFEF00D
#define PMU_PMA_AUTO_CFG			(MTK_2P5GPHY_XBZ_PCS + 0x820)
#define   POWER_ON_AUTO_MODE			BIT(16)
#define   PMU_AUTO_MODE_EN			BIT(0)
#define PMU_PMA_STATUS				(MTK_2P5GPHY_XBZ_PCS + 0x840)
#define   CLK_IS_DISABLED			BIT(3)

#define MTK_2P5GPHY_XBZ_PMA_RX			0x080000
#define SMEM_WDAT0				(MTK_2P5GPHY_XBZ_PMA_RX + 0x5000)
#define SMEM_WDAT1				(MTK_2P5GPHY_XBZ_PMA_RX + 0x5004)
#define SMEM_WDAT2				(MTK_2P5GPHY_XBZ_PMA_RX + 0x5008)
#define SMEM_WDAT3				(MTK_2P5GPHY_XBZ_PMA_RX + 0x500c)
#define SMEM_CTRL				(MTK_2P5GPHY_XBZ_PMA_RX + 0x5024)
#define   SMEM_HW_RDATA_ZERO			BIT(24)
#define SMEM_ADDR_REF_ADDR			(MTK_2P5GPHY_XBZ_PMA_RX + 0x502c)
#define CM_CTRL_P01				(MTK_2P5GPHY_XBZ_PMA_RX + 0x5100)
#define CM_CTRL_P23				(MTK_2P5GPHY_XBZ_PMA_RX + 0x5124)
#define DM_CTRL_P01				(MTK_2P5GPHY_XBZ_PMA_RX + 0x5200)
#define DM_CTRL_P23				(MTK_2P5GPHY_XBZ_PMA_RX + 0x5224)

#define MTK_2P5GPHY_CHIP_SCU			0x0cf800
#define SYS_SW_RESET				(MTK_2P5GPHY_CHIP_SCU + 0x128)
#define   RESET_RST_CNT				BIT(0)

#define MTK_2P5GPHY_MCU_CSR			0x0f0000
#define MD32_EN_CFG				(MTK_2P5GPHY_MCU_CSR + 0x18)
#define   MD32_EN				BIT(0)

#define MTK_2P5GPHY_PMB_FW			0x100000

#define MTK_2P5GPHY_FCM_BASE			0x0e0000
#define FC_LWM					(MTK_2P5GPHY_FCM_BASE + 0x14)
#define   TX_FC_LWM_MASK			GENMASK(31, 16)
#define MIN_IPG_NUM				(MTK_2P5GPHY_FCM_BASE + 0x2c)
#define   LS_MIN_IPG_NUM_MASK			GENMASK(7, 0)
#define FIFO_CTRL				(MTK_2P5GPHY_FCM_BASE + 0x40)
#define   TX_SFIFO_IDLE_CNT_MASK		GENMASK(31, 28)
#define   TX_SFIFO_DEL_IPG_WM_MASK		GENMASK(23, 16)

#define MTK_2P5GPHY_APB_BASE			0x11c30000
#define MTK_2P5GPHY_APB_SIZE			0x9c
#define SW_RESET				0x94
#define   MD32_RESTART_EN_CLEAR			BIT(9)

/* Registers on CL22 page 0 */
#define PHY_AUX_CTRL_STATUS			0x1d
#define   PHY_AUX_DPX_MASK			GENMASK(5, 5)
#define   PHY_AUX_SPEED_MASK			GENMASK(4, 2)

enum {
	PHY_AUX_SPD_10 = 0,
	PHY_AUX_SPD_100,
	PHY_AUX_SPD_1000,
	PHY_AUX_SPD_2500,
};

/* Registers on MDIO_MMD_VEND1 */
#define MTK_PHY_LINK_STATUS_RELATED		0x147
#define   MTK_PHY_BYPASS_LINK_STATUS_OK		BIT(4)
#define   MTK_PHY_FORCE_LINK_STATUS_HCD		BIT(3)

#define MTK_PHY_AN_FORCE_SPEED_REG		0x313
#define   MTK_PHY_MASTER_FORCE_SPEED_SEL_EN	BIT(7)
#define   MTK_PHY_MASTER_FORCE_SPEED_SEL_MASK	GENMASK(6, 0)

#define MTK_PHY_LPI_PCS_DSP_CTRL		0x121
#define   MTK_PHY_LPI_SIG_EN_LO_THRESH100_MASK	GENMASK(12, 8)

#define MTK_PHY_PMA_PMD_SPEED_ABILITY		0x300
#define   CAP_100X_HDX				BIT(14)
#define   CAP_10T_HDX				BIT(12)

/* Registers on MDIO_MMD_VEND2 */
#define MT7987_OPTIONS				0x110
#define   NORMAL_RETRAIN_DISABLE		BIT(0)

/* Registers on Token Ring debug nodes */
/* ch_addr = 0x0, node_addr = 0xf, data_addr = 0x3c */
#define AUTO_NP_10XEN				BIT(6)

/* Firmware file size */
#define MT7987_2P5GE_PMB_FW_SIZE		0x18000
#define MT7988_2P5GE_PMB_FW_SIZE		0x20000

#define MT7987_2P5GE_DSPBITTB_SIZE		0x7000

struct mtk_i2p5ge_fw_info {
	u8 datecode[4];
	u8 plat[2];
	u8 ver[2];
};

struct mtk_i2p5ge_priv {
	void __iomem *reg_base;
};

static inline void pbus_write(struct mtk_i2p5ge_priv *priv, u32 offset,
			      u32 val)
{
	writel(val, priv->reg_base + offset);
}

static inline void pbus_rmw(struct mtk_i2p5ge_priv *priv, u32 offset, u32 clr,
			    u32 set)
{
	clrsetbits_le32(priv->reg_base + offset, clr, set);
}

static int mt798x_i2p5ge_download_fw(struct mtk_i2p5ge_priv *priv,
				     size_t fwsize, const void *fwdata)
{
	u32 __iomem *fwmem = priv->reg_base + MTK_2P5GPHY_PMB_FW;
	const u32 *fw;
	u32 i;

	/* Assume fw data is 4-byte aligned */
	fw = fwdata;

	for (i = 0; i < (fwsize >> 2); i++)
		writel(fw[i], &fwmem[i]);

	return 0;
}

static int mt798x_i2p5ge_phy_probe(struct phy_device *phydev)
{
	struct mtk_i2p5ge_priv *priv;

	priv = malloc(sizeof(*priv));
	if (!priv)
		return -ENOMEM;

	priv->reg_base = ioremap(PBUS_BASE, PBUS_SIZE);
	if (!priv->reg_base) {
		free(priv);
		return -ENODEV;
	}

	phydev->priv = priv;

	return 0;
}

static int mt798x_i2p5ge_phy_config(struct phy_device *phydev)
{

	/* Setup LED */
	phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, MTK_PHY_LED0_ON_CTRL,
			 MTK_PHY_LED_ON_LINK10 |
			 MTK_PHY_LED_ON_LINK100 |
			 MTK_PHY_LED_ON_LINK1000 |
			 MTK_PHY_LED_ON_LINK2500);
	phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, MTK_PHY_LED1_ON_CTRL,
			 MTK_PHY_LED_ON_FDX | MTK_PHY_LED_ON_HDX);

	/* Switch pinctrl after setting polarity to avoid bogus blinking */
	pinctrl_select_state(phydev->dev, "i2p5gbe-led");

	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_LPI_PCS_DSP_CTRL,
		       MTK_PHY_LPI_SIG_EN_LO_THRESH100_MASK, 0);

	/* Enable 16-bit next page exchange bit if 1000-BT isn't advertising */
	mtk_tr_modify(phydev, 0x0, 0xf, 0x3c, AUTO_NP_10XEN,
		      FIELD_PREP(AUTO_NP_10XEN, 0x1));

	/* Set HW auto downshift */
	mtk_phy_select_page(phydev, MTK_PHY_PAGE_EXTENDED_1);
	phy_set_bits_mmd(phydev, MDIO_MMD_VEND1,
			 MTK_PHY_AUX_CTRL_AND_STATUS,
			 MTK_PHY_ENABLE_DOWNSHIFT);
	mtk_phy_restore_page(phydev);

	/* Configure parallel detction functionality */
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1,
			   MTK_PHY_PMA_PMD_SPEED_ABILITY,
			   CAP_10T_HDX | CAP_100X_HDX);
	phy_clear_bits_mmd(phydev, MDIO_DEVAD_NONE,
			   MII_ADVERTISE,
			   ADVERTISE_10HALF | ADVERTISE_100HALF);

	return 0;
}

static void mt798x_i2p5ge_print_fw_info(const void *fwdata, size_t fwsize)
{
	const struct mtk_i2p5ge_fw_info *info;
	u8 ver_minor, ver_patch;

	info = (const void *)((uintptr_t)fwdata + fwsize - sizeof(*info));

	ver_minor = info->ver[1] >> 4;
	ver_patch = info->ver[1] & 0xf;

	printf("Firmware loaded, date %02x%02x/%02x/%02x, ver %x.%u.%u\n",
	       info->datecode[0], info->datecode[1], info->datecode[2],
	       info->datecode[3], info->ver[0], ver_minor, ver_patch);
}

int __weak mt7987_i2p5ge_get_fw(void **fw, size_t *fwsize,
				void **dspfw, size_t *dspfwsize)
{
	void *pmb, *dsp;
	int ret;

	pmb = malloc(MT7987_2P5GE_PMB_FW_SIZE);
	if (!pmb)
		return -ENOMEM;

	ret = request_firmware_into_buf_via_script(
		pmb, MT7987_2P5GE_PMB_FW_SIZE,
		"mt7987_i2p5ge_load_pmb_firmware", fwsize);
	if (ret) {
		free(pmb);
		return ret;
	}

	dsp = malloc(MT7987_2P5GE_DSPBITTB_SIZE);
	if (!dsp) {
		free(pmb);
		return -ENOMEM;
	}

	ret = request_firmware_into_buf_via_script(
		dsp, MT7987_2P5GE_DSPBITTB_SIZE,
		"mt7987_i2p5ge_load_dspbit_firmware", dspfwsize);
	if (ret) {
		free(pmb);
		free(dsp);
		return ret;
	}

	*fw = pmb;
	*dspfw = dsp;

	return 1;
}

static int mt7987_i2p5ge_download_dspfw(struct mtk_i2p5ge_priv *priv,
					const void *dspfw_data)
{
	const u32 *dspfw;
	u32 i;

	pbus_rmw(priv, SMEM_CTRL, 0, SMEM_HW_RDATA_ZERO);

	pbus_rmw(priv, PHY_CTRL_CONFIG, 0, BIT(16));

	/* Initialize data memory */
	pbus_rmw(priv, DM_CTRL_P01, 0, BIT(28));
	pbus_rmw(priv, DM_CTRL_P23, 0, BIT(28));

	/* Initialize coefficient memory */
	pbus_rmw(priv, CM_CTRL_P01, 0, BIT(28));
	pbus_rmw(priv, CM_CTRL_P23, 0, BIT(28));

	/* Initialize PM offset */
	pbus_write(priv, SMEM_ADDR_REF_ADDR, 0);

	/* Assume DSP bit data is 4-byte aligned */
	dspfw = dspfw_data;

	for (i = 0; i < (MT7987_2P5GE_DSPBITTB_SIZE >> 2); i += 4) {
		pbus_write(priv, SMEM_WDAT0, dspfw[i]);
		pbus_write(priv, SMEM_WDAT1, dspfw[i + 1]);
		pbus_write(priv, SMEM_WDAT2, dspfw[i + 2]);
		pbus_write(priv, SMEM_WDAT3, dspfw[i + 3]);
	}

	pbus_rmw(priv, DM_CTRL_P01, BIT(28), 0);
	pbus_rmw(priv, DM_CTRL_P23, BIT(28), 0);

	pbus_rmw(priv, CM_CTRL_P01, BIT(28), 0);
	pbus_rmw(priv, CM_CTRL_P23, BIT(28), 0);

	return 0;
}

static int mt7987_i2p5ge_phy_load_fw(struct phy_device *phydev)
{
	struct mtk_i2p5ge_priv *priv = phydev->priv;
	size_t fw_size, dspfw_size;
	void __iomem *apb_base;
	void *fw, *dspfw;
	int ret, fwrc;
	u32 reg;

	apb_base = ioremap(MTK_2P5GPHY_APB_BASE, MTK_2P5GPHY_APB_SIZE);
	if (!apb_base)
		return -ENODEV;

	fwrc = mt7987_i2p5ge_get_fw(&fw, &fw_size, &dspfw, &dspfw_size);
	if (fwrc < 0) {
		dev_err(phydev->dev, "Failed to get firmware data\n");
		return -EINVAL;
	}

	if (fw_size != MT7987_2P5GE_PMB_FW_SIZE) {
		dev_err(phydev->dev,
			"PMB firmware size mismatch (0x%zx != 0x%x)\n",
			fw_size, MT7987_2P5GE_PMB_FW_SIZE);
		ret = -EINVAL;
		goto cleanup;
	}

	if (dspfw_size != MT7987_2P5GE_DSPBITTB_SIZE) {
		dev_err(phydev->dev,
			"DSP code size mismatch (0x%zx != 0x%x)\n",
			dspfw_size, MT7987_2P5GE_DSPBITTB_SIZE);
		ret = -EINVAL;
		goto cleanup;
	}

	/* Force 2.5Gphy back to AN state */
	phy_set_bits_mmd(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);
	mdelay(5);
	phy_set_bits_mmd(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_PDOWN);

	clrbits_le16(apb_base + SW_RESET, MD32_RESTART_EN_CLEAR);
	setbits_le16(apb_base + SW_RESET, MD32_RESTART_EN_CLEAR);
	clrbits_le16(apb_base + SW_RESET, MD32_RESTART_EN_CLEAR);

	pbus_rmw(priv, MD32_EN_CFG, MD32_EN, 0);

	ret = mt798x_i2p5ge_download_fw(priv, MT7987_2P5GE_PMB_FW_SIZE, fw);
	if (ret)
		goto cleanup;

	/* Enable 100Mbps module clock. */
	pbus_rmw(priv, FNPLL_PWR_CTRL1, RG_SPEED_MASK, RG_SPEED_100);

	/* Check if 100Mbps module clock is ready. */
	ret = readl_poll_timeout(priv->reg_base + FNPLL_PWR_CTRL_STATUS, reg,
				 reg & RG_SPEED_100_STABLE, 10000);
	if (ret) {
		dev_err(phydev->dev,
			"Timed out enabling 100Mbps module clock\n");
	}

	/* Enable 2.5Gbps module clock. */
	pbus_rmw(priv, FNPLL_PWR_CTRL1, RG_SPEED_MASK, RG_SPEED_2500);

	/* Check if 2.5Gbps module clock is ready. */
	ret = readl_poll_timeout(priv->reg_base + FNPLL_PWR_CTRL_STATUS, reg,
				 reg & RG_SPEED_2500_STABLE, 10000);

	if (ret) {
		dev_err(phydev->dev,
			"Timed out enabling 2.5Gbps module clock\n");
	}

	/* Disable AN */
	phy_clear_bits_mmd(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_ANENABLE);

	/* Force to run at 2.5G speed */
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_AN_FORCE_SPEED_REG,
		       MTK_PHY_MASTER_FORCE_SPEED_SEL_MASK,
		       MTK_PHY_MASTER_FORCE_SPEED_SEL_EN |
		       FIELD_PREP(MTK_PHY_MASTER_FORCE_SPEED_SEL_MASK, 0x1b));

	phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_LINK_STATUS_RELATED,
			 MTK_PHY_BYPASS_LINK_STATUS_OK |
			 MTK_PHY_FORCE_LINK_STATUS_HCD);

	/* Set xbz, pma and rx as "do not reset" in order to input DSP code. */
	pbus_rmw(priv, DO_NOT_RESET, 0,
		 DO_NOT_RESET_XBZ | DO_NOT_RESET_PMA | DO_NOT_RESET_RX);

	pbus_rmw(priv, SYS_SW_RESET, RESET_RST_CNT, 0);

	pbus_write(priv, PMU_WP, WRITE_PROTECT_KEY);

	pbus_rmw(priv, PMU_PMA_AUTO_CFG, 0,
		 PMU_AUTO_MODE_EN | POWER_ON_AUTO_MODE);

	/* Check if clock in auto mode is disabled. */
	ret = readl_poll_timeout(priv->reg_base + PMU_PMA_STATUS, reg,
				 (reg & CLK_IS_DISABLED) == 0x0, 100000);
	if (ret)
		dev_err(phydev->dev, "Timed out enabling clock auto mode\n");

	ret = mt7987_i2p5ge_download_dspfw(priv, dspfw);
	if (ret)
		goto cleanup;

	pbus_rmw(priv, MD32_EN_CFG, 0, MD32_EN);

	phy_set_bits_mmd(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

	/* We need a delay here to stabilize initialization of MCU */
	mdelay(8);

	mt798x_i2p5ge_print_fw_info(fw, fw_size);

cleanup:
	if (fwrc > 0) {
		free(fw);
		free(dspfw);
	}

	iounmap(apb_base);

	return ret;
}

static int mt7987_i2p5ge_phy_config(struct phy_device *phydev)
{
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND2, MTK_PHY_LED0_ON_CTRL,
			   MTK_PHY_LED_ON_POLARITY);

	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND2, MT7987_OPTIONS,
			   NORMAL_RETRAIN_DISABLE);

	return mt798x_i2p5ge_phy_config(phydev);
}

static int mt7987_i2p5ge_phy_probe(struct phy_device *phydev)
{
	int ret;

	ret = mt798x_i2p5ge_phy_probe(phydev);
	if (ret)
		return ret;

	return mt7987_i2p5ge_phy_load_fw(phydev);
}

int __weak mt7988_i2p5ge_get_fw(void **fw, size_t *size)
{
	void *pmb;
	int ret;

	pmb = malloc(MT7988_2P5GE_PMB_FW_SIZE);
	if (!pmb)
		return -ENOMEM;

	ret = request_firmware_into_buf_via_script(
		pmb, MT7988_2P5GE_PMB_FW_SIZE,
		"mt7988_i2p5ge_load_pmb_firmware", size);
	if (ret) {
		free(pmb);
		return ret;
	}

	*fw = pmb;

	return 1;
}

static int mt7988_i2p5ge_phy_load_fw(struct phy_device *phydev)
{
	struct mtk_i2p5ge_priv *priv = phydev->priv;
	size_t fw_size;
	int ret, fwrc;
	void *fw;

	fwrc = mt7988_i2p5ge_get_fw(&fw, &fw_size);
	if (fwrc < 0) {
		dev_err(phydev->dev, "Failed to get firmware data\n");
		return -EINVAL;
	}

	if (fw_size != MT7988_2P5GE_PMB_FW_SIZE) {
		dev_err(phydev->dev, "Firmware size mismatch (0x%zx != 0x%x)\n",
			fw_size, MT7988_2P5GE_PMB_FW_SIZE);
		ret = -EINVAL;
		goto cleanup;
	}

	ret = mt798x_i2p5ge_download_fw(priv, MT7988_2P5GE_PMB_FW_SIZE, fw);
	if (ret)
		goto cleanup;

	pbus_rmw(priv, MD32_EN_CFG, MD32_EN, 0);
	pbus_rmw(priv, MD32_EN_CFG, 0, MD32_EN);

	phy_set_bits_mmd(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

	/* We need a delay here to stabilize initialization of MCU */
	mdelay(8);

	mt798x_i2p5ge_print_fw_info(fw, fw_size);

cleanup:
	if (fwrc > 0)
		free(fw);

	return ret;
}

static int mt7988_i2p5ge_phy_config(struct phy_device *phydev)
{
	phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, MTK_PHY_LED0_ON_CTRL,
			 MTK_PHY_LED_ON_POLARITY);

	return mt798x_i2p5ge_phy_config(phydev);
}

static int mt7988_i2p5ge_phy_probe(struct phy_device *phydev)
{
	int ret;

	ret = mt798x_i2p5ge_phy_probe(phydev);
	if (ret)
		return ret;

	return mt7988_i2p5ge_phy_load_fw(phydev);
}

static int mt798x_i2p5ge_phy_startup(struct phy_device *phydev)
{
	struct mtk_i2p5ge_priv *priv = phydev->priv;
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	/* Initialize speed/duplex */
	phydev->speed = SPEED_10;
	phydev->duplex = DUPLEX_HALF;

	if (phydev->link) {
		ret = phy_read(phydev, MDIO_DEVAD_NONE, PHY_AUX_CTRL_STATUS);
		if (ret < 0)
			return ret;

		switch (FIELD_GET(PHY_AUX_SPEED_MASK, ret)) {
		case PHY_AUX_SPD_10:
			phydev->speed = SPEED_10;
			break;

		case PHY_AUX_SPD_100:
			phydev->speed = SPEED_100;
			break;

		case PHY_AUX_SPD_1000:
			pbus_rmw(priv, FIFO_CTRL,
				 TX_SFIFO_IDLE_CNT_MASK | TX_SFIFO_DEL_IPG_WM_MASK,
				 FIELD_PREP(TX_SFIFO_IDLE_CNT_MASK, 0x1) |
				 FIELD_PREP(TX_SFIFO_DEL_IPG_WM_MASK, 0x10));
			pbus_rmw(priv, MIN_IPG_NUM, LS_MIN_IPG_NUM_MASK,
				 FIELD_PREP(LS_MIN_IPG_NUM_MASK, 0xa));
			pbus_rmw(priv, FC_LWM, TX_FC_LWM_MASK,
				 FIELD_PREP(TX_FC_LWM_MASK, 0x340));
			phydev->speed = SPEED_1000;
			break;

		case PHY_AUX_SPD_2500:
			phydev->speed = SPEED_2500;
			break;

		default:
			break;
		}

		/* This PHY always operates in full duplex */
		phydev->duplex = DUPLEX_FULL;
	}

	return 0;
}

U_BOOT_PHY_DRIVER(mt7987_i2p5ge) = {
	.name = "MediaTek MT7987 built-in 2.5GbE PHY",
	.uid = MTK_2P5GPHY_ID_MT7987,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PCS | MDIO_MMD_AN | MDIO_MMD_VEND1 | MDIO_MMD_VEND2),
	.probe = &mt7987_i2p5ge_phy_probe,
	.config = &mt7987_i2p5ge_phy_config,
	.startup = &mt798x_i2p5ge_phy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(mt7988_i2p5ge) = {
	.name = "MediaTek MT7988 built-in 2.5GbE PHY",
	.uid = MTK_2P5GPHY_ID_MT7988,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PCS | MDIO_MMD_AN | MDIO_MMD_VEND1 | MDIO_MMD_VEND2),
	.probe = &mt7988_i2p5ge_phy_probe,
	.config = &mt7988_i2p5ge_phy_config,
	.startup = &mt798x_i2p5ge_phy_startup,
	.shutdown = &genphy_shutdown,
};
