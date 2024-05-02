// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 */

#include <clk.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <spi-mem.h>
#include <spi.h>
#include <spi_flash.h>
#include <wait_bit.h>

#define CV1800B_SPI_CTRL_SCK_DIV_MASK                GENMASK(10, 0)
#define CV1800B_SPI_CTRL_CPHA                        BIT(12)
#define CV1800B_SPI_CTRL_CPOL                        BIT(13)

#define CV1800B_SPI_CE_MANUAL                        BIT(0)
#define CV1800B_SPI_CE_MANUAL_EN                     BIT(1)
#define CV1800B_SPI_CE_ENABLE                        (CV1800B_SPI_CE_MANUAL | \
						      CV1800B_SPI_CE_MANUAL_EN)
#define CV1800B_SPI_CE_DISABLE                       CV1800B_SPI_CE_MANUAL_EN
#define CV1800B_SPI_CE_HARDWARE                      0

#define CV1800B_SPI_DLY_CTRL_NEG_SAMPLE              BIT(14)

#define CV1800B_SPI_TRAN_MODE_RX                     BIT(0)
#define CV1800B_SPI_TRAN_MODE_TX                     BIT(1)
#define CV1800B_SPI_TRAN_FAST_MODE                   BIT(3)
#define CV1800B_SPI_TRAN_BUS_WIDTH_1_BIT             0x0
#define CV1800B_SPI_TRAN_BUS_WIDTH_2_BIT             BIT(4)
#define CV1800B_SPI_TRAN_BUS_WIDTH_4_BIT             BIT(5)
#define CV1800B_SPI_TRAN_ADDR_3_BYTES                (3 << 8)
#define CV1800B_SPI_TRAN_ADDR_4_BYTES                (4 << 8)
#define CV1800B_SPI_TRAN_WITH_CMD                    BIT(11)
#define CV1800B_SPI_TRAN_GO_BUSY                     BIT(15)
#define CV1800B_SPI_TRAN_DUMMY_CYC_MASK              GENMASK(19, 16)
#define CV1800B_SPI_TRAN_DUMMY_CYC_OFFSET            16
#define CV1800B_SPI_TRAN_BYTE4_EN                    BIT(20)
#define CV1800B_SPI_TRAN_BYTE4_CMD                   BIT(21)

#define CV1800B_SPI_FF_PT_AVAILABLE_MASK             GENMASK(3, 0)

#define CV1800B_SPI_INT_TRAN_DONE                    BIT(0)
#define CV1800B_SPI_INT_RD_FIFO                      BIT(2)
#define CV1800B_SPI_INT_WR_FIFO                      BIT(3)

#define CV1800B_FIFO_CAPACITY           8
#define CV1800B_DEFAULT_DIV             4

struct cv1800b_spif_regs {
	u32 spi_ctrl;
	u32 ce_ctrl;
	u32 dly_ctrl;
	u32 dmmr_ctrl;
	u32 tran_csr;
	u32 tran_num;
	u32 ff_port;
	u32 reserved0;
	u32 ff_pt;
	u32 reserved1;
	u32 int_sts;
	u32 int_en;
};

struct cv1800b_spi_priv {
	struct cv1800b_spif_regs *regs;
	uint clk_freq;
	uint mode;
	int div;
};

static int cv1800b_spi_probe(struct udevice *bus)
{
	struct cv1800b_spi_priv *priv = dev_get_priv(bus);
	struct clk clkdev;
	int ret;

	priv->regs = (struct cv1800b_spif_regs *)dev_read_addr_ptr(bus);
	if (priv->regs == 0)
		return -EINVAL;

	ret = clk_get_by_index(bus, 0, &clkdev);
	if (ret)
		return ret;
	priv->clk_freq = clk_get_rate(&clkdev);

	/* DMMR mode is enabled by default, disable it */
	writel(0, &priv->regs->dmmr_ctrl);

	return 0;
}

