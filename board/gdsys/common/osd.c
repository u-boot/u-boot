/*
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
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
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>

#include "fpga.h"

#define CH7301_I2C_ADDR 0x75

#define PIXCLK_640_480_60 25180000

#define BASE_WIDTH 32
#define BASE_HEIGHT 16
#define BUFSIZE (BASE_WIDTH * BASE_HEIGHT)

enum {
	REG_CONTROL = 0x0010,
	REG_MPC3W_CONTROL = 0x001a,
	REG_VIDEOCONTROL = 0x0042,
	REG_OSDVERSION = 0x0100,
	REG_OSDFEATURES = 0x0102,
	REG_OSDCONTROL = 0x0104,
	REG_XY_SIZE = 0x0106,
	REG_VIDEOMEM = 0x0800,
};

enum {
	CH7301_CM = 0x1c,		/* Clock Mode Register */
	CH7301_IC = 0x1d,		/* Input Clock Register */
	CH7301_GPIO = 0x1e,		/* GPIO Control Register */
	CH7301_IDF = 0x1f,		/* Input Data Format Register */
	CH7301_CD = 0x20,		/* Connection Detect Register */
	CH7301_DC = 0x21,		/* DAC Control Register */
	CH7301_HPD = 0x23,		/* Hot Plug Detection Register */
	CH7301_TCTL = 0x31,		/* DVI Control Input Register */
	CH7301_TPCP = 0x33,		/* DVI PLL Charge Pump Ctrl Register */
	CH7301_TPD = 0x34,		/* DVI PLL Divide Register */
	CH7301_TPVT = 0x35,		/* DVI PLL Supply Control Register */
	CH7301_TPF = 0x36,		/* DVI PLL Filter Register */
	CH7301_TCT = 0x37,		/* DVI Clock Test Register */
	CH7301_TSTP = 0x48,		/* Test Pattern Register */
	CH7301_PM = 0x49,		/* Power Management register */
	CH7301_VID = 0x4a,		/* Version ID Register */
	CH7301_DID = 0x4b,		/* Device ID Register */
	CH7301_DSP = 0x56,		/* DVI Sync polarity Register */
};

static void mpc92469ac_calc_parameters(unsigned int fout,
	unsigned int *post_div, unsigned int *feedback_div)
{
	unsigned int n = *post_div;
	unsigned int m = *feedback_div;
	unsigned int a;
	unsigned int b = 14745600 / 16;

	if (fout < 50169600)
		n = 8;
	else if (fout < 100339199)
		n = 4;
	else if (fout < 200678399)
		n = 2;
	else
		n = 1;

	a = fout * n + (b / 2); /* add b/2 for proper rounding */

	m = a / b;

	*post_div = n;
	*feedback_div = m;
}

static void mpc92469ac_set(unsigned int fout)
{
	unsigned int n;
	unsigned int m;
	unsigned int bitval = 0;
	mpc92469ac_calc_parameters(fout, &n, &m);

	switch (n) {
	case 1:
		bitval = 0x00;
		break;
	case 2:
		bitval = 0x01;
		break;
	case 4:
		bitval = 0x02;
		break;
	case 8:
		bitval = 0x03;
		break;
	}

	fpga_set_reg(REG_MPC3W_CONTROL, (bitval << 9) | m);
}

static int osd_write_videomem(unsigned offset, u16 *data, size_t charcount)
{
	unsigned int k;

	for (k = 0; k < charcount; ++k) {
		if (offset + k >= BUFSIZE)
			return -1;
		fpga_set_reg(REG_VIDEOMEM + 2 * (offset + k), data[k]);
	}

	return charcount;
}

static int osd_print(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned x;
	unsigned y;
	unsigned charcount;
	unsigned len;
	u8 color;
	unsigned int k;
	u16 buf[BUFSIZE];
	char *text;

	if (argc < 5) {
		return cmd_usage(cmdtp);
	}

	x = simple_strtoul(argv[1], NULL, 16);
	y = simple_strtoul(argv[2], NULL, 16);
	color = simple_strtoul(argv[3], NULL, 16);
	text = argv[4];
	charcount = strlen(text);
	len = (charcount > BUFSIZE) ? BUFSIZE : charcount;

	for (k = 0; k < len; ++k)
		buf[k] = (text[k] << 8) | color;

	return osd_write_videomem(y * BASE_WIDTH + x, buf, len);
}

int osd_probe(void)
{
	u8 value;
	u16 version = fpga_get_reg(REG_OSDVERSION);
	u16 features = fpga_get_reg(REG_OSDFEATURES);
	unsigned width;
	unsigned height;

	width = ((features & 0x3f00) >> 8) + 1;
	height = (features & 0x001f) + 1;

	printf("OSD:   Digital-OSD version %01d.%02d, %d" "x%d characters\n",
		version/100, version%100, width, height);

	value = i2c_reg_read(CH7301_I2C_ADDR, CH7301_DID);
	if (value != 0x17) {
		printf("       Probing CH7301 failed, DID %02x\n", value);
		return -1;
	}
	i2c_reg_write(CH7301_I2C_ADDR, CH7301_TPCP, 0x08);
	i2c_reg_write(CH7301_I2C_ADDR, CH7301_TPD, 0x16);
	i2c_reg_write(CH7301_I2C_ADDR, CH7301_TPF, 0x60);
	i2c_reg_write(CH7301_I2C_ADDR, CH7301_DC, 0x09);
	i2c_reg_write(CH7301_I2C_ADDR, CH7301_PM, 0xc0);

	mpc92469ac_set(PIXCLK_640_480_60);
	fpga_set_reg(REG_VIDEOCONTROL, 0x0002);
	fpga_set_reg(REG_OSDCONTROL, 0x0049);

	fpga_set_reg(REG_XY_SIZE, ((32 - 1) << 8) | (16 - 1));

	return 0;
}

int osd_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned x;
	unsigned y;
	unsigned k;
	u16 buffer[BASE_WIDTH];
	char *rp;
	u16 *wp = buffer;
	unsigned count = (argc > 4) ?  simple_strtoul(argv[4], NULL, 16) : 1;

	if ((argc < 4) || (strlen(argv[3]) % 4)) {
		return cmd_usage(cmdtp);
	}

	x = simple_strtoul(argv[1], NULL, 16);
	y = simple_strtoul(argv[2], NULL, 16);
	rp = argv[3];


	while (*rp) {
		char substr[5];

		memcpy(substr, rp, 4);
		substr[4] = 0;
		*wp = simple_strtoul(substr, NULL, 16);

		rp += 4;
		wp++;
		if (wp - buffer > BASE_WIDTH)
			break;
	}

	for (k = 0; k < count; ++k) {
		unsigned offset = y * BASE_WIDTH + x + k * (wp - buffer);
		osd_write_videomem(offset, buffer, wp - buffer);
	}

	return 0;
}

U_BOOT_CMD(
	osdw, 5, 0, osd_write,
	"write 16-bit hex encoded buffer to osd memory",
	"pos_x pos_y buffer count\n"
);

U_BOOT_CMD(
	osdp, 5, 0, osd_print,
	"write ASCII buffer to osd memory",
	"pos_x pos_y color text\n"
);
