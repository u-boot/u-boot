/*
 * Driver of Andes SPI Controller
 *
 * (C) Copyright 2011 Andes Technology
 * Macpaul Lin <macpaul@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>

#include <asm/io.h>
#include "andes_spi.h"

void spi_init(void)
{
	/* do nothing */
}

static void andes_spi_spit_en(struct andes_spi_slave *ds)
{
	unsigned int dcr = readl(&ds->regs->dcr);

	debug("%s: dcr: %x, write value: %x\n",
			__func__, dcr, (dcr | ANDES_SPI_DCR_SPIT));

	writel((dcr | ANDES_SPI_DCR_SPIT), &ds->regs->dcr);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
			unsigned int max_hz, unsigned int mode)
{
	struct andes_spi_slave	*ds;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	ds = spi_alloc_slave(struct andes_spi_slave, bus, cs);
	if (!ds)
		return NULL;

	ds->regs = (struct andes_spi_regs *)CONFIG_SYS_SPI_BASE;

	/*
	 * The hardware of andes_spi will set its frequency according
	 * to APB/AHB bus clock. Hence the hardware doesn't allow changing of
	 * requency and so the user requested speed is always ignored.
	 */
	ds->freq = max_hz;

	return &ds->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct andes_spi_slave *ds = to_andes_spi(slave);

	free(ds);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct andes_spi_slave *ds = to_andes_spi(slave);
	unsigned int apb;
	unsigned int baud;

	/* Enable the SPI hardware */
	writel(ANDES_SPI_CR_SPIRST, &ds->regs->cr);
	udelay(1000);

	/* setup format */
	baud = ((CONFIG_SYS_CLK_FREQ / CONFIG_SYS_SPI_CLK / 2) - 1) & 0xFF;

	/*
	 * SPI_CLK = AHB bus clock / ((BAUD + 1)*2)
	 * BAUD = AHB bus clock / SPI_CLK / 2) - 1
	 */
	apb = (readl(&ds->regs->apb) & 0xffffff00) | baud;
	writel(apb, &ds->regs->apb);

	/* no interrupts */
	writel(0, &ds->regs->ie);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct andes_spi_slave *ds = to_andes_spi(slave);

	/* Disable the SPI hardware */
	writel(ANDES_SPI_CR_SPIRST, &ds->regs->cr);
}

static int andes_spi_read(struct spi_slave *slave, unsigned int len,
			    u8 *rxp, unsigned long flags)
{
	struct andes_spi_slave *ds = to_andes_spi(slave);
	unsigned int i, left;
	unsigned int data;

	debug("%s: slave: %x, len: %d, rxp: %x, flags: %d\n",
		__func__, slave, len, rxp, flags);

	debug("%s: data: ", __func__);
	while (len > 0) {
		left = min(len, 4);
		data = readl(&ds->regs->data);

		debug(" ");
		for (i = 0; i < left; i++) {
			debug("%02x ", data & 0xff);
			*rxp++ = data;
			data >>= 8;
			len--;
		}
	}
	debug("\n");

	return 0;
}

