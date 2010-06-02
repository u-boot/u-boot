/*
 * Driver for Blackfin On-Chip SPI device
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

/*#define DEBUG*/

#include <common.h>
#include <malloc.h>
#include <spi.h>

#include <asm/blackfin.h>
#include <asm/gpio.h>
#include <asm/portmux.h>
#include <asm/mach-common/bits/spi.h>

struct bfin_spi_slave {
	struct spi_slave slave;
	void *mmr_base;
	u16 ctl, baud, flg;
};

#define MAKE_SPI_FUNC(mmr, off) \
static inline void write_##mmr(struct bfin_spi_slave *bss, u16 val) { bfin_write16(bss->mmr_base + off, val); } \
static inline u16 read_##mmr(struct bfin_spi_slave *bss) { return bfin_read16(bss->mmr_base + off); }
MAKE_SPI_FUNC(SPI_CTL,  0x00)
MAKE_SPI_FUNC(SPI_FLG,  0x04)
MAKE_SPI_FUNC(SPI_STAT, 0x08)
MAKE_SPI_FUNC(SPI_TDBR, 0x0c)
MAKE_SPI_FUNC(SPI_RDBR, 0x10)
MAKE_SPI_FUNC(SPI_BAUD, 0x14)

#define to_bfin_spi_slave(s) container_of(s, struct bfin_spi_slave, slave)

#define MAX_CTRL_CS 7

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
		gpio_set_value(cs, bss->flg);
		debug("%s: SPI_CS_GPIO:%x\n", __func__, gpio_get_value(cs));
	} else {
		write_SPI_FLG(bss,
			(read_SPI_FLG(bss) &
			~((!bss->flg << 8) << slave->cs)) |
			(1 << slave->cs));
		debug("%s: SPI_FLG:%x\n", __func__, read_SPI_FLG(bss));
	}

	SSYNC();
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);

	if (is_gpio_cs(slave->cs)) {
		unsigned int cs = gpio_cs(slave->cs);
		gpio_set_value(cs, !bss->flg);
		debug("%s: SPI_CS_GPIO:%x\n", __func__, gpio_get_value(cs));
	} else {
		u16 flg;

		/* make sure we force the cs to deassert rather than let the
		 * pin float back up.  otherwise, exact timings may not be
		 * met some of the time leading to random behavior (ugh).
		 */
		flg = read_SPI_FLG(bss) | ((!bss->flg << 8) << slave->cs);
		write_SPI_FLG(bss, flg);
		SSYNC();
		debug("%s: SPI_FLG:%x\n", __func__, read_SPI_FLG(bss));

		flg &= ~(1 << slave->cs);
		write_SPI_FLG(bss, flg);
		debug("%s: SPI_FLG:%x\n", __func__, read_SPI_FLG(bss));
	}

	SSYNC();
}

void spi_init()
{
}

#ifdef SPI_CTL
# define SPI0_CTL SPI_CTL
#endif

#define SPI_PINS(n) \
	[n] = { 0, P_SPI##n##_SCK, P_SPI##n##_MISO, P_SPI##n##_MOSI, 0 }
static unsigned short pins[][5] = {
#ifdef SPI0_CTL
	SPI_PINS(0),
#endif
#ifdef SPI1_CTL
	SPI_PINS(1),
#endif
#ifdef SPI2_CTL
	SPI_PINS(2),
#endif
};

#define SPI_CS_PINS(n) \
	[n] = { \
		P_SPI##n##_SSEL1, P_SPI##n##_SSEL2, P_SPI##n##_SSEL3, \
		P_SPI##n##_SSEL4, P_SPI##n##_SSEL5, P_SPI##n##_SSEL6, \
		P_SPI##n##_SSEL7, \
	}
static const unsigned short cs_pins[][7] = {
#ifdef SPI0_CTL
	SPI_CS_PINS(0),
#endif
#ifdef SPI1_CTL
	SPI_CS_PINS(1),
#endif
#ifdef SPI2_CTL
	SPI_CS_PINS(2),
#endif
};

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct bfin_spi_slave *bss;
	ulong sclk;
	u32 mmr_base;
	u32 baud;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	if (bus >= ARRAY_SIZE(pins) || pins[bus] == NULL) {
		debug("%s: invalid bus %u\n", __func__, bus);
		return NULL;
	}
	switch (bus) {
#ifdef SPI0_CTL
		case 0: mmr_base = SPI0_CTL; break;
#endif
#ifdef SPI1_CTL
		case 1: mmr_base = SPI1_CTL; break;
#endif
#ifdef SPI2_CTL
		case 2: mmr_base = SPI2_CTL; break;
#endif
		default: return NULL;
	}

	sclk = get_sclk();
	baud = sclk / (2 * max_hz);
	/* baud should be rounded up */
	if (sclk % (2 * max_hz))
		baud += 1;
	if (baud < 2)
		baud = 2;
	else if (baud > (u16)-1)
		baud = -1;

	bss = malloc(sizeof(*bss));
	if (!bss)
		return NULL;

	bss->slave.bus = bus;
	bss->slave.cs = cs;
	bss->mmr_base = (void *)mmr_base;
	bss->ctl = SPE | MSTR | TDBR_CORE;
	if (mode & SPI_CPHA) bss->ctl |= CPHA;
	if (mode & SPI_CPOL) bss->ctl |= CPOL;
	if (mode & SPI_LSB_FIRST) bss->ctl |= LSBF;
	bss->baud = baud;
	bss->flg = mode & SPI_CS_HIGH ? 1 : 0;

	debug("%s: bus:%i cs:%i mmr:%x ctl:%x baud:%i flg:%i\n", __func__,
		bus, cs, mmr_base, bss->ctl, baud, bss->flg);

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
		gpio_direction_output(cs, !bss->flg);
		pins[slave->bus][0] = P_DONTCARE;
	} else
		pins[slave->bus][0] = cs_pins[slave->bus][slave->cs - 1];
	peripheral_request_list(pins[slave->bus], "bfin-spi");

	write_SPI_CTL(bss, bss->ctl);
	write_SPI_BAUD(bss, bss->baud);
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

	write_SPI_CTL(bss, 0);
	SSYNC();
}

#ifndef CONFIG_BFIN_SPI_IDLE_VAL
# define CONFIG_BFIN_SPI_IDLE_VAL 0xff
#endif

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

	/* todo: take advantage of hardware fifos and setup RX dma */
	while (bytes--) {
		u8 value = (tx ? *tx++ : CONFIG_BFIN_SPI_IDLE_VAL);
		debug("%s: tx:%x ", __func__, value);
		write_SPI_TDBR(bss, value);
		SSYNC();
		while ((read_SPI_STAT(bss) & TXS))
			if (ctrlc()) {
				ret = -1;
				goto done;
			}
		while (!(read_SPI_STAT(bss) & SPIF))
			if (ctrlc()) {
				ret = -1;
				goto done;
			}
		while (!(read_SPI_STAT(bss) & RXS))
			if (ctrlc()) {
				ret = -1;
				goto done;
			}
		value = read_SPI_RDBR(bss);
		if (rx)
			*rx++ = value;
		debug("rx:%x\n", value);
	}

 done:
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return ret;
}
