/*
 * (C) Copyright 2005 - 2006
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * TB5200 specific functions
 */
/*#define DEBUG*/

#include <common.h>
#include <command.h>

#if defined(CONFIG_CMD_BSP)
#if defined (CONFIG_TB5200)

#define SM501_PANEL_DISPLAY_CONTROL	0x00080000UL

static void led_init(void)
{
	struct mpc5xxx_gpt_0_7 *gpt = (struct mpc5xxx_gpt_0_7 *)MPC5XXX_GPT;

	/* configure timer 4 for simple GPIO output */
	gpt->gpt4.emsr |=  0x00000024;
}

int cmd_led(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct mpc5xxx_gpt_0_7 *gpt = (struct mpc5xxx_gpt_0_7 *)MPC5XXX_GPT;

	led_init();

	if (strcmp (argv[1], "on") == 0) {
		debug ("switch status LED on\n");
		gpt->gpt4.emsr |=  (1 << 4);
	} else if (strcmp (argv[1], "off") == 0) {
		debug ("switch status LED off\n");
		gpt->gpt4.emsr &=  ~(1 << 4);
	} else {
		printf ("Usage:\nled on/off\n");
		return 1;
	}

	return 0;
}

static void sm501_backlight (unsigned int state)
{
	if (state == 1) {
		*(vu_long *)(SM501_MMIO_BASE+SM501_PANEL_DISPLAY_CONTROL) |=
			(1 << 26) | (1 << 27);
	} else if (state == 0)
		*(vu_long *)(SM501_MMIO_BASE+SM501_PANEL_DISPLAY_CONTROL) &=
			~((1 << 26) | (1 << 27));
}

int cmd_backlight(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (strcmp (argv[1], "on") == 0) {
		debug ("switch backlight on\n");
		sm501_backlight (1);
	} else if (strcmp (argv[1], "off") == 0) {
		debug ("switch backlight off\n");
		sm501_backlight (0);
	} else {
		printf ("Usage:\nbacklight on/off\n");
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	led ,	2,	1,	cmd_led,
	"switch status LED on or off",
	"on/off"
);

U_BOOT_CMD(
	backlight ,	2,	1,	cmd_backlight,
	"switch backlight on or off",
	"on/off"
	);

#endif /* CONFIG_STK52XX */
#endif
