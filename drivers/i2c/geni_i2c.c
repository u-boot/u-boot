// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2024, Linaro Limited
 * Author: Neil Armstrong <neil.armstrong@linaro.org>
 *
 * Based on Linux driver: drivers/i2c/busses/i2c-qcom-geni.c
 */

#include <log.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/bitops.h>
#include <asm/io.h>
#include <i2c.h>
#include <fdtdec.h>
#include <clk.h>
#include <reset.h>
#include <time.h>
#include <soc/qcom/geni-se.h>

#define SE_I2C_TX_TRANS_LEN		0x26c
#define SE_I2C_RX_TRANS_LEN		0x270
#define SE_I2C_SCL_COUNTERS		0x278

#define SE_I2C_ERR  (M_CMD_OVERRUN_EN | M_ILLEGAL_CMD_EN | M_CMD_FAILURE_EN |\
			M_GP_IRQ_1_EN | M_GP_IRQ_3_EN | M_GP_IRQ_4_EN)
#define SE_I2C_ABORT		BIT(1)

/* M_CMD OP codes for I2C */
#define I2C_WRITE		0x1
#define I2C_READ		0x2
#define I2C_WRITE_READ		0x3
#define I2C_ADDR_ONLY		0x4
#define I2C_BUS_CLEAR		0x6
#define I2C_STOP_ON_BUS		0x7
/* M_CMD params for I2C */
#define PRE_CMD_DELAY		BIT(0)
#define TIMESTAMP_BEFORE	BIT(1)
#define STOP_STRETCH		BIT(2)
#define TIMESTAMP_AFTER		BIT(3)
#define POST_COMMAND_DELAY	BIT(4)
#define IGNORE_ADD_NACK		BIT(6)
#define READ_FINISHED_WITH_ACK	BIT(7)
#define BYPASS_ADDR_PHASE	BIT(8)
#define SLV_ADDR_MSK		GENMASK(15, 9)
#define SLV_ADDR_SHFT		9
/* I2C SCL COUNTER fields */
#define HIGH_COUNTER_MSK	GENMASK(29, 20)
#define HIGH_COUNTER_SHFT	20
#define LOW_COUNTER_MSK		GENMASK(19, 10)
#define LOW_COUNTER_SHFT	10
#define CYCLE_COUNTER_MSK	GENMASK(9, 0)

#define I2C_PACK_TX		BIT(0)
#define I2C_PACK_RX		BIT(1)

#define PACKING_BYTES_PW	4

#define GENI_I2C_IS_MASTER_HUB	BIT(0)

#define I2C_TIMEOUT_MS		100

struct geni_i2c_clk_fld {
	u32	clk_freq_out;
	u8	clk_div;
	u8	t_high_cnt;
	u8	t_low_cnt;
	u8	t_cycle_cnt;
};

struct geni_i2c_priv {
	fdt_addr_t wrapper;
	phys_addr_t base;
	struct clk core;
	struct clk se;
	u32 tx_wm;
	bool is_master_hub;
	const struct geni_i2c_clk_fld *clk_fld;
};

/*
 * Hardware uses the underlying formula to calculate time periods of
 * SCL clock cycle. Firmware uses some additional cycles excluded from the
 * below formula and it is confirmed that the time periods are within
 * specification limits.
 *
 * time of high period of SCL: t_high = (t_high_cnt * clk_div) / source_clock
 * time of low period of SCL: t_low = (t_low_cnt * clk_div) / source_clock
 * time of full period of SCL: t_cycle = (t_cycle_cnt * clk_div) / source_clock
 * clk_freq_out = t / t_cycle
 * source_clock = 19.2 MHz
 */
static const struct geni_i2c_clk_fld geni_i2c_clk_map[] = {
	{I2C_SPEED_STANDARD_RATE, 7, 10, 11, 26},
	{I2C_SPEED_FAST_RATE, 2,  5, 12, 24},
	{I2C_SPEED_FAST_PLUS_RATE, 1, 3,  9, 18},
};

static int geni_i2c_clk_map_idx(struct geni_i2c_priv *geni, unsigned int clk_freq)
{
	const struct geni_i2c_clk_fld *itr = geni_i2c_clk_map;
	int i;

	for (i = 0; i < ARRAY_SIZE(geni_i2c_clk_map); i++, itr++) {
		if (itr->clk_freq_out == clk_freq) {
			geni->clk_fld = itr;
			return 0;
		}
	}

	return -EINVAL;
}

