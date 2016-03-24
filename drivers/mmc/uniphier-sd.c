/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <fdtdec.h>
#include <mapmem.h>
#include <mmc.h>
#include <dm/device.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/unaligned.h>
#include <asm/dma-mapping.h>

DECLARE_GLOBAL_DATA_PTR;

#define UNIPHIER_SD_CMD			0x000	/* command */
#define   UNIPHIER_SD_CMD_NOSTOP	BIT(14)	/* No automatic CMD12 issue */
#define   UNIPHIER_SD_CMD_MULTI		BIT(13)	/* multiple block transfer */
#define   UNIPHIER_SD_CMD_RD		BIT(12)	/* 1: read, 0: write */
#define   UNIPHIER_SD_CMD_DATA		BIT(11)	/* data transfer */
#define   UNIPHIER_SD_CMD_APP		BIT(6)	/* ACMD preceded by CMD55 */
#define   UNIPHIER_SD_CMD_NORMAL	(0 << 8)/* auto-detect of resp-type */
#define   UNIPHIER_SD_CMD_RSP_NONE	(3 << 8)/* response: none */
#define   UNIPHIER_SD_CMD_RSP_R1	(4 << 8)/* response: R1, R5, R6, R7 */
#define   UNIPHIER_SD_CMD_RSP_R1B	(5 << 8)/* response: R1b, R5b */
#define   UNIPHIER_SD_CMD_RSP_R2	(6 << 8)/* response: R2 */
#define   UNIPHIER_SD_CMD_RSP_R3	(7 << 8)/* response: R3, R4 */
#define UNIPHIER_SD_ARG			0x008	/* command argument */
#define UNIPHIER_SD_STOP		0x010	/* stop action control */
#define   UNIPHIER_SD_STOP_SEC		BIT(8)	/* use sector count */
#define   UNIPHIER_SD_STOP_STP		BIT(0)	/* issue CMD12 */
#define UNIPHIER_SD_SECCNT		0x014	/* sector counter */
#define UNIPHIER_SD_RSP10		0x018	/* response[39:8] */
#define UNIPHIER_SD_RSP32		0x020	/* response[71:40] */
#define UNIPHIER_SD_RSP54		0x028	/* response[103:72] */
#define UNIPHIER_SD_RSP76		0x030	/* response[127:104] */
#define UNIPHIER_SD_INFO1		0x038	/* IRQ status 1 */
#define   UNIPHIER_SD_INFO1_CD		BIT(5)	/* state of card detect */
#define   UNIPHIER_SD_INFO1_INSERT	BIT(4)	/* card inserted */
#define   UNIPHIER_SD_INFO1_REMOVE	BIT(3)	/* card removed */
#define   UNIPHIER_SD_INFO1_CMP		BIT(2)	/* data complete */
#define   UNIPHIER_SD_INFO1_RSP		BIT(0)	/* response complete */
#define UNIPHIER_SD_INFO2		0x03c	/* IRQ status 2 */
#define   UNIPHIER_SD_INFO2_ERR_ILA	BIT(15)	/* illegal access err */
#define   UNIPHIER_SD_INFO2_CBSY	BIT(14)	/* command busy */
#define   UNIPHIER_SD_INFO2_BWE		BIT(9)	/* write buffer ready */
#define   UNIPHIER_SD_INFO2_BRE		BIT(8)	/* read buffer ready */
#define   UNIPHIER_SD_INFO2_DAT0	BIT(7)	/* SDDAT0 */
#define   UNIPHIER_SD_INFO2_ERR_RTO	BIT(6)	/* response time out */
#define   UNIPHIER_SD_INFO2_ERR_ILR	BIT(5)	/* illegal read err */
#define   UNIPHIER_SD_INFO2_ERR_ILW	BIT(4)	/* illegal write err */
#define   UNIPHIER_SD_INFO2_ERR_TO	BIT(3)	/* time out error */
#define   UNIPHIER_SD_INFO2_ERR_END	BIT(2)	/* END bit error */
#define   UNIPHIER_SD_INFO2_ERR_CRC	BIT(1)	/* CRC error */
#define   UNIPHIER_SD_INFO2_ERR_IDX	BIT(0)	/* cmd index error */
#define UNIPHIER_SD_INFO1_MASK		0x040
#define UNIPHIER_SD_INFO2_MASK		0x044
#define UNIPHIER_SD_CLKCTL		0x048	/* clock divisor */
#define   UNIPHIER_SD_CLKCTL_DIV_MASK	0x104ff
#define   UNIPHIER_SD_CLKCTL_DIV1024	BIT(16)	/* SDCLK = CLK / 1024 */
#define   UNIPHIER_SD_CLKCTL_DIV512	BIT(7)	/* SDCLK = CLK / 512 */
#define   UNIPHIER_SD_CLKCTL_DIV256	BIT(6)	/* SDCLK = CLK / 256 */
#define   UNIPHIER_SD_CLKCTL_DIV128	BIT(5)	/* SDCLK = CLK / 128 */
#define   UNIPHIER_SD_CLKCTL_DIV64	BIT(4)	/* SDCLK = CLK / 64 */
#define   UNIPHIER_SD_CLKCTL_DIV32	BIT(3)	/* SDCLK = CLK / 32 */
#define   UNIPHIER_SD_CLKCTL_DIV16	BIT(2)	/* SDCLK = CLK / 16 */
#define   UNIPHIER_SD_CLKCTL_DIV8	BIT(1)	/* SDCLK = CLK / 8 */
#define   UNIPHIER_SD_CLKCTL_DIV4	BIT(0)	/* SDCLK = CLK / 4 */
#define   UNIPHIER_SD_CLKCTL_DIV2	0	/* SDCLK = CLK / 2 */
#define   UNIPHIER_SD_CLKCTL_DIV1	BIT(10)	/* SDCLK = CLK */
#define   UNIPHIER_SD_CLKCTL_OFFEN	BIT(9)	/* stop SDCLK when unused */
#define   UNIPHIER_SD_CLKCTL_SCLKEN	BIT(8)	/* SDCLK output enable */
#define UNIPHIER_SD_SIZE		0x04c	/* block size */
#define UNIPHIER_SD_OPTION		0x050
#define   UNIPHIER_SD_OPTION_WIDTH_MASK	(5 << 13)
#define   UNIPHIER_SD_OPTION_WIDTH_1	(4 << 13)
#define   UNIPHIER_SD_OPTION_WIDTH_4	(0 << 13)
#define   UNIPHIER_SD_OPTION_WIDTH_8	(1 << 13)
#define UNIPHIER_SD_BUF			0x060	/* read/write buffer */
#define UNIPHIER_SD_EXTMODE		0x1b0
#define   UNIPHIER_SD_EXTMODE_DMA_EN	BIT(1)	/* transfer 1: DMA, 0: pio */
#define UNIPHIER_SD_SOFT_RST		0x1c0
#define UNIPHIER_SD_SOFT_RST_RSTX	BIT(0)	/* reset deassert */
#define UNIPHIER_SD_VERSION		0x1c4	/* version register */
#define UNIPHIER_SD_VERSION_IP		0xff	/* IP version */
#define UNIPHIER_SD_HOST_MODE		0x1c8
#define UNIPHIER_SD_IF_MODE		0x1cc
#define   UNIPHIER_SD_IF_MODE_DDR	BIT(0)	/* DDR mode */
#define UNIPHIER_SD_VOLT		0x1e4	/* voltage switch */
#define   UNIPHIER_SD_VOLT_MASK		(3 << 0)
#define   UNIPHIER_SD_VOLT_OFF		(0 << 0)
#define   UNIPHIER_SD_VOLT_330		(1 << 0)/* 3.3V signal */
#define   UNIPHIER_SD_VOLT_180		(2 << 0)/* 1.8V signal */
#define UNIPHIER_SD_DMA_MODE		0x410
#define   UNIPHIER_SD_DMA_MODE_DIR_RD	BIT(16)	/* 1: from device, 0: to dev */
#define   UNIPHIER_SD_DMA_MODE_ADDR_INC	BIT(0)	/* 1: address inc, 0: fixed */
#define UNIPHIER_SD_DMA_CTL		0x414
#define   UNIPHIER_SD_DMA_CTL_START	BIT(0)	/* start DMA (auto cleared) */
#define UNIPHIER_SD_DMA_RST		0x418
#define   UNIPHIER_SD_DMA_RST_RD	BIT(9)
#define   UNIPHIER_SD_DMA_RST_WR	BIT(8)
#define UNIPHIER_SD_DMA_INFO1		0x420
#define   UNIPHIER_SD_DMA_INFO1_END_RD2	BIT(20)	/* DMA from device is complete*/
#define   UNIPHIER_SD_DMA_INFO1_END_RD	BIT(17)	/* Don't use!  Hardware bug */
#define   UNIPHIER_SD_DMA_INFO1_END_WR	BIT(16)	/* DMA to device is complete */
#define UNIPHIER_SD_DMA_INFO1_MASK	0x424
#define UNIPHIER_SD_DMA_INFO2		0x428
#define   UNIPHIER_SD_DMA_INFO2_ERR_RD	BIT(17)
#define   UNIPHIER_SD_DMA_INFO2_ERR_WR	BIT(16)
#define UNIPHIER_SD_DMA_INFO2_MASK	0x42c
#define UNIPHIER_SD_DMA_ADDR_L		0x440
#define UNIPHIER_SD_DMA_ADDR_H		0x444

