/*
 * Altera SPI driver
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

#define ALTERA_SPI_RXDATA	0
#define ALTERA_SPI_TXDATA	4
#define ALTERA_SPI_STATUS	8
#define ALTERA_SPI_CONTROL	12
#define ALTERA_SPI_SLAVE_SEL	20

#define ALTERA_SPI_STATUS_ROE_MSK	(0x8)
#define ALTERA_SPI_STATUS_TOE_MSK	(0x10)
#define ALTERA_SPI_STATUS_TMT_MSK	(0x20)
#define ALTERA_SPI_STATUS_TRDY_MSK	(0x40)
#define ALTERA_SPI_STATUS_RRDY_MSK	(0x80)
#define ALTERA_SPI_STATUS_E_MSK	(0x100)

#define ALTERA_SPI_CONTROL_IROE_MSK	(0x8)
#define ALTERA_SPI_CONTROL_ITOE_MSK	(0x10)
#define ALTERA_SPI_CONTROL_ITRDY_MSK	(0x40)
#define ALTERA_SPI_CONTROL_IRRDY_MSK	(0x80)
#define ALTERA_SPI_CONTROL_IE_MSK	(0x100)
#define ALTERA_SPI_CONTROL_SSO_MSK	(0x400)

#ifndef CONFIG_SYS_ALTERA_SPI_LIST
#define CONFIG_SYS_ALTERA_SPI_LIST { CONFIG_SYS_SPI_BASE }
#endif

static ulong altera_spi_base_list[] = CONFIG_SYS_ALTERA_SPI_LIST;

struct altera_spi_slave {
	struct spi_slave slave;
	ulong base;
};
#define to_altera_spi_slave(s) container_of(s, struct altera_spi_slave, slave)

__attribute__((weak))
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus < ARRAY_SIZE(altera_spi_base_list) && cs < 32;
}

__attribute__((weak))
void spi_cs_activate(struct spi_slave *slave)
{
	struct altera_spi_slave *altspi = to_altera_spi_slave(slave);
	writel(1 << slave->cs, altspi->base + ALTERA_SPI_SLAVE_SEL);
	writel(ALTERA_SPI_CONTROL_SSO_MSK, altspi->base + ALTERA_SPI_CONTROL);
}

__attribute__((weak))
void spi_cs_deactivate(struct spi_slave *slave)
{
	struct altera_spi_slave *altspi = to_altera_spi_slave(slave);
	writel(0, altspi->base + ALTERA_SPI_CONTROL);
	writel(0, altspi->base + ALTERA_SPI_SLAVE_SEL);
}

void spi_init(void)
{
}

void spi_set_speed(struct spi_slave *slave, uint hz)
{
	/* altera spi core does not support programmable speed */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct altera_spi_slave *altspi;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	altspi = spi_alloc_slave(struct altera_spi_slave, bus, cs);
	if (!altspi)
		return NULL;

	altspi->base = altera_spi_base_list[bus];
	debug("%s: bus:%i cs:%i base:%lx\n", __func__,
		bus, cs, altspi->base);

	return &altspi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct altera_spi_slave *altspi = to_altera_spi_slave(slave);
	free(altspi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct altera_spi_slave *altspi = to_altera_spi_slave(slave);

	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);
	writel(0, altspi->base + ALTERA_SPI_CONTROL);
	writel(0, altspi->base + ALTERA_SPI_SLAVE_SEL);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct altera_spi_slave *altspi = to_altera_spi_slave(slave);

	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);
	writel(0, altspi->base + ALTERA_SPI_SLAVE_SEL);
}

#ifndef CONFIG_ALTERA_SPI_IDLE_VAL
# define CONFIG_ALTERA_SPI_IDLE_VAL 0xff
#endif

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct altera_spi_slave *altspi = to_altera_spi_slave(slave);
	/* assume spi core configured to do 8 bit transfers */
	uint bytes = bitlen / 8;
	const uchar *txp = dout;
	uchar *rxp = din;

	debug("%s: bus:%i cs:%i bitlen:%i bytes:%i flags:%lx\n", __func__,
		slave->bus, slave->cs, bitlen, bytes, flags);
	if (bitlen == 0)
		goto done;

	if (bitlen % 8) {
		flags |= SPI_XFER_END;
		goto done;
	}

	/* empty read buffer */
	if (readl(altspi->base + ALTERA_SPI_STATUS) &
	    ALTERA_SPI_STATUS_RRDY_MSK)
		readl(altspi->base + ALTERA_SPI_RXDATA);
	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	while (bytes--) {
		uchar d = txp ? *txp++ : CONFIG_ALTERA_SPI_IDLE_VAL;
		debug("%s: tx:%x ", __func__, d);
		writel(d, altspi->base + ALTERA_SPI_TXDATA);
		while (!(readl(altspi->base + ALTERA_SPI_STATUS) &
			 ALTERA_SPI_STATUS_RRDY_MSK))
			;
		d = readl(altspi->base + ALTERA_SPI_RXDATA);
		if (rxp)
			*rxp++ = d;
		debug("rx:%x\n", d);
	}
 done:
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
