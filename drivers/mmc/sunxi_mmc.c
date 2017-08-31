/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Aaron <leafy.myeh@allwinnertech.com>
 *
 * MMC driver for allwinner sunxi platform.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm-generic/gpio.h>

struct sunxi_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct sunxi_mmc_priv {
	unsigned mmc_no;
	uint32_t *mclkreg;
	unsigned fatal_err;
	struct gpio_desc cd_gpio;	/* Change Detect GPIO */
	struct sunxi_mmc *reg;
	struct mmc_config cfg;
};

#if !CONFIG_IS_ENABLED(DM_MMC)
/* support 4 mmc hosts */
struct sunxi_mmc_priv mmc_host[4];

static int sunxi_mmc_getcd_gpio(int sdc_no)
{
	switch (sdc_no) {
	case 0: return sunxi_name_to_gpio(CONFIG_MMC0_CD_PIN);
	case 1: return sunxi_name_to_gpio(CONFIG_MMC1_CD_PIN);
	case 2: return sunxi_name_to_gpio(CONFIG_MMC2_CD_PIN);
	case 3: return sunxi_name_to_gpio(CONFIG_MMC3_CD_PIN);
	}
	return -EINVAL;
}

static int mmc_resource_init(int sdc_no)
{
	struct sunxi_mmc_priv *priv = &mmc_host[sdc_no];
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	int cd_pin, ret = 0;

	debug("init mmc %d resource\n", sdc_no);

	switch (sdc_no) {
	case 0:
		priv->reg = (struct sunxi_mmc *)SUNXI_MMC0_BASE;
		priv->mclkreg = &ccm->sd0_clk_cfg;
		break;
	case 1:
		priv->reg = (struct sunxi_mmc *)SUNXI_MMC1_BASE;
		priv->mclkreg = &ccm->sd1_clk_cfg;
		break;
	case 2:
		priv->reg = (struct sunxi_mmc *)SUNXI_MMC2_BASE;
		priv->mclkreg = &ccm->sd2_clk_cfg;
		break;
	case 3:
		priv->reg = (struct sunxi_mmc *)SUNXI_MMC3_BASE;
		priv->mclkreg = &ccm->sd3_clk_cfg;
		break;
	default:
		printf("Wrong mmc number %d\n", sdc_no);
		return -1;
	}
	priv->mmc_no = sdc_no;

	cd_pin = sunxi_mmc_getcd_gpio(sdc_no);
	if (cd_pin >= 0) {
		ret = gpio_request(cd_pin, "mmc_cd");
		if (!ret) {
			sunxi_gpio_set_pull(cd_pin, SUNXI_GPIO_PULL_UP);
			ret = gpio_direction_input(cd_pin);
		}
	}

	return ret;
}
#endif

static int mmc_set_mod_clk(struct sunxi_mmc_priv *priv, unsigned int hz)
{
	unsigned int pll, pll_hz, div, n, oclk_dly, sclk_dly;
	bool new_mode = false;
	u32 val = 0;

	if (IS_ENABLED(CONFIG_MMC_SUNXI_HAS_NEW_MODE) && (priv->mmc_no == 2))
		new_mode = true;

	/*
	 * The MMC clock has an extra /2 post-divider when operating in the new
	 * mode.
	 */
	if (new_mode)
		hz = hz * 2;

	if (hz <= 24000000) {
		pll = CCM_MMC_CTRL_OSCM24;
		pll_hz = 24000000;
	} else {
#ifdef CONFIG_MACH_SUN9I
		pll = CCM_MMC_CTRL_PLL_PERIPH0;
		pll_hz = clock_get_pll4_periph0();
#else
		pll = CCM_MMC_CTRL_PLL6;
		pll_hz = clock_get_pll6();
#endif
	}

	div = pll_hz / hz;
	if (pll_hz % hz)
		div++;

	n = 0;
	while (div > 16) {
		n++;
		div = (div + 1) / 2;
	}

	if (n > 3) {
		printf("mmc %u error cannot set clock to %u\n", priv->mmc_no,
		       hz);
		return -1;
	}

	/* determine delays */
	if (hz <= 400000) {
		oclk_dly = 0;
		sclk_dly = 0;
	} else if (hz <= 25000000) {
		oclk_dly = 0;
		sclk_dly = 5;
#ifdef CONFIG_MACH_SUN9I
	} else if (hz <= 50000000) {
		oclk_dly = 5;
		sclk_dly = 4;
	} else {
		/* hz > 50000000 */
		oclk_dly = 2;
		sclk_dly = 4;
#else
	} else if (hz <= 50000000) {
		oclk_dly = 3;
		sclk_dly = 4;
	} else {
		/* hz > 50000000 */
		oclk_dly = 1;
		sclk_dly = 4;
#endif
	}

	if (new_mode) {
#ifdef CONFIG_MMC_SUNXI_HAS_NEW_MODE
		val = CCM_MMC_CTRL_MODE_SEL_NEW;
		setbits_le32(&priv->reg->ntsr, SUNXI_MMC_NTSR_MODE_SEL_NEW);
#endif
	} else {
		val = CCM_MMC_CTRL_OCLK_DLY(oclk_dly) |
			CCM_MMC_CTRL_SCLK_DLY(sclk_dly);
	}

	writel(CCM_MMC_CTRL_ENABLE| pll | CCM_MMC_CTRL_N(n) |
	       CCM_MMC_CTRL_M(div) | val, priv->mclkreg);

	debug("mmc %u set mod-clk req %u parent %u n %u m %u rate %u\n",
	      priv->mmc_no, hz, pll_hz, 1u << n, div, pll_hz / (1u << n) / div);

	return 0;
}