/* alignment required by the DMA engine of this controller */
#define UNIPHIER_SD_DMA_MINALIGN	0x10

struct uniphier_sd_priv {
	struct mmc_config cfg;
	struct mmc *mmc;
	struct udevice *dev;
	void __iomem *regbase;
	unsigned long mclk;
	unsigned int version;
	u32 caps;
#define UNIPHIER_SD_CAP_NONREMOVABLE	BIT(0)	/* Nonremovable e.g. eMMC */
#define UNIPHIER_SD_CAP_DMA_INTERNAL	BIT(1)	/* have internal DMA engine */
#define UNIPHIER_SD_CAP_DIV1024		BIT(2)	/* divisor 1024 is available */
};

static dma_addr_t __dma_map_single(void *ptr, size_t size,
				   enum dma_data_direction dir)
{
	unsigned long addr = (unsigned long)ptr;

	if (dir == DMA_FROM_DEVICE)
		invalidate_dcache_range(addr, addr + size);
	else
		flush_dcache_range(addr, addr + size);

	return addr;
}

static void __dma_unmap_single(dma_addr_t addr, size_t size,
			       enum dma_data_direction dir)
{
	if (dir != DMA_TO_DEVICE)
		invalidate_dcache_range(addr, addr + size);
}

static int uniphier_sd_check_error(struct uniphier_sd_priv *priv)
{
	u32 info2 = readl(priv->regbase + UNIPHIER_SD_INFO2);

	if (info2 & UNIPHIER_SD_INFO2_ERR_RTO) {
		/*
		 * TIMEOUT must be returned for unsupported command.  Do not
		 * display error log since this might be a part of sequence to
		 * distinguish between SD and MMC.
		 */
		return TIMEOUT;
	}

	if (info2 & UNIPHIER_SD_INFO2_ERR_TO) {
		dev_err(priv->dev, "timeout error\n");
		return -ETIMEDOUT;
	}

	if (info2 & (UNIPHIER_SD_INFO2_ERR_END | UNIPHIER_SD_INFO2_ERR_CRC |
		     UNIPHIER_SD_INFO2_ERR_IDX)) {
		dev_err(priv->dev, "communication out of sync\n");
		return -EILSEQ;
	}

	if (info2 & (UNIPHIER_SD_INFO2_ERR_ILA | UNIPHIER_SD_INFO2_ERR_ILR |
		     UNIPHIER_SD_INFO2_ERR_ILW)) {
		dev_err(priv->dev, "illegal access\n");
		return -EIO;
	}

	return 0;
}