static void geni_i2c_setup_m_cmd(struct geni_i2c_priv *geni, u32 cmd, u32 params)
{
	u32 m_cmd;

	m_cmd = (cmd << M_OPCODE_SHFT) | (params & M_PARAMS_MSK);
	writel(m_cmd, geni->base + SE_GENI_M_CMD0);
}

static void qcom_geni_i2c_conf(struct geni_i2c_priv *geni)
{
	const struct geni_i2c_clk_fld *itr = geni->clk_fld;
	u32 val;

	writel(0, geni->base + SE_GENI_CLK_SEL);

	val = (itr->clk_div << CLK_DIV_SHFT) | SER_CLK_EN;
	writel(val, geni->base + GENI_SER_M_CLK_CFG);

	val = itr->t_high_cnt << HIGH_COUNTER_SHFT;
	val |= itr->t_low_cnt << LOW_COUNTER_SHFT;
	val |= itr->t_cycle_cnt;
	writel(val, geni->base + SE_I2C_SCL_COUNTERS);

	writel(0xffffffff, geni->base + SE_GENI_M_IRQ_CLEAR);
}

static int geni_i2c_fifo_tx_fill(struct geni_i2c_priv *geni, struct i2c_msg *msg)
{
	ulong start = get_timer(0);
	ulong cur_xfer = 0;
	int i;

	while (get_timer(start) < I2C_TIMEOUT_MS) {
		u32 status = readl(geni->base + SE_GENI_M_IRQ_STATUS);

		if (status & (M_CMD_ABORT_EN |
			      M_CMD_OVERRUN_EN |
			      M_ILLEGAL_CMD_EN |
			      M_CMD_FAILURE_EN |
			      M_GP_IRQ_1_EN |
			      M_GP_IRQ_3_EN |
			      M_GP_IRQ_4_EN)) {
			writel(status, geni->base + SE_GENI_M_IRQ_CLEAR);
			writel(0, geni->base + SE_GENI_TX_WATERMARK_REG);
			return -EREMOTEIO;
		}

		if ((status & M_TX_FIFO_WATERMARK_EN) == 0) {
			udelay(1);
			goto skip_fill;
		}

		for (i = 0; i < geni->tx_wm; i++) {
			u32 temp, tx = 0;
			unsigned int p = 0;

			while (cur_xfer < msg->len && p < sizeof(tx)) {
				temp = msg->buf[cur_xfer++];
				tx |= temp << (p * 8);
				p++;
			}

			writel(tx, geni->base + SE_GENI_TX_FIFOn);

			if (cur_xfer == msg->len) {
				writel(0, geni->base + SE_GENI_TX_WATERMARK_REG);
				break;
			}
		}

skip_fill:
		writel(status, geni->base + SE_GENI_M_IRQ_CLEAR);

		if (status & M_CMD_DONE_EN)
			return 0;
	}

	return -ETIMEDOUT;
}

static int geni_i2c_fifo_rx_drain(struct geni_i2c_priv *geni, struct i2c_msg *msg)
{
	ulong start = get_timer(0);
	ulong cur_xfer = 0;
	int i;

	while (get_timer(start) < I2C_TIMEOUT_MS) {
		u32 status = readl(geni->base + SE_GENI_M_IRQ_STATUS);
		u32 rxstatus = readl(geni->base + SE_GENI_RX_FIFO_STATUS);
		u32 rxcnt = rxstatus & RX_FIFO_WC_MSK;

		if (status & (M_CMD_ABORT_EN |
			      M_CMD_FAILURE_EN |
			      M_CMD_OVERRUN_EN |
			      M_ILLEGAL_CMD_EN |
			      M_GP_IRQ_1_EN |
			      M_GP_IRQ_3_EN |
			      M_GP_IRQ_4_EN)) {
			writel(status, geni->base + SE_GENI_M_IRQ_CLEAR);
			return -EREMOTEIO;
		}

		if ((status & (M_RX_FIFO_WATERMARK_EN | M_RX_FIFO_LAST_EN)) == 0) {
			udelay(1);
			goto skip_drain;
		}

		for (i = 0; cur_xfer < msg->len && i < rxcnt; i++) {
			u32 rx = readl(geni->base + SE_GENI_RX_FIFOn);
			unsigned int p = 0;

			while (cur_xfer < msg->len && p < sizeof(rx)) {
				msg->buf[cur_xfer++] = rx & 0xff;
				rx >>= 8;
				p++;
			}
		}

skip_drain:
		writel(status, geni->base + SE_GENI_M_IRQ_CLEAR);

		if (status & M_CMD_DONE_EN)
			return 0;
	}

	return -ETIMEDOUT;
}

