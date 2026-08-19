// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Starfive.
 *   Author: Kuan Lim Lee <kuanlim.lee@starfivetech.com>
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <dm.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/delay.h>
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

#define SDHCI_CDNS6_PHY_CFG_NUM		5
#define SDHCI_CDNS6_CTRL_CFG_NUM	4

struct sdhci_cdns6_phy_cfg {
	const char *property;
	u32 val;
};

struct sdhci_cdns6_ctrl_cfg {
	const char *property;
	u32 val;
};

static const struct sdhci_cdns6_phy_cfg sd_ds_phy_cfgs[] = {
	{ "cdns,phy-dqs-timing-delay-sd-ds", 0x00380004, },
	{ "cdns,phy-gate-lpbk-ctrl-delay-sd-ds", 0x01A00040, },
	{ "cdns,phy-dll-slave-ctrl-sd-ds", 0x00000000, },
	{ "cdns,phy-dq-timing-delay-sd-ds", 0x00000001, },
	{ "cdns,phy-dll-master-ctrl-sd-ds", 0x00800004, },
};

static const struct sdhci_cdns6_phy_cfg sd_hs_phy_cfgs[] = {
	{ "cdns,phy-dqs-timing-delay-sd-hs", 0x00380004, },
	{ "cdns,phy-gate-lpbk-ctrl-delay-sd-hs", 0x01A00040, },
	{ "cdns,phy-dll-slave-ctrl-sd-hs", 0x00000000, },
	{ "cdns,phy-dq-timing-delay-sd-hs", 0x00000001, },
	{ "cdns,phy-dll-master-ctrl-sd-hs", 0x00800004, },
};

static const struct sdhci_cdns6_phy_cfg emmc_sdr_phy_cfgs[] = {
	{ "cdns,phy-dqs-timing-delay-emmc-sdr", 0x00380004, },
	{ "cdns,phy-gate-lpbk-ctrl-delay-emmc-sdr", 0x01A00040, },
	{ "cdns,phy-dll-slave-ctrl-emmc-sdr", 0x00000000, },
	{ "cdns,phy-dq-timing-delay-emmc-sdr", 0x00000001, },
	{ "cdns,phy-dll-master-ctrl-emmc-sdr", 0x00800004, },
};

static const struct sdhci_cdns6_phy_cfg emmc_ddr_phy_cfgs[] = {
	{ "cdns,phy-dqs-timing-delay-emmc-ddr", 0x00380004, },
	{ "cdns,phy-gate-lpbk-ctrl-delay-emmc-ddr", 0x01A00040, },
	{ "cdns,phy-dll-slave-ctrl-emmc-ddr", 0x00000000, },
	{ "cdns,phy-dq-timing-delay-emmc-ddr", 0x10000001, },
	{ "cdns,phy-dll-master-ctrl-emmc-ddr", 0x00800004, },
};

static const struct sdhci_cdns6_phy_cfg emmc_hs200_phy_cfgs[] = {
	{ "cdns,phy-dqs-timing-delay-emmc-hs200", 0x00380004, },
	{ "cdns,phy-gate-lpbk-ctrl-delay-emmc-hs200", 0x01A00040, },
	{ "cdns,phy-dll-slave-ctrl-emmc-hs200", 0x00DADA00, },
	{ "cdns,phy-dq-timing-delay-emmc-hs200", 0x00000001, },
	{ "cdns,phy-dll-master-ctrl-emmc-hs200", 0x00000004, },
};

static const struct sdhci_cdns6_phy_cfg emmc_hs400_phy_cfgs[] = {
	{ "cdns,phy-dqs-timing-delay-emmc-hs400", 0x00280004, },
	{ "cdns,phy-gate-lpbk-ctrl-delay-emmc-hs400", 0x01A00040, },
	{ "cdns,phy-dll-slave-ctrl-emmc-hs400", 0x00DAD800, },
	{ "cdns,phy-dq-timing-delay-emmc-hs400", 0x00000001, },
	{ "cdns,phy-dll-master-ctrl-emmc-hs400", 0x00000004, },
};

static const struct sdhci_cdns6_ctrl_cfg sd_ds_ctrl_cfgs[] = {
	{ "cdns,ctrl-hrs09-timing-delay-sd-ds", 0x0001800C, },
	{ "cdns,ctrl-hrs10-lpbk-ctrl-delay-sd-ds", 0x00020000, },
	{ "cdns,ctrl-hrs16-slave-ctrl-sd-ds", 0x00000000, },
	{ "cdns,ctrl-hrs07-timing-delay-sd-ds", 0x00080000, },
};

