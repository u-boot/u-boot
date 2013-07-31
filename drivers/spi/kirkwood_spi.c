/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Derived from drivers/spi/mpc8xxx_spi.c
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/arch/kirkwood.h>
#include <asm/arch/spi.h>
#include <asm/arch/mpp.h>

static struct kwspi_registers *spireg = (struct kwspi_registers *)KW_SPI_BASE;

u32 cs_spi_mpp_back[2];

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				unsigned int max_hz, unsigned int mode)
{
	struct spi_slave *slave;
	u32 data;
	static const u32 kwspi_mpp_config[2][2] = {
		{ MPP0_SPI_SCn, 0 }, /* if cs == 0 */
		{ MPP7_SPI_SCn, 0 } /* if cs != 0 */
	};

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	slave = spi_alloc_slave_base(bus, cs);
	if (!slave)
		return NULL;

	writel(~KWSPI_CSN_ACT | KWSPI_SMEMRDY, &spireg->ctrl);

	/* calculate spi clock prescaller using max_hz */
	data = ((CONFIG_SYS_TCLK / 2) / max_hz) + 0x10;
	data = data < KWSPI_CLKPRESCL_MIN ? KWSPI_CLKPRESCL_MIN : data;
	data = data > KWSPI_CLKPRESCL_MASK ? KWSPI_CLKPRESCL_MASK : data;

	/* program spi clock prescaller using max_hz */
	writel(KWSPI_ADRLEN_3BYTE | data, &spireg->cfg);
	debug("data = 0x%08x \n", data);

	writel(KWSPI_SMEMRDIRQ, &spireg->irq_cause);
	writel(KWSPI_IRQMASK, &spireg->irq_mask);

	/* program mpp registers to select  SPI_CSn */
	kirkwood_mpp_conf(kwspi_mpp_config[cs ? 1 : 0], cs_spi_mpp_back);

	return slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	kirkwood_mpp_conf(cs_spi_mpp_back, NULL);
	free(slave);
}

#if defined(CONFIG_SYS_KW_SPI_MPP)
u32 spi_mpp_backup[4];
#endif

__attribute__((weak)) int board_spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

int spi_claim_bus(struct spi_slave *slave)
{
#if defined(CONFIG_SYS_KW_SPI_MPP)
	u32 config;
	u32 spi_mpp_config[4];

	config = CONFIG_SYS_KW_SPI_MPP;

	if (config & MOSI_MPP6)
		spi_mpp_config[0] = MPP6_SPI_MOSI;
	else
		spi_mpp_config[0] = MPP1_SPI_MOSI;

	if (config & SCK_MPP10)
		spi_mpp_config[1] = MPP10_SPI_SCK;
	else
		spi_mpp_config[1] = MPP2_SPI_SCK;

	if (config & MISO_MPP11)
		spi_mpp_config[2] = MPP11_SPI_MISO;
	else
		spi_mpp_config[2] = MPP3_SPI_MISO;

	spi_mpp_config[3] = 0;
	spi_mpp_backup[3] = 0;

	/* set new spi mpp and save current mpp config */
	kirkwood_mpp_conf(spi_mpp_config, spi_mpp_backup);

#endif

	return board_spi_claim_bus(slave);
}

__attribute__((weak)) void board_spi_release_bus(struct spi_slave *slave)
{
}

void spi_release_bus(struct spi_slave *slave)
{
#if defined(CONFIG_SYS_KW_SPI_MPP)
	kirkwood_mpp_conf(spi_mpp_backup, NULL);
#endif

	board_spi_release_bus(slave);
}

#ifndef CONFIG_SPI_CS_IS_VALID
/*
 * you can define this function board specific
 * define above CONFIG in board specific config file and
 * provide the function in board specific src file
 */
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return (bus == 0 && (cs == 0 || cs == 1));
}
#endif

void spi_init(void)
{
}

void spi_cs_activate(struct spi_slave *slave)
{
	writel(readl(&spireg->ctrl) | KWSPI_IRQUNMASK, &spireg->ctrl);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	writel(readl(&spireg->ctrl) & KWSPI_IRQMASK, &spireg->ctrl);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	unsigned int tmpdout, tmpdin;
	int tm, isread = 0;

	debug("spi_xfer: slave %u:%u dout %p din %p bitlen %u\n",
	      slave->bus, slave->cs, dout, din, bitlen);

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	/*
	 * handle data in 8-bit chunks
	 * TBD: 2byte xfer mode to be enabled
	 */
	writel(((readl(&spireg->cfg) & ~KWSPI_XFERLEN_MASK) |
		KWSPI_XFERLEN_1BYTE), &spireg->cfg);

	while (bitlen > 4) {
		debug("loopstart bitlen %d\n", bitlen);
		tmpdout = 0;

		/* Shift data so it's msb-justified */
		if (dout)
			tmpdout = *(u32 *) dout & 0x0ff;

		writel(~KWSPI_SMEMRDIRQ, &spireg->irq_cause);
		writel(tmpdout, &spireg->dout);	/* Write the data out */
		debug("*** spi_xfer: ... %08x written, bitlen %d\n",
		      tmpdout, bitlen);

		/*
		 * Wait for SPI transmit to get out
		 * or time out (1 second = 1000 ms)
		 * The NE event must be read and cleared first
		 */
		for (tm = 0, isread = 0; tm < KWSPI_TIMEOUT; ++tm) {
			if (readl(&spireg->irq_cause) & KWSPI_SMEMRDIRQ) {
				isread = 1;
				tmpdin = readl(&spireg->din);
				debug
					("spi_xfer: din %p..%08x read\n",
					din, tmpdin);

				if (din) {
					*((u8 *) din) = (u8) tmpdin;
					din += 1;
				}
				if (dout)
					dout += 1;
				bitlen -= 8;
			}
			if (isread)
				break;
		}
		if (tm >= KWSPI_TIMEOUT)
			printf("*** spi_xfer: Time out during SPI transfer\n");

		debug("loopend bitlen %d\n", bitlen);
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
