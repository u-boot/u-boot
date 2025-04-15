// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All Rights Reserved.
 *
 * Author: SkyLake.Huang <skylake.huang@mediatek.com>
 */

#include <clk.h>
#include <cpu_func.h>
#include <div64.h>
#include <dm.h>
#include <spi.h>
#include <spi-mem.h>
#include <stdbool.h>
#include <watchdog.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>

#define SPI_CFG0_REG				0x0000
#define SPI_CFG1_REG				0x0004
#define SPI_TX_SRC_REG				0x0008
#define SPI_RX_DST_REG				0x000c
#define SPI_TX_DATA_REG				0x0010
#define SPI_RX_DATA_REG				0x0014
#define SPI_CMD_REG				0x0018
#define SPI_IRQ_REG				0x001c
#define SPI_STATUS_REG				0x0020
#define SPI_PAD_SEL_REG				0x0024
#define SPI_CFG2_REG				0x0028
#define SPI_TX_SRC_REG_64			0x002c
#define SPI_RX_DST_REG_64			0x0030
#define SPI_CFG3_IPM_REG			0x0040

#define SPI_CFG0_SCK_HIGH_OFFSET		0
#define SPI_CFG0_SCK_LOW_OFFSET			8
#define SPI_CFG0_CS_HOLD_OFFSET			16
#define SPI_CFG0_CS_SETUP_OFFSET		24
#define SPI_ADJUST_CFG0_CS_HOLD_OFFSET		0
#define SPI_ADJUST_CFG0_CS_SETUP_OFFSET		16

#define SPI_CFG1_CS_IDLE_OFFSET			0
#define SPI_CFG1_PACKET_LOOP_OFFSET		8
#define SPI_CFG1_PACKET_LENGTH_OFFSET		16
#define SPI_CFG1_GET_TICKDLY_OFFSET		29

#define SPI_CFG1_GET_TICKDLY_MASK		GENMASK(31, 29)
#define SPI_CFG1_CS_IDLE_MASK			0xff
#define SPI_CFG1_PACKET_LOOP_MASK		0xff00
#define SPI_CFG1_PACKET_LENGTH_MASK		0x3ff0000
#define SPI_CFG1_IPM_PACKET_LENGTH_MASK		GENMASK(31, 16)
#define SPI_CFG2_SCK_HIGH_OFFSET		0
#define SPI_CFG2_SCK_LOW_OFFSET			16
#define SPI_CFG2_SCK_HIGH_MASK			GENMASK(15, 0)
#define SPI_CFG2_SCK_LOW_MASK			GENMASK(31, 16)

#define SPI_CMD_ACT				BIT(0)
#define SPI_CMD_RESUME				BIT(1)
#define SPI_CMD_RST				BIT(2)
#define SPI_CMD_PAUSE_EN			BIT(4)
#define SPI_CMD_DEASSERT			BIT(5)
#define SPI_CMD_SAMPLE_SEL			BIT(6)
#define SPI_CMD_CS_POL				BIT(7)
#define SPI_CMD_CPHA				BIT(8)
#define SPI_CMD_CPOL				BIT(9)
#define SPI_CMD_RX_DMA				BIT(10)
#define SPI_CMD_TX_DMA				BIT(11)
#define SPI_CMD_TXMSBF				BIT(12)
#define SPI_CMD_RXMSBF				BIT(13)
#define SPI_CMD_RX_ENDIAN			BIT(14)
#define SPI_CMD_TX_ENDIAN			BIT(15)
#define SPI_CMD_FINISH_IE			BIT(16)
#define SPI_CMD_PAUSE_IE			BIT(17)
#define SPI_CMD_IPM_NONIDLE_MODE		BIT(19)
#define SPI_CMD_IPM_SPIM_LOOP			BIT(21)
#define SPI_CMD_IPM_GET_TICKDLY_OFFSET		22

#define SPI_CMD_IPM_GET_TICKDLY_MASK		GENMASK(24, 22)

#define PIN_MODE_CFG(x)				((x) / 2)

#define SPI_CFG3_IPM_PIN_MODE_OFFSET		0
#define SPI_CFG3_IPM_HALF_DUPLEX_DIR		BIT(2)
#define SPI_CFG3_IPM_HALF_DUPLEX_EN		BIT(3)
#define SPI_CFG3_IPM_XMODE_EN			BIT(4)
#define SPI_CFG3_IPM_NODATA_FLAG		BIT(5)
#define SPI_CFG3_IPM_CMD_BYTELEN_OFFSET		8
#define SPI_CFG3_IPM_ADDR_BYTELEN_OFFSET	12
#define SPI_CFG3_IPM_DUMMY_BYTELEN_OFFSET	16

