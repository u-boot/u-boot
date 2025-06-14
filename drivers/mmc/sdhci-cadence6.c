// SPDX-License-Identifier: GPL-2.0-or-platform_driver
/*
 * Copyright (C) 2023 Starfive.
 *   Author: Kuan Lim Lee <kuanlim.lee@starfivetech.com>
 */

#include <dm.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>
#include <linux/libfdt.h>
#include <mmc.h>
#include <sdhci.h>
#include "sdhci-cadence.h"

/* IO Delay Information */
#define SDHCI_CDNS_HRS07		0x1C
#define   SDHCI_CDNS_HRS07_RW_COMPENSATE	GENMASK(20, 16)
#define   SDHCI_CDNS_HRS07_IDELAY_VAL		GENMASK(4, 0)

/* PHY Control and Status */
#define SDHCI_CDNS_HRS09		0x24
#define   SDHCI_CDNS_HRS09_RDDATA_EN		BIT(16)
#define   SDHCI_CDNS_HRS09_RDCMD_EN		BIT(15)
#define   SDHCI_CDNS_HRS09_EXTENDED_WR_MODE	BIT(3)
#define   SDHCI_CDNS_HRS09_EXTENDED_RD_MODE	BIT(2)
#define   SDHCI_CDNS_HRS09_PHY_INIT_COMPLETE	BIT(1)
#define   SDHCI_CDNS_HRS09_PHY_SW_RESET		BIT(0)

/* SDCLK adjustment */
#define SDHCI_CDNS_HRS10		0x28
#define   SDHCI_CDNS_HRS10_HCSDCLKADJ		GENMASK(19, 16)

/* CMD/DAT output delay */
#define SDHCI_CDNS_HRS16		0x40

/* PHY Special Function Registers */
/* register to control the DQ related timing */
#define PHY_DQ_TIMING_REG_ADDR		0x2000

/* register to control the DQS related timing */
#define PHY_DQS_TIMING_REG_ADDR		0x2004

/* register to control the gate and loopback control related timing */
#define PHY_GATE_LPBK_CTRL_REG_ADDR	0x2008

/* register to control the Master DLL logic */
#define PHY_DLL_MASTER_CTRL_REG_ADDR	0x200C

/* register to control the Slave DLL logic */
#define PHY_DLL_SLAVE_CTRL_REG_ADDR	0x2010
#define PHY_DLL_SLAVE_CTRL_REG_READ_DQS_CMD_DELAY	GENMASK(31, 24)
#define PHY_DLL_SLAVE_CTRL_REG_READ_DQS_DELAY		GENMASK(7, 0)

#define SDHCI_CDNS6_PHY_CFG_NUM		4
#define SDHCI_CDNS6_CTRL_CFG_NUM	4

struct sdhci_cdns6_phy_cfg {
	const char *property;
	u32 val;
};

struct sdhci_cdns6_ctrl_cfg {
	const char *property;
	u32 val;
};

static struct sdhci_cdns6_phy_cfg sd_ds_phy_cfgs[] = {
	{ "cdns,phy-dqs-timing-delay-sd-ds", 0x00380004, },
	{ "cdns,phy-gate-lpbk_ctrl-delay-sd-ds", 0x01A00040, },
	{ "cdns,phy-dll-slave-ctrl-sd-ds", 0x00000000, },
	{ "cdns,phy-dq-timing-delay-sd-ds", 0x00000001, },
};

static struct sdhci_cdns6_phy_cfg emmc_sdr_phy_cfgs[] = {
	{ "cdns,phy-dqs-timing-delay-semmc-sdr", 0x00380004, },
	{ "cdns,phy-gate-lpbk_ctrl-delay-emmc-sdr", 0x01A00040, },
	{ "cdns,phy-dll-slave-ctrl-emmc-sdr", 0x00000000, },
	{ "cdns,phy-dq-timing-delay-emmc-sdr", 0x00000001, },
};

