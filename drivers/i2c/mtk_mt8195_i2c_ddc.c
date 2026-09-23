// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 MediaTek Inc.
 * Copyright (c) 2026 BayLibre, SAS
 *
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <edid.h>
#include <i2c.h>
#include <linux/delay.h>
#include <time.h>

#define DDC2_CLOCK 572 /* BIM=208M/(v*4) = 90Khz */
#define DDC2_CLOCK_EDID 832 /* BIM=208M/(v*4) = 62.5Khz */

#define SCDC_I2C_SLAVE_ADDRESS 0x54

#define HDCP2X_DDCM_STATUS 0xC68

#define SCDC_CTRL 0xC18

#define CLEAR_FIFO 0x9

#define CLOCK_SCL 0xA

#define ENH_READ_NO_ACK 0x4

#define DDC_CMD GENMASK(31, 28)
#define DDC_CMD_SHIFT (28)
#define DDC_CTRL 0xC10
#define DDC_DATA_OUT GENMASK(23, 16)
#define DDC_DATA_OUT_SHIFT (16)
#define DDC_DELAY_CNT GENMASK(31, 16)
#define DDC_DELAY_CNT_SHIFT (16)
#define DDC_DIN_CNT_SHIFT (16)
#define DDC_I2C_BUS_LOW BIT(11)
#define DDC_I2C_IN_PROG BIT(13)
#define DDC_I2C_NO_ACK BIT(10)
#define DDC_OFFSET_SHIFT (8)
#define DDC_SEGMENT GENMASK(15, 8)
#define DDC_SEGMENT_SHIFT (8)

#define HPD_DDC_CTRL 0xC08
#define HPD_DDC_STATUS 0xC60

#define SEQ_READ_NO_ACK 0x2
#define SEQ_WRITE_REQ_ACK 0x7

#define SI2C_CTRL 0xCAC
#define SI2C_ADDR_READ (0xF4)
#define SI2C_ADDR_SHIFT (16)
#define SI2C_WDATA GENMASK(15, 8)
#define SI2C_WDATA_SHIFT (8)
#define SI2C_CONFIRM_READ BIT(2)
#define SI2C_RD BIT(1)
#define SI2C_WR BIT(0)

#define HDCP2X_POL_CTRL 0xC54
#define HDCP2X_DIS_POLL_EN BIT(16)

struct mtk_hdmi_ddc {
	struct udevice *udev;
	struct clk clk;
	void __iomem *regs;
};

enum sif_bit_t_hdmi {
	SIF_8_BIT_HDMI, /* 8 bits data address */
	SIF_16_BIT_HDMI, /* 16 bits data address */
};

static inline unsigned int mtk_ddc_read(struct mtk_hdmi_ddc *ddc,
					unsigned int reg)
{
	return readl(ddc->regs + reg);
}

static inline void mtk_ddc_write(struct mtk_hdmi_ddc *ddc, unsigned int reg,
				 unsigned int val)
{
	writel(val, ddc->regs + reg);
}

static inline void mtk_ddc_mask(struct mtk_hdmi_ddc *ddc, unsigned int reg,
				unsigned int val, unsigned int mask)
{
	clrsetbits_32(ddc->regs + reg, mask, val);
}

static void mtk_ddc_disable_hdcp_polling(struct mtk_hdmi_ddc *ddc)
{
	mtk_ddc_mask(ddc, HDCP2X_POL_CTRL, HDCP2X_DIS_POLL_EN,
		     HDCP2X_DIS_POLL_EN);
}

