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
#include <command.h>
#include <i2c.h>

static u8 boot_533_nor[] = {
	0x87, 0x78, 0x82, 0x52, 0x09, 0x57, 0xa0, 0x30,
	0x40, 0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
};

static u8 boot_533_nand[] = {
	0x87, 0x78, 0x82, 0x52, 0x09, 0x57, 0xd0, 0x10,
	0xa0, 0x68, 0x23, 0x58, 0x0d, 0x05, 0x00, 0x00
};

static u8 boot_667_nor[] = {
	0x87, 0x78, 0xa2, 0x52, 0x09, 0xd7, 0xa0, 0x30,
	0x40, 0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
};

static u8 boot_667_nand[] = {
	0x87, 0x78, 0xa2, 0x52, 0x09, 0xd7, 0xd0, 0x10,
	0xa0, 0x68, 0x23, 0x58, 0x0d, 0x05, 0x00, 0x00
};

static int do_bootstrap(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u8 chip;
	u8 *buf;
	int cpu_freq;

	if (argc < 3) {
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	cpu_freq = simple_strtol(argv[1], NULL, 10);
	if (!((cpu_freq == 533) || (cpu_freq == 667))) {
		printf("Unsupported cpu-frequency - only 533 and 667 supported\n");
		return 1;
	}

	/* use 0x52 as I2C EEPROM address for now */
	chip = 0x52;

	if ((strcmp(argv[2], "nor") != 0) &&
	    (strcmp(argv[2], "nand") != 0)) {
		printf("Unsupported boot-device - only nor|nand support\n");
		return 1;
	}

	if (strcmp(argv[2], "nand") == 0) {
		switch (cpu_freq) {
		default:
		case 533:
			buf = boot_533_nand;
			break;
		case 667:
			buf = boot_667_nand;
			break;
		}
	} else {
		switch (cpu_freq) {
		default:
		case 533:
			buf = boot_533_nor;
			break;
		case 667:
			buf = boot_667_nor;
			break;
		}
	}

	if (i2c_write(chip, 0, 1, buf, 16) != 0)
		printf("Error writing to EEPROM at address 0x%x\n", chip);
	udelay(CFG_EEPROM_PAGE_WRITE_DELAY_MS * 1000);

	printf("Done\n");
	printf("Please power-cycle the board for the changes to take effect\n");

	return 0;
}

U_BOOT_CMD(
	bootstrap,	3,	0,	do_bootstrap,
	"bootstrap - program the I2C bootstrap EEPROM\n",
	"<cpu-freq> <nor|nand> - program the I2C bootstrap EEPROM\n"
	);
