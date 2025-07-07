// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Maksim Kiselev <bigunclemax@gmail.com>
 */

#include <clk.h>
#include <dm.h>
#include <linux/bitfield.h>
#include <linux/sizes.h>
#include <sdhci.h>

/* DWCMSHC specific Mode Select value */
#define DWCMSHC_CTRL_HS400		0x7
/* 400KHz is max freq for card ID etc. Use that as min */
#define EMMC_MIN_FREQ	400000
#define SDHCI_TUNING_LOOP_COUNT		128

/* PHY register area pointer */
#define DWC_MSHC_PTR_PHY_R		0x300

/* PHY general configuration */
#define PHY_CNFG_R			(DWC_MSHC_PTR_PHY_R + 0x00)
#define PHY_CNFG_RSTN_DEASSERT		0x1  /* Deassert PHY reset */
#define PHY_CNFG_PAD_SP_MASK		GENMASK(19, 16) /* bits [19:16] */
#define PHY_CNFG_PAD_SP			0x0c /* PMOS TX drive strength */
#define PHY_CNFG_PAD_SN_MASK		GENMASK(23, 20) /* bits [23:20] */
#define PHY_CNFG_PAD_SN			0x0c /* NMOS TX drive strength */

/* PHY command/response pad settings */
#define PHY_CMDPAD_CNFG_R		(DWC_MSHC_PTR_PHY_R + 0x04)

/* PHY data pad settings */
#define PHY_DATAPAD_CNFG_R		(DWC_MSHC_PTR_PHY_R + 0x06)

/* PHY clock pad settings */
#define PHY_CLKPAD_CNFG_R		(DWC_MSHC_PTR_PHY_R + 0x08)

/* PHY strobe pad settings */
#define PHY_STBPAD_CNFG_R		(DWC_MSHC_PTR_PHY_R + 0x0a)

/* PHY reset pad settings */
#define PHY_RSTNPAD_CNFG_R		(DWC_MSHC_PTR_PHY_R + 0x0c)

/* Bitfields are common for all pad settings */
#define PHY_PAD_RXSEL_1V8		0x1 /* Receiver type select for 1.8V */
#define PHY_PAD_RXSEL_3V3		0x2 /* Receiver type select for 3.3V */

#define PHY_PAD_WEAKPULL_MASK		GENMASK(4, 3) /* bits [4:3] */
#define PHY_PAD_WEAKPULL_PULLUP		0x1 /* Weak pull up enabled */
#define PHY_PAD_WEAKPULL_PULLDOWN	0x2 /* Weak pull down enabled */

#define PHY_PAD_TXSLEW_CTRL_P_MASK	GENMASK(8, 5) /* bits [8:5] */
#define PHY_PAD_TXSLEW_CTRL_P		0x3 /* Slew control for P-Type pad TX */
#define PHY_PAD_TXSLEW_CTRL_N_MASK	GENMASK(12, 9) /* bits [12:9] */
#define PHY_PAD_TXSLEW_CTRL_N		0x3 /* Slew control for N-Type pad TX */

/* PHY CLK delay line settings */
#define PHY_SDCLKDL_CNFG_R		(DWC_MSHC_PTR_PHY_R + 0x1d)
#define PHY_SDCLKDL_CNFG_UPDATE		BIT(4) /* set before writing to SDCLKDL_DC */

/* PHY CLK delay line delay code */
#define PHY_SDCLKDL_DC_R		(DWC_MSHC_PTR_PHY_R + 0x1e)
#define PHY_SDCLKDL_DC_INITIAL		0x40 /* initial delay code */
#define PHY_SDCLKDL_DC_DEFAULT		0x32 /* default delay code */
#define PHY_SDCLKDL_DC_HS400		0x18 /* delay code for HS400 mode */

/* PHY drift_cclk_rx delay line configuration setting */
#define PHY_ATDL_CNFG_R			(DWC_MSHC_PTR_PHY_R + 0x21)
#define PHY_ATDL_CNFG_INPSEL_MASK	GENMASK(3, 2) /* bits [3:2] */
#define PHY_ATDL_CNFG_INPSEL		0x3 /* delay line input source */