static int mmc_update_clk(struct sunxi_mmc_priv *priv)
{
	unsigned int cmd;
	unsigned timeout_msecs = 2000;

	cmd = SUNXI_MMC_CMD_START |
	      SUNXI_MMC_CMD_UPCLK_ONLY |
	      SUNXI_MMC_CMD_WAIT_PRE_OVER;
	writel(cmd, &priv->reg->cmd);
	while (readl(&priv->reg->cmd) & SUNXI_MMC_CMD_START) {
		if (!timeout_msecs--)
			return -1;
		udelay(1000);
	}

	/* clock update sets various irq status bits, clear these */
	writel(readl(&priv->reg->rint), &priv->reg->rint);

	return 0;
}

static int mmc_config_clock(struct sunxi_mmc_priv *priv, struct mmc *mmc)
{
	unsigned rval = readl(&priv->reg->clkcr);

	/* Disable Clock */
	rval &= ~SUNXI_MMC_CLK_ENABLE;
	writel(rval, &priv->reg->clkcr);
	if (mmc_update_clk(priv))
		return -1;

	/* Set mod_clk to new rate */
	if (mmc_set_mod_clk(priv, mmc->clock))
		return -1;

	/* Clear internal divider */
	rval &= ~SUNXI_MMC_CLK_DIVIDER_MASK;
	writel(rval, &priv->reg->clkcr);

	/* Re-enable Clock */
	rval |= SUNXI_MMC_CLK_ENABLE;
	writel(rval, &priv->reg->clkcr);
	if (mmc_update_clk(priv))
		return -1;

	return 0;
}

static int sunxi_mmc_set_ios_common(struct sunxi_mmc_priv *priv,
				    struct mmc *mmc)
{
	debug("set ios: bus_width: %x, clock: %d\n",
	      mmc->bus_width, mmc->clock);

	/* Change clock first */
	if (mmc->clock && mmc_config_clock(priv, mmc) != 0) {
		priv->fatal_err = 1;
		return -EINVAL;
	}

	/* Change bus width */
	if (mmc->bus_width == 8)
		writel(0x2, &priv->reg->width);
	else if (mmc->bus_width == 4)
		writel(0x1, &priv->reg->width);
	else
		writel(0x0, &priv->reg->width);

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
static int sunxi_mmc_core_init(struct mmc *mmc)
{
	struct sunxi_mmc_priv *priv = mmc->priv;

	/* Reset controller */
	writel(SUNXI_MMC_GCTRL_RESET, &priv->reg->gctrl);
	udelay(1000);

	return 0;
}
#endif

static int mmc_trans_data_by_cpu(struct sunxi_mmc_priv *priv, struct mmc *mmc,
				 struct mmc_data *data)
{
	const int reading = !!(data->flags & MMC_DATA_READ);
	const uint32_t status_bit = reading ? SUNXI_MMC_STATUS_FIFO_EMPTY :
					      SUNXI_MMC_STATUS_FIFO_FULL;
	unsigned i;
	unsigned *buff = (unsigned int *)(reading ? data->dest : data->src);
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned timeout_usecs = (byte_cnt >> 8) * 1000;
	if (timeout_usecs < 2000000)
		timeout_usecs = 2000000;

