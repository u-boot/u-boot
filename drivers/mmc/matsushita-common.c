/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <fdtdec.h>
#include <mmc.h>
#include <dm.h>
#include <linux/compat.h>
#include <linux/dma-direction.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <power/regulator.h>
#include <asm/unaligned.h>

#include "matsushita-common.h"

DECLARE_GLOBAL_DATA_PTR;

static u64 matsu_sd_readq(struct matsu_sd_priv *priv, unsigned int reg)
{
	if (priv->caps & MATSU_SD_CAP_64BIT)
		return readq(priv->regbase + (reg << 1));
	else
		return readq(priv->regbase + reg);
}

static void matsu_sd_writeq(struct matsu_sd_priv *priv,
			       u64 val, unsigned int reg)
{
	if (priv->caps & MATSU_SD_CAP_64BIT)
		writeq(val, priv->regbase + (reg << 1));
	else
		writeq(val, priv->regbase + reg);
}

static u32 matsu_sd_readl(struct matsu_sd_priv *priv, unsigned int reg)
{
	if (priv->caps & MATSU_SD_CAP_64BIT)
		return readl(priv->regbase + (reg << 1));
	else
		return readl(priv->regbase + reg);
}

static void matsu_sd_writel(struct matsu_sd_priv *priv,
			       u32 val, unsigned int reg)
{
	if (priv->caps & MATSU_SD_CAP_64BIT)
		writel(val, priv->regbase + (reg << 1));
	else
		writel(val, priv->regbase + reg);
}

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

static int matsu_sd_check_error(struct udevice *dev)
{
	struct matsu_sd_priv *priv = dev_get_priv(dev);
	u32 info2 = matsu_sd_readl(priv, MATSU_SD_INFO2);

	if (info2 & MATSU_SD_INFO2_ERR_RTO) {
		/*
		 * TIMEOUT must be returned for unsupported command.  Do not
		 * display error log since this might be a part of sequence to
		 * distinguish between SD and MMC.
		 */
		return -ETIMEDOUT;
	}

	if (info2 & MATSU_SD_INFO2_ERR_TO) {
		dev_err(dev, "timeout error\n");
		return -ETIMEDOUT;
	}

	if (info2 & (MATSU_SD_INFO2_ERR_END | MATSU_SD_INFO2_ERR_CRC |
		     MATSU_SD_INFO2_ERR_IDX)) {
		dev_err(dev, "communication out of sync\n");
		return -EILSEQ;
	}

	if (info2 & (MATSU_SD_INFO2_ERR_ILA | MATSU_SD_INFO2_ERR_ILR |
		     MATSU_SD_INFO2_ERR_ILW)) {
		dev_err(dev, "illegal access\n");
		return -EIO;
	}

	return 0;
}

static int matsu_sd_wait_for_irq(struct udevice *dev, unsigned int reg,
				    u32 flag)
{
	struct matsu_sd_priv *priv = dev_get_priv(dev);
	long wait = 1000000;
	int ret;

	while (!(matsu_sd_readl(priv, reg) & flag)) {
		if (wait-- < 0) {
			dev_err(dev, "timeout\n");
			return -ETIMEDOUT;
		}

		ret = matsu_sd_check_error(dev);
		if (ret)
			return ret;

		udelay(1);
	}

	return 0;
}