static int uniphier_sd_wait_for_irq(struct uniphier_sd_priv *priv,
				    unsigned int reg, u32 flag)
{
	long wait = 1000000;
	int ret;

	while (!(readl(priv->regbase + reg) & flag)) {
		if (wait-- < 0) {
			dev_err(priv->dev, "timeout\n");
			return -ETIMEDOUT;
		}

		ret = uniphier_sd_check_error(priv);
		if (ret)
			return ret;

		udelay(1);
	}

	return 0;
}

static int uniphier_sd_pio_read_one_block(struct mmc *mmc, u32 **pbuf,
					  uint blocksize)
{
	struct uniphier_sd_priv *priv = mmc->priv;
	int i, ret;

	/* wait until the buffer is filled with data */
	ret = uniphier_sd_wait_for_irq(priv, UNIPHIER_SD_INFO2,
				       UNIPHIER_SD_INFO2_BRE);
	if (ret)
		return ret;

	/*
	 * Clear the status flag _before_ read the buffer out because
	 * UNIPHIER_SD_INFO2_BRE is edge-triggered, not level-triggered.
	 */
	writel(0, priv->regbase + UNIPHIER_SD_INFO2);

	if (likely(IS_ALIGNED((unsigned long)*pbuf, 4))) {
		for (i = 0; i < blocksize / 4; i++)
			*(*pbuf)++ = readl(priv->regbase + UNIPHIER_SD_BUF);
	} else {
		for (i = 0; i < blocksize / 4; i++)
			put_unaligned(readl(priv->regbase + UNIPHIER_SD_BUF),
				      (*pbuf)++);
	}

	return 0;
}

