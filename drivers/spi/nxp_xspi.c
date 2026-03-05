// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 NXP
 */

#include <asm/arch/clock.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <spi-mem.h>

#include "nxp_xspi.h"

static inline void xspi_writel(struct nxp_xspi *x, u32 val, u32 addr)
{
	void __iomem *_addr = (void __iomem *)(uintptr_t)addr;

	out_le32(_addr, val);
};

static inline u32 xspi_readl(struct nxp_xspi *x, u32 addr)
{
	return in_le32((uintptr_t)addr);
};

#define xspi_config_sfp_tg(x, env, sfar, ipcr) \
	do { \
		xspi_writel_offset(x, env, (sfar), SFP_TG_SFAR); \
		xspi_writel_offset(x, env, (ipcr), SFP_TG_IPCR); \
	} while (0)

static int xspi_readl_poll_tout(struct nxp_xspi *x, int env, u32 offset,
				u32 mask, u32 delay_us,
				u32 timeout_us, bool c)
{
	u32 reg;
	void __iomem *addr = (void __iomem *)(uintptr_t)x->iobase + (env * ENV_ADDR_SIZE) + offset;

	if (c)
		return readl_poll_sleep_timeout(addr, reg, (reg & mask),
						delay_us, timeout_us);
	else
		return readl_poll_sleep_timeout(addr, reg, !(reg & mask),
						delay_us, timeout_us);
};

static struct nxp_xspi_devtype_data imx94_data = {
	.rxfifo = SZ_512,	/* RX fifo Size*/
	.rx_buf_size = 64 * 4, /* RBDR buffer size */
	.txfifo = SZ_1K,
	.ahb_buf_size = SZ_4K,
	.quirks = 0,
};

static const struct udevice_id nxp_xspi_ids[] = {
	{ .compatible = "nxp,imx94-xspi", .data = (ulong)&imx94_data, },
	{ }
};

static int nxp_xspi_claim_bus(struct udevice *dev)
{
	return 0;
}

#if CONFIG_IS_ENABLED(CLK)
static int nxp_xspi_clk_prep_enable(struct nxp_xspi *x)
{
	int ret;

	ret = clk_enable(&x->clk);
	if (ret)
		return ret;

	return 0;
};

static void nxp_xspi_clk_disable_unprep(struct nxp_xspi *x)
{
	clk_disable(&x->clk);
};
#endif

static int xspi_swreset(struct nxp_xspi *x)
{
	u32 reg;

	reg = xspi_readl_offset(x, 0, MCR);
	reg |= (XSPI_MCR_SWRSTHD_MASK | XSPI_MCR_SWRSTSD_MASK);
	xspi_writel_offset(x, 0, reg, MCR);
	udelay(2);
	reg &= ~(XSPI_MCR_SWRSTHD_MASK | XSPI_MCR_SWRSTSD_MASK);
	xspi_writel_offset(x, 0, reg, MCR);

	return 0;
};

static void nxp_xspi_dll_bypass(struct nxp_xspi *x)
{
	u32 reg;
	int ret;

	xspi_swreset(x);

	xspi_writel_offset(x, 0, 0, DLLCRA);

	reg = XSPI_DLLCRA_SLV_EN_MASK;
	xspi_writel_offset(x, 0, reg, DLLCRA);

	reg = XSPI_DLLCRA_FREQEN_MASK | XSPI_DLLCRA_SLV_EN_MASK |
		XSPI_DLLCRA_SLV_DLL_BYPASS_MASK | XSPI_DLLCRA_SLV_DLY_COARSE(7);
	xspi_writel_offset(x, 0, reg, DLLCRA);

	reg |= XSPI_DLLCRA_SLV_UPD_MASK;
	xspi_writel_offset(x, 0, reg, DLLCRA);

	ret = xspi_readl_poll_tout(x, 0, XSPI_DLLSR, XSPI_DLLSR_SLVA_LOCK_MASK, 1, POLL_TOUT, true);
	WARN_ON(ret);

	reg &= ~XSPI_DLLCRA_SLV_UPD_MASK;
	xspi_writel_offset(x, 0, reg, DLLCRA);
}