static struct sdhci_cdns6_phy_cfg emmc_ddr_phy_cfgs[] = {
	{ "cdns,phy-dqs-timing-delay-emmc-ddr", 0x00380004, },
	{ "cdns,phy-gate-lpbk_ctrl-delay-emmc-ddr", 0x01A00040, },
	{ "cdns,phy-dll-slave-ctrl-emmc-ddr", 0x00000000, },
	{ "cdns,phy-dq-timing-delay-emmc-ddr", 0x10000001, },
};

static struct sdhci_cdns6_phy_cfg emmc_hs200_phy_cfgs[] = {
	{ "cdns,phy-dqs-timing-delay-emmc-hs200", 0x00380004, },
	{ "cdns,phy-gate-lpbk_ctrl-delay-emmc-hs200", 0x01A00040, },
	{ "cdns,phy-dll-slave-ctrl-emmc-hs200", 0x00DADA00, },
	{ "cdns,phy-dq-timing-delay-emmc-hs200", 0x00000001, },
};

static struct sdhci_cdns6_phy_cfg emmc_hs400_phy_cfgs[] = {
	{ "cdns,phy-dqs-timing-delay-emmc-hs400", 0x00280004, },
	{ "cdns,phy-gate-lpbk_ctrl-delay-emmc-hs400", 0x01A00040, },
	{ "cdns,phy-dll-slave-ctrl-emmc-hs400", 0x00DAD800, },
	{ "cdns,phy-dq-timing-delay-emmc-hs400", 0x00000001, },
};

static struct sdhci_cdns6_ctrl_cfg sd_ds_ctrl_cfgs[] = {
	{ "cdns,ctrl-hrs09-timing-delay-sd-ds", 0x0001800C, },
	{ "cdns,ctrl-hrs10-lpbk_ctrl-delay-sd-ds", 0x00020000, },
	{ "cdns,ctrl-hrs16-slave-ctrl-sd-ds", 0x00000000, },
	{ "cdns,ctrl-hrs07-timing-delay-sd-ds", 0x00080000, },
};

static struct sdhci_cdns6_ctrl_cfg emmc_sdr_ctrl_cfgs[] = {
	{ "cdns,ctrl-hrs09-timing-delay-emmc-sdr", 0x0001800C, },
	{ "cdns,ctrl-hrs10-lpbk_ctrl-delay-emmc-sdr", 0x00030000, },
	{ "cdns,ctrl-hrs16-slave-ctrl-emmc-sdr", 0x00000000, },
	{ "cdns,ctrl-hrs07-timing-delay-emmc-sdr", 0x00080000, },
};

static struct sdhci_cdns6_ctrl_cfg emmc_ddr_ctrl_cfgs[] = {
	{ "cdns,ctrl-hrs09-timing-delay-emmc-ddr", 0x0001800C, },
	{ "cdns,ctrl-hrs10-lpbk_ctrl-delay-emmc-ddr", 0x00020000, },
	{ "cdns,ctrl-hrs16-slave-ctrl-emmc-ddr", 0x11000001, },
	{ "cdns,ctrl-hrs07-timing-delay-emmc-ddr", 0x00090001, },
};

static struct sdhci_cdns6_ctrl_cfg emmc_hs200_ctrl_cfgs[] = {
	{ "cdns,ctrl-hrs09-timing-delay-emmc-hs200", 0x00018000, },
	{ "cdns,ctrl-hrs10-lpbk_ctrl-delay-emmc-hs200", 0x00080000, },
	{ "cdns,ctrl-hrs16-slave-ctrl-emmc-hs200", 0x00000000, },
	{ "cdns,ctrl-hrs07-timing-delay-emmc-hs200", 0x00090000, },
};

static struct sdhci_cdns6_ctrl_cfg emmc_hs400_ctrl_cfgs[] = {
	{ "cdns,ctrl-hrs09-timing-delay-emmc-hs400", 0x00018000, },
	{ "cdns,ctrl-hrs10-lpbk_ctrl-delay-emmc-hs400", 0x00080000, },
	{ "cdns,ctrl-hrs16-slave-ctrl-emmc-hs400", 0x11000000, },
	{ "cdns,ctrl-hrs07-timing-delay-emmc-hs400", 0x00080000, },
};