#define SPI_CFG3_IPM_CMD_PIN_MODE_MASK		GENMASK(1, 0)
#define SPI_CFG3_IPM_CMD_BYTELEN_MASK		GENMASK(11, 8)
#define SPI_CFG3_IPM_ADDR_BYTELEN_MASK		GENMASK(15, 12)
#define SPI_CFG3_IPM_DUMMY_BYTELEN_MASK		GENMASK(19, 16)

#define MT8173_SPI_MAX_PAD_SEL			3

#define MTK_SPI_PAUSE_INT_STATUS		0x2

#define MTK_SPI_IDLE				0
#define MTK_SPI_PAUSED				1

#define MTK_SPI_MAX_FIFO_SIZE			32U
#define MTK_SPI_PACKET_SIZE			1024
#define MTK_SPI_IPM_PACKET_SIZE			SZ_64K
#define MTK_SPI_IPM_PACKET_LOOP			SZ_256

#define MTK_SPI_32BITS_MASK			0xffffffff

#define DMA_ADDR_EXT_BITS			36
#define DMA_ADDR_DEF_BITS			32

#define CLK_TO_US(freq, clkcnt) DIV_ROUND_UP((clkcnt), (freq) / 1000000)

/* struct mtk_spim_capability
 * @enhance_timing:	Some IC design adjust cfg register to enhance time accuracy
 * @dma_ext:		Some IC support DMA addr extension
 * @ipm_design:		The IPM IP design improves some features, and supports dual/quad mode
 * @support_quad:	Whether quad mode is supported
 */
struct mtk_spim_capability {
	bool enhance_timing;
	bool dma_ext;
	bool ipm_design;
	bool support_quad;
};

/* struct mtk_spim_priv
 * @base:		Base address of the spi controller
 * @state:		Controller state
 * @sel_clk:		Pad clock
 * @spi_clk:		Core clock
 * @parent_clk:		Parent clock (needed for mediatek,spi-ipm, upstream DTSI)
 * @hclk:		HCLK clock (needed for mediatek,spi-ipm, upstream DTSI)
 * @pll_clk_rate:	Controller's PLL source clock rate, which is different
 *			from SPI bus clock rate
 * @xfer_len:		Current length of data for transfer
 * @hw_cap:		Controller capabilities
 * @tick_dly:		Used to postpone SPI sampling time
 * @sample_sel:		Sample edge of MISO
 * @dev:		udevice of this spi controller
 * @tx_dma:		Tx DMA address
 * @rx_dma:		Rx DMA address
 */
struct mtk_spim_priv {
	void __iomem *base;
	u32 state;
	struct clk sel_clk, spi_clk;
	struct clk parent_clk, hclk;
	u32 pll_clk_rate;
	u32 xfer_len;
	struct mtk_spim_capability hw_cap;
	u32 tick_dly;
	u32 sample_sel;

	struct device *dev;
	dma_addr_t tx_dma;
	dma_addr_t rx_dma;
};

static void mtk_spim_reset(struct mtk_spim_priv *priv)
{
	/* set the software reset bit in SPI_CMD_REG. */
	setbits_le32(priv->base + SPI_CMD_REG, SPI_CMD_RST);
	clrbits_le32(priv->base + SPI_CMD_REG, SPI_CMD_RST);
}

