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
#include <asm/io.h>

/*
 * There are 2 versions of production Sequoia & Rainier platforms.
 * The primary difference is the reference clock. Those with
 * 33333333 reference clocks will also have 667MHz rated
 * processors. Not enough differences to have unique clock
 * settings.
 *
 * NOR and NAND boot options change bytes 6, 7, 8, 9, 11. The
 * values are independent of the rest of the clock settings.
 *
 * All Sequoias & Rainiers select from two possible EEPROMs in Boot
 * Config F. One for 33MHz PCI, one for 66MHz PCI. The following
 * values are for the 33MHz PCI configuration. Byte 5 (0 base) is
 * the only value affected for a 33MHz PCI and simply needs a | 0x08.
 */

#define NAND_COMPATIBLE	0x01
#define NOR_COMPATIBLE  0x02

/* check with Stefan on CFG_I2C_EEPROM_ADDR */
#define I2C_EEPROM_ADDR 0x52

static char *config_labels[] = {
	"CPU: 333 PLB: 133 OPB: 66 EBC: 66",
	"CPU: 333 PLB: 166 OPB: 83 EBC: 55",
	"CPU: 400 PLB: 133 OPB: 66 EBC: 66",
	"CPU: 400 PLB: 160 OPB: 80 EBC: 53",
	"CPU: 416 PLB: 166 OPB: 83 EBC: 55",
	"CPU: 500 PLB: 166 OPB: 83 EBC: 55",
	"CPU: 533 PLB: 133 OPB: 66 EBC: 66",
	"CPU: 667 PLB: 133 OPB: 66 EBC: 66",
	"CPU: 667 PLB: 166 OPB: 83 EBC: 55",
	NULL
};

static u8 boot_configs[][17] = {
	{
		(NOR_COMPATIBLE),
		0x84, 0x70, 0xa2, 0xa6, 0x05, 0x57, 0xa0, 0x10, 0x40,
		0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
	},
	{
		(NAND_COMPATIBLE | NOR_COMPATIBLE),
		0xc7, 0x78, 0xf3, 0x4e, 0x05, 0xd7, 0xa0, 0x30, 0x40,
		0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
	},
	{
		(NOR_COMPATIBLE),
		0x86, 0x78, 0xc2, 0xc6, 0x05, 0x57, 0xa0, 0x30, 0x40,
		0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
	},
	{
		(NOR_COMPATIBLE),
		0x86, 0x78, 0xc2, 0xa6, 0x05, 0xd7, 0xa0, 0x10, 0x40,
		0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
	},
	{
		(NAND_COMPATIBLE | NOR_COMPATIBLE),
		0xc6, 0x78, 0x52, 0xa6, 0x05, 0xd7, 0xa0, 0x10, 0x40,
		0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
	},
	{
		(NAND_COMPATIBLE | NOR_COMPATIBLE),
		0xc7, 0x78, 0x52, 0xc6, 0x05, 0xd7, 0xa0, 0x30, 0x40,
		0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
	},
	{
		(NOR_COMPATIBLE),
		0x87, 0x78, 0x82, 0x52, 0x09, 0x57, 0xa0, 0x30, 0x40,
		0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
	},
	{
		(NOR_COMPATIBLE),
		0x87, 0x78, 0xa2, 0x56, 0x09, 0x57, 0xa0, 0x30, 0x40,
		0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
	},
	{
		(NAND_COMPATIBLE | NOR_COMPATIBLE),
		0x87, 0x78, 0xa2, 0x52, 0x09, 0xd7, 0xa0, 0x30, 0x40,
		0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
	},
	{
		0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	}
};

/*
 * Bytes 6,8,9,11 change for NAND boot
 */
static u8 nand_boot[] = {
	0xd0,  0xa0, 0x68, 0x58
};

static int do_bootstrap(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u8 *buf, bNAND;
	int x, y, nbytes, selcfg;
	extern char console_buffer[];

	if (argc < 2) {
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if ((strcmp(argv[1], "nor") != 0) &&
	    (strcmp(argv[1], "nand") != 0)) {
		printf("Unsupported boot-device - only nor|nand support\n");
		return 1;
	}

	/* set the nand flag based on provided input */
	if ((strcmp(argv[1], "nand") == 0))
		bNAND = 1;
	else
		bNAND = 0;

	printf("Available configurations: \n\n");

	if (bNAND) {
		for(x = 0, y = 0; boot_configs[x][0] != 0; x++) {
			/* filter on nand compatible */
			if (boot_configs[x][0] & NAND_COMPATIBLE) {
				printf(" %d - %s\n", (y+1), config_labels[x]);
				y++;
			}
		}
	} else {
		for(x = 0, y = 0; boot_configs[x][0] != 0; x++) {
			/* filter on nor compatible */
			if (boot_configs[x][0] & NOR_COMPATIBLE) {
				printf(" %d - %s\n", (y+1), config_labels[x]);
				y++;
			}
		}
	}

	do {
		nbytes = readline(" Selection [1-x / quit]: ");

		if (nbytes) {
			if (strcmp(console_buffer, "quit") == 0)
				return 0;
			selcfg = simple_strtol(console_buffer, NULL, 10);
			if ((selcfg < 1) || (selcfg > y))
				nbytes = 0;
		}
	} while (nbytes == 0);


	y = (selcfg - 1);

	for (x = 0; boot_configs[x][0] != 0; x++) {
		if (bNAND) {
			if (boot_configs[x][0] & NAND_COMPATIBLE) {
				if (y > 0)
					y--;
				else if (y < 1)
					break;
			}
		} else {
			if (boot_configs[x][0] & NOR_COMPATIBLE) {
				if (y > 0)
					y--;
				else if (y < 1)
					break;
			}
		}
	}

	buf = &boot_configs[x][1];

	if (bNAND) {
		buf[6] = nand_boot[0];
		buf[8] = nand_boot[1];
		buf[9] = nand_boot[2];
		buf[11] = nand_boot[3];
	}

	/* check CPLD register +5 for PCI 66MHz flag */
	if ((in_8((void *)(CFG_BCSR_BASE + 5)) & CFG_BCSR5_PCI66EN) == 0)
		/*
		 * PLB-to-PCI divisor = 3 for 33MHz sync PCI
		 * instead of 2 for 66MHz systems
		 */
		buf[5] |= 0x08;

	if (i2c_write(I2C_EEPROM_ADDR, 0, 1, buf, 16) != 0)
		printf("Error writing to EEPROM at address 0x%x\n", I2C_EEPROM_ADDR);
	udelay(CFG_EEPROM_PAGE_WRITE_DELAY_MS * 1000);

	printf("Done\n");
	printf("Please power-cycle the board for the changes to take effect\n");

	return 0;
}

U_BOOT_CMD(
	bootstrap,	2,	0,	do_bootstrap,
	"bootstrap - program the I2C bootstrap EEPROM\n",
	"<nand|nor> - strap to boot from NAND or NOR flash\n"
	);
