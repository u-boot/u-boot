/*
 * Chromium OS Matrix Keyboard
 *
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cros_ec.h>
#include <errno.h>
#include <fdtdec.h>
#include <input.h>
#include <key_matrix.h>
#include <stdio_dev.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	KBC_MAX_KEYS		= 8,	/* Maximum keys held down at once */
	KBC_REPEAT_RATE_MS	= 30,
	KBC_REPEAT_DELAY_MS	= 240,
};

static struct keyb {
	struct cros_ec_dev *dev;		/* The CROS_EC device */
	struct input_config input;	/* The input layer */
	struct key_matrix matrix;	/* The key matrix layer */
	int key_rows;			/* Number of keyboard rows */
	int key_cols;			/* Number of keyboard columns */
	int ghost_filter;		/* 1 to enable ghost filter, else 0 */
	int inited;			/* 1 if keyboard is ready */
} config;


/**
 * Check the keyboard controller and return a list of key matrix positions
 * for which a key is pressed
 *
 * @param config	Keyboard config
 * @param keys		List of keys that we have detected
 * @param max_count	Maximum number of keys to return
 * @param samep		Set to true if this scan repeats the last, else false
 * @return number of pressed keys, 0 for none, -EIO on error
 */
static int check_for_keys(struct keyb *config,
			   struct key_matrix_key *keys, int max_count,
			   bool *samep)
{
	struct key_matrix_key *key;
	static struct mbkp_keyscan last_scan;
	static bool last_scan_valid;
	struct mbkp_keyscan scan;
	unsigned int row, col, bit, data;
	int num_keys;

	if (cros_ec_scan_keyboard(config->dev, &scan)) {
		debug("%s: keyboard scan failed\n", __func__);
		return -EIO;
	}
	*samep = last_scan_valid && !memcmp(&last_scan, &scan, sizeof(scan));

	/*
	 * This is a bit odd. The EC has no way to tell us that it has run
	 * out of key scans. It just returns the same scan over and over
	 * again. So the only way to detect that we have run out is to detect
	 * that this scan is the same as the last.
	 */
	last_scan_valid = true;
	memcpy(&last_scan, &scan, sizeof(last_scan));

	for (col = num_keys = bit = 0; col < config->matrix.num_cols;
			col++) {
		for (row = 0; row < config->matrix.num_rows; row++) {
			unsigned int mask = 1 << (bit & 7);

			data = scan.data[bit / 8];
			if ((data & mask) && num_keys < max_count) {
				key = keys + num_keys++;
				key->row = row;
				key->col = col;
				key->valid = 1;
			}
			bit++;
		}
	}

	return num_keys;
}

/**
 * Test if keys are available to be read
 *
 * @return 0 if no keys available, 1 if keys are available
 */
static int kbd_tstc(struct stdio_dev *dev)
{
	/* Just get input to do this for us */
	return config.inited ? input_tstc(&config.input) : 0;
}

/**
 * Read a key
 *
 * @return ASCII key code, or 0 if no key, or -1 if error
 */
static int kbd_getc(struct stdio_dev *dev)
{
	/* Just get input to do this for us */
	return config.inited ? input_getc(&config.input) : 0;
}

/**
 * Check the keyboard, and send any keys that are pressed.
 *
 * This is called by input_tstc() and input_getc() when they need more
 * characters
 *
 * @param input		Input configuration
 * @return 1, to indicate that we have something to look at
 */
