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
#include <malloc.h>
#include <spi.h>
#include <asm/errno.h>
#include <asm/io.h>

#ifdef CONFIG_MX27
/* i.MX27 has a completely wrong register layout and register definitions in the
 * datasheet, the correct one is in the Freescale's Linux driver */

#error "i.MX27 CSPI not supported due to drastic differences in register definisions" \
"See linux mxc_spi driver from Freescale for details."

#else

#include <asm/arch/mx31.h>

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
#define MXC_CSPICTRL_SSPOL	(1 << 7)
#define MXC_CSPICTRL_CHIPSELECT(x)	(((x) & 0x3) << 24)
#define MXC_CSPICTRL_BITCOUNT(x)	(((x) & 0x1f) << 8)
#define MXC_CSPICTRL_DATARATE(x)	(((x) & 0x7) << 16)

#define MXC_CSPIPERIOD_32KHZ	(1 << 15)

static unsigned long spi_bases[] = {
	0x43fa4000,
	0x50010000,
	0x53f84000,
};

#endif

struct mxc_spi_slave {
	struct spi_slave slave;
	unsigned long	base;
	u32		ctrl_reg;
	int		gpio;
};

static inline struct mxc_spi_slave *to_mxc_spi_slave(struct spi_slave *slave)
{
	return container_of(slave, struct mxc_spi_slave, slave);
}

static inline u32 reg_read(unsigned long addr)
{
	return *(volatile unsigned long*)addr;
}

static inline void reg_write(unsigned long addr, u32 val)
{
	*(volatile unsigned long*)addr = val;
}

static u32 spi_xchg_single(struct spi_slave *slave, u32 data, int bitlen,
			   unsigned long flags)
{
	struct mxc_spi_slave *mxcs = to_mxc_spi_slave(slave);
	unsigned int cfg_reg = reg_read(mxcs->base + MXC_CSPICTRL);

	mxcs->ctrl_reg = (mxcs->ctrl_reg & ~MXC_CSPICTRL_BITCOUNT(31)) |
		MXC_CSPICTRL_BITCOUNT(bitlen - 1);

	if (cfg_reg != mxcs->ctrl_reg)
		reg_write(mxcs->base + MXC_CSPICTRL, mxcs->ctrl_reg);

	if (mxcs->gpio > 0 && (flags & SPI_XFER_BEGIN))
		mx31_gpio_set(mxcs->gpio, mxcs->ctrl_reg & MXC_CSPICTRL_SSPOL);

	reg_write(mxcs->base + MXC_CSPITXDATA, data);

	reg_write(mxcs->base + MXC_CSPICTRL, mxcs->ctrl_reg | MXC_CSPICTRL_XCH);

	while (reg_read(mxcs->base + MXC_CSPICTRL) & MXC_CSPICTRL_XCH)
		;

	if (mxcs->gpio > 0 && (flags & SPI_XFER_END)) {
		mx31_gpio_set(mxcs->gpio,
			      !(mxcs->ctrl_reg & MXC_CSPICTRL_SSPOL));
	}

	return reg_read(mxcs->base + MXC_CSPIRXDATA);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	int n_blks = (bitlen + 31) / 32;
	u32 *out_l, *in_l;
	int i;

	if ((int)dout & 3 || (int)din & 3) {
		printf("Error: unaligned buffers in: %p, out: %p\n", din, dout);
		return 1;
	}

	for (i = 0, in_l = (u32 *)din, out_l = (u32 *)dout;
	     i < n_blks;
	     i++, in_l++, out_l++, bitlen -= 32) {
		u32 data = spi_xchg_single(slave, *out_l, bitlen, flags);

		/* Check if we're only transfering 8 or 16 bits */
		if (!i) {
			if (bitlen < 9)
				*(u8 *)din = data;
			else if (bitlen < 17)
				*(u16 *)din = data;
		}
	}

	return 0;
}

void spi_init(void)
{
}

static int decode_cs(struct mxc_spi_slave *mxcs, unsigned int cs)
{
	int ret;

	/*
	 * Some SPI devices require active chip-select over multiple
	 * transactions, we achieve this using a GPIO. Still, the SPI
	 * controller has to be configured to use one of its own chipselects.
	 * To use this feature you have to call spi_setup_slave() with
	 * cs = internal_cs | (gpio << 8), and you have to use some unused
	 * on this SPI controller cs between 0 and 3.
	 */
	if (cs > 3) {
		mxcs->gpio = cs >> 8;
		cs &= 3;
		ret = mx31_gpio_direction(mxcs->gpio, MX31_GPIO_DIRECTION_OUT);
		if (ret) {
			printf("mxc_spi: cannot setup gpio %d\n", mxcs->gpio);
			return -EINVAL;
		}
	} else {
		mxcs->gpio = -1;
	}

	return cs;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
			unsigned int max_hz, unsigned int mode)
{
	unsigned int ctrl_reg;
	struct mxc_spi_slave *mxcs;
	int ret;

	if (bus >= ARRAY_SIZE(spi_bases))
		return NULL;

	mxcs = malloc(sizeof(struct mxc_spi_slave));
	if (!mxcs)
		return NULL;

	ret = decode_cs(mxcs, cs);
	if (ret < 0) {
		free(mxcs);
		return NULL;
	}

	cs = ret;

	ctrl_reg = MXC_CSPICTRL_CHIPSELECT(cs) |
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

	mxcs->slave.bus = bus;
	mxcs->slave.cs = cs;
	mxcs->base = spi_bases[bus];
	mxcs->ctrl_reg = ctrl_reg;

	return &mxcs->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct mxc_spi_slave *mxcs = to_mxc_spi_slave(slave);

	free(mxcs);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct mxc_spi_slave *mxcs = to_mxc_spi_slave(slave);

	reg_write(mxcs->base + MXC_CSPIRESET, 1);
	udelay(1);
	reg_write(mxcs->base + MXC_CSPICTRL, mxcs->ctrl_reg);
	reg_write(mxcs->base + MXC_CSPIPERIOD,
		  MXC_CSPIPERIOD_32KHZ);
	reg_write(mxcs->base + MXC_CSPIINT, 0);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	/* TODO: Shut the controller down */
}
