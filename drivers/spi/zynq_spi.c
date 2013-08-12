/*
 * (C) Copyright 2013 Inc.
 *
 * Xilinx Zynq PS SPI controller driver (master mode only)
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

/* zynq spi register bit masks ZYNQ_SPI_<REG>_<BIT>_MASK */
#define ZYNQ_SPI_CR_MSA_MASK		(1 << 15)	/* Manual start enb */
#define ZYNQ_SPI_CR_MCS_MASK		(1 << 14)	/* Manual chip select */
#define ZYNQ_SPI_CR_CS_MASK		(0xF << 10)	/* Chip select */
#define ZYNQ_SPI_CR_BRD_MASK		(0x7 << 3)	/* Baud rate div */
#define ZYNQ_SPI_CR_CPHA_MASK		(1 << 2)	/* Clock phase */
#define ZYNQ_SPI_CR_CPOL_MASK		(1 << 1)	/* Clock polarity */
#define ZYNQ_SPI_CR_MSTREN_MASK		(1 << 0)	/* Mode select */
#define ZYNQ_SPI_IXR_RXNEMPTY_MASK	(1 << 4)	/* RX_FIFO_not_empty */
#define ZYNQ_SPI_IXR_TXOW_MASK		(1 << 2)	/* TX_FIFO_not_full */
#define ZYNQ_SPI_IXR_ALL_MASK		0x7F		/* All IXR bits */
#define ZYNQ_SPI_ENR_SPI_EN_MASK	(1 << 0)	/* SPI Enable */

#define ZYNQ_SPI_FIFO_DEPTH		128
#ifndef CONFIG_SYS_ZYNQ_SPI_WAIT
#define CONFIG_SYS_ZYNQ_SPI_WAIT	(CONFIG_SYS_HZ/100)	/* 10 ms */
#endif

/* zynq spi register set */
struct zynq_spi_regs {
	u32 cr;		/* 0x00 */
	u32 isr;	/* 0x04 */
	u32 ier;	/* 0x08 */
	u32 idr;	/* 0x0C */
	u32 imr;	/* 0x10 */
	u32 enr;	/* 0x14 */
	u32 dr;		/* 0x18 */
	u32 txdr;	/* 0x1C */
	u32 rxdr;	/* 0x20 */
};

/* zynq spi slave */
struct zynq_spi_slave {
	struct spi_slave slave;
	struct zynq_spi_regs *base;
	u8 mode;
	u8 fifo_depth;
	u32 speed_hz;
	u32 input_hz;
	u32 req_hz;
};

static inline struct zynq_spi_slave *to_zynq_spi_slave(struct spi_slave *slave)
{
	return container_of(slave, struct zynq_spi_slave, slave);
}

static inline struct zynq_spi_regs *get_zynq_spi_base(int dev)
{
	if (dev)
		return (struct zynq_spi_regs *)ZYNQ_SPI_BASEADDR1;
	else
		return (struct zynq_spi_regs *)ZYNQ_SPI_BASEADDR0;
}

static void zynq_spi_init_hw(struct zynq_spi_slave *zslave)
{
	u32 confr;

	/* Disable SPI */
	writel(~ZYNQ_SPI_ENR_SPI_EN_MASK, &zslave->base->enr);

	/* Disable Interrupts */
	writel(ZYNQ_SPI_IXR_ALL_MASK, &zslave->base->idr);

	/* Clear RX FIFO */
	while (readl(&zslave->base->isr) &
			ZYNQ_SPI_IXR_RXNEMPTY_MASK)
		readl(&zslave->base->rxdr);

	/* Clear Interrupts */
	writel(ZYNQ_SPI_IXR_ALL_MASK, &zslave->base->isr);

	/* Manual slave select and Auto start */
	confr = ZYNQ_SPI_CR_MCS_MASK | ZYNQ_SPI_CR_CS_MASK |
		ZYNQ_SPI_CR_MSTREN_MASK;
	confr &= ~ZYNQ_SPI_CR_MSA_MASK;
	writel(confr, &zslave->base->cr);

	/* Enable SPI */
	writel(ZYNQ_SPI_ENR_SPI_EN_MASK, &zslave->base->enr);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	/* 2 bus with 3 chipselect */
	return bus < 2 && cs < 3;
}

void spi_cs_activate(struct spi_slave *slave)
{
	struct zynq_spi_slave *zslave = to_zynq_spi_slave(slave);
	u32 cr;

	debug("spi_cs_activate: 0x%08x\n", (u32)slave);

	clrbits_le32(&zslave->base->cr, ZYNQ_SPI_CR_CS_MASK);
	cr = readl(&zslave->base->cr);
	/*
	 * CS cal logic: CS[13:10]
	 * xxx0	- cs0
	 * xx01	- cs1
	 * x011 - cs2
	 */
	cr |= (~(0x1 << slave->cs) << 10) & ZYNQ_SPI_CR_CS_MASK;
	writel(cr, &zslave->base->cr);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct zynq_spi_slave *zslave = to_zynq_spi_slave(slave);

	debug("spi_cs_deactivate: 0x%08x\n", (u32)slave);

	setbits_le32(&zslave->base->cr, ZYNQ_SPI_CR_CS_MASK);
}