static void ddc_wr_one(struct mtk_hdmi_ddc *ddc, unsigned int addr_id,
		       unsigned int offset_id, unsigned char wr_data)
{
	if (mtk_ddc_read(ddc, HDCP2X_DDCM_STATUS) & DDC_I2C_BUS_LOW) {
		mtk_ddc_mask(ddc, DDC_CTRL, (CLOCK_SCL << DDC_CMD_SHIFT),
			     DDC_CMD);
		udelay(300);
	}
	mtk_ddc_mask(ddc, HPD_DDC_CTRL, DDC2_CLOCK << DDC_DELAY_CNT_SHIFT,
		     DDC_DELAY_CNT);
	mtk_ddc_write(ddc, SI2C_CTRL, SI2C_ADDR_READ << SI2C_ADDR_SHIFT);
	mtk_ddc_mask(ddc, SI2C_CTRL, wr_data << SI2C_WDATA_SHIFT, SI2C_WDATA);
	mtk_ddc_mask(ddc, SI2C_CTRL, SI2C_WR, SI2C_WR);

	mtk_ddc_write(ddc, DDC_CTRL,
		      (SEQ_WRITE_REQ_ACK << DDC_CMD_SHIFT) +
		      (1 << DDC_DIN_CNT_SHIFT) +
		      (offset_id << DDC_OFFSET_SHIFT) + (addr_id << 1));

	udelay(1250);

	if ((mtk_ddc_read(ddc, HDCP2X_DDCM_STATUS) &
	     (DDC_I2C_NO_ACK | DDC_I2C_BUS_LOW))) {
		if (mtk_ddc_read(ddc, HDCP2X_DDCM_STATUS) & DDC_I2C_BUS_LOW) {
			mtk_ddc_mask(ddc, DDC_CTRL,
				     (CLOCK_SCL << DDC_CMD_SHIFT), DDC_CMD);
			udelay(300);
		}
	}
}

