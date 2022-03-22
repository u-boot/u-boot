// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Fuzhou Rockchip Electronics Co., Ltd
 *
 * Rockchip SD Host Controller Interface
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <dt-structs.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/libfdt.h>
#include <linux/iopoll.h>
#include <malloc.h>
#include <mapmem.h>
#include "mmc_private.h"
#include <sdhci.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>

/* DWCMSHC specific Mode Select value */
#define DWCMSHC_CTRL_HS400		0x7
/* 400KHz is max freq for card ID etc. Use that as min */
#define EMMC_MIN_FREQ	400000
#define KHz	(1000)
#define MHz	(1000 * KHz)
#define SDHCI_TUNING_LOOP_COUNT		40

#define PHYCTRL_CALDONE_MASK		0x1
#define PHYCTRL_CALDONE_SHIFT		0x6
#define PHYCTRL_CALDONE_DONE		0x1
#define PHYCTRL_DLLRDY_MASK		0x1
#define PHYCTRL_DLLRDY_SHIFT		0x5
#define PHYCTRL_DLLRDY_DONE		0x1
#define PHYCTRL_FREQSEL_200M		0x0
#define PHYCTRL_FREQSEL_50M		0x1
#define PHYCTRL_FREQSEL_100M		0x2
#define PHYCTRL_FREQSEL_150M		0x3
#define PHYCTRL_DLL_LOCK_WO_TMOUT(x)	\
	((((x) >> PHYCTRL_DLLRDY_SHIFT) & PHYCTRL_DLLRDY_MASK) ==\
	PHYCTRL_DLLRDY_DONE)

#define ARASAN_VENDOR_REGISTER		0x78
#define ARASAN_VENDOR_ENHANCED_STROBE	BIT(0)

/* DWC IP vendor area 1 pointer */
#define DWCMSHC_P_VENDOR_AREA1		0xe8
#define DWCMSHC_AREA1_MASK		GENMASK(11, 0)
/* Offset inside the vendor area 1 */
#define DWCMSHC_EMMC_CONTROL		0x2c
#define DWCMSHC_CARD_IS_EMMC		BIT(0)
#define DWCMSHC_ENHANCED_STROBE		BIT(8)

/* Rockchip specific Registers */
#define DWCMSHC_EMMC_DLL_CTRL		0x800
#define DWCMSHC_EMMC_DLL_CTRL_RESET	BIT(1)
#define DWCMSHC_EMMC_DLL_RXCLK		0x804
#define DWCMSHC_EMMC_DLL_TXCLK		0x808
#define DWCMSHC_EMMC_DLL_STRBIN		0x80c
#define DWCMSHC_EMMC_DLL_STATUS0	0x840
#define DWCMSHC_EMMC_DLL_STATUS1	0x844
#define DWCMSHC_EMMC_DLL_START		BIT(0)
#define DWCMSHC_EMMC_DLL_RXCLK_SRCSEL	29
#define DWCMSHC_EMMC_DLL_START_POINT	16
#define DWCMSHC_EMMC_DLL_START_DEFAULT	5
#define DWCMSHC_EMMC_DLL_INC_VALUE	2
#define DWCMSHC_EMMC_DLL_INC		8
#define DWCMSHC_EMMC_DLL_DLYENA		BIT(27)
#define DLL_TXCLK_TAPNUM_DEFAULT	0xA

#define DLL_STRBIN_TAPNUM_DEFAULT	0x8
#define DLL_STRBIN_TAPNUM_FROM_SW	BIT(24)
#define DLL_STRBIN_DELAY_NUM_SEL	BIT(26)
#define DLL_STRBIN_DELAY_NUM_OFFSET	16
#define DLL_STRBIN_DELAY_NUM_DEFAULT	0x16

#define DLL_TXCLK_TAPNUM_FROM_SW	BIT(24)
#define DWCMSHC_EMMC_DLL_LOCKED		BIT(8)
#define DWCMSHC_EMMC_DLL_TIMEOUT	BIT(9)
#define DLL_RXCLK_NO_INVERTER		1
#define DLL_RXCLK_INVERTER		0
#define DWCMSHC_ENHANCED_STROBE		BIT(8)
#define DLL_LOCK_WO_TMOUT(x) \
	((((x) & DWCMSHC_EMMC_DLL_LOCKED) == DWCMSHC_EMMC_DLL_LOCKED) && \
	(((x) & DWCMSHC_EMMC_DLL_TIMEOUT) == 0))
#define ROCKCHIP_MAX_CLKS		3

struct rockchip_sdhc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct rockchip_emmc_phy {
	u32 emmcphy_con[7];
	u32 reserved;
	u32 emmcphy_status;
};

