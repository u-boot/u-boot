/*
 * Xilinx SPI driver
 *
 * Supports 8 bit SPI transfers only, with or w/o FIFO
 *
 * Based on bfin_spi.c, by way of altera_spi.c
 * Copyright (c) 2012 Stephan Linz <linz@li-pro.net>
 * Copyright (c) 2010 Graeme Smecher <graeme.smecher@mail.mcgill.ca>
 * Copyright (c) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <spi.h>

/*
 * [0]: http://www.xilinx.com/support/documentation
 *
 * Xilinx SPI Register Definitions
 * [1]:	[0]/ip_documentation/xps_spi.pdf
 *	page 8, Register Descriptions
 * [2]:	[0]/ip_documentation/axi_spi_ds742.pdf
 *	page 7, Register Overview Table
 */

/* SPI Control Register (spicr), [1] p9, [2] p8 */
#define SPICR_LSB_FIRST		(1 << 9)
#define SPICR_MASTER_INHIBIT	(1 << 8)
#define SPICR_MANUAL_SS		(1 << 7)
#define SPICR_RXFIFO_RESEST	(1 << 6)
#define SPICR_TXFIFO_RESEST	(1 << 5)
#define SPICR_CPHA		(1 << 4)
#define SPICR_CPOL		(1 << 3)
#define SPICR_MASTER_MODE	(1 << 2)
#define SPICR_SPE		(1 << 1)
#define SPICR_LOOP		(1 << 0)

/* SPI Status Register (spisr), [1] p11, [2] p10 */
#define SPISR_SLAVE_MODE_SELECT	(1 << 5)
#define SPISR_MODF		(1 << 4)
#define SPISR_TX_FULL		(1 << 3)
#define SPISR_TX_EMPTY		(1 << 2)
#define SPISR_RX_FULL		(1 << 1)
#define SPISR_RX_EMPTY		(1 << 0)

/* SPI Data Transmit Register (spidtr), [1] p12, [2] p12 */
#define SPIDTR_8BIT_MASK	(0xff << 0)
#define SPIDTR_16BIT_MASK	(0xffff << 0)
#define SPIDTR_32BIT_MASK	(0xffffffff << 0)

/* SPI Data Receive Register (spidrr), [1] p12, [2] p12 */
#define SPIDRR_8BIT_MASK	(0xff << 0)
#define SPIDRR_16BIT_MASK	(0xffff << 0)
#define SPIDRR_32BIT_MASK	(0xffffffff << 0)

/* SPI Slave Select Register (spissr), [1] p13, [2] p13 */
#define SPISSR_MASK(cs)		(1 << (cs))
#define SPISSR_ACT(cs)		~SPISSR_MASK(cs)
#define SPISSR_OFF		~0UL

/* SPI Software Reset Register (ssr) */
#define SPISSR_RESET_VALUE	0x0a

#define XILSPI_MAX_XFER_BITS	8
#define XILSPI_SPICR_DFLT_ON	(SPICR_MANUAL_SS | SPICR_MASTER_MODE | \
				SPICR_SPE)
#define XILSPI_SPICR_DFLT_OFF	(SPICR_MASTER_INHIBIT | SPICR_MANUAL_SS)

#ifndef CONFIG_XILINX_SPI_IDLE_VAL
#define CONFIG_XILINX_SPI_IDLE_VAL	0xff
#endif

#ifndef CONFIG_SYS_XILINX_SPI_LIST
#define CONFIG_SYS_XILINX_SPI_LIST	{ CONFIG_SYS_SPI_BASE }
#endif

/* xilinx spi register set */
struct xilinx_spi_reg {
	u32 __space0__[7];
	u32 dgier;	/* Device Global Interrupt Enable Register (DGIER) */
	u32 ipisr;	/* IP Interrupt Status Register (IPISR) */
	u32 __space1__;
	u32 ipier;	/* IP Interrupt Enable Register (IPIER) */
	u32 __space2__[5];
	u32 srr;	/* Softare Reset Register (SRR) */
	u32 __space3__[7];
	u32 spicr;	/* SPI Control Register (SPICR) */
	u32 spisr;	/* SPI Status Register (SPISR) */
	u32 spidtr;	/* SPI Data Transmit Register (SPIDTR) */
	u32 spidrr;	/* SPI Data Receive Register (SPIDRR) */
	u32 spissr;	/* SPI Slave Select Register (SPISSR) */
	u32 spitfor;	/* SPI Transmit FIFO Occupancy Register (SPITFOR) */
	u32 spirfor;	/* SPI Receive FIFO Occupancy Register (SPIRFOR) */
};