static unsigned int
ddcm_read_hdmi(struct mtk_hdmi_ddc *ddc, unsigned int u4_clk_div,
	       unsigned char uc_dev, unsigned int u4_addr,
	       unsigned char *puc_value, unsigned int u4_count)
{
	unsigned int i, temp_length, loop_counter;
	unsigned int uc_read_count = 0, uc_idx;
	unsigned long ddc_start_time, ddc_end_time, ddc_timeout;

	if (!puc_value || !u4_count || !u4_clk_div)
		return 0;

	if (mtk_ddc_read(ddc, HDCP2X_DDCM_STATUS) & DDC_I2C_BUS_LOW) {
		mtk_ddc_mask(ddc, DDC_CTRL, (CLOCK_SCL << DDC_CMD_SHIFT),
			     DDC_CMD);
		udelay(300);
	}

	mtk_ddc_mask(ddc, DDC_CTRL, (CLEAR_FIFO << DDC_CMD_SHIFT), DDC_CMD);

	if (u4_count >= 16) {
		temp_length = 16;
		loop_counter = u4_count / 16 + ((u4_count % 16 == 0) ? 0 : 1);
	} else {
		temp_length = u4_count;
		loop_counter = 1;
	}

	if (uc_dev >= EDID_ADDR && u4_clk_div < DDC2_CLOCK_EDID)
		u4_clk_div = DDC2_CLOCK_EDID;

	mtk_ddc_mask(ddc, HPD_DDC_CTRL, u4_clk_div << DDC_DELAY_CNT_SHIFT,
		     DDC_DELAY_CNT);

	for (i = 0; i < loop_counter; i++) {
		if (i == (loop_counter - 1) && i != 0 && u4_count % 16)
			temp_length = u4_count % 16;

		/* EDID_ADDR(0x50) + 1 .. 0x53 select an EDID segment */
		if (uc_dev > EDID_ADDR && uc_dev <= 0x53) {
			mtk_ddc_mask(ddc, SCDC_CTRL,
				     (uc_dev - EDID_ADDR)
				     << DDC_SEGMENT_SHIFT,
				     DDC_SEGMENT);
			mtk_ddc_write(ddc, DDC_CTRL,
				      (ENH_READ_NO_ACK << DDC_CMD_SHIFT) +
				      (temp_length << DDC_DIN_CNT_SHIFT) +
				      ((u4_addr + i * temp_length)
				       << DDC_OFFSET_SHIFT) +
				      (EDID_ADDR << 1));
		} else {
			mtk_ddc_write(ddc, DDC_CTRL,
				      (SEQ_READ_NO_ACK << DDC_CMD_SHIFT) +
				      (temp_length << DDC_DIN_CNT_SHIFT) +
				      ((u4_addr + i * 16)
				       << DDC_OFFSET_SHIFT) + (uc_dev << 1));
		}
		udelay(5500);
		ddc_start_time = get_timer(0);
		/* timeout in ms: about 1 ms per byte plus some margin */
		ddc_timeout = temp_length + 5;
		ddc_end_time = ddc_start_time + ddc_timeout;
		while (1) {
			if ((mtk_ddc_read(ddc, HPD_DDC_STATUS) &
			     DDC_I2C_IN_PROG) == 0)
				break;

			if (time_after(get_timer(0), ddc_end_time)) {
				dev_err(ddc->udev, "DDC transfer timeout\n");
				return 0;
			}
			udelay(1500);
		}
		if ((mtk_ddc_read(ddc, HDCP2X_DDCM_STATUS) &
		     (DDC_I2C_NO_ACK | DDC_I2C_BUS_LOW))) {
			if (mtk_ddc_read(ddc, HDCP2X_DDCM_STATUS) &
			    DDC_I2C_BUS_LOW) {
				mtk_ddc_mask(ddc, DDC_CTRL,
					     (CLOCK_SCL << DDC_CMD_SHIFT),
					     DDC_CMD);
				udelay(300);
			}
			return 0;
		}

		/* get the DDC data from the FIFO */
		for (uc_idx = 0; uc_idx < temp_length; uc_idx++) {
			/* latch the FIFO output */
			mtk_ddc_write(ddc, SI2C_CTRL,
				      (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) +
				      SI2C_RD);

			/* read FIFO output value from DDC_STATUS */
			puc_value[i * 16 + uc_idx] =
				(mtk_ddc_read(ddc, HPD_DDC_STATUS) &
				 DDC_DATA_OUT) >> DDC_DATA_OUT_SHIFT;

			/* increment FIFO read pointer, un-latch the FIFO */
			mtk_ddc_write(ddc, SI2C_CTRL,
				      (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) +
				      SI2C_CONFIRM_READ);
			/*
			 * if the hdmi block was reset while reading, the DDC
			 * speed falls back below DDC2_CLOCK: abort
			 */
			if (((mtk_ddc_read(ddc, HPD_DDC_CTRL) >> 16) &
			     0xFFFF) < DDC2_CLOCK)
				return 0;

			uc_read_count = i * 16 + uc_idx + 1;
		}
	}

	return uc_read_count;
}

static unsigned int vddc_read(struct mtk_hdmi_ddc *ddc,
			      unsigned int u4_clk_div, unsigned char uc_dev,
			      unsigned int u4_addr,
			      enum sif_bit_t_hdmi uc_addr_type,
			      unsigned char *puc_value, unsigned int u4_count)
{
	unsigned int u4_read_count = 0;

	if (!puc_value || !u4_count || !u4_clk_div)
		return 0;
	if (uc_addr_type > SIF_16_BIT_HDMI)
		return 0;
	if (uc_addr_type == SIF_8_BIT_HDMI && u4_addr > 255)
		return 0;
	if (uc_addr_type == SIF_16_BIT_HDMI && u4_addr > 65535)
		return 0;

	if (uc_addr_type == SIF_8_BIT_HDMI)
		u4_read_count = 255 - u4_addr + 1;
	else if (uc_addr_type == SIF_16_BIT_HDMI)
		u4_read_count = 65535 - u4_addr + 1;

	u4_read_count = min(u4_read_count, u4_count);

	return ddcm_read_hdmi(ddc, u4_clk_div, uc_dev, u4_addr, puc_value,
			      u4_read_count);
}

