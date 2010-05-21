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

#elif defined(CONFIG_MX31)

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
#define MXC_CSPICTRL_TC		(1 << 8)
#define MXC_CSPICTRL_RXOVF	(1 << 6)
#define MXC_CSPICTRL_MAXBITS	0x1f

#define MXC_CSPIPERIOD_32KHZ	(1 << 15)

static unsigned long spi_bases[] = {
	0x43fa4000,
	0x50010000,
	0x53f84000,
};

#define OUT	MX31_GPIO_DIRECTION_OUT
#define mxc_gpio_direction	mx31_gpio_direction
#define mxc_gpio_set		mx31_gpio_set
#elif defined(CONFIG_MX51)
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>

#define MXC_CSPIRXDATA		0x00
#define MXC_CSPITXDATA		0x04
#define MXC_CSPICTRL		0x08
#define MXC_CSPICON		0x0C
#define MXC_CSPIINT		0x10
#define MXC_CSPIDMA		0x14
#define MXC_CSPISTAT		0x18
#define MXC_CSPIPERIOD		0x1C
#define MXC_CSPIRESET		0x00
#define MXC_CSPICTRL_EN		(1 << 0)
#define MXC_CSPICTRL_MODE	(1 << 1)
#define MXC_CSPICTRL_XCH	(1 << 2)
#define MXC_CSPICTRL_CHIPSELECT(x)	(((x) & 0x3) << 12)
#define MXC_CSPICTRL_BITCOUNT(x)	(((x) & 0xfff) << 20)
#define MXC_CSPICTRL_PREDIV(x)	(((x) & 0xF) << 12)
#define MXC_CSPICTRL_POSTDIV(x)	(((x) & 0xF) << 8)
#define MXC_CSPICTRL_SELCHAN(x)	(((x) & 0x3) << 18)
#define MXC_CSPICTRL_MAXBITS	0xfff
#define MXC_CSPICTRL_TC		(1 << 7)
#define MXC_CSPICTRL_RXOVF	(1 << 6)

#define MXC_CSPIPERIOD_32KHZ	(1 << 15)

/* Bit position inside CTRL register to be associated with SS */
#define MXC_CSPICTRL_CHAN	18

/* Bit position inside CON register to be associated with SS */
#define MXC_CSPICON_POL		4
#define MXC_CSPICON_PHA		0
#define MXC_CSPICON_SSPOL	12

static unsigned long spi_bases[] = {
	CSPI1_BASE_ADDR,
	CSPI2_BASE_ADDR,
	CSPI3_BASE_ADDR,
};
#define mxc_gpio_direction(gpio, dir)	(0)
#define mxc_gpio_set(gpio, value)	{}
#define OUT	1
#else
#error "Unsupported architecture"
#endif