static int matsu_sd_pio_read_one_block(struct udevice *dev, char *pbuf,
					  uint blocksize)
{
	struct matsu_sd_priv *priv = dev_get_priv(dev);
	int i, ret;

	/* wait until the buffer is filled with data */
	ret = matsu_sd_wait_for_irq(dev, MATSU_SD_INFO2,
				       MATSU_SD_INFO2_BRE);
	if (ret)
		return ret;

	/*
	 * Clear the status flag _before_ read the buffer out because
	 * MATSU_SD_INFO2_BRE is edge-triggered, not level-triggered.
	 */
	matsu_sd_writel(priv, 0, MATSU_SD_INFO2);

	if (priv->caps & MATSU_SD_CAP_64BIT) {
		u64 *buf = (u64 *)pbuf;
		if (likely(IS_ALIGNED((uintptr_t)buf, 8))) {
			for (i = 0; i < blocksize / 8; i++) {
				*buf++ = matsu_sd_readq(priv,
							   MATSU_SD_BUF);
			}
		} else {
			for (i = 0; i < blocksize / 8; i++) {
				u64 data;
				data = matsu_sd_readq(priv,
							 MATSU_SD_BUF);
				put_unaligned(data, buf++);
			}
		}
	} else {
		u32 *buf = (u32 *)pbuf;
		if (likely(IS_ALIGNED((uintptr_t)buf, 4))) {
			for (i = 0; i < blocksize / 4; i++) {
				*buf++ = matsu_sd_readl(priv,
							   MATSU_SD_BUF);
			}
		} else {
			for (i = 0; i < blocksize / 4; i++) {
				u32 data;
				data = matsu_sd_readl(priv, MATSU_SD_BUF);
				put_unaligned(data, buf++);
			}
		}
	}

	return 0;
}

static int matsu_sd_pio_write_one_block(struct udevice *dev,
					   const char *pbuf, uint blocksize)
{
	struct matsu_sd_priv *priv = dev_get_priv(dev);
	int i, ret;

	/* wait until the buffer becomes empty */
	ret = matsu_sd_wait_for_irq(dev, MATSU_SD_INFO2,
				       MATSU_SD_INFO2_BWE);
	if (ret)
		return ret;

	matsu_sd_writel(priv, 0, MATSU_SD_INFO2);

	if (priv->caps & MATSU_SD_CAP_64BIT) {
		const u64 *buf = (const u64 *)pbuf;
		if (likely(IS_ALIGNED((uintptr_t)buf, 8))) {
			for (i = 0; i < blocksize / 8; i++) {
				matsu_sd_writeq(priv, *buf++,
						   MATSU_SD_BUF);
			}
		} else {
			for (i = 0; i < blocksize / 8; i++) {
				u64 data = get_unaligned(buf++);
				matsu_sd_writeq(priv, data,
						   MATSU_SD_BUF);
			}
		}
	} else {
		const u32 *buf = (const u32 *)pbuf;
		if (likely(IS_ALIGNED((uintptr_t)buf, 4))) {
			for (i = 0; i < blocksize / 4; i++) {
				matsu_sd_writel(priv, *buf++,
						   MATSU_SD_BUF);
			}
		} else {
			for (i = 0; i < blocksize / 4; i++) {
				u32 data = get_unaligned(buf++);
				matsu_sd_writel(priv, data,
						   MATSU_SD_BUF);
			}
		}
	}

	return 0;
}

static int matsu_sd_pio_xfer(struct udevice *dev, struct mmc_data *data)
{
	const char *src = data->src;
	char *dest = data->dest;
	int i, ret;

	for (i = 0; i < data->blocks; i++) {
		if (data->flags & MMC_DATA_READ)
			ret = matsu_sd_pio_read_one_block(dev, dest,
							     data->blocksize);
		else
			ret = matsu_sd_pio_write_one_block(dev, src,
							      data->blocksize);
		if (ret)
			return ret;

		if (data->flags & MMC_DATA_READ)
			dest += data->blocksize;
		else
			src += data->blocksize;
	}

	return 0;
}

static void matsu_sd_dma_start(struct matsu_sd_priv *priv,
				  dma_addr_t dma_addr)
{
	u32 tmp;

	matsu_sd_writel(priv, 0, MATSU_SD_DMA_INFO1);
	matsu_sd_writel(priv, 0, MATSU_SD_DMA_INFO2);

	/* enable DMA */
	tmp = matsu_sd_readl(priv, MATSU_SD_EXTMODE);
	tmp |= MATSU_SD_EXTMODE_DMA_EN;
	matsu_sd_writel(priv, tmp, MATSU_SD_EXTMODE);

	matsu_sd_writel(priv, dma_addr & U32_MAX, MATSU_SD_DMA_ADDR_L);

	/* suppress the warning "right shift count >= width of type" */
	dma_addr >>= min_t(int, 32, 8 * sizeof(dma_addr));

	matsu_sd_writel(priv, dma_addr & U32_MAX, MATSU_SD_DMA_ADDR_H);

	matsu_sd_writel(priv, MATSU_SD_DMA_CTL_START, MATSU_SD_DMA_CTL);
}