static u32 sdhci_cdns6_read_phy_reg(struct sdhci_cdns_plat *plat, u32 addr)
{
	writel(addr, plat->hrs_addr + SDHCI_CDNS_HRS04);
	return readl(plat->hrs_addr + SDHCI_CDNS_HRS05);
}

static void sdhci_cdns6_write_phy_reg(struct sdhci_cdns_plat *plat, u32 addr, u32 val)
{
	writel(addr, plat->hrs_addr + SDHCI_CDNS_HRS04);
	writel(val, plat->hrs_addr + SDHCI_CDNS_HRS05);
}

static int sdhci_cdns6_reset_phy_dll(struct sdhci_cdns_plat *plat, bool reset)
{
	void __iomem *reg = plat->hrs_addr + SDHCI_CDNS_HRS09;
	u32 tmp;
	int ret;

	tmp = readl(reg);
	tmp &= ~SDHCI_CDNS_HRS09_PHY_SW_RESET;

	/* Switch On DLL Reset */
	if (reset)
		tmp |= FIELD_PREP(SDHCI_CDNS_HRS09_PHY_SW_RESET, 0);
	else
		tmp |= FIELD_PREP(SDHCI_CDNS_HRS09_PHY_SW_RESET, 1);

	writel(tmp, reg);

	/* After reset, wait until HRS09.PHY_INIT_COMPLETE is set to 1 within 3000us*/
	if (!reset) {
		ret = readl_poll_timeout(reg, tmp, (tmp & SDHCI_CDNS_HRS09_PHY_INIT_COMPLETE),
					 3000);
	}

	return ret;
}

