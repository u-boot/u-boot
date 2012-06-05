/*
 * Driver for Pele On-Chip SPI device
 *
 * Copyright (c) 2010 Xilinx
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>

#include <asm/arch/xparameters.h>

#include "xilinx_qspips.h"

struct pele_spi_slave {
	struct spi_slave  slave;
	struct spi_device qspi;
};

#define to_pele_spi_slave(s) container_of(s, struct pele_spi_slave, slave)

__attribute__((weak))
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
#ifdef DEBUG
	printf("spi_cs_is_valid: bus: %d cs: %d\n",
		bus, cs);
#endif
	return 1;
}

__attribute__((weak))
void spi_cs_activate(struct spi_slave *slave)
{
#ifdef DEBUG
	printf("spi_cs_activate: slave 0x%08x\n", (unsigned)slave);
#endif
}

__attribute__((weak))
void spi_cs_deactivate(struct spi_slave *slave)
{
#ifdef DEBUG
	printf("spi_cs_deactivate: slave 0x%08x\n", (unsigned)slave);
#endif
}

void spi_init()
{
#ifdef DEBUG
	printf("spi_init:\n");
#endif
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct pele_spi_slave *pspi;

#ifdef DEBUG
	printf("spi_setup_slave: bus: %d cs: %d max_hz: %d mode: %d\n",
		bus, cs, max_hz, mode);
#endif

	xqspips_init_hw((void *)XPSS_QSPI_BASEADDR);

	pspi = malloc(sizeof(struct pele_spi_slave));
	if (!pspi) {
		return NULL;
	}
	pspi->slave.bus = bus;
	pspi->slave.cs = cs;
	pspi->qspi.master.input_clk_hz = 100000000;
	pspi->qspi.master.speed_hz     = pspi->qspi.master.input_clk_hz / 2;
	pspi->qspi.max_speed_hz = pspi->qspi.master.speed_hz;
	pspi->qspi.master.dev_busy     = 0;
	pspi->qspi.master.regs = (void*)XPSS_QSPI_BASEADDR;
	pspi->qspi.mode = mode;
	pspi->qspi.chip_select = 0;
	pspi->qspi.bits_per_word = 32;
	xqspips_setup_transfer(&pspi->qspi, NULL);

	return &pspi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct pele_spi_slave *pspi;

#ifdef DEBUG
	printf("spi_free_slave: slave: 0x%08x\n", (u32)slave);
#endif

	pspi = to_pele_spi_slave(slave);
	free(pspi);
}

int spi_claim_bus(struct spi_slave *slave)
{
#ifdef DEBUG
	printf("spi_claim_bus: slave: 0x%08x\n", (u32)slave);
#endif
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
#ifdef DEBUG
	printf("spi_release_bus: slave: 0x%08x\n", (u32)slave);
#endif
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct pele_spi_slave *pspi;
	struct spi_transfer transfer;

#ifdef DEBUG
	printf("spi_xfer: slave: 0x%08x bitlen: %d dout: 0x%08x din: 0x%08x flags: 0x%lx\n",
		(u32)slave, bitlen, (u32)dout, (u32)din, flags);
#endif

	pspi = (struct pele_spi_slave *)slave;
	transfer.tx_buf = dout;
	transfer.rx_buf = din;
	transfer.len = bitlen / 8;
	
	/* Festering sore.
	 * Assume that the beginning of a transfer with bits to
	 * transmit must contain a device command.
	 */
	if (dout && flags & SPI_XFER_BEGIN) {
		pspi->qspi.master.is_inst = 1;
	} else {
		pspi->qspi.master.is_inst = 0;
	}

	if (flags & SPI_XFER_END) {
		transfer.cs_change = 1;
	} else {
		transfer.cs_change = 0;
	}

	transfer.delay_usecs = 0;
	transfer.bits_per_word = 32;
	transfer.speed_hz = pspi->qspi.max_speed_hz;

	xqspips_transfer(&pspi->qspi, &transfer);

	return 0;
}