static int uniphier_sd_pio_write_one_block(struct mmc *mmc, const u32 **pbuf,
					   uint blocksize)
{
	struct uniphier_sd_priv *priv = mmc->priv;
	int i, ret;

	/* wait until the buffer becomes empty */
	ret = uniphier_sd_wait_for_irq(priv, UNIPHIER_SD_INFO2,
				       UNIPHIER_SD_INFO2_BWE);
	if (ret)
		return ret;

	writel(0, priv->regbase + UNIPHIER_SD_INFO2);

	if (likely(IS_ALIGNED((unsigned long)*pbuf, 4))) {
		for (i = 0; i < blocksize / 4; i++)
			writel(*(*pbuf)++, priv->regbase + UNIPHIER_SD_BUF);
	} else {
		for (i = 0; i < blocksize / 4; i++)
			writel(get_unaligned((*pbuf)++),
			       priv->regbase + UNIPHIER_SD_BUF);
	}

	return 0;
}

static int uniphier_sd_pio_xfer(struct mmc *mmc, struct mmc_data *data)
{
	u32 *dest = (u32 *)data->dest;
	const u32 *src = (const u32 *)data->src;
	int i, ret;

	for (i = 0; i < data->blocks; i++) {
		if (data->flags & MMC_DATA_READ)
			ret = uniphier_sd_pio_read_one_block(mmc, &dest,
							     data->blocksize);
		else
			ret = uniphier_sd_pio_write_one_block(mmc, &src,
							      data->blocksize);
		if (ret)
			return ret;
	}

	return 0;
}

static void uniphier_sd_dma_start(struct uniphier_sd_priv *priv,
				  dma_addr_t dma_addr)
{
	u32 tmp;

	writel(0, priv->regbase + UNIPHIER_SD_DMA_INFO1);
	writel(0, priv->regbase + UNIPHIER_SD_DMA_INFO2);

	/* enable DMA */
	tmp = readl(priv->regbase + UNIPHIER_SD_EXTMODE);
	tmp |= UNIPHIER_SD_EXTMODE_DMA_EN;
	writel(tmp, priv->regbase + UNIPHIER_SD_EXTMODE);

	writel(dma_addr & U32_MAX, priv->regbase + UNIPHIER_SD_DMA_ADDR_L);

	/* suppress the warning "right shift count >= width of type" */
	dma_addr >>= min_t(int, 32, 8 * sizeof(dma_addr));

	writel(dma_addr & U32_MAX, priv->regbase + UNIPHIER_SD_DMA_ADDR_H);

	writel(UNIPHIER_SD_DMA_CTL_START, priv->regbase + UNIPHIER_SD_DMA_CTL);
}

