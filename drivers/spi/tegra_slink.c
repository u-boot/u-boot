/*
 * NVIDIA Tegra SPI-SLINK controller
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
#include <asm/arch-tegra/tegra_slink.h>
#include <spi.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

struct tegra_spi_ctrl {
	struct slink_tegra *regs;
	unsigned int freq;
	unsigned int mode;
	int periph_id;
	int valid;
};

struct tegra_spi_slave {
	struct spi_slave slave;
	struct tegra_spi_ctrl *ctrl;
};

static struct tegra_spi_ctrl spi_ctrls[CONFIG_TEGRA_SLINK_CTRLS];

static inline struct tegra_spi_slave *to_tegra_spi(struct spi_slave *slave)
{
	return container_of(slave, struct tegra_spi_slave, slave);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	if (bus >= CONFIG_TEGRA_SLINK_CTRLS || cs > 3 || !spi_ctrls[bus].valid)
		return 0;
	else
		return 1;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
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

	spi = malloc(sizeof(struct tegra_spi_slave));
	if (!spi) {
		printf("SPI error: malloc of SPI structure failed\n");
		return NULL;
	}
	spi->slave.bus = bus;
	spi->slave.cs = cs;
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

void spi_free_slave(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);

	free(spi);
}

void spi_init(void)
{
	struct tegra_spi_ctrl *ctrl;
	int i;
#ifdef CONFIG_OF_CONTROL
	int node = 0;
	int count;
	int node_list[CONFIG_TEGRA_SLINK_CTRLS];

	count = fdtdec_find_aliases_for_id(gd->fdt_blob, "spi",
					   COMPAT_NVIDIA_TEGRA20_SLINK,
					   node_list,
					   CONFIG_TEGRA_SLINK_CTRLS);
	for (i = 0; i < count; i++) {
		ctrl = &spi_ctrls[i];
		node = node_list[i];

		ctrl->regs = (struct slink_tegra *)fdtdec_get_addr(gd->fdt_blob,
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

		debug("%s: found controller at %p, freq = %u, periph_id = %d\n",
		      __func__, ctrl->regs, ctrl->freq, ctrl->periph_id);
	}
#else
	for (i = 0; i < CONFIG_TEGRA_SLINK_CTRLS; i++) {
		ctrl = &spi_ctrls[i];
		u32 base_regs[] = {
			NV_PA_SLINK1_BASE,
			NV_PA_SLINK2_BASE,
			NV_PA_SLINK3_BASE,
			NV_PA_SLINK4_BASE,
			NV_PA_SLINK5_BASE,
			NV_PA_SLINK6_BASE,
		};
		int periph_ids[] = {
			PERIPH_ID_SBC1,
			PERIPH_ID_SBC2,
			PERIPH_ID_SBC3,
			PERIPH_ID_SBC4,
			PERIPH_ID_SBC5,
			PERIPH_ID_SBC6,
		};
		ctrl->regs = (struct slink_tegra *)base_regs[i];
		ctrl->freq = TEGRA_SPI_MAX_FREQ;
		ctrl->periph_id = periph_ids[i];
		ctrl->valid = 1;

		debug("%s: found controller at %p, freq = %u, periph_id = %d\n",
		      __func__, ctrl->regs, ctrl->freq, ctrl->periph_id);
	}
#endif
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct slink_tegra *regs = spi->ctrl->regs;
	u32 reg;

	/* Change SPI clock to correct frequency, PLLP_OUT0 source */
	clock_start_periph_pll(spi->ctrl->periph_id, CLOCK_ID_PERIPH,
			       spi->ctrl->freq);

	/* Clear stale status here */
	reg = SLINK_STAT_RDY | SLINK_STAT_RXF_FLUSH | SLINK_STAT_TXF_FLUSH | \
		SLINK_STAT_RXF_UNR | SLINK_STAT_TXF_OVF;
	writel(reg, &regs->status);
	debug("%s: STATUS = %08x\n", __func__, readl(&regs->status));

	/* Set master mode and sw controlled CS */
	reg = readl(&regs->command);
	reg |= SLINK_CMD_M_S | SLINK_CMD_CS_SOFT;
	writel(reg, &regs->command);
	debug("%s: COMMAND = %08x\n", __func__, readl(&regs->command));

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

void spi_cs_activate(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct slink_tegra *regs = spi->ctrl->regs;

	/* CS is negated on Tegra, so drive a 1 to get a 0 */
	setbits_le32(&regs->command, SLINK_CMD_CS_VAL);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct slink_tegra *regs = spi->ctrl->regs;

	/* CS is negated on Tegra, so drive a 0 to get a 1 */
	clrbits_le32(&regs->command, SLINK_CMD_CS_VAL);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *data_out, void *data_in, unsigned long flags)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct slink_tegra *regs = spi->ctrl->regs;
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

	reg = readl(&regs->status);
	writel(reg, &regs->status);	/* Clear all SPI events via R/W */
	debug("%s entry: STATUS = %08x\n", __func__, reg);

	reg = readl(&regs->status2);
	writel(reg, &regs->status2);	/* Clear all STATUS2 events via R/W */
	debug("%s entry: STATUS2 = %08x\n", __func__, reg);

	debug("%s entry: COMMAND = %08x\n", __func__, readl(&regs->command));

	clrsetbits_le32(&regs->command2, SLINK_CMD2_SS_EN_MASK,
			SLINK_CMD2_TXEN | SLINK_CMD2_RXEN |
			(slave->cs << SLINK_CMD2_SS_EN_SHIFT));
	debug("%s entry: COMMAND2 = %08x\n", __func__, readl(&regs->command2));

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
			dout += bytes;
		}

		num_bytes -= bytes;

		clrsetbits_le32(&regs->command, SLINK_CMD_BIT_LENGTH_MASK,
				bytes * 8 - 1);
		writel(tmpdout, &regs->tx_fifo);
		setbits_le32(&regs->command, SLINK_CMD_GO);

		/*
		 * Wait for SPI transmit FIFO to empty, or to time out.
		 * The RX FIFO status will be read and cleared last
		 */
		for (tm = 0, is_read = 0; tm < SPI_TIMEOUT; ++tm) {
			u32 status;

			status = readl(&regs->status);

			/* We can exit when we've had both RX and TX activity */
			if (is_read && (status & SLINK_STAT_TXF_EMPTY))
				break;

			if ((status & (SLINK_STAT_BSY | SLINK_STAT_RDY)) !=
					SLINK_STAT_RDY)
				tm++;

			else if (!(status & SLINK_STAT_RXF_EMPTY)) {
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

	debug("%s: transfer ended. Value=%08x, status = %08x\n",
	      __func__, tmpdin, readl(&regs->status));

	if (ret) {
		printf("%s: timeout during SPI transfer, tm %d\n",
		       __func__, ret);
		return -1;
	}

	return 0;
}
