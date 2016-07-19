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
#include <errno.h>
#include <malloc.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm-generic/gpio.h>

struct sunxi_mmc_host {
	unsigned mmc_no;
	uint32_t *mclkreg;
	unsigned fatal_err;
	struct sunxi_mmc *reg;
	struct mmc_config cfg;
};

/* support 4 mmc hosts */
struct sunxi_mmc_host mmc_host[4];

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
	struct sunxi_mmc_host *mmchost = &mmc_host[sdc_no];
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	int cd_pin, ret = 0;

	debug("init mmc %d resource\n", sdc_no);

	switch (sdc_no) {
	case 0:
		mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC0_BASE;
		mmchost->mclkreg = &ccm->sd0_clk_cfg;
		break;
	case 1:
		mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC1_BASE;
		mmchost->mclkreg = &ccm->sd1_clk_cfg;
		break;
	case 2:
		mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC2_BASE;
		mmchost->mclkreg = &ccm->sd2_clk_cfg;
		break;
	case 3:
		mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC3_BASE;
		mmchost->mclkreg = &ccm->sd3_clk_cfg;
		break;
	default:
		printf("Wrong mmc number %d\n", sdc_no);
		return -1;
	}
	mmchost->mmc_no = sdc_no;

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

static int mmc_set_mod_clk(struct sunxi_mmc_host *mmchost, unsigned int hz)
{
	unsigned int pll, pll_hz, div, n, oclk_dly, sclk_dly;

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
		printf("mmc %u error cannot set clock to %u\n",
		       mmchost->mmc_no, hz);
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

	writel(CCM_MMC_CTRL_ENABLE | pll | CCM_MMC_CTRL_SCLK_DLY(sclk_dly) |
	       CCM_MMC_CTRL_N(n) | CCM_MMC_CTRL_OCLK_DLY(oclk_dly) |
	       CCM_MMC_CTRL_M(div), mmchost->mclkreg);

	debug("mmc %u set mod-clk req %u parent %u n %u m %u rate %u\n",
	      mmchost->mmc_no, hz, pll_hz, 1u << n, div,
	      pll_hz / (1u << n) / div);

	return 0;
}

static int mmc_clk_io_on(int sdc_no)
{
	struct sunxi_mmc_host *mmchost = &mmc_host[sdc_no];
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	debug("init mmc %d clock and io\n", sdc_no);

	/* config ahb clock */
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

	return mmc_set_mod_clk(mmchost, 24000000);
}

static int mmc_update_clk(struct mmc *mmc)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	unsigned int cmd;
	unsigned timeout_msecs = 2000;

	cmd = SUNXI_MMC_CMD_START |
	      SUNXI_MMC_CMD_UPCLK_ONLY |
	      SUNXI_MMC_CMD_WAIT_PRE_OVER;
	writel(cmd, &mmchost->reg->cmd);
	while (readl(&mmchost->reg->cmd) & SUNXI_MMC_CMD_START) {
		if (!timeout_msecs--)
			return -1;
		udelay(1000);
	}

	/* clock update sets various irq status bits, clear these */
	writel(readl(&mmchost->reg->rint), &mmchost->reg->rint);

	return 0;
}

static int mmc_config_clock(struct mmc *mmc)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	unsigned rval = readl(&mmchost->reg->clkcr);

	/* Disable Clock */
	rval &= ~SUNXI_MMC_CLK_ENABLE;
	writel(rval, &mmchost->reg->clkcr);
	if (mmc_update_clk(mmc))
		return -1;

	/* Set mod_clk to new rate */
	if (mmc_set_mod_clk(mmchost, mmc->clock))
		return -1;

	/* Clear internal divider */
	rval &= ~SUNXI_MMC_CLK_DIVIDER_MASK;
	writel(rval, &mmchost->reg->clkcr);

	/* Re-enable Clock */
	rval |= SUNXI_MMC_CLK_ENABLE;
	writel(rval, &mmchost->reg->clkcr);
	if (mmc_update_clk(mmc))
		return -1;

	return 0;
}

static void sunxi_mmc_set_ios(struct mmc *mmc)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;

	debug("set ios: bus_width: %x, clock: %d\n",
	      mmc->bus_width, mmc->clock);

	/* Change clock first */
	if (mmc->clock && mmc_config_clock(mmc) != 0) {
		mmchost->fatal_err = 1;
		return;
	}

	/* Change bus width */
	if (mmc->bus_width == 8)
		writel(0x2, &mmchost->reg->width);
	else if (mmc->bus_width == 4)
		writel(0x1, &mmchost->reg->width);
	else
		writel(0x0, &mmchost->reg->width);
}

static int sunxi_mmc_core_init(struct mmc *mmc)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;

	/* Reset controller */
	writel(SUNXI_MMC_GCTRL_RESET, &mmchost->reg->gctrl);
	udelay(1000);

	return 0;
}