static int uniphier_sd_dma_wait_for_irq(struct uniphier_sd_priv *priv, u32 flag,
					unsigned int blocks)
{
	long wait = 1000000 + 10 * blocks;

	while (!(readl(priv->regbase + UNIPHIER_SD_DMA_INFO1) & flag)) {
		if (wait-- < 0) {
			dev_err(priv->dev, "timeout during DMA\n");
			return -ETIMEDOUT;
		}

		udelay(10);
	}

	if (readl(priv->regbase + UNIPHIER_SD_DMA_INFO2)) {
		dev_err(priv->dev, "error during DMA\n");
		return -EIO;
	}

	return 0;
}

static int uniphier_sd_dma_xfer(struct mmc *mmc, struct mmc_data *data)
{
	struct uniphier_sd_priv *priv = mmc->priv;
	size_t len = data->blocks * data->blocksize;
	void *buf;
	enum dma_data_direction dir;
	dma_addr_t dma_addr;
	u32 poll_flag, tmp;
	int ret;

	tmp = readl(priv->regbase + UNIPHIER_SD_DMA_MODE);

	if (data->flags & MMC_DATA_READ) {
		buf = data->dest;
		dir = DMA_FROM_DEVICE;
		poll_flag = UNIPHIER_SD_DMA_INFO1_END_RD2;
		tmp |= UNIPHIER_SD_DMA_MODE_DIR_RD;
	} else {
		buf = (void *)data->src;
		dir = DMA_TO_DEVICE;
		poll_flag = UNIPHIER_SD_DMA_INFO1_END_WR;
		tmp &= ~UNIPHIER_SD_DMA_MODE_DIR_RD;
	}

	writel(tmp, priv->regbase + UNIPHIER_SD_DMA_MODE);

	dma_addr = __dma_map_single(buf, len, dir);

	uniphier_sd_dma_start(priv, dma_addr);

	ret = uniphier_sd_dma_wait_for_irq(priv, poll_flag, data->blocks);

	__dma_unmap_single(dma_addr, len, dir);

	return ret;
}

/* check if the address is DMA'able */
static bool uniphier_sd_addr_is_dmaable(unsigned long addr)
{
	if (!IS_ALIGNED(addr, UNIPHIER_SD_DMA_MINALIGN))
		return false;

#if defined(CONFIG_ARCH_UNIPHIER) && !defined(CONFIG_ARM64) && \
	defined(CONFIG_SPL_BUILD)
	/*
	 * For UniPhier ARMv7 SoCs, the stack is allocated in the locked ways
	 * of L2, which is unreachable from the DMA engine.
	 */
	if (addr < CONFIG_SPL_STACK)
		return false;
#endif

	return true;
}

static int uniphier_sd_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
				struct mmc_data *data)
{
	struct uniphier_sd_priv *priv = mmc->priv;
	int ret;
	u32 tmp;

	if (readl(priv->regbase + UNIPHIER_SD_INFO2) & UNIPHIER_SD_INFO2_CBSY) {
		dev_err(priv->dev, "command busy\n");
		return -EBUSY;
	}

	/* clear all status flags */
	writel(0, priv->regbase + UNIPHIER_SD_INFO1);
	writel(0, priv->regbase + UNIPHIER_SD_INFO2);

	/* disable DMA once */
	tmp = readl(priv->regbase + UNIPHIER_SD_EXTMODE);
	tmp &= ~UNIPHIER_SD_EXTMODE_DMA_EN;
	writel(tmp, priv->regbase + UNIPHIER_SD_EXTMODE);

	writel(cmd->cmdarg, priv->regbase + UNIPHIER_SD_ARG);

	tmp = cmd->cmdidx;

	if (data) {
		writel(data->blocksize, priv->regbase + UNIPHIER_SD_SIZE);
		writel(data->blocks, priv->regbase + UNIPHIER_SD_SECCNT);

		/* Do not send CMD12 automatically */
		tmp |= UNIPHIER_SD_CMD_NOSTOP | UNIPHIER_SD_CMD_DATA;

		if (data->blocks > 1)
			tmp |= UNIPHIER_SD_CMD_MULTI;

		if (data->flags & MMC_DATA_READ)
			tmp |= UNIPHIER_SD_CMD_RD;
	}

