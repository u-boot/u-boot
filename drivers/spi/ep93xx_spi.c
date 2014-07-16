/*
 * SPI Driver for EP93xx
 *
 * Copyright (C) 2013 Sergey Kostanabev <sergey.kostanbaev <at> fairwaves.ru>
 *
 * Inspired form linux kernel driver and atmel uboot driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spi.h>
#include <malloc.h>

#include <asm/io.h>

#include <asm/arch/ep93xx.h>


#define BIT(x)			(1<<(x))
#define SSPBASE			SPI_BASE

#define SSPCR0			0x0000
#define SSPCR0_MODE_SHIFT	6
#define SSPCR0_SCR_SHIFT	8
#define SSPCR0_SPH		BIT(7)
#define SSPCR0_SPO		BIT(6)
#define SSPCR0_FRF_SPI		0
#define SSPCR0_DSS_8BIT		7

#define SSPCR1			0x0004
#define SSPCR1_RIE		BIT(0)
#define SSPCR1_TIE		BIT(1)
#define SSPCR1_RORIE		BIT(2)
#define SSPCR1_LBM		BIT(3)
#define SSPCR1_SSE		BIT(4)
#define SSPCR1_MS		BIT(5)
#define SSPCR1_SOD		BIT(6)

#define SSPDR			0x0008

#define SSPSR			0x000c
#define SSPSR_TFE		BIT(0)
#define SSPSR_TNF		BIT(1)
#define SSPSR_RNE		BIT(2)
#define SSPSR_RFF		BIT(3)
#define SSPSR_BSY		BIT(4)
#define SSPCPSR			0x0010

#define SSPIIR			0x0014
#define SSPIIR_RIS		BIT(0)
#define SSPIIR_TIS		BIT(1)
#define SSPIIR_RORIS		BIT(2)
#define SSPICR			SSPIIR

#define SSPCLOCK		14745600
#define SSP_MAX_RATE		(SSPCLOCK / 2)
#define SSP_MIN_RATE		(SSPCLOCK / (254 * 256))

/* timeout in milliseconds */
#define SPI_TIMEOUT		5
/* maximum depth of RX/TX FIFO */
#define SPI_FIFO_SIZE		8

struct ep93xx_spi_slave {
	struct spi_slave slave;

	unsigned sspcr0;
	unsigned sspcpsr;
};

static inline struct ep93xx_spi_slave *to_ep93xx_spi(struct spi_slave *slave)
{
	return container_of(slave, struct ep93xx_spi_slave, slave);
}

void spi_init()
{
}

static inline void ep93xx_spi_write_u8(u16 reg, u8 value)
{
	writel(value, (unsigned int *)(SSPBASE + reg));
}

static inline u8 ep93xx_spi_read_u8(u16 reg)
{
	return readl((unsigned int *)(SSPBASE + reg));
}

static inline void ep93xx_spi_write_u16(u16 reg, u16 value)
{
	writel(value, (unsigned int *)(SSPBASE + reg));
}

static inline u16 ep93xx_spi_read_u16(u16 reg)
{
	return (u16)readl((unsigned int *)(SSPBASE + reg));
}

static int ep93xx_spi_init_hw(unsigned int rate, unsigned int mode,
				struct ep93xx_spi_slave *slave)
{
	unsigned cpsr, scr;

	if (rate > SSP_MAX_RATE)
		rate = SSP_MAX_RATE;

	if (rate < SSP_MIN_RATE)
		return -1;