void spi_init()
{
	/* nothing to do */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct zynq_spi_slave *zslave;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	zslave = spi_alloc_slave(struct zynq_spi_slave, bus, cs);
	if (!zslave) {
		printf("SPI_error: Fail to allocate zynq_spi_slave\n");
		return NULL;
	}

	zslave->base = get_zynq_spi_base(bus);
	zslave->mode = mode;
	zslave->fifo_depth = ZYNQ_SPI_FIFO_DEPTH;
	zslave->input_hz = 166666700;
	zslave->speed_hz = zslave->input_hz / 2;
	zslave->req_hz = max_hz;

	/* init the zynq spi hw */
	zynq_spi_init_hw(zslave);

	return &zslave->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct zynq_spi_slave *zslave = to_zynq_spi_slave(slave);

	debug("spi_free_slave: 0x%08x\n", (u32)slave);
	free(zslave);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct zynq_spi_slave *zslave = to_zynq_spi_slave(slave);
	u32 confr = 0;
	u8 baud_rate_val = 0;

	writel(~ZYNQ_SPI_ENR_SPI_EN_MASK, &zslave->base->enr);

	/* Set the SPI Clock phase and polarities */
	confr = readl(&zslave->base->cr);
	confr &= ~(ZYNQ_SPI_CR_CPHA_MASK | ZYNQ_SPI_CR_CPOL_MASK);
	if (zslave->mode & SPI_CPHA)
		confr |= ZYNQ_SPI_CR_CPHA_MASK;
	if (zslave->mode & SPI_CPOL)
		confr |= ZYNQ_SPI_CR_CPOL_MASK;

	/* Set the clock frequency */
	if (zslave->req_hz == 0) {
		/* Set baudrate x8, if the req_hz is 0 */
		baud_rate_val = 0x2;
	} else if (zslave->speed_hz != zslave->req_hz) {
		while ((baud_rate_val < 8) &&
				((zslave->input_hz /
				(2 << baud_rate_val)) > zslave->req_hz))
			baud_rate_val++;
		zslave->speed_hz = zslave->req_hz / (2 << baud_rate_val);
	}
	confr &= ~ZYNQ_SPI_CR_BRD_MASK;
	confr |= (baud_rate_val << 3);
	writel(confr, &zslave->base->cr);

	writel(ZYNQ_SPI_ENR_SPI_EN_MASK, &zslave->base->enr);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct zynq_spi_slave *zslave = to_zynq_spi_slave(slave);

	debug("spi_release_bus: 0x%08x\n", (u32)slave);
	writel(~ZYNQ_SPI_ENR_SPI_EN_MASK, &zslave->base->enr);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct zynq_spi_slave *zslave = to_zynq_spi_slave(slave);
	u32 len = bitlen / 8;
	u32 tx_len = len, rx_len = len, tx_tvl;
	const u8 *tx_buf = dout;
	u8 *rx_buf = din, buf;
	u32 ts, status;

	debug("spi_xfer: bus:%i cs:%i bitlen:%i len:%i flags:%lx\n",
	      slave->bus, slave->cs, bitlen, len, flags);

	if (bitlen == 0)
		return -1;

	if (bitlen % 8) {
		debug("spi_xfer: Non byte aligned SPI transfer\n");
		return -1;
	}

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	while (rx_len > 0) {
		/* Write the data into TX FIFO - tx threshold is fifo_depth */
		tx_tvl = 0;
		while ((tx_tvl < zslave->fifo_depth) && tx_len) {
			if (tx_buf)
				buf = *tx_buf++;
			else
				buf = 0;
			writel(buf, &zslave->base->txdr);
			tx_len--;
			tx_tvl++;
		}

		/* Check TX FIFO completion */
		ts = get_timer(0);
		status = readl(&zslave->base->isr);
		while (!(status & ZYNQ_SPI_IXR_TXOW_MASK)) {
			if (get_timer(ts) > CONFIG_SYS_ZYNQ_SPI_WAIT) {
				printf("spi_xfer: Timeout! TX FIFO not full\n");
				return -1;
			}
			status = readl(&zslave->base->isr);
		}

		/* Read the data from RX FIFO */
		status = readl(&zslave->base->isr);
		while (status & ZYNQ_SPI_IXR_RXNEMPTY_MASK) {
			buf = readl(&zslave->base->rxdr);
			if (rx_buf)
				*rx_buf++ = buf;
			status = readl(&zslave->base->isr);
			rx_len--;
		}
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