static void nxp_xspi_dll_auto(struct nxp_xspi *x, unsigned long rate)
{
	u32 reg;
	int ret;

	xspi_swreset(x);

	xspi_writel_offset(x, 0, 0, DLLCRA);

	reg = XSPI_DLLCRA_SLV_EN_MASK;
	xspi_writel_offset(x, 0, reg, DLLCRA);

	reg = XSPI_DLLCRA_DLL_REFCNTR(2) | XSPI_DLLCRA_DLLRES(8) |
		XSPI_DLLCRA_SLAVE_AUTO_UPDT_MASK | XSPI_DLLCRA_SLV_EN_MASK;
	if (rate > MHZ(133))
		reg |= XSPI_DLLCRA_FREQEN_MASK;

	xspi_writel_offset(x, 0, reg, DLLCRA);

	reg |= XSPI_DLLCRA_SLV_UPD_MASK;
	xspi_writel_offset(x, 0, reg, DLLCRA);

	reg |= XSPI_DLLCRA_DLLEN_MASK;
	xspi_writel_offset(x, 0, reg, DLLCRA);

	ret = xspi_readl_poll_tout(x, 0, XSPI_DLLSR,
				   XSPI_DLLSR_DLLA_LOCK_MASK | XSPI_DLLSR_SLVA_LOCK_MASK,
				   1, POLL_TOUT, true);
	WARN_ON(ret);
}

static void nxp_xspi_disable_ddr(struct nxp_xspi *x)
{
	u32 reg;

	reg = xspi_readl_offset(x, 0, MCR);
	reg |= XSPI_MCR_MDIS_MASK;
	xspi_writel_offset(x, 0, reg, MCR);

	reg &= ~(XSPI_MCR_DQS_EN_MASK | XSPI_MCR_DDR_EN_MASK);
	reg &= ~XSPI_MCR_DQS_FA_SEL_MASK;
	reg |= XSPI_MCR_DQS_FA_SEL(1);
	xspi_writel_offset(x, 0, reg, MCR);

	reg = xspi_readl_offset(x, 0, FLSHCR);
	reg &= ~XSPI_FLSHCR_TDH_MASK;
	xspi_writel_offset(x, 0, reg, FLSHCR);

	xspi_writel_offset(x, 0, XSPI_SMPR_DLLFSMPFA(7), SMPR);

	reg = xspi_readl_offset(x, 0, MCR);
	reg &= ~XSPI_MCR_MDIS_MASK;
	xspi_writel_offset(x, 0, reg, MCR);

	x->support_max_rate = MHZ(133);
}

static void nxp_xspi_enable_ddr(struct nxp_xspi *x)
{
	u32 reg;

	reg = xspi_readl_offset(x, 0, MCR);
	reg |= XSPI_MCR_MDIS_MASK;
	xspi_writel_offset(x, 0, reg, MCR);

	reg |= XSPI_MCR_DQS_EN_MASK | XSPI_MCR_DDR_EN_MASK;
	reg &= ~XSPI_MCR_DQS_FA_SEL_MASK;
	reg |= XSPI_MCR_DQS_FA_SEL(3);
	xspi_writel_offset(x, 0, reg, MCR);

	reg = xspi_readl_offset(x, 0, FLSHCR);
	reg |= XSPI_FLSHCR_TDH(1);
	xspi_writel_offset(x, 0, reg, FLSHCR);

	xspi_writel_offset(x, 0, XSPI_SMPR_DLLFSMPFA(4), SMPR);

	reg = xspi_readl_offset(x, 0, MCR);
	reg &= ~XSPI_MCR_MDIS_MASK;
	xspi_writel_offset(x, 0, reg, MCR);

	x->support_max_rate = MHZ(200);
}

static int nxp_xspi_set_speed(struct udevice *bus, uint speed)
{
	printf("%s: %u\n", __func__, speed);
#if CONFIG_IS_ENABLED(CLK)
	struct nxp_xspi *x = dev_get_priv(bus);
	int ret;

	nxp_xspi_clk_disable_unprep(x);

	ret = clk_set_rate(&x->clk, speed);
	if (ret < 0)
		return ret;

	ret = nxp_xspi_clk_prep_enable(x);
	if (ret)
		return ret;

	xspi_swreset(x);
#endif
	return 0;
}

static int nxp_xspi_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

static int nxp_xspi_adjust_op_size(struct spi_slave *slave,
				   struct spi_mem_op *op)
{
	struct nxp_xspi *x;
	struct udevice *bus;

	bus = slave->dev->parent;
	x = dev_get_priv(bus);

