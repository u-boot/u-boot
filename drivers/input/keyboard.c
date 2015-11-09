/***********************************************************************
 *
 * (C) Copyright 2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 *
 * Keyboard driver
 *
 ***********************************************************************/

#include <common.h>
#include <console.h>
#include <input.h>

#include <stdio_dev.h>
#include <keyboard.h>
#include <stdio_dev.h>

static struct input_config config;

static int kbd_read_keys(struct input_config *config)
{
#if defined(CONFIG_MPC5xxx) || defined(CONFIG_MPC8540) || \
		defined(CONFIG_MPC8541) || defined(CONFIG_MPC8555)
	/* no ISR is used, so received chars must be polled */
	ps2ser_check();
#endif

	return 1;
}

static int check_leds(int ret)
{
	int leds;

	leds = input_leds_changed(&config);
	if (leds >= 0)
		pckbd_leds(leds);

	return ret;
}

/* test if a character is in the queue */
static int kbd_testc(struct stdio_dev *dev)
{
	return check_leds(input_tstc(&config));
}

/* gets the character from the queue */
static int kbd_getc(struct stdio_dev *dev)
{
	return check_leds(input_getc(&config));
}

void handle_scancode(unsigned char scan_code)
{
	bool release = false;

	/* Compare with i8042_kbd_check() in i8042.c if some logic is missing */
	if (scan_code & 0x80) {
		scan_code &= 0x7f;
		release = true;
	}

	input_add_keycode(&config, scan_code, release);
}

/* TODO: convert to driver model */
int kbd_init (void)
{
	struct stdio_dev kbddev;
	struct input_config *input = &config;

	if(kbd_init_hw()==-1)
		return -1;
	memset (&kbddev, 0, sizeof(kbddev));
	strcpy(kbddev.name, "kbd");
	kbddev.flags =  DEV_FLAGS_INPUT;
	kbddev.getc = kbd_getc;
	kbddev.tstc = kbd_testc;

	input_init(input, 0);
	input->read_keys = kbd_read_keys;
	input_add_tables(input, true);

	return input_stdio_register(&kbddev);
}
