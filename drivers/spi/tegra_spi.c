/*
 * Copyright (c) 2010-2012 NVIDIA Corporation
 * With help from the mpc8xxx SPI driver
 * With more help from omap3_spi SPI driver
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 */

#include <common.h>

#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clk_rst.h>
#include <asm/arch/clock.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/uart-spi-switch.h>
#include <asm/arch/tegra_spi.h>

#if defined(CONFIG_SPI_CORRUPTS_UART)
 #define corrupt_delay()	udelay(CONFIG_SPI_CORRUPTS_UART_DLY);
#else
 #define corrupt_delay()
#endif

struct tegra_spi_slave {
	struct spi_slave slave;
	struct spi_tegra *regs;
	unsigned int freq;
	unsigned int mode;
};

static inline struct tegra_spi_slave *to_tegra_spi(struct spi_slave *slave)
{
	return container_of(slave, struct tegra_spi_slave, slave);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	/* Tegra2 SPI-Flash - only 1 device ('bus/cs') */
	if (bus != 0 || cs != 0)
		return 0;
	else
		return 1;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct tegra_spi_slave *spi;

	if (!spi_cs_is_valid(bus, cs)) {
		printf("SPI error: unsupported bus %d / chip select %d\n",
		       bus, cs);
		return NULL;
	}

	if (max_hz > TEGRA2_SPI_MAX_FREQ) {
		printf("SPI error: unsupported frequency %d Hz. Max frequency"
			" is %d Hz\n", max_hz, TEGRA2_SPI_MAX_FREQ);
		return NULL;
	}

	spi = malloc(sizeof(struct tegra_spi_slave));
	if (!spi) {
		printf("SPI error: malloc of SPI structure failed\n");
		return NULL;
	}
	spi->slave.bus = bus;
	spi->slave.cs = cs;
	spi->freq = max_hz;
	spi->regs = (struct spi_tegra *)TEGRA2_SPI_BASE;
	spi->mode = mode;

	return &spi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);

	free(spi);
}

void spi_init(void)
{
	/* do nothing */
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct spi_tegra *regs = spi->regs;
	u32 reg;

	/* Change SPI clock to correct frequency, PLLP_OUT0 source */
	clock_start_periph_pll(PERIPH_ID_SPI1, CLOCK_ID_PERIPH, spi->freq);

	/* Clear stale status here */
	reg = SPI_STAT_RDY | SPI_STAT_RXF_FLUSH | SPI_STAT_TXF_FLUSH | \
		SPI_STAT_RXF_UNR | SPI_STAT_TXF_OVF;
	writel(reg, &regs->status);
	debug("spi_init: STATUS = %08x\n", readl(&regs->status));

	/*
	 * Use sw-controlled CS, so we can clock in data after ReadID, etc.
	 */
	reg = (spi->mode & 1) << SPI_CMD_ACTIVE_SDA_SHIFT;
	if (spi->mode & 2)
		reg |= 1 << SPI_CMD_ACTIVE_SCLK_SHIFT;
	clrsetbits_le32(&regs->command, SPI_CMD_ACTIVE_SCLK_MASK |
		SPI_CMD_ACTIVE_SDA_MASK, SPI_CMD_CS_SOFT | reg);
	debug("spi_init: COMMAND = %08x\n", readl(&regs->command));

	/*
	 * SPI pins on Tegra2 are muxed - change pinmux later due to UART
	 * issue.
	 */
	pinmux_set_func(PINGRP_GMD, PMUX_FUNC_SFLASH);
	pinmux_tristate_disable(PINGRP_LSPI);

#ifndef CONFIG_SPI_UART_SWITCH
	/*
	 * NOTE:
	 * Only set PinMux bits 3:2 to SPI here on boards that don't have the
	 * SPI UART switch or subsequent UART data won't go out!  See
	 * spi_uart_switch().
	 */
	/* TODO: pinmux_set_func(PINGRP_GMC, PMUX_FUNC_SFLASH); */
#endif
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	/*
	 * We can't release UART_DISABLE and set pinmux to UART4 here since
	 * some code (e,g, spi_flash_probe) uses printf() while the SPI
	 * bus is held. That is arguably bad, but it has the advantage of
	 * already being in the source tree.
	 */
}

void spi_cs_activate(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);

	pinmux_select_spi();

	/* CS is negated on Tegra, so drive a 1 to get a 0 */
	setbits_le32(&spi->regs->command, SPI_CMD_CS_VAL);

	corrupt_delay();		/* Let UART settle */
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);

	pinmux_select_uart();

	/* CS is negated on Tegra, so drive a 0 to get a 1 */
	clrbits_le32(&spi->regs->command, SPI_CMD_CS_VAL);

	corrupt_delay();		/* Let SPI settle */
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *data_out, void *data_in, unsigned long flags)
{
	struct tegra_spi_slave *spi = to_tegra_spi(slave);
	struct spi_tegra *regs = spi->regs;
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