static void cv1800b_spi_config_dmmr(struct cv1800b_spi_priv *priv, struct spi_nor *flash)
{
	struct cv1800b_spif_regs *regs = priv->regs;
	u32 read_cmd = flash->read_opcode;
	u32 val;

	val = CV1800B_SPI_TRAN_MODE_RX | CV1800B_SPI_TRAN_WITH_CMD;

	switch (read_cmd) {
	case SPINOR_OP_READ_4B:
	case SPINOR_OP_READ_FAST_4B:
	case SPINOR_OP_READ_1_1_2_4B:
	case SPINOR_OP_READ_1_1_4_4B:
		val |= CV1800B_SPI_TRAN_ADDR_4_BYTES |
		       CV1800B_SPI_TRAN_BYTE4_EN | CV1800B_SPI_TRAN_BYTE4_CMD;
		break;
	case SPINOR_OP_READ:
	case SPINOR_OP_READ_FAST:
	case SPINOR_OP_READ_1_1_2:
	case SPINOR_OP_READ_1_1_4:
		val |= CV1800B_SPI_TRAN_ADDR_3_BYTES;
		break;
	}

	switch (read_cmd) {
	case SPINOR_OP_READ_FAST:
	case SPINOR_OP_READ_FAST_4B:
		val |= CV1800B_SPI_TRAN_FAST_MODE;
		break;
	}

	switch (read_cmd) {
	case SPINOR_OP_READ_1_1_2:
	case SPINOR_OP_READ_1_1_2_4B:
		val |= CV1800B_SPI_TRAN_BUS_WIDTH_2_BIT;
		break;
	case SPINOR_OP_READ_1_1_4:
	case SPINOR_OP_READ_1_1_4_4B:
		val |= CV1800B_SPI_TRAN_BUS_WIDTH_4_BIT;
		break;
	}

	val |= (flash->read_dummy & CV1800B_SPI_TRAN_DUMMY_CYC_MASK)
	       << CV1800B_SPI_TRAN_DUMMY_CYC_OFFSET;
	writel(val, &regs->tran_csr);
}

static void cv1800b_set_clk_div(struct cv1800b_spi_priv *priv, u32 div)
{
	struct cv1800b_spif_regs *regs = priv->regs;
	u32 neg_sample = 0;

	clrsetbits_le32(&regs->spi_ctrl, CV1800B_SPI_CTRL_SCK_DIV_MASK, div);

	if (div < CV1800B_DEFAULT_DIV)
		neg_sample = CV1800B_SPI_DLY_CTRL_NEG_SAMPLE;
	clrsetbits_le32(&regs->dly_ctrl, CV1800B_SPI_DLY_CTRL_NEG_SAMPLE, neg_sample);
}

static int cv1800b_spi_transfer(struct cv1800b_spi_priv *priv,
				u8 *din, const u8 *dout, uint len, ulong flags)
{
	struct cv1800b_spif_regs *regs = priv->regs;
	u32 tran_csr;
	u32 xfer_size, off;
	u32 fifo_cnt;
	u32 interrupt_mask;

	if (din) {
		/* Slow down on receiving */
		cv1800b_set_clk_div(priv, CV1800B_DEFAULT_DIV);
		interrupt_mask = CV1800B_SPI_INT_RD_FIFO;
	} else {
		interrupt_mask = CV1800B_SPI_INT_WR_FIFO;
	}

	writel(0, &regs->ff_pt);
	writel(len, &regs->tran_num);

	tran_csr = CV1800B_SPI_TRAN_GO_BUSY;
	if (din) {
		tran_csr |= CV1800B_SPI_TRAN_MODE_RX;
	} else {
		tran_csr |= CV1800B_SPI_TRAN_MODE_TX;
		if (!(flags & SPI_XFER_BEGIN) && (priv->mode & SPI_TX_QUAD))
			tran_csr |= CV1800B_SPI_TRAN_BUS_WIDTH_4_BIT;
	}
	writel(tran_csr, &regs->tran_csr);

	wait_for_bit_le32(&regs->int_sts, interrupt_mask, true, 3000, false);

	off = 0;
	while (off < len) {
		xfer_size = min_t(u32, len - off, CV1800B_FIFO_CAPACITY);

		fifo_cnt = readl(&regs->ff_pt) & CV1800B_SPI_FF_PT_AVAILABLE_MASK;
		if (din)
			xfer_size = min(xfer_size, fifo_cnt);
		else
			xfer_size = min(xfer_size, CV1800B_FIFO_CAPACITY - fifo_cnt);

		while (xfer_size--) {
			if (din)
				din[off++] = readb(&regs->ff_port);
			else
				writeb(dout[off++], &regs->ff_port);
		}
	}

	wait_for_bit_le32(&regs->int_sts, CV1800B_SPI_INT_TRAN_DONE, true, 3000, false);
	writel(0, &regs->ff_pt);
	clrbits_le32(&regs->int_sts, CV1800B_SPI_INT_TRAN_DONE | interrupt_mask);

	if (din)
		cv1800b_set_clk_div(priv, priv->div);
	return 0;
}