static int andes_spi_write(struct spi_slave *slave, unsigned int wlen,
			unsigned int rlen, const u8 *txp, unsigned long flags)
{
	struct andes_spi_slave *ds = to_andes_spi(slave);
	unsigned int data;
	unsigned int i, left;
	unsigned int spit_enabled = 0;

	debug("%s: slave: %x, wlen: %d, rlen: %d, txp: %x, flags: %x\n",
		__func__, slave, wlen, rlen, txp, flags);

	/* The value of wlen and rlen wrote to register must minus 1 */
	if (rlen == 0)					/* write only */
		writel(ANDES_SPI_DCR_MODE_WO | ANDES_SPI_DCR_WCNT(wlen-1) |
				ANDES_SPI_DCR_RCNT(0), &ds->regs->dcr);
	else						/* write then read */
		writel(ANDES_SPI_DCR_MODE_WR | ANDES_SPI_DCR_WCNT(wlen-1) |
				ANDES_SPI_DCR_RCNT(rlen-1), &ds->regs->dcr);

	/* wait till SPIBSY is cleared */
	while (readl(&ds->regs->st) & ANDES_SPI_ST_SPIBSY)
		;

	/* data write process */
	debug("%s: txp: ", __func__);
	while (wlen > 0) {
		/* clear the data */
		data = 0;

		/* data are usually be read 32bits once a time */
		left = min(wlen, 4);

		for (i = 0; i < left; i++) {
			debug("%x ", *txp);
			data |= *txp++ << (i * 8);
			wlen--;
		}
		debug("\n");

		debug("data: %08x\n", data);
		debug("streg before write: %08x\n", readl(&ds->regs->st));
		/* wait till TXFULL is deasserted */
		while (readl(&ds->regs->st) & ANDES_SPI_ST_TXFEL)
			;
		writel(data, &ds->regs->data);
		debug("streg after write: %08x\n", readl(&ds->regs->st));


		if (spit_enabled == 0) {
			/* enable SPIT bit -  trigger the tx and rx progress */
			andes_spi_spit_en(ds);
			spit_enabled = 1;
		}

	}
	debug("\n");

	return 0;
}

/*
 * spi_xfer:
 *	Since andes_spi doesn't support independent command transaction,
 *	that is, write and than read must be operated in continuous
 *	execution, there is no need to set dcr and trigger spit again in
 *	RX process.
 */
int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	     const void *dout, void *din, unsigned long flags)
{
	unsigned int len;
	static int op_nextime;
	static u8 tmp_cmd[5];
	static int tmp_wlen;
	unsigned int i;

	if (bitlen == 0)
		/* Finish any previously submitted transfers */
		goto out;

	if (bitlen % 8) {
		/* Errors always terminate an ongoing transfer */
		flags |= SPI_XFER_END;
		goto out;
	}

	len = bitlen / 8;

	debug("%s: slave: %08x, bitlen: %d, dout: "
		"%08x, din: %08x, flags: %d, len: %d\n",
		__func__, slave, bitlen, dout, din, flags, len);

	/*
	 * Important:
	 *	andes_spi's hardware doesn't support 2 data channel. The read
	 *	and write cmd/data share the same register (data register).
	 *
	 *	If a command has write and read transaction, you cannot do write
	 *	this time and then do read on next time.
	 *
	 *	A command writes first with a read response must indicating
	 *	the read length in write operation. Hence the write action must
	 *	be stored temporary and wait until the next read action has been
	 *	arrived. Then we flush the write and read action out together.
	 */
	if (!dout) {
		if (op_nextime == 1) {
			/* flags should be SPI_XFER_END, value is 2 */
			op_nextime = 0;
			andes_spi_write(slave, tmp_wlen, len, tmp_cmd, flags);
		}
		return andes_spi_read(slave, len, din, flags);
	} else if (!din) {
		if (flags == SPI_XFER_BEGIN) {
			/* store the write command and do operation next time */
			op_nextime = 1;
			memset(tmp_cmd, 0, sizeof(tmp_cmd));
			memcpy(tmp_cmd, dout, len);

			debug("%s: tmp_cmd: ", __func__);
			for (i = 0; i < len; i++)
				debug("%x ", *(tmp_cmd + i));
			debug("\n");

			tmp_wlen = len;
		} else {
			/*
			 * flags should be (SPI_XFER_BEGIN | SPI_XFER_END),
			 * the value is 3.
			 */
			if (op_nextime == 1) {
				/* flags should be SPI_XFER_END, value is 2 */
				op_nextime = 0;
				/* flags 3 implies write only */
				andes_spi_write(slave, tmp_wlen, 0, tmp_cmd, 3);
			}

			debug("flags: %x\n", flags);
			return andes_spi_write(slave, len, 0, dout, flags);
		}
	}

out:
	return 0;
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	/* do nothing */
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	/* do nothing */
}