	if (op->data.dir == SPI_MEM_DATA_OUT) {
		if (op->data.nbytes > x->devtype_data->txfifo)
			op->data.nbytes = x->devtype_data->txfifo;
	} else {
		if (op->data.nbytes > x->devtype_data->ahb_buf_size)
			op->data.nbytes = x->devtype_data->ahb_buf_size;
		else if (op->data.nbytes > x->devtype_data->rxfifo)
			op->data.nbytes = ALIGN_DOWN(op->data.nbytes, 8);
	}

	return 0;
}

static int nxp_xspi_check_buswidth(struct nxp_xspi *x, u8 width)
{
	switch (width) {
	case 1:
	case 2:
	case 4:
	case 8:
		return 0;
	}

	return -ENOTSUPP;
}

static bool nxp_xspi_supports_op(struct spi_slave *slave,
				 const struct spi_mem_op *op)
{
	struct nxp_xspi *x;
	struct udevice *bus;
	int ret;

	bus = slave->dev->parent;
	x = dev_get_priv(bus);

	ret = nxp_xspi_check_buswidth(x, op->cmd.buswidth);

	if (op->addr.nbytes)
		ret |= nxp_xspi_check_buswidth(x, op->addr.buswidth);

	if (op->dummy.nbytes)
		ret |= nxp_xspi_check_buswidth(x, op->dummy.buswidth);

	if (op->data.nbytes)
		ret |= nxp_xspi_check_buswidth(x, op->data.buswidth);

	if (ret)
		return false;

	/*
	 * The number of address bytes should be equal to or less than 4 bytes.
	 */
	if (op->addr.nbytes > 4)
		return false;

	/*
	 * If requested address value is greater than controller assigned
	 * memory mapped space, return error as it didn't fit in the range
	 * of assigned address space.
	 */
	if (op->addr.val >= x->a1_size + x->a2_size)
		return false;

	/* Max 64 dummy clock cycles supported */
	if (op->dummy.buswidth &&
	    (op->dummy.nbytes * 8 / op->dummy.buswidth > 64))
		return false;

	/* Max data length, check controller limits and alignment */
	if (op->data.dir == SPI_MEM_DATA_IN &&
	    (op->data.nbytes > x->devtype_data->ahb_buf_size ||
	     (op->data.nbytes > x->devtype_data->rxfifo &&
	      !IS_ALIGNED(op->data.nbytes, 8))))
		return false;

	if (op->data.dir == SPI_MEM_DATA_OUT &&
	    op->data.nbytes > x->devtype_data->txfifo)
		return false;

	if (op->cmd.dtr)
		return spi_mem_dtr_supports_op(slave, op);
	else
		return spi_mem_default_supports_op(slave, op);
}

static int xspi_update_lut(struct nxp_xspi *x, u32 seq_index, const u32 *lut_base, u32 num_of_seq)
{
	int ret;

	ret = xspi_readl_poll_tout(x, 0, XSPI_SR, XSPI_SR_BUSY_MASK, 1, POLL_TOUT, false);
	WARN_ON(ret);

	xspi_writel_offset(x, 0, XSPI_LUT_KEY_VAL, LUTKEY);
	xspi_writel_offset(x, 0, 0x2, LCKCR);

	for (int i = 0; i < num_of_seq * 5; i++)
		xspi_writel(x, *(lut_base + i), x->iobase + XSPI_LUT + (seq_index * 5 + i) * 4);

	xspi_writel_offset(x, 0, XSPI_LUT_KEY_VAL, LUTKEY);
	xspi_writel_offset(x, 0, 0x1, LCKCR);

	return 0;
}

