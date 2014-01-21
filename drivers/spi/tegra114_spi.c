/*
 * NVIDIA Tegra SPI controller (T114 and later)
 *
 * Copyright (c) 2010-2013 NVIDIA Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra114/tegra114_spi.h>
#include <spi.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

/* COMMAND1 */
#define SPI_CMD1_GO			(1 << 31)
#define SPI_CMD1_M_S			(1 << 30)
#define SPI_CMD1_MODE_MASK		0x3
#define SPI_CMD1_MODE_SHIFT		28
#define SPI_CMD1_CS_SEL_MASK		0x3
#define SPI_CMD1_CS_SEL_SHIFT		26
#define SPI_CMD1_CS_POL_INACTIVE3	(1 << 25)
#define SPI_CMD1_CS_POL_INACTIVE2	(1 << 24)
#define SPI_CMD1_CS_POL_INACTIVE1	(1 << 23)
#define SPI_CMD1_CS_POL_INACTIVE0	(1 << 22)
#define SPI_CMD1_CS_SW_HW		(1 << 21)
#define SPI_CMD1_CS_SW_VAL		(1 << 20)
#define SPI_CMD1_IDLE_SDA_MASK		0x3
#define SPI_CMD1_IDLE_SDA_SHIFT		18
#define SPI_CMD1_BIDIR			(1 << 17)
#define SPI_CMD1_LSBI_FE		(1 << 16)
#define SPI_CMD1_LSBY_FE		(1 << 15)
#define SPI_CMD1_BOTH_EN_BIT		(1 << 14)
#define SPI_CMD1_BOTH_EN_BYTE		(1 << 13)
#define SPI_CMD1_RX_EN			(1 << 12)
#define SPI_CMD1_TX_EN			(1 << 11)
#define SPI_CMD1_PACKED			(1 << 5)
#define SPI_CMD1_BIT_LEN_MASK		0x1F
#define SPI_CMD1_BIT_LEN_SHIFT		0

/* COMMAND2 */
#define SPI_CMD2_TX_CLK_TAP_DELAY	(1 << 6)
#define SPI_CMD2_TX_CLK_TAP_DELAY_MASK	(0x3F << 6)
#define SPI_CMD2_RX_CLK_TAP_DELAY	(1 << 0)
#define SPI_CMD2_RX_CLK_TAP_DELAY_MASK	(0x3F << 0)

/* TRANSFER STATUS */
#define SPI_XFER_STS_RDY		(1 << 30)

/* FIFO STATUS */
#define SPI_FIFO_STS_CS_INACTIVE	(1 << 31)
#define SPI_FIFO_STS_FRAME_END		(1 << 30)
#define SPI_FIFO_STS_RX_FIFO_FLUSH	(1 << 15)
#define SPI_FIFO_STS_TX_FIFO_FLUSH	(1 << 14)
#define SPI_FIFO_STS_ERR		(1 << 8)
#define SPI_FIFO_STS_TX_FIFO_OVF	(1 << 7)
#define SPI_FIFO_STS_TX_FIFO_UNR	(1 << 6)
#define SPI_FIFO_STS_RX_FIFO_OVF	(1 << 5)
#define SPI_FIFO_STS_RX_FIFO_UNR	(1 << 4)
#define SPI_FIFO_STS_TX_FIFO_FULL	(1 << 3)
#define SPI_FIFO_STS_TX_FIFO_EMPTY	(1 << 2)
#define SPI_FIFO_STS_RX_FIFO_FULL	(1 << 1)
#define SPI_FIFO_STS_RX_FIFO_EMPTY	(1 << 0)

#define SPI_TIMEOUT		1000
#define TEGRA_SPI_MAX_FREQ	52000000

