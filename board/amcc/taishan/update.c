/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
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

int do_update_boot_eeprom(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	return update_boot_eeprom();
}

U_BOOT_CMD(update_boot_eeprom, 1, 1, do_update_boot_eeprom,
	   "update bootstrap eeprom content", "");
#endif