static void nxp_xspi_prepare_lut(struct nxp_xspi *x,
				 const struct spi_mem_op *op)
{
	u32 lutval[5] = {0};
	int lutidx = 1;

	/* cmd */
	if (op->cmd.dtr) {
		lutval[0] |= LUT_DEF(0, CMD_DDR, LUT_PAD(op->cmd.buswidth),
				     op->cmd.opcode >> 8);
		lutval[lutidx / 2] |= LUT_DEF(lutidx, CMD_DDR,
					      LUT_PAD(op->cmd.buswidth),
					      op->cmd.opcode & 0x00ff);
		lutidx++;
	} else {
		lutval[0] |= LUT_DEF(0, CMD_SDR, LUT_PAD(op->cmd.buswidth),
				     op->cmd.opcode);
	}

	/* addr bytes */
	if (op->addr.nbytes) {
		lutval[lutidx / 2] |= LUT_DEF(lutidx, op->addr.dtr ?  RADDR_DDR : RADDR_SDR,
					      LUT_PAD(op->addr.buswidth),
					      op->addr.nbytes * 8);
		lutidx++;
	}

	/* dummy bytes, if needed */
	if (op->dummy.nbytes) {
		lutval[lutidx / 2] |= LUT_DEF(lutidx, DUMMY_CYCLE,
					      LUT_PAD(op->data.buswidth),
					      op->dummy.nbytes * 8 /
					      op->dummy.buswidth / (op->dummy.dtr ? 2 : 1));
		lutidx++;
	}

	/* read/write data bytes */
	if (op->data.nbytes) {
		lutval[lutidx / 2] |= LUT_DEF(lutidx,
					      op->data.dir == SPI_MEM_DATA_IN ?
					      (op->data.dtr ? READ_DDR : READ_SDR) :
					      (op->data.dtr ? WRITE_DDR : WRITE_SDR),
					      LUT_PAD(op->data.buswidth),
					      0);
		lutidx++;
	}

	/* stop condition. */
	lutval[lutidx / 2] |= LUT_DEF(lutidx, CMD_STOP, 0, 0);
#ifdef DEBUG
	print_buffer(0, lutval, 4, lutidx / 2 + 1, 4);
#endif
	xspi_update_lut(x, CMD_LUT_FOR_IP_CMD, lutval, 1);

	if (op->data.nbytes &&
	    (op->data.dir == SPI_MEM_DATA_IN || op->data.dir == SPI_MEM_DATA_OUT) &&
	    op->addr.nbytes)
		xspi_update_lut(x, CMD_LUT_FOR_AHB_CMD, lutval, 1);
}

static void nxp_xspi_read_ahb(struct nxp_xspi *x, const struct spi_mem_op *op)
{
	u32 len = op->data.nbytes;

	/* Read out the data directly from the AHB buffer. */
	memcpy_fromio(op->data.buf.in, (void *)(uintptr_t)(x->ahb_addr + op->addr.val), len);
}

static void nxp_xspi_fill_txfifo(struct nxp_xspi *x,
				 const struct spi_mem_op *op)
{
	const u8 *buf = (u8 *)op->data.buf.out;
	int xfer_remaining_size = op->data.nbytes;
	u32 reg, val = 0;
	int ret;

	/* clear the TX FIFO. */
	xspi_set_reg_field(x, x->config.env, 1, MCR, CLR_TXF);
	ret = xspi_readl_poll_tout(x, x->config.env, XSPI_MCR,
				   XSPI_MCR_CLR_TXF_MASK, 1, POLL_TOUT, false);
	WARN_ON(ret);

	reg = XSPI_TBCT_WMRK((x->devtype_data->txfifo - ALIGN_DOWN(op->data.nbytes, 4)) / 4 + 1);
	xspi_writel_offset(x, x->config.env, reg, TBCT);

	reg = x->ahb_addr + op->addr.val;
	xspi_writel_offset(x, x->config.env, reg, SFP_TG_SFAR);

	udelay(2);
	reg = XSPI_SFP_TG_IPCR_SEQID(CMD_LUT_FOR_IP_CMD) | XSPI_SFP_TG_IPCR_IDATSZ(op->data.nbytes);
	u64 start = timer_get_us();

	xspi_writel_offset(x, x->config.env, reg, SFP_TG_IPCR);

	while (xfer_remaining_size > 0) {
		if (xspi_get_reg_field(x, x->config.env, SR, TXFULL))
			continue;

		if (xfer_remaining_size > 4) {
			memcpy(&val, buf, 4);
			buf += 4;
		} else {
			val = 0;
			memcpy(&val, buf, xfer_remaining_size);
			buf += xfer_remaining_size;
		}

		xspi_writel_offset(x, x->config.env, val, TBDR);
		xfer_remaining_size -= 4;

		if (xspi_get_reg_field(x, x->config.env, FR, ILLINE))
			break;
	}

	/* Wait for controller being ready. */
	ret = xspi_readl_poll_tout(x, x->config.env, XSPI_SR,
				   XSPI_SR_BUSY_MASK, 1, POLL_TOUT, false);
	WARN_ON(ret);

	u32 trctr = xspi_get_reg_field(x, x->config.env, TBSR, TRCTR);

	if ((ALIGN(op->data.nbytes, 4) / 4) != trctr)
		dev_dbg(x->dev, "Fail to write data. tx_size = %u, trctr = %u.\n",
			op->data.nbytes, trctr * 4);

	dev_dbg(x->dev, "tx data size: %u bytes, spend: %llu us\r\n",
		op->data.nbytes, timer_get_us() - start);
}