static int cv1800b_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct cv1800b_spi_priv *priv = dev_get_priv(bus);
	struct cv1800b_spif_regs *regs = priv->regs;

	if (bitlen == 0)
		goto out;

	if (bitlen % 8) {
		flags |= SPI_XFER_END;
		goto out;
	}

	if (flags & SPI_XFER_BEGIN)
		writel(CV1800B_SPI_CE_DISABLE, &regs->ce_ctrl);

	if (din || dout)
		cv1800b_spi_transfer(priv, din, dout, bitlen / 8, flags);

out:
	if (flags & SPI_XFER_END)
		writel(CV1800B_SPI_CE_ENABLE, &regs->ce_ctrl);
	return 0;
}

static int cv1800b_spi_set_speed(struct udevice *bus, uint speed)
{
	struct cv1800b_spi_priv *priv = dev_get_priv(bus);

	priv->div = DIV_ROUND_CLOSEST(priv->clk_freq, speed * 2) - 1;
	if (priv->div <= 0)
		priv->div = CV1800B_DEFAULT_DIV;

	cv1800b_set_clk_div(priv, priv->div);

	return 0;
}

static int cv1800b_spi_set_mode(struct udevice *bus, uint mode)
{
	struct cv1800b_spi_priv *priv = dev_get_priv(bus);
	struct cv1800b_spif_regs *regs = priv->regs;
	u32 val = 0;

	if (mode & SPI_CPHA)
		val |= CV1800B_SPI_CTRL_CPHA;
	if (mode & SPI_CPOL)
		val |= CV1800B_SPI_CTRL_CPOL;
	clrsetbits_le32(&regs->spi_ctrl, CV1800B_SPI_CTRL_CPHA | CV1800B_SPI_CTRL_CPOL, val);

	priv->mode = mode;

	return 0;
}

static int cv1800b_spi_exec_op(struct spi_slave *slave, const struct spi_mem_op *op)
{
	struct udevice *bus = slave->dev->parent;
	struct cv1800b_spi_priv *priv = dev_get_priv(bus);
	struct cv1800b_spif_regs *regs = priv->regs;
	struct spi_nor *flash = dev_get_uclass_priv(slave->dev);
	u32 old_tran_csr;

	if (!(op->data.nbytes > 0 && op->data.dir == SPI_MEM_DATA_IN) ||
	    !(op->addr.nbytes > 0 && op->addr.nbytes <= 4))
		return -ENOTSUPP;

	old_tran_csr = readl(&regs->tran_csr);
	writel(CV1800B_SPI_CE_HARDWARE, &regs->ce_ctrl);

	cv1800b_spi_config_dmmr(priv, flash);

	writel(1, &regs->dmmr_ctrl);
	memcpy(op->data.buf.in, (void *)priv->regs + op->addr.val, op->data.nbytes);
	writel(0, &regs->dmmr_ctrl);

	writel(CV1800B_SPI_CE_ENABLE, &regs->ce_ctrl);
	writel(old_tran_csr, &regs->tran_csr);

	return 0;
}

static const struct spi_controller_mem_ops cv1800b_spi_mem_ops = {
	.exec_op = cv1800b_spi_exec_op,
};

static const struct dm_spi_ops cv1800b_spi_ops = {
	.xfer      = cv1800b_spi_xfer,
	.mem_ops   = &cv1800b_spi_mem_ops,
	.set_speed = cv1800b_spi_set_speed,
	.set_mode  = cv1800b_spi_set_mode,
};

static const struct udevice_id cv1800b_spi_ids[] = {
	{ .compatible = "sophgo,cv1800b-spif" },
	{ }
};

U_BOOT_DRIVER(cv1800b_spi) = {
	.name      = "cv1800b_spif",
	.id        = UCLASS_SPI,
	.of_match  = cv1800b_spi_ids,
	.ops       = &cv1800b_spi_ops,
	.priv_auto = sizeof(struct cv1800b_spi_priv),
	.probe     = cv1800b_spi_probe,
};
