/*
 * Copyright (C) 2007 Atmel Corporation
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
#include <spi.h>
#include <malloc.h>

#include <asm/io.h>

#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>

#include "atmel_spi.h"

void spi_init()
{

}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
			unsigned int max_hz, unsigned int mode)
{
	struct atmel_spi_slave	*as;
	unsigned int		scbr;
	u32			csrx;
	void			*regs;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	switch (bus) {
	case 0:
		regs = (void *)ATMEL_BASE_SPI0;
		break;
#ifdef ATMEL_BASE_SPI1
	case 1:
		regs = (void *)ATMEL_BASE_SPI1;
		break;
#endif
#ifdef ATMEL_BASE_SPI2
	case 2:
		regs = (void *)ATMEL_BASE_SPI2;
		break;
#endif
#ifdef ATMEL_BASE_SPI3
	case 3:
		regs = (void *)ATMEL_BASE_SPI3;
		break;
#endif
	default:
		return NULL;
	}


	scbr = (get_spi_clk_rate(bus) + max_hz - 1) / max_hz;
	if (scbr > ATMEL_SPI_CSRx_SCBR_MAX)
		/* Too low max SCK rate */
		return NULL;
	if (scbr < 1)
		scbr = 1;

	csrx = ATMEL_SPI_CSRx_SCBR(scbr);
	csrx |= ATMEL_SPI_CSRx_BITS(ATMEL_SPI_BITS_8);
	if (!(mode & SPI_CPHA))
		csrx |= ATMEL_SPI_CSRx_NCPHA;
	if (mode & SPI_CPOL)
		csrx |= ATMEL_SPI_CSRx_CPOL;

	as = malloc(sizeof(struct atmel_spi_slave));
	if (!as)
		return NULL;

	as->slave.bus = bus;
	as->slave.cs = cs;
	as->regs = regs;
	as->mr = ATMEL_SPI_MR_MSTR | ATMEL_SPI_MR_MODFDIS
			| ATMEL_SPI_MR_PCS(~(1 << cs) & 0xf);
	spi_writel(as, CSR(cs), csrx);

	return &as->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct atmel_spi_slave *as = to_atmel_spi(slave);

	free(as);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct atmel_spi_slave *as = to_atmel_spi(slave);

	/* Enable the SPI hardware */
	spi_writel(as, CR, ATMEL_SPI_CR_SPIEN);

	/*
	 * Select the slave. This should set SCK to the correct
	 * initial state, etc.
	 */
	spi_writel(as, MR, as->mr);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct atmel_spi_slave *as = to_atmel_spi(slave);

	/* Disable the SPI hardware */
	spi_writel(as, CR, ATMEL_SPI_CR_SPIDIS);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct atmel_spi_slave *as = to_atmel_spi(slave);
	unsigned int	len_tx;
	unsigned int	len_rx;
	unsigned int	len;
	u32		status;
	const u8	*txp = dout;
	u8		*rxp = din;
	u8		value;

	if (bitlen == 0)
		/* Finish any previously submitted transfers */
		goto out;

	/*
	 * TODO: The controller can do non-multiple-of-8 bit
	 * transfers, but this driver currently doesn't support it.
	 *
	 * It's also not clear how such transfers are supposed to be
	 * represented as a stream of bytes...this is a limitation of
	 * the current SPI interface.
	 */
	if (bitlen % 8) {
		/* Errors always terminate an ongoing transfer */
		flags |= SPI_XFER_END;
		goto out;
	}

	len = bitlen / 8;

	/*
	 * The controller can do automatic CS control, but it is
	 * somewhat quirky, and it doesn't really buy us much anyway
	 * in the context of U-Boot.
	 */
	if (flags & SPI_XFER_BEGIN) {
		spi_cs_activate(slave);
		/*
		 * sometimes the RDR is not empty when we get here,
		 * in theory that should not happen, but it DOES happen.
		 * Read it here to be on the safe side.
		 * That also clears the OVRES flag. Required if the
		 * following loop exits due to OVRES!
		 */
		spi_readl(as, RDR);
	}

	for (len_tx = 0, len_rx = 0; len_rx < len; ) {
		status = spi_readl(as, SR);

		if (status & ATMEL_SPI_SR_OVRES)
			return -1;

		if (len_tx < len && (status & ATMEL_SPI_SR_TDRE)) {
			if (txp)
				value = *txp++;
			else
				value = 0;
			spi_writel(as, TDR, value);
			len_tx++;
		}
		if (status & ATMEL_SPI_SR_RDRF) {
			value = spi_readl(as, RDR);
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
			status = spi_readl(as, SR);
		} while (!(status & ATMEL_SPI_SR_TXEMPTY));

		spi_cs_deactivate(slave);
	}

	return 0;
}