struct rockchip_sdhc {
	struct sdhci_host host;
	struct udevice *dev;
	void *base;
	struct rockchip_emmc_phy *phy;
	struct clk emmc_clk;
};

struct sdhci_data {
	int (*emmc_phy_init)(struct udevice *dev);
	int (*get_phy)(struct udevice *dev);

	/**
	 * set_control_reg() - Set SDHCI control registers
	 *
	 * This is the set_control_reg() SDHCI operation that should be
	 * used for the hardware this driver data is associated with.
	 * Normally, this is used to set up control registers for
	 * voltage level and UHS speed mode.
	 *
	 * @host: SDHCI host structure
	 */
	void (*set_control_reg)(struct sdhci_host *host);

	/**
	 * set_ios_post() - Host specific hook after set_ios() calls
	 *
	 * This is the set_ios_post() SDHCI operation that should be
	 * used for the hardware this driver data is associated with.
	 * Normally, this is a hook that is called after sdhci_set_ios()
	 * that does any necessary host-specific configuration.
	 *
	 * @host: SDHCI host structure
	 * Return: 0 if successful, -ve on error
	 */
	int (*set_ios_post)(struct sdhci_host *host);

	/**
	 * set_enhanced_strobe() - Set HS400 Enhanced Strobe config
	 *
	 * This is the set_enhanced_strobe() SDHCI operation that should
	 * be used for the hardware this driver data is associated with.
	 * Normally, this is used to set any host-specific configuration
	 * necessary for HS400 ES.
	 *
	 * @host: SDHCI host structure
	 * Return: 0 if successful, -ve on error
	 */
	int (*set_enhanced_strobe)(struct sdhci_host *host);
};

static int rk3399_emmc_phy_init(struct udevice *dev)
{
	return 0;
}

static void rk3399_emmc_phy_power_on(struct rockchip_emmc_phy *phy, u32 clock)
{
	u32 caldone, dllrdy, freqsel;

	writel(RK_CLRSETBITS(7 << 4, 0), &phy->emmcphy_con[6]);
	writel(RK_CLRSETBITS(1 << 11, 1 << 11), &phy->emmcphy_con[0]);
	writel(RK_CLRSETBITS(0xf << 7, 6 << 7), &phy->emmcphy_con[0]);

	/*
	 * According to the user manual, calpad calibration
	 * cycle takes more than 2us without the minimal recommended
	 * value, so we may need a little margin here
	 */
	udelay(3);
	writel(RK_CLRSETBITS(1, 1), &phy->emmcphy_con[6]);

	/*
	 * According to the user manual, it asks driver to
	 * wait 5us for calpad busy trimming. But it seems that
	 * 5us of caldone isn't enough for all cases.
	 */
	udelay(500);
	caldone = readl(&phy->emmcphy_status);
	caldone = (caldone >> PHYCTRL_CALDONE_SHIFT) & PHYCTRL_CALDONE_MASK;
	if (caldone != PHYCTRL_CALDONE_DONE) {
		printf("%s: caldone timeout.\n", __func__);
		return;
	}

	/* Set the frequency of the DLL operation */
	if (clock < 75 * MHz)
		freqsel = PHYCTRL_FREQSEL_50M;
	else if (clock < 125 * MHz)
		freqsel = PHYCTRL_FREQSEL_100M;
	else if (clock < 175 * MHz)
		freqsel = PHYCTRL_FREQSEL_150M;
	else
		freqsel = PHYCTRL_FREQSEL_200M;

	/* Set the frequency of the DLL operation */
	writel(RK_CLRSETBITS(3 << 12, freqsel << 12), &phy->emmcphy_con[0]);
	writel(RK_CLRSETBITS(1 << 1, 1 << 1), &phy->emmcphy_con[6]);

	/* REN Enable on STRB Line for HS400 */
	writel(RK_CLRSETBITS(0, 1 << 9), &phy->emmcphy_con[2]);

	read_poll_timeout(readl, dllrdy, PHYCTRL_DLL_LOCK_WO_TMOUT(dllrdy), 1,
			  5000, &phy->emmcphy_status);
}

static void rk3399_emmc_phy_power_off(struct rockchip_emmc_phy *phy)
{
	writel(RK_CLRSETBITS(1, 0), &phy->emmcphy_con[6]);
	writel(RK_CLRSETBITS(1 << 1, 0), &phy->emmcphy_con[6]);
}