/* PHY DLL control settings */
#define PHY_DLL_CTRL_R			(DWC_MSHC_PTR_PHY_R + 0x24)
#define PHY_DLL_CTRL_DISABLE		0x0 /* PHY DLL is enabled */
#define PHY_DLL_CTRL_ENABLE		0x1 /* PHY DLL is disabled */

/* PHY DLL  configuration register 1 */
#define PHY_DLL_CNFG1_R			(DWC_MSHC_PTR_PHY_R + 0x25)
#define PHY_DLL_CNFG1_SLVDLY_MASK	GENMASK(5, 4) /* bits [5:4] */
#define PHY_DLL_CNFG1_SLVDLY		0x2 /* DLL slave update delay input */
#define PHY_DLL_CNFG1_WAITCYCLE		0x5 /* DLL wait cycle input */

/* PHY DLL configuration register 2 */
#define PHY_DLL_CNFG2_R			(DWC_MSHC_PTR_PHY_R + 0x26)
#define PHY_DLL_CNFG2_JUMPSTEP		0xa /* DLL jump step input */

/* PHY DLL master and slave delay line configuration settings */
#define PHY_DLLDL_CNFG_R		(DWC_MSHC_PTR_PHY_R + 0x28)
#define PHY_DLLDL_CNFG_SLV_INPSEL_MASK	GENMASK(6, 5) /* bits [6:5] */
#define PHY_DLLDL_CNFG_SLV_INPSEL	0x3 /* clock source select for slave DL */

/* Vendor specific Registers */
#define P_VENDOR_SPECIFIC_AREA		0x500

#define DWCMSHC_EMMC_CONTROL		0x2c
#define DWCMSHC_CARD_IS_EMMC		BIT(0)
#define DWCMSHC_ENHANCED_STROBE		BIT(8)
#define DWCMSHC_EMMC_ATCTRL		0x40
/* Tuning and auto-tuning fields in AT_CTRL_R control register */
#define AT_CTRL_AT_EN			BIT(0) /* autotuning is enabled */
#define AT_CTRL_CI_SEL			BIT(1) /* interval to drive center phase select */
#define AT_CTRL_SWIN_TH_EN		BIT(2) /* sampling window threshold enable */
#define AT_CTRL_RPT_TUNE_ERR		BIT(3) /* enable reporting framing errors */
#define AT_CTRL_SW_TUNE_EN		BIT(4) /* enable software managed tuning */
#define AT_CTRL_WIN_EDGE_SEL_MASK	GENMASK(11, 8) /* bits [11:8] */
#define AT_CTRL_WIN_EDGE_SEL		0xf /* sampling window edge select */
#define AT_CTRL_TUNE_CLK_STOP_EN	BIT(16) /* clocks stopped during phase code change */
#define AT_CTRL_PRE_CHANGE_DLY_MASK	GENMASK(18, 17) /* bits [18:17] */
#define AT_CTRL_PRE_CHANGE_DLY		0x1  /* 2-cycle latency */
#define AT_CTRL_POST_CHANGE_DLY_MASK	GENMASK(20, 19) /* bits [20:19] */
#define AT_CTRL_POST_CHANGE_DLY		0x3  /* 4-cycle latency */
#define AT_CTRL_SWIN_TH_VAL_MASK	GENMASK(31, 24) /* bits [31:24] */
#define AT_CTRL_SWIN_TH_VAL		0x9  /* sampling window threshold */

#define FLAG_IO_FIXED_1V8		BIT(0)

#define BOUNDARY_OK(addr, len) \
	(((addr) | (SZ_128M - 1)) == (((addr) + (len) - 1) | (SZ_128M - 1)))

struct snps_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	u16 delay_line;
	u16 flags;
};

/*
 * If DMA addr spans 128MB boundary, we split the DMA transfer into two
 * so that each DMA transfer doesn't exceed the boundary.
 */
void snps_sdhci_adma_write_desc(struct sdhci_host *host, void **desc,
				dma_addr_t addr, int len, bool end)
{
	int tmplen, offset;

	if (likely(!len || BOUNDARY_OK(addr, len))) {
		sdhci_adma_write_desc(host, desc, addr, len, end);
		return;
	}

	offset = addr & (SZ_128M - 1);
	tmplen = SZ_128M - offset;
	sdhci_adma_write_desc(host, desc, addr, tmplen, false);

	addr += tmplen;
	len -= tmplen;
	sdhci_adma_write_desc(host, desc, addr, len, end);
}