static void nxp_xspi_read_rxfifo(struct nxp_xspi *x,
				 const struct spi_mem_op *op)
{
	u32 reg;
	int ret, i;
	u32 val;

	u8 *buf = op->data.buf.in;

	reg = XSPI_RBCT_WMRK(x->devtype_data->rx_buf_size / 4 - 1);
	xspi_writel_offset(x, x->config.env, reg, RBCT);

	/* clear the TX FIFO. */
	xspi_set_reg_field(x, x->config.env, 1, MCR, CLR_RXF);
	ret = xspi_readl_poll_tout(x, x->config.env, XSPI_MCR,
				   XSPI_MCR_CLR_RXF_MASK, 1, POLL_TOUT, false);
	WARN_ON(ret);

	xspi_writel_offset(x, x->config.env,  x->ahb_addr + op->addr.val, SFP_TG_SFAR);
	reg = XSPI_SFP_TG_IPCR_SEQID(CMD_LUT_FOR_IP_CMD) | XSPI_SFP_TG_IPCR_IDATSZ(op->data.nbytes);
	u64 start = timer_get_us();

	xspi_writel_offset(x, x->config.env, reg, SFP_TG_IPCR);

	ret = xspi_readl_poll_tout(x, x->config.env, XSPI_SR, XSPI_SR_BUSY_MASK, 1,
				   POLL_TOUT, false);
	WARN_ON(ret);

	for (i = 0; i < ALIGN_DOWN(op->data.nbytes, 4); i += 4) {
		if (i == x->devtype_data->rx_buf_size) {
			reg = xspi_readl_offset(x, x->config.env, FR);
			reg |= XSPI_FR_RBDF_MASK;
			xspi_writel_offset(x, x->config.env, reg, FR);
		}
		val = xspi_readl(x, x->iobase + (x->config.env * ENV_ADDR_SIZE) +
				 XSPI_RBDR + (i % x->devtype_data->rx_buf_size));
		memcpy(buf + i, &val, 4);
	}

	if (i < op->data.nbytes) {
		val = xspi_readl(x, x->iobase + (x->config.env * ENV_ADDR_SIZE) +
				 XSPI_RBDR + (i % x->devtype_data->rx_buf_size));
		memcpy(buf + i, &val, op->data.nbytes - i);
	}

	/* clear the RX FIFO. */
	xspi_set_reg_field(x, x->config.env, 1, MCR, CLR_RXF);
	ret = xspi_readl_poll_tout(x, x->config.env, XSPI_MCR,
				   XSPI_MCR_CLR_RXF_MASK, 1, POLL_TOUT, false);
	WARN_ON(ret);

	dev_dbg(x->dev, "rx data size: %u bytes, spend: %llu us\r\n",
		op->data.nbytes, timer_get_us() - start);
}

static int nxp_xspi_xfer_cmd(struct nxp_xspi *x, const struct spi_mem_op *op)
{
	u32 reg;
	int ret;

	xspi_writel_offset(x, x->config.env, x->ahb_addr + op->addr.val, SFP_TG_SFAR);
	reg = XSPI_SFP_TG_IPCR_SEQID(CMD_LUT_FOR_IP_CMD) | XSPI_SFP_TG_IPCR_IDATSZ(op->data.nbytes);
	xspi_writel_offset(x, x->config.env, reg, SFP_TG_IPCR);

	/* Wait for controller being ready. */
	ret = xspi_readl_poll_tout(x, x->config.env, XSPI_SR, XSPI_SR_BUSY_MASK, 1,
				   POLL_TOUT, false);
	WARN_ON(ret);

	return 0;
}

static void nxp_xspi_select_mem(struct nxp_xspi *xspi, struct spi_slave *slave,
				const struct spi_mem_op *op)
{
	unsigned long rate = slave->max_hz;

	if (xspi->selected == spi_chip_select(slave->dev) &&
		xspi->dtr == op->cmd.dtr)
		return;

	if (!op->cmd.dtr) {
		nxp_xspi_disable_ddr(xspi);
		rate = min(xspi->support_max_rate, rate);
		xspi->dtr = false;
	} else {
		nxp_xspi_enable_ddr(xspi);
		rate = min(xspi->support_max_rate, rate);
		rate *= 2;
		xspi->dtr = true;
	}

#if CONFIG_IS_ENABLED(CLK)
	int ret;

	nxp_xspi_clk_disable_unprep(xspi);

	ret = clk_set_rate(&xspi->clk, rate);
	if (ret < 0)
		return;

	ret = nxp_xspi_clk_prep_enable(xspi);
	if (ret)
		return;
#endif

	xspi->selected = spi_chip_select(slave->dev);

	if (!op->cmd.dtr || rate < MHZ(60))
		nxp_xspi_dll_bypass(xspi);
	else
		nxp_xspi_dll_auto(xspi, rate);
}