static int mmc_trans_data_by_cpu(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
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
	setbits_le32(&mmchost->reg->gctrl, SUNXI_MMC_GCTRL_ACCESS_BY_AHB);

	for (i = 0; i < (byte_cnt >> 2); i++) {
		while (readl(&mmchost->reg->status) & status_bit) {
			if (!timeout_usecs--)
				return -1;
			udelay(1);
		}

		if (reading)
			buff[i] = readl(&mmchost->reg->fifo);
		else
			writel(buff[i], &mmchost->reg->fifo);
	}

	return 0;
}

static int mmc_rint_wait(struct mmc *mmc, unsigned int timeout_msecs,
			 unsigned int done_bit, const char *what)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	unsigned int status;

	do {
		status = readl(&mmchost->reg->rint);
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

static int sunxi_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	unsigned int cmdval = SUNXI_MMC_CMD_START;
	unsigned int timeout_msecs;
	int error = 0;
	unsigned int status = 0;
	unsigned int bytecnt = 0;

	if (mmchost->fatal_err)
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
		writel(data->blocksize, &mmchost->reg->blksz);
		writel(data->blocks * data->blocksize, &mmchost->reg->bytecnt);
	}

	debug("mmc %d, cmd %d(0x%08x), arg 0x%08x\n", mmchost->mmc_no,
	      cmd->cmdidx, cmdval | cmd->cmdidx, cmd->cmdarg);
	writel(cmd->cmdarg, &mmchost->reg->arg);

	if (!data)
		writel(cmdval | cmd->cmdidx, &mmchost->reg->cmd);

	/*
	 * transfer data and check status
	 * STATREG[2] : FIFO empty
	 * STATREG[3] : FIFO full
	 */
	if (data) {
		int ret = 0;

		bytecnt = data->blocksize * data->blocks;
		debug("trans data %d bytes\n", bytecnt);
		writel(cmdval | cmd->cmdidx, &mmchost->reg->cmd);
		ret = mmc_trans_data_by_cpu(mmc, data);
		if (ret) {
			error = readl(&mmchost->reg->rint) & \
				SUNXI_MMC_RINT_INTERRUPT_ERROR_BIT;
			error = -ETIMEDOUT;
			goto out;
		}
	}

	error = mmc_rint_wait(mmc, 1000, SUNXI_MMC_RINT_COMMAND_DONE, "cmd");
	if (error)
		goto out;

	if (data) {
		timeout_msecs = 120;
		debug("cacl timeout %x msec\n", timeout_msecs);
		error = mmc_rint_wait(mmc, timeout_msecs,
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
			status = readl(&mmchost->reg->status);
			if (!timeout_msecs--) {
				debug("busy timeout\n");
				error = -ETIMEDOUT;
				goto out;
			}
			udelay(1000);
		} while (status & SUNXI_MMC_STATUS_CARD_DATA_BUSY);
	}

	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = readl(&mmchost->reg->resp3);
		cmd->response[1] = readl(&mmchost->reg->resp2);
		cmd->response[2] = readl(&mmchost->reg->resp1);
		cmd->response[3] = readl(&mmchost->reg->resp0);
		debug("mmc resp 0x%08x 0x%08x 0x%08x 0x%08x\n",
		      cmd->response[3], cmd->response[2],
		      cmd->response[1], cmd->response[0]);
	} else {
		cmd->response[0] = readl(&mmchost->reg->resp0);
		debug("mmc resp 0x%08x\n", cmd->response[0]);
	}
out:
	if (error < 0) {
		writel(SUNXI_MMC_GCTRL_RESET, &mmchost->reg->gctrl);
		mmc_update_clk(mmc);
	}
	writel(0xffffffff, &mmchost->reg->rint);
	writel(readl(&mmchost->reg->gctrl) | SUNXI_MMC_GCTRL_FIFO_RESET,
	       &mmchost->reg->gctrl);

	return error;
}

static int sunxi_mmc_getcd(struct mmc *mmc)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	int cd_pin;

	cd_pin = sunxi_mmc_getcd_gpio(mmchost->mmc_no);
	if (cd_pin < 0)
		return 1;

	return !gpio_get_value(cd_pin);
}

static const struct mmc_ops sunxi_mmc_ops = {
	.send_cmd	= sunxi_mmc_send_cmd,
	.set_ios	= sunxi_mmc_set_ios,
	.init		= sunxi_mmc_core_init,
	.getcd		= sunxi_mmc_getcd,
};

struct mmc *sunxi_mmc_init(int sdc_no)
{
	struct mmc_config *cfg = &mmc_host[sdc_no].cfg;

	memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));

	cfg->name = "SUNXI SD/MMC";
	cfg->ops  = &sunxi_mmc_ops;

	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	cfg->host_caps = MMC_MODE_4BIT;
#ifdef CONFIG_MACH_SUN50I
	if (sdc_no == 2)
		cfg->host_caps = MMC_MODE_8BIT;
#endif
	cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	cfg->f_min = 400000;
	cfg->f_max = 52000000;

	if (mmc_resource_init(sdc_no) != 0)
		return NULL;

	mmc_clk_io_on(sdc_no);

	return mmc_create(cfg, &mmc_host[sdc_no]);
}