static void snps_sdhci_set_phy(struct sdhci_host *host)
{
	struct snps_sdhci_plat *plat = dev_get_plat(host->mmc->dev);
	u32 rxsel = PHY_PAD_RXSEL_3V3;
	u32 val;

	if (plat->flags & FLAG_IO_FIXED_1V8 ||
	    host->mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180)
		rxsel = PHY_PAD_RXSEL_1V8;

	/* deassert phy reset & set tx drive strength */
	val = PHY_CNFG_RSTN_DEASSERT;
	val |= FIELD_PREP(PHY_CNFG_PAD_SP_MASK, PHY_CNFG_PAD_SP);
	val |= FIELD_PREP(PHY_CNFG_PAD_SN_MASK, PHY_CNFG_PAD_SN);
	sdhci_writel(host, val, PHY_CNFG_R);

	/* disable delay line */
	sdhci_writeb(host, PHY_SDCLKDL_CNFG_UPDATE, PHY_SDCLKDL_CNFG_R);

	/* set delay line */
	sdhci_writeb(host, plat->delay_line, PHY_SDCLKDL_DC_R);
	sdhci_writeb(host, PHY_DLL_CNFG2_JUMPSTEP, PHY_DLL_CNFG2_R);

	/* enable delay lane */
	val = sdhci_readb(host, PHY_SDCLKDL_CNFG_R);
	val &= ~(PHY_SDCLKDL_CNFG_UPDATE);
	sdhci_writeb(host, val, PHY_SDCLKDL_CNFG_R);

	/* configure phy pads */
	val = rxsel;
	val |= FIELD_PREP(PHY_PAD_WEAKPULL_MASK, PHY_PAD_WEAKPULL_PULLUP);
	val |= FIELD_PREP(PHY_PAD_TXSLEW_CTRL_P_MASK, PHY_PAD_TXSLEW_CTRL_P);
	val |= FIELD_PREP(PHY_PAD_TXSLEW_CTRL_N_MASK, PHY_PAD_TXSLEW_CTRL_N);
	sdhci_writew(host, val, PHY_CMDPAD_CNFG_R);
	sdhci_writew(host, val, PHY_DATAPAD_CNFG_R);
	sdhci_writew(host, val, PHY_RSTNPAD_CNFG_R);

	val = FIELD_PREP(PHY_PAD_TXSLEW_CTRL_P_MASK, PHY_PAD_TXSLEW_CTRL_P);
	val |= FIELD_PREP(PHY_PAD_TXSLEW_CTRL_N_MASK, PHY_PAD_TXSLEW_CTRL_N);
	sdhci_writew(host, val, PHY_CLKPAD_CNFG_R);

	val = rxsel;
	val |= FIELD_PREP(PHY_PAD_WEAKPULL_MASK, PHY_PAD_WEAKPULL_PULLDOWN);
	val |= FIELD_PREP(PHY_PAD_TXSLEW_CTRL_P_MASK, PHY_PAD_TXSLEW_CTRL_P);
	val |= FIELD_PREP(PHY_PAD_TXSLEW_CTRL_N_MASK, PHY_PAD_TXSLEW_CTRL_N);
	sdhci_writew(host, val, PHY_STBPAD_CNFG_R);

	/* enable data strobe mode */
	if (plat->flags & FLAG_IO_FIXED_1V8 ||
	    host->mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180) {
		u8 sel = FIELD_PREP(PHY_DLLDL_CNFG_SLV_INPSEL_MASK, PHY_DLLDL_CNFG_SLV_INPSEL);

		sdhci_writeb(host, sel, PHY_DLLDL_CNFG_R);
	}

	/* enable phy dll */
	sdhci_writeb(host, PHY_DLL_CTRL_ENABLE, PHY_DLL_CTRL_R);

	sdhci_writeb(host, FIELD_PREP(PHY_DLL_CNFG1_SLVDLY_MASK, PHY_DLL_CNFG1_SLVDLY) |
		     PHY_DLL_CNFG1_WAITCYCLE, PHY_DLL_CNFG1_R);
}