	/*
	 * Do not use the response type auto-detection on this hardware.
	 * CMD8, for example, has different response types on SD and eMMC,
	 * while this controller always assumes the response type for SD.
	 * Set the response type manually.
	 */
	switch (cmd->resp_type) {
	case MMC_RSP_NONE:
		tmp |= UNIPHIER_SD_CMD_RSP_NONE;
		break;
	case MMC_RSP_R1:
		tmp |= UNIPHIER_SD_CMD_RSP_R1;
		break;
	case MMC_RSP_R1b:
		tmp |= UNIPHIER_SD_CMD_RSP_R1B;
		break;
	case MMC_RSP_R2:
		tmp |= UNIPHIER_SD_CMD_RSP_R2;
		break;
	case MMC_RSP_R3:
		tmp |= UNIPHIER_SD_CMD_RSP_R3;
		break;
	default:
		dev_err(priv->dev, "unknown response type\n");
		return -EINVAL;
	}

	dev_dbg(priv->dev, "sending CMD%d (SD_CMD=%08x, SD_ARG=%08x)\n",
		cmd->cmdidx, tmp, cmd->cmdarg);
	writel(tmp, priv->regbase + UNIPHIER_SD_CMD);

	ret = uniphier_sd_wait_for_irq(priv, UNIPHIER_SD_INFO1,
				       UNIPHIER_SD_INFO1_RSP);
	if (ret)
		return ret;

	if (cmd->resp_type & MMC_RSP_136) {
		u32 rsp_127_104 = readl(priv->regbase + UNIPHIER_SD_RSP76);
		u32 rsp_103_72 = readl(priv->regbase + UNIPHIER_SD_RSP54);
		u32 rsp_71_40 = readl(priv->regbase + UNIPHIER_SD_RSP32);
		u32 rsp_39_8 = readl(priv->regbase + UNIPHIER_SD_RSP10);

		cmd->response[0] = (rsp_127_104 & 0xffffff) << 8 |
							(rsp_103_72 & 0xff);
		cmd->response[1] = (rsp_103_72  & 0xffffff) << 8 |
							(rsp_71_40 & 0xff);
		cmd->response[2] = (rsp_71_40   & 0xffffff) << 8 |
							(rsp_39_8 & 0xff);
		cmd->response[3] = (rsp_39_8    & 0xffffff) << 8;
	} else {
		/* bit 39-8 */
		cmd->response[0] = readl(priv->regbase + UNIPHIER_SD_RSP10);
	}

	if (data) {
		/* use DMA if the HW supports it and the buffer is aligned */
		if (priv->caps & UNIPHIER_SD_CAP_DMA_INTERNAL &&
		    uniphier_sd_addr_is_dmaable((long)data->src))
			ret = uniphier_sd_dma_xfer(mmc, data);
		else
			ret = uniphier_sd_pio_xfer(mmc, data);

		ret = uniphier_sd_wait_for_irq(priv, UNIPHIER_SD_INFO1,
					       UNIPHIER_SD_INFO1_CMP);
		if (ret)
			return ret;
	}

	return ret;
}

static void uniphier_sd_set_bus_width(struct uniphier_sd_priv *priv,
				      struct mmc *mmc)
{
	u32 val, tmp;

	switch (mmc->bus_width) {
	case 1:
		val = UNIPHIER_SD_OPTION_WIDTH_1;
		break;
	case 4:
		val = UNIPHIER_SD_OPTION_WIDTH_4;
		break;
	case 8:
		val = UNIPHIER_SD_OPTION_WIDTH_8;
		break;
	default:
		BUG();
		break;
	}

	tmp = readl(priv->regbase + UNIPHIER_SD_OPTION);
	tmp &= ~UNIPHIER_SD_OPTION_WIDTH_MASK;
	tmp |= val;
	writel(tmp, priv->regbase + UNIPHIER_SD_OPTION);
}

static void uniphier_sd_set_ddr_mode(struct uniphier_sd_priv *priv,
				     struct mmc *mmc)
{
	u32 tmp;

	tmp = readl(priv->regbase + UNIPHIER_SD_IF_MODE);
	if (mmc->ddr_mode)
		tmp |= UNIPHIER_SD_IF_MODE_DDR;
	else
		tmp &= ~UNIPHIER_SD_IF_MODE_DDR;
	writel(tmp, priv->regbase + UNIPHIER_SD_IF_MODE);
}

