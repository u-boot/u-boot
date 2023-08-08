// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Marvell International Ltd.
 *
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <spi.h>
#include <clk.h>
#include <wait_bit.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define MVEBU_SPI_A3700_XFER_RDY		BIT(1)
#define MVEBU_SPI_A3700_FIFO_FLUSH		BIT(9)
#define MVEBU_SPI_A3700_BYTE_LEN		BIT(5)
#define MVEBU_SPI_A3700_CLK_PHA			BIT(6)
#define MVEBU_SPI_A3700_CLK_POL			BIT(7)
#define MVEBU_SPI_A3700_FIFO_EN			BIT(17)
#define MVEBU_SPI_A3700_SPI_EN_0		BIT(16)
#define MVEBU_SPI_A3700_WFIFO_FULL		BIT(7)
#define MVEBU_SPI_A3700_WFIFO_EMPTY		BIT(6)
#define MVEBU_SPI_A3700_RFIFO_EMPTY		BIT(4)
#define MVEBU_SPI_A3700_WFIFO_RDY		BIT(3)
#define MVEBU_SPI_A3700_RFIFO_RDY		BIT(2)
#define MVEBU_SPI_A3700_XFER_START		BIT(15)
#define MVEBU_SPI_A3700_XFER_STOP		BIT(14)
#define MVEBU_SPI_A3700_RW_EN			BIT(8)
#define MVEBU_SPI_A3700_WFIFO_THRS_BIT		28
#define MVEBU_SPI_A3700_RFIFO_THRS_BIT		24
#define MVEBU_SPI_A3700_FIFO_THRS_MASK		0x7
#define MVEBU_SPI_A3700_CLK_PRESCALE_MASK	0x1f
#define MVEBU_SPI_A3700_ADDR_CNT_BIT		4
#define MVEBU_SPI_A3700_ADDR_CNT_MASK		0x7

/* SPI registers */
struct spi_reg {
	u32 ctrl;	/* 0x10600 */
	u32 cfg;	/* 0x10604 */
	u32 dout;	/* 0x10608 */
	u32 din;	/* 0x1060c */
	u32 inst_addr;	/* 0x10610 */
	u32 addr;	/* 0x10614 */
	u32 rmode;	/* 0x10618 */
	u32 hdr_cnt;	/* 0x1061c */
	u32 din_cnt;	/* 0x10620 */
};

struct mvebu_spi_platdata {
	struct spi_reg *spireg;
	struct clk clk;
};

static void spi_cs_activate(struct spi_reg *reg, int cs)
{
	setbits_le32(&reg->ctrl, MVEBU_SPI_A3700_SPI_EN_0 << cs);
}

static void spi_cs_deactivate(struct spi_reg *reg, int cs)
{
	clrbits_le32(&reg->ctrl, MVEBU_SPI_A3700_SPI_EN_0 << cs);
}

static inline void spi_bytelen_set(struct spi_reg *reg, unsigned int len)
{
	unsigned int data;

	data = readl(&reg->cfg);
	if (len == 4)
		data = data | MVEBU_SPI_A3700_BYTE_LEN;
	else
		data = data & (~MVEBU_SPI_A3700_BYTE_LEN);

	writel(data, &reg->cfg);
}

static inline int spi_is_wfifo_full(struct spi_reg *reg)
{
	u32 val;

	val = readl(&reg->ctrl);
	return val & MVEBU_SPI_A3700_WFIFO_FULL;
}

static inline int spi_is_wfifo_empty(struct spi_reg *reg)
{
	u32 val;

	val = readl(&reg->ctrl);
	return val & MVEBU_SPI_A3700_WFIFO_EMPTY;
}

static int spi_fifo_out(struct spi_reg *reg, unsigned int buf_len,
			unsigned char *tx_buf)
{
	unsigned int val = 0;

	while (!spi_is_wfifo_full(reg) && buf_len) {
		val = (tx_buf[3] << 24) | (tx_buf[2] << 16) |
				 (tx_buf[1] << 8) | tx_buf[0];

		writel(val, &reg->dout);

		buf_len -= 4;
		tx_buf += 4;
	}

	/* Return the unwritten bytes number */
	return buf_len;
}