struct mxc_spi_slave {
	struct spi_slave slave;
	unsigned long	base;
	u32		ctrl_reg;
#if defined(CONFIG_MX51)
	u32		cfg_reg;
#endif
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

void spi_cs_activate(struct spi_slave *slave)
{
	struct mxc_spi_slave *mxcs = to_mxc_spi_slave(slave);
	if (mxcs->gpio > 0)
		mxc_gpio_set(mxcs->gpio, mxcs->ctrl_reg & MXC_CSPICTRL_SSPOL);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct mxc_spi_slave *mxcs = to_mxc_spi_slave(slave);
	if (mxcs->gpio > 0)
		mxc_gpio_set(mxcs->gpio,
			      !(mxcs->ctrl_reg & MXC_CSPICTRL_SSPOL));
}

#ifdef CONFIG_MX51
static s32 spi_cfg(struct mxc_spi_slave *mxcs, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	u32 clk_src = mxc_get_clock(MXC_CSPI_CLK);
	s32 pre_div = 0, post_div = 0, i, reg_ctrl, reg_config;
	u32 ss_pol = 0, sclkpol = 0, sclkpha = 0;

	if (max_hz == 0) {
		printf("Error: desired clock is 0\n");
		return -1;
	}

	reg_ctrl = reg_read(mxcs->base + MXC_CSPICTRL);

	/* Reset spi */
	reg_write(mxcs->base + MXC_CSPICTRL, 0);
	reg_write(mxcs->base + MXC_CSPICTRL, (reg_ctrl | 0x1));

	/*
	 * The following computation is taken directly from Freescale's code.
	 */
	if (clk_src > max_hz) {
		pre_div = clk_src / max_hz;
		if (pre_div > 16) {
			post_div = pre_div / 16;
			pre_div = 15;
		}
		if (post_div != 0) {
			for (i = 0; i < 16; i++) {
				if ((1 << i) >= post_div)
					break;
			}
			if (i == 16) {
				printf("Error: no divider for the freq: %d\n",
					max_hz);
				return -1;
			}
			post_div = i;
		}
	}

	debug("pre_div = %d, post_div=%d\n", pre_div, post_div);
	reg_ctrl = (reg_ctrl & ~MXC_CSPICTRL_SELCHAN(3)) |
		MXC_CSPICTRL_SELCHAN(cs);
	reg_ctrl = (reg_ctrl & ~MXC_CSPICTRL_PREDIV(0x0F)) |
		MXC_CSPICTRL_PREDIV(pre_div);
	reg_ctrl = (reg_ctrl & ~MXC_CSPICTRL_POSTDIV(0x0F)) |
		MXC_CSPICTRL_POSTDIV(post_div);

	/* always set to master mode */
	reg_ctrl |= 1 << (cs + 4);

	/* We need to disable SPI before changing registers */
	reg_ctrl &= ~MXC_CSPICTRL_EN;

	if (mode & SPI_CS_HIGH)
		ss_pol = 1;

	if (!(mode & SPI_CPOL))
		sclkpol = 1;

	if (mode & SPI_CPHA)
		sclkpha = 1;

	reg_config = reg_read(mxcs->base + MXC_CSPICON);

	/*
	 * Configuration register setup
	 * The MX51 has support different setup for each SS
	 */
	reg_config = (reg_config & ~(1 << (cs + MXC_CSPICON_SSPOL))) |
		(ss_pol << (cs + MXC_CSPICON_SSPOL));
	reg_config = (reg_config & ~(1 << (cs + MXC_CSPICON_POL))) |
		(sclkpol << (cs + MXC_CSPICON_POL));
	reg_config = (reg_config & ~(1 << (cs + MXC_CSPICON_PHA))) |
		(sclkpha << (cs + MXC_CSPICON_PHA));

	debug("reg_ctrl = 0x%x\n", reg_ctrl);
	reg_write(mxcs->base + MXC_CSPICTRL, reg_ctrl);
	debug("reg_config = 0x%x\n", reg_config);
	reg_write(mxcs->base + MXC_CSPICON, reg_config);

	/* save config register and control register */
	mxcs->ctrl_reg = reg_ctrl;
	mxcs->cfg_reg = reg_config;

	/* clear interrupt reg */
	reg_write(mxcs->base + MXC_CSPIINT, 0);
	reg_write(mxcs->base + MXC_CSPISTAT,
		MXC_CSPICTRL_TC | MXC_CSPICTRL_RXOVF);

	return 0;
}
#endif

static u32 spi_xchg_single(struct spi_slave *slave, u32 data, int bitlen,
			   unsigned long flags)
{
	struct mxc_spi_slave *mxcs = to_mxc_spi_slave(slave);

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	mxcs->ctrl_reg = (mxcs->ctrl_reg &
		~MXC_CSPICTRL_BITCOUNT(MXC_CSPICTRL_MAXBITS)) |
		MXC_CSPICTRL_BITCOUNT(bitlen - 1);

	reg_write(mxcs->base + MXC_CSPICTRL, mxcs->ctrl_reg | MXC_CSPICTRL_EN);
#ifdef CONFIG_MX51
	reg_write(mxcs->base + MXC_CSPICON, mxcs->cfg_reg);
#endif

	/* Clear interrupt register */
	reg_write(mxcs->base + MXC_CSPISTAT,
		MXC_CSPICTRL_TC | MXC_CSPICTRL_RXOVF);

	debug("Sending SPI 0x%x\n", data);
	reg_write(mxcs->base + MXC_CSPITXDATA, data);

	/* FIFO is written, now starts the transfer setting the XCH bit */
	reg_write(mxcs->base + MXC_CSPICTRL, mxcs->ctrl_reg |
		MXC_CSPICTRL_EN | MXC_CSPICTRL_XCH);

	/* Wait until the TC (Transfer completed) bit is set */
	while ((reg_read(mxcs->base + MXC_CSPISTAT) & MXC_CSPICTRL_TC) == 0)
		;

	/* Transfer completed, clear any pending request */
	reg_write(mxcs->base + MXC_CSPISTAT,
		MXC_CSPICTRL_TC | MXC_CSPICTRL_RXOVF);

	data = reg_read(mxcs->base + MXC_CSPIRXDATA);
	debug("SPI Rx: 0x%x\n", data);

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return data;

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

	/* This driver is currently partly broken, alert the user */
	if (bitlen > 16 && (bitlen % 32)) {
		printf("Error: SPI transfer with bitlen=%d is broken.\n",
		       bitlen);
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
			else
				*in_l = data;
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
		ret = mxc_gpio_direction(mxcs->gpio, OUT);
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

	mxcs->slave.bus = bus;
	mxcs->slave.cs = cs;
	mxcs->base = spi_bases[bus];

#ifdef CONFIG_MX51
	/* Can be used for i.MX31 too ? */
	ctrl_reg = 0;
	ret = spi_cfg(mxcs, cs, max_hz, mode);
	if (ret) {
		printf("mxc_spi: cannot setup SPI controller\n");
		free(mxcs);
		return NULL;
	}
#else
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
	mxcs->ctrl_reg = ctrl_reg;
#endif
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
