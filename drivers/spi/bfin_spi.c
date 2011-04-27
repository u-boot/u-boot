/*
 * Driver for Blackfin On-Chip SPI device
 *
 * Copyright (c) 2005-2010 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

/*#define DEBUG*/

#include <common.h>
#include <malloc.h>
#include <spi.h>

#include <asm/blackfin.h>
#include <asm/dma.h>
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

void spi_set_speed(struct spi_slave *slave, uint hz)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);
	ulong sclk;
	u32 baud;

	sclk = get_sclk();
	baud = sclk / (2 * hz);
	/* baud should be rounded up */
	if (sclk % (2 * hz))
		baud += 1;
	if (baud < 2)
		baud = 2;
	else if (baud > (u16)-1)
		baud = -1;
	bss->baud = baud;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct bfin_spi_slave *bss;
	u32 mmr_base;

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
	bss->flg = mode & SPI_CS_HIGH ? 1 : 0;
	spi_set_speed(&bss->slave, max_hz);

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

#ifdef __ADSPBF54x__
# define SPI_DMA_BASE DMA4_NEXT_DESC_PTR
#elif defined(__ADSPBF533__) || defined(__ADSPBF532__) || defined(__ADSPBF531__) || \
      defined(__ADSPBF538__) || defined(__ADSPBF539__)
# define SPI_DMA_BASE DMA5_NEXT_DESC_PTR
#elif defined(__ADSPBF561__)
# define SPI_DMA_BASE DMA2_4_NEXT_DESC_PTR
#elif defined(__ADSPBF537__) || defined(__ADSPBF536__) || defined(__ADSPBF534__) || \
      defined(__ADSPBF52x__) || defined(__ADSPBF51x__)
# define SPI_DMA_BASE DMA7_NEXT_DESC_PTR
# elif defined(__ADSPBF50x__)
# define SPI_DMA_BASE DMA6_NEXT_DESC_PTR
#else
# error "Please provide SPI DMA channel defines"
#endif
static volatile struct dma_register *dma = (void *)SPI_DMA_BASE;

#ifndef CONFIG_BFIN_SPI_IDLE_VAL
# define CONFIG_BFIN_SPI_IDLE_VAL 0xff
#endif

#ifdef CONFIG_BFIN_SPI_NO_DMA
# define SPI_DMA 0
#else
# define SPI_DMA 1
#endif

static int spi_dma_xfer(struct bfin_spi_slave *bss, const u8 *tx, u8 *rx,
			uint bytes)
{
	int ret = -1;
	u16 ndsize, spi_config, dma_config;
	struct dmasg dmasg[2];
	const u8 *buf;

	if (tx) {
		debug("%s: doing half duplex TX\n", __func__);
		buf = tx;
		spi_config = TDBR_DMA;
		dma_config = 0;
	} else {
		debug("%s: doing half duplex RX\n", __func__);
		buf = rx;
		spi_config = RDBR_DMA;
		dma_config = WNR;
	}

	dmasg[0].start_addr = (unsigned long)buf;
	dmasg[0].x_modify = 1;
	dma_config |= WDSIZE_8 | DMAEN;
	if (bytes <= 65536) {
		blackfin_dcache_flush_invalidate_range(buf, buf + bytes);
		ndsize = NDSIZE_5;
		dmasg[0].cfg = NDSIZE_0 | dma_config | FLOW_STOP | DI_EN;
		dmasg[0].x_count = bytes;
	} else {
		blackfin_dcache_flush_invalidate_range(buf, buf + 65536 - 1);
		ndsize = NDSIZE_7;
		dmasg[0].cfg = NDSIZE_5 | dma_config | FLOW_ARRAY | DMA2D;
		dmasg[0].x_count = 0;	/* 2^16 */
		dmasg[0].y_count = bytes >> 16;	/* count / 2^16 */
		dmasg[0].y_modify = 1;
		dmasg[1].start_addr = (unsigned long)(buf + (bytes & ~0xFFFF));
		dmasg[1].cfg = NDSIZE_0 | dma_config | FLOW_STOP | DI_EN;
		dmasg[1].x_count = bytes & 0xFFFF; /* count % 2^16 */
		dmasg[1].x_modify = 1;
	}

	dma->cfg = 0;
	dma->irq_status = DMA_DONE | DMA_ERR;
	dma->curr_desc_ptr = dmasg;
	write_SPI_CTL(bss, (bss->ctl & ~TDBR_CORE));
	write_SPI_STAT(bss, -1);
	SSYNC();

	write_SPI_TDBR(bss, CONFIG_BFIN_SPI_IDLE_VAL);
	dma->cfg = ndsize | FLOW_ARRAY | DMAEN;
	write_SPI_CTL(bss, (bss->ctl & ~TDBR_CORE) | spi_config);
	SSYNC();

	/*
	 * We already invalidated the first 64k,
	 * now while we just wait invalidate the remaining part.
	 * Its not likely that the DMA is going to overtake
	 */
	if (bytes > 65536)
		blackfin_dcache_flush_invalidate_range(buf + 65536, buf + bytes);

	while (!(dma->irq_status & DMA_DONE))
		if (ctrlc())
			goto done;

	dma->cfg = 0;

	ret = 0;
 done:
	write_SPI_CTL(bss, bss->ctl);
	return ret;
}

static int spi_pio_xfer(struct bfin_spi_slave *bss, const u8 *tx, u8 *rx,
			uint bytes)
{
	/* todo: take advantage of hardware fifos  */
	while (bytes--) {
		u8 value = (tx ? *tx++ : CONFIG_BFIN_SPI_IDLE_VAL);
		debug("%s: tx:%x ", __func__, value);
		write_SPI_TDBR(bss, value);
		SSYNC();
		while ((read_SPI_STAT(bss) & TXS))
			if (ctrlc())
				return -1;
		while (!(read_SPI_STAT(bss) & SPIF))
			if (ctrlc())
				return -1;
		while (!(read_SPI_STAT(bss) & RXS))
			if (ctrlc())
				return -1;
		value = read_SPI_RDBR(bss);
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

	/* TX DMA doesn't work quite right */
	if (SPI_DMA && bytes > 6 && (!tx /*|| !rx*/))
		ret = spi_dma_xfer(bss, tx, rx, bytes);
	else
		ret = spi_pio_xfer(bss, tx, rx, bytes);

 done:
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return ret;
}