static void uniphier_sd_set_clk_rate(struct uniphier_sd_priv *priv,
				     struct mmc *mmc)
{
	unsigned int divisor;
	u32 val, tmp;

	if (!mmc->clock)
		return;

	divisor = DIV_ROUND_UP(priv->mclk, mmc->clock);

	if (divisor <= 1)
		val = UNIPHIER_SD_CLKCTL_DIV1;
	else if (divisor <= 2)
		val = UNIPHIER_SD_CLKCTL_DIV2;
	else if (divisor <= 4)
		val = UNIPHIER_SD_CLKCTL_DIV4;
	else if (divisor <= 8)
		val = UNIPHIER_SD_CLKCTL_DIV8;
	else if (divisor <= 16)
		val = UNIPHIER_SD_CLKCTL_DIV16;
	else if (divisor <= 32)
		val = UNIPHIER_SD_CLKCTL_DIV32;
	else if (divisor <= 64)
		val = UNIPHIER_SD_CLKCTL_DIV64;
	else if (divisor <= 128)
		val = UNIPHIER_SD_CLKCTL_DIV128;
	else if (divisor <= 256)
		val = UNIPHIER_SD_CLKCTL_DIV256;
	else if (divisor <= 512 || !(priv->caps & UNIPHIER_SD_CAP_DIV1024))
		val = UNIPHIER_SD_CLKCTL_DIV512;
	else
		val = UNIPHIER_SD_CLKCTL_DIV1024;

	tmp = readl(priv->regbase + UNIPHIER_SD_CLKCTL);

	/* stop the clock before changing its rate to avoid a glitch signal */
	tmp &= ~UNIPHIER_SD_CLKCTL_SCLKEN;
	writel(tmp, priv->regbase + UNIPHIER_SD_CLKCTL);

	tmp &= ~UNIPHIER_SD_CLKCTL_DIV_MASK;
	tmp |= val | UNIPHIER_SD_CLKCTL_OFFEN;
	writel(tmp, priv->regbase + UNIPHIER_SD_CLKCTL);

	tmp |= UNIPHIER_SD_CLKCTL_SCLKEN;
	writel(tmp, priv->regbase + UNIPHIER_SD_CLKCTL);
}

static void uniphier_sd_set_ios(struct mmc *mmc)
{
	struct uniphier_sd_priv *priv = mmc->priv;

	dev_dbg(priv->dev, "clock %uHz, DDRmode %d, width %u\n",
		mmc->clock, mmc->ddr_mode, mmc->bus_width);

	uniphier_sd_set_bus_width(priv, mmc);
	uniphier_sd_set_ddr_mode(priv, mmc);
	uniphier_sd_set_clk_rate(priv, mmc);

	udelay(1000);
}

static int uniphier_sd_init(struct mmc *mmc)
{
	struct uniphier_sd_priv *priv = mmc->priv;
	u32 tmp;

	/* soft reset of the host */
	tmp = readl(priv->regbase + UNIPHIER_SD_SOFT_RST);
	tmp &= ~UNIPHIER_SD_SOFT_RST_RSTX;
	writel(tmp, priv->regbase + UNIPHIER_SD_SOFT_RST);
	tmp |= UNIPHIER_SD_SOFT_RST_RSTX;
	writel(tmp, priv->regbase + UNIPHIER_SD_SOFT_RST);

	/* FIXME: implement eMMC hw_reset */

	writel(UNIPHIER_SD_STOP_SEC, priv->regbase + UNIPHIER_SD_STOP);

	/*
	 * Connected to 32bit AXI.
	 * This register dropped backward compatibility at version 0x10.
	 * Write an appropriate value depending on the IP version.
	 */
	writel(priv->version >= 0x10 ? 0x00000101 : 0x00000000,
	       priv->regbase + UNIPHIER_SD_HOST_MODE);

	if (priv->caps & UNIPHIER_SD_CAP_DMA_INTERNAL) {
		tmp = readl(priv->regbase + UNIPHIER_SD_DMA_MODE);
		tmp |= UNIPHIER_SD_DMA_MODE_ADDR_INC;
		writel(tmp, priv->regbase + UNIPHIER_SD_DMA_MODE);
	}

	return 0;
}

