/*
 * Copyright (c) 2010-2013 NVIDIA Corporation
 * With help from the mpc8xxx SPI driver
 * With more help from omap3_spi SPI driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/pinmux.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra20/tegra20_sflash.h>
#include <spi.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

#define SPI_CMD_GO			(1 << 30)
#define SPI_CMD_ACTIVE_SCLK_SHIFT	26
#define SPI_CMD_ACTIVE_SCLK_MASK	(3 << SPI_CMD_ACTIVE_SCLK_SHIFT)
#define SPI_CMD_CK_SDA			(1 << 21)
#define SPI_CMD_ACTIVE_SDA_SHIFT	18
#define SPI_CMD_ACTIVE_SDA_MASK		(3 << SPI_CMD_ACTIVE_SDA_SHIFT)
#define SPI_CMD_CS_POL			(1 << 16)
#define SPI_CMD_TXEN			(1 << 15)
#define SPI_CMD_RXEN			(1 << 14)
#define SPI_CMD_CS_VAL			(1 << 13)
#define SPI_CMD_CS_SOFT			(1 << 12)
#define SPI_CMD_CS_DELAY		(1 << 9)
#define SPI_CMD_CS3_EN			(1 << 8)
#define SPI_CMD_CS2_EN			(1 << 7)
#define SPI_CMD_CS1_EN			(1 << 6)
#define SPI_CMD_CS0_EN			(1 << 5)
#define SPI_CMD_BIT_LENGTH		(1 << 4)
#define SPI_CMD_BIT_LENGTH_MASK		0x0000001F

#define SPI_STAT_BSY			(1 << 31)
#define SPI_STAT_RDY			(1 << 30)
#define SPI_STAT_RXF_FLUSH		(1 << 29)
#define SPI_STAT_TXF_FLUSH		(1 << 28)
#define SPI_STAT_RXF_UNR		(1 << 27)
#define SPI_STAT_TXF_OVF		(1 << 26)
#define SPI_STAT_RXF_EMPTY		(1 << 25)
#define SPI_STAT_RXF_FULL		(1 << 24)
#define SPI_STAT_TXF_EMPTY		(1 << 23)
#define SPI_STAT_TXF_FULL		(1 << 22)
#define SPI_STAT_SEL_TXRX_N		(1 << 16)
#define SPI_STAT_CUR_BLKCNT		(1 << 15)

#define SPI_TIMEOUT		1000
#define TEGRA_SPI_MAX_FREQ	52000000

struct spi_regs {
	u32 command;	/* SPI_COMMAND_0 register  */
	u32 status;	/* SPI_STATUS_0 register */
	u32 rx_cmp;	/* SPI_RX_CMP_0 register  */
	u32 dma_ctl;	/* SPI_DMA_CTL_0 register */
	u32 tx_fifo;	/* SPI_TX_FIFO_0 register */
	u32 rsvd[3];	/* offsets 0x14 to 0x1F reserved */
	u32 rx_fifo;	/* SPI_RX_FIFO_0 register */
};

struct tegra_spi_ctrl {
	struct spi_regs *regs;
	unsigned int freq;
	unsigned int mode;
	int periph_id;
	int valid;
};

struct tegra_spi_slave {
	struct spi_slave slave;
	struct tegra_spi_ctrl *ctrl;
};

/* tegra20 only supports one SFLASH controller */
static struct tegra_spi_ctrl spi_ctrls[1];

static inline struct tegra_spi_slave *to_tegra_spi(struct spi_slave *slave)
{
	return container_of(slave, struct tegra_spi_slave, slave);
}

int tegra20_spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	/* Tegra20 SPI-Flash - only 1 device ('bus/cs') */
	if (bus != 0 || cs != 0)
		return 0;
	else
		return 1;
}