static int matsu_sd_dma_wait_for_irq(struct udevice *dev, u32 flag,
					unsigned int blocks)
{
	struct matsu_sd_priv *priv = dev_get_priv(dev);
	long wait = 1000000 + 10 * blocks;

	while (!(matsu_sd_readl(priv, MATSU_SD_DMA_INFO1) & flag)) {
		if (wait-- < 0) {
			dev_err(dev, "timeout during DMA\n");
			return -ETIMEDOUT;
		}

		udelay(10);
	}

	if (matsu_sd_readl(priv, MATSU_SD_DMA_INFO2)) {
		dev_err(dev, "error during DMA\n");
		return -EIO;
	}

	return 0;
}

static int matsu_sd_dma_xfer(struct udevice *dev, struct mmc_data *data)
{
	struct matsu_sd_priv *priv = dev_get_priv(dev);
	size_t len = data->blocks * data->blocksize;
	void *buf;
	enum dma_data_direction dir;
	dma_addr_t dma_addr;
	u32 poll_flag, tmp;
	int ret;

	tmp = matsu_sd_readl(priv, MATSU_SD_DMA_MODE);

	if (data->flags & MMC_DATA_READ) {
		buf = data->dest;
		dir = DMA_FROM_DEVICE;
		poll_flag = MATSU_SD_DMA_INFO1_END_RD2;
		tmp |= MATSU_SD_DMA_MODE_DIR_RD;
	} else {
		buf = (void *)data->src;
		dir = DMA_TO_DEVICE;
		poll_flag = MATSU_SD_DMA_INFO1_END_WR;
		tmp &= ~MATSU_SD_DMA_MODE_DIR_RD;
	}

	matsu_sd_writel(priv, tmp, MATSU_SD_DMA_MODE);

	dma_addr = __dma_map_single(buf, len, dir);

	matsu_sd_dma_start(priv, dma_addr);

	ret = matsu_sd_dma_wait_for_irq(dev, poll_flag, data->blocks);

	__dma_unmap_single(dma_addr, len, dir);

	return ret;
}

/* check if the address is DMA'able */
static bool matsu_sd_addr_is_dmaable(unsigned long addr)
{
	if (!IS_ALIGNED(addr, MATSU_SD_DMA_MINALIGN))
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

int matsu_sd_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
		      struct mmc_data *data)
{
	struct matsu_sd_priv *priv = dev_get_priv(dev);
	int ret;
	u32 tmp;

	if (matsu_sd_readl(priv, MATSU_SD_INFO2) & MATSU_SD_INFO2_CBSY) {
		dev_err(dev, "command busy\n");
		return -EBUSY;
	}

	/* clear all status flags */
	matsu_sd_writel(priv, 0, MATSU_SD_INFO1);
	matsu_sd_writel(priv, 0, MATSU_SD_INFO2);

	/* disable DMA once */
	tmp = matsu_sd_readl(priv, MATSU_SD_EXTMODE);
	tmp &= ~MATSU_SD_EXTMODE_DMA_EN;
	matsu_sd_writel(priv, tmp, MATSU_SD_EXTMODE);

	matsu_sd_writel(priv, cmd->cmdarg, MATSU_SD_ARG);

	tmp = cmd->cmdidx;

	if (data) {
		matsu_sd_writel(priv, data->blocksize, MATSU_SD_SIZE);
		matsu_sd_writel(priv, data->blocks, MATSU_SD_SECCNT);

		/* Do not send CMD12 automatically */
		tmp |= MATSU_SD_CMD_NOSTOP | MATSU_SD_CMD_DATA;

		if (data->blocks > 1)
			tmp |= MATSU_SD_CMD_MULTI;

		if (data->flags & MMC_DATA_READ)
			tmp |= MATSU_SD_CMD_RD;
	}