static inline int spi_is_rfifo_empty(struct spi_reg *reg)
{
	u32 val;

	val = readl(&reg->ctrl);
	return val & MVEBU_SPI_A3700_RFIFO_EMPTY;
}

static int spi_fifo_in(struct spi_reg *reg, unsigned int buf_len,
		       unsigned char *rx_buf)
{
	unsigned int val;

	while (!spi_is_rfifo_empty(reg)) {
		val = readl(&reg->din);
		if (buf_len >= 4) {
			rx_buf[0] = val & 0xff;
			rx_buf[1] = (val >> 8) & 0xff;
			rx_buf[2] = (val >> 16) & 0xff;
			rx_buf[3] = (val >> 24) & 0xff;
			buf_len -= 4;
			rx_buf += 4;
		} else {
			/*
			 * When remain bytes is smaller than 4, we should
			 * avoid memory overwriting and and just write the
			 * left rx buffer bytes.
			 */
			while (buf_len) {
				*rx_buf++ = val & 0xff;
				val >>= 8;
				buf_len--;
			}
			break;
		}
	}

	/* Return the unread bytes number */
	return buf_len;
}

static int spi_fifo_xfer_finisher(struct spi_reg *reg, bool force_stop)
{
	unsigned int val;
	int ret = 0;

	val = readl(&reg->cfg);
	if (force_stop) {
		val |= MVEBU_SPI_A3700_XFER_STOP;
		writel(val, &reg->cfg);
	}

	ret = wait_for_bit_le32(&reg->ctrl,
				MVEBU_SPI_A3700_XFER_START, false, 100, false);
	if (ret) {
		printf("spi_fifo_abort_xfer timeout\n");
		return ret;
	}

	val = readl(&reg->cfg);
	if (force_stop) {
		val &= ~MVEBU_SPI_A3700_XFER_STOP;
		writel(val, &reg->cfg);
	}

	return ret;
}

static unsigned int spi_fifo_header_set(struct spi_reg *reg,
					unsigned int bytelen, const void *dout)
{
	unsigned int addr_cnt = 0, val = 0, done_len = 0;
	unsigned char *dout_ptr = (unsigned char *)dout;

	/*
	 * Clean number of bytes for instruction, address,
	 * dummy field and read mode
	 */
	writel(0x0, &reg->inst_addr);
	writel(0x0, &reg->addr);
	writel(0x0, &reg->rmode);
	writel(0x0, &reg->hdr_cnt);

	if (bytelen % 4) {
		addr_cnt = bytelen % 4;
		val = ((addr_cnt & MVEBU_SPI_A3700_ADDR_CNT_MASK)
			<< MVEBU_SPI_A3700_ADDR_CNT_BIT);

		writel(val, &reg->hdr_cnt);
		done_len = addr_cnt;

		/* transfer 1~3 bytes by address count */
		val = 0;
		while (addr_cnt--) {
			val = (val << 8) | dout_ptr[0];
			dout_ptr++;
		}
		writel(val, &reg->addr);
	}
	return done_len;
}

static int spi_fifo_write(struct spi_reg *reg, unsigned int bytelen,
			  const void *dout)
{
	unsigned int val;
	int ret = 0;
	unsigned char *char_p;
	unsigned int len_done;
	int remain_len;

	/* when tx data is not 4 bytes aligned, there will be unexpected
	 * MSB bytes of SPI output register, since it always shifts out
	 * as whole 4 bytes. Which might causes incorrect transaction with
	 * some device and flash. To fix this, Serial Peripheral Interface
	 * Address (0xd0010614) in header count is used to transfer
	 * 1 to 3 bytes to make the rest of data 4 bytes aligned.
	 */
	len_done = spi_fifo_header_set(reg, bytelen, dout);
	bytelen -= len_done;

	/* Start Write transfer */
	val = readl(&reg->cfg);
	val |= (MVEBU_SPI_A3700_XFER_START | MVEBU_SPI_A3700_RW_EN);
	writel(val, &reg->cfg);

	/* Write data to spi */
	char_p = (unsigned char *)dout + len_done;
	while (bytelen) {
		ret = wait_for_bit_le32(&reg->ctrl,
					MVEBU_SPI_A3700_WFIFO_RDY, true, 100,
					false);
		if (ret) {
			printf("SPI: waiting write_fifo_ready timeout.n");
			return ret;
		}

		ret = wait_for_bit_le32(&reg->ctrl,
					MVEBU_SPI_A3700_WFIFO_FULL,
					false, 100, false);
		if (ret) {
			printf("SPI: write fifo is full.\n");
			return ret;
		}

		remain_len = spi_fifo_out(reg, bytelen, char_p);
		/* move fifo out pointer to unfinished data for next shift */
		char_p += bytelen - remain_len;
		bytelen = remain_len;
	}

	ret = wait_for_bit_le32(&reg->ctrl,
				MVEBU_SPI_A3700_WFIFO_EMPTY, true, 100, false);
	if (ret) {
		printf("SPI: waiting write_fifo_empty timeout.\n");
		return ret;
	}

	/* When write xfer finishes, force stop is needed */
	ret = spi_fifo_xfer_finisher(reg, true);

	return ret;
}