struct spi_slave *tegra20_spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct tegra_spi_slave *spi;

	if (!spi_cs_is_valid(bus, cs)) {
		printf("SPI error: unsupported bus %d / chip select %d\n",
		       bus, cs);
		return NULL;
	}

	if (max_hz > TEGRA_SPI_MAX_FREQ) {
		printf("SPI error: unsupported frequency %d Hz. Max frequency"
			" is %d Hz\n", max_hz, TEGRA_SPI_MAX_FREQ);
		return NULL;
	}

	spi = spi_alloc_slave(struct tegra_spi_slave, bus, cs);
	if (!spi) {
		printf("SPI error: malloc of SPI structure failed\n");
		return NULL;
	}
	spi->ctrl = &spi_ctrls[bus];
	if (!spi->ctrl) {
		printf("SPI error: could not find controller for bus %d\n",
		       bus);
		return NULL;
	}

	if (max_hz < spi->ctrl->freq) {
		debug("%s: limiting frequency from %u to %u\n", __func__,
		      spi->ctrl->freq, max_hz);
		spi->ctrl->freq = max_hz;
	}
	spi->ctrl->mode = mode;

	return &spi->slave;
}

void tegra20_spi_free_slave(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);

	free(spi);
}

int tegra20_spi_init(int *node_list, int count)
{
	struct tegra_spi_ctrl *ctrl;
	int i;
	int node = 0;
	int found = 0;

	for (i = 0; i < count; i++) {
		ctrl = &spi_ctrls[i];
		node = node_list[i];

		ctrl->regs = (struct spi_regs *)fdtdec_get_addr(gd->fdt_blob,
								node, "reg");
		if ((fdt_addr_t)ctrl->regs == FDT_ADDR_T_NONE) {
			debug("%s: no slink register found\n", __func__);
			continue;
		}
		ctrl->freq = fdtdec_get_int(gd->fdt_blob, node,
					    "spi-max-frequency", 0);
		if (!ctrl->freq) {
			debug("%s: no slink max frequency found\n", __func__);
			continue;
		}

		ctrl->periph_id = clock_decode_periph_id(gd->fdt_blob, node);
		if (ctrl->periph_id == PERIPH_ID_NONE) {
			debug("%s: could not decode periph id\n", __func__);
			continue;
		}
		ctrl->valid = 1;
		found = 1;

		debug("%s: found controller at %p, freq = %u, periph_id = %d\n",
		      __func__, ctrl->regs, ctrl->freq, ctrl->periph_id);
	}
	return !found;
}

int tegra20_spi_claim_bus(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct spi_regs *regs = spi->ctrl->regs;
	u32 reg;

	/* Change SPI clock to correct frequency, PLLP_OUT0 source */
	clock_start_periph_pll(spi->ctrl->periph_id, CLOCK_ID_PERIPH,
			       spi->ctrl->freq);

	/* Clear stale status here */
	reg = SPI_STAT_RDY | SPI_STAT_RXF_FLUSH | SPI_STAT_TXF_FLUSH | \
		SPI_STAT_RXF_UNR | SPI_STAT_TXF_OVF;
	writel(reg, &regs->status);
	debug("%s: STATUS = %08x\n", __func__, readl(&regs->status));

	/*
	 * Use sw-controlled CS, so we can clock in data after ReadID, etc.
	 */
	reg = (spi->ctrl->mode & 1) << SPI_CMD_ACTIVE_SDA_SHIFT;
	if (spi->ctrl->mode & 2)
		reg |= 1 << SPI_CMD_ACTIVE_SCLK_SHIFT;
	clrsetbits_le32(&regs->command, SPI_CMD_ACTIVE_SCLK_MASK |
		SPI_CMD_ACTIVE_SDA_MASK, SPI_CMD_CS_SOFT | reg);
	debug("%s: COMMAND = %08x\n", __func__, readl(&regs->command));

	/*
	 * SPI pins on Tegra20 are muxed - change pinmux later due to UART
	 * issue.
	 */
	pinmux_set_func(PMUX_PINGRP_GMD, PMUX_FUNC_SFLASH);
	pinmux_tristate_disable(PMUX_PINGRP_LSPI);
	pinmux_set_func(PMUX_PINGRP_GMC, PMUX_FUNC_SFLASH);

	return 0;
}

