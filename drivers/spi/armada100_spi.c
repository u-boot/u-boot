/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
 *
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Based on SSP driver
 * Written-by: Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <common.h>
#include <malloc.h>
#include <spi.h>

#include <asm/io.h>
#include <asm/arch/spi.h>
#include <asm/gpio.h>

#define to_armd_spi_slave(s)	container_of(s, struct armd_spi_slave, slave)

struct armd_spi_slave {
	struct spi_slave slave;
	struct ssp_reg *spi_reg;
	u32 cr0, cr1;
	u32 int_cr1;
	u32 clear_sr;
	const void *tx;
	void *rx;
	int gpio_cs_inverted;
};

static int spi_armd_write(struct armd_spi_slave *pss)
{
	int wait_timeout = SSP_FLUSH_NUM;
	while (--wait_timeout && !(readl(&pss->spi_reg->sssr) & SSSR_TNF))
		;
	if (!wait_timeout) {
		debug("%s: timeout error\n", __func__);
		return -1;
	}

	if (pss->tx != NULL) {
		writel(*(u8 *)pss->tx, &pss->spi_reg->ssdr);
		++pss->tx;
	} else {
		writel(0, &pss->spi_reg->ssdr);
	}
	return 0;
}

static int spi_armd_read(struct armd_spi_slave *pss)
{
	int wait_timeout = SSP_FLUSH_NUM;
	while (--wait_timeout && !(readl(&pss->spi_reg->sssr) & SSSR_RNE))
		;
	if (!wait_timeout) {
		debug("%s: timeout error\n", __func__);
		return -1;
	}

	if (pss->rx != NULL) {
		*(u8 *)pss->rx = readl(&pss->spi_reg->ssdr);
		++pss->rx;
	} else {
		readl(&pss->spi_reg->ssdr);
	}
	return 0;
}

static int spi_armd_flush(struct armd_spi_slave *pss)
{
	unsigned long limit = SSP_FLUSH_NUM;

	do {
		while (readl(&pss->spi_reg->sssr) & SSSR_RNE)
			readl(&pss->spi_reg->ssdr);
	} while ((readl(&pss->spi_reg->sssr) & SSSR_BSY) && limit--);

	writel(SSSR_ROR, &pss->spi_reg->sssr);

	return limit;
}

void spi_cs_activate(struct spi_slave *slave)
{
	struct armd_spi_slave *pss = to_armd_spi_slave(slave);

	gpio_set_value(slave->cs, pss->gpio_cs_inverted);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct armd_spi_slave *pss = to_armd_spi_slave(slave);

	gpio_set_value(slave->cs, !pss->gpio_cs_inverted);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct armd_spi_slave *pss;

	pss = spi_alloc_slave(struct armd_spi_slave, bus, cs);
	if (!pss)
		return NULL;

	pss->spi_reg = (struct ssp_reg *)SSP_REG_BASE(CONFIG_SYS_SSP_PORT);

	pss->cr0 = SSCR0_MOTO | SSCR0_DATASIZE(DEFAULT_WORD_LEN) | SSCR0_SSE;

	pss->cr1 = (SSCR1_RXTRESH(RX_THRESH_DEF) & SSCR1_RFT) |
		(SSCR1_TXTRESH(TX_THRESH_DEF) & SSCR1_TFT);
	pss->cr1 &= ~(SSCR1_SPO | SSCR1_SPH);
	pss->cr1 |= (((mode & SPI_CPHA) != 0) ? SSCR1_SPH : 0)
		| (((mode & SPI_CPOL) != 0) ? SSCR1_SPO : 0);

	pss->int_cr1 = SSCR1_TIE | SSCR1_RIE | SSCR1_TINTE;
	pss->clear_sr = SSSR_ROR | SSSR_TINT;

	pss->gpio_cs_inverted = mode & SPI_CS_HIGH;
	gpio_set_value(cs, !pss->gpio_cs_inverted);

	return &pss->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct armd_spi_slave *pss = to_armd_spi_slave(slave);

	free(pss);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct armd_spi_slave *pss = to_armd_spi_slave(slave);

	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);
	if (spi_armd_flush(pss) == 0)
		return -1;

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct armd_spi_slave *pss = to_armd_spi_slave(slave);
	uint bytes = bitlen / 8;
	unsigned long limit;
	int ret = 0;

	if (bitlen == 0)
		goto done;

	/* we can only do 8 bit transfers */
	if (bitlen % 8) {
		flags |= SPI_XFER_END;
		goto done;
	}

	pss->tx = dout;
	pss->rx = din;

	if (flags & SPI_XFER_BEGIN) {
		spi_cs_activate(slave);
		writel(pss->cr1 | pss->int_cr1, &pss->spi_reg->sscr1);
		writel(TIMEOUT_DEF, &pss->spi_reg->ssto);
		writel(pss->cr0, &pss->spi_reg->sscr0);
	}

	while (bytes--) {
		limit = SSP_FLUSH_NUM;
		ret = spi_armd_write(pss);
		if (ret)
			break;

		while ((readl(&pss->spi_reg->sssr) & SSSR_BSY) && limit--)
			udelay(1);

		ret = spi_armd_read(pss);
		if (ret)
			break;
	}

 done:
	if (flags & SPI_XFER_END) {
		/* Stop SSP */
		writel(pss->clear_sr, &pss->spi_reg->sssr);
		clrbits_le32(&pss->spi_reg->sscr1, pss->int_cr1);
		writel(0, &pss->spi_reg->ssto);
		spi_cs_deactivate(slave);
	}

	return ret;
}
