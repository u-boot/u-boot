/*
 * Analog Devices SPI3 controller driver
 *
 * Copyright (c) 2011 Analog Devices Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <console.h>
#include <malloc.h>
#include <spi.h>

#include <asm/blackfin.h>
#include <asm/clock.h>
#include <asm/gpio.h>
#include <asm/portmux.h>
#include <asm/mach-common/bits/spi6xx.h>

struct bfin_spi_slave {
	struct spi_slave slave;
	u32 control, clock;
	struct bfin_spi_regs *regs;
	int cs_pol;
};

#define to_bfin_spi_slave(s) container_of(s, struct bfin_spi_slave, slave)

#define gpio_cs(cs) ((cs) - MAX_CTRL_CS)
#ifdef CONFIG_BFIN_SPI_GPIO_CS
# define is_gpio_cs(cs) ((cs) > MAX_CTRL_CS)
#else
# define is_gpio_cs(cs) 0
#endif

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	if (is_gpio_cs(cs))
		return gpio_is_valid(gpio_cs(cs));
	else
		return (cs >= 1 && cs <= MAX_CTRL_CS);
}

void spi_cs_activate(struct spi_slave *slave)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);

	if (is_gpio_cs(slave->cs)) {
		unsigned int cs = gpio_cs(slave->cs);
		gpio_set_value(cs, bss->cs_pol);
	} else {
		u32 ssel;
		ssel = bfin_read32(&bss->regs->ssel);
		ssel |= 1 << slave->cs;
		if (bss->cs_pol)
			ssel |= BIT(8) << slave->cs;
		else
			ssel &= ~(BIT(8) << slave->cs);
		bfin_write32(&bss->regs->ssel, ssel);
	}

	SSYNC();
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);

	if (is_gpio_cs(slave->cs)) {
		unsigned int cs = gpio_cs(slave->cs);
		gpio_set_value(cs, !bss->cs_pol);
	} else {
		u32 ssel;
		ssel = bfin_read32(&bss->regs->ssel);
		if (bss->cs_pol)
			ssel &= ~(BIT(8) << slave->cs);
		else
			ssel |= BIT(8) << slave->cs;
		/* deassert cs */
		bfin_write32(&bss->regs->ssel, ssel);
		SSYNC();
		/* disable cs */
		ssel &= ~(1 << slave->cs);
		bfin_write32(&bss->regs->ssel, ssel);
	}

	SSYNC();
}

void spi_init()
{
}

