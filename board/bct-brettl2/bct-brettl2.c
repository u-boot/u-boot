/*
 * U-boot - main board file for BCT brettl2
 *
 * Copyright (c) 2010 BCT Electronic GmbH
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <asm/blackfin.h>
#include <asm/portmux.h>
#include <asm/gpio.h>
#include <net.h>
#include <netdev.h>
#include <miiphy.h>

#include "../cm-bf537e/gpio_cfi_flash.h"
#include "smsc9303.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: bct-brettl2 board\n");
	printf("       Support: http://www.bct-electronic.com/\n");
	return 0;
}

#ifdef CONFIG_BFIN_MAC
int board_eth_init(bd_t *bis)
{
	int retry = 3;
	int ret;

	ret = bfin_EMAC_initialize(bis);

	uchar enetaddr[6];
	if (eth_getenv_enetaddr("ethaddr", enetaddr)) {
		printf("setting MAC %pM\n", enetaddr);
	}
	puts("       ");

	puts("initialize SMSC LAN9303i ethernet switch\n");

	while (retry-- > 0) {
		if (init_smsc9303i_mii())
			return ret;
	}

	return ret;
}
#endif

static void init_tlv320aic31(void)
{
	puts("Audio: setup TIMER0 to enable 16.384 MHz clock for tlv320aic31\n");
	peripheral_request(P_TMR0, "tlv320aic31 clock");
	bfin_write_TIMER0_CONFIG(0x020d);
	bfin_write_TIMER0_PERIOD(0x0008);
	bfin_write_TIMER0_WIDTH(0x0008/2);
	bfin_write_TIMER_ENABLE(bfin_read_TIMER_ENABLE() | 1);
	SSYNC();
	udelay(10000);

	puts("       resetting tlv320aic31\n");

	gpio_request(GPIO_PF2, "tlv320aic31");
	gpio_direction_output(GPIO_PF2, 0);
	udelay(10000);
	gpio_direction_output(GPIO_PF2, 1);
	udelay(10000);
	gpio_free(GPIO_PF2);
}

static void init_mute_pin(void)
{
	printf("       unmute class D amplifier\n");

	gpio_request(GPIO_PF5, "mute");
	gpio_direction_output(GPIO_PF5, 1);
	gpio_free(GPIO_PF5);
}

/* sometimes LEDs (speech, status) are still on after reboot, turn 'em off */
static void turn_leds_off(void)
{
	printf("       turn LEDs off\n");

	gpio_request(GPIO_PF6, "led");
	gpio_direction_output(GPIO_PF6, 0);
	gpio_free(GPIO_PF6);

	gpio_request(GPIO_PF15, "led");
	gpio_direction_output(GPIO_PF15, 0);
	gpio_free(GPIO_PF15);
}

/* miscellaneous platform dependent initialisations */
int misc_init_r(void)
{
	gpio_cfi_flash_init();
	init_tlv320aic31();
	init_mute_pin();
	turn_leds_off();

	return 0;
}
