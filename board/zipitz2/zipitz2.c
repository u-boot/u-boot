/*
 * Copyright (C) 2009
 * Marek Vasut <marek.vasut@gmail.com>
 *
 * Heavily based on pxa255_idp platform
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
#include <command.h>
#include <serial.h>
#include <asm/arch/hardware.h>
#include <spi.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef	CONFIG_CMD_SPI
void lcd_start(void);
#else
inline void lcd_start(void) {};
#endif

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */

	/* arch number of Lubbock-Board */
	gd->bd->bi_arch_number = MACH_TYPE_ZIPIT2;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	/* Enable LCD */
	lcd_start();

	return 0;
}

int board_late_init(void)
{
	setenv("stdout", "serial");
	setenv("stderr", "serial");
	return 0;
}

struct serial_device *default_serial_console (void)
{
	return &serial_stuart_device;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

#ifdef	CONFIG_CMD_SPI

struct {
	unsigned char	reg;
	unsigned short	data;
	unsigned char	mdelay;
} lcd_data[] = {
	{ 0x07,	0x0000,	0 },
	{ 0x13,	0x0000,	10 },
	{ 0x11,	0x3004,	0 },
	{ 0x14,	0x200F,	0 },
	{ 0x10,	0x1a20,	0 },
	{ 0x13,	0x0040,	50 },
	{ 0x13,	0x0060,	0 },
	{ 0x13,	0x0070,	200 },
	{ 0x01,	0x0127,	0 },
	{ 0x02,	0x0700,	0 },
	{ 0x03,	0x1030,	0 },
	{ 0x08,	0x0208,	0 },
	{ 0x0B,	0x0620,	0 },
	{ 0x0C,	0x0110,	0 },
	{ 0x30,	0x0120,	0 },
	{ 0x31,	0x0127,	0 },
	{ 0x32,	0x0000,	0 },
	{ 0x33,	0x0503,	0 },
	{ 0x34,	0x0727,	0 },
	{ 0x35,	0x0124,	0 },
	{ 0x36,	0x0706,	0 },
	{ 0x37,	0x0701,	0 },
	{ 0x38,	0x0F00,	0 },
	{ 0x39,	0x0F00,	0 },
	{ 0x40,	0x0000,	0 },
	{ 0x41,	0x0000,	0 },
	{ 0x42,	0x013f,	0 },
	{ 0x43,	0x0000,	0 },
	{ 0x44,	0x013f,	0 },
	{ 0x45,	0x0000,	0 },
	{ 0x46,	0xef00,	0 },
	{ 0x47,	0x013f,	0 },
	{ 0x48,	0x0000,	0 },
	{ 0x07,	0x0015,	30 },
	{ 0x07,	0x0017,	0 },
	{ 0x20,	0x0000,	0 },
	{ 0x21,	0x0000,	0 },
	{ 0x22,	0x0000,	0 },
};

void zipitz2_spi_sda(int set)
{
	/* GPIO 13 */
	if (set)
		GPSR0 = (1 << 13);
	else
		GPCR0 = (1 << 13);
}

void zipitz2_spi_scl(int set)
{
	/* GPIO 22 */
	if (set)
		GPCR0 = (1 << 22);
	else
		GPSR0 = (1 << 22);
}

unsigned char zipitz2_spi_read(void)
{
	/* GPIO 40 */
	return !!(GPLR1 & (1 << 8));
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	/* Always valid */
	return 1;
}

void spi_cs_activate(struct spi_slave *slave)
{
	/* GPIO 88 low */
	GPCR2 = (1 << 24);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	/* GPIO 88 high */
	GPSR2 = (1 << 24);

}

void lcd_start(void)
{
	int i;
	unsigned char reg[3] = { 0x74, 0x00, 0 };
	unsigned char data[3] = { 0x76, 0, 0 };
	unsigned char dummy[3] = { 0, 0, 0 };

	/* PWM2 AF */
	GAFR0_L |= 0x00800000;
	/* Enable clock to all PWM */
	CKEN |= 0x3;
	/* Configure PWM2 */
	PWM_CTRL2 = 0x4f;
	PWM_PWDUTY2 = 0x2ff;
	PWM_PERVAL2 = 792;

	/* Toggle the reset pin to reset the LCD */
	GPSR0 = (1 << 19);
	udelay(100000);
	GPCR0 = (1 << 19);
	udelay(20000);
	GPSR0 = (1 << 19);
	udelay(20000);

	/* Program the LCD init sequence */
	for (i = 0; i < sizeof(lcd_data) / sizeof(lcd_data[0]); i++) {
		reg[0] = 0x74;
		reg[1] = 0x0;
		reg[2] = lcd_data[i].reg;
		spi_xfer(NULL, 24, reg, dummy, SPI_XFER_BEGIN | SPI_XFER_END);

		data[0] = 0x76;
		data[1] = lcd_data[i].data >> 8;
		data[2] = lcd_data[i].data & 0xff;
		spi_xfer(NULL, 24, data, dummy, SPI_XFER_BEGIN | SPI_XFER_END);

		if (lcd_data[i].mdelay)
			udelay(lcd_data[i].mdelay * 1000);
	}

	GPSR0 = (1 << 11);
}
#endif