static int nxp_xspi_exec_op(struct spi_slave *slave,
			    const struct spi_mem_op *op)
{
	struct nxp_xspi *x;
	struct udevice *bus;
	int err = 0;

	bus = slave->dev->parent;
	x = dev_get_priv(bus);

	dev_dbg(bus, "%s:%s:%d\n", __FILE__, __func__, __LINE__);
	dev_dbg(bus, "buswidth = %u, nbytes = %u, dtr = %u, opcode = 0x%x\n",
		op->cmd.buswidth, op->cmd.nbytes, op->cmd.dtr, op->cmd.opcode);
	dev_dbg(bus, "buswidth = %u, nbytes = %u, dtr = %u, val = 0x%llx\n",
		op->addr.buswidth, op->addr.nbytes, op->addr.dtr, op->addr.val);
	dev_dbg(bus, "buswidth = %u, nbytes = %u, dtr = %u\n",
		op->dummy.buswidth, op->dummy.nbytes, op->dummy.dtr);
	dev_dbg(bus, "buswidth = %u, nbytes = %u, dtr = %u, dir = %u, buf = 0x%llx\n",
		op->data.buswidth, op->data.nbytes, op->data.dtr, op->data.dir,
		(u64)op->data.buf.in);

	nxp_xspi_select_mem(x, slave, op);

	nxp_xspi_prepare_lut(x, op);
	/*
	 * If we have large chunks of data, we read them through the AHB bus by
	 * accessing the mapped memory. In all other cases we use IP commands
	 * to access the flash. Read via AHB bus may be corrupted due to
	 * existence of an errata and therefore discard AHB read in such cases.
	 */
	if (op->data.nbytes > (x->config.gmid ? x->devtype_data->rxfifo : DEFAULT_XMIT_SIZE) &&
	    op->data.dir == SPI_MEM_DATA_IN) {
		dev_dbg(bus, "ahb read\n");
		nxp_xspi_read_ahb(x, op);
	} else {
		dev_dbg(bus, "ip command\n");
		/* Wait for controller being ready. */
		err = xspi_readl_poll_tout(x, x->config.env, XSPI_SR, XSPI_SR_BUSY_MASK,
					   1, POLL_TOUT, false);
		WARN_ON(err);

		xspi_writel_offset(x, x->config.env, GENMASK(31, 0), FR);

		if (op->data.nbytes) {
			if (op->data.dir == SPI_MEM_DATA_OUT)
				nxp_xspi_fill_txfifo(x, op);
			else if (op->data.dir == SPI_MEM_DATA_IN)
				nxp_xspi_read_rxfifo(x, op);
			else
				dev_dbg(x->dev, "%d: never should happen\r\n", __LINE__);
		} else {
			nxp_xspi_xfer_cmd(x, op);
		}
	}

#ifdef DEBUG
	if (op->data.nbytes <= 10)
		if (op->data.dir != SPI_MEM_NO_DATA)
			print_buffer(0, op->data.buf.out, 1, op->data.nbytes, 16);
#endif

	return err;
}

static const struct spi_controller_mem_ops nxp_xspi_mem_ops = {
	.adjust_op_size = nxp_xspi_adjust_op_size,
	.supports_op = nxp_xspi_supports_op,
	.exec_op = nxp_xspi_exec_op,
};

static const struct dm_spi_ops nxp_xspi_ops = {
	.claim_bus	= nxp_xspi_claim_bus,
	.set_speed	= nxp_xspi_set_speed,
	.set_mode	= nxp_xspi_set_mode,
	.mem_ops        = &nxp_xspi_mem_ops,
};

