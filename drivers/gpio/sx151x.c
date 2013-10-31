/*
 * (C) Copyright 2013
 * Viktar Palstsiuk, Promwad, viktar.palstsiuk@promwad.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Driver for Semtech SX151x SPI GPIO Expanders
 */

#include <common.h>
#include <spi.h>
#include <sx151x.h>

#ifndef CONFIG_SX151X_SPI_BUS
#define CONFIG_SX151X_SPI_BUS 0
#endif

/*
 * The SX151x registers
 */

#ifdef CONFIG_SX151X_GPIO_COUNT_8
/* 8bit: SX1511 */
#define SX151X_REG_DIR		0x07
#define SX151X_REG_DATA		0x08
#else
/* 16bit: SX1512 */
#define SX151X_REG_DIR		0x0F
#define SX151X_REG_DATA		0x11
#endif
#define SX151X_REG_RESET	0x7D

static int sx151x_spi_write(int chip, unsigned char reg, unsigned char val)
{
	struct spi_slave *slave;
	unsigned char buf[2];
	int ret;

	slave = spi_setup_slave(CONFIG_SX151X_SPI_BUS, chip, 1000000,
				SPI_MODE_0);
	if (!slave)
		return 0;

	spi_claim_bus(slave);

	buf[0] = reg;
	buf[1] = val;

	ret = spi_xfer(slave, 16, buf, NULL, SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret < 0)
		printf("spi%d.%d write fail: can't write %02x to %02x: %d\n",
			CONFIG_SX151X_SPI_BUS, chip, val, reg, ret);
	else
		printf("spi%d.%d write 0x%02x to register 0x%02x\n",
		       CONFIG_SX151X_SPI_BUS, chip, val, reg);
	spi_release_bus(slave);
	spi_free_slave(slave);

	return ret;
}

static int sx151x_spi_read(int chip, unsigned char reg)
{
	struct spi_slave *slave;
	int ret;

	slave = spi_setup_slave(CONFIG_SX151X_SPI_BUS, chip, 1000000,
				SPI_MODE_0);
	if (!slave)
		return 0;

	spi_claim_bus(slave);

	ret = spi_w8r8(slave, reg | 0x80);
	if (ret < 0)
		printf("spi%d.%d read fail: can't read %02x: %d\n",
			CONFIG_SX151X_SPI_BUS, chip, reg, ret);
	else
		printf("spi%d.%d read register 0x%02x: 0x%02x\n",
		       CONFIG_SX151X_SPI_BUS, chip, reg, ret);

	spi_release_bus(slave);
	spi_free_slave(slave);

	return ret;
}

static inline void sx151x_find_cfg(int gpio, unsigned char *reg, unsigned char *mask)
{
	*reg   -= gpio / 8;
	*mask   = 1 << (gpio % 8);
}

static int sx151x_write_cfg(int chip, unsigned char gpio, unsigned char reg, int val)
{
	unsigned char  mask;
	unsigned char  data;
	int ret;

	sx151x_find_cfg(gpio, &reg, &mask);
	ret = sx151x_spi_read(chip, reg);
	if (ret < 0)
		return ret;
	else
		data = ret;
	data &= ~mask;
	data |= (val << (gpio % 8)) & mask;
	return sx151x_spi_write(chip, reg, data);
}

int sx151x_get_value(int chip, int gpio)
{
	unsigned char  reg = SX151X_REG_DATA;
	unsigned char  mask;
	int ret;

	sx151x_find_cfg(gpio, &reg, &mask);
	ret = sx151x_spi_read(chip, reg);
	if (ret >= 0)
		ret = (ret & mask) != 0 ? 1 : 0;

	return ret;
}

int sx151x_set_value(int chip, int gpio, int val)
{
	return sx151x_write_cfg(chip, gpio, SX151X_REG_DATA, (val ? 1 : 0));
}

int sx151x_direction_input(int chip, int gpio)
{
	return sx151x_write_cfg(chip, gpio, SX151X_REG_DIR, 1);
}

int sx151x_direction_output(int chip, int gpio)
{
	return sx151x_write_cfg(chip, gpio, SX151X_REG_DIR, 0);
}

int sx151x_reset(int chip)
{
	int err;

	err = sx151x_spi_write(chip, SX151X_REG_RESET, 0x12);
	if (err < 0)
		return err;

	err = sx151x_spi_write(chip, SX151X_REG_RESET, 0x34);
	return err;
}

#ifdef CONFIG_CMD_SX151X

int do_sx151x(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = CMD_RET_USAGE, chip = 0, gpio = 0, val = 0;

	if (argc < 3)
		return CMD_RET_USAGE;

	/* arg2 used as chip number */
	chip = simple_strtoul(argv[2], NULL, 10);

	if (strcmp(argv[1], "reset") == 0) {
		ret = sx151x_reset(chip);
		if (!ret) {
			printf("Device at spi%d.%d was reset\n",
			       CONFIG_SX151X_SPI_BUS, chip);
		}
		return ret;
	}

	if (argc < 4)
		return CMD_RET_USAGE;

	/* arg3 used as gpio number */
	gpio = simple_strtoul(argv[3], NULL, 10);

	if (strcmp(argv[1], "get") == 0) {
		ret = sx151x_get_value(chip, gpio);
		if (ret < 0)
			printf("Failed to get value at spi%d.%d gpio %d\n",
			       CONFIG_SX151X_SPI_BUS, chip, gpio);
		else {
			printf("Value at spi%d.%d gpio %d is %d\n",
			       CONFIG_SX151X_SPI_BUS, chip, gpio, ret);
			ret = 0;
		}
		return ret;
	}

	if (argc < 5)
		return CMD_RET_USAGE;

	/* arg4 used as value or direction */
	val = simple_strtoul(argv[4], NULL, 10);

	if (strcmp(argv[1], "set") == 0) {
		ret = sx151x_set_value(chip, gpio, val);
		if (ret < 0)
			printf("Failed to set value at spi%d.%d gpio %d\n",
			       CONFIG_SX151X_SPI_BUS, chip, gpio);
		else
			printf("New value at spi%d.%d gpio %d is %d\n",
			       CONFIG_SX151X_SPI_BUS, chip, gpio, val);
		return ret;
	} else if (strcmp(argv[1], "dir") == 0) {
		if (val == 0)
			ret = sx151x_direction_output(chip, gpio);
		else
			ret = sx151x_direction_input(chip, gpio);

		if (ret < 0)
			printf("Failed to set direction of spi%d.%d gpio %d\n",
			       CONFIG_SX151X_SPI_BUS, chip, gpio);
		else
			printf("New direction of spi%d.%d gpio %d is %d\n",
			       CONFIG_SX151X_SPI_BUS, chip, gpio, val);
		return ret;
	}

	printf("Please see usage\n");

	return ret;
}

U_BOOT_CMD(
	sx151x,	5,	1,	do_sx151x,
	"sx151x gpio access",
	"dir chip gpio 0|1\n"
	"	- set gpio direction (0 for output, 1 for input)\n"
	"sx151x get chip gpio\n"
	"	- get gpio value\n"
	"sx151x set chip gpio 0|1\n"
	"	- set gpio value\n"
	"sx151x reset chip\n"
	"	- reset chip"
);

#endif /* CONFIG_CMD_SX151X */