static int geni_i2c_xfer_tx(struct geni_i2c_priv *geni, struct i2c_msg *msg, u32 params)
{
	writel(msg->len, geni->base + SE_I2C_TX_TRANS_LEN);
	geni_i2c_setup_m_cmd(geni, I2C_WRITE, params);
	writel(1, geni->base + SE_GENI_TX_WATERMARK_REG);

	return geni_i2c_fifo_tx_fill(geni, msg);
}

static int geni_i2c_xfer_rx(struct geni_i2c_priv *geni, struct i2c_msg *msg, u32 params)
{
	writel(msg->len, geni->base + SE_I2C_RX_TRANS_LEN);
	geni_i2c_setup_m_cmd(geni, I2C_READ, params);

	return geni_i2c_fifo_rx_drain(geni, msg);
}

static int geni_i2c_xfer(struct udevice *bus, struct i2c_msg msgs[], int num)
{
	struct geni_i2c_priv *geni = dev_get_priv(bus);
	int i, ret = 0;

	qcom_geni_i2c_conf(geni);

	for (i = 0; i < num; i++) {
		struct i2c_msg *msg = &msgs[i];
		u32 m_param = i < (num - 1) ? STOP_STRETCH : 0;

		m_param |= ((msg->addr << SLV_ADDR_SHFT) & SLV_ADDR_MSK);

		if (msg->flags & I2C_M_RD)
			ret = geni_i2c_xfer_rx(geni, msg, m_param);
		else
			ret = geni_i2c_xfer_tx(geni, msg, m_param);

		if (ret)
			break;
	}

	if (ret) {
		if (ret == -ETIMEDOUT) {
			u32 status;

			writel(M_GENI_CMD_ABORT, geni->base + SE_GENI_M_CMD_CTRL_REG);

			/* Wait until Abort has finished */
			do {
				status = readl(geni->base + SE_GENI_M_IRQ_STATUS);
			} while ((status & M_CMD_ABORT_EN) == 0);

			writel(status, geni->base + SE_GENI_M_IRQ_STATUS);
		}

		return ret;
	}

	return 0;
}

static int geni_i2c_enable_clocks(struct udevice *dev, struct geni_i2c_priv *geni)
{
	int ret;

	if (geni->is_master_hub) {
		ret = clk_enable(&geni->core);
		if (ret) {
			dev_err(dev, "clk_enable core failed %d\n", ret);
			return ret;
		}
	}

	ret = clk_enable(&geni->se);
	if (ret) {
		dev_err(dev, "clk_enable se failed %d\n", ret);
		return ret;
	}

	return 0;
}

static int geni_i2c_disable_clocks(struct udevice *dev, struct geni_i2c_priv *geni)
{
	int ret;

	if (geni->is_master_hub) {
		ret = clk_disable(&geni->core);
		if (ret) {
			dev_err(dev, "clk_enable core failed %d\n", ret);
			return ret;
		}
	}

	ret = clk_disable(&geni->se);
	if (ret) {
		dev_err(dev, "clk_enable se failed %d\n", ret);
		return ret;
	}

	return 0;
}

