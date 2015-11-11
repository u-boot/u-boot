/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* i8042.c - Intel 8042 keyboard driver routines */

#include <common.h>
#include <i8042.h>
#include <input.h>
#include <asm/io.h>

/* defines */
#define in8(p)		inb(p)
#define out8(p, v)	outb(v, p)

/* locals */
static struct input_config config;
static bool extended;

static unsigned char ext_key_map[] = {
	0x1c, /* keypad enter */
	0x1d, /* right control */
	0x35, /* keypad slash */
	0x37, /* print screen */
	0x38, /* right alt */
	0x46, /* break */
	0x47, /* editpad home */
	0x48, /* editpad up */
	0x49, /* editpad pgup */
	0x4b, /* editpad left */
	0x4d, /* editpad right */
	0x4f, /* editpad end */
	0x50, /* editpad dn */
	0x51, /* editpad pgdn */
	0x52, /* editpad ins */
	0x53, /* editpad del */
	0x00  /* map end */
	};

static int kbd_input_empty(void)
{
	int kbd_timeout = KBD_TIMEOUT * 1000;

	while ((in8(I8042_STS_REG) & STATUS_IBF) && kbd_timeout--)
		udelay(1);

	return kbd_timeout != -1;
}

static int kbd_output_full(void)
{
	int kbd_timeout = KBD_TIMEOUT * 1000;

	while (((in8(I8042_STS_REG) & STATUS_OBF) == 0) && kbd_timeout--)
		udelay(1);

	return kbd_timeout != -1;
}

static void kbd_led_set(int flags)
{
	kbd_input_empty();
	out8(I8042_DATA_REG, CMD_SET_KBD_LED);
	kbd_input_empty();
	out8(I8042_DATA_REG, flags & 0x7);
}

static int kbd_write(int reg, int value)
{
	if (!kbd_input_empty())
		return -1;
	out8(reg, value);

	return 0;
}

static int kbd_read(int reg)
{
	if (!kbd_output_full())
		return -1;

	return in8(reg);
}

static int kbd_cmd_read(int cmd)
{
	if (kbd_write(I8042_CMD_REG, cmd))
		return -1;

	return kbd_read(I8042_DATA_REG);
}

static int kbd_cmd_write(int cmd, int data)
{
	if (kbd_write(I8042_CMD_REG, cmd))
		return -1;

	return kbd_write(I8042_DATA_REG, data);
}

static int kbd_reset(void)
{
	int config;

	/* controller self test */
	if (kbd_cmd_read(CMD_SELF_TEST) != KBC_TEST_OK)
		goto err;

	/* keyboard reset */
	if (kbd_write(I8042_DATA_REG, CMD_RESET_KBD) ||
	    kbd_read(I8042_DATA_REG) != KBD_ACK ||
	    kbd_read(I8042_DATA_REG) != KBD_POR)
		goto err;

	/* set AT translation and disable irq */
	config = kbd_cmd_read(CMD_RD_CONFIG);
	if (config == -1)
		goto err;

	config |= CONFIG_AT_TRANS;
	config &= ~(CONFIG_KIRQ_EN | CONFIG_MIRQ_EN);
	if (kbd_cmd_write(CMD_WR_CONFIG, config))
		goto err;

	/* enable keyboard */
	if (kbd_write(I8042_CMD_REG, CMD_KBD_EN) ||
	    !kbd_input_empty())
		goto err;

	return 0;
err:
	debug("%s: Keyboard failure\n", __func__);
	return -1;
}

static int kbd_controller_present(void)
{
	return in8(I8042_STS_REG) != 0xff;
}

/*
 * Implement a weak default function for boards that optionally
 * need to skip the i8042 initialization.
 */
int __weak board_i8042_skip(void)
{
	/* As default, don't skip */
	return 0;
}

void i8042_flush(void)
{
	int timeout;

	/*
	 * The delay is to give the keyboard controller some time
	 * to fill the next byte.
	 */
	while (1) {
		timeout = 100;	/* wait for no longer than 100us */
		while (timeout > 0 && !(in8(I8042_STS_REG) & STATUS_OBF)) {
			udelay(1);
			timeout--;
		}

		/* Try to pull next byte if not timeout */
		if (in8(I8042_STS_REG) & STATUS_OBF)
			in8(I8042_DATA_REG);
		else
			break;
	}
}

int i8042_disable(void)
{
	if (kbd_input_empty() == 0)
		return -1;

	/* Disable keyboard */
	out8(I8042_CMD_REG, CMD_KBD_DIS);

	if (kbd_input_empty() == 0)
		return -1;

	return 0;
}

static int i8042_kbd_check(struct input_config *input)
{
	if ((in8(I8042_STS_REG) & STATUS_OBF) == 0) {
		return 0;
	} else {
		bool release = false;
		int scan_code;
		int i;

		scan_code = in8(I8042_DATA_REG);
		if (scan_code == 0xfa) {
			return 0;
		} else if (scan_code == 0xe0) {
			extended = true;
			return 0;
		}
		if (scan_code & 0x80) {
			scan_code &= 0x7f;
			release = true;
		}
		if (extended) {
			extended = false;
			for (i = 0; ext_key_map[i]; i++) {
				if (ext_key_map[i] == scan_code) {
					scan_code = 0x60 + i;
					break;
				}
			}
			/* not found ? */
			if (!ext_key_map[i])
				return 0;
		}

		input_add_keycode(&config, scan_code, release);
		return 1;
	}
}

/* i8042_kbd_init - reset keyboard and init state flags */
int i8042_kbd_init(void)
{
	int keymap, try;
	char *penv;
	int ret;

	if (!kbd_controller_present() || board_i8042_skip()) {
		debug("i8042 keyboard controller is not present\n");
		return -1;
	}

	/* Init keyboard device (default US layout) */
	keymap = KBD_US;
	penv = getenv("keymap");
	if (penv != NULL) {
		if (strncmp(penv, "de", 3) == 0)
			keymap = KBD_GER;
	}

	for (try = 0; kbd_reset() != 0; try++) {
		if (try >= KBD_RESET_TRIES)
			return -1;
	}

	ret = input_init(&config, keymap == KBD_GER);
	if (ret)
		return ret;
	config.read_keys = i8042_kbd_check;
	input_allow_repeats(&config, true);

	kbd_led_set(NORMAL);

	return 0;
}

/**
 * check_leds() - Check the keyboard LEDs and update them it needed
 *
 * @ret:	Value to return
 * @return value of @ret
 */
static int check_leds(int ret)
{
	int leds;

	leds = input_leds_changed(&config);
	if (leds >= 0)
		kbd_led_set(leds);

	return ret;
}

/* i8042_tstc - test if keyboard input is available */
int i8042_tstc(struct stdio_dev *dev)
{
	return check_leds(input_tstc(&config));
}

/* i8042_getc - wait till keyboard input is available */
int i8042_getc(struct stdio_dev *dev)
{
	return check_leds(input_getc(&config));
}
