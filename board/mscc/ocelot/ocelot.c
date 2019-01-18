// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <environment.h>
#include <spi.h>
#include <led.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	BOARD_TYPE_PCB120 = 0xAABBCC00,
	BOARD_TYPE_PCB123,
};

void board_debug_uart_init(void)
{
	/* too early for the pinctrl driver, so configure the UART pins here */
	mscc_gpio_set_alternate(6, 1);
	mscc_gpio_set_alternate(7, 1);
}

int board_early_init_r(void)
{
	/* Prepare SPI controller to be used in master mode */
	writel(0, BASE_CFG + ICPU_SW_MODE);
	clrsetbits_le32(BASE_CFG + ICPU_GENERAL_CTRL,
			ICPU_GENERAL_CTRL_IF_SI_OWNER_M,
			ICPU_GENERAL_CTRL_IF_SI_OWNER(2));

	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE;

	/* LED setup */
	if (IS_ENABLED(CONFIG_LED))
		led_default_state();

	return 0;
}

static void do_board_detect(void)
{
	u16 dummy = 0;

	/* Enable MIIM */
	mscc_gpio_set_alternate(14, 1);
	mscc_gpio_set_alternate(15, 1);
	if (mscc_phy_rd(1, 0, 0, &dummy) == 0)
		gd->board_type = BOARD_TYPE_PCB120;
	else
		gd->board_type = BOARD_TYPE_PCB123;
}

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	if (gd->board_type == BOARD_TYPE_PCB120 &&
	    strcmp(name, "ocelot_pcb120") == 0)
		return 0;

	if (gd->board_type == BOARD_TYPE_PCB123 &&
	    strcmp(name, "ocelot_pcb123") == 0)
		return 0;

	return -1;
}
#endif

#if defined(CONFIG_DTB_RESELECT)
int embedded_dtb_select(void)
{
	do_board_detect();
	fdtdec_setup();

	return 0;
}
#endif