struct spi_regs {
	u32 command1;	/* 000:SPI_COMMAND1 register */
	u32 command2;	/* 004:SPI_COMMAND2 register */
	u32 timing1;	/* 008:SPI_CS_TIM1 register */
	u32 timing2;	/* 00c:SPI_CS_TIM2 register */
	u32 xfer_status;/* 010:SPI_TRANS_STATUS register */
	u32 fifo_status;/* 014:SPI_FIFO_STATUS register */
	u32 tx_data;	/* 018:SPI_TX_DATA register */
	u32 rx_data;	/* 01c:SPI_RX_DATA register */
	u32 dma_ctl;	/* 020:SPI_DMA_CTL register */
	u32 dma_blk;	/* 024:SPI_DMA_BLK register */
	u32 rsvd[56];	/* 028-107 reserved */
	u32 tx_fifo;	/* 108:SPI_FIFO1 register */
	u32 rsvd2[31];	/* 10c-187 reserved */
	u32 rx_fifo;	/* 188:SPI_FIFO2 register */
	u32 spare_ctl;	/* 18c:SPI_SPARE_CTRL register */
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

static struct tegra_spi_ctrl spi_ctrls[CONFIG_TEGRA114_SPI_CTRLS];

static inline struct tegra_spi_slave *to_tegra_spi(struct spi_slave *slave)
{
	return container_of(slave, struct tegra_spi_slave, slave);
}

int tegra114_spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	if (bus >= CONFIG_TEGRA114_SPI_CTRLS || cs > 3 || !spi_ctrls[bus].valid)
		return 0;
	else
		return 1;
}

struct spi_slave *tegra114_spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct tegra_spi_slave *spi;

	debug("%s: bus: %u, cs: %u, max_hz: %u, mode: %u\n", __func__,
		bus, cs, max_hz, mode);

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

void tegra114_spi_free_slave(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);

	free(spi);
}