static int snps_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct snps_sdhci_plat *plat = dev_get_plat(host->mmc->dev);
	struct mmc *mmc = host->mmc;
	u32 reg;

	reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	reg &= ~SDHCI_CTRL_UHS_MASK;

	switch (mmc->selected_mode) {
	case UHS_SDR50:
	case MMC_HS_52:
		reg |= SDHCI_CTRL_UHS_SDR50;
		break;
	case UHS_DDR50:
	case MMC_DDR_52:
		reg |= SDHCI_CTRL_UHS_DDR50;
		break;
	case UHS_SDR104:
	case MMC_HS_200:
		reg |= SDHCI_CTRL_UHS_SDR104;
		break;
	case MMC_HS_400:
	case MMC_HS_400_ES:
		reg |= DWCMSHC_CTRL_HS400;
		break;
	default:
		reg |= SDHCI_CTRL_UHS_SDR12;
	}

	if ((plat->flags & FLAG_IO_FIXED_1V8) ||
	    mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180)
		reg |= SDHCI_CTRL_VDD_180;
	else
		reg &= ~SDHCI_CTRL_VDD_180;

	sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);

	reg = sdhci_readw(host, P_VENDOR_SPECIFIC_AREA + DWCMSHC_EMMC_CONTROL);

	if (IS_MMC(mmc))
		reg |= DWCMSHC_CARD_IS_EMMC;
	else
		reg &= ~DWCMSHC_CARD_IS_EMMC;

	if (mmc->selected_mode == MMC_HS_400_ES)
		reg |= DWCMSHC_ENHANCED_STROBE;
	else
		reg &= ~DWCMSHC_ENHANCED_STROBE;

	sdhci_writeb(host, reg, P_VENDOR_SPECIFIC_AREA + DWCMSHC_EMMC_CONTROL);

	if (mmc->selected_mode == MMC_HS_400 ||
	    mmc->selected_mode == MMC_HS_400_ES)
		plat->delay_line = PHY_SDCLKDL_DC_HS400;
	else
		sdhci_writeb(host, 0, PHY_DLLDL_CNFG_R);

	snps_sdhci_set_phy(host);

	return 0;
}

static int snps_sdhci_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct sdhci_host *host = dev_get_priv(mmc->dev);
	char tuning_loop_counter = SDHCI_TUNING_LOOP_COUNT;
	struct mmc_cmd cmd;
	u32 ctrl, blk_size, val;
	int ret;

	sdhci_writeb(host, FIELD_PREP(PHY_ATDL_CNFG_INPSEL_MASK, PHY_ATDL_CNFG_INPSEL),
		     PHY_ATDL_CNFG_R);
	val = sdhci_readl(host, P_VENDOR_SPECIFIC_AREA + DWCMSHC_EMMC_ATCTRL);

	/*
	 * configure tuning settings:
	 *  - center phase select code driven in block gap interval
	 *  - disable reporting of framing errors
	 *  - disable software managed tuning
	 *  - disable user selection of sampling window edges,
	 *    instead tuning calculated edges are used
	 */
	val &= ~(AT_CTRL_CI_SEL | AT_CTRL_RPT_TUNE_ERR | AT_CTRL_SW_TUNE_EN |
		 FIELD_PREP(AT_CTRL_WIN_EDGE_SEL_MASK, AT_CTRL_WIN_EDGE_SEL));

	/*
	 * configure tuning settings:
	 *  - enable auto-tuning
	 *  - enable sampling window threshold
	 *  - stop clocks during phase code change
	 *  - set max latency in cycles between tx and rx clocks
	 *  - set max latency in cycles to switch output phase
	 *  - set max sampling window threshold value
	 */
	val |= AT_CTRL_AT_EN | AT_CTRL_SWIN_TH_EN | AT_CTRL_TUNE_CLK_STOP_EN;
	val |= FIELD_PREP(AT_CTRL_PRE_CHANGE_DLY_MASK, AT_CTRL_PRE_CHANGE_DLY);
	val |= FIELD_PREP(AT_CTRL_POST_CHANGE_DLY_MASK, AT_CTRL_POST_CHANGE_DLY);
	val |= FIELD_PREP(AT_CTRL_SWIN_TH_VAL_MASK, AT_CTRL_SWIN_TH_VAL);

	sdhci_writel(host, val, P_VENDOR_SPECIFIC_AREA + DWCMSHC_EMMC_ATCTRL);
	val = sdhci_readl(host, P_VENDOR_SPECIFIC_AREA + DWCMSHC_EMMC_ATCTRL);

	/* perform tuning */
	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl |= SDHCI_CTRL_EXEC_TUNING;
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

	blk_size = SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG, 64);
	if (opcode == MMC_CMD_SEND_TUNING_BLOCK_HS200 && mmc->bus_width == 8)
		blk_size = SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG, 128);
	sdhci_writew(host, blk_size, SDHCI_BLOCK_SIZE);
	sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

	cmd.cmdidx = opcode;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;

	do {
		ret = mmc_send_cmd(mmc, &cmd, NULL);
		ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		if (ret || tuning_loop_counter-- == 0)
			break;

	} while (ctrl & SDHCI_CTRL_EXEC_TUNING);

	if (ret || tuning_loop_counter < 0 || !(ctrl & SDHCI_CTRL_TUNED_CLK)) {
		if (!ret)
			ret = -EIO;
		printf("%s: Tuning failed: %d\n", __func__, ret);

		ctrl &= ~SDHCI_CTRL_TUNED_CLK;
		ctrl &= ~SDHCI_CTRL_EXEC_TUNING;
		sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
	}

	return ret;
}

