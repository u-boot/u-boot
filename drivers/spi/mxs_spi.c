/*
 * Freescale i.MX28 SPI driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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
 *
 * NOTE: This driver only supports the SPI-controller chipselects,
 *       GPIO driven chipselects are not supported.
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>

#define	MXS_SPI_MAX_TIMEOUT	1000000
#define	MXS_SPI_PORT_OFFSET	0x2000

struct mxs_spi_slave {
	struct spi_slave	slave;
	uint32_t		max_khz;
	uint32_t		mode;
	struct mx28_ssp_regs	*regs;
};

static inline struct mxs_spi_slave *to_mxs_slave(struct spi_slave *slave)
{
	return container_of(slave, struct mxs_spi_slave, slave);
}

void spi_init(void)
{
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct mxs_spi_slave *mxs_slave;
	uint32_t addr;

	if (bus > 3) {
		printf("MXS SPI: Max bus number is 3\n");
		return NULL;
	}

	mxs_slave = malloc(sizeof(struct mxs_spi_slave));
	if (!mxs_slave)
		return NULL;

	addr = MXS_SSP0_BASE + (bus * MXS_SPI_PORT_OFFSET);

	mxs_slave->slave.bus = bus;
	mxs_slave->slave.cs = cs;
	mxs_slave->max_khz = max_hz / 1000;
	mxs_slave->mode = mode;
	mxs_slave->regs = (struct mx28_ssp_regs *)addr;

	return &mxs_slave->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct mxs_spi_slave *mxs_slave = to_mxs_slave(slave);
	free(mxs_slave);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct mxs_spi_slave *mxs_slave = to_mxs_slave(slave);
	struct mx28_ssp_regs *ssp_regs = mxs_slave->regs;
	uint32_t reg = 0;

	mx28_reset_block(&ssp_regs->hw_ssp_ctrl0_reg);

	writel(SSP_CTRL0_BUS_WIDTH_ONE_BIT, &ssp_regs->hw_ssp_ctrl0);

	reg = SSP_CTRL1_SSP_MODE_SPI | SSP_CTRL1_WORD_LENGTH_EIGHT_BITS;
	reg |= (mxs_slave->mode & SPI_CPOL) ? SSP_CTRL1_POLARITY : 0;
	reg |= (mxs_slave->mode & SPI_CPHA) ? SSP_CTRL1_PHASE : 0;
	writel(reg, &ssp_regs->hw_ssp_ctrl1);

	writel(0, &ssp_regs->hw_ssp_cmd0);

	mx28_set_ssp_busclock(slave->bus, mxs_slave->max_khz);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

static void mxs_spi_start_xfer(struct mx28_ssp_regs *ssp_regs)
{
	writel(SSP_CTRL0_LOCK_CS, &ssp_regs->hw_ssp_ctrl0_set);
	writel(SSP_CTRL0_IGNORE_CRC, &ssp_regs->hw_ssp_ctrl0_clr);
}

static void mxs_spi_end_xfer(struct mx28_ssp_regs *ssp_regs)
{
	writel(SSP_CTRL0_LOCK_CS, &ssp_regs->hw_ssp_ctrl0_clr);
	writel(SSP_CTRL0_IGNORE_CRC, &ssp_regs->hw_ssp_ctrl0_set);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct mxs_spi_slave *mxs_slave = to_mxs_slave(slave);
	struct mx28_ssp_regs *ssp_regs = mxs_slave->regs;
	int len = bitlen / 8;
	const char *tx = dout;
	char *rx = din;
	char dummy;

	if (bitlen == 0) {
		if (flags & SPI_XFER_END) {
			rx = &dummy;
			len = 1;
		} else
			return 0;
	}

	if (!rx && !tx)
		return 0;

	if (flags & SPI_XFER_BEGIN)
		mxs_spi_start_xfer(ssp_regs);

	while (len--) {
		/* We transfer 1 byte */
		writel(1, &ssp_regs->hw_ssp_xfer_size);

		if ((flags & SPI_XFER_END) && !len)
			mxs_spi_end_xfer(ssp_regs);

		if (tx)
			writel(SSP_CTRL0_READ, &ssp_regs->hw_ssp_ctrl0_clr);
		else
			writel(SSP_CTRL0_READ, &ssp_regs->hw_ssp_ctrl0_set);

		writel(SSP_CTRL0_RUN, &ssp_regs->hw_ssp_ctrl0_set);

		if (mx28_wait_mask_set(&ssp_regs->hw_ssp_ctrl0_reg,
			SSP_CTRL0_RUN, MXS_SPI_MAX_TIMEOUT)) {
			printf("MXS SPI: Timeout waiting for start\n");
			return -ETIMEDOUT;
		}

		if (tx)
			writel(*tx++, &ssp_regs->hw_ssp_data);

		writel(SSP_CTRL0_DATA_XFER, &ssp_regs->hw_ssp_ctrl0_set);

		if (rx) {
			if (mx28_wait_mask_clr(&ssp_regs->hw_ssp_status_reg,
				SSP_STATUS_FIFO_EMPTY, MXS_SPI_MAX_TIMEOUT)) {
				printf("MXS SPI: Timeout waiting for data\n");
				return -ETIMEDOUT;
			}

			*rx = readl(&ssp_regs->hw_ssp_data);
			rx++;
		}

		if (mx28_wait_mask_clr(&ssp_regs->hw_ssp_ctrl0_reg,
			SSP_CTRL0_RUN, MXS_SPI_MAX_TIMEOUT)) {
			printf("MXS SPI: Timeout waiting for finish\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}