static int mtk_spim_hw_init(struct spi_slave *slave)
{
	struct udevice *bus = dev_get_parent(slave->dev);
	struct mtk_spim_priv *priv = dev_get_priv(bus);
	u16 cpha, cpol;
	u32 reg_val;

	cpha = slave->mode & SPI_CPHA ? 1 : 0;
	cpol = slave->mode & SPI_CPOL ? 1 : 0;

	if (priv->hw_cap.enhance_timing) {
		if (priv->hw_cap.ipm_design) {
			/* CFG3 reg only used for spi-mem,
			 * here write to default value
			 */
			writel(0x0, priv->base + SPI_CFG3_IPM_REG);
			clrsetbits_le32(priv->base + SPI_CMD_REG,
					SPI_CMD_IPM_GET_TICKDLY_MASK,
					priv->tick_dly <<
					SPI_CMD_IPM_GET_TICKDLY_OFFSET);
		} else {
			clrsetbits_le32(priv->base + SPI_CFG1_REG,
					SPI_CFG1_GET_TICKDLY_MASK,
					priv->tick_dly <<
					SPI_CFG1_GET_TICKDLY_OFFSET);
		}
	}

	reg_val = readl(priv->base + SPI_CMD_REG);
	if (priv->hw_cap.ipm_design) {
		/* SPI transfer without idle time until packet length done */
		reg_val |= SPI_CMD_IPM_NONIDLE_MODE;
		if (slave->mode & SPI_LOOP)
			reg_val |= SPI_CMD_IPM_SPIM_LOOP;
		else
			reg_val &= ~SPI_CMD_IPM_SPIM_LOOP;
	}

	if (cpha)
		reg_val |= SPI_CMD_CPHA;
	else
		reg_val &= ~SPI_CMD_CPHA;
	if (cpol)
		reg_val |= SPI_CMD_CPOL;
	else
		reg_val &= ~SPI_CMD_CPOL;

	/* set the mlsbx and mlsbtx */
	if (slave->mode & SPI_LSB_FIRST) {
		reg_val &= ~SPI_CMD_TXMSBF;
		reg_val &= ~SPI_CMD_RXMSBF;
	} else {
		reg_val |= SPI_CMD_TXMSBF;
		reg_val |= SPI_CMD_RXMSBF;
	}

	/* do not reverse tx/rx endian */
	reg_val &= ~SPI_CMD_TX_ENDIAN;
	reg_val &= ~SPI_CMD_RX_ENDIAN;

	if (priv->hw_cap.enhance_timing) {
		/* set CS polarity */
		if (slave->mode & SPI_CS_HIGH)
			reg_val |= SPI_CMD_CS_POL;
		else
			reg_val &= ~SPI_CMD_CS_POL;

		if (priv->sample_sel)
			reg_val |= SPI_CMD_SAMPLE_SEL;
		else
			reg_val &= ~SPI_CMD_SAMPLE_SEL;
	}

	/* Disable interrupt enable for pause mode & normal mode */
	reg_val &= ~(SPI_CMD_PAUSE_IE | SPI_CMD_FINISH_IE);

	/* disable dma mode */
	reg_val &= ~(SPI_CMD_TX_DMA | SPI_CMD_RX_DMA);

	/* disable deassert mode */
	reg_val &= ~SPI_CMD_DEASSERT;

	writel(reg_val, priv->base + SPI_CMD_REG);

	return 0;
}

static void mtk_spim_prepare_transfer(struct mtk_spim_priv *priv,
				      u32 speed_hz)
{
	u32 div, sck_time, cs_time, reg_val;

	if (speed_hz <= priv->pll_clk_rate / 4)
		div = DIV_ROUND_UP(priv->pll_clk_rate, speed_hz);
	else
		div = 4;

	sck_time = (div + 1) / 2;
	cs_time = sck_time * 2;

	if (priv->hw_cap.enhance_timing) {
		reg_val = ((sck_time - 1) & 0xffff)
			   << SPI_CFG2_SCK_HIGH_OFFSET;
		reg_val |= ((sck_time - 1) & 0xffff)
			   << SPI_CFG2_SCK_LOW_OFFSET;
		writel(reg_val, priv->base + SPI_CFG2_REG);

		reg_val = ((cs_time - 1) & 0xffff)
			   << SPI_ADJUST_CFG0_CS_HOLD_OFFSET;
		reg_val |= ((cs_time - 1) & 0xffff)
			   << SPI_ADJUST_CFG0_CS_SETUP_OFFSET;
		writel(reg_val, priv->base + SPI_CFG0_REG);
	} else {
		reg_val = ((sck_time - 1) & 0xff)
			   << SPI_CFG0_SCK_HIGH_OFFSET;
		reg_val |= ((sck_time - 1) & 0xff) << SPI_CFG0_SCK_LOW_OFFSET;
		reg_val |= ((cs_time - 1) & 0xff) << SPI_CFG0_CS_HOLD_OFFSET;
		reg_val |= ((cs_time - 1) & 0xff) << SPI_CFG0_CS_SETUP_OFFSET;
		writel(reg_val, priv->base + SPI_CFG0_REG);
	}

	reg_val = readl(priv->base + SPI_CFG1_REG);
	reg_val &= ~SPI_CFG1_CS_IDLE_MASK;
	reg_val |= ((cs_time - 1) & 0xff) << SPI_CFG1_CS_IDLE_OFFSET;
	writel(reg_val, priv->base + SPI_CFG1_REG);
}

