/*
 * SH QSPI (Quad SPI) driver
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <console.h>
#include <malloc.h>
#include <spi.h>
#include <asm/arch/rmobile.h>
#include <asm/io.h>

/* SH QSPI register bit masks <REG>_<BIT> */
#define SPCR_MSTR	0x08
#define SPCR_SPE	0x40
#define SPSR_SPRFF	0x80
#define SPSR_SPTEF	0x20
#define SPPCR_IO3FV	0x04
#define SPPCR_IO2FV	0x02
#define SPPCR_IO1FV	0x01
#define SPBDCR_RXBC0	BIT(0)
#define SPCMD_SCKDEN	BIT(15)
#define SPCMD_SLNDEN	BIT(14)
#define SPCMD_SPNDEN	BIT(13)
#define SPCMD_SSLKP	BIT(7)
#define SPCMD_BRDV0	BIT(2)
#define SPCMD_INIT1	SPCMD_SCKDEN | SPCMD_SLNDEN | \
			SPCMD_SPNDEN | SPCMD_SSLKP | \
			SPCMD_BRDV0
#define SPCMD_INIT2	SPCMD_SPNDEN | SPCMD_SSLKP | \
			SPCMD_BRDV0
#define SPBFCR_TXRST	BIT(7)
#define SPBFCR_RXRST	BIT(6)

/* SH QSPI register set */
struct sh_qspi_regs {
	unsigned char spcr;
	unsigned char sslp;
	unsigned char sppcr;
	unsigned char spsr;
	unsigned long spdr;
	unsigned char spscr;
	unsigned char spssr;
	unsigned char spbr;
	unsigned char spdcr;
	unsigned char spckd;
	unsigned char sslnd;
	unsigned char spnd;
	unsigned char dummy0;
	unsigned short spcmd0;
	unsigned short spcmd1;
	unsigned short spcmd2;
	unsigned short spcmd3;
	unsigned char spbfcr;
	unsigned char dummy1;
	unsigned short spbdcr;
	unsigned long spbmul0;
	unsigned long spbmul1;
	unsigned long spbmul2;
	unsigned long spbmul3;
};

struct sh_qspi_slave {
	struct spi_slave	slave;
	struct sh_qspi_regs	*regs;
};

static inline struct sh_qspi_slave *to_sh_qspi(struct spi_slave *slave)
{
	return container_of(slave, struct sh_qspi_slave, slave);
}

static void sh_qspi_init(struct sh_qspi_slave *ss)
{
	/* QSPI initialize */
	/* Set master mode only */
	writeb(SPCR_MSTR, &ss->regs->spcr);

	/* Set SSL signal level */
	writeb(0x00, &ss->regs->sslp);

	/* Set MOSI signal value when transfer is in idle state */
	writeb(SPPCR_IO3FV|SPPCR_IO2FV, &ss->regs->sppcr);

	/* Set bit rate. See 58.3.8 Quad Serial Peripheral Interface */
	writeb(0x01, &ss->regs->spbr);

	/* Disable Dummy Data Transmission */
	writeb(0x00, &ss->regs->spdcr);

	/* Set clock delay value */
	writeb(0x00, &ss->regs->spckd);

	/* Set SSL negation delay value */
	writeb(0x00, &ss->regs->sslnd);

	/* Set next-access delay value */
	writeb(0x00, &ss->regs->spnd);

	/* Set equence command */
	writew(SPCMD_INIT2, &ss->regs->spcmd0);

	/* Reset transfer and receive Buffer */
	setbits_8(&ss->regs->spbfcr, SPBFCR_TXRST|SPBFCR_RXRST);

	/* Clear transfer and receive Buffer control bit */
	clrbits_8(&ss->regs->spbfcr, SPBFCR_TXRST|SPBFCR_RXRST);

	/* Set equence control method. Use equence0 only */
	writeb(0x00, &ss->regs->spscr);

	/* Enable SPI function */
	setbits_8(&ss->regs->spcr, SPCR_SPE);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return 1;
}

void spi_cs_activate(struct spi_slave *slave)
{
	struct sh_qspi_slave *ss = to_sh_qspi(slave);

	/* Set master mode only */
	writeb(SPCR_MSTR, &ss->regs->spcr);

	/* Set command */
	writew(SPCMD_INIT1, &ss->regs->spcmd0);

	/* Reset transfer and receive Buffer */
	setbits_8(&ss->regs->spbfcr, SPBFCR_TXRST|SPBFCR_RXRST);

	/* Clear transfer and receive Buffer control bit */
	clrbits_8(&ss->regs->spbfcr, SPBFCR_TXRST|SPBFCR_RXRST);

	/* Set equence control method. Use equence0 only */
	writeb(0x00, &ss->regs->spscr);

	/* Enable SPI function */
	setbits_8(&ss->regs->spcr, SPCR_SPE);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct sh_qspi_slave *ss = to_sh_qspi(slave);

	/* Disable SPI Function */
	clrbits_8(&ss->regs->spcr, SPCR_SPE);
}

void spi_init(void)
{
	/* nothing to do */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct sh_qspi_slave *ss;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	ss = spi_alloc_slave(struct sh_qspi_slave, bus, cs);
	if (!ss) {
		printf("SPI_error: Fail to allocate sh_qspi_slave\n");
		return NULL;
	}

	ss->regs = (struct sh_qspi_regs *)SH_QSPI_BASE;

	/* Init SH QSPI */
	sh_qspi_init(ss);

	return &ss->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct sh_qspi_slave *spi = to_sh_qspi(slave);

	free(spi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct sh_qspi_slave *ss = to_sh_qspi(slave);
	unsigned long nbyte;
	int ret = 0;
	unsigned char dtdata = 0, drdata;
	unsigned char *tdata = &dtdata, *rdata = &drdata;
	unsigned long *spbmul0 = &ss->regs->spbmul0;

	if (dout == NULL && din == NULL) {
		if (flags & SPI_XFER_END)
			spi_cs_deactivate(slave);
		return 0;
	}

	if (bitlen % 8) {
		printf("%s: bitlen is not 8bit alined %d", __func__, bitlen);
		return 1;
	}

	nbyte = bitlen / 8;

	if (flags & SPI_XFER_BEGIN) {
		spi_cs_activate(slave);

		/* Set 1048576 byte */
		writel(0x100000, spbmul0);
	}

	if (flags & SPI_XFER_END)
		writel(nbyte, spbmul0);

	if (dout != NULL)
		tdata = (unsigned char *)dout;

	if (din != NULL)
		rdata = din;

	while (nbyte > 0) {
		while (!(readb(&ss->regs->spsr) & SPSR_SPTEF)) {
			if (ctrlc()) {
				puts("abort\n");
				return 1;
			}
			udelay(10);
		}

		writeb(*tdata, (unsigned char *)(&ss->regs->spdr));

		while ((readw(&ss->regs->spbdcr) != SPBDCR_RXBC0)) {
			if (ctrlc()) {
				puts("abort\n");
				return 1;
			}
			udelay(1);
		}

		while (!(readb(&ss->regs->spsr) & SPSR_SPRFF)) {
			if (ctrlc()) {
				puts("abort\n");
				return 1;
			}
			udelay(10);
		}

		*rdata = readb((unsigned char *)(&ss->regs->spdr));

		if (dout != NULL)
			tdata++;
		if (din != NULL)
			rdata++;

		nbyte--;
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return ret;
}
