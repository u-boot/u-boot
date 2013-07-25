/*
 * (C) Copyright 2009
 * Frank Bodammer <frank.bodammer@gcd-solutions.de>
 * (C) Copyright 2009 Semihalf, Grzegorz Bernacki
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <spi.h>
#include <mpc5xxx.h>

void spi_init(void)
{
	struct mpc5xxx_spi *spi = (struct mpc5xxx_spi *)MPC5XXX_SPI;
	/*
	 * Its important to use the correct order when initializing the
	 * registers
	 */
	out_8(&spi->ddr, 0x0F);	/* set all SPI pins as output */
	out_8(&spi->pdr, 0x00);	/* set SS low */
	/* SPI is master, SS is general purpose output */
	out_8(&spi->cr1, SPI_CR_MSTR | SPI_CR_SPE);
	out_8(&spi->cr2, 0x00);	/* normal operation */
	out_8(&spi->brr, 0x77);	/* baud rate: IPB clock / 2048 */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct spi_slave *slave;

	slave = spi_alloc_slave_base(bus, cs);
	if (!slave)
		return NULL;

	return slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	free(slave);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	return;
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct mpc5xxx_spi *spi = (struct mpc5xxx_spi *)MPC5XXX_SPI;
	int i, iter = bitlen >> 3;
	const uchar *txp = dout;
	uchar *rxp = din;

	debug("spi_xfer: slave %u:%u dout %08X din %08X bitlen %u\n",
	      slave->bus, slave->cs, *(uint *) dout, *(uint *) din, bitlen);

	if (flags & SPI_XFER_BEGIN)
		setbits_8(&spi->pdr, SPI_PDR_SS);

	for (i = 0; i < iter; i++) {
		udelay(1000);
		debug("spi_xfer: sending %x\n", txp[i]);
		out_8(&spi->dr, txp[i]);
		while (!(in_8(&spi->sr) & SPI_SR_SPIF)) {
			udelay(1000);
			if (in_8(&spi->sr) & SPI_SR_WCOL) {
				rxp[i] = in_8(&spi->dr);
				puts("spi_xfer: write collision\n");
				return -1;
			}
		}
		rxp[i] = in_8(&spi->dr);
		debug("spi_xfer: received %x\n", rxp[i]);
	}
	if (flags & SPI_XFER_END)
		clrbits_8(&spi->pdr, SPI_PDR_SS);

	return 0;
}