#define SPI_PINS(n) \
	{ 0, P_SPI##n##_SCK, P_SPI##n##_MISO, P_SPI##n##_MOSI, 0 }
static unsigned short pins[][5] = {
#ifdef SPI0_REGBASE
	[0] = SPI_PINS(0),
#endif
#ifdef SPI1_REGBASE
	[1] = SPI_PINS(1),
#endif
#ifdef SPI2_REGBASE
	[2] = SPI_PINS(2),
#endif
};

#define SPI_CS_PINS(n) \
	{ \
		P_SPI##n##_SSEL1, P_SPI##n##_SSEL2, P_SPI##n##_SSEL3, \
		P_SPI##n##_SSEL4, P_SPI##n##_SSEL5, P_SPI##n##_SSEL6, \
		P_SPI##n##_SSEL7, \
	}
static const unsigned short cs_pins[][7] = {
#ifdef SPI0_REGBASE
	[0] = SPI_CS_PINS(0),
#endif
#ifdef SPI1_REGBASE
	[1] = SPI_CS_PINS(1),
#endif
#ifdef SPI2_REGBASE
	[2] = SPI_CS_PINS(2),
#endif
};

void spi_set_speed(struct spi_slave *slave, uint hz)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);
	ulong clk;
	u32 clock;

	clk = get_spi_clk();
	clock = clk / hz;
	if (clock)
		clock--;
	bss->clock = clock;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct bfin_spi_slave *bss;
	u32 reg_base;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	switch (bus) {
#ifdef SPI0_REGBASE
	case 0:
		reg_base = SPI0_REGBASE;
		break;
#endif
#ifdef SPI1_REGBASE
	case 1:
		reg_base = SPI1_REGBASE;
		break;
#endif
#ifdef SPI2_REGBASE
	case 2:
		reg_base = SPI2_REGBASE;
		break;
#endif
	default:
		debug("%s: invalid bus %u\n", __func__, bus);
		return NULL;
	}

	bss = spi_alloc_slave(struct bfin_spi_slave, bus, cs);
	if (!bss)
		return NULL;

	bss->regs = (struct bfin_spi_regs *)reg_base;
	bss->control = SPI_CTL_EN | SPI_CTL_MSTR;
	if (mode & SPI_CPHA)
		bss->control |= SPI_CTL_CPHA;
	if (mode & SPI_CPOL)
		bss->control |= SPI_CTL_CPOL;
	if (mode & SPI_LSB_FIRST)
		bss->control |= SPI_CTL_LSBF;
	bss->control &= ~SPI_CTL_ASSEL;
	bss->cs_pol = mode & SPI_CS_HIGH ? 1 : 0;
	spi_set_speed(&bss->slave, max_hz);

	return &bss->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);
	free(bss);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);

	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);

	if (is_gpio_cs(slave->cs)) {
		unsigned int cs = gpio_cs(slave->cs);
		gpio_request(cs, "bfin-spi");
		gpio_direction_output(cs, !bss->cs_pol);
		pins[slave->bus][0] = P_DONTCARE;
	} else
		pins[slave->bus][0] = cs_pins[slave->bus][slave->cs - 1];
	peripheral_request_list(pins[slave->bus], "bfin-spi");

	bfin_write32(&bss->regs->control, bss->control);
	bfin_write32(&bss->regs->clock, bss->clock);
	bfin_write32(&bss->regs->delay, 0x0);
	bfin_write32(&bss->regs->rx_control, SPI_RXCTL_REN);
	bfin_write32(&bss->regs->tx_control, SPI_TXCTL_TEN | SPI_TXCTL_TTI);
	SSYNC();

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);

	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);

	peripheral_free_list(pins[slave->bus]);
	if (is_gpio_cs(slave->cs))
		gpio_free(gpio_cs(slave->cs));

	bfin_write32(&bss->regs->rx_control, 0x0);
	bfin_write32(&bss->regs->tx_control, 0x0);
	bfin_write32(&bss->regs->control, 0x0);
	SSYNC();
}

#ifndef CONFIG_BFIN_SPI_IDLE_VAL
# define CONFIG_BFIN_SPI_IDLE_VAL 0xff
#endif

static int spi_pio_xfer(struct bfin_spi_slave *bss, const u8 *tx, u8 *rx,
			uint bytes)
{
	/* discard invalid rx data and empty rfifo */
	while (!(bfin_read32(&bss->regs->status) & SPI_STAT_RFE))
		bfin_read32(&bss->regs->rfifo);

	while (bytes--) {
		u8 value = (tx ? *tx++ : CONFIG_BFIN_SPI_IDLE_VAL);
		debug("%s: tx:%x ", __func__, value);
		bfin_write32(&bss->regs->tfifo, value);
		SSYNC();
		while (bfin_read32(&bss->regs->status) & SPI_STAT_RFE)
			if (ctrlc())
				return -1;
		value = bfin_read32(&bss->regs->rfifo);
		if (rx)
			*rx++ = value;
		debug("rx:%x\n", value);
	}

	return 0;
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);
	const u8 *tx = dout;
	u8 *rx = din;
	uint bytes = bitlen / 8;
	int ret = 0;

	debug("%s: bus:%i cs:%i bitlen:%i bytes:%i flags:%lx\n", __func__,
		slave->bus, slave->cs, bitlen, bytes, flags);

	if (bitlen == 0)
		goto done;

	/* we can only do 8 bit transfers */
	if (bitlen % 8) {
		flags |= SPI_XFER_END;
		goto done;
	}

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	ret = spi_pio_xfer(bss, tx, rx, bytes);

 done:
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return ret;
}
