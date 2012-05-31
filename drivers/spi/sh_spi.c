/*
 * SH SPI driver
 *
 * Copyright (C) 2011 Renesas Solutions Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include "sh_spi.h"

static void sh_spi_write(unsigned long data, unsigned long *reg)
{
	writel(data, reg);
}

static unsigned long sh_spi_read(unsigned long *reg)
{
	return readl(reg);
}

static void sh_spi_set_bit(unsigned long val, unsigned long *reg)
{
	unsigned long tmp;

	tmp = sh_spi_read(reg);
	tmp |= val;
	sh_spi_write(tmp, reg);
}

static void sh_spi_clear_bit(unsigned long val, unsigned long *reg)
{
	unsigned long tmp;

	tmp = sh_spi_read(reg);
	tmp &= ~val;
	sh_spi_write(tmp, reg);
}

static void clear_fifo(struct sh_spi *ss)
{
	sh_spi_set_bit(SH_SPI_RSTF, &ss->regs->cr2);
	sh_spi_clear_bit(SH_SPI_RSTF, &ss->regs->cr2);
}

static int recvbuf_wait(struct sh_spi *ss)
{
	while (sh_spi_read(&ss->regs->cr1) & SH_SPI_RBE) {
		if (ctrlc())
			return 1;
		udelay(10);
	}
	return 0;
}

static int write_fifo_empty_wait(struct sh_spi *ss)
{
	while (!(sh_spi_read(&ss->regs->cr1) & SH_SPI_TBE)) {
		if (ctrlc())
			return 1;
		udelay(10);
	}
	return 0;
}

void spi_init(void)
{
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct sh_spi *ss;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	ss = malloc(sizeof(struct spi_slave));
	if (!ss)
		return NULL;

	ss->slave.bus = bus;
	ss->slave.cs = cs;
	ss->regs = (struct sh_spi_regs *)CONFIG_SH_SPI_BASE;

	/* SPI sycle stop */
	sh_spi_write(0xfe, &ss->regs->cr1);
	/* CR1 init */
	sh_spi_write(0x00, &ss->regs->cr1);
	/* CR3 init */
	sh_spi_write(0x00, &ss->regs->cr3);

	clear_fifo(ss);

	/* 1/8 clock */
	sh_spi_write(sh_spi_read(&ss->regs->cr2) | 0x07, &ss->regs->cr2);
	udelay(10);

	return &ss->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct sh_spi *spi = to_sh_spi(slave);

	free(spi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct sh_spi *ss = to_sh_spi(slave);

	sh_spi_write(sh_spi_read(&ss->regs->cr1) &
		~(SH_SPI_SSA | SH_SPI_SSDB | SH_SPI_SSD), &ss->regs->cr1);
}

static int sh_spi_send(struct sh_spi *ss, const unsigned char *tx_data,
			unsigned int len, unsigned long flags)
{
	int i, cur_len, ret = 0;
	int remain = (int)len;
	unsigned long tmp;

	if (len >= SH_SPI_FIFO_SIZE)
		sh_spi_set_bit(SH_SPI_SSA, &ss->regs->cr1);

	while (remain > 0) {
		cur_len = (remain < SH_SPI_FIFO_SIZE) ?
				remain : SH_SPI_FIFO_SIZE;
		for (i = 0; i < cur_len &&
			!(sh_spi_read(&ss->regs->cr4) & SH_SPI_WPABRT) &&
			!(sh_spi_read(&ss->regs->cr1) & SH_SPI_TBF);
				i++)
			sh_spi_write(tx_data[i], &ss->regs->tbr_rbr);

		cur_len = i;

		if (sh_spi_read(&ss->regs->cr4) & SH_SPI_WPABRT) {
			/* Abort the transaction */
			flags |= SPI_XFER_END;
			sh_spi_set_bit(SH_SPI_WPABRT, &ss->regs->cr4);
			ret = 1;
			break;
		}

		remain -= cur_len;
		tx_data += cur_len;

		if (remain > 0)
			write_fifo_empty_wait(ss);
	}

	if (flags & SPI_XFER_END) {
		tmp = sh_spi_read(&ss->regs->cr1);
		tmp = tmp & ~(SH_SPI_SSD | SH_SPI_SSDB);
		sh_spi_write(tmp, &ss->regs->cr1);
		sh_spi_set_bit(SH_SPI_SSA, &ss->regs->cr1);
		udelay(100);
		write_fifo_empty_wait(ss);
	}

	return ret;
}

static int sh_spi_receive(struct sh_spi *ss, unsigned char *rx_data,
			  unsigned int len, unsigned long flags)
{
	int i;
	unsigned long tmp;

	if (len > SH_SPI_MAX_BYTE)
		sh_spi_write(SH_SPI_MAX_BYTE, &ss->regs->cr3);
	else
		sh_spi_write(len, &ss->regs->cr3);

	tmp = sh_spi_read(&ss->regs->cr1);
	tmp = tmp & ~(SH_SPI_SSD | SH_SPI_SSDB);
	sh_spi_write(tmp, &ss->regs->cr1);
	sh_spi_set_bit(SH_SPI_SSA, &ss->regs->cr1);

	for (i = 0; i < len; i++) {
		if (recvbuf_wait(ss))
			return 0;

		rx_data[i] = (unsigned char)sh_spi_read(&ss->regs->tbr_rbr);
	}
	sh_spi_write(0, &ss->regs->cr3);

	return 0;
}

int  spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct sh_spi *ss = to_sh_spi(slave);
	const unsigned char *tx_data = dout;
	unsigned char *rx_data = din;
	unsigned int len = bitlen / 8;
	int ret = 0;

	if (flags & SPI_XFER_BEGIN)
		sh_spi_write(sh_spi_read(&ss->regs->cr1) & ~SH_SPI_SSA,
				&ss->regs->cr1);

	if (tx_data)
		ret = sh_spi_send(ss, tx_data, len, flags);

	if (ret == 0 && rx_data)
		ret = sh_spi_receive(ss, rx_data, len, flags);

	if (flags & SPI_XFER_END) {
		sh_spi_set_bit(SH_SPI_SSD, &ss->regs->cr1);
		udelay(100);

		sh_spi_clear_bit(SH_SPI_SSA | SH_SPI_SSDB | SH_SPI_SSD,
				 &ss->regs->cr1);
		clear_fifo(ss);
	}

	return ret;
}

int  spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	/* This driver supports "bus = 0" and "cs = 0" only. */
	if (!bus && !cs)
		return 1;
	else
		return 0;
}

void spi_cs_activate(struct spi_slave *slave)
{

}

void spi_cs_deactivate(struct spi_slave *slave)
{

}