static int fg_ddc_data_read(struct mtk_hdmi_ddc *ddc, unsigned char b_dev,
			    unsigned char b_data_addr,
			    unsigned int b_data_count, unsigned char *pr_data)
{
	mtk_ddc_disable_hdcp_polling(ddc);
	if (vddc_read(ddc, DDC2_CLOCK, b_dev, b_data_addr, SIF_8_BIT_HDMI,
		      pr_data, b_data_count) != b_data_count)
		return -EREMOTEIO;

	return 0;
}

static int fg_ddc_data_write(struct mtk_hdmi_ddc *ddc, unsigned char b_dev,
			     unsigned char b_data_addr,
			     unsigned int b_data_count,
			     unsigned char *pr_data)
{
	unsigned int i;

	mtk_ddc_disable_hdcp_polling(ddc);
	for (i = 0; i < b_data_count; i++)
		ddc_wr_one(ddc, b_dev, b_data_addr + i, *(pr_data + i));

	return 0;
}

static int mtk_hdmi_ddc_xfer(struct udevice *dev, struct i2c_msg *msgs, int num)
{
	struct mtk_hdmi_ddc *ddc = dev_get_priv(dev);
	unsigned char offset = 0;
	int ret;
	int i;

	if (!msgs)
		return -EINVAL;

	if (!ddc || !ddc->regs)
		return -EINVAL;

	for (i = 0; i < num; i++) {
		struct i2c_msg *msg = &msgs[i];

		if (!msg->buf || !msg->len)
			return -EINVAL;

		if (msg->flags & I2C_M_RD) {
			/*
			 * The underlying DDC hardware always issues a write
			 * request that assigns the read offset as part of the
			 * read operation, so use the offset value stored on
			 * the previous write request.
			 */
			ret = fg_ddc_data_read(ddc, msg->addr, offset,
					       msg->len, &msg->buf[0]);
		} else {
			ret = fg_ddc_data_write(ddc, msg->addr, msg->buf[0],
						msg->len - 1, &msg->buf[1]);

			/*
			 * store the offset requested by the EDID/SCDC
			 * framework for use by subsequent read requests
			 */
			if ((msg->addr == EDID_ADDR ||
			     msg->addr == SCDC_I2C_SLAVE_ADDRESS) &&
			    msg->len == 1)
				offset = msg->buf[0];
		}

		if (ret) {
			dev_err(dev, "ddc transfer failed: %d\n", ret);
			return ret;
		}
	}

	return 0;
}

static int mtk_hdmi_ddc_probe(struct udevice *dev)
{
	struct mtk_hdmi_ddc *ddc = dev_get_priv(dev);
	int ret;

	ddc->udev = dev;
	/* the ddc node sits below the hdmi node, which holds the registers */
	ddc->regs = dev_read_addr_ptr(dev->parent);
	if (!ddc->regs)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &ddc->clk);
	if (ret) {
		dev_err(dev, "failed to get ddc clk: %d\n", ret);
		return ret;
	}

	ret = clk_enable(&ddc->clk);
	if (ret) {
		dev_err(dev, "failed to enable ddc clk: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct dm_i2c_ops mtk_hdmi_ddc_ops = {
	.xfer	= mtk_hdmi_ddc_xfer,
};

static const struct udevice_id mtk_hdmi_ddc_ids[] = {
	{ .compatible = "mediatek,mt8195-hdmi-ddc", },
	{ }
};

U_BOOT_DRIVER(mtk_i2c_ddc) = {
	.name		= "mtk_i2c_ddc",
	.id		= UCLASS_I2C,
	.of_match	= mtk_hdmi_ddc_ids,
	.probe		= mtk_hdmi_ddc_probe,
	.priv_auto	= sizeof(struct mtk_hdmi_ddc),
	.ops		= &mtk_hdmi_ddc_ops,
};
