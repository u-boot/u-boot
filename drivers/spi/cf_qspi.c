/*
 * Freescale Coldfire Queued SPI driver
 *
 * NOTE:
 * This driver is written to transfer 8 bit at-a-time and uses the dedicated
 * SPI slave select pins as bit-banged GPIO to work with spi_flash subsystem.
 *
 * Copyright (C) 2011 Ruggedcom, Inc.
 * Richard Retanubun (richardretanubun@freescale.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <asm/immap.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define clamp(x, low, high) (min(max(low, x), high))
#define to_cf_qspi_slave(s) container_of(s, struct cf_qspi_slave, slave)

struct cf_qspi_slave {
	struct spi_slave slave;	/* Specific bus:cs ID for each device */
	qspi_t *regs;		/* Pointer to SPI controller registers */
	u16 qmr;		/* QMR: Queued Mode Register */
	u16 qwr;		/* QWR: Queued Wrap Register */
	u16 qcr;		/* QCR: Queued Command Ram */
};

/* Register write wrapper functions */
static void write_qmr(volatile qspi_t *qspi, u16 val)   { qspi->mr = val; }
static void write_qdlyr(volatile qspi_t *qspi, u16 val) { qspi->dlyr = val; }
static void write_qwr(volatile qspi_t *qspi, u16 val)   { qspi->wr = val; }
static void write_qir(volatile qspi_t *qspi, u16 val)   { qspi->ir = val; }
static void write_qar(volatile qspi_t *qspi, u16 val)   { qspi->ar = val; }
static void write_qdr(volatile qspi_t *qspi, u16 val)   { qspi->dr = val; }
/* Register read wrapper functions */
static u16 read_qdlyr(volatile qspi_t *qspi) { return qspi->dlyr; }
static u16 read_qwr(volatile qspi_t *qspi)   { return qspi->wr; }
static u16 read_qir(volatile qspi_t *qspi)   { return qspi->ir; }
static u16 read_qdr(volatile qspi_t *qspi)   { return qspi->dr; }

/* These call points may be different for each ColdFire CPU */
extern void cfspi_port_conf(void);
static void cfspi_cs_activate(uint bus, uint cs, uint cs_active_high);
static void cfspi_cs_deactivate(uint bus, uint cs, uint cs_active_high);

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}
void spi_release_bus(struct spi_slave *slave)
{
}

__attribute__((weak))
void spi_init(void)
{
	cfspi_port_conf();
}

__attribute__((weak))
void spi_cs_activate(struct spi_slave *slave)
{
	struct cf_qspi_slave *dev = to_cf_qspi_slave(slave);

	cfspi_cs_activate(slave->bus, slave->cs, !(dev->qwr & QSPI_QWR_CSIV));
}

__attribute__((weak))
void spi_cs_deactivate(struct spi_slave *slave)
{
	struct cf_qspi_slave *dev = to_cf_qspi_slave(slave);

	cfspi_cs_deactivate(slave->bus, slave->cs, !(dev->qwr & QSPI_QWR_CSIV));
}

__attribute__((weak))
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	/* Only 1 bus and 4 chipselect per controller */
	if (bus == 0 && (cs >= 0 && cs < 4))
		return 1;
	else
		return 0;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct cf_qspi_slave *dev = to_cf_qspi_slave(slave);

	free(dev);
}

/* Translate information given by spi_setup_slave to members of cf_qspi_slave */
struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct cf_qspi_slave *dev = NULL;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	dev = spi_alloc_slave(struct cf_qspi_slave, bus, cs);
	if (!dev)
		return NULL;

	/* Initialize to known value */
	dev->regs      = (qspi_t *)MMAP_QSPI;
	dev->qmr       = 0;
	dev->qwr       = 0;
	dev->qcr       = 0;


	/* Map max_hz to QMR[BAUD] */
	if (max_hz == 0) /* Go as fast as possible */
		dev->qmr = 2u;
	else /* Get the closest baud rate */
		dev->qmr = clamp(((gd->bus_clk >> 2) + max_hz - 1)/max_hz,
					2u, 255u);

	/* Map mode to QMR[CPOL] and QMR[CPHA] */
	if (mode & SPI_CPOL)
		dev->qmr |= QSPI_QMR_CPOL;

	if (mode & SPI_CPHA)
		dev->qmr |= QSPI_QMR_CPHA;

	/* Hardcode bit length to 8 bit per transter */
	dev->qmr |= QSPI_QMR_BITS_8;

	/* Set QMR[MSTR] to enable QSPI as master */
	dev->qmr |= QSPI_QMR_MSTR;

	/*
	 * Set QCR and QWR to default values for spi flash operation.
	 * If more custom QCR and QRW are needed, overload mode variable
	 */
	dev->qcr = (QSPI_QDR_CONT | QSPI_QDR_BITSE);

	if (!(mode & SPI_CS_HIGH))
		dev->qwr |= QSPI_QWR_CSIV;

	return &dev->slave;
}

