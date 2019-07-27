// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 SiFive, Inc.
 * Copyright 2019 Bhargav Shah <bhargavshah1988@gmail.com>
 *
 * SiFive SPI controller driver (master mode only)
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <linux/log2.h>
#include <clk.h>

#define SIFIVE_SPI_MAX_CS		32

#define SIFIVE_SPI_DEFAULT_DEPTH	8
#define SIFIVE_SPI_DEFAULT_BITS		8

/* register offsets */
#define SIFIVE_SPI_REG_SCKDIV            0x00 /* Serial clock divisor */
#define SIFIVE_SPI_REG_SCKMODE           0x04 /* Serial clock mode */
#define SIFIVE_SPI_REG_CSID              0x10 /* Chip select ID */
#define SIFIVE_SPI_REG_CSDEF             0x14 /* Chip select default */
#define SIFIVE_SPI_REG_CSMODE            0x18 /* Chip select mode */
#define SIFIVE_SPI_REG_DELAY0            0x28 /* Delay control 0 */
#define SIFIVE_SPI_REG_DELAY1            0x2c /* Delay control 1 */
#define SIFIVE_SPI_REG_FMT               0x40 /* Frame format */
#define SIFIVE_SPI_REG_TXDATA            0x48 /* Tx FIFO data */
#define SIFIVE_SPI_REG_RXDATA            0x4c /* Rx FIFO data */
#define SIFIVE_SPI_REG_TXMARK            0x50 /* Tx FIFO watermark */
#define SIFIVE_SPI_REG_RXMARK            0x54 /* Rx FIFO watermark */
#define SIFIVE_SPI_REG_FCTRL             0x60 /* SPI flash interface control */
#define SIFIVE_SPI_REG_FFMT              0x64 /* SPI flash instruction format */
#define SIFIVE_SPI_REG_IE                0x70 /* Interrupt Enable Register */
#define SIFIVE_SPI_REG_IP                0x74 /* Interrupt Pendings Register */

/* sckdiv bits */
#define SIFIVE_SPI_SCKDIV_DIV_MASK       0xfffU

/* sckmode bits */
#define SIFIVE_SPI_SCKMODE_PHA           BIT(0)
#define SIFIVE_SPI_SCKMODE_POL           BIT(1)
#define SIFIVE_SPI_SCKMODE_MODE_MASK     (SIFIVE_SPI_SCKMODE_PHA | \
					  SIFIVE_SPI_SCKMODE_POL)

/* csmode bits */
#define SIFIVE_SPI_CSMODE_MODE_AUTO      0U
#define SIFIVE_SPI_CSMODE_MODE_HOLD      2U
#define SIFIVE_SPI_CSMODE_MODE_OFF       3U

/* delay0 bits */
#define SIFIVE_SPI_DELAY0_CSSCK(x)       ((u32)(x))
#define SIFIVE_SPI_DELAY0_CSSCK_MASK     0xffU
#define SIFIVE_SPI_DELAY0_SCKCS(x)       ((u32)(x) << 16)
#define SIFIVE_SPI_DELAY0_SCKCS_MASK     (0xffU << 16)

/* delay1 bits */
#define SIFIVE_SPI_DELAY1_INTERCS(x)     ((u32)(x))
#define SIFIVE_SPI_DELAY1_INTERCS_MASK   0xffU
#define SIFIVE_SPI_DELAY1_INTERXFR(x)    ((u32)(x) << 16)
#define SIFIVE_SPI_DELAY1_INTERXFR_MASK  (0xffU << 16)

/* fmt bits */
#define SIFIVE_SPI_FMT_PROTO_SINGLE      0U
#define SIFIVE_SPI_FMT_PROTO_DUAL        1U
#define SIFIVE_SPI_FMT_PROTO_QUAD        2U
#define SIFIVE_SPI_FMT_PROTO_MASK        3U
#define SIFIVE_SPI_FMT_ENDIAN            BIT(2)
#define SIFIVE_SPI_FMT_DIR               BIT(3)
#define SIFIVE_SPI_FMT_LEN(x)            ((u32)(x) << 16)
#define SIFIVE_SPI_FMT_LEN_MASK          (0xfU << 16)

/* txdata bits */
#define SIFIVE_SPI_TXDATA_DATA_MASK      0xffU
#define SIFIVE_SPI_TXDATA_FULL           BIT(31)

/* rxdata bits */
#define SIFIVE_SPI_RXDATA_DATA_MASK      0xffU
#define SIFIVE_SPI_RXDATA_EMPTY          BIT(31)

/* ie and ip bits */
#define SIFIVE_SPI_IP_TXWM               BIT(0)
#define SIFIVE_SPI_IP_RXWM               BIT(1)

struct sifive_spi {
	void		*regs;		/* base address of the registers */
	u32		fifo_depth;
	u32		bits_per_word;
	u32		cs_inactive;	/* Level of the CS pins when inactive*/
	u32		freq;
	u32		num_cs;
};

