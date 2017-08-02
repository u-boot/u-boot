/*
 * ULCB board CPLD access support
 *
 * Copyright (C) 2017 Renesas Electronics Corporation
 * Copyright (C) 2017 Cogent Embedded, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/gpio.h>

#define SCLK			GPIO_GP_6_8
#define SSTBZ			GPIO_GP_2_3
#define MOSI			GPIO_GP_6_7
#define MISO			GPIO_GP_6_10

#define CPLD_ADDR_MODE		0x00 /* RW */
#define CPLD_ADDR_MUX		0x02 /* RW */
#define CPLD_ADDR_DIPSW6	0x08 /* R */
#define CPLD_ADDR_RESET		0x80 /* RW */
#define CPLD_ADDR_VERSION	0xFF /* R */

static int cpld_initialized;

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	/* Always valid */
	return 1;
}

void spi_cs_activate(struct spi_slave *slave)
{
	/* Always active */
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	/* Always active */
}

void ulcb_softspi_sda(int set)
{
	gpio_set_value(MOSI, set);
}

void ulcb_softspi_scl(int set)
{
	gpio_set_value(SCLK, set);
}

unsigned char ulcb_softspi_read(void)
{
	return !!gpio_get_value(MISO);
}

static void cpld_rw(u8 write)
{
	gpio_set_value(MOSI, write);
	gpio_set_value(SSTBZ, 0);
	gpio_set_value(SCLK, 1);
	gpio_set_value(SCLK, 0);
	gpio_set_value(SSTBZ, 1);
}

static u32 cpld_read(u8 addr)
{
	u32 data = 0;

	spi_xfer(NULL, 8, &addr, NULL, SPI_XFER_BEGIN | SPI_XFER_END);

	cpld_rw(0);

	spi_xfer(NULL, 32, NULL, &data, SPI_XFER_BEGIN | SPI_XFER_END);

	return swab32(data);
}

static void cpld_write(u8 addr, u32 data)
{
	data = swab32(data);

	spi_xfer(NULL, 32, &data, NULL, SPI_XFER_BEGIN | SPI_XFER_END);

	spi_xfer(NULL, 8, NULL, &addr, SPI_XFER_BEGIN | SPI_XFER_END);

	cpld_rw(1);
}

static void cpld_init(void)
{
	if (cpld_initialized)
		return;

	/* PULL-UP on MISO line */
	setbits_le32(PFC_PUEN5, PUEN_SSI_SDATA4);

	gpio_request(SCLK, NULL);
	gpio_request(SSTBZ, NULL);
	gpio_request(MOSI, NULL);
	gpio_request(MISO, NULL);

	gpio_direction_output(SCLK, 0);
	gpio_direction_output(SSTBZ, 1);
	gpio_direction_output(MOSI, 0);
	gpio_direction_input(MISO);

	/* Dummy read */
	cpld_read(CPLD_ADDR_VERSION);

	cpld_initialized = 1;
}

static int do_cpld(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 addr, val;

	cpld_init();

	if (argc == 2 && strcmp(argv[1], "info") == 0) {
		printf("CPLD version:\t\t\t0x%08x\n",
		       cpld_read(CPLD_ADDR_VERSION));
		printf("H3 Mode setting (MD0..28):\t0x%08x\n",
		       cpld_read(CPLD_ADDR_MODE));
		printf("Multiplexer settings:\t\t0x%08x\n",
		       cpld_read(CPLD_ADDR_MUX));
		printf("DIPSW (SW6):\t\t\t0x%08x\n",
		       cpld_read(CPLD_ADDR_DIPSW6));
		return 0;
	}

	if (argc < 3)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[2], NULL, 16);
	if (!(addr == CPLD_ADDR_VERSION || addr == CPLD_ADDR_MODE ||
	      addr == CPLD_ADDR_MUX || addr == CPLD_ADDR_DIPSW6 ||
	      addr == CPLD_ADDR_RESET)) {
		printf("Invalid CPLD register address\n");
		return CMD_RET_USAGE;
	}

	if (argc == 3 && strcmp(argv[1], "read") == 0) {
		printf("0x%x\n", cpld_read(addr));
	} else if (argc == 4 && strcmp(argv[1], "write") == 0) {
		val = simple_strtoul(argv[3], NULL, 16);
		cpld_write(addr, val);
	}

	return 0;
}

U_BOOT_CMD(
	cpld, 4, 1, do_cpld,
	"CPLD access",
	"info\n"
	"cpld read addr\n"
	"cpld write addr val\n"
);

void reset_cpu(ulong addr)
{
	cpld_init();
	cpld_write(CPLD_ADDR_RESET, 1);
}
