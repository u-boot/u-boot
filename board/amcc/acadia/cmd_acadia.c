/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <i2c.h>

static u8 boot_267_nor[] = {
	0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8e, 0x00,
	0x14, 0xc0, 0x36, 0xcc, 0x00, 0x0c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

static u8 boot_267_nand[] = {
	0xd0, 0x38, 0xc3, 0x50, 0x13, 0x88, 0x8e, 0x00,
	0x14, 0xc0, 0x36, 0xcc, 0x00, 0x0c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

static int do_bootstrap(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u8 chip;
	u8 *buf;
	int cpu_freq;

	if (argc < 3)
		return cmd_usage(cmdtp);

	cpu_freq = simple_strtol(argv[1], NULL, 10);
	if (cpu_freq != 267) {
		printf("Unsupported cpu-frequency - only 267 supported\n");
		return 1;
	}

	/* use 0x50 as I2C EEPROM address for now */
	chip = 0x50;

	if ((strcmp(argv[2], "nor") != 0) &&
	    (strcmp(argv[2], "nand") != 0)) {
		printf("Unsupported boot-device - only nor|nand support\n");
		return 1;
	}

	if (strcmp(argv[2], "nand") == 0) {
		switch (cpu_freq) {
		case 267:
			buf = boot_267_nand;
			break;
		default:
			break;
		}
	} else {
		switch (cpu_freq) {
		case 267:
			buf = boot_267_nor;
			break;
		default:
			break;
		}
	}

	if (i2c_write(chip, 0, 1, buf, 16) != 0)
		printf("Error writing to EEPROM at address 0x%x\n", chip);
	udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
	if (i2c_write(chip, 0x10, 1, buf+16, 4) != 0)
		printf("Error2 writing to EEPROM at address 0x%x\n", chip);

	printf("Done\n");
	printf("Please power-cycle the board for the changes to take effect\n");

	return 0;
}

U_BOOT_CMD(
	bootstrap,	3,	0,	do_bootstrap,
	"program the I2C bootstrap EEPROM",
	"<cpu-freq> <nor|nand> - program the I2C bootstrap EEPROM"
);