	/* Calculate divisors so that we can get speed according the
	 * following formula:
	 *	rate = spi_clock_rate / (cpsr * (1 + scr))
	 *
	 * cpsr must be even number and starts from 2, scr can be any number
	 * between 0 and 255.
	 */
	for (cpsr = 2; cpsr <= 254; cpsr += 2) {
		for (scr = 0; scr <= 255; scr++) {
			if ((SSPCLOCK / (cpsr * (scr + 1))) <= rate) {
				/* Set CHPA and CPOL, SPI format and 8bit */
				unsigned sspcr0 = (scr << SSPCR0_SCR_SHIFT) |
					SSPCR0_FRF_SPI | SSPCR0_DSS_8BIT;
				if (mode & SPI_CPHA)
					sspcr0 |= SSPCR0_SPH;
				if (mode & SPI_CPOL)
					sspcr0 |= SSPCR0_SPO;

				slave->sspcr0 = sspcr0;
				slave->sspcpsr = cpsr;
				return 0;
			}
		}
	}

	return -1;
}

void spi_set_speed(struct spi_slave *slave, unsigned int hz)
{
	struct ep93xx_spi_slave *as = to_ep93xx_spi(slave);

	unsigned int mode = 0;
	if (as->sspcr0 & SSPCR0_SPH)
		mode |= SPI_CPHA;
	if (as->sspcr0 & SSPCR0_SPO)
		mode |= SPI_CPOL;

	ep93xx_spi_init_hw(hz, mode, as);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
			unsigned int max_hz, unsigned int mode)
{
	struct ep93xx_spi_slave	*as;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	as = spi_alloc_slave(struct ep93xx_spi_slave, bus, cs);
	if (!as)
		return NULL;

	if (ep93xx_spi_init_hw(max_hz, mode, as)) {
		free(as);
		return NULL;
	}

	return &as->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct ep93xx_spi_slave *as = to_ep93xx_spi(slave);

	free(as);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct ep93xx_spi_slave *as = to_ep93xx_spi(slave);

	/* Enable the SPI hardware */
	ep93xx_spi_write_u8(SSPCR1, SSPCR1_SSE);


	ep93xx_spi_write_u8(SSPCPSR, as->sspcpsr);
	ep93xx_spi_write_u16(SSPCR0, as->sspcr0);

	debug("Select CS:%d SSPCPSR=%02x SSPCR0=%04x\n",
	      slave->cs, as->sspcpsr, as->sspcr0);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	/* Disable the SPI hardware */
	ep93xx_spi_write_u8(SSPCR1, 0);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	unsigned int	len_tx;
	unsigned int	len_rx;
	unsigned int	len;
	u32		status;
	const u8	*txp = dout;
	u8		*rxp = din;
	u8		value;

	debug("spi_xfer: slave %u:%u dout %p din %p bitlen %u\n",
	      slave->bus, slave->cs, (uint *)dout, (uint *)din, bitlen);


	if (bitlen == 0)
		/* Finish any previously submitted transfers */
		goto out;

	if (bitlen % 8) {
		/* Errors always terminate an ongoing transfer */
		flags |= SPI_XFER_END;
		goto out;
	}

	len = bitlen / 8;


	if (flags & SPI_XFER_BEGIN) {
		/* Empty RX FIFO */
		while ((ep93xx_spi_read_u8(SSPSR) & SSPSR_RNE))
			ep93xx_spi_read_u8(SSPDR);

		spi_cs_activate(slave);
	}

	for (len_tx = 0, len_rx = 0; len_rx < len; ) {
		status = ep93xx_spi_read_u8(SSPSR);

		if ((len_tx < len) && (status & SSPSR_TNF)) {
			if (txp)
				value = *txp++;
			else
				value = 0xff;

			ep93xx_spi_write_u8(SSPDR, value);
			len_tx++;
		}

		if (status & SSPSR_RNE) {
			value = ep93xx_spi_read_u8(SSPDR);

			if (rxp)
				*rxp++ = value;
			len_rx++;
		}
	}

out:
	if (flags & SPI_XFER_END) {
		/*
		 * Wait until the transfer is completely done before
		 * we deactivate CS.
		 */
		do {
			status = ep93xx_spi_read_u8(SSPSR);
		} while (status & SSPSR_BSY);

		spi_cs_deactivate(slave);
	}

	return 0;
}