void tegra20_spi_cs_activate(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct spi_regs *regs = spi->ctrl->regs;

	/* CS is negated on Tegra, so drive a 1 to get a 0 */
	setbits_le32(&regs->command, SPI_CMD_CS_VAL);
}

void tegra20_spi_cs_deactivate(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct spi_regs *regs = spi->ctrl->regs;

	/* CS is negated on Tegra, so drive a 0 to get a 1 */
	clrbits_le32(&regs->command, SPI_CMD_CS_VAL);
}

int tegra20_spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *data_out, void *data_in, unsigned long flags)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct spi_regs *regs = spi->ctrl->regs;
	u32 reg, tmpdout, tmpdin = 0;
	const u8 *dout = data_out;
	u8 *din = data_in;
	int num_bytes;
	int ret;

	debug("spi_xfer: slave %u:%u dout %08X din %08X bitlen %u\n",
	      slave->bus, slave->cs, *(u8 *)dout, *(u8 *)din, bitlen);
	if (bitlen % 8)
		return -1;
	num_bytes = bitlen / 8;

	ret = 0;

	reg = readl(&regs->status);
	writel(reg, &regs->status);	/* Clear all SPI events via R/W */
	debug("spi_xfer entry: STATUS = %08x\n", reg);

	reg = readl(&regs->command);
	reg |= SPI_CMD_TXEN | SPI_CMD_RXEN;
	writel(reg, &regs->command);
	debug("spi_xfer: COMMAND = %08x\n", readl(&regs->command));

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	/* handle data in 32-bit chunks */
	while (num_bytes > 0) {
		int bytes;
		int is_read = 0;
		int tm, i;

		tmpdout = 0;
		bytes = (num_bytes > 4) ?  4 : num_bytes;

		if (dout != NULL) {
			for (i = 0; i < bytes; ++i)
				tmpdout = (tmpdout << 8) | dout[i];
		}

		num_bytes -= bytes;
		if (dout)
			dout += bytes;

		clrsetbits_le32(&regs->command, SPI_CMD_BIT_LENGTH_MASK,
				bytes * 8 - 1);
		writel(tmpdout, &regs->tx_fifo);
		setbits_le32(&regs->command, SPI_CMD_GO);

		/*
		 * Wait for SPI transmit FIFO to empty, or to time out.
		 * The RX FIFO status will be read and cleared last
		 */
		for (tm = 0, is_read = 0; tm < SPI_TIMEOUT; ++tm) {
			u32 status;

			status = readl(&regs->status);

			/* We can exit when we've had both RX and TX activity */
			if (is_read && (status & SPI_STAT_TXF_EMPTY))
				break;

			if ((status & (SPI_STAT_BSY | SPI_STAT_RDY)) !=
					SPI_STAT_RDY)
				tm++;

			else if (!(status & SPI_STAT_RXF_EMPTY)) {
				tmpdin = readl(&regs->rx_fifo);
				is_read = 1;

				/* swap bytes read in */
				if (din != NULL) {
					for (i = bytes - 1; i >= 0; --i) {
						din[i] = tmpdin & 0xff;
						tmpdin >>= 8;
					}
					din += bytes;
				}
			}
		}

		if (tm >= SPI_TIMEOUT)
			ret = tm;

		/* clear ACK RDY, etc. bits */
		writel(readl(&regs->status), &regs->status);
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	debug("spi_xfer: transfer ended. Value=%08x, status = %08x\n",
		tmpdin, readl(&regs->status));

	if (ret) {
		printf("spi_xfer: timeout during SPI transfer, tm %d\n", ret);
		return -1;
	}

	return 0;
}
