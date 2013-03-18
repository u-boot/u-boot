/*
 * Xilinx SPI driver
 *
 * supports 8 bit SPI transfers only, with or w/o FIFO
 *
 * based on bfin_spi.c, by way of altera_spi.c
 * Copyright (c) 2005-2008 Analog Devices Inc.
 * Copyright (c) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Copyright (c) 2010 Graeme Smecher <graeme.smecher@mail.mcgill.ca>
 * Copyright (c) 2012 Stephan Linz <linz@li-pro.net>
 *
 * Licensed under the GPL-2 or later.
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [S]:	[0]/ip_documentation/xps_spi.pdf
 *	[0]/ip_documentation/axi_spi_ds742.pdf
 */
#include <config.h>
#include <common.h>
#include <malloc.h>
#include <spi.h>

#include "xilinx_spi.h"

#ifndef CONFIG_SYS_XILINX_SPI_LIST
#define CONFIG_SYS_XILINX_SPI_LIST	{ CONFIG_SYS_SPI_BASE }
#endif

#ifndef CONFIG_XILINX_SPI_IDLE_VAL
#define CONFIG_XILINX_SPI_IDLE_VAL	0xff
#endif

#define XILSPI_SPICR_DFLT_ON		(SPICR_MANUAL_SS | \
					 SPICR_MASTER_MODE | \
					 SPICR_SPE)

#define XILSPI_SPICR_DFLT_OFF		(SPICR_MASTER_INHIBIT | \
					 SPICR_MANUAL_SS)

#define XILSPI_MAX_XFER_BITS		8

static unsigned long xilinx_spi_base_list[] = CONFIG_SYS_XILINX_SPI_LIST;

__attribute__((weak))
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus < ARRAY_SIZE(xilinx_spi_base_list) && cs < 32;
}

__attribute__((weak))
void spi_cs_activate(struct spi_slave *slave)
{
	struct xilinx_spi_slave *xilspi = to_xilinx_spi_slave(slave);

	writel(SPISSR_ACT(slave->cs), &xilspi->regs->spissr);
}

__attribute__((weak))
void spi_cs_deactivate(struct spi_slave *slave)
{
	struct xilinx_spi_slave *xilspi = to_xilinx_spi_slave(slave);

	writel(SPISSR_OFF, &xilspi->regs->spissr);
}

void spi_init(void)
{
	/* do nothing */
}

void spi_set_speed(struct spi_slave *slave, uint hz)
{
	/* xilinx spi core does not support programmable speed */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct xilinx_spi_slave *xilspi;

	if (!spi_cs_is_valid(bus, cs)) {
		printf("XILSPI error: %s: unsupported bus %d / cs %d\n",
				__func__, bus, cs);
		return NULL;
	}

	xilspi = spi_alloc_slave(struct xilinx_spi_slave, bus, cs);
	if (!xilspi) {
		printf("XILSPI error: %s: malloc of SPI structure failed\n",
				__func__);
		return NULL;
	}
	xilspi->regs = (struct xilinx_spi_reg *)xilinx_spi_base_list[bus];
	xilspi->freq = max_hz;
	xilspi->mode = mode;
	debug("%s: bus:%i cs:%i base:%p mode:%x max_hz:%d\n", __func__,
		bus, cs, xilspi->regs, xilspi->mode, xilspi->freq);

	writel(SPISSR_RESET_VALUE, &xilspi->regs->srr);

	return &xilspi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct xilinx_spi_slave *xilspi = to_xilinx_spi_slave(slave);

	free(xilspi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct xilinx_spi_slave *xilspi = to_xilinx_spi_slave(slave);
	u32 spicr;

	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);
	writel(SPISSR_OFF, &xilspi->regs->spissr);

	spicr = XILSPI_SPICR_DFLT_ON;
	if (xilspi->mode & SPI_LSB_FIRST)
		spicr |= SPICR_LSB_FIRST;
	if (xilspi->mode & SPI_CPHA)
		spicr |= SPICR_CPHA;
	if (xilspi->mode & SPI_CPOL)
		spicr |= SPICR_CPOL;
	if (xilspi->mode & SPI_LOOP)
		spicr |= SPICR_LOOP;

	writel(spicr, &xilspi->regs->spicr);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct xilinx_spi_slave *xilspi = to_xilinx_spi_slave(slave);

	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);
	writel(SPISSR_OFF, &xilspi->regs->spissr);
	writel(XILSPI_SPICR_DFLT_OFF, &xilspi->regs->spicr);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct xilinx_spi_slave *xilspi = to_xilinx_spi_slave(slave);
	/* assume spi core configured to do 8 bit transfers */
	unsigned int bytes = bitlen / XILSPI_MAX_XFER_BITS;
	const unsigned char *txp = dout;
	unsigned char *rxp = din;
	unsigned rxecount = 17;	/* max. 16 elements in FIFO, leftover 1 */

	debug("%s: bus:%i cs:%i bitlen:%i bytes:%i flags:%lx\n", __func__,
		slave->bus, slave->cs, bitlen, bytes, flags);
	if (bitlen == 0)
		goto done;

	if (bitlen % XILSPI_MAX_XFER_BITS) {
		printf("XILSPI warning: %s: Not a multiple of %d bits\n",
				__func__, XILSPI_MAX_XFER_BITS);
		flags |= SPI_XFER_END;
		goto done;
	}

	/* empty read buffer */
	while (rxecount && !(readl(&xilspi->regs->spisr) & SPISR_RX_EMPTY)) {
		readl(&xilspi->regs->spidrr);
		rxecount--;
	}

	if (!rxecount) {
		printf("XILSPI error: %s: Rx buffer not empty\n", __func__);
		return -1;
	}

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	while (bytes--) {
		unsigned timeout = /* at least 1usec or greater, leftover 1 */
			xilspi->freq > XILSPI_MAX_XFER_BITS * 1000000 ? 2 :
			(XILSPI_MAX_XFER_BITS * 1000000 / xilspi->freq) + 1;

		/* get Tx element from data out buffer and count up */
		unsigned char d = txp ? *txp++ : CONFIG_XILINX_SPI_IDLE_VAL;
		debug("%s: tx:%x ", __func__, d);

		/* write out and wait for processing (receive data) */
		writel(d & SPIDTR_8BIT_MASK, &xilspi->regs->spidtr);
		while (timeout && readl(&xilspi->regs->spisr)
						& SPISR_RX_EMPTY) {
			timeout--;
			udelay(1);
		}

		if (!timeout) {
			printf("XILSPI error: %s: Xfer timeout\n", __func__);
			return -1;
		}

		/* read Rx element and push into data in buffer */
		d = readl(&xilspi->regs->spidrr) & SPIDRR_8BIT_MASK;
		if (rxp)
			*rxp++ = d;
		debug("rx:%x\n", d);
	}

 done:
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
