/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <i2c.h>

#if defined(CONFIG_TAISHAN)

const uchar bootstrap_buf[16] = {
	0x86,
	0x78,
	0xc1,
	0xa6,
	0x09,
	0x67,
	0x04,
	0x63,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00
};

static int update_boot_eeprom(void)
{
	ulong len = 0x10;
	uchar chip = CONFIG_SYS_BOOTSTRAP_IIC_ADDR;
	uchar *pbuf = (uchar *)bootstrap_buf;
	int ii, jj;

	for (ii = 0; ii < len; ii++) {
		if (i2c_write(chip, ii, 1, &pbuf[ii], 1) != 0) {
			printf("i2c_write failed\n");
			return -1;
		}

		/* wait 10ms */
		for (jj = 0; jj < 10; jj++)
			udelay(1000);
	}
	return 0;
}

int do_update_boot_eeprom(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	return update_boot_eeprom();
}

U_BOOT_CMD(update_boot_eeprom, 1, 1, do_update_boot_eeprom,
	   "update bootstrap eeprom content", "");
#endif