static const struct sdhci_cdns6_ctrl_cfg sd_hs_ctrl_cfgs[] = {
	{ "cdns,ctrl-hrs09-timing-delay-sd-hs", 0x0001800C, },
	{ "cdns,ctrl-hrs10-lpbk-ctrl-delay-sd-hs", 0x00030000, },
	{ "cdns,ctrl-hrs16-slave-ctrl-sd-hs", 0x00000000, },
	{ "cdns,ctrl-hrs07-timing-delay-sd-hs", 0x00080000, },
};

static const struct sdhci_cdns6_ctrl_cfg emmc_sdr_ctrl_cfgs[] = {
	{ "cdns,ctrl-hrs09-timing-delay-emmc-sdr", 0x0001800C, },
	{ "cdns,ctrl-hrs10-lpbk-ctrl-delay-emmc-sdr", 0x00030000, },
	{ "cdns,ctrl-hrs16-slave-ctrl-emmc-sdr", 0x00000000, },
	{ "cdns,ctrl-hrs07-timing-delay-emmc-sdr", 0x00080000, },
};

static const struct sdhci_cdns6_ctrl_cfg emmc_ddr_ctrl_cfgs[] = {
	{ "cdns,ctrl-hrs09-timing-delay-emmc-ddr", 0x0001800C, },
	{ "cdns,ctrl-hrs10-lpbk-ctrl-delay-emmc-ddr", 0x00020000, },
	{ "cdns,ctrl-hrs16-slave-ctrl-emmc-ddr", 0x11000001, },
	{ "cdns,ctrl-hrs07-timing-delay-emmc-ddr", 0x00090001, },
};

static const struct sdhci_cdns6_ctrl_cfg emmc_hs200_ctrl_cfgs[] = {
	{ "cdns,ctrl-hrs09-timing-delay-emmc-hs200", 0x00018000, },
	{ "cdns,ctrl-hrs10-lpbk-ctrl-delay-emmc-hs200", 0x00080000, },
	{ "cdns,ctrl-hrs16-slave-ctrl-emmc-hs200", 0x00000000, },
	{ "cdns,ctrl-hrs07-timing-delay-emmc-hs200", 0x00090000, },
};