/* Transfer 8 bit at a time */
int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct cf_qspi_slave *dev = to_cf_qspi_slave(slave);
	volatile qspi_t *qspi = dev->regs;
	u8 *txbuf = (u8 *)dout;
	u8 *rxbuf = (u8 *)din;
	u32 count = DIV_ROUND_UP(bitlen, 8);
	u32 n, i = 0;

	/* Sanitize arguments */
	if (slave == NULL) {
		printf("%s: NULL slave ptr\n", __func__);
		return -1;
	}

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	/* There is something to send, lets process it. spi_xfer is also called
	 * just to toggle chip select, so bitlen of 0 is valid */
	if (count > 0) {
		/*
		* NOTE: Since chip select is driven as a bit-bang-ed GPIO
		* using spi_cs_activate() and spi_cs_deactivate(),
		* the chip select settings inside the controller
		* (i.e. QCR[CONT] and QWR[CSIV]) are moot. The bits are set to
		* keep the controller settings consistent with the actual
		* operation of the bus.
		*/

		/* Write the slave device's settings for the controller.*/
		write_qmr(qspi, dev->qmr);
		write_qwr(qspi, dev->qwr);

		/* Limit transfer to 16 at a time */
		n = min(count, 16u);
		do {
			/* Setup queue end point */
			write_qwr(qspi, ((read_qwr(qspi) & QSPI_QWR_ENDQP_MASK)
				| QSPI_QWR_ENDQP((n-1))));

			/* Write Command RAM */
			write_qar(qspi, QSPI_QAR_CMD);
			for (i = 0; i < n; ++i)
				write_qdr(qspi, dev->qcr);

			/* Write TxBuf, if none given, fill with ZEROes */
			write_qar(qspi, QSPI_QAR_TRANS);
			if (txbuf) {
				for (i = 0; i < n; ++i)
					write_qdr(qspi, *txbuf++);
			} else {
				for (i = 0; i < n; ++i)
					write_qdr(qspi, 0);
			}

			/* Clear QIR[SPIF] by writing a 1 to it */
			write_qir(qspi, read_qir(qspi) | QSPI_QIR_SPIF);
			/* Set QDLYR[SPE] to start sending */
			write_qdlyr(qspi, read_qdlyr(qspi) | QSPI_QDLYR_SPE);

			/* Poll QIR[SPIF] for transfer completion */
			while ((read_qir(qspi) & QSPI_QIR_SPIF) != 1)
				udelay(1);

			/* If given read RxBuf, load data to it */
			if (rxbuf) {
				write_qar(qspi, QSPI_QAR_RECV);
				for (i = 0; i < n; ++i)
					*rxbuf++ = read_qdr(qspi);
			}

			/* Decrement count */
			count -= n;
		} while (count);
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}