/**
 * mtk_spim_setup_packet() - setup packet format.
 * @priv:	controller priv
 *
 * This controller sents/receives data in packets. The packet size is
 * configurable.
 *
 * This function calculates the maximum packet size available for current
 * data, and calculates the number of packets required to sent/receive data
 * as much as possible.
 */
static void mtk_spim_setup_packet(struct mtk_spim_priv *priv)
{
	u32 packet_size, packet_loop, reg_val;

	/* Calculate maximum packet size */
	if (priv->hw_cap.ipm_design)
		packet_size = min_t(u32,
				    priv->xfer_len,
				    MTK_SPI_IPM_PACKET_SIZE);
	else
		packet_size = min_t(u32,
				    priv->xfer_len,
				    MTK_SPI_PACKET_SIZE);

	/* Calculates number of packets to sent/receive */
	packet_loop = priv->xfer_len / packet_size;

	reg_val = readl(priv->base + SPI_CFG1_REG);
	if (priv->hw_cap.ipm_design)
		reg_val &= ~SPI_CFG1_IPM_PACKET_LENGTH_MASK;
	else
		reg_val &= ~SPI_CFG1_PACKET_LENGTH_MASK;

	reg_val |= (packet_size - 1) << SPI_CFG1_PACKET_LENGTH_OFFSET;

	reg_val &= ~SPI_CFG1_PACKET_LOOP_MASK;

	reg_val |= (packet_loop - 1) << SPI_CFG1_PACKET_LOOP_OFFSET;

	writel(reg_val, priv->base + SPI_CFG1_REG);
}

static void mtk_spim_enable_transfer(struct mtk_spim_priv *priv)
{
	u32 cmd;

	cmd = readl(priv->base + SPI_CMD_REG);
	if (priv->state == MTK_SPI_IDLE)
		cmd |= SPI_CMD_ACT;
	else
		cmd |= SPI_CMD_RESUME;
	writel(cmd, priv->base + SPI_CMD_REG);
}

static bool mtk_spim_supports_op(struct spi_slave *slave,
				 const struct spi_mem_op *op)
{
	struct udevice *bus = dev_get_parent(slave->dev);
	struct mtk_spim_priv *priv = dev_get_priv(bus);

	if (!spi_mem_default_supports_op(slave, op))
		return false;

	if (op->cmd.buswidth == 0 || op->cmd.buswidth > 4 ||
	    op->addr.buswidth > 4 || op->dummy.buswidth > 4 ||
	    op->data.buswidth > 4)
		return false;

	if (!priv->hw_cap.support_quad && (op->cmd.buswidth > 2 ||
	    op->addr.buswidth > 2 || op->dummy.buswidth > 2 ||
	    op->data.buswidth > 2))
		return false;

	if (op->addr.nbytes && op->dummy.nbytes &&
	    op->addr.buswidth != op->dummy.buswidth)
		return false;

	if (op->addr.nbytes + op->dummy.nbytes > 16)
		return false;

	if (op->data.nbytes > MTK_SPI_IPM_PACKET_SIZE) {
		if (op->data.nbytes / MTK_SPI_IPM_PACKET_SIZE >
		    MTK_SPI_IPM_PACKET_LOOP ||
		    op->data.nbytes % MTK_SPI_IPM_PACKET_SIZE != 0)
			return false;
	}

	return true;
}

static void mtk_spim_setup_dma_xfer(struct mtk_spim_priv *priv,
				    const struct spi_mem_op *op)
{
	writel((u32)(priv->tx_dma & MTK_SPI_32BITS_MASK),
	       priv->base + SPI_TX_SRC_REG);

	if (priv->hw_cap.dma_ext)
		writel((u32)(priv->tx_dma >> 32),
		       priv->base + SPI_TX_SRC_REG_64);

	if (op->data.dir == SPI_MEM_DATA_IN) {
		writel((u32)(priv->rx_dma & MTK_SPI_32BITS_MASK),
		       priv->base + SPI_RX_DST_REG);

		if (priv->hw_cap.dma_ext)
			writel((u32)(priv->rx_dma >> 32),
			       priv->base + SPI_RX_DST_REG_64);
	}
}

