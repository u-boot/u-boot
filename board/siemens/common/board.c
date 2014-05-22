/*
 * Common board functions for siemens AM335X based boards
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * U-Boot file:/board/ti/am335x/board.c
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <watchdog.h>
#include "../common/factoryset.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPL_BUILD
void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	/* Initalize the board header */
	enable_i2c0_pin_mux();
	i2c_set_bus_num(0);
	if (read_eeprom() < 0)
		puts("Could not get board ID.\n");

	enable_board_pin_mux();
}

void sdram_init(void)
{
	spl_siemens_board_init();
	board_init_ddr();

	return;
}
#endif /* #ifdef CONFIG_SPL_BUILD */

#ifndef CONFIG_SPL_BUILD
/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
#if defined(CONFIG_HW_WATCHDOG)
	hw_watchdog_init();
#endif /* defined(CONFIG_HW_WATCHDOG) */
	i2c_set_bus_num(0);
	if (read_eeprom() < 0)
		puts("Could not get board ID.\n");

	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_FACTORYSET
	factoryset_read_eeprom(CONFIG_SYS_I2C_EEPROM_ADDR);
#endif
	gpmc_init();

#ifdef CONFIG_VIDEO
	board_video_init();
#endif

	return 0;
}
#endif /* #ifndef CONFIG_SPL_BUILD */

#define OSC	(V_OSCK/1000000)
const struct dpll_params dpll_ddr = {
		DDR_PLL_FREQ, OSC-1, 1, -1, -1, -1, -1};

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	omap_nand_switch_ecc(1, 8);

	return 0;
}
#endif

#ifndef CONFIG_SPL_BUILD
#if defined(BOARD_DFU_BUTTON_GPIO)
/*
 * This command returns the status of the user button on
 * Input - none
 * Returns -	1 if button is held down
 *		0 if button is not held down
 */
static int
do_userbutton(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int button = 0;
	int gpio;

	gpio = BOARD_DFU_BUTTON_GPIO;
	gpio_request(gpio, "DFU");
	gpio_direction_input(gpio);
	if (gpio_get_value(gpio))
		button = 1;
	else
		button = 0;

	gpio_free(gpio);

	return button;
}

U_BOOT_CMD(
	dfubutton, CONFIG_SYS_MAXARGS, 1, do_userbutton,
	"Return the status of the DFU button",
	""
);
#endif
/*
 * This command sets led
 * Input -	name of led
 *		value of led
 * Returns -	1 if input does not match
 *		0 if led was set
 */
static int
do_setled(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int gpio = 0;
	if (argc != 3)
		goto exit;
#if defined(BOARD_STATUS_LED)
	if (!strcmp(argv[1], "stat"))
		gpio = BOARD_STATUS_LED;
#endif
#if defined(BOARD_DFU_BUTTON_LED)
	if (!strcmp(argv[1], "dfu"))
		gpio = BOARD_DFU_BUTTON_LED;
#endif
	/* If argument does not mach exit */
	if (gpio == 0)
		goto exit;
	gpio_request(gpio, "");
	gpio_direction_output(gpio, 1);
	if (!strcmp(argv[2], "1"))
		gpio_set_value(gpio, 1);
	else
		gpio_set_value(gpio, 0);
	return 0;
exit:
	return 1;
}

U_BOOT_CMD(
	led, CONFIG_SYS_MAXARGS, 2, do_setled,
	"Set led on or off",
	"dfu val - set dfu led\nled stat val - set status led"
);

static int
do_usertestwdt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("\n\n\n Go into infinite loop\n\n\n");
	while (1)
		;
	return 0;
};

U_BOOT_CMD(
	testwdt, CONFIG_SYS_MAXARGS, 1,	do_usertestwdt,
	"Sends U-Boot into infinite loop",
	""
);
#endif /* !CONFIG_SPL_BUILD */