/* xilinx spi slave */
struct xilinx_spi_slave {
	struct spi_slave slave;
	struct xilinx_spi_reg *regs;
	unsigned int freq;
	unsigned int mode;
};

static inline struct xilinx_spi_slave *to_xilinx_spi_slave(
			struct spi_slave *slave)
{
	return container_of(slave, struct xilinx_spi_slave, slave);
}

static unsigned long xilinx_spi_base_list[] = CONFIG_SYS_XILINX_SPI_LIST;
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus < ARRAY_SIZE(xilinx_spi_base_list) && cs < 32;
}

void spi_cs_activate(struct spi_slave *slave)
{
	struct xilinx_spi_slave *xilspi = to_xilinx_spi_slave(slave);

	writel(SPISSR_ACT(slave->cs), &xilspi->regs->spissr);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct xilinx_spi_slave *xilspi = to_xilinx_spi_slave(slave);

	writel(SPISSR_OFF, &xilspi->regs->spissr);
}

void spi_init(void)
{
	/* do nothing */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct xilinx_spi_slave *xilspi;

	if (!spi_cs_is_valid(bus, cs)) {
		printf("XILSPI error: unsupported bus %d / cs %d\n", bus, cs);
		return NULL;
	}

	xilspi = spi_alloc_slave(struct xilinx_spi_slave, bus, cs);
	if (!xilspi) {
		printf("XILSPI error: malloc of SPI structure failed\n");
		return NULL;
	}
	xilspi->regs = (struct xilinx_spi_reg *)xilinx_spi_base_list[bus];
	xilspi->freq = max_hz;
	xilspi->mode = mode;
	debug("spi_setup_slave: bus:%i cs:%i base:%p mode:%x max_hz:%d\n",
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

	debug("spi_claim_bus: bus:%i cs:%i\n", slave->bus, slave->cs);
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

	debug("spi_release_bus: bus:%i cs:%i\n", slave->bus, slave->cs);
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
	unsigned global_timeout;

	debug("spi_xfer: bus:%i cs:%i bitlen:%i bytes:%i flags:%lx\n",
	      slave->bus, slave->cs, bitlen, bytes, flags);

	if (bitlen == 0)
		goto done;

	if (bitlen % XILSPI_MAX_XFER_BITS) {
		printf("XILSPI warning: Not a multiple of %d bits\n",
		       XILSPI_MAX_XFER_BITS);
		flags |= SPI_XFER_END;
		goto done;
	}

	/* empty read buffer */
	while (rxecount && !(readl(&xilspi->regs->spisr) & SPISR_RX_EMPTY)) {
		readl(&xilspi->regs->spidrr);
		rxecount--;
	}

	if (!rxecount) {
		printf("XILSPI error: Rx buffer not empty\n");
		return -1;
	}

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	/* at least 1usec or greater, leftover 1 */
	global_timeout = xilspi->freq > XILSPI_MAX_XFER_BITS * 1000000 ? 2 :
			(XILSPI_MAX_XFER_BITS * 1000000 / xilspi->freq) + 1;

	while (bytes--) {
		unsigned timeout = global_timeout;
		/* get Tx element from data out buffer and count up */
		unsigned char d = txp ? *txp++ : CONFIG_XILINX_SPI_IDLE_VAL;
		debug("spi_xfer: tx:%x ", d);

		/* write out and wait for processing (receive data) */
		writel(d & SPIDTR_8BIT_MASK, &xilspi->regs->spidtr);
		while (timeout && readl(&xilspi->regs->spisr)
						& SPISR_RX_EMPTY) {
			timeout--;
			udelay(1);
		}

		if (!timeout) {
			printf("XILSPI error: Xfer timeout\n");
			return -1;
		}

		/* read Rx element and push into data in buffer */
		d = readl(&xilspi->regs->spidrr) & SPIDRR_8BIT_MASK;
		if (rxp)
			*rxp++ = d;
		debug("spi_xfer: rx:%x\n", d);
	}

 done:
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