static void sifive_spi_prep_device(struct sifive_spi *spi,
				   struct dm_spi_slave_platdata *slave)
{
	/* Update the chip select polarity */
	if (slave->mode & SPI_CS_HIGH)
		spi->cs_inactive &= ~BIT(slave->cs);
	else
		spi->cs_inactive |= BIT(slave->cs);
	writel(spi->cs_inactive, spi->regs + SIFIVE_SPI_REG_CSDEF);

	/* Select the correct device */
	writel(slave->cs, spi->regs + SIFIVE_SPI_REG_CSID);
}

static int sifive_spi_set_cs(struct sifive_spi *spi,
			     struct dm_spi_slave_platdata *slave)
{
	u32 cs_mode = SIFIVE_SPI_CSMODE_MODE_HOLD;

	if (slave->mode & SPI_CS_HIGH)
		cs_mode = SIFIVE_SPI_CSMODE_MODE_AUTO;

	writel(cs_mode, spi->regs + SIFIVE_SPI_REG_CSMODE);

	return 0;
}

static void sifive_spi_clear_cs(struct sifive_spi *spi)
{
	writel(SIFIVE_SPI_CSMODE_MODE_AUTO, spi->regs + SIFIVE_SPI_REG_CSMODE);
}

static void sifive_spi_prep_transfer(struct sifive_spi *spi,
				     bool is_rx_xfer,
				     struct dm_spi_slave_platdata *slave)
{
	u32 cr;

	/* Modify the SPI protocol mode */
	cr = readl(spi->regs + SIFIVE_SPI_REG_FMT);

	/* Bits per word ? */
	cr &= ~SIFIVE_SPI_FMT_LEN_MASK;
	cr |= SIFIVE_SPI_FMT_LEN(spi->bits_per_word);

	/* LSB first? */
	cr &= ~SIFIVE_SPI_FMT_ENDIAN;
	if (slave->mode & SPI_LSB_FIRST)
		cr |= SIFIVE_SPI_FMT_ENDIAN;

	/* Number of wires ? */
	cr &= ~SIFIVE_SPI_FMT_PROTO_MASK;
	if ((slave->mode & SPI_TX_QUAD) || (slave->mode & SPI_RX_QUAD))
		cr |= SIFIVE_SPI_FMT_PROTO_QUAD;
	else if ((slave->mode & SPI_TX_DUAL) || (slave->mode & SPI_RX_DUAL))
		cr |= SIFIVE_SPI_FMT_PROTO_DUAL;
	else
		cr |= SIFIVE_SPI_FMT_PROTO_SINGLE;

	/* SPI direction in/out ? */
	cr &= ~SIFIVE_SPI_FMT_DIR;
	if (!is_rx_xfer)
		cr |= SIFIVE_SPI_FMT_DIR;

	writel(cr, spi->regs + SIFIVE_SPI_REG_FMT);
}

static void sifive_spi_rx(struct sifive_spi *spi, u8 *rx_ptr)
{
	u32 data;

	do {
		data = readl(spi->regs + SIFIVE_SPI_REG_RXDATA);
	} while (data & SIFIVE_SPI_RXDATA_EMPTY);

	if (rx_ptr)
		*rx_ptr = data & SIFIVE_SPI_RXDATA_DATA_MASK;
}

static void sifive_spi_tx(struct sifive_spi *spi, const u8 *tx_ptr)
{
	u32 data;
	u8 tx_data = (tx_ptr) ? *tx_ptr & SIFIVE_SPI_TXDATA_DATA_MASK :
				SIFIVE_SPI_TXDATA_DATA_MASK;

	do {
		data = readl(spi->regs + SIFIVE_SPI_REG_TXDATA);
	} while (data & SIFIVE_SPI_TXDATA_FULL);

	writel(tx_data, spi->regs + SIFIVE_SPI_REG_TXDATA);
}

static int sifive_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct sifive_spi *spi = dev_get_priv(bus);
	struct dm_spi_slave_platdata *slave = dev_get_parent_platdata(dev);
	const unsigned char *tx_ptr = dout;
	u8 *rx_ptr = din;
	u32 remaining_len;
	int ret;

	if (flags & SPI_XFER_BEGIN) {
		sifive_spi_prep_device(spi, slave);

		ret = sifive_spi_set_cs(spi, slave);
		if (ret)
			return ret;
	}

	sifive_spi_prep_transfer(spi, true, slave);

	remaining_len = bitlen / 8;

	while (remaining_len) {
		int n_words, tx_words, rx_words;

		n_words = min(remaining_len, spi->fifo_depth);

		/* Enqueue n_words for transmission */
		if (tx_ptr) {
			for (tx_words = 0; tx_words < n_words; ++tx_words) {
				sifive_spi_tx(spi, tx_ptr);
				sifive_spi_rx(spi, NULL);
				tx_ptr++;
			}
		}

		/* Read out all the data from the RX FIFO */
		if (rx_ptr) {
			for (rx_words = 0; rx_words < n_words; ++rx_words) {
				sifive_spi_tx(spi, NULL);
				sifive_spi_rx(spi, rx_ptr);
				rx_ptr++;
			}
		}

		remaining_len -= n_words;
	}

	if (flags & SPI_XFER_END)
		sifive_spi_clear_cs(spi);

	return 0;
}