	/* Always read / write data through the CPU */
	setbits_le32(&priv->reg->gctrl, SUNXI_MMC_GCTRL_ACCESS_BY_AHB);

	for (i = 0; i < (byte_cnt >> 2); i++) {
		while (readl(&priv->reg->status) & status_bit) {
			if (!timeout_usecs--)
				return -1;
			udelay(1);
		}

		if (reading)
			buff[i] = readl(&priv->reg->fifo);
		else
			writel(buff[i], &priv->reg->fifo);
	}

	return 0;
}

static int mmc_rint_wait(struct sunxi_mmc_priv *priv, struct mmc *mmc,
			 uint timeout_msecs, uint done_bit, const char *what)
{
	unsigned int status;

	do {
		status = readl(&priv->reg->rint);
		if (!timeout_msecs-- ||
		    (status & SUNXI_MMC_RINT_INTERRUPT_ERROR_BIT)) {
			debug("%s timeout %x\n", what,
			      status & SUNXI_MMC_RINT_INTERRUPT_ERROR_BIT);
			return -ETIMEDOUT;
		}
		udelay(1000);
	} while (!(status & done_bit));

	return 0;
}

static int sunxi_mmc_send_cmd_common(struct sunxi_mmc_priv *priv,
				     struct mmc *mmc, struct mmc_cmd *cmd,
				     struct mmc_data *data)
{
	unsigned int cmdval = SUNXI_MMC_CMD_START;
	unsigned int timeout_msecs;
	int error = 0;
	unsigned int status = 0;
	unsigned int bytecnt = 0;

	if (priv->fatal_err)
		return -1;
	if (cmd->resp_type & MMC_RSP_BUSY)
		debug("mmc cmd %d check rsp busy\n", cmd->cmdidx);
	if (cmd->cmdidx == 12)
		return 0;

	if (!cmd->cmdidx)
		cmdval |= SUNXI_MMC_CMD_SEND_INIT_SEQ;
	if (cmd->resp_type & MMC_RSP_PRESENT)
		cmdval |= SUNXI_MMC_CMD_RESP_EXPIRE;
	if (cmd->resp_type & MMC_RSP_136)
		cmdval |= SUNXI_MMC_CMD_LONG_RESPONSE;
	if (cmd->resp_type & MMC_RSP_CRC)
		cmdval |= SUNXI_MMC_CMD_CHK_RESPONSE_CRC;

	if (data) {
		if ((u32)(long)data->dest & 0x3) {
			error = -1;
			goto out;
		}

		cmdval |= SUNXI_MMC_CMD_DATA_EXPIRE|SUNXI_MMC_CMD_WAIT_PRE_OVER;
		if (data->flags & MMC_DATA_WRITE)
			cmdval |= SUNXI_MMC_CMD_WRITE;
		if (data->blocks > 1)
			cmdval |= SUNXI_MMC_CMD_AUTO_STOP;
		writel(data->blocksize, &priv->reg->blksz);
		writel(data->blocks * data->blocksize, &priv->reg->bytecnt);
	}

	debug("mmc %d, cmd %d(0x%08x), arg 0x%08x\n", priv->mmc_no,
	      cmd->cmdidx, cmdval | cmd->cmdidx, cmd->cmdarg);
	writel(cmd->cmdarg, &priv->reg->arg);

	if (!data)
		writel(cmdval | cmd->cmdidx, &priv->reg->cmd);

	/*
	 * transfer data and check status
	 * STATREG[2] : FIFO empty
	 * STATREG[3] : FIFO full
	 */
	if (data) {
		int ret = 0;

		bytecnt = data->blocksize * data->blocks;
		debug("trans data %d bytes\n", bytecnt);
		writel(cmdval | cmd->cmdidx, &priv->reg->cmd);
		ret = mmc_trans_data_by_cpu(priv, mmc, data);
		if (ret) {
			error = readl(&priv->reg->rint) &
				SUNXI_MMC_RINT_INTERRUPT_ERROR_BIT;
			error = -ETIMEDOUT;
			goto out;
		}
	}

