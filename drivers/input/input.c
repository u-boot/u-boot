/*
 * Translate key codes into ASCII
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2004 DENX Software Engineering, Wolfgang Denk, wd@denx.de
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <stdio_dev.h>
#include <input.h>
#include <linux/input.h>

enum {
	/* These correspond to the lights on the keyboard */
	FLAG_NUM_LOCK		= 1 << 0,
	FLAG_CAPS_LOCK		= 1 << 1,
	FLAG_SCROLL_LOCK	= 1 << 2,

	/* Special flag ORed with key code to indicate release */
	KEY_RELEASE		= 1 << 15,
	KEY_MASK		= 0xfff,
};

/*
 * These takes map key codes to ASCII. 0xff means no key, or special key.
 * Three tables are provided - one for plain keys, one for when the shift
 * 'modifier' key is pressed and one for when the ctrl modifier key is
 * pressed.
 */
static const uchar kbd_plain_xlate[] = {
	0xff, 0x1b, '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',  '-',  '=', '\b', '\t',	/* 0x00 - 0x0f */
	'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
	'o',  'p',  '[',  ']', '\r', 0xff,  'a',  's',  /* 0x10 - 0x1f */
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
	'\'',  '`', 0xff, '\\', 'z',  'x',  'c',  'v',	/* 0x20 - 0x2f */
	'b',  'n',  'm',  ',' ,  '.', '/', 0xff, 0xff, 0xff,
	' ', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x30 - 0x3f */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  '7',
	'8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',	/* 0x40 - 0x4f */
	'2',  '3',  '0',  '.', 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x50 - 0x5F */
	'\r', 0xff, 0xff
};

static unsigned char kbd_shift_xlate[] = {
	0xff, 0x1b, '!', '@', '#', '$', '%', '^',
	'&', '*', '(', ')', '_', '+', '\b', '\t',	/* 0x00 - 0x0f */
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
	'O', 'P', '{', '}', '\r', 0xff, 'A', 'S',	/* 0x10 - 0x1f */
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'"', '~', 0xff, '|', 'Z', 'X', 'C', 'V',	/* 0x20 - 0x2f */
	'B', 'N', 'M', '<', '>', '?', 0xff, 0xff, 0xff,
	' ', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x30 - 0x3f */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, '7',
	'8', '9', '-', '4', '5', '6', '+', '1',	/* 0x40 - 0x4f */
	'2', '3', '0', '.', 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x50 - 0x5F */
	'\r', 0xff, 0xff
};

static unsigned char kbd_ctrl_xlate[] = {
	0xff, 0x1b, '1', 0x00, '3', '4', '5', 0x1E,
	'7', '8', '9', '0', 0x1F, '=', '\b', '\t',	/* 0x00 - 0x0f */
	0x11, 0x17, 0x05, 0x12, 0x14, 0x18, 0x15, 0x09,
	0x0f, 0x10, 0x1b, 0x1d, '\n', 0xff, 0x01, 0x13,	/* 0x10 - 0x1f */
	0x04, 0x06, 0x08, 0x09, 0x0a, 0x0b, 0x0c, ';',
	'\'', '~', 0x00, 0x1c, 0x1a, 0x18, 0x03, 0x16,	/* 0x20 - 0x2f */
	0x02, 0x0e, 0x0d, '<', '>', '?', 0xff, 0xff,
	0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x30 - 0x3f */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, '7',
	'8', '9', '-', '4', '5', '6', '+', '1',		/* 0x40 - 0x4f */
	'2', '3', '0', '.', 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x50 - 0x5F */
	'\r', 0xff, 0xff
};


int input_queue_ascii(struct input_config *config, int ch)
{
	if (config->fifo_in + 1 == INPUT_BUFFER_LEN) {
		if (!config->fifo_out)
			return -1; /* buffer full */
		else
			config->fifo_in = 0;
	} else {
		if (config->fifo_in + 1 == config->fifo_out)
			return -1; /* buffer full */
		config->fifo_in++;
	}
	config->fifo[config->fifo_in] = (uchar)ch;

	return 0;
}

int input_tstc(struct input_config *config)
{
	if (config->fifo_in == config->fifo_out && config->read_keys) {
		if (!(*config->read_keys)(config))
			return 0;
	}
	return config->fifo_in != config->fifo_out;
}