static int uniphier_sd_getcd(struct mmc *mmc)
{
	struct uniphier_sd_priv *priv = mmc->priv;

	if (priv->caps & UNIPHIER_SD_CAP_NONREMOVABLE)
		return 1;

	return !!(readl(priv->regbase + UNIPHIER_SD_INFO1) &
		  UNIPHIER_SD_INFO1_CD);
}

static const struct mmc_ops uniphier_sd_ops = {
	.send_cmd = uniphier_sd_send_cmd,
	.set_ios = uniphier_sd_set_ios,
	.init = uniphier_sd_init,
	.getcd = uniphier_sd_getcd,
};

int uniphier_sd_probe(struct udevice *dev)
{
	struct uniphier_sd_priv *priv = dev_get_priv(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	fdt_addr_t base;
	struct udevice *clk_dev;
	int clk_id;
	int ret;

	priv->dev = dev;

	base = dev_get_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regbase = map_sysmem(base, SZ_2K);
	if (!priv->regbase)
		return -ENOMEM;

	clk_id = clk_get_by_index(dev, 0, &clk_dev);
	if (clk_id < 0) {
		dev_err(dev, "failed to get host clock\n");
		return clk_id;
	}

	/* set to max rate */
	priv->mclk = clk_set_periph_rate(clk_dev, clk_id, ULONG_MAX);
	if (IS_ERR_VALUE(priv->mclk)) {
		dev_err(dev, "failed to set rate for host clock\n");
		return priv->mclk;
	}

	ret = clk_enable(clk_dev, clk_id);
	if (ret) {
		dev_err(dev, "failed to enable host clock\n");
		return ret;
	}

	priv->cfg.name = dev->name;
	priv->cfg.ops = &uniphier_sd_ops;
	priv->cfg.host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS;

	switch (fdtdec_get_int(gd->fdt_blob, dev->of_offset, "bus-width", 1)) {
	case 8:
		priv->cfg.host_caps |= MMC_MODE_8BIT;
		break;
	case 4:
		priv->cfg.host_caps |= MMC_MODE_4BIT;
		break;
	case 1:
		break;
	default:
		dev_err(dev, "Invalid \"bus-width\" value\n");
		return -EINVAL;
	}

	if (fdt_get_property(gd->fdt_blob, dev->of_offset, "non-removable",
			     NULL))
		priv->caps |= UNIPHIER_SD_CAP_NONREMOVABLE;

	priv->version = readl(priv->regbase + UNIPHIER_SD_VERSION) &
							UNIPHIER_SD_VERSION_IP;
	dev_dbg(dev, "version %x\n", priv->version);
	if (priv->version >= 0x10) {
		priv->caps |= UNIPHIER_SD_CAP_DMA_INTERNAL;
		priv->caps |= UNIPHIER_SD_CAP_DIV1024;
	}

	priv->cfg.voltages = MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;
	priv->cfg.f_min = priv->mclk /
			(priv->caps & UNIPHIER_SD_CAP_DIV1024 ? 1024 : 512);
	priv->cfg.f_max = priv->mclk;
	priv->cfg.b_max = U32_MAX; /* max value of UNIPHIER_SD_SECCNT */

	priv->mmc = mmc_create(&priv->cfg, priv);
	if (!priv->mmc)
		return -EIO;

	upriv->mmc = priv->mmc;

	return 0;
}

int uniphier_sd_remove(struct udevice *dev)
{
	struct uniphier_sd_priv *priv = dev_get_priv(dev);

	unmap_sysmem(priv->regbase);
	mmc_destroy(priv->mmc);

	return 0;
}

static const struct udevice_id uniphier_sd_match[] = {
	{ .compatible = "socionext,uniphier-sdhc" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_mmc) = {
	.name = "uniphier-mmc",
	.id = UCLASS_MMC,
	.of_match = uniphier_sd_match,
	.probe = uniphier_sd_probe,
	.remove = uniphier_sd_remove,
	.priv_auto_alloc_size = sizeof(struct uniphier_sd_priv),
};