static int mtk_spim_transfer_wait(struct spi_slave *slave,
				  const struct spi_mem_op *op)
{
	struct udevice *bus = dev_get_parent(slave->dev);
	struct mtk_spim_priv *priv = dev_get_priv(bus);
	u32 pll_clk, sck_l, sck_h, clk_count, reg;
	ulong us = 1;
	int ret = 0;

	if (op->data.dir == SPI_MEM_NO_DATA)
		clk_count = 32;
	else
		clk_count = op->data.nbytes;

	pll_clk = priv->pll_clk_rate;
	sck_l = readl(priv->base + SPI_CFG2_REG) >> SPI_CFG2_SCK_LOW_OFFSET;
	sck_h = readl(priv->base + SPI_CFG2_REG) & SPI_CFG2_SCK_HIGH_MASK;
	do_div(pll_clk, sck_l + sck_h + 2);

	us = CLK_TO_US(pll_clk, clk_count * 8);
	us += 1000 * 1000; /* 1s tolerance */

	if (us > UINT_MAX)
		us = UINT_MAX;

	ret = readl_poll_timeout(priv->base + SPI_STATUS_REG, reg,
				 reg & 0x1, us);
	if (ret < 0) {
		dev_err(priv->dev, "transfer timeout, val: 0x%lx\n", us);
		return -ETIMEDOUT;
	}

	return 0;
}