	error = mmc_rint_wait(priv, mmc, 1000, SUNXI_MMC_RINT_COMMAND_DONE,
			      "cmd");
	if (error)
		goto out;

	if (data) {
		timeout_msecs = 120;
		debug("cacl timeout %x msec\n", timeout_msecs);
		error = mmc_rint_wait(priv, mmc, timeout_msecs,
				      data->blocks > 1 ?
				      SUNXI_MMC_RINT_AUTO_COMMAND_DONE :
				      SUNXI_MMC_RINT_DATA_OVER,
				      "data");
		if (error)
			goto out;
	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		timeout_msecs = 2000;
		do {
			status = readl(&priv->reg->status);
			if (!timeout_msecs--) {
				debug("busy timeout\n");
				error = -ETIMEDOUT;
				goto out;
			}
			udelay(1000);
		} while (status & SUNXI_MMC_STATUS_CARD_DATA_BUSY);
	}

	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = readl(&priv->reg->resp3);
		cmd->response[1] = readl(&priv->reg->resp2);
		cmd->response[2] = readl(&priv->reg->resp1);
		cmd->response[3] = readl(&priv->reg->resp0);
		debug("mmc resp 0x%08x 0x%08x 0x%08x 0x%08x\n",
		      cmd->response[3], cmd->response[2],
		      cmd->response[1], cmd->response[0]);
	} else {
		cmd->response[0] = readl(&priv->reg->resp0);
		debug("mmc resp 0x%08x\n", cmd->response[0]);
	}
out:
	if (error < 0) {
		writel(SUNXI_MMC_GCTRL_RESET, &priv->reg->gctrl);
		mmc_update_clk(priv);
	}
	writel(0xffffffff, &priv->reg->rint);
	writel(readl(&priv->reg->gctrl) | SUNXI_MMC_GCTRL_FIFO_RESET,
	       &priv->reg->gctrl);

	return error;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
static int sunxi_mmc_set_ios_legacy(struct mmc *mmc)
{
	struct sunxi_mmc_priv *priv = mmc->priv;

	return sunxi_mmc_set_ios_common(priv, mmc);
}

static int sunxi_mmc_send_cmd_legacy(struct mmc *mmc, struct mmc_cmd *cmd,
				     struct mmc_data *data)
{
	struct sunxi_mmc_priv *priv = mmc->priv;

	return sunxi_mmc_send_cmd_common(priv, mmc, cmd, data);
}

static int sunxi_mmc_getcd_legacy(struct mmc *mmc)
{
	struct sunxi_mmc_priv *priv = mmc->priv;
	int cd_pin;

	cd_pin = sunxi_mmc_getcd_gpio(priv->mmc_no);
	if (cd_pin < 0)
		return 1;

	return !gpio_get_value(cd_pin);
}

static const struct mmc_ops sunxi_mmc_ops = {
	.send_cmd	= sunxi_mmc_send_cmd_legacy,
	.set_ios	= sunxi_mmc_set_ios_legacy,
	.init		= sunxi_mmc_core_init,
	.getcd		= sunxi_mmc_getcd_legacy,
};

struct mmc *sunxi_mmc_init(int sdc_no)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_mmc_priv *priv = &mmc_host[sdc_no];
	struct mmc_config *cfg = &priv->cfg;
	int ret;

	memset(priv, '\0', sizeof(struct sunxi_mmc_priv));

	cfg->name = "SUNXI SD/MMC";
	cfg->ops  = &sunxi_mmc_ops;

	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	cfg->host_caps = MMC_MODE_4BIT;
#if defined(CONFIG_MACH_SUN50I) || defined(CONFIG_MACH_SUN8I)
	if (sdc_no == 2)
		cfg->host_caps = MMC_MODE_8BIT;
#endif
	cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	cfg->f_min = 400000;
	cfg->f_max = 52000000;

	if (mmc_resource_init(sdc_no) != 0)
		return NULL;

	/* config ahb clock */
	debug("init mmc %d clock and io\n", sdc_no);
	setbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_MMC(sdc_no));

#ifdef CONFIG_SUNXI_GEN_SUN6I
	/* unassert reset */
	setbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_MMC(sdc_no));