static int nxp_xspi_of_to_plat(struct udevice *bus)
{
	struct nxp_xspi *x = dev_get_priv(bus);
	fdt_addr_t iobase;
	fdt_addr_t iobase_size;
	fdt_addr_t ahb_addr;
	fdt_addr_t ahb_size;

#if CONFIG_IS_ENABLED(CLK)
	int ret;
#endif

	x->dev = bus;

	iobase = devfdt_get_addr_size_name(bus, "xspi_base", &iobase_size);
	if (iobase == FDT_ADDR_T_NONE) {
		dev_err(bus, "xspi_base regs missing\n");
		return -ENODEV;
	}
	x->iobase = iobase;

	ahb_addr = devfdt_get_addr_size_name(bus, "xspi_mmap", &ahb_size);
	if (ahb_addr == FDT_ADDR_T_NONE) {
		dev_err(bus, "xspi_mmap regs missing\n");
		return -ENODEV;
	}
	x->ahb_addr = ahb_addr;
	x->a1_size = ahb_size;
	x->a2_size = 0;
	x->config.gmid = true;
	x->config.env = 0;

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_get_by_name(bus, "xspi", &x->clk);
	if (ret) {
		dev_err(bus, "failed to get xspi clock\n");
		return ret;
	}
#endif

	dev_dbg(bus, "iobase=<0x%x>, ahb_addr=<0x%x>, a1_size=<0x%x>, a2_size=<0x%x>, env=<0x%x>, gmid=<0x%x>\n",
		x->iobase, x->ahb_addr, x->a1_size, x->a2_size, x->config.env, x->config.gmid);

	return 0;
}

static int nxp_xspi_config_ahb_buffers(struct nxp_xspi *x)
{
	u32 reg;

	reg = XSPI_BUF3CR_MSTRID(0xa);
	xspi_writel_offset(x, 0, reg, BUF0CR);
	reg = XSPI_BUF3CR_MSTRID(0x2);
	xspi_writel_offset(x, 0, reg, BUF1CR);
	reg = XSPI_BUF3CR_MSTRID(0xd);
	xspi_writel_offset(x, 0, reg, BUF2CR);

	reg = XSPI_BUF3CR_MSTRID(0x6) | XSPI_BUF3CR_ALLMST_MASK;
	reg |= XSPI_BUF3CR_ADATSZ(x->devtype_data->ahb_buf_size / 8U);
	xspi_writel_offset(x, 0, reg, BUF3CR);

	/* Only the buffer3 is used */
	xspi_writel_offset(x, 0, 0, BUF0IND);
	xspi_writel_offset(x, 0, 0, BUF1IND);
	xspi_writel_offset(x, 0, 0, BUF2IND);

	/* Program the Sequence ID for read/write operation. */
	reg = XSPI_BFGENCR_SEQID_WR_EN_MASK | XSPI_BFGENCR_SEQID(CMD_LUT_FOR_AHB_CMD);
	xspi_writel_offset(x, 0, reg, BFGENCR);

	/* AHB access towards flash is broken if this AHB alignment boundary is crossed */
	/* 0-No limit 1-256B 10-512B 11b-limit */
	xspi_set_reg_field(x, 0, 0, BFGENCR, ALIGN);

	return 0;
};

static void nxp_xspi_config_mdad(struct nxp_xspi *x)
{
	xspi_writel_offset(x, 0, XSPI_TG2MDAD_EXT_VLD_MASK, TG0MDAD);
	xspi_writel_offset(x, 0, XSPI_TG2MDAD_EXT_VLD_MASK, TG1MDAD);
	xspi_writel_offset(x, 0, XSPI_TG2MDAD_EXT_VLD_MASK, TG2MDAD_EXT);
	xspi_writel_offset(x, 0, XSPI_TG2MDAD_EXT_VLD_MASK, TG3MDAD_EXT);
	xspi_writel_offset(x, 0, XSPI_TG2MDAD_EXT_VLD_MASK, TG4MDAD_EXT);
}

static void nxp_xspi_config_frad(struct nxp_xspi *x)
{
	/* Enable Read/Write Access permissions & Valid */
	for (int i = 0; i < 8; i++) {
		xspi_writel(x, XSPI_FRAD0_WORD2_MD0ACP_MASK | XSPI_FRAD0_WORD2_MD1ACP_MASK,
			    x->iobase + XSPI_FRAD0_WORD2 + (i * 0x20U));
		xspi_writel(x, XSPI_FRAD0_WORD3_VLD_MASK,
			    x->iobase + XSPI_FRAD0_WORD3 + (i * 0x20U));
	}
	for (int i = 0; i < 8; i++) {
		xspi_writel(x, XSPI_FRAD0_WORD2_MD0ACP_MASK | XSPI_FRAD0_WORD2_MD1ACP_MASK,
			    x->iobase + XSPI_FRAD8_WORD2 + (i * 0x20U));
		xspi_writel(x, XSPI_FRAD0_WORD3_VLD_MASK,
			    x->iobase + XSPI_FRAD8_WORD3 + (i * 0x20U));
	}
}