int cros_ec_kbc_check(struct input_config *input)
{
	static struct key_matrix_key last_keys[KBC_MAX_KEYS];
	static int last_num_keys;
	struct key_matrix_key keys[KBC_MAX_KEYS];
	int keycodes[KBC_MAX_KEYS];
	int num_keys, num_keycodes;
	int irq_pending, sent;
	bool same = false;

	/*
	 * Loop until the EC has no more keyscan records, or we have
	 * received at least one character. This means we know that tstc()
	 * will always return non-zero if keys have been pressed.
	 *
	 * Without this loop, a key release (which generates no new ascii
	 * characters) will cause us to exit this function, and just tstc()
	 * may return 0 before all keys have been read from the EC.
	 */
	do {
		irq_pending = cros_ec_interrupt_pending(config.dev);
		if (irq_pending) {
			num_keys = check_for_keys(&config, keys, KBC_MAX_KEYS,
						  &same);
			if (num_keys < 0)
				return 0;
			last_num_keys = num_keys;
			memcpy(last_keys, keys, sizeof(keys));
		} else {
			/*
			 * EC doesn't want to be asked, so use keys from last
			 * time.
			 */
			num_keys = last_num_keys;
			memcpy(keys, last_keys, sizeof(keys));
		}

		if (num_keys < 0)
			return -1;
		num_keycodes = key_matrix_decode(&config.matrix, keys,
				num_keys, keycodes, KBC_MAX_KEYS);
		sent = input_send_keycodes(input, keycodes, num_keycodes);

		/*
		 * For those ECs without an interrupt, stop scanning when we
		 * see that the scan is the same as last time.
		 */
		if ((irq_pending < 0) && same)
			break;
	} while (irq_pending && !sent);

	return 1;
}

/**
 * Decode MBKP keyboard details from the device tree
 *
 * @param blob		Device tree blob
 * @param node		Node to decode from
 * @param config	Configuration data read from fdt
 * @return 0 if ok, -1 on error
 */
static int cros_ec_keyb_decode_fdt(const void *blob, int node,
				struct keyb *config)
{
	/*
	 * Get keyboard rows and columns - at present we are limited to
	 * 8 columns by the protocol (one byte per row scan)
	 */
	config->key_rows = fdtdec_get_int(blob, node, "keypad,num-rows", 0);
	config->key_cols = fdtdec_get_int(blob, node, "keypad,num-columns", 0);
	if (!config->key_rows || !config->key_cols ||
			config->key_rows * config->key_cols / 8
				> CROS_EC_KEYSCAN_COLS) {
		debug("%s: Invalid key matrix size %d x %d\n", __func__,
		      config->key_rows, config->key_cols);
		return -1;
	}
	config->ghost_filter = fdtdec_get_bool(blob, node,
					       "google,ghost-filter");
	return 0;
}

/**
 * Set up the keyboard. This is called by the stdio device handler.
 *
 * We want to do this init when the keyboard is actually used rather than
 * at start-up, since keyboard input may not currently be selected.
 *
 * @return 0 if ok, -1 on error
 */
static int cros_ec_init_keyboard(struct stdio_dev *dev)
{
	const void *blob = gd->fdt_blob;
	int node;

	config.dev = board_get_cros_ec_dev();
	if (!config.dev) {
		debug("%s: no cros_ec device: cannot init keyboard\n",
		      __func__);
		return -1;
	}
	node = fdtdec_next_compatible(blob, 0, COMPAT_GOOGLE_CROS_EC_KEYB);
	if (node < 0) {
		debug("%s: Node not found\n", __func__);
		return -1;
	}
	if (cros_ec_keyb_decode_fdt(blob, node, &config))
		return -1;
	input_set_delays(&config.input, KBC_REPEAT_DELAY_MS,
			 KBC_REPEAT_RATE_MS);
	if (key_matrix_init(&config.matrix, config.key_rows,
			config.key_cols, config.ghost_filter)) {
		debug("%s: cannot init key matrix\n", __func__);
		return -1;
	}
	if (key_matrix_decode_fdt(&config.matrix, gd->fdt_blob, node)) {
		debug("%s: Could not decode key matrix from fdt\n", __func__);
		return -1;
	}
	config.inited = 1;
	debug("%s: Matrix keyboard %dx%d ready\n", __func__, config.key_rows,
	      config.key_cols);

	return 0;
}

int drv_keyboard_init(void)
{
	struct stdio_dev dev;

	if (input_init(&config.input, 0)) {
		debug("%s: Cannot set up input\n", __func__);
		return -1;
	}
	config.input.read_keys = cros_ec_kbc_check;

	memset(&dev, '\0', sizeof(dev));
	strcpy(dev.name, "cros-ec-keyb");
	dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	dev.getc = kbd_getc;
	dev.tstc = kbd_tstc;
	dev.start = cros_ec_init_keyboard;

	/* Register the device. cros_ec_init_keyboard() will be called soon */
	return input_stdio_register(&dev);
}
