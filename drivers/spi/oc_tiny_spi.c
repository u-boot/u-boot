/*
 * Opencore tiny_spi driver
 *
 * http://opencores.org/project,tiny_spi
 *
 * based on bfin_spi.c
 * Copyright (c) 2005-2008 Analog Devices Inc.
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <spi.h>
#include <asm/gpio.h>

#define TINY_SPI_STATUS_TXE 0x1
#define TINY_SPI_STATUS_TXR 0x2

struct tiny_spi_regs {
	unsigned rxdata;	/* Rx data reg */
	unsigned txdata;	/* Tx data reg */
	unsigned status;	/* Status reg */
	unsigned control;	/* Control reg */
	unsigned baud;		/* Baud reg */
};

struct tiny_spi_host {
	uint base;
	uint freq;
	uint baudwidth;
};
static const struct tiny_spi_host tiny_spi_host_list[] =
	CONFIG_SYS_TINY_SPI_LIST;

struct tiny_spi_slave {
	struct spi_slave slave;
	const struct tiny_spi_host *host;
	uint mode;
	uint baud;
	uint flg;
};
#define to_tiny_spi_slave(s) container_of(s, struct tiny_spi_slave, slave)

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus < ARRAY_SIZE(tiny_spi_host_list) && gpio_is_valid(cs);
}

void spi_cs_activate(struct spi_slave *slave)
{
	struct tiny_spi_slave *tiny_spi = to_tiny_spi_slave(slave);
	unsigned int cs = slave->cs;

	gpio_set_value(cs, tiny_spi->flg);
	debug("%s: SPI_CS_GPIO:%x\n", __func__, gpio_get_value(cs));
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct tiny_spi_slave *tiny_spi = to_tiny_spi_slave(slave);
	unsigned int cs = slave->cs;

	gpio_set_value(cs, !tiny_spi->flg);
	debug("%s: SPI_CS_GPIO:%x\n", __func__, gpio_get_value(cs));
}

void spi_set_speed(struct spi_slave *slave, uint hz)
{
	struct tiny_spi_slave *tiny_spi = to_tiny_spi_slave(slave);
	const struct tiny_spi_host *host = tiny_spi->host;

	tiny_spi->baud = min(DIV_ROUND_UP(host->freq, hz * 2),
			     (1 << host->baudwidth)) - 1;
	debug("%s: speed %u actual %u\n", __func__, hz,
	      host->freq / ((tiny_spi->baud + 1) * 2));
}

void spi_init(void)
{
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int hz, unsigned int mode)
{
	struct tiny_spi_slave *tiny_spi;

	if (!spi_cs_is_valid(bus, cs) || gpio_request(cs, "tiny_spi"))
		return NULL;

	tiny_spi = malloc(sizeof(*tiny_spi));
	if (!tiny_spi)
		return NULL;
	memset(tiny_spi, 0, sizeof(*tiny_spi));

	tiny_spi->slave.bus = bus;
	tiny_spi->slave.cs = cs;
	tiny_spi->host = &tiny_spi_host_list[bus];
	tiny_spi->mode = mode & (SPI_CPOL | SPI_CPHA);
	tiny_spi->flg = mode & SPI_CS_HIGH ? 1 : 0;
	spi_set_speed(&tiny_spi->slave, hz);

	debug("%s: bus:%i cs:%i base:%lx\n", __func__,
		bus, cs, tiny_spi->host->base);
	return &tiny_spi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct tiny_spi_slave *tiny_spi = to_tiny_spi_slave(slave);

	gpio_free(slave->cs);
	free(tiny_spi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct tiny_spi_slave *tiny_spi = to_tiny_spi_slave(slave);
	struct tiny_spi_regs *regs = (void *)tiny_spi->host->base;

	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);
	gpio_direction_output(slave->cs, !tiny_spi->flg);
	writel(tiny_spi->mode, &regs->control);
	writel(tiny_spi->baud, &regs->baud);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);
}

#ifndef CONFIG_TINY_SPI_IDLE_VAL
# define CONFIG_TINY_SPI_IDLE_VAL 0xff
#endif

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct tiny_spi_slave *tiny_spi = to_tiny_spi_slave(slave);
	struct tiny_spi_regs *regs = (void *)tiny_spi->host->base;
	const u8 *txp = dout;
	u8 *rxp = din;
	uint bytes = bitlen / 8;
	uint i;

	debug("%s: bus:%i cs:%i bitlen:%i bytes:%i flags:%lx\n", __func__,
		slave->bus, slave->cs, bitlen, bytes, flags);
	if (bitlen == 0)
		goto done;

	/* assume to do 8 bits transfers */
	if (bitlen % 8) {
		flags |= SPI_XFER_END;
		goto done;
	}

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	/* we need to tighten the transfer loop */
	if (txp && rxp) {
		writeb(*txp++, &regs->txdata);
		if (bytes > 1) {
			writeb(*txp++, &regs->txdata);
			for (i = 2; i < bytes; i++) {
				u8 rx, tx = *txp++;
				while (!(readb(&regs->status) &
					 TINY_SPI_STATUS_TXR))
					;
				rx = readb(&regs->txdata);
				writeb(tx, &regs->txdata);
				*rxp++ = rx;
			}
			while (!(readb(&regs->status) &
				 TINY_SPI_STATUS_TXR))
				;
			*rxp++ = readb(&regs->txdata);
		}
		while (!(readb(&regs->status) &
			 TINY_SPI_STATUS_TXE))
			;
		*rxp++ = readb(&regs->rxdata);
	} else if (rxp) {
		writeb(CONFIG_TINY_SPI_IDLE_VAL, &regs->txdata);
		if (bytes > 1) {
			writeb(CONFIG_TINY_SPI_IDLE_VAL,
			       &regs->txdata);
			for (i = 2; i < bytes; i++) {
				u8 rx;
				while (!(readb(&regs->status) &
					 TINY_SPI_STATUS_TXR))
					;
				rx = readb(&regs->txdata);
				writeb(CONFIG_TINY_SPI_IDLE_VAL,
				       &regs->txdata);
				*rxp++ = rx;
			}
			while (!(readb(&regs->status) &
				 TINY_SPI_STATUS_TXR))
				;
			*rxp++ = readb(&regs->txdata);
		}
		while (!(readb(&regs->status) &
			 TINY_SPI_STATUS_TXE))
			;
		*rxp++ = readb(&regs->rxdata);
	} else if (txp) {
		writeb(*txp++, &regs->txdata);
		if (bytes > 1) {
			writeb(*txp++, &regs->txdata);
			for (i = 2; i < bytes; i++) {
				u8 tx = *txp++;
				while (!(readb(&regs->status) &
					 TINY_SPI_STATUS_TXR))
					;
				writeb(tx, &regs->txdata);
			}
		}
		while (!(readb(&regs->status) &
			 TINY_SPI_STATUS_TXE))
			;
	} else {
		writeb(CONFIG_TINY_SPI_IDLE_VAL, &regs->txdata);
		if (bytes > 1) {
			writeb(CONFIG_TINY_SPI_IDLE_VAL,
			       &regs->txdata);
			for (i = 2; i < bytes; i++) {
				while (!(readb(&regs->status) &
					 TINY_SPI_STATUS_TXR))
					;
				writeb(CONFIG_TINY_SPI_IDLE_VAL,
				       &regs->txdata);
			}
		}
		while (!(readb(&regs->status) &
			 TINY_SPI_STATUS_TXE))
			;
	}

 done:
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
