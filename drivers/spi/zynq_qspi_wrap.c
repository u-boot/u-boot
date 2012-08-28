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

#include "zynq_qspi.h"

struct zynq_spi_slave {
	struct spi_slave  slave;
	struct spi_device qspi;
};

#define to_zynq_spi_slave(s) container_of(s, struct zynq_spi_slave, slave)

__attribute__((weak))
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	debug("spi_cs_is_valid: bus: %d cs: %d\n",
		bus, cs);
	return 1;
}

__attribute__((weak))
void spi_cs_activate(struct spi_slave *slave)
{
	debug("spi_cs_activate: slave 0x%08x\n", (unsigned)slave);
}

__attribute__((weak))
void spi_cs_deactivate(struct spi_slave *slave)
{
	debug("spi_cs_deactivate: slave 0x%08x\n", (unsigned)slave);
}

void spi_init()
{
	debug("spi_init\n");
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	int is_dual;
	struct zynq_spi_slave *pspi;

	debug("spi_setup_slave: bus: %d cs: %d max_hz: %d mode: %d\n",
		bus, cs, max_hz, mode);

	is_dual = xqspips_check_is_dual_flash((void *)XPSS_SYS_CTRL_BASEADDR);

	if (is_dual == -1) {
		printf("SPI error: No QSPI device detected based"
				" on MIO settings\n");
		return NULL;
	}

	xqspips_init_hw((void *)XPSS_QSPI_BASEADDR, is_dual);

	pspi = malloc(sizeof(struct zynq_spi_slave));
	if (!pspi) {
		return NULL;
	}
	pspi->slave.bus = bus;
	pspi->slave.cs = cs;
	pspi->slave.is_dual = is_dual;
	pspi->qspi.master.input_clk_hz = 100000000;
	pspi->qspi.master.speed_hz     = pspi->qspi.master.input_clk_hz / 2;
	pspi->qspi.max_speed_hz = pspi->qspi.master.speed_hz;
	pspi->qspi.master.dev_busy     = 0;
	pspi->qspi.master.regs = (void*)XPSS_QSPI_BASEADDR;
	pspi->qspi.master.is_dual = is_dual;
	pspi->qspi.mode = mode;
	pspi->qspi.chip_select = 0;
	pspi->qspi.bits_per_word = 32;
	xqspips_setup_transfer(&pspi->qspi, NULL);

	return &pspi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct zynq_spi_slave *pspi;

	debug("spi_free_slave: slave: 0x%08x\n", (u32)slave);

	pspi = to_zynq_spi_slave(slave);
	free(pspi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	debug("spi_claim_bus: slave: 0x%08x\n", (u32)slave);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	debug("spi_release_bus: slave: 0x%08x\n", (u32)slave);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct zynq_spi_slave *pspi;
	struct spi_transfer transfer;

	debug("spi_xfer: slave: 0x%08x bitlen: %d dout: 0x%08x din:"
		" 0x%08x flags: 0x%lx\n",
		(u32)slave, bitlen, (u32)dout, (u32)din, flags);

	pspi = (struct zynq_spi_slave *)slave;
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

