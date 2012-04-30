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

#include <gdsys_fpga.h>

#define CH7301_I2C_ADDR 0x75

#define ICS8N3QV01_I2C_ADDR 0x6E
#define ICS8N3QV01_FREF 114285000
#define ICS8N3QV01_FREF_LL 114285000LL
#define ICS8N3QV01_F_DEFAULT_0 156250000LL
#define ICS8N3QV01_F_DEFAULT_1 125000000LL
#define ICS8N3QV01_F_DEFAULT_2 100000000LL
#define ICS8N3QV01_F_DEFAULT_3  25175000LL

#define SIL1178_MASTER_I2C_ADDRESS 0x38
#define SIL1178_SLAVE_I2C_ADDRESS 0x39

#define PIXCLK_640_480_60 25180000

#define BASE_WIDTH 32
#define BASE_HEIGHT 16
#define BUFSIZE (BASE_WIDTH * BASE_HEIGHT)

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

#if defined(CONFIG_SYS_ICS8N3QV01) || defined(CONFIG_SYS_SIL1178)
static void fpga_iic_write(unsigned screen, u8 slave, u8 reg, u8 data)
{
	struct ihs_fpga *fpga = (struct ihs_fpga *)CONFIG_SYS_FPGA_BASE(screen);
	struct ihs_i2c *i2c = &fpga->i2c;

	while (in_le16(&fpga->extended_interrupt) & (1 << 12))
		;
	out_le16(&i2c->write_mailbox_ext, reg | (data << 8));
	out_le16(&i2c->write_mailbox, 0xc400 | (slave << 1));
}

static u8 fpga_iic_read(unsigned screen, u8 slave, u8 reg)
{
	struct ihs_fpga *fpga = (struct ihs_fpga *)CONFIG_SYS_FPGA_BASE(screen);
	struct ihs_i2c *i2c = &fpga->i2c;
	unsigned int ctr = 0;

	while (in_le16(&fpga->extended_interrupt) & (1 << 12))
		;
	out_le16(&fpga->extended_interrupt, 1 << 14);
	out_le16(&i2c->write_mailbox_ext, reg);
	out_le16(&i2c->write_mailbox, 0xc000 | (slave << 1));
	while (!(in_le16(&fpga->extended_interrupt) & (1 << 14))) {
		udelay(100000);
		if (ctr++ > 5) {
			printf("iic receive timeout\n");
			break;
		}
	}
	return in_le16(&i2c->read_mailbox_ext) >> 8;
}
#endif

#ifdef CONFIG_SYS_MPC92469AC
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

static void mpc92469ac_set(unsigned screen, unsigned int fout)
{
	struct ihs_fpga *fpga = (struct ihs_fpga *)CONFIG_SYS_FPGA_BASE(screen);
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

	out_le16(&fpga->mpc3w_control, (bitval << 9) | m);
}
#endif

#ifdef CONFIG_SYS_ICS8N3QV01

static unsigned int ics8n3qv01_get_fout_calc(unsigned screen, unsigned index)
{
	unsigned long long n;
	unsigned long long mint;
	unsigned long long mfrac;
	u8 reg_a, reg_b, reg_c, reg_d, reg_f;
	unsigned long long fout_calc;

	if (index > 3)
		return 0;

	reg_a = fpga_iic_read(screen, ICS8N3QV01_I2C_ADDR, 0 + index);
	reg_b = fpga_iic_read(screen, ICS8N3QV01_I2C_ADDR, 4 + index);
	reg_c = fpga_iic_read(screen, ICS8N3QV01_I2C_ADDR, 8 + index);
	reg_d = fpga_iic_read(screen, ICS8N3QV01_I2C_ADDR, 12 + index);
	reg_f = fpga_iic_read(screen, ICS8N3QV01_I2C_ADDR, 20 + index);

	mint = ((reg_a >> 1) & 0x1f) | (reg_f & 0x20);
	mfrac = ((reg_a & 0x01) << 17) | (reg_b << 9) | (reg_c << 1)
		| (reg_d >> 7);
	n = reg_d & 0x7f;

	fout_calc = (mint * ICS8N3QV01_FREF_LL
		     + mfrac * ICS8N3QV01_FREF_LL / 262144LL
		     + ICS8N3QV01_FREF_LL / 524288LL
		     + n / 2)
		    / n
		    * 1000000
		    / (1000000 - 100);

	return fout_calc;
}