	/*
	 * Do not use the response type auto-detection on this hardware.
	 * CMD8, for example, has different response types on SD and eMMC,
	 * while this controller always assumes the response type for SD.
	 * Set the response type manually.
	 */
	switch (cmd->resp_type) {
	case MMC_RSP_NONE:
		tmp |= MATSU_SD_CMD_RSP_NONE;
		break;
	case MMC_RSP_R1:
		tmp |= MATSU_SD_CMD_RSP_R1;
		break;
	case MMC_RSP_R1b:
		tmp |= MATSU_SD_CMD_RSP_R1B;
		break;
	case MMC_RSP_R2:
		tmp |= MATSU_SD_CMD_RSP_R2;
		break;
	case MMC_RSP_R3:
		tmp |= MATSU_SD_CMD_RSP_R3;
		break;
	default:
		dev_err(dev, "unknown response type\n");
		return -EINVAL;
	}

	dev_dbg(dev, "sending CMD%d (SD_CMD=%08x, SD_ARG=%08x)\n",
		cmd->cmdidx, tmp, cmd->cmdarg);
	matsu_sd_writel(priv, tmp, MATSU_SD_CMD);

	ret = matsu_sd_wait_for_irq(dev, MATSU_SD_INFO1,
				       MATSU_SD_INFO1_RSP);
	if (ret)
		return ret;

	if (cmd->resp_type & MMC_RSP_136) {
		u32 rsp_127_104 = matsu_sd_readl(priv, MATSU_SD_RSP76);
		u32 rsp_103_72 = matsu_sd_readl(priv, MATSU_SD_RSP54);
		u32 rsp_71_40 = matsu_sd_readl(priv, MATSU_SD_RSP32);
		u32 rsp_39_8 = matsu_sd_readl(priv, MATSU_SD_RSP10);

		cmd->response[0] = ((rsp_127_104 & 0x00ffffff) << 8) |
				   ((rsp_103_72  & 0xff000000) >> 24);
		cmd->response[1] = ((rsp_103_72  & 0x00ffffff) << 8) |
				   ((rsp_71_40   & 0xff000000) >> 24);
		cmd->response[2] = ((rsp_71_40   & 0x00ffffff) << 8) |
				   ((rsp_39_8    & 0xff000000) >> 24);
		cmd->response[3] = (rsp_39_8     & 0xffffff)   << 8;
	} else {
		/* bit 39-8 */
		cmd->response[0] = matsu_sd_readl(priv, MATSU_SD_RSP10);
	}

	if (data) {
		/* use DMA if the HW supports it and the buffer is aligned */
		if (priv->caps & MATSU_SD_CAP_DMA_INTERNAL &&
		    matsu_sd_addr_is_dmaable((long)data->src))
			ret = matsu_sd_dma_xfer(dev, data);
		else
			ret = matsu_sd_pio_xfer(dev, data);

		ret = matsu_sd_wait_for_irq(dev, MATSU_SD_INFO1,
					       MATSU_SD_INFO1_CMP);
		if (ret)
			return ret;
	}

	return ret;
}

static int matsu_sd_set_bus_width(struct matsu_sd_priv *priv,
				     struct mmc *mmc)
{
	u32 val, tmp;

	switch (mmc->bus_width) {
	case 1:
		val = MATSU_SD_OPTION_WIDTH_1;
		break;
	case 4:
		val = MATSU_SD_OPTION_WIDTH_4;
		break;
	case 8:
		val = MATSU_SD_OPTION_WIDTH_8;
		break;
	default:
		return -EINVAL;
	}

	tmp = matsu_sd_readl(priv, MATSU_SD_OPTION);
	tmp &= ~MATSU_SD_OPTION_WIDTH_MASK;
	tmp |= val;
	matsu_sd_writel(priv, tmp, MATSU_SD_OPTION);

	return 0;
}

static void matsu_sd_set_ddr_mode(struct matsu_sd_priv *priv,
				     struct mmc *mmc)
{
	u32 tmp;