static int rk3399_emmc_get_phy(struct udevice *dev)
{
	struct rockchip_sdhc *priv = dev_get_priv(dev);
	ofnode phy_node;
	void *grf_base;
	u32 grf_phy_offset, phandle;

	phandle = dev_read_u32_default(dev, "phys", 0);
	phy_node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(phy_node)) {
		debug("Not found emmc phy device\n");
		return -ENODEV;
	}

	grf_base = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR_OR_NULL(grf_base)) {
		printf("%s Get syscon grf failed", __func__);
		return -ENODEV;
	}
	grf_phy_offset = ofnode_read_u32_default(phy_node, "reg", 0);

	priv->phy = (struct rockchip_emmc_phy *)(grf_base + grf_phy_offset);

	return 0;
}

static int rk3399_sdhci_set_enhanced_strobe(struct sdhci_host *host)
{
	struct mmc *mmc = host->mmc;
	u32 vendor;

	vendor = sdhci_readl(host, ARASAN_VENDOR_REGISTER);
	if (mmc->selected_mode == MMC_HS_400_ES)
		vendor |= ARASAN_VENDOR_ENHANCED_STROBE;
	else
		vendor &= ~ARASAN_VENDOR_ENHANCED_STROBE;
	sdhci_writel(host, vendor, ARASAN_VENDOR_REGISTER);

	return 0;
}

static void rk3399_sdhci_set_control_reg(struct sdhci_host *host)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct mmc *mmc = host->mmc;
	uint clock = mmc->tran_speed;
	int cycle_phy = host->clock != clock && clock > EMMC_MIN_FREQ;

	if (cycle_phy)
		rk3399_emmc_phy_power_off(priv->phy);

	sdhci_set_control_reg(host);

	/*
	 * Reinitializing the device tries to set it to lower-speed modes
	 * first, which fails if the Enhanced Strobe bit is set, making
	 * the device impossible to use. Set the correct value here to
	 * let reinitialization attempts succeed.
	 */
	if (CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT))
		rk3399_sdhci_set_enhanced_strobe(host);
};

static int rk3399_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct mmc *mmc = host->mmc;
	uint clock = mmc->tran_speed;
	int cycle_phy = host->clock != clock && clock > EMMC_MIN_FREQ;

	if (!clock)
		clock = mmc->clock;

	if (cycle_phy)
		rk3399_emmc_phy_power_on(priv->phy, clock);

	return 0;
}

static int rk3568_emmc_phy_init(struct udevice *dev)
{
	struct rockchip_sdhc *prv = dev_get_priv(dev);
	struct sdhci_host *host = &prv->host;
	u32 extra;

	extra = DLL_RXCLK_NO_INVERTER << DWCMSHC_EMMC_DLL_RXCLK_SRCSEL;
	sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_RXCLK);

	return 0;
}

static int rk3568_sdhci_emmc_set_clock(struct sdhci_host *host, unsigned int clock)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	int val, ret;
	u32 extra;

	if (clock > host->max_clk)
		clock = host->max_clk;
	if (clock)
		clk_set_rate(&priv->emmc_clk, clock);

	sdhci_set_clock(host->mmc, clock);

	if (clock >= 100 * MHz) {
		/* reset DLL */
		sdhci_writel(host, DWCMSHC_EMMC_DLL_CTRL_RESET, DWCMSHC_EMMC_DLL_CTRL);
		udelay(1);
		sdhci_writel(host, 0, DWCMSHC_EMMC_DLL_CTRL);

		/* Init DLL settings */
		extra = DWCMSHC_EMMC_DLL_START_DEFAULT << DWCMSHC_EMMC_DLL_START_POINT |
			DWCMSHC_EMMC_DLL_INC_VALUE << DWCMSHC_EMMC_DLL_INC |
			DWCMSHC_EMMC_DLL_START;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_CTRL);

		ret = read_poll_timeout(readl, val, DLL_LOCK_WO_TMOUT(val), 1,
					500,
					host->ioaddr + DWCMSHC_EMMC_DLL_STATUS0);
		if (ret)
			return ret;

		extra = DWCMSHC_EMMC_DLL_DLYENA |
			DLL_RXCLK_NO_INVERTER << DWCMSHC_EMMC_DLL_RXCLK_SRCSEL;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_RXCLK);

		extra = DWCMSHC_EMMC_DLL_DLYENA |
			DLL_TXCLK_TAPNUM_DEFAULT |
			DLL_TXCLK_TAPNUM_FROM_SW;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_TXCLK);

		extra = DWCMSHC_EMMC_DLL_DLYENA |
			DLL_STRBIN_TAPNUM_DEFAULT |
			DLL_STRBIN_TAPNUM_FROM_SW;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_STRBIN);
	} else {
		/* reset the clock phase when the frequency is lower than 100MHz */
		sdhci_writel(host, 0, DWCMSHC_EMMC_DLL_CTRL);
		extra = DLL_RXCLK_NO_INVERTER << DWCMSHC_EMMC_DLL_RXCLK_SRCSEL;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_RXCLK);
		sdhci_writel(host, 0, DWCMSHC_EMMC_DLL_TXCLK);
		/*
		 * Before switching to hs400es mode, the driver will enable
		 * enhanced strobe first. PHY needs to configure the parameters
		 * of enhanced strobe first.
		 */
		extra = DWCMSHC_EMMC_DLL_DLYENA |
			DLL_STRBIN_DELAY_NUM_SEL |
			DLL_STRBIN_DELAY_NUM_DEFAULT << DLL_STRBIN_DELAY_NUM_OFFSET;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_STRBIN);
	}

	return 0;
}

