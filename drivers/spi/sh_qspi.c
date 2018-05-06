// SPDX-License-Identifier: GPL-2.0
/*
 * SH QSPI (Quad SPI) driver
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 */

#include <common.h>
#include <console.h>
#include <malloc.h>
#include <spi.h>
#include <wait_bit.h>
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
#define SPBFCR_TXTRG	0x30
#define SPBFCR_RXTRG	0x07

/* SH QSPI register set */
struct sh_qspi_regs {
	u8	spcr;
	u8	sslp;
	u8	sppcr;
	u8	spsr;
	u32	spdr;
	u8	spscr;
	u8	spssr;
	u8	spbr;
	u8	spdcr;
	u8	spckd;
	u8	sslnd;
	u8	spnd;
	u8	dummy0;
	u16	spcmd0;
	u16	spcmd1;
	u16	spcmd2;
	u16	spcmd3;
	u8	spbfcr;
	u8	dummy1;
	u16	spbdcr;
	u32	spbmul0;
	u32	spbmul1;
	u32	spbmul2;
	u32	spbmul3;
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
	u32 nbyte, chunk;
	int i, ret = 0;
	u8 dtdata = 0, drdata;
	u8 *tdata = &dtdata, *rdata = &drdata;
	u32 *spbmul0 = &ss->regs->spbmul0;

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
		tdata = (u8 *)dout;

	if (din != NULL)
		rdata = din;

	while (nbyte > 0) {
		/*
		 * Check if there is 32 Byte chunk and if there is, transfer
		 * it in one burst, otherwise transfer on byte-by-byte basis.
		 */
		chunk = (nbyte >= 32) ? 32 : 1;

		clrsetbits_8(&ss->regs->spbfcr, SPBFCR_TXTRG | SPBFCR_RXTRG,
			     chunk == 32 ? SPBFCR_TXTRG | SPBFCR_RXTRG : 0);

		ret = wait_for_bit_8(&ss->regs->spsr, SPSR_SPTEF,
				     true, 1000, true);
		if (ret)
			return ret;

		for (i = 0; i < chunk; i++) {
			writeb(*tdata, &ss->regs->spdr);
			if (dout != NULL)
				tdata++;
		}

		ret = wait_for_bit_8(&ss->regs->spsr, SPSR_SPRFF,
				     true, 1000, true);
		if (ret)
			return ret;

		for (i = 0; i < chunk; i++) {
			*rdata = readb(&ss->regs->spdr);
			if (din != NULL)
				rdata++;
		}

		nbyte -= chunk;
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return ret;
}
