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

/**
 * spi_enable_quad_bit - Enable the QUAD bit for SPI flash
 *
 * This function will enable the quad bit in flash using
 * the QSPI controller. Supports only spansion.
 *
 * @spi : SPI slave structure
 */
void spi_enable_quad_bit(struct spi_slave *spi)
{
	int ret;
	u8 idcode[5];
	u8 rdid_cmd = 0x9f;	/* RDID */
	u8 rcr_data = 0;
	u8 rcr_cmd = 0x35;	/* RCR */
	u8 rdsr_cmd = 0x05;	/* RDSR */
	u8 wren_cmd = 0x06;	/* WREN */

	ret = spi_flash_cmd(spi, rdid_cmd, &idcode, sizeof(idcode));
	if (ret) {
		debug("SF error: Failed read RDID\n");
		return;
	}

	if (idcode[0] == 0x01) {
		/* Read config register */
		ret = spi_flash_cmd_read(spi, &rcr_cmd, sizeof(rcr_cmd),
					&rcr_data, sizeof(rcr_data));
		if (ret) {
			debug("SF error: Failed read RCR\n");
			return;
		}

		if (rcr_data & 0x2)
			debug("QUAD bit is already set..\n");
		else {
			debug("QUAD bit needs to be set ..\n");

			/* Write enable */
			ret = spi_flash_cmd(spi, wren_cmd, NULL, 0);
			if (ret) {
				debug("SF error: Failed write WREN\n");
				return;
			}

			/* Write QUAD bit */
			xqspips_write_quad_bit((void *)XPSS_QSPI_BASEADDR);

			/* Read RDSR */
			do {
				ret = spi_flash_cmd_read(spi, &rdsr_cmd,
						sizeof(rdsr_cmd), &rcr_data,
						sizeof(rcr_data));
			} while ((ret == 0) && (rcr_data != 0));

			/* Read config register */
			ret = spi_flash_cmd_read(spi, &rcr_cmd, sizeof(rcr_cmd),
						&rcr_data, sizeof(rcr_data));
			if (!(rcr_data & 0x2)) {
				printf("SF error: Fail to set QUAD enable bit"
					" 0x%x\n", rcr_data);
				return;
			} else
				debug("SF: QUAD enable bit is set 0x%x\n",
						rcr_data);
		}
	} else
		debug("SF: QUAD bit not enabled for 0x%x SPI flash\n",
					idcode[0]);

	return;
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

	spi_enable_quad_bit(&pspi->slave);

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

