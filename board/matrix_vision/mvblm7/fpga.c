/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com.
 *
 * (C) Copyright 2008
 * Andre Schwarz, Matrix Vision GmbH, andre.schwarz@matrix-vision.de
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <ACEX1K.h>
#include <command.h>
#include "fpga.h"
#include "mvblm7.h"

#ifdef FPGA_DEBUG
#define fpga_debug(fmt, args...)      printf("%s: "fmt, __func__, ##args)
#else
#define fpga_debug(fmt, args...)
#endif

Altera_CYC2_Passive_Serial_fns altera_fns = {
	fpga_null_fn,
	fpga_config_fn,
	fpga_status_fn,
	fpga_done_fn,
	fpga_wr_fn,
	fpga_null_fn,
	fpga_null_fn,
	0
};

Altera_desc cyclone2 = {
	Altera_CYC2,
	passive_serial,
	Altera_EP2C20_SIZE,
	(void *) &altera_fns,
	NULL,
	0
};

DECLARE_GLOBAL_DATA_PTR;

int mvblm7_init_fpga(void)
{
	fpga_debug("Initialize FPGA interface (reloc 0x%.8lx)\n",
		gd->reloc_off);
	fpga_init(gd->reloc_off);
	fpga_add(fpga_altera, &cyclone2);
	fpga_config_fn(0, 1, 0);
	udelay(60);

	return 1;
}

int fpga_null_fn(int cookie)
{
	return 0;
}

int fpga_config_fn(int assert, int flush, int cookie)
{
	volatile immap_t *im = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile gpio83xx_t *gpio = (volatile gpio83xx_t *)&im->gpio[0];
	u32 dvo = gpio->dat;

	fpga_debug("SET config : %s\n", assert ? "low" : "high");
	if (assert)
		dvo |= FPGA_CONFIG;
	else
		dvo &= ~FPGA_CONFIG;

	if (flush)
		gpio->dat = dvo;

	return assert;
}

int fpga_done_fn(int cookie)
{
	volatile immap_t *im = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile gpio83xx_t *gpio = (volatile gpio83xx_t *)&im->gpio[0];
	int result = 0;

	udelay(10);
	fpga_debug("CONF_DONE check ... ");
	if (gpio->dat & FPGA_CONF_DONE)  {
		fpga_debug("high\n");
		result = 1;
	} else
		fpga_debug("low\n");

	return result;
}

int fpga_status_fn(int cookie)
{
	volatile immap_t *im = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile gpio83xx_t *gpio = (volatile gpio83xx_t *)&im->gpio[0];
	int result = 0;

	fpga_debug("STATUS check ... ");
	if (gpio->dat & FPGA_STATUS)  {
		fpga_debug("high\n");
		result = 1;
	} else
		fpga_debug("low\n");

	return result;
}

int fpga_clk_fn(int assert_clk, int flush, int cookie)
{
	volatile immap_t *im = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile gpio83xx_t *gpio = (volatile gpio83xx_t *)&im->gpio[0];
	u32 dvo = gpio->dat;

	fpga_debug("CLOCK %s\n", assert_clk ? "high" : "low");
	if (assert_clk)
		dvo |= FPGA_CCLK;
	else
		dvo &= ~FPGA_CCLK;

	if (flush)
		gpio->dat = dvo;

	return assert_clk;
}

static inline int _write_fpga(u8 val, int dump)
{
	volatile immap_t *im = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile gpio83xx_t *gpio = (volatile gpio83xx_t *)&im->gpio[0];
	int i;
	u32 dvo = gpio->dat;

	if (dump)
		fpga_debug("  %02x -> ", val);
	for (i = 0; i < 8; i++) {
		dvo &= ~FPGA_CCLK;
		gpio->dat = dvo;
		dvo &= ~FPGA_DIN;
		if (dump)
			fpga_debug("%d ", val&1);
		if (val & 1)
			dvo |= FPGA_DIN;
		gpio->dat = dvo;
		dvo |= FPGA_CCLK;
		gpio->dat = dvo;
		val >>= 1;
	}
	if (dump)
		fpga_debug("\n");

	return 0;
}

int fpga_wr_fn(void *buf, size_t len, int flush, int cookie)
{
	unsigned char *data = (unsigned char *) buf;
	int i;

	fpga_debug("fpga_wr: buf %p / size %d\n", buf, len);
	for (i = 0; i < len; i++)
		_write_fpga(data[i], 0);
	fpga_debug("\n");

	return FPGA_SUCCESS;
}