	tmp = matsu_sd_readl(priv, MATSU_SD_IF_MODE);
	if (mmc->ddr_mode)
		tmp |= MATSU_SD_IF_MODE_DDR;
	else
		tmp &= ~MATSU_SD_IF_MODE_DDR;
	matsu_sd_writel(priv, tmp, MATSU_SD_IF_MODE);
}

static void matsu_sd_set_clk_rate(struct matsu_sd_priv *priv,
				     struct mmc *mmc)
{
	unsigned int divisor;
	u32 val, tmp;

	if (!mmc->clock)
		return;

	divisor = DIV_ROUND_UP(priv->mclk, mmc->clock);

	if (divisor <= 1)
		val = MATSU_SD_CLKCTL_DIV1;
	else if (divisor <= 2)
		val = MATSU_SD_CLKCTL_DIV2;
	else if (divisor <= 4)
		val = MATSU_SD_CLKCTL_DIV4;
	else if (divisor <= 8)
		val = MATSU_SD_CLKCTL_DIV8;
	else if (divisor <= 16)
		val = MATSU_SD_CLKCTL_DIV16;
	else if (divisor <= 32)
		val = MATSU_SD_CLKCTL_DIV32;
	else if (divisor <= 64)
		val = MATSU_SD_CLKCTL_DIV64;
	else if (divisor <= 128)
		val = MATSU_SD_CLKCTL_DIV128;
	else if (divisor <= 256)
		val = MATSU_SD_CLKCTL_DIV256;
	else if (divisor <= 512 || !(priv->caps & MATSU_SD_CAP_DIV1024))
		val = MATSU_SD_CLKCTL_DIV512;
	else
		val = MATSU_SD_CLKCTL_DIV1024;

	tmp = matsu_sd_readl(priv, MATSU_SD_CLKCTL);
	if (tmp & MATSU_SD_CLKCTL_SCLKEN &&
	    (tmp & MATSU_SD_CLKCTL_DIV_MASK) == val)
		return;

	/* stop the clock before changing its rate to avoid a glitch signal */
	tmp &= ~MATSU_SD_CLKCTL_SCLKEN;
	matsu_sd_writel(priv, tmp, MATSU_SD_CLKCTL);

	tmp &= ~MATSU_SD_CLKCTL_DIV_MASK;
	tmp |= val | MATSU_SD_CLKCTL_OFFEN;
	matsu_sd_writel(priv, tmp, MATSU_SD_CLKCTL);

	tmp |= MATSU_SD_CLKCTL_SCLKEN;
	matsu_sd_writel(priv, tmp, MATSU_SD_CLKCTL);

	udelay(1000);
}