static int spi_fifo_read(struct spi_reg *reg, unsigned int bytelen,
			 void *din)
{
	unsigned int val;
	int ret = 0;
	unsigned char *char_p;
	int remain_len;

	/*
	 * Clean number of bytes for instruction, address,
	 * dummy field and read mode
	 */
	writel(0x0, &reg->inst_addr);
	writel(0x0, &reg->addr);
	writel(0x0, &reg->rmode);
	writel(0x0, &reg->hdr_cnt);

	/* Set read data length */
	writel(bytelen, &reg->din_cnt);
	/* Start READ transfer */
	val = readl(&reg->cfg);
	val &= ~MVEBU_SPI_A3700_RW_EN;
	val |= MVEBU_SPI_A3700_XFER_START;

	writel(val, &reg->cfg);

	/* Read data from spi */
	char_p = (unsigned char *)din;

	while (bytelen) {
		ret = wait_for_bit_le32(&reg->ctrl,
					MVEBU_SPI_A3700_RFIFO_RDY,
					true, 100, false);
		if (ret) {
			printf("SPI: read fifo ready is timeout.\n");
			return ret;
		}

		remain_len = spi_fifo_in(reg, bytelen, char_p);
		/* Move fifo in pointer to unfinished data for next shift */
		char_p += bytelen - remain_len;
		bytelen = remain_len;
	}

	/* When read xfer finishes, force stop is not needed. */
	ret = spi_fifo_xfer_finisher(reg, false);

	return ret;
}

static int mvebu_spi_xfer(struct udevice *dev, unsigned int bitlen,
			  const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);
	struct spi_reg *reg = plat->spireg;
	unsigned int bytelen;
	int ret = 0;
	u32 data;

	bytelen = bitlen / 8;

	if (dout && din)
		debug("This is a duplex transfer.\n");

	/* Activate CS */
	if (flags & SPI_XFER_BEGIN) {
		debug("SPI: activate cs.\n");
		spi_cs_activate(reg, spi_chip_select(dev));
	}

	/* Flush read/write FIFO */
	data = readl(&reg->cfg);
	writel(data | MVEBU_SPI_A3700_FIFO_FLUSH, &reg->cfg);
	ret = wait_for_bit_le32(&reg->cfg, MVEBU_SPI_A3700_FIFO_FLUSH,
				false, 1000, false);
	if (ret) {
		printf("SPI: fifo flush action is timeout.\n");
		return ret;
	}

	/* Send and/or receive */
	if (dout)
		ret = spi_fifo_write(reg, bytelen, dout);
	else if (din)
		ret = spi_fifo_read(reg, bytelen, din);

	/* Deactivate CS */
	if (flags & SPI_XFER_END) {
		ret = wait_for_bit_le32(&reg->ctrl, MVEBU_SPI_A3700_XFER_RDY,
					true, 100, false);
		if (ret) {
			printf("SPI: spi transfer data ready timeout\n");
			return ret;
		}

		debug("SPI: deactivate cs.\n");
		spi_cs_deactivate(reg, spi_chip_select(dev));
	}

	return ret;
}