static int mtk_spim_exec_op(struct spi_slave *slave,
			    const struct spi_mem_op *op)
{
	struct udevice *bus = dev_get_parent(slave->dev);
	struct mtk_spim_priv *priv = dev_get_priv(bus);
	u32 reg_val, nio = 1, tx_size;
	char *tx_tmp_buf;
	char *rx_tmp_buf;
	int i, ret = 0;

	mtk_spim_reset(priv);
	mtk_spim_hw_init(slave);
	mtk_spim_prepare_transfer(priv, slave->max_hz);

	reg_val = readl(priv->base + SPI_CFG3_IPM_REG);
	/* opcode byte len */
	reg_val &= ~SPI_CFG3_IPM_CMD_BYTELEN_MASK;
	reg_val |= 1 << SPI_CFG3_IPM_CMD_BYTELEN_OFFSET;

	/* addr & dummy byte len */
	if (op->addr.nbytes || op->dummy.nbytes)
		reg_val |= (op->addr.nbytes + op->dummy.nbytes) <<
			    SPI_CFG3_IPM_ADDR_BYTELEN_OFFSET;

	/* data byte len */
	if (!op->data.nbytes) {
		reg_val |= SPI_CFG3_IPM_NODATA_FLAG;
		writel(0, priv->base + SPI_CFG1_REG);
	} else {
		reg_val &= ~SPI_CFG3_IPM_NODATA_FLAG;
		priv->xfer_len = op->data.nbytes;
		mtk_spim_setup_packet(priv);
	}

	if (op->addr.nbytes || op->dummy.nbytes) {
		if (op->addr.buswidth == 1 || op->dummy.buswidth == 1)
			reg_val |= SPI_CFG3_IPM_XMODE_EN;
		else
			reg_val &= ~SPI_CFG3_IPM_XMODE_EN;
	}

	if (op->addr.buswidth == 2 ||
	    op->dummy.buswidth == 2 ||
	    op->data.buswidth == 2)
		nio = 2;
	else if (op->addr.buswidth == 4 ||
		 op->dummy.buswidth == 4 ||
		 op->data.buswidth == 4)
		nio = 4;

	reg_val &= ~SPI_CFG3_IPM_CMD_PIN_MODE_MASK;
	reg_val |= PIN_MODE_CFG(nio) << SPI_CFG3_IPM_PIN_MODE_OFFSET;

	reg_val |= SPI_CFG3_IPM_HALF_DUPLEX_EN;
	if (op->data.dir == SPI_MEM_DATA_IN)
		reg_val |= SPI_CFG3_IPM_HALF_DUPLEX_DIR;
	else
		reg_val &= ~SPI_CFG3_IPM_HALF_DUPLEX_DIR;
	writel(reg_val, priv->base + SPI_CFG3_IPM_REG);

	tx_size = 1 + op->addr.nbytes + op->dummy.nbytes;
	if (op->data.dir == SPI_MEM_DATA_OUT)
		tx_size += op->data.nbytes;

	tx_size = max(tx_size, (u32)32);

	/* Fill up tx data */
	tx_tmp_buf = kzalloc(tx_size, GFP_KERNEL);
	if (!tx_tmp_buf) {
		ret = -ENOMEM;
		goto exit;
	}

	tx_tmp_buf[0] = op->cmd.opcode;

	if (op->addr.nbytes) {
		for (i = 0; i < op->addr.nbytes; i++)
			tx_tmp_buf[i + 1] = op->addr.val >>
					(8 * (op->addr.nbytes - i - 1));
	}

	if (op->dummy.nbytes)
		memset(tx_tmp_buf + op->addr.nbytes + 1, 0xff,
		       op->dummy.nbytes);

	if (op->data.nbytes && op->data.dir == SPI_MEM_DATA_OUT)
		memcpy(tx_tmp_buf + op->dummy.nbytes + op->addr.nbytes + 1,
		       op->data.buf.out, op->data.nbytes);
	/* Finish filling up tx data */

	priv->tx_dma = dma_map_single(tx_tmp_buf, tx_size, DMA_TO_DEVICE);
	if (dma_mapping_error(priv->dev, priv->tx_dma)) {
		ret = -ENOMEM;
		goto tx_free;
	}

	if (op->data.dir == SPI_MEM_DATA_IN) {
		if (!IS_ALIGNED((size_t)op->data.buf.in, 4)) {
			rx_tmp_buf = kzalloc(op->data.nbytes, GFP_KERNEL);
			if (!rx_tmp_buf) {
				ret = -ENOMEM;
				goto tx_unmap;
			}
		} else {
			rx_tmp_buf = op->data.buf.in;
		}

		priv->rx_dma = dma_map_single(rx_tmp_buf, op->data.nbytes,
					      DMA_FROM_DEVICE);
		if (dma_mapping_error(priv->dev, priv->rx_dma)) {
			ret = -ENOMEM;
			goto rx_free;
		}
	}

	reg_val = readl(priv->base + SPI_CMD_REG);
	reg_val |= SPI_CMD_TX_DMA;
	if (op->data.dir == SPI_MEM_DATA_IN)
		reg_val |= SPI_CMD_RX_DMA;

	writel(reg_val, priv->base + SPI_CMD_REG);

	mtk_spim_setup_dma_xfer(priv, op);

	mtk_spim_enable_transfer(priv);

	/* Wait for the interrupt. */
	ret = mtk_spim_transfer_wait(slave, op);
	if (ret)
		goto rx_unmap;

	if (op->data.dir == SPI_MEM_DATA_IN &&
	    !IS_ALIGNED((size_t)op->data.buf.in, 4))
		memcpy(op->data.buf.in, rx_tmp_buf, op->data.nbytes);

rx_unmap:
	/* spi disable dma */
	reg_val = readl(priv->base + SPI_CMD_REG);
	reg_val &= ~SPI_CMD_TX_DMA;
	if (op->data.dir == SPI_MEM_DATA_IN)
		reg_val &= ~SPI_CMD_RX_DMA;
	writel(reg_val, priv->base + SPI_CMD_REG);

	writel(0, priv->base + SPI_TX_SRC_REG);
	writel(0, priv->base + SPI_RX_DST_REG);

	if (op->data.dir == SPI_MEM_DATA_IN)
		dma_unmap_single(priv->rx_dma,
				 op->data.nbytes, DMA_FROM_DEVICE);
rx_free:
	if (op->data.dir == SPI_MEM_DATA_IN &&
	    !IS_ALIGNED((size_t)op->data.buf.in, 4))
		kfree(rx_tmp_buf);
tx_unmap:
	dma_unmap_single(priv->tx_dma,
			 tx_size, DMA_TO_DEVICE);
tx_free:
	kfree(tx_tmp_buf);
exit:
	return ret;
}

static int mtk_spim_adjust_op_size(struct spi_slave *slave,
				   struct spi_mem_op *op)
{
	int opcode_len;

	if (!op->data.nbytes)
		return 0;