static int rk3568_emmc_get_phy(struct udevice *dev)
{
	return 0;
}

static int rk3568_sdhci_set_enhanced_strobe(struct sdhci_host *host)
{
	struct mmc *mmc = host->mmc;
	u32 vendor;
	int reg;

	reg = (sdhci_readl(host, DWCMSHC_P_VENDOR_AREA1) & DWCMSHC_AREA1_MASK)
	      + DWCMSHC_EMMC_CONTROL;

	vendor = sdhci_readl(host, reg);
	if (mmc->selected_mode == MMC_HS_400_ES)
		vendor |= DWCMSHC_ENHANCED_STROBE;
	else
		vendor &= ~DWCMSHC_ENHANCED_STROBE;
	sdhci_writel(host, vendor, reg);

	return 0;
}

static int rk3568_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct mmc *mmc = host->mmc;
	uint clock = mmc->tran_speed;
	u32 reg, vendor_reg;

	if (!clock)
		clock = mmc->clock;

	rk3568_sdhci_emmc_set_clock(host, clock);

	if (mmc->selected_mode == MMC_HS_400 || mmc->selected_mode == MMC_HS_400_ES) {
		reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		reg &= ~SDHCI_CTRL_UHS_MASK;
		reg |= DWCMSHC_CTRL_HS400;
		sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);

		vendor_reg = (sdhci_readl(host, DWCMSHC_P_VENDOR_AREA1) & DWCMSHC_AREA1_MASK)
			     + DWCMSHC_EMMC_CONTROL;
		/* set CARD_IS_EMMC bit to enable Data Strobe for HS400 */
		reg = sdhci_readw(host, vendor_reg);
		reg |= DWCMSHC_CARD_IS_EMMC;
		sdhci_writew(host, reg, vendor_reg);
	} else {
		sdhci_set_uhs_timing(host);
	}

	return 0;
}

static void rockchip_sdhci_set_control_reg(struct sdhci_host *host)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct sdhci_data *data = (struct sdhci_data *)dev_get_driver_data(priv->dev);

	if (data->set_control_reg)
		data->set_control_reg(host);
}

static int rockchip_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct sdhci_data *data = (struct sdhci_data *)dev_get_driver_data(priv->dev);

	if (data->set_ios_post)
		return data->set_ios_post(host);

	return 0;
}

static int rockchip_sdhci_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct sdhci_host *host = dev_get_priv(mmc->dev);
	char tuning_loop_counter = SDHCI_TUNING_LOOP_COUNT;
	struct mmc_cmd cmd;
	u32 ctrl, blk_size;
	int ret = 0;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl |= SDHCI_CTRL_EXEC_TUNING;
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

	sdhci_writel(host, SDHCI_INT_DATA_AVAIL, SDHCI_INT_ENABLE);
	sdhci_writel(host, SDHCI_INT_DATA_AVAIL, SDHCI_SIGNAL_ENABLE);

	blk_size = SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG, 64);
	if (opcode == MMC_CMD_SEND_TUNING_BLOCK_HS200 && host->mmc->bus_width == 8)
		blk_size = SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG, 128);
	sdhci_writew(host, blk_size, SDHCI_BLOCK_SIZE);
	sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

	cmd.cmdidx = opcode;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;

	do {
		if (tuning_loop_counter-- == 0)
			break;

		mmc_send_cmd(mmc, &cmd, NULL);

		if (opcode == MMC_CMD_SEND_TUNING_BLOCK)
			/*
			 * For tuning command, do not do busy loop. As tuning
			 * is happening (CLK-DATA latching for setup/hold time
			 * requirements), give time to complete
			 */
			udelay(1);

		ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	} while (ctrl & SDHCI_CTRL_EXEC_TUNING);

	if (!(ctrl & SDHCI_CTRL_TUNED_CLK)) {
		printf("%s:Tuning failed\n", __func__);
		ret = -EIO;
	}

	if (tuning_loop_counter < 0) {
		ctrl &= ~SDHCI_CTRL_TUNED_CLK;
		sdhci_writel(host, ctrl, SDHCI_HOST_CONTROL2);
	}

	/* Enable only interrupts served by the SD controller */
	sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK, SDHCI_INT_ENABLE);
	/* Mask all sdhci interrupt sources */
	sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

	return ret;
}

