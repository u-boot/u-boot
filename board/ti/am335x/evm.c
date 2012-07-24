/*
 * evm.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <errno.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/common_def.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * I2C Address of on-board EEPROM
 */
#define I2C_BASE_BOARD_ADDR	0x50

#define NO_OF_MAC_ADDR          3
#define ETH_ALEN		6

#define NAME_LEN	8

struct am335x_baseboard_id {
	unsigned int  magic;
	char name[NAME_LEN];
	char version[4];
	char serial[12];
	char config[32];
	char mac_addr[NO_OF_MAC_ADDR][ETH_ALEN];
};

static struct am335x_baseboard_id header;

static inline int board_is_bone(void)
{
	return !strncmp(header.name, "A335BONE", NAME_LEN);
}

/*
 * Read header information from EEPROM into global structure.
 */
int read_eeprom(void)
{
	/* Check if baseboard eeprom is available */
	if (i2c_probe(I2C_BASE_BOARD_ADDR)) {
		printf("Could not probe the EEPROM; something fundamentally "
			"wrong on the I2C bus.\n");
		return -ENODEV;
	}

	/* read the eeprom using i2c */
	if (i2c_read(I2C_BASE_BOARD_ADDR, 0, 2, (uchar *)&header,
							sizeof(header))) {
		printf("Could not read the EEPROM; something fundamentally"
			" wrong on the I2C bus.\n");
		return -EIO;
	}

	if (header.magic != 0xEE3355AA) {
		/*
		 * read the eeprom using i2c again,
		 * but use only a 1 byte address
		 */
		if (i2c_read(I2C_BASE_BOARD_ADDR, 0, 1, (uchar *)&header,
							sizeof(header))) {
			printf("Could not read the EEPROM; something "
				"fundamentally wrong on the I2C bus.\n");
			return -EIO;
		}

		if (header.magic != 0xEE3355AA) {
			printf("Incorrect magic number in EEPROM\n");
			return -EINVAL;
		}
	}

	return 0;
}

/*
 * Basic board specific setup
 */
int board_init(void)
{
	enable_uart0_pin_mux();

	enable_i2c0_pin_mux();
	enable_i2c1_pin_mux();
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	if (read_eeprom() < 0)
		printf("Could not get board ID.\n");

	gd->bd->bi_boot_params = PHYS_DRAM_1 + 0x100;

	return 0;
}