	if (op->data.dir != SPI_MEM_NO_DATA) {
		opcode_len = 1 + op->addr.nbytes + op->dummy.nbytes;
		if (opcode_len + op->data.nbytes > MTK_SPI_IPM_PACKET_SIZE) {
			op->data.nbytes = MTK_SPI_IPM_PACKET_SIZE - opcode_len;
			/* force data buffer dma-aligned. */
			op->data.nbytes -= op->data.nbytes % 4;
		}
	}

	return 0;
}

static int mtk_spim_get_attr(struct mtk_spim_priv *priv, struct udevice *dev)
{
	int ret;

	priv->hw_cap.enhance_timing = dev_read_bool(dev, "enhance_timing");
	priv->hw_cap.dma_ext = dev_read_bool(dev, "dma_ext");
	priv->hw_cap.ipm_design = dev_read_bool(dev, "ipm_design");
	priv->hw_cap.support_quad = dev_read_bool(dev, "support_quad");

	ret = dev_read_u32(dev, "tick_dly", &priv->tick_dly);
	if (ret < 0)
		dev_err(priv->dev, "tick dly not set.\n");

	ret = dev_read_u32(dev, "sample_sel", &priv->sample_sel);
	if (ret < 0)
		dev_err(priv->dev, "sample sel not set.\n");

	return ret;
}

static int mtk_spim_probe(struct udevice *dev)
{
	struct mtk_spim_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	/*
	 * Upstream linux driver for ipm design enable all the modes
	 * and setup the calibrarion values directly in the driver with
	 * standard values.
	 */
	if (device_is_compatible(dev, "mediatek,spi-ipm")) {
		priv->hw_cap.enhance_timing = true;
		priv->hw_cap.dma_ext = true;
		priv->hw_cap.ipm_design = true;
		priv->hw_cap.support_quad = true;
		priv->sample_sel = 0;
		priv->tick_dly = 2;
	} else {
		mtk_spim_get_attr(priv, dev);
	}

	ret = clk_get_by_name(dev, "sel-clk", &priv->sel_clk);
	if (ret < 0) {
		dev_err(dev, "failed to get sel-clk\n");
		return ret;
	}

	ret = clk_get_by_name(dev, "spi-clk", &priv->spi_clk);
	if (ret < 0) {
		dev_err(dev, "failed to get spi-clk\n");
		return ret;
	}

	/*
	 * Upstream DTSI use a different compatible that provide additional
	 * clock instead of the assigned-clock implementation.
	 */
	if (device_is_compatible(dev, "mediatek,spi-ipm")) {
		ret = clk_get_by_name(dev, "parent-clk", &priv->parent_clk);
		if (ret < 0) {
			dev_err(dev, "failed to get parent-clk\n");
			return ret;
		}

		ret = clk_get_by_name(dev, "hclk", &priv->hclk);
		if (ret < 0) {
			dev_err(dev, "failed to get hclk\n");
			return ret;
		}

		clk_enable(&priv->parent_clk);
		clk_set_parent(&priv->sel_clk, &priv->parent_clk);

		clk_enable(&priv->hclk);
	}

	clk_enable(&priv->spi_clk);
	clk_enable(&priv->sel_clk);

	priv->pll_clk_rate = clk_get_rate(&priv->spi_clk);
	if (priv->pll_clk_rate == 0)
		return -EINVAL;

	return 0;
}

static int mtk_spim_set_speed(struct udevice *dev, uint speed)
{
	return 0;
}

static int mtk_spim_set_mode(struct udevice *dev, uint mode)
{
	return 0;
}

static const struct spi_controller_mem_ops mtk_spim_mem_ops = {
	.adjust_op_size = mtk_spim_adjust_op_size,
	.supports_op = mtk_spim_supports_op,
	.exec_op = mtk_spim_exec_op
};

static const struct dm_spi_ops mtk_spim_ops = {
	.mem_ops = &mtk_spim_mem_ops,
	.set_speed = mtk_spim_set_speed,
	.set_mode = mtk_spim_set_mode,
};

static const struct udevice_id mtk_spim_ids[] = {
	{ .compatible = "mediatek,ipm-spi" },
	{ .compatible = "mediatek,spi-ipm", },
	{}
};

U_BOOT_DRIVER(mtk_spim) = {
	.name = "mtk_spim",
	.id = UCLASS_SPI,
	.of_match = mtk_spim_ids,
	.ops = &mtk_spim_ops,
	.priv_auto = sizeof(struct mtk_spim_priv),
	.probe = mtk_spim_probe,
};