int input_getc(struct input_config *config)
{
	int err = 0;

	while (config->fifo_in == config->fifo_out) {
		if (config->read_keys)
			err = (*config->read_keys)(config);
		if (err)
			return -1;
	}

	if (++config->fifo_out == INPUT_BUFFER_LEN)
		config->fifo_out = 0;

	return config->fifo[config->fifo_out];
}

/**
 * Process a modifier/special key press or release and decide which key
 * translation array should be used as a result.
 *
 * TODO: Should keep track of modifier press/release
 *
 * @param config	Input state
 * @param key		Key code to process
 * @param release	0 if a press, 1 if a release
 * @return pointer to keycode->ascii translation table that should be used
 */
static struct input_key_xlate *process_modifier(struct input_config *config,
						int key, int release)
{
	struct input_key_xlate *table;
	int flip = -1;
	int i;

	/* Start with the main table, and see what modifiers change it */
	assert(config->num_tables > 0);
	table = &config->table[0];
	for (i = 1; i < config->num_tables; i++) {
		struct input_key_xlate *tab = &config->table[i];

		if (key == tab->left_keycode || key == tab->right_keycode)
			table = tab;
	}

	/* Handle the lighted keys */
	if (!release) {
		switch (key) {
		case KEY_SCROLLLOCK:
			flip = FLAG_SCROLL_LOCK;
			break;
		case KEY_NUMLOCK:
			flip = FLAG_NUM_LOCK;
			break;
		case KEY_CAPSLOCK:
			flip = FLAG_CAPS_LOCK;
			break;
		}
	}

	if (flip != -1) {
		int leds = 0;

		config->leds ^= flip;
		if (config->flags & FLAG_NUM_LOCK)
			leds |= INPUT_LED_NUM;
		if (config->flags & FLAG_CAPS_LOCK)
			leds |= INPUT_LED_CAPS;
		if (config->flags & FLAG_SCROLL_LOCK)
			leds |= INPUT_LED_SCROLL;
		config->leds = leds;
	}

	return table;
}

/**
 * Search an int array for a key value
 *
 * @param array	Array to search
 * @param count	Number of elements in array
 * @param key	Key value to find
 * @return element where value was first found, -1 if none
 */
static int array_search(int *array, int count, int key)
{
	int i;

	for (i = 0; i < count; i++) {
		if (array[i] == key)
			return i;
	}

	return -1;
}

/**
 * Sort an array so that those elements that exist in the ordering are
 * first in the array, and in the same order as the ordering. The algorithm
 * is O(count * ocount) and designed for small arrays.
 *
 * TODO: Move this to common / lib?
 *
 * @param dest		Array with elements to sort, also destination array
 * @param count		Number of elements to sort
 * @param order		Array containing ordering elements
 * @param ocount	Number of ordering elements
 * @return number of elements in dest that are in order (these will be at the
 *	start of dest).
 */
static int sort_array_by_ordering(int *dest, int count, int *order,
				   int ocount)
{
	int temp[count];
	int dest_count;
	int same;	/* number of elements which are the same */
	int i;

	/* setup output items, copy items to be sorted into our temp area */
	memcpy(temp, dest, count * sizeof(*dest));
	dest_count = 0;

	/* work through the ordering, move over the elements we agree on */
	for (i = 0; i < ocount; i++) {
		if (array_search(temp, count, order[i]) != -1)
			dest[dest_count++] = order[i];
	}
	same = dest_count;

	/* now move over the elements that are not in the ordering */
	for (i = 0; i < count; i++) {
		if (array_search(order, ocount, temp[i]) == -1)
			dest[dest_count++] = temp[i];
	}
	assert(dest_count == count);
	return same;
}

/**
 * Check a list of key codes against the previous key scan
 *
 * Given a list of new key codes, we check how many of these are the same
 * as last time.
 *
 * @param config	Input state
 * @param keycode	List of key codes to examine
 * @param num_keycodes	Number of key codes
 * @param same		Returns number of key codes which are the same
 */
static int input_check_keycodes(struct input_config *config,
			   int keycode[], int num_keycodes, int *same)
{
	/* Select the 'plain' xlate table to start with */
	if (!config->num_tables) {
		debug("%s: No xlate tables: cannot decode keys\n", __func__);
		return -1;
	}

	/* sort the keycodes into the same order as the previous ones */
	*same = sort_array_by_ordering(keycode, num_keycodes,
			config->prev_keycodes, config->num_prev_keycodes);

	memcpy(config->prev_keycodes, keycode, num_keycodes * sizeof(int));
	config->num_prev_keycodes = num_keycodes;

	return *same != num_keycodes;
}