static int nxp_xspi_default_setup(struct nxp_xspi *x)
{
	int ret = 0;
	u32 reg;

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_set_rate(&x->clk, 20UL * 1000000UL);
	if (ret < 0) {
		dev_err(x->dev, "clk_set_rate fail\n");
		return ret;
	}
	dev_dbg(x->dev, "clk rate = %lu\n", clk_get_rate(&x->clk));

	ret = nxp_xspi_clk_prep_enable(x);
	if (ret) {
		dev_err(x->dev, "nxp_xspi_clk_prep_enable fail\n");
		return ret;
	}
#endif

	if (x->config.gmid) {
		reg = xspi_readl_offset(x, 0, MGC);
		reg &= ~(XSPI_MGC_GVLD_MASK | XSPI_MGC_GVLDMDAD_MASK | XSPI_MGC_GVLDFRAD_MASK);
		xspi_writel_offset(x, 0, reg, MGC);

		xspi_writel_offset(x, 0, GENMASK(31, 0), MTO);
	}

	nxp_xspi_config_mdad(x);
	nxp_xspi_config_frad(x);

	xspi_set_reg_field(x, 0, 0, MCR, MDIS);

	xspi_swreset(x);

	xspi_set_reg_field(x, 0, 1, MCR, MDIS);

	reg = xspi_readl_offset(x, 0, MCR);
	reg &= ~(XSPI_MCR_END_CFG_MASK | XSPI_MCR_DQS_FA_SEL_MASK |
		 XSPI_MCR_DDR_EN_MASK | XSPI_MCR_DQS_EN_MASK | XSPI_MCR_CKN_FA_EN_MASK |
		 XSPI_MCR_DQS_OUT_EN_MASK | XSPI_MCR_ISD2FA_MASK | XSPI_MCR_ISD3FA_MASK);

	reg |= XSPI_MCR_ISD2FA_MASK;
	reg |= XSPI_MCR_ISD3FA_MASK;

	reg |= XSPI_MCR_END_CFG(3);

	xspi_writel_offset(x, 0, reg, MCR);

	reg = xspi_readl_offset(x, 0, SFACR);

	reg &= ~(uint32_t)(XSPI_SFACR_CAS_MASK | XSPI_SFACR_WA_MASK |
			   XSPI_SFACR_BYTE_SWAP_MASK | XSPI_SFACR_WA_4B_EN_MASK |
			   XSPI_SFACR_FORCE_A10_MASK);

	xspi_writel_offset(x, 0, reg, SFACR);

	nxp_xspi_config_ahb_buffers(x);

	reg = XSPI_FLSHCR_TCSH(3) | XSPI_FLSHCR_TCSS(3);
	xspi_writel_offset(x, 0, reg, FLSHCR);

	xspi_writel_offset(x, 0, x->ahb_addr + x->a1_size, SFA1AD);
	xspi_writel_offset(x, 0, x->ahb_addr + x->a1_size + x->a2_size, SFA2AD);

	reg = XSPI_SMPR_DLLFSMPFA(7);
	xspi_writel_offset(x, 0, reg, SMPR);

	xspi_set_reg_field(x, 0, 0, MCR, MDIS);

	xspi_swreset(x);

	x->selected = -1;

	return ret;
};

static int nxp_xspi_probe(struct udevice *bus)
{
	int ret;
	struct nxp_xspi *x = dev_get_priv(bus);

	x->devtype_data =
		(struct nxp_xspi_devtype_data *)dev_get_driver_data(bus);

	ret = nxp_xspi_default_setup(x);
	if (ret)
		dev_err(x->dev, "nxp_xspi_default_setup fail %d\n", ret);

	return ret;
};

U_BOOT_DRIVER(nxp_xspi) = {
	.name	= "nxp_xspi",
	.id	= UCLASS_SPI,
	.of_match = nxp_xspi_ids,
	.ops	= &nxp_xspi_ops,
	.of_to_plat = nxp_xspi_of_to_plat,
	.priv_auto	= sizeof(struct nxp_xspi),
	.probe	= nxp_xspi_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};
