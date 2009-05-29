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

__attribute__((weak))
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
#if defined(__ADSPBF538__) || defined(__ADSPBF539__)
	/* The SPI1/SPI2 buses are weird ... only 1 CS */
	if (bus > 0 && cs != 1)
		return 0;
#endif
	return (cs >= 1 && cs <= 7);
}

__attribute__((weak))
void spi_cs_activate(struct spi_slave *slave)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);
	write_SPI_FLG(bss,
		(read_SPI_FLG(bss) &
		~((!bss->flg << 8) << slave->cs)) |
		(1 << slave->cs));
	SSYNC();
	debug("%s: SPI_FLG:%x\n", __func__, read_SPI_FLG(bss));
}

__attribute__((weak))
void spi_cs_deactivate(struct spi_slave *slave)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);
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
	SSYNC();
	debug("%s: SPI_FLG:%x\n", __func__, read_SPI_FLG(bss));
}

void spi_init()
{
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct bfin_spi_slave *bss;
	u32 mmr_base;
	u32 baud;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	switch (bus) {
#ifdef SPI_CTL
# define SPI0_CTL SPI_CTL
#endif
		case 0: mmr_base = SPI0_CTL; break;
#ifdef SPI1_CTL
		case 1: mmr_base = SPI1_CTL; break;
#endif
#ifdef SPI2_CTL
		case 2: mmr_base = SPI2_CTL; break;
#endif
		default: return NULL;
	}

	baud = get_sclk() / (2 * max_hz);
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

static void spi_portmux(struct spi_slave *slave)
{
#if defined(__ADSPBF51x__)
#define SET_MUX(port, mux, func) port##_mux = ((port##_mux & ~PORT_x_MUX_##mux##_MASK) | PORT_x_MUX_##mux##_FUNC_##func)
	u16 f_mux = bfin_read_PORTF_MUX();
	u16 f_fer = bfin_read_PORTF_FER();
	u16 g_mux = bfin_read_PORTG_MUX();
	u16 g_fer = bfin_read_PORTG_FER();
	u16 h_mux = bfin_read_PORTH_MUX();
	u16 h_fer = bfin_read_PORTH_FER();
	switch (slave->bus) {
	case 0:
		/* set SCK/MISO/MOSI */
		SET_MUX(g, 7, 1);
		g_fer |= PG12 | PG13 | PG14;
		switch (slave->cs) {
			case 1: SET_MUX(f, 2, 1); f_fer |= PF7;  break;
			case 2: /* see G above */ g_fer |= PG15; break;
			case 3: SET_MUX(h, 1, 3); f_fer |= PH4;  break;
			case 4: /* no muxing */   h_fer |= PH8;  break;
			case 5: SET_MUX(g, 1, 3); h_fer |= PG3;  break;
			case 6: /* no muxing */                  break;
			case 7: /* no muxing */                  break;
		}
	case 1:
		/* set SCK/MISO/MOSI */
		SET_MUX(h, 0, 2);
		h_fer |= PH1 | PH2 | PH3;
		switch (slave->cs) {
			case 1: SET_MUX(h, 2, 3); h_fer |= PH6;  break;
			case 2: SET_MUX(f, 0, 3); f_fer |= PF0;  break;
			case 3: SET_MUX(g, 0, 3); g_fer |= PG0;  break;
			case 4: SET_MUX(f, 3, 3); f_fer |= PF8;  break;
			case 5: SET_MUX(g, 6, 3); h_fer |= PG11; break;
			case 6: /* no muxing */                  break;
			case 7: /* no muxing */                  break;
		}
	}
	bfin_write_PORTF_MUX(f_mux);
	bfin_write_PORTF_FER(f_fer);
	bfin_write_PORTG_MUX(g_mux);
	bfin_write_PORTG_FER(g_fer);
	bfin_write_PORTH_MUX(h_mux);
	bfin_write_PORTH_FER(h_fer);
#elif defined(__ADSPBF52x__)
#define SET_MUX(port, mux, func) port##_mux = ((port##_mux & ~PORT_x_MUX_##mux##_MASK) | PORT_x_MUX_##mux##_FUNC_##func)
	u16 f_mux = bfin_read_PORTF_MUX();
	u16 f_fer = bfin_read_PORTF_FER();
	u16 g_mux = bfin_read_PORTG_MUX();
	u16 g_fer = bfin_read_PORTG_FER();
	u16 h_mux = bfin_read_PORTH_MUX();
	u16 h_fer = bfin_read_PORTH_FER();
	/* set SCK/MISO/MOSI */
	SET_MUX(g, 0, 3);
	g_fer |= PG2 | PG3 | PG4;
	switch (slave->cs) {
		case 1: /* see G above */ g_fer |= PG1;  break;
		case 2: SET_MUX(f, 4, 3); f_fer |= PF12; break;
		case 3: SET_MUX(f, 4, 3); f_fer |= PF13; break;
		case 4: SET_MUX(h, 1, 1); h_fer |= PH8;  break;
		case 5: SET_MUX(h, 2, 1); h_fer |= PH9;  break;
		case 6: SET_MUX(f, 1, 3); f_fer |= PF9;  break;
		case 7: SET_MUX(f, 2, 3); f_fer |= PF10; break;
	}
	bfin_write_PORTF_MUX(f_mux);
	bfin_write_PORTF_FER(f_fer);
	bfin_write_PORTG_MUX(g_mux);
	bfin_write_PORTG_FER(g_fer);
	bfin_write_PORTH_MUX(h_mux);
	bfin_write_PORTH_FER(h_fer);
#elif defined(__ADSPBF534__) || defined(__ADSPBF536__) || defined(__ADSPBF537__)
	u16 mux = bfin_read_PORT_MUX();
	u16 f_fer = bfin_read_PORTF_FER();
	/* set SCK/MISO/MOSI */
	f_fer |= PF11 | PF12 | PF13;
	switch (slave->cs) {
		case 1: f_fer |= PF10; break;
		case 2: mux |= PJSE; break;
		case 3: mux |= PJSE; break;
		case 4: mux |= PFS4E; f_fer |= PF6; break;
		case 5: mux |= PFS5E; f_fer |= PF5; break;
		case 6: mux |= PFS6E; f_fer |= PF4; break;
		case 7: mux |= PJCE_SPI; break;
	}
	bfin_write_PORT_MUX(mux);
	bfin_write_PORTF_FER(f_fer);
#elif defined(__ADSPBF538__) || defined(__ADSPBF539__)
	u16 fer, pins;
	if (slave->bus == 1)
		pins = PD0 | PD1 | PD2 | (slave->cs == 1 ? PD4 : 0);
	else if (slave->bus == 2)
		pins = PD5 | PD6 | PD7 | (slave->cs == 1 ? PD9 : 0);
	else
		pins = 0;
	if (pins) {
		fer = bfin_read_PORTDIO_FER();
		fer &= ~pins;
		bfin_write_PORTDIO_FER(fer);
	}
#elif defined(__ADSPBF54x__)
#define DO_MUX(port, pin) \
	mux = ((mux & ~PORT_x_MUX_##pin##_MASK) | PORT_x_MUX_##pin##_FUNC_1); \
	fer |= P##port##pin;
	u32 mux;
	u16 fer;
	switch (slave->bus) {
	case 0:
		mux = bfin_read_PORTE_MUX();
		fer = bfin_read_PORTE_FER();
		/* set SCK/MISO/MOSI */
		DO_MUX(E, 0);
		DO_MUX(E, 1);
		DO_MUX(E, 2);
		switch (slave->cs) {
			case 1: DO_MUX(E, 4); break;
			case 2: DO_MUX(E, 5); break;
			case 3: DO_MUX(E, 6); break;
		}
		bfin_write_PORTE_MUX(mux);
		bfin_write_PORTE_FER(fer);
		break;
	case 1:
		mux = bfin_read_PORTG_MUX();
		fer = bfin_read_PORTG_FER();
		/* set SCK/MISO/MOSI */
		DO_MUX(G, 8);
		DO_MUX(G, 9);
		DO_MUX(G, 10);
		switch (slave->cs) {
			case 1: DO_MUX(G, 5); break;
			case 2: DO_MUX(G, 6); break;
			case 3: DO_MUX(G, 7); break;
		}
		bfin_write_PORTG_MUX(mux);
		bfin_write_PORTG_FER(fer);
		break;
	case 2:
		mux = bfin_read_PORTB_MUX();
		fer = bfin_read_PORTB_FER();
		/* set SCK/MISO/MOSI */
		DO_MUX(B, 12);
		DO_MUX(B, 13);
		DO_MUX(B, 14);
		switch (slave->cs) {
			case 1: DO_MUX(B, 9);  break;
			case 2: DO_MUX(B, 10); break;
			case 3: DO_MUX(B, 11); break;
		}
		bfin_write_PORTB_MUX(mux);
		bfin_write_PORTB_FER(fer);
		break;
	}
#endif
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);

	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);

	spi_portmux(slave);
	write_SPI_CTL(bss, bss->ctl);
	write_SPI_BAUD(bss, bss->baud);
	SSYNC();

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct bfin_spi_slave *bss = to_bfin_spi_slave(slave);
	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);
	write_SPI_CTL(bss, 0);
	SSYNC();
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

	/* todo: take advantage of hardware fifos and setup RX dma */
	while (bytes--) {
		u8 value = (tx ? *tx++ : 0);
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
