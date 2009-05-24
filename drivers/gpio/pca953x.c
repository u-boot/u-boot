/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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

/*
 * Driver for NXP's 4 and 8 bit I2C gpio expanders (eg pca9537, pca9557, etc)
 * TODO: support additional devices with more than 8-bits GPIO
 */

#include <common.h>
#include <i2c.h>
#include <pca953x.h>

/* Default to an address that hopefully won't corrupt other i2c devices */
#ifndef CONFIG_SYS_I2C_PCA953X_ADDR
#define CONFIG_SYS_I2C_PCA953X_ADDR	(~0)
#endif

enum {
	PCA953X_CMD_INFO,
	PCA953X_CMD_DEVICE,
	PCA953X_CMD_OUTPUT,
	PCA953X_CMD_INPUT,
	PCA953X_CMD_INVERT,
};

/*
 * Modify masked bits in register
 */
static int pca953x_reg_write(uint8_t chip, uint addr, uint mask, uint data)
{
	uint8_t val;

	if (i2c_read(chip, addr, 1, &val, 1))
		return -1;

	val &= ~mask;
	val |= data;

	return i2c_write(chip, addr, 1, &val, 1);
}

/*
 * Set output value of IO pins in 'mask' to corresponding value in 'data'
 * 0 = low, 1 = high
 */
int pca953x_set_val(uint8_t chip, uint mask, uint data)
{
	return pca953x_reg_write(chip, PCA953X_OUT, mask, data);
}

/*
 * Set read polarity of IO pins in 'mask' to corresponding value in 'data'
 * 0 = read pin value, 1 = read inverted pin value
 */
int pca953x_set_pol(uint8_t chip, uint mask, uint data)
{
	return pca953x_reg_write(chip, PCA953X_POL, mask, data);
}

/*
 * Set direction of IO pins in 'mask' to corresponding value in 'data'
 * 0 = output, 1 = input
 */
int pca953x_set_dir(uint8_t chip, uint mask, uint data)
{
	return pca953x_reg_write(chip, PCA953X_CONF, mask, data);
}

/*
 * Read current logic level of all IO pins
 */
int pca953x_get_val(uint8_t chip)
{
	uint8_t val;

	if (i2c_read(chip, 0, 1, &val, 1))
		return -1;

	return (int)val;
}

#ifdef CONFIG_CMD_PCA953X
#ifdef CONFIG_CMD_PCA953X_INFO
/*
 * Display pca953x information
 */
static int pca953x_info(uint8_t chip)
{
	int i;
	uint8_t data;

	printf("pca953x@ 0x%x:\n\n", chip);
	printf("gpio pins: 76543210\n");
	printf("-------------------\n");

	if (i2c_read(chip, PCA953X_CONF, 1, &data, 1))
		return -1;
	printf("conf:      ");
	for (i = 7; i >= 0; i--)
		printf("%c", data & (1 << i) ? 'i' : 'o');
	printf("\n");

	if (i2c_read(chip, PCA953X_POL, 1, &data, 1))
		return -1;
	printf("invert:    ");
	for (i = 7; i >= 0; i--)
		printf("%c", data & (1 << i) ? '1' : '0');
	printf("\n");

	if (i2c_read(chip, PCA953X_IN, 1, &data, 1))
		return -1;
	printf("input:     ");
	for (i = 7; i >= 0; i--)
		printf("%c", data & (1 << i) ? '1' : '0');
	printf("\n");

	if (i2c_read(chip, PCA953X_OUT, 1, &data, 1))
		return -1;
	printf("output:    ");
	for (i = 7; i >= 0; i--)
		printf("%c", data & (1 << i) ? '1' : '0');
	printf("\n");

	return 0;
}
#endif /* CONFIG_CMD_PCA953X_INFO */

cmd_tbl_t cmd_pca953x[] = {
	U_BOOT_CMD_MKENT(device, 3, 0, (void *)PCA953X_CMD_DEVICE, "", ""),
	U_BOOT_CMD_MKENT(output, 4, 0, (void *)PCA953X_CMD_OUTPUT, "", ""),
	U_BOOT_CMD_MKENT(input, 3, 0, (void *)PCA953X_CMD_INPUT, "", ""),
	U_BOOT_CMD_MKENT(invert, 4, 0, (void *)PCA953X_CMD_INVERT, "", ""),
#ifdef CONFIG_CMD_PCA953X_INFO
	U_BOOT_CMD_MKENT(info, 2, 0, (void *)PCA953X_CMD_INFO, "", ""),
#endif
};

int do_pca953x(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	static uint8_t chip = CONFIG_SYS_I2C_PCA953X_ADDR;
	int val;
	ulong ul_arg2 = 0;
	ulong ul_arg3 = 0;
	cmd_tbl_t *c;

	c = find_cmd_tbl(argv[1], cmd_pca953x, ARRAY_SIZE(cmd_pca953x));

	/* All commands but "device" require 'maxargs' arguments */
	if (!c || !((argc == (c->maxargs)) ||
		(((int)c->cmd == PCA953X_CMD_DEVICE) &&
		 (argc == (c->maxargs - 1))))) {
		cmd_usage(cmdtp);
		return 1;
	}

	/* arg2 used as chip number or pin number */
	if (argc > 2)
		ul_arg2 = simple_strtoul(argv[2], NULL, 16);

	/* arg3 used as pin or invert value */
	if (argc > 3)
		ul_arg3 = simple_strtoul(argv[3], NULL, 16) & 0x1;

	switch ((int)c->cmd) {
#ifdef CONFIG_CMD_PCA953X_INFO
	case PCA953X_CMD_INFO:
		return pca953x_info(chip);
#endif
	case PCA953X_CMD_DEVICE:
		if (argc == 3)
			chip = (uint8_t)ul_arg2;
		printf("Current device address: 0x%x\n", chip);
		return 0;
	case PCA953X_CMD_INPUT:
		pca953x_set_dir(chip, (1 << ul_arg2),
				PCA953X_DIR_IN << ul_arg2);
		val = (pca953x_get_val(chip) & (1 << ul_arg2)) != 0;

		printf("chip 0x%02x, pin 0x%lx = %d\n", chip, ul_arg2, val);
		return val;
	case PCA953X_CMD_OUTPUT:
		pca953x_set_dir(chip, (1 << ul_arg2),
				(PCA953X_DIR_OUT << ul_arg2));
		return pca953x_set_val(chip, (1 << ul_arg2),
					(ul_arg3 << ul_arg2));
	case PCA953X_CMD_INVERT:
		return pca953x_set_pol(chip, (1 << ul_arg2),
					(ul_arg3 << ul_arg2));
	default:
		/* We should never get here */
		return 1;
	}
}

U_BOOT_CMD(
	pca953x,	5,	1,	do_pca953x,
	"pca953x gpio access",
	"device [dev]\n"
	"	- show or set current device address\n"
#ifdef CONFIG_CMD_PCA953X_INFO
	"pca953x info\n"
	"	- display info for current chip\n"
#endif
	"pca953x output pin 0|1\n"
	"	- set pin as output and drive low or high\n"
	"pca953x invert pin 0|1\n"
	"	- disable/enable polarity inversion for reads\n"
	"pca953x intput pin\n"
	"	- set pin as input and read value"
);

#endif /* CONFIG_CMD_PCA953X */