#endif
#if defined(CONFIG_MACH_SUN9I)
	/* sun9i has a mmc-common module, also set the gate and reset there */
	writel(SUNXI_MMC_COMMON_CLK_GATE | SUNXI_MMC_COMMON_RESET,
	       SUNXI_MMC_COMMON_BASE + 4 * sdc_no);
#endif
	ret = mmc_set_mod_clk(priv, 24000000);
	if (ret)
		return NULL;

	return mmc_create(cfg, priv);
}
#else

static int sunxi_mmc_set_ios(struct udevice *dev)
{
	struct sunxi_mmc_plat *plat = dev_get_platdata(dev);
	struct sunxi_mmc_priv *priv = dev_get_priv(dev);

	return sunxi_mmc_set_ios_common(priv, &plat->mmc);
}

static int sunxi_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct sunxi_mmc_plat *plat = dev_get_platdata(dev);
	struct sunxi_mmc_priv *priv = dev_get_priv(dev);

	return sunxi_mmc_send_cmd_common(priv, &plat->mmc, cmd, data);
}

static int sunxi_mmc_getcd(struct udevice *dev)
{
	struct sunxi_mmc_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->cd_gpio))
		return dm_gpio_get_value(&priv->cd_gpio);

	return 1;
}

static const struct dm_mmc_ops sunxi_mmc_ops = {
	.send_cmd	= sunxi_mmc_send_cmd,
	.set_ios	= sunxi_mmc_set_ios,
	.get_cd		= sunxi_mmc_getcd,
};

static int sunxi_mmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sunxi_mmc_plat *plat = dev_get_platdata(dev);
	struct sunxi_mmc_priv *priv = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	struct ofnode_phandle_args args;
	u32 *gate_reg;
	int bus_width, ret;

	cfg->name = dev->name;
	bus_width = dev_read_u32_default(dev, "bus-width", 1);

	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	cfg->host_caps = 0;
	if (bus_width == 8)
		cfg->host_caps |= MMC_MODE_8BIT;
	if (bus_width >= 4)
		cfg->host_caps |= MMC_MODE_4BIT;
	cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	cfg->f_min = 400000;
	cfg->f_max = 52000000;

	priv->reg = (void *)dev_read_addr(dev);

	/* We don't have a sunxi clock driver so find the clock address here */
	ret = dev_read_phandle_with_args(dev, "clocks", "#clock-cells", 0,
					  1, &args);
	if (ret)
		return ret;
	priv->mclkreg = (u32 *)ofnode_get_addr(args.node);

	ret = dev_read_phandle_with_args(dev, "clocks", "#clock-cells", 0,
					  0, &args);
	if (ret)
		return ret;
	gate_reg = (u32 *)ofnode_get_addr(args.node);
	setbits_le32(gate_reg, 1 << args.args[0]);
	priv->mmc_no = args.args[0] - 8;

	ret = mmc_set_mod_clk(priv, 24000000);
	if (ret)
		return ret;

	/* This GPIO is optional */
	if (!gpio_request_by_name(dev, "cd-gpios", 0, &priv->cd_gpio,
				  GPIOD_IS_IN)) {
		int cd_pin = gpio_get_number(&priv->cd_gpio);

		sunxi_gpio_set_pull(cd_pin, SUNXI_GPIO_PULL_UP);
	}

	upriv->mmc = &plat->mmc;

	/* Reset controller */
	writel(SUNXI_MMC_GCTRL_RESET, &priv->reg->gctrl);
	udelay(1000);

	return 0;
}

static int sunxi_mmc_bind(struct udevice *dev)
{
	struct sunxi_mmc_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id sunxi_mmc_ids[] = {
	{ .compatible = "allwinner,sun5i-a13-mmc" },
	{ }
};

U_BOOT_DRIVER(sunxi_mmc_drv) = {
	.name		= "sunxi_mmc",
	.id		= UCLASS_MMC,
	.of_match	= sunxi_mmc_ids,
	.bind		= sunxi_mmc_bind,
	.probe		= sunxi_mmc_probe,
	.ops		= &sunxi_mmc_ops,
	.platdata_auto_alloc_size = sizeof(struct sunxi_mmc_plat),
	.priv_auto_alloc_size = sizeof(struct sunxi_mmc_priv),
};
#endif