static int sifive_spi_set_speed(struct udevice *bus, uint speed)
{
	struct sifive_spi *spi = dev_get_priv(bus);
	u32 scale;

	if (speed > spi->freq)
		speed = spi->freq;

	/* Cofigure max speed */
	scale = (DIV_ROUND_UP(spi->freq >> 1, speed) - 1)
					& SIFIVE_SPI_SCKDIV_DIV_MASK;
	writel(scale, spi->regs + SIFIVE_SPI_REG_SCKDIV);

	return 0;
}

static int sifive_spi_set_mode(struct udevice *bus, uint mode)
{
	struct sifive_spi *spi = dev_get_priv(bus);
	u32 cr;

	/* Switch clock mode bits */
	cr = readl(spi->regs + SIFIVE_SPI_REG_SCKMODE) &
				~SIFIVE_SPI_SCKMODE_MODE_MASK;
	if (mode & SPI_CPHA)
		cr |= SIFIVE_SPI_SCKMODE_PHA;
	if (mode & SPI_CPOL)
		cr |= SIFIVE_SPI_SCKMODE_POL;

	writel(cr, spi->regs + SIFIVE_SPI_REG_SCKMODE);

	return 0;
}

static int sifive_spi_cs_info(struct udevice *bus, uint cs,
			      struct spi_cs_info *info)
{
	struct sifive_spi *spi = dev_get_priv(bus);

	if (cs >= spi->num_cs)
		return -EINVAL;

	return 0;
}

static void sifive_spi_init_hw(struct sifive_spi *spi)
{
	u32 cs_bits;

	/* probe the number of CS lines */
	spi->cs_inactive = readl(spi->regs + SIFIVE_SPI_REG_CSDEF);
	writel(0xffffffffU, spi->regs + SIFIVE_SPI_REG_CSDEF);
	cs_bits = readl(spi->regs + SIFIVE_SPI_REG_CSDEF);
	writel(spi->cs_inactive, spi->regs + SIFIVE_SPI_REG_CSDEF);
	if (!cs_bits) {
		printf("Could not auto probe CS lines\n");
		return;
	}

	spi->num_cs = ilog2(cs_bits) + 1;
	if (spi->num_cs > SIFIVE_SPI_MAX_CS) {
		printf("Invalid number of spi slaves\n");
		return;
	}

	/* Watermark interrupts are disabled by default */
	writel(0, spi->regs + SIFIVE_SPI_REG_IE);

	/* Set CS/SCK Delays and Inactive Time to defaults */
	writel(SIFIVE_SPI_DELAY0_CSSCK(1) | SIFIVE_SPI_DELAY0_SCKCS(1),
	       spi->regs + SIFIVE_SPI_REG_DELAY0);
	writel(SIFIVE_SPI_DELAY1_INTERCS(1) | SIFIVE_SPI_DELAY1_INTERXFR(0),
	       spi->regs + SIFIVE_SPI_REG_DELAY1);

	/* Exit specialized memory-mapped SPI flash mode */
	writel(0, spi->regs + SIFIVE_SPI_REG_FCTRL);
}

static int sifive_spi_probe(struct udevice *bus)
{
	struct sifive_spi *spi = dev_get_priv(bus);
	struct clk clkdev;
	int ret;

	spi->regs = (void *)(ulong)dev_remap_addr(bus);
	if (!spi->regs)
		return -ENODEV;

	spi->fifo_depth = dev_read_u32_default(bus,
					       "sifive,fifo-depth",
					       SIFIVE_SPI_DEFAULT_DEPTH);

	spi->bits_per_word = dev_read_u32_default(bus,
						  "sifive,max-bits-per-word",
						  SIFIVE_SPI_DEFAULT_BITS);

	ret = clk_get_by_index(bus, 0, &clkdev);
	if (ret)
		return ret;
	spi->freq = clk_get_rate(&clkdev);

	/* init the sifive spi hw */
	sifive_spi_init_hw(spi);

	return 0;
}

static const struct dm_spi_ops sifive_spi_ops = {
	.xfer		= sifive_spi_xfer,
	.set_speed	= sifive_spi_set_speed,
	.set_mode	= sifive_spi_set_mode,
	.cs_info        = sifive_spi_cs_info,
};

static const struct udevice_id sifive_spi_ids[] = {
	{ .compatible = "sifive,spi0" },
	{ }
};

U_BOOT_DRIVER(sifive_spi) = {
	.name	= "sifive_spi",
	.id	= UCLASS_SPI,
	.of_match = sifive_spi_ids,
	.ops	= &sifive_spi_ops,
	.priv_auto_alloc_size = sizeof(struct sifive_spi),
	.probe	= sifive_spi_probe,
};