#define NUM_PACKING_VECTORS 4
#define PACKING_START_SHIFT 5
#define PACKING_DIR_SHIFT 4
#define PACKING_LEN_SHIFT 1
#define PACKING_STOP_BIT BIT(0)
#define PACKING_VECTOR_SHIFT 10
static void geni_i2c_config_packing(struct geni_i2c_priv *geni, int bpw,
				    int pack_words, bool msb_to_lsb,
				    bool tx_cfg, bool rx_cfg)
{
	u32 cfg0, cfg1, cfg[NUM_PACKING_VECTORS] = {0};
	int len;
	int temp_bpw = bpw;
	int idx_start = msb_to_lsb ? bpw - 1 : 0;
	int idx = idx_start;
	int idx_delta = msb_to_lsb ? -BITS_PER_BYTE : BITS_PER_BYTE;
	int ceil_bpw = ALIGN(bpw, BITS_PER_BYTE);
	int iter = (ceil_bpw * pack_words) / BITS_PER_BYTE;
	int i;

	if (iter <= 0 || iter > NUM_PACKING_VECTORS)
		return;

	for (i = 0; i < iter; i++) {
		len = min_t(int, temp_bpw, BITS_PER_BYTE) - 1;
		cfg[i] = idx << PACKING_START_SHIFT;
		cfg[i] |= msb_to_lsb << PACKING_DIR_SHIFT;
		cfg[i] |= len << PACKING_LEN_SHIFT;

		if (temp_bpw <= BITS_PER_BYTE) {
			idx = ((i + 1) * BITS_PER_BYTE) + idx_start;
			temp_bpw = bpw;
		} else {
			idx = idx + idx_delta;
			temp_bpw = temp_bpw - BITS_PER_BYTE;
		}
	}
	cfg[iter - 1] |= PACKING_STOP_BIT;
	cfg0 = cfg[0] | (cfg[1] << PACKING_VECTOR_SHIFT);
	cfg1 = cfg[2] | (cfg[3] << PACKING_VECTOR_SHIFT);

	if (tx_cfg) {
		writel(cfg0, geni->base + SE_GENI_TX_PACKING_CFG0);
		writel(cfg1, geni->base + SE_GENI_TX_PACKING_CFG1);
	}
	if (rx_cfg) {
		writel(cfg0, geni->base + SE_GENI_RX_PACKING_CFG0);
		writel(cfg1, geni->base + SE_GENI_RX_PACKING_CFG1);
	}

	/*
	 * Number of protocol words in each FIFO entry
	 * 0 - 4x8, four words in each entry, max word size of 8 bits
	 * 1 - 2x16, two words in each entry, max word size of 16 bits
	 * 2 - 1x32, one word in each entry, max word size of 32 bits
	 * 3 - undefined
	 */
	if (pack_words || bpw == 32)
		writel(bpw / 16, geni->base + SE_GENI_BYTE_GRAN);
}

static void geni_i2c_init(struct geni_i2c_priv *geni, unsigned int tx_depth)
{
	u32 val;

	writel(0, geni->base + SE_GSI_EVENT_EN);
	writel(0xffffffff, geni->base + SE_GENI_M_IRQ_CLEAR);
	writel(0xffffffff, geni->base + SE_GENI_S_IRQ_CLEAR);
	writel(0xffffffff, geni->base + SE_IRQ_EN);

	val = readl(geni->base + GENI_CGC_CTRL);
	val |= DEFAULT_CGC_EN;
	writel(val, geni->base + GENI_CGC_CTRL);

	writel(DEFAULT_IO_OUTPUT_CTRL_MSK, geni->base + GENI_OUTPUT_CTRL);
	writel(FORCE_DEFAULT, geni->base + GENI_FORCE_DEFAULT_REG);

	val = readl(geni->base + SE_IRQ_EN);
	val |= GENI_M_IRQ_EN | GENI_S_IRQ_EN;
	writel(val, geni->base + SE_IRQ_EN);

	val = readl(geni->base + SE_GENI_DMA_MODE_EN);
	val &= ~GENI_DMA_MODE_EN;
	writel(val, geni->base + SE_GENI_DMA_MODE_EN);

	writel(0, geni->base + SE_GSI_EVENT_EN);

	writel(tx_depth - 1, geni->base + SE_GENI_RX_WATERMARK_REG);
	writel(tx_depth, geni->base + SE_GENI_RX_RFR_WATERMARK_REG);

	val = readl(geni->base + SE_GENI_M_IRQ_EN);
	val |= M_COMMON_GENI_M_IRQ_EN;
	val |= M_CMD_DONE_EN | M_TX_FIFO_WATERMARK_EN;
	val |= M_RX_FIFO_WATERMARK_EN | M_RX_FIFO_LAST_EN;
	writel(val, geni->base + SE_GENI_M_IRQ_EN);

	val = readl(geni->base + SE_GENI_S_IRQ_EN);
	val |= S_COMMON_GENI_S_IRQ_EN;
	writel(val, geni->base + SE_GENI_S_IRQ_EN);
}

static u32 geni_i2c_get_tx_fifo_depth(struct geni_i2c_priv *geni)
{
	u32 val, hw_version, hw_major, hw_minor, tx_fifo_depth_mask;

	hw_version = readl(geni->wrapper + QUP_HW_VER_REG);
	hw_major = GENI_SE_VERSION_MAJOR(hw_version);
	hw_minor = GENI_SE_VERSION_MINOR(hw_version);

	if ((hw_major == 3 && hw_minor >= 10) || hw_major > 3)
		tx_fifo_depth_mask = TX_FIFO_DEPTH_MSK_256_BYTES;
	else
		tx_fifo_depth_mask = TX_FIFO_DEPTH_MSK;

	val = readl(geni->base + SE_HW_PARAM_0);

	return (val & tx_fifo_depth_mask) >> TX_FIFO_DEPTH_SHFT;
}