static const struct sdhci_cdns6_ctrl_cfg emmc_hs400_ctrl_cfgs[] = {
	{ "cdns,ctrl-hrs09-timing-delay-emmc-hs400", 0x00018000, },
	{ "cdns,ctrl-hrs10-lpbk-ctrl-delay-emmc-hs400", 0x00080000, },
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

	/*
	 * HRS04/HRS05 form the indirect PHY register port: HRS04 latches the
	 * target address, HRS05 the data. Both are posted writes, so read
	 * HRS05 back to force them to complete before the next PHY access.
	 * The readback also orders the preceding HRS04 write - reads and
	 * writes to the same slave are not reordered on this interconnect, so
	 * a single HRS05 readback flushes the whole address+data pair.
	 */
	(void)readl(plat->hrs_addr + SDHCI_CDNS_HRS05);
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
	const struct sdhci_cdns6_phy_cfg *phy_defaults;
	const struct sdhci_cdns6_ctrl_cfg *ctrl_defaults;
	struct sdhci_cdns6_phy_cfg phy_cfgs[SDHCI_CDNS6_PHY_CFG_NUM];
	struct sdhci_cdns6_ctrl_cfg ctrl_cfgs[SDHCI_CDNS6_CTRL_CFG_NUM];
	u32 tmp;
	int i, ret;

	switch (mode) {
	case UHS_SDR12:
	case MMC_LEGACY:
		phy_defaults = sd_ds_phy_cfgs;
		ctrl_defaults = sd_ds_ctrl_cfgs;
		break;

	case SD_HS:
	case UHS_SDR25:
	case MMC_HS:
		phy_defaults = sd_hs_phy_cfgs;
		ctrl_defaults = sd_hs_ctrl_cfgs;
		break;

	case UHS_SDR50:
	case MMC_HS_52:
		phy_defaults = emmc_sdr_phy_cfgs;
		ctrl_defaults = emmc_sdr_ctrl_cfgs;
		break;

	case UHS_DDR50:
	case MMC_DDR_52:
		phy_defaults = emmc_ddr_phy_cfgs;
		ctrl_defaults = emmc_ddr_ctrl_cfgs;
		break;

	case UHS_SDR104:
	case MMC_HS_200:
		phy_defaults = emmc_hs200_phy_cfgs;
		ctrl_defaults = emmc_hs200_ctrl_cfgs;
		break;

	case MMC_HS_400:
	case MMC_HS_400_ES:
		phy_defaults = emmc_hs400_phy_cfgs;
		ctrl_defaults = emmc_hs400_ctrl_cfgs;
		break;
	default:
		return -EINVAL;
	}

	/*
	 * Work on per-call copies so DT overrides never mutate the shared
	 * const defaults, which would otherwise leak between controllers.
	 */
	for (i = 0; i < SDHCI_CDNS6_PHY_CFG_NUM; i++) {
		phy_cfgs[i] = phy_defaults[i];
		dev_read_u32(dev, phy_cfgs[i].property, &phy_cfgs[i].val);
	}

	for (i = 0; i < SDHCI_CDNS6_CTRL_CFG_NUM; i++) {
		ctrl_cfgs[i] = ctrl_defaults[i];
		dev_read_u32(dev, ctrl_cfgs[i].property, &ctrl_cfgs[i].val);
	}

	/* Switch On the DLL Reset */
	sdhci_cdns6_reset_phy_dll(plat, true);

	sdhci_cdns6_write_phy_reg(plat, PHY_DQS_TIMING_REG_ADDR, phy_cfgs[0].val);
	sdhci_cdns6_write_phy_reg(plat, PHY_GATE_LPBK_CTRL_REG_ADDR, phy_cfgs[1].val);
	sdhci_cdns6_write_phy_reg(plat, PHY_DLL_MASTER_CTRL_REG_ADDR, phy_cfgs[4].val);
	sdhci_cdns6_write_phy_reg(plat, PHY_DLL_SLAVE_CTRL_REG_ADDR, phy_cfgs[2].val);

	/* Switch Off the DLL Reset */
	ret = sdhci_cdns6_reset_phy_dll(plat, false);
	if (ret) {
		printf("sdhci_cdns6_reset_phy is not completed\n");
		return ret;
	}

	/* Set PHY DQ TIMING control register */
	sdhci_cdns6_write_phy_reg(plat, PHY_DQ_TIMING_REG_ADDR, phy_cfgs[3].val);

	/* Set HRS09 register */
	tmp = readl(plat->hrs_addr + SDHCI_CDNS_HRS09);
	tmp &= ~(SDHCI_CDNS_HRS09_EXTENDED_WR_MODE |
		 SDHCI_CDNS_HRS09_EXTENDED_RD_MODE |
		 SDHCI_CDNS_HRS09_RDDATA_EN |
		 SDHCI_CDNS_HRS09_RDCMD_EN);
	tmp |= ctrl_cfgs[0].val;
	writel(tmp, plat->hrs_addr + SDHCI_CDNS_HRS09);

	/* Set HRS10 register */
	tmp = readl(plat->hrs_addr + SDHCI_CDNS_HRS10);
	tmp &= ~SDHCI_CDNS_HRS10_HCSDCLKADJ;
	tmp |= ctrl_cfgs[1].val;
	writel(tmp, plat->hrs_addr + SDHCI_CDNS_HRS10);

	/* Set HRS16 register */
	writel(ctrl_cfgs[2].val, plat->hrs_addr + SDHCI_CDNS_HRS16);

	/* Set HRS07 register */
	writel(ctrl_cfgs[3].val, plat->hrs_addr + SDHCI_CDNS_HRS07);

	/*
	 * Wait for the PHY/DLL to settle before the first data transfer.
	 * The HRS09.PHY_INIT_COMPLETE handshake polled above only covers DLL
	 * relock, not the analog read/write path retiming that follows a
	 * timing change. Without this extra settle (together with the HRS05
	 * posted-write flush in sdhci_cdns6_write_phy_reg()) the first
	 * transfer after a DDR50 mode switch intermittently returns a CRC
	 * error.
	 */
	mdelay(5);

	return 0;
}

int sdhci_cdns6_phy_init(struct udevice *dev, struct sdhci_cdns_plat *plat)
{
	return sdhci_cdns6_phy_adj(dev, plat, MMC_LEGACY);
}

int sdhci_cdns6_set_tune_val(struct sdhci_cdns_plat *plat, unsigned int val)
{
	u32 tmp, tuneval;
	int ret;

	tuneval = (val * 256) / SDHCI_CDNS_MAX_TUNING_LOOP;

	tmp = sdhci_cdns6_read_phy_reg(plat, PHY_DLL_SLAVE_CTRL_REG_ADDR);
	tmp &= ~(PHY_DLL_SLAVE_CTRL_REG_READ_DQS_CMD_DELAY |
		 PHY_DLL_SLAVE_CTRL_REG_READ_DQS_DELAY);
	tmp |= FIELD_PREP(PHY_DLL_SLAVE_CTRL_REG_READ_DQS_CMD_DELAY, tuneval) |
		FIELD_PREP(PHY_DLL_SLAVE_CTRL_REG_READ_DQS_DELAY, tuneval);

	/* Switch On the DLL Reset */
	sdhci_cdns6_reset_phy_dll(plat, true);

	sdhci_cdns6_write_phy_reg(plat, PHY_DLL_SLAVE_CTRL_REG_ADDR, tmp);

	/* Switch Off the DLL Reset */
	ret = sdhci_cdns6_reset_phy_dll(plat, false);
	if (ret) {
		printf("sdhci_cdns6_reset_phy is not completed\n");
		return ret;
	}

	return 0;
}
