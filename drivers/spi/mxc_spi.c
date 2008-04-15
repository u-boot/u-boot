/*
 * Copyright (C) 2008, Guennadi Liakhovetski <lg@denx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <common.h>
#include <spi.h>
#include <asm/io.h>

#ifdef CONFIG_MX27
/* i.MX27 has a completely wrong register layout and register definitions in the
 * datasheet, the correct one is in the Freescale's Linux driver */

#error "i.MX27 CSPI not supported due to drastic differences in register definisions" \
"See linux mxc_spi driver from Freescale for details."

#else

#define MXC_CSPIRXDATA		0x00
#define MXC_CSPITXDATA		0x04
#define MXC_CSPICTRL		0x08
#define MXC_CSPIINT		0x0C
#define MXC_CSPIDMA		0x10
#define MXC_CSPISTAT		0x14
#define MXC_CSPIPERIOD		0x18
#define MXC_CSPITEST		0x1C
#define MXC_CSPIRESET		0x00

#define MXC_CSPICTRL_EN		(1 << 0)
#define MXC_CSPICTRL_MODE	(1 << 1)
#define MXC_CSPICTRL_XCH	(1 << 2)
#define MXC_CSPICTRL_SMC	(1 << 3)
#define MXC_CSPICTRL_POL	(1 << 4)
#define MXC_CSPICTRL_PHA	(1 << 5)
#define MXC_CSPICTRL_SSCTL	(1 << 6)
#define MXC_CSPICTRL_SSPOL 	(1 << 7)
#define MXC_CSPICTRL_CHIPSELECT(x)	(((x) & 0x3) << 24)
#define MXC_CSPICTRL_BITCOUNT(x)	(((x) & 0x1f) << 8)
#define MXC_CSPICTRL_DATARATE(x)	(((x) & 0x7) << 16)

#define MXC_CSPIPERIOD_32KHZ	(1 << 15)

static unsigned long spi_bases[] = {
	0x43fa4000,
	0x50010000,
	0x53f84000,
};

static unsigned long spi_base;

#endif

spi_chipsel_type spi_chipsel[] = {
	(spi_chipsel_type)0,
	(spi_chipsel_type)1,
	(spi_chipsel_type)2,
	(spi_chipsel_type)3,
};
int spi_chipsel_cnt = sizeof(spi_chipsel) / sizeof(spi_chipsel[0]);

static inline u32 reg_read(unsigned long addr)
{
	return *(volatile unsigned long*)addr;
}

static inline void reg_write(unsigned long addr, u32 val)
{
	*(volatile unsigned long*)addr = val;
}

static u32 spi_xchg_single(u32 data, int bitlen)
{

	unsigned int cfg_reg = reg_read(spi_base + MXC_CSPICTRL);

	if (MXC_CSPICTRL_BITCOUNT(bitlen - 1) != (cfg_reg & MXC_CSPICTRL_BITCOUNT(31))) {
		cfg_reg = (cfg_reg & ~MXC_CSPICTRL_BITCOUNT(31)) |
			MXC_CSPICTRL_BITCOUNT(bitlen - 1);
		reg_write(spi_base + MXC_CSPICTRL, cfg_reg);
	}

	reg_write(spi_base + MXC_CSPITXDATA, data);

	cfg_reg |= MXC_CSPICTRL_XCH;

	reg_write(spi_base + MXC_CSPICTRL, cfg_reg);

	while (reg_read(spi_base + MXC_CSPICTRL) & MXC_CSPICTRL_XCH)
		;

	return reg_read(spi_base + MXC_CSPIRXDATA);
}

int spi_xfer(spi_chipsel_type chipsel, int bitlen, uchar *dout, uchar *din)
{
	int n_blks = (bitlen + 31) / 32;
	u32 *out_l, *in_l;
	int i;

	if ((int)dout & 3 || (int)din & 3) {
		printf("Error: unaligned buffers in: %p, out: %p\n", din, dout);
		return 1;
	}

	if (!spi_base)
		spi_select(CONFIG_MXC_SPI_IFACE, (int)chipsel, SPI_MODE_2 | SPI_CS_HIGH);

	for (i = 0, in_l = (u32 *)din, out_l = (u32 *)dout;
	     i < n_blks;
	     i++, in_l++, out_l++, bitlen -= 32)
		*in_l = spi_xchg_single(*out_l, bitlen);

	return 0;
}

void spi_init(void)
{
}

int spi_select(unsigned int bus, unsigned int dev, unsigned long mode)
{
	unsigned int ctrl_reg;

	if (bus >= sizeof(spi_bases) / sizeof(spi_bases[0]) ||
	    dev > 3)
		return 1;

	spi_base = spi_bases[bus];

	ctrl_reg = MXC_CSPICTRL_CHIPSELECT(dev) |
		MXC_CSPICTRL_BITCOUNT(31) |
		MXC_CSPICTRL_DATARATE(7) | /* FIXME: calculate data rate */
		MXC_CSPICTRL_EN |
		MXC_CSPICTRL_MODE;

	if (mode & SPI_CPHA)
		ctrl_reg |= MXC_CSPICTRL_PHA;
	if (!(mode & SPI_CPOL))
		ctrl_reg |= MXC_CSPICTRL_POL;
	if (mode & SPI_CS_HIGH)
		ctrl_reg |= MXC_CSPICTRL_SSPOL;

	reg_write(spi_base + MXC_CSPIRESET, 1);
	udelay(1);
	reg_write(spi_base + MXC_CSPICTRL, ctrl_reg);
	reg_write(spi_base + MXC_CSPIPERIOD,
		  MXC_CSPIPERIOD_32KHZ);
	reg_write(spi_base + MXC_CSPIINT, 0);

	return 0;
}