static int rockchip_sdhci_set_enhanced_strobe(struct sdhci_host *host)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct sdhci_data *data = (struct sdhci_data *)dev_get_driver_data(priv->dev);

	if (data->set_enhanced_strobe)
		return data->set_enhanced_strobe(host);

	return -ENOTSUPP;
}

static struct sdhci_ops rockchip_sdhci_ops = {
	.set_ios_post	= rockchip_sdhci_set_ios_post,
	.platform_execute_tuning = &rockchip_sdhci_execute_tuning,
	.set_control_reg = rockchip_sdhci_set_control_reg,
	.set_enhanced_strobe = rockchip_sdhci_set_enhanced_strobe,
};

static int rockchip_sdhci_probe(struct udevice *dev)
{
	struct sdhci_data *data = (struct sdhci_data *)dev_get_driver_data(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct rockchip_sdhc_plat *plat = dev_get_plat(dev);
	struct rockchip_sdhc *prv = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	struct sdhci_host *host = &prv->host;
	struct clk clk;
	int ret;

	host->max_clk = cfg->f_max;
	ret = clk_get_by_index(dev, 0, &clk);
	if (!ret) {
		ret = clk_set_rate(&clk, host->max_clk);
		if (IS_ERR_VALUE(ret))
			printf("%s clk set rate fail!\n", __func__);
	} else {
		printf("%s fail to get clk\n", __func__);
	}

	prv->emmc_clk = clk;
	prv->dev = dev;

	if (data->get_phy) {
		ret = data->get_phy(dev);
		if (ret)
			return ret;
	}

	if (data->emmc_phy_init) {
		ret = data->emmc_phy_init(dev);
		if (ret)
			return ret;
	}

	host->ops = &rockchip_sdhci_ops;
	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD;

	host->mmc = &plat->mmc;
	host->mmc->priv = &prv->host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	ret = sdhci_setup_cfg(cfg, host, cfg->f_max, EMMC_MIN_FREQ);
	if (ret)
		return ret;

	return sdhci_probe(dev);
}

static int rockchip_sdhci_of_to_plat(struct udevice *dev)
{
	struct rockchip_sdhc_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	int ret;

	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);

	ret = mmc_of_parse(dev, cfg);
	if (ret)
		return ret;

	return 0;
}

static int rockchip_sdhci_bind(struct udevice *dev)
{
	struct rockchip_sdhc_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct sdhci_data rk3399_data = {
	.get_phy = rk3399_emmc_get_phy,
	.emmc_phy_init = rk3399_emmc_phy_init,
	.set_control_reg = rk3399_sdhci_set_control_reg,
	.set_ios_post = rk3399_sdhci_set_ios_post,
	.set_enhanced_strobe = rk3399_sdhci_set_enhanced_strobe,
};

static const struct sdhci_data rk3568_data = {
	.get_phy = rk3568_emmc_get_phy,
	.emmc_phy_init = rk3568_emmc_phy_init,
	.set_ios_post = rk3568_sdhci_set_ios_post,
	.set_enhanced_strobe = rk3568_sdhci_set_enhanced_strobe,
};

static const struct udevice_id sdhci_ids[] = {
	{
		.compatible = "arasan,sdhci-5.1",
		.data = (ulong)&rk3399_data,
	},
	{
		.compatible = "rockchip,rk3568-dwcmshc",
		.data = (ulong)&rk3568_data,
	},
	{ }
};

U_BOOT_DRIVER(arasan_sdhci_drv) = {
	.name		= "rockchip_sdhci_5_1",
	.id		= UCLASS_MMC,
	.of_match	= sdhci_ids,
	.of_to_plat	= rockchip_sdhci_of_to_plat,
	.ops		= &sdhci_ops,
	.bind		= rockchip_sdhci_bind,
	.probe		= rockchip_sdhci_probe,
	.priv_auto	= sizeof(struct rockchip_sdhc),
	.plat_auto	= sizeof(struct rockchip_sdhc_plat),
};