static int mvebu_spi_set_speed(struct udevice *bus, uint hz)
{
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);
	struct dm_spi_bus *spi = dev_get_uclass_priv(bus);
	struct spi_reg *reg = plat->spireg;
	u32 data, prescale;

	if (hz > spi->max_hz) {
		debug("%s: limit speed to the max_hz of the bus %d\n",
		      __func__, spi->max_hz);
		hz = spi->max_hz;
	}

	data = readl(&reg->cfg);

	prescale = DIV_ROUND_UP(clk_get_rate(&plat->clk), hz);
	if (prescale > 0xf)
		prescale = 0x10 + (prescale + 1) / 2;
	prescale = min(prescale, 0x1fu);

	data &= ~MVEBU_SPI_A3700_CLK_PRESCALE_MASK;
	data |= prescale & MVEBU_SPI_A3700_CLK_PRESCALE_MASK;

	writel(data, &reg->cfg);

	return 0;
}

static int mvebu_spi_set_mode(struct udevice *bus, uint mode)
{
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);
	struct spi_reg *reg = plat->spireg;

	/*
	 * Set SPI polarity
	 * 0: Serial interface clock is low when inactive
	 * 1: Serial interface clock is high when inactive
	 */
	if (mode & SPI_CPOL)
		setbits_le32(&reg->cfg, MVEBU_SPI_A3700_CLK_POL);
	else
		clrbits_le32(&reg->cfg, MVEBU_SPI_A3700_CLK_POL);
	if (mode & SPI_CPHA)
		setbits_le32(&reg->cfg, MVEBU_SPI_A3700_CLK_PHA);
	else
		clrbits_le32(&reg->cfg, MVEBU_SPI_A3700_CLK_PHA);

	return 0;
}

static int mvebu_spi_probe(struct udevice *bus)
{
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);
	struct spi_reg *reg = plat->spireg;
	u32 data;
	int ret;


	/* Flush read/write FIFO */
	data = readl(&reg->cfg);
	writel(data | MVEBU_SPI_A3700_FIFO_FLUSH, &reg->cfg);
	ret = wait_for_bit_le32(&reg->cfg, MVEBU_SPI_A3700_FIFO_FLUSH,
				false, 1000, false);
	if (ret)
		return ret;

	/* Enable FIFO mode */
	data |= MVEBU_SPI_A3700_FIFO_EN;

	/*
	 * Set FIFO threshold
	 * For read FIFO threshold, value 0 presents 1 data entry, which means
	 * when data in the read FIFO is equal to or greater than 1 entry,
	 * flag RFIFO_RDY_IS will be set;
	 * For write FIFO threshold, value 7 presents 7 data entry, which means
	 * when data in the write FIFO is less than or equal to 7 entry,
	 * flag WFIFO_RDY_IS will be set;
	 */
	data |= 0 << MVEBU_SPI_A3700_RFIFO_THRS_BIT;
	data |= 7 << MVEBU_SPI_A3700_WFIFO_THRS_BIT;

	writel(data, &reg->cfg);

	/* set shift 4 bytes for writing and reading */
	spi_bytelen_set(reg, 4);
	return 0;
}

static int mvebu_spi_ofdata_to_platdata(struct udevice *bus)
{
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);
	int ret;

	plat->spireg = (struct spi_reg *)devfdt_get_addr(bus);

	ret = clk_get_by_index(bus, 0, &plat->clk);
	if (ret) {
		dev_err(bus, "cannot get clock\n");
		return ret;
	}

	return 0;
}

static int mvebu_spi_remove(struct udevice *bus)
{
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);

	clk_free(&plat->clk);

	return 0;
}

static const struct dm_spi_ops mvebu_spi_ops = {
	.xfer		= mvebu_spi_xfer,
	.set_speed	= mvebu_spi_set_speed,
	.set_mode	= mvebu_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id mvebu_spi_ids[] = {
	{ .compatible = "marvell,armada-3700-spi" },
	{ }
};

U_BOOT_DRIVER(mvebu_spi) = {
	.name = "mvebu_spi",
	.id = UCLASS_SPI,
	.of_match = mvebu_spi_ids,
	.ops = &mvebu_spi_ops,
	.ofdata_to_platdata = mvebu_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct mvebu_spi_platdata),
	.probe = mvebu_spi_probe,
	.remove = mvebu_spi_remove,
};