int tegra114_spi_init(int *node_list, int count)
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
			debug("%s: no spi register found\n", __func__);
			continue;
		}
		ctrl->freq = fdtdec_get_int(gd->fdt_blob, node,
					    "spi-max-frequency", 0);
		if (!ctrl->freq) {
			debug("%s: no spi max frequency found\n", __func__);
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

int tegra114_spi_claim_bus(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct spi_regs *regs = spi->ctrl->regs;

	/* Change SPI clock to correct frequency, PLLP_OUT0 source */
	clock_start_periph_pll(spi->ctrl->periph_id, CLOCK_ID_PERIPH,
			       spi->ctrl->freq);

	/* Clear stale status here */
	setbits_le32(&regs->fifo_status,
		     SPI_FIFO_STS_ERR		|
		     SPI_FIFO_STS_TX_FIFO_OVF	|
		     SPI_FIFO_STS_TX_FIFO_UNR	|
		     SPI_FIFO_STS_RX_FIFO_OVF	|
		     SPI_FIFO_STS_RX_FIFO_UNR	|
		     SPI_FIFO_STS_TX_FIFO_FULL	|
		     SPI_FIFO_STS_TX_FIFO_EMPTY	|
		     SPI_FIFO_STS_RX_FIFO_FULL	|
		     SPI_FIFO_STS_RX_FIFO_EMPTY);
	debug("%s: FIFO STATUS = %08x\n", __func__, readl(&regs->fifo_status));

	/* Set master mode and sw controlled CS */
	setbits_le32(&regs->command1, SPI_CMD1_M_S | SPI_CMD1_CS_SW_HW |
		     (spi->ctrl->mode << SPI_CMD1_MODE_SHIFT));
	debug("%s: COMMAND1 = %08x\n", __func__, readl(&regs->command1));

	return 0;
}

void tegra114_spi_cs_activate(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct spi_regs *regs = spi->ctrl->regs;

	clrbits_le32(&regs->command1, SPI_CMD1_CS_SW_VAL);
}

void tegra114_spi_cs_deactivate(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct spi_regs *regs = spi->ctrl->regs;

	setbits_le32(&regs->command1, SPI_CMD1_CS_SW_VAL);
}

int tegra114_spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *data_out, void *data_in, unsigned long flags)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct spi_regs *regs = spi->ctrl->regs;
	u32 reg, tmpdout, tmpdin = 0;
	const u8 *dout = data_out;
	u8 *din = data_in;
	int num_bytes;
	int ret;

	debug("%s: slave %u:%u dout %p din %p bitlen %u\n",
	      __func__, slave->bus, slave->cs, dout, din, bitlen);
	if (bitlen % 8)
		return -1;
	num_bytes = bitlen / 8;

	ret = 0;

	/* clear all error status bits */
	reg = readl(&regs->fifo_status);
	writel(reg, &regs->fifo_status);

	clrsetbits_le32(&regs->command1, SPI_CMD1_CS_SW_VAL,
			SPI_CMD1_RX_EN | SPI_CMD1_TX_EN | SPI_CMD1_LSBY_FE |
			(slave->cs << SPI_CMD1_CS_SEL_SHIFT));

	/* set xfer size to 1 block (32 bits) */
	writel(0, &regs->dma_blk);

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	/* handle data in 32-bit chunks */
	while (num_bytes > 0) {
		int bytes;
		int tm, i;

		tmpdout = 0;
		bytes = (num_bytes > 4) ?  4 : num_bytes;

		if (dout != NULL) {
			for (i = 0; i < bytes; ++i)
				tmpdout = (tmpdout << 8) | dout[i];
			dout += bytes;
		}

		num_bytes -= bytes;

		/* clear ready bit */
		setbits_le32(&regs->xfer_status, SPI_XFER_STS_RDY);

		clrsetbits_le32(&regs->command1,
				SPI_CMD1_BIT_LEN_MASK << SPI_CMD1_BIT_LEN_SHIFT,
				(bytes * 8 - 1) << SPI_CMD1_BIT_LEN_SHIFT);
		writel(tmpdout, &regs->tx_fifo);
		setbits_le32(&regs->command1, SPI_CMD1_GO);

		/*
		 * Wait for SPI transmit FIFO to empty, or to time out.
		 * The RX FIFO status will be read and cleared last
		 */
		for (tm = 0; tm < SPI_TIMEOUT; ++tm) {
			u32 fifo_status, xfer_status;

			xfer_status = readl(&regs->xfer_status);
			if (!(xfer_status & SPI_XFER_STS_RDY))
				continue;

			fifo_status = readl(&regs->fifo_status);
			if (fifo_status & SPI_FIFO_STS_ERR) {
				debug("%s: got a fifo error: ", __func__);
				if (fifo_status & SPI_FIFO_STS_TX_FIFO_OVF)
					debug("tx FIFO overflow ");
				if (fifo_status & SPI_FIFO_STS_TX_FIFO_UNR)
					debug("tx FIFO underrun ");
				if (fifo_status & SPI_FIFO_STS_RX_FIFO_OVF)
					debug("rx FIFO overflow ");
				if (fifo_status & SPI_FIFO_STS_RX_FIFO_UNR)
					debug("rx FIFO underrun ");
				if (fifo_status & SPI_FIFO_STS_TX_FIFO_FULL)
					debug("tx FIFO full ");
				if (fifo_status & SPI_FIFO_STS_TX_FIFO_EMPTY)
					debug("tx FIFO empty ");
				if (fifo_status & SPI_FIFO_STS_RX_FIFO_FULL)
					debug("rx FIFO full ");
				if (fifo_status & SPI_FIFO_STS_RX_FIFO_EMPTY)
					debug("rx FIFO empty ");
				debug("\n");
				break;
			}

			if (!(fifo_status & SPI_FIFO_STS_RX_FIFO_EMPTY)) {
				tmpdin = readl(&regs->rx_fifo);

				/* swap bytes read in */
				if (din != NULL) {
					for (i = bytes - 1; i >= 0; --i) {
						din[i] = tmpdin & 0xff;
						tmpdin >>= 8;
					}
					din += bytes;
				}

				/* We can exit when we've had both RX and TX */
				break;
			}
		}

		if (tm >= SPI_TIMEOUT)
			ret = tm;

		/* clear ACK RDY, etc. bits */
		writel(readl(&regs->fifo_status), &regs->fifo_status);
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	debug("%s: transfer ended. Value=%08x, fifo_status = %08x\n",
	      __func__, tmpdin, readl(&regs->fifo_status));

	if (ret) {
		printf("%s: timeout during SPI transfer, tm %d\n",
		       __func__, ret);
		return -1;
	}

	return 0;
}