static void ics8n3qv01_calc_parameters(unsigned int fout,
	unsigned int *_mint, unsigned int *_mfrac,
	unsigned int *_n)
{
	unsigned int n;
	unsigned int foutiic;
	unsigned int fvcoiic;
	unsigned int mint;
	unsigned long long mfrac;

	n = (2215000000U + fout / 2) / fout;
	if ((n & 1) && (n > 5))
		n -= 1;

	foutiic = fout - (fout / 10000);
	fvcoiic = foutiic * n;

	mint = fvcoiic / 114285000;
	if ((mint < 17) || (mint > 63))
		printf("ics8n3qv01_calc_parameters: cannot determine mint\n");

	mfrac = ((unsigned long long)fvcoiic % 114285000LL) * 262144LL
		/ 114285000LL;

	*_mint = mint;
	*_mfrac = mfrac;
	*_n = n;
}

static void ics8n3qv01_set(unsigned screen, unsigned int fout)
{
	unsigned int n;
	unsigned int mint;
	unsigned int mfrac;
	unsigned int fout_calc;
	unsigned long long fout_prog;
	long long off_ppm;
	u8 reg0, reg4, reg8, reg12, reg18, reg20;

	fout_calc = ics8n3qv01_get_fout_calc(screen, 1);
	off_ppm = (fout_calc - ICS8N3QV01_F_DEFAULT_1) * 1000000
		  / ICS8N3QV01_F_DEFAULT_1;
	printf("       PLL is off by %lld ppm\n", off_ppm);
	fout_prog = (unsigned long long)fout * (unsigned long long)fout_calc
		    / ICS8N3QV01_F_DEFAULT_1;
	ics8n3qv01_calc_parameters(fout_prog, &mint, &mfrac, &n);

	reg0 = fpga_iic_read(screen, ICS8N3QV01_I2C_ADDR, 0) & 0xc0;
	reg0 |= (mint & 0x1f) << 1;
	reg0 |= (mfrac >> 17) & 0x01;
	fpga_iic_write(screen, ICS8N3QV01_I2C_ADDR, 0, reg0);

	reg4 = mfrac >> 9;
	fpga_iic_write(screen, ICS8N3QV01_I2C_ADDR, 4, reg4);

	reg8 = mfrac >> 1;
	fpga_iic_write(screen, ICS8N3QV01_I2C_ADDR, 8, reg8);

	reg12 = mfrac << 7;
	reg12 |= n & 0x7f;
	fpga_iic_write(screen, ICS8N3QV01_I2C_ADDR, 12, reg12);

	reg18 = fpga_iic_read(screen, ICS8N3QV01_I2C_ADDR, 18) & 0x03;
	reg18 |= 0x20;
	fpga_iic_write(screen, ICS8N3QV01_I2C_ADDR, 18, reg18);

	reg20 = fpga_iic_read(screen, ICS8N3QV01_I2C_ADDR, 20) & 0x1f;
	reg20 |= mint & (1 << 5);
	fpga_iic_write(screen, ICS8N3QV01_I2C_ADDR, 20, reg20);
}
#endif

static int osd_write_videomem(unsigned screen, unsigned offset,
	u16 *data, size_t charcount)
{
	struct ihs_fpga *fpga =
		(struct ihs_fpga *) CONFIG_SYS_FPGA_BASE(screen);
	unsigned int k;

	for (k = 0; k < charcount; ++k) {
		if (offset + k >= BUFSIZE)
			return -1;
		out_le16(&fpga->videomem + offset + k, data[k]);
	}

	return charcount;
}