/* Each MCF CPU may have different pin assignments for chip selects. */
#if defined(CONFIG_M5271)
/* Assert chip select, val = [1|0] , dir = out, mode = GPIO */
void cfspi_cs_activate(uint bus, uint cs, uint cs_active_high)
{
	debug("%s: bus %d cs %d cs_active_high %d\n",
		__func__, bus, cs, cs_active_high);

	switch (cs) {
	case 0: /* QSPI_CS[0] = PQSPI[3] */
		if (cs_active_high)
			mbar_writeByte(MCF_GPIO_PPDSDR_QSPI, 0x08);
		else
			mbar_writeByte(MCF_GPIO_PCLRR_QSPI, 0xF7);

		mbar_writeByte(MCF_GPIO_PDDR_QSPI,
			mbar_readByte(MCF_GPIO_PDDR_QSPI) | 0x08);

		mbar_writeByte(MCF_GPIO_PAR_QSPI,
			mbar_readByte(MCF_GPIO_PAR_QSPI) & 0xDF);
		break;
	case 1: /* QSPI_CS[1] = PQSPI[4] */
		if (cs_active_high)
			mbar_writeByte(MCF_GPIO_PPDSDR_QSPI, 0x10);
		else
			mbar_writeByte(MCF_GPIO_PCLRR_QSPI, 0xEF);

		mbar_writeByte(MCF_GPIO_PDDR_QSPI,
			mbar_readByte(MCF_GPIO_PDDR_QSPI) | 0x10);

		mbar_writeByte(MCF_GPIO_PAR_QSPI,
			mbar_readByte(MCF_GPIO_PAR_QSPI) & 0x3F);
		break;
	case 2: /* QSPI_CS[2] = PTIMER[7] */
		if (cs_active_high)
			mbar_writeByte(MCF_GPIO_PPDSDR_TIMER, 0x80);
		else
			mbar_writeByte(MCF_GPIO_PCLRR_TIMER, 0x7F);

		mbar_writeByte(MCF_GPIO_PDDR_TIMER,
			mbar_readByte(MCF_GPIO_PDDR_TIMER) | 0x80);

		mbar_writeShort(MCF_GPIO_PAR_TIMER,
			mbar_readShort(MCF_GPIO_PAR_TIMER) & 0x3FFF);
		break;
	case 3: /* QSPI_CS[3] = PTIMER[3] */
		if (cs_active_high)
			mbar_writeByte(MCF_GPIO_PPDSDR_TIMER, 0x08);
		else
			mbar_writeByte(MCF_GPIO_PCLRR_TIMER, 0xF7);

		mbar_writeByte(MCF_GPIO_PDDR_TIMER,
			mbar_readByte(MCF_GPIO_PDDR_TIMER) | 0x08);

		mbar_writeShort(MCF_GPIO_PAR_TIMER,
			mbar_readShort(MCF_GPIO_PAR_TIMER) & 0xFF3F);
		break;
	}
}

/* Deassert chip select, val = [1|0], dir = in, mode = GPIO
 * direction set as IN to undrive the pin, external pullup/pulldown will bring
 * bus to deassert state.
 */
void cfspi_cs_deactivate(uint bus, uint cs, uint cs_active_high)
{
	debug("%s: bus %d cs %d cs_active_high %d\n",
		__func__, bus, cs, cs_active_high);

	switch (cs) {
	case 0: /* QSPI_CS[0] = PQSPI[3] */
		if (cs_active_high)
			mbar_writeByte(MCF_GPIO_PCLRR_QSPI, 0xF7);
		else
			mbar_writeByte(MCF_GPIO_PPDSDR_QSPI, 0x08);

		mbar_writeByte(MCF_GPIO_PDDR_QSPI,
			mbar_readByte(MCF_GPIO_PDDR_QSPI) & 0xF7);

		mbar_writeByte(MCF_GPIO_PAR_QSPI,
			mbar_readByte(MCF_GPIO_PAR_QSPI) & 0xDF);
		break;
	case 1: /* QSPI_CS[1] = PQSPI[4] */
		if (cs_active_high)
			mbar_writeByte(MCF_GPIO_PCLRR_QSPI, 0xEF);
		else
			mbar_writeByte(MCF_GPIO_PPDSDR_QSPI, 0x10);

		mbar_writeByte(MCF_GPIO_PDDR_QSPI,
			mbar_readByte(MCF_GPIO_PDDR_QSPI) & 0xEF);

		mbar_writeByte(MCF_GPIO_PAR_QSPI,
			mbar_readByte(MCF_GPIO_PAR_QSPI) & 0x3F);
		break;
	case 2: /* QSPI_CS[2] = PTIMER[7] */
		if (cs_active_high)
			mbar_writeByte(MCF_GPIO_PCLRR_TIMER, 0x7F);
		else
			mbar_writeByte(MCF_GPIO_PPDSDR_TIMER, 0x80);

		mbar_writeByte(MCF_GPIO_PDDR_TIMER,
			mbar_readByte(MCF_GPIO_PDDR_TIMER) & 0x7F);

		mbar_writeShort(MCF_GPIO_PAR_TIMER,
			mbar_readShort(MCF_GPIO_PAR_TIMER) & 0x3FFF);
		break;
	case 3: /* QSPI_CS[3] = PTIMER[3] */
		if (cs_active_high)
			mbar_writeByte(MCF_GPIO_PCLRR_TIMER, 0xF7);
		else
			mbar_writeByte(MCF_GPIO_PPDSDR_TIMER, 0x08);

		mbar_writeByte(MCF_GPIO_PDDR_TIMER,
			mbar_readByte(MCF_GPIO_PDDR_TIMER) & 0xF7);

		mbar_writeShort(MCF_GPIO_PAR_TIMER,
			mbar_readShort(MCF_GPIO_PAR_TIMER) & 0xFF3F);
		break;
	}
}
#endif /* CONFIG_M5271 */