static int geni_i2c_probe(struct udevice *dev)
{
	ofnode parent_node = ofnode_get_parent(dev_ofnode(dev));
	struct geni_i2c_priv *geni = dev_get_priv(dev);
	u32 proto, tx_depth, fifo_disable;
	int ret;

	geni->is_master_hub = dev_get_driver_data(dev) & GENI_I2C_IS_MASTER_HUB;

	geni->wrapper = ofnode_get_addr(parent_node);
	if (geni->wrapper == FDT_ADDR_T_NONE)
		return -EINVAL;

	geni->base = (phys_addr_t)dev_read_addr_ptr(dev);
	if (!geni->base)
		return -EINVAL;

	if (geni->is_master_hub) {
		ret = clk_get_by_name(dev, "core", &geni->core);
		if (ret) {
			dev_err(dev, "clk_get_by_name(core) failed: %d\n", ret);
			return ret;
		}
	}

	ret = clk_get_by_name(dev, "se", &geni->se);
	if (ret) {
		dev_err(dev, "clk_get_by_name(se) failed: %d\n", ret);
		return ret;
	}

	geni_i2c_enable_clocks(dev, geni);

	proto = readl(geni->base + GENI_FW_REVISION_RO);
	proto &= FW_REV_PROTOCOL_MSK;
	proto >>= FW_REV_PROTOCOL_SHFT;

	if (proto != GENI_SE_I2C) {
		dev_err(dev, "Invalid proto %d\n", proto);
		geni_i2c_disable_clocks(dev, geni);
		return -ENXIO;
	}

	fifo_disable = readl(geni->base + GENI_IF_DISABLE_RO) & FIFO_IF_DISABLE;
	if (fifo_disable) {
		geni_i2c_disable_clocks(dev, geni);
		dev_err(dev, "FIFO mode disabled, DMA mode unsupported\n");
		return -ENXIO;
	}

	if (!geni->is_master_hub) {
		tx_depth = geni_i2c_get_tx_fifo_depth(geni);
		if (!tx_depth) {
			geni_i2c_disable_clocks(dev, geni);
			dev_err(dev, "Invalid TX FIFO depth\n");
			return -ENXIO;
		}
	} else {
		tx_depth = 16;
	}
	geni->tx_wm = tx_depth - 1;

	geni_i2c_init(geni, tx_depth);
	geni_i2c_config_packing(geni, BITS_PER_BYTE,
				PACKING_BYTES_PW, true, true, true);

	/* Setup for standard rate */
	return geni_i2c_clk_map_idx(geni, I2C_SPEED_STANDARD_RATE);
}

static int geni_i2c_set_bus_speed(struct udevice *dev, unsigned int clk_freq)
{
	struct geni_i2c_priv *geni = dev_get_priv(dev);

	return geni_i2c_clk_map_idx(geni, clk_freq);
}

static const struct dm_i2c_ops geni_i2c_ops = {
	.xfer		= geni_i2c_xfer,
	.set_bus_speed	= geni_i2c_set_bus_speed,
};

static const struct udevice_id geni_i2c_ids[] = {
	{ .compatible = "qcom,geni-i2c" },
	{ .compatible = "qcom,geni-i2c-master-hub", .data = GENI_I2C_IS_MASTER_HUB},
	{}
};

U_BOOT_DRIVER(i2c_geni) = {
	.name	= "i2c_geni",
	.id	= UCLASS_I2C,
	.of_match = geni_i2c_ids,
	.probe	= geni_i2c_probe,
	.priv_auto = sizeof(struct geni_i2c_priv),
	.ops	= &geni_i2c_ops,
};

static const struct udevice_id geni_i2c_master_hub_ids[] = {
	{ .compatible = "qcom,geni-se-i2c-master-hub" },
	{ }
};

U_BOOT_DRIVER(geni_i2c_master_hub) = {
	.name = "geni-se-master-hub",
	.id = UCLASS_NOP,
	.of_match = geni_i2c_master_hub_ids,
	.bind = dm_scan_fdt_dev,
	.flags = DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