static int osd_print(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned screen;

	for (screen = 0; screen < CONFIG_SYS_OSD_SCREENS; ++screen) {
		unsigned x;
		unsigned y;
		unsigned charcount;
		unsigned len;
		u8 color;
		unsigned int k;
		u16 buf[BUFSIZE];
		char *text;
		int res;

		if (argc < 5) {
			cmd_usage(cmdtp);
			return 1;
		}

		x = simple_strtoul(argv[1], NULL, 16);
		y = simple_strtoul(argv[2], NULL, 16);
		color = simple_strtoul(argv[3], NULL, 16);
		text = argv[4];
		charcount = strlen(text);
		len = (charcount > BUFSIZE) ? BUFSIZE : charcount;

		for (k = 0; k < len; ++k)
			buf[k] = (text[k] << 8) | color;

		res = osd_write_videomem(screen, y * BASE_WIDTH + x, buf, len);
		if (res < 0)
			return res;
	}

	return 0;
}

int osd_probe(unsigned screen)
{
	struct ihs_fpga *fpga = (struct ihs_fpga *)CONFIG_SYS_FPGA_BASE(screen);
	struct ihs_osd *osd = &fpga->osd;
	u16 version = in_le16(&osd->version);
	u16 features = in_le16(&osd->features);
	unsigned width;
	unsigned height;
	u8 value;

	width = ((features & 0x3f00) >> 8) + 1;
	height = (features & 0x001f) + 1;

	printf("OSD%d:  Digital-OSD version %01d.%02d, %d" "x%d characters\n",
		screen, version/100, version%100, width, height);

#ifdef CONFIG_SYS_CH7301
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
#endif

#ifdef CONFIG_SYS_MPC92469AC
	mpc92469ac_set(screen, PIXCLK_640_480_60);
#endif

#ifdef CONFIG_SYS_ICS8N3QV01
	ics8n3qv01_set(screen, PIXCLK_640_480_60);
#endif

#ifdef CONFIG_SYS_SIL1178
	value = fpga_iic_read(screen, SIL1178_SLAVE_I2C_ADDRESS, 0x02);
	if (value != 0x06) {
		printf("       Probing CH7301 SIL1178, DEV_IDL %02x\n", value);
		return -1;
	}
	/* magic initialization sequence adapted from datasheet */
	fpga_iic_write(screen, SIL1178_SLAVE_I2C_ADDRESS, 0x08, 0x36);
	fpga_iic_write(screen, SIL1178_MASTER_I2C_ADDRESS, 0x0f, 0x44);
	fpga_iic_write(screen, SIL1178_MASTER_I2C_ADDRESS, 0x0f, 0x4c);
	fpga_iic_write(screen, SIL1178_MASTER_I2C_ADDRESS, 0x0e, 0x10);
	fpga_iic_write(screen, SIL1178_MASTER_I2C_ADDRESS, 0x0a, 0x80);
	fpga_iic_write(screen, SIL1178_MASTER_I2C_ADDRESS, 0x09, 0x30);
	fpga_iic_write(screen, SIL1178_MASTER_I2C_ADDRESS, 0x0c, 0x89);
	fpga_iic_write(screen, SIL1178_MASTER_I2C_ADDRESS, 0x0d, 0x60);
	fpga_iic_write(screen, SIL1178_MASTER_I2C_ADDRESS, 0x08, 0x36);
	fpga_iic_write(screen, SIL1178_MASTER_I2C_ADDRESS, 0x08, 0x37);
#endif

	out_le16(&fpga->videocontrol, 0x0002);
	out_le16(&osd->control, 0x0049);

	out_le16(&osd->xy_size, ((32 - 1) << 8) | (16 - 1));
	out_le16(&osd->x_pos, 0x007f);
	out_le16(&osd->y_pos, 0x005f);

	return 0;
}

int osd_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned screen;

	for (screen = 0; screen < CONFIG_SYS_OSD_SCREENS; ++screen) {
		unsigned x;
		unsigned y;
		unsigned k;
		u16 buffer[BASE_WIDTH];
		char *rp;
		u16 *wp = buffer;
		unsigned count = (argc > 4) ?
			simple_strtoul(argv[4], NULL, 16) : 1;

		if ((argc < 4) || (strlen(argv[3]) % 4)) {
			cmd_usage(cmdtp);
			return 1;
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
			unsigned offset =
				y * BASE_WIDTH + x + k * (wp - buffer);
			osd_write_videomem(screen, offset, buffer,
				wp - buffer);
		}
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
