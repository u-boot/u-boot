/*
 * Add by Alan Lu, 07-29-2005
 * For ATMEL AT24C16 EEPROM
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
#ifdef CFG_EEPROM_AT24C16
#undef DEBUG

void eeprom_init(void)
{
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);
#endif
}

int eeprom_read(unsigned dev_addr, unsigned offset, uchar *buffer,
			unsigned cnt)
{
	int page, count = 0, i = 0;
	page = offset / 0x100;
	i = offset % 0x100;

	while (count < cnt) {
		if (i2c_read(dev_addr|page, i++, 1, buffer+count++, 1) != 0)
			return 1;
		if (i > 0xff) {
			page++;
			i = 0;
		}
	}

	return 0;
}

/*
 * for CFG_I2C_EEPROM_ADDR_LEN == 2 (16-bit EEPROM address) offset is
 *   0x000nxxxx for EEPROM address selectors at n, offset xxxx in EEPROM.
 *
 * for CFG_I2C_EEPROM_ADDR_LEN == 1 (8-bit EEPROM page address) offset is
 *   0x00000nxx for EEPROM address selectors and page number at n.
 */
int eeprom_write(unsigned dev_addr, unsigned offset, uchar *buffer,
			unsigned cnt)
{
	int page, i = 0, count = 0;

	page = offset / 0x100;
	i = offset % 0x100;

	while (count < cnt) {
		if (i2c_write(dev_addr|page, i++, 1, buffer+count++, 1) != 0)
			return 1;
		if (i > 0xff) {
			page++;
			i = 0;
		}
	}

#if defined(CFG_EEPROM_PAGE_WRITE_DELAY_MS)
	udelay(CFG_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
#endif

	return 0;
}

#ifndef CONFIG_SPI
int eeprom_probe(unsigned dev_addr, unsigned offset)
{
	unsigned char chip;

	/* Probe the chip address */
#if CFG_I2C_EEPROM_ADDR_LEN == 1 && !defined(CONFIG_SPI_X)
	chip = offset >> 8; /* block number */
#else
	chip = offset >> 16; /* block number */
#endif /* CFG_I2C_EEPROM_ADDR_LEN, CONFIG_SPI_X */

	chip |= dev_addr; /* insert device address */
	return (i2c_probe(chip));
}
#endif
#endif