int matsu_sd_set_ios(struct udevice *dev)
{
	struct matsu_sd_priv *priv = dev_get_priv(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	int ret;

	dev_dbg(dev, "clock %uHz, DDRmode %d, width %u\n",
		mmc->clock, mmc->ddr_mode, mmc->bus_width);

	ret = matsu_sd_set_bus_width(priv, mmc);
	if (ret)
		return ret;
	matsu_sd_set_ddr_mode(priv, mmc);
	matsu_sd_set_clk_rate(priv, mmc);

	return 0;
}

int matsu_sd_get_cd(struct udevice *dev)
{
	struct matsu_sd_priv *priv = dev_get_priv(dev);

	if (priv->caps & MATSU_SD_CAP_NONREMOVABLE)
		return 1;

	return !!(matsu_sd_readl(priv, MATSU_SD_INFO1) &
		  MATSU_SD_INFO1_CD);
}

static void matsu_sd_host_init(struct matsu_sd_priv *priv)
{
	u32 tmp;

	/* soft reset of the host */
	tmp = matsu_sd_readl(priv, MATSU_SD_SOFT_RST);
	tmp &= ~MATSU_SD_SOFT_RST_RSTX;
	matsu_sd_writel(priv, tmp, MATSU_SD_SOFT_RST);
	tmp |= MATSU_SD_SOFT_RST_RSTX;
	matsu_sd_writel(priv, tmp, MATSU_SD_SOFT_RST);

	/* FIXME: implement eMMC hw_reset */

	matsu_sd_writel(priv, MATSU_SD_STOP_SEC, MATSU_SD_STOP);

	/*
	 * Connected to 32bit AXI.
	 * This register dropped backward compatibility at version 0x10.
	 * Write an appropriate value depending on the IP version.
	 */
	matsu_sd_writel(priv, priv->version >= 0x10 ? 0x00000101 : 0x00000000,
			   MATSU_SD_HOST_MODE);

	if (priv->caps & MATSU_SD_CAP_DMA_INTERNAL) {
		tmp = matsu_sd_readl(priv, MATSU_SD_DMA_MODE);
		tmp |= MATSU_SD_DMA_MODE_ADDR_INC;
		matsu_sd_writel(priv, tmp, MATSU_SD_DMA_MODE);
	}
}

int matsu_sd_bind(struct udevice *dev)
{
	struct matsu_sd_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

int matsu_sd_probe(struct udevice *dev)
{
	struct matsu_sd_plat *plat = dev_get_platdata(dev);
	struct matsu_sd_priv *priv = dev_get_priv(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	const u32 quirks = dev_get_driver_data(dev);
	fdt_addr_t base;
	struct clk clk;
	int ret;
#ifdef CONFIG_DM_REGULATOR
	struct udevice *vqmmc_dev;
#endif

	base = devfdt_get_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regbase = devm_ioremap(dev, base, SZ_2K);
	if (!priv->regbase)
		return -ENOMEM;

#ifdef CONFIG_DM_REGULATOR
	ret = device_get_supply_regulator(dev, "vqmmc-supply", &vqmmc_dev);
	if (!ret) {
		/* Set the regulator to 3.3V until we support 1.8V modes */
		regulator_set_value(vqmmc_dev, 3300000);
		regulator_set_enable(vqmmc_dev, true);
	}
#endif

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get host clock\n");
		return ret;
	}

	/* set to max rate */
	priv->mclk = clk_set_rate(&clk, ULONG_MAX);
	if (IS_ERR_VALUE(priv->mclk)) {
		dev_err(dev, "failed to set rate for host clock\n");
		clk_free(&clk);
		return priv->mclk;
	}

	ret = clk_enable(&clk);
	clk_free(&clk);
	if (ret) {
		dev_err(dev, "failed to enable host clock\n");
		return ret;
	}

	plat->cfg.name = dev->name;
	plat->cfg.host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS;

	switch (fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "bus-width",
			       1)) {
	case 8:
		plat->cfg.host_caps |= MMC_MODE_8BIT;
		break;
	case 4:
		plat->cfg.host_caps |= MMC_MODE_4BIT;
		break;
	case 1:
		break;
	default:
		dev_err(dev, "Invalid \"bus-width\" value\n");
		return -EINVAL;
	}

	if (quirks) {
		priv->caps = quirks;
	} else {
		priv->version = matsu_sd_readl(priv, MATSU_SD_VERSION) &
							MATSU_SD_VERSION_IP;
		dev_dbg(dev, "version %x\n", priv->version);
		if (priv->version >= 0x10) {
			priv->caps |= MATSU_SD_CAP_DMA_INTERNAL;
			priv->caps |= MATSU_SD_CAP_DIV1024;
		}
	}

	if (fdt_get_property(gd->fdt_blob, dev_of_offset(dev), "non-removable",
			     NULL))
		priv->caps |= MATSU_SD_CAP_NONREMOVABLE;

	matsu_sd_host_init(priv);

	plat->cfg.voltages = MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;
	plat->cfg.f_min = priv->mclk /
			(priv->caps & MATSU_SD_CAP_DIV1024 ? 1024 : 512);
	plat->cfg.f_max = priv->mclk;
	plat->cfg.b_max = U32_MAX; /* max value of MATSU_SD_SECCNT */

	upriv->mmc = &plat->mmc;

	return 0;
}