/**
 * Convert a list of key codes into ASCII
 *
 * You must call input_check_keycodes() before this. It turns the keycode
 * list into a list of ASCII characters which are ready to send to the
 * input layer.
 *
 * Characters which were seen last time do not generate fresh ASCII output.
 *
 * @param config	Input state
 * @param keycode	List of key codes to examine
 * @param num_keycodes	Number of key codes
 * @param same		Number of key codes which are the same
 */
static int input_keycodes_to_ascii(struct input_config *config,
		int keycode[], int num_keycodes, char output_ch[], int same)
{
	struct input_key_xlate *table;
	int ch_count;
	int i;

	table = &config->table[0];

	/* deal with modifiers first */
	for (i = 0; i < num_keycodes; i++) {
		int key = keycode[i] & KEY_MASK;

		if (key >= table->num_entries || table->xlate[key] == 0xff) {
			table = process_modifier(config, key,
					keycode[i] & KEY_RELEASE);
		}
	}

	/* now find normal keys */
	for (i = ch_count = 0; i < num_keycodes; i++) {
		int key = keycode[i];

		if (key < table->num_entries && i >= same) {
			int ch = table->xlate[key];

			/* If a normal key with an ASCII value, add it! */
			if (ch != 0xff)
				output_ch[ch_count++] = (uchar)ch;
		}
	}

	/* ok, so return keys */
	return ch_count;
}

int input_send_keycodes(struct input_config *config,
			int keycode[], int num_keycodes)
{
	char ch[num_keycodes];
	int count, i, same = 0;
	int is_repeat = 0;
	unsigned delay_ms;

	config->modifiers = 0;
	if (!input_check_keycodes(config, keycode, num_keycodes, &same)) {
		/*
		 * Same as last time - is it time for another repeat?
		 * TODO(sjg@chromium.org) We drop repeats here and since
		 * the caller may not call in again for a while, our
		 * auto-repeat speed is not quite correct. We should
		 * insert another character if we later realise that we
		 * have missed a repeat slot.
		 */
		is_repeat = (int)get_timer(config->next_repeat_ms) >= 0;
		if (!is_repeat)
			return 0;
	}

	count = input_keycodes_to_ascii(config, keycode, num_keycodes,
					ch, is_repeat ? 0 : same);
	for (i = 0; i < count; i++)
		input_queue_ascii(config, ch[i]);
	delay_ms = is_repeat ?
			config->repeat_rate_ms :
			config->repeat_delay_ms;

	config->next_repeat_ms = get_timer(0) + delay_ms;
	return 0;
}

int input_add_table(struct input_config *config, int left_keycode,
		    int right_keycode, const uchar *xlate, int num_entries)
{
	struct input_key_xlate *table;

	if (config->num_tables == INPUT_MAX_MODIFIERS) {
		debug("%s: Too many modifier tables\n", __func__);
		return -1;
	}

	table = &config->table[config->num_tables++];
	table->left_keycode = left_keycode;
	table->right_keycode = right_keycode;
	table->xlate = xlate;
	table->num_entries = num_entries;

	return 0;
}

int input_init(struct input_config *config, int leds, int repeat_delay_ms,
	       int repeat_rate_ms)
{
	memset(config, '\0', sizeof(*config));
	config->leds = leds;
	config->repeat_delay_ms = repeat_delay_ms;
	config->repeat_rate_ms = repeat_rate_ms;
	if (input_add_table(config, -1, -1,
			kbd_plain_xlate, ARRAY_SIZE(kbd_plain_xlate)) ||
		input_add_table(config, KEY_LEFTSHIFT, KEY_RIGHTSHIFT,
			kbd_shift_xlate, ARRAY_SIZE(kbd_shift_xlate)) ||
		input_add_table(config, KEY_LEFTCTRL, KEY_RIGHTCTRL,
			kbd_ctrl_xlate, ARRAY_SIZE(kbd_ctrl_xlate))) {
		debug("%s: Could not add modifier tables\n", __func__);
		return -1;
	}

	return 0;
}

int input_stdio_register(struct stdio_dev *dev)
{
	int error;

	error = stdio_register(dev);

	/* check if this is the standard input device */
	if (!error && strcmp(getenv("stdin"), dev->name) == 0) {
		/* reassign the console */
		if (OVERWRITE_CONSOLE ||
				console_assign(stdin, dev->name))
			return -1;
	}

	return 0;
}