static int snps_sdhci_set_enhanced_strobe(struct sdhci_host *host)
{
	return 0;
}

static const struct sdhci_ops snps_sdhci_ops = {
	.set_ios_post = snps_sdhci_set_ios_post,
	.platform_execute_tuning = snps_sdhci_execute_tuning,
	.set_enhanced_strobe = snps_sdhci_set_enhanced_strobe,
#if CONFIG_IS_ENABLED(MMC_SDHCI_ADMA_HELPERS)
	.adma_write_desc = snps_sdhci_adma_write_desc,
#endif
};

static int snps_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct snps_sdhci_plat *plat = dev_get_plat(dev);
	struct mmc_config *cfg = &plat->cfg;
	struct sdhci_host *host = dev_get_priv(dev);
	struct clk clk;
	int ret;

	plat->delay_line = PHY_SDCLKDL_DC_DEFAULT;

	ret = clk_get_by_name(dev, "core", &clk);
	if (ret)
		return ret;

	ret = clk_prepare_enable(&clk);
	if (ret)
		return ret;

	host->max_clk = clk_get_rate(&clk);

	host->ops = &snps_sdhci_ops;

	host->mmc = &plat->mmc;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	ret = sdhci_setup_cfg(cfg, host, cfg->f_max, EMMC_MIN_FREQ);
	if (ret)
		return ret;

	if ((dev_read_bool(dev, "mmc-ddr-1_8v")) ||
	    (dev_read_bool(dev, "mmc-hs200-1_8v")) ||
	    (dev_read_bool(dev, "mmc-hs400-1_8v")))
		plat->flags |= FLAG_IO_FIXED_1V8;
	else
		plat->flags &= ~FLAG_IO_FIXED_1V8;

	return sdhci_probe(dev);
}

static int snps_sdhci_of_to_plat(struct udevice *dev)
{
	struct snps_sdhci_plat *plat = dev_get_plat(dev);
	struct mmc_config *cfg = &plat->cfg;
	struct sdhci_host *host = dev_get_priv(dev);
	int ret;

	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);

	ret = mmc_of_parse(dev, cfg);
	if (ret)
		return ret;

	return 0;
}

static int snps_sdhci_bind(struct udevice *dev)
{
	struct snps_sdhci_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id snps_sdhci_ids[] = {
	{ .compatible = "thead,th1520-dwcmshc" }
};

U_BOOT_DRIVER(snps_sdhci_drv) = {
	.name		= "snps_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= snps_sdhci_ids,
	.of_to_plat	= snps_sdhci_of_to_plat,
	.ops		= &sdhci_ops,
	.bind		= snps_sdhci_bind,
	.probe		= snps_sdhci_probe,
	.priv_auto = sizeof(struct sdhci_host),
	.plat_auto = sizeof(struct snps_sdhci_plat),
};