int sdhci_cdns6_phy_adj(struct udevice *dev, struct sdhci_cdns_plat *plat, u32 mode)
{
	DECLARE_GLOBAL_DATA_PTR;
	struct sdhci_cdns6_phy_cfg *sdhci_cdns6_phy_cfgs;
	struct sdhci_cdns6_ctrl_cfg *sdhci_cdns6_ctrl_cfgs;
	const fdt32_t *prop;
	u32 tmp;
	int i, ret;

	switch (mode) {
	case SDHCI_CDNS_HRS06_MODE_SD:
		sdhci_cdns6_phy_cfgs = sd_ds_phy_cfgs;
		sdhci_cdns6_ctrl_cfgs = sd_ds_ctrl_cfgs;
		break;

	case SDHCI_CDNS_HRS06_MODE_MMC_SDR:
		sdhci_cdns6_phy_cfgs = emmc_sdr_phy_cfgs;
		sdhci_cdns6_ctrl_cfgs = emmc_sdr_ctrl_cfgs;
		break;

	case SDHCI_CDNS_HRS06_MODE_MMC_DDR:
		sdhci_cdns6_phy_cfgs = emmc_ddr_phy_cfgs;
		sdhci_cdns6_ctrl_cfgs = emmc_ddr_ctrl_cfgs;
		break;

	case SDHCI_CDNS_HRS06_MODE_MMC_HS200:
		sdhci_cdns6_phy_cfgs = emmc_hs200_phy_cfgs;
		sdhci_cdns6_ctrl_cfgs = emmc_hs200_ctrl_cfgs;
		break;

	case SDHCI_CDNS_HRS06_MODE_MMC_HS400:
		sdhci_cdns6_phy_cfgs = emmc_hs400_phy_cfgs;
		sdhci_cdns6_ctrl_cfgs = emmc_hs400_ctrl_cfgs;
		break;
	default:
		return -EINVAL;
	}

	for (i = 0; i < SDHCI_CDNS6_PHY_CFG_NUM; i++) {
		prop = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
				   sdhci_cdns6_phy_cfgs[i].property, NULL);
		if (prop)
			sdhci_cdns6_phy_cfgs[i].val = *prop;
	}

	for (i = 0; i < SDHCI_CDNS6_CTRL_CFG_NUM; i++) {
		prop = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
				   sdhci_cdns6_ctrl_cfgs[i].property, NULL);
		if (prop)
			sdhci_cdns6_ctrl_cfgs[i].val = *prop;
	}

	/* Switch On the DLL Reset */
	sdhci_cdns6_reset_phy_dll(plat, true);

	sdhci_cdns6_write_phy_reg(plat, PHY_DQS_TIMING_REG_ADDR, sdhci_cdns6_phy_cfgs[0].val);
	sdhci_cdns6_write_phy_reg(plat, PHY_GATE_LPBK_CTRL_REG_ADDR, sdhci_cdns6_phy_cfgs[1].val);
	sdhci_cdns6_write_phy_reg(plat, PHY_DLL_SLAVE_CTRL_REG_ADDR, sdhci_cdns6_phy_cfgs[2].val);

	/* Switch Off the DLL Reset */
	ret = sdhci_cdns6_reset_phy_dll(plat, false);
	if (ret) {
		printf("sdhci_cdns6_reset_phy is not completed\n");
		return ret;
	}

	/* Set PHY DQ TIMING control register */
	sdhci_cdns6_write_phy_reg(plat, PHY_DQ_TIMING_REG_ADDR, sdhci_cdns6_phy_cfgs[3].val);

	/* Set HRS09 register */
	tmp = readl(plat->hrs_addr + SDHCI_CDNS_HRS09);
	tmp &= ~(SDHCI_CDNS_HRS09_EXTENDED_WR_MODE |
		 SDHCI_CDNS_HRS09_EXTENDED_RD_MODE |
		 SDHCI_CDNS_HRS09_RDDATA_EN |
		 SDHCI_CDNS_HRS09_RDCMD_EN);
	tmp |= sdhci_cdns6_ctrl_cfgs[0].val;
	writel(tmp, plat->hrs_addr + SDHCI_CDNS_HRS09);

	/* Set HRS10 register */
	tmp = readl(plat->hrs_addr + SDHCI_CDNS_HRS10);
	tmp &= ~SDHCI_CDNS_HRS10_HCSDCLKADJ;
	tmp |= sdhci_cdns6_ctrl_cfgs[1].val;
	writel(tmp, plat->hrs_addr + SDHCI_CDNS_HRS10);

	/* Set HRS16 register */
	writel(sdhci_cdns6_ctrl_cfgs[2].val, plat->hrs_addr + SDHCI_CDNS_HRS16);

	/* Set HRS07 register */
	writel(sdhci_cdns6_ctrl_cfgs[3].val, plat->hrs_addr + SDHCI_CDNS_HRS07);

	return 0;
}

int sdhci_cdns6_phy_init(struct udevice *dev, struct sdhci_cdns_plat *plat)
{
	return sdhci_cdns6_phy_adj(dev, plat, SDHCI_CDNS_HRS06_MODE_SD);
}

int sdhci_cdns6_set_tune_val(struct sdhci_cdns_plat *plat, unsigned int val)
{
	u32 tmp, tuneval;

	tuneval = (val * 256) / SDHCI_CDNS_MAX_TUNING_LOOP;

	tmp = sdhci_cdns6_read_phy_reg(plat, PHY_DLL_SLAVE_CTRL_REG_ADDR);
	tmp &= ~(PHY_DLL_SLAVE_CTRL_REG_READ_DQS_CMD_DELAY |
		 PHY_DLL_SLAVE_CTRL_REG_READ_DQS_DELAY);
	tmp |= FIELD_PREP(PHY_DLL_SLAVE_CTRL_REG_READ_DQS_CMD_DELAY, tuneval) |
		FIELD_PREP(PHY_DLL_SLAVE_CTRL_REG_READ_DQS_DELAY, tuneval);
	sdhci_cdns6_write_phy_reg(plat, PHY_DLL_SLAVE_CTRL_REG_ADDR, tmp);

	return 0;
}
