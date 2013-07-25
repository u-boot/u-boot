/*
 *  (C) Copyright 2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <input.h>
#include <key_matrix.h>
#include <stdio_dev.h>
#include <tegra-kbc.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch-tegra/timer.h>
#include <linux/input.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	KBC_MAX_GPIO		= 24,
	KBC_MAX_KPENT		= 8,	/* size of keypress entry queue */
};

#define KBC_FIFO_TH_CNT_SHIFT		14
#define KBC_DEBOUNCE_CNT_SHIFT		4
#define KBC_CONTROL_FIFO_CNT_INT_EN	(1 << 3)
#define KBC_CONTROL_KBC_EN		(1 << 0)
#define KBC_INT_FIFO_CNT_INT_STATUS	(1 << 2)
#define KBC_KPENT_VALID			(1 << 7)
#define KBC_ST_STATUS			(1 << 3)

enum {
	KBC_DEBOUNCE_COUNT	= 2,
	KBC_REPEAT_RATE_MS	= 30,
	KBC_REPEAT_DELAY_MS	= 240,
	KBC_CLOCK_KHZ		= 32,	/* Keyboard uses a 32KHz clock */
};

/* keyboard controller config and state */
static struct keyb {
	struct input_config input;	/* The input layer */
	struct key_matrix matrix;	/* The key matrix layer */

	struct kbc_tegra *kbc;		/* tegra keyboard controller */
	unsigned char inited;		/* 1 if keyboard has been inited */
	unsigned char first_scan;	/* 1 if this is our first key scan */
	unsigned char created;		/* 1 if driver has been created */

	/*
	 * After init we must wait a short time before polling the keyboard.
	 * This gives the tegra keyboard controller time to react after reset
	 * and lets us grab keys pressed during reset.
	 */
	unsigned int init_dly_ms;	/* Delay before we can read keyboard */
	unsigned int start_time_ms;	/* Time that we inited (in ms) */
	unsigned int last_poll_ms;	/* Time we should last polled */
	unsigned int next_repeat_ms;	/* Next time we repeat a key */
} config;

/**
 * reads the keyboard fifo for current keypresses
 *
 * @param config	Keyboard config
 * @param fifo		Place to put fifo results
 * @param max_keycodes	Maximum number of key codes to put in the fifo
 * @return number of items put into fifo
 */
static int tegra_kbc_find_keys(struct keyb *config, int *fifo,
			       int max_keycodes)
{
	struct key_matrix_key keys[KBC_MAX_KPENT], *key;
	u32 kp_ent = 0;
	int i;

	for (key = keys, i = 0; i < KBC_MAX_KPENT; i++, key++) {
		/* Get next word */
		if (!(i & 3))
			kp_ent = readl(&config->kbc->kp_ent[i / 4]);

		key->valid = (kp_ent & KBC_KPENT_VALID) != 0;
		key->row = (kp_ent >> 3) & 0xf;
		key->col = kp_ent & 0x7;

		/* Shift to get next entry */
		kp_ent >>= 8;
	}
	return key_matrix_decode(&config->matrix, keys, KBC_MAX_KPENT, fifo,
				 max_keycodes);
}

/**
 * Process all the keypress sequences in fifo and send key codes
 *
 * The fifo contains zero or more keypress sets. Each set
 * consists of from 1-8 keycodes, representing the keycodes which
 * were simultaneously pressed during that scan.
 *
 * This function works through each set and generates ASCII characters
 * for each. Not that one set may produce more than one ASCII characters -
 * for example holding down 'd' and 'f' at the same time will generate
 * two ASCII characters.
 *
 * Note: if fifo_cnt is 0, we will tell the input layer that no keys are
 * pressed.
 *
 * @param config	Keyboard config
 * @param fifo_cnt	Number of entries in the keyboard fifo
 */
static void process_fifo(struct keyb *config, int fifo_cnt)
{
	int fifo[KBC_MAX_KPENT];
	int cnt = 0;

	/* Always call input_send_keycodes() at least once */
	do {
		if (fifo_cnt)
			cnt = tegra_kbc_find_keys(config, fifo, KBC_MAX_KPENT);

		input_send_keycodes(&config->input, fifo, cnt);
	} while (--fifo_cnt > 0);
}

/**
 * Check the keyboard controller and emit ASCII characters for any keys that
 * are pressed.
 *
 * @param config	Keyboard config
 */
static void check_for_keys(struct keyb *config)
{
	int fifo_cnt;

	if (!config->first_scan &&
			get_timer(config->last_poll_ms) < KBC_REPEAT_RATE_MS)
		return;
	config->last_poll_ms = get_timer(0);
	config->first_scan = 0;

	/*
	 * Once we get here we know the keyboard has been scanned. So if there
	 * scan waiting for us, we know that nothing is held down.
	 */
	fifo_cnt = (readl(&config->kbc->interrupt) >> 4) & 0xf;
	process_fifo(config, fifo_cnt);
}

/**
 * In order to detect keys pressed on boot, wait for the hardware to
 * complete scanning the keys. This includes time to transition from
 * Wkup mode to Continous polling mode and the repoll time. We can
 * deduct the time that's already elapsed.
 *
 * @param config	Keyboard config
 */
static void kbd_wait_for_fifo_init(struct keyb *config)
{
	if (!config->inited) {
		unsigned long elapsed_time;
		long delay_ms;

		elapsed_time = get_timer(config->start_time_ms);
		delay_ms = config->init_dly_ms - elapsed_time;
		if (delay_ms > 0) {
			udelay(delay_ms * 1000);
			debug("%s: delay %ldms\n", __func__, delay_ms);
		}

		config->inited = 1;
	}
}

/**
 * Check the tegra keyboard, and send any keys that are pressed.
 *
 * This is called by input_tstc() and input_getc() when they need more
 * characters
 *
 * @param input		Input configuration
 * @return 1, to indicate that we have something to look at
 */
int tegra_kbc_check(struct input_config *input)
{
	kbd_wait_for_fifo_init(&config);
	check_for_keys(&config);

	return 1;
}

/**
 * Test if keys are available to be read
 *
 * @return 0 if no keys available, 1 if keys are available
 */
static int kbd_tstc(void)
{
	/* Just get input to do this for us */
	return input_tstc(&config.input);
}

/**
 * Read a key
 *
 * TODO: U-Boot wants 0 for no key, but Ctrl-@ is a valid key...
 *
 * @return ASCII key code, or 0 if no key, or -1 if error
 */
static int kbd_getc(void)
{
	/* Just get input to do this for us */
	return input_getc(&config.input);
}

/* configures keyboard GPIO registers to use the rows and columns */
static void config_kbc_gpio(struct kbc_tegra *kbc)
{
	int i;

	for (i = 0; i < KBC_MAX_GPIO; i++) {
		u32 row_cfg, col_cfg;
		u32 r_shift = 5 * (i % 6);
		u32 c_shift = 4 * (i % 8);
		u32 r_mask = 0x1f << r_shift;
		u32 c_mask = 0xf << c_shift;
		u32 r_offs = i / 6;
		u32 c_offs = i / 8;

		row_cfg = readl(&kbc->row_cfg[r_offs]);
		col_cfg = readl(&kbc->col_cfg[c_offs]);

		row_cfg &= ~r_mask;
		col_cfg &= ~c_mask;

		if (i < config.matrix.num_rows) {
			row_cfg |= ((i << 1) | 1) << r_shift;
		} else {
			col_cfg |= (((i - config.matrix.num_rows) << 1) | 1)
					<< c_shift;
		}

		writel(row_cfg, &kbc->row_cfg[r_offs]);
		writel(col_cfg, &kbc->col_cfg[c_offs]);
	}
}

/**
 * Start up the keyboard device
 */
static void tegra_kbc_open(void)
{
	struct kbc_tegra *kbc = config.kbc;
	unsigned int scan_period;
	u32 val;

	/*
	 * We will scan at twice the keyboard repeat rate, so that there is
	 * always a scan ready when we check it in check_for_keys().
	 */
	scan_period = KBC_REPEAT_RATE_MS / 2;
	writel(scan_period * KBC_CLOCK_KHZ, &kbc->rpt_dly);
	writel(scan_period * KBC_CLOCK_KHZ, &kbc->init_dly);
	/*
	 * Before reading from the keyboard we must wait for the init_dly
	 * plus the rpt_delay, plus 2ms for the row scan time.
	 */
	config.init_dly_ms = scan_period * 2 + 2;

	val = KBC_DEBOUNCE_COUNT << KBC_DEBOUNCE_CNT_SHIFT;
	val |= 1 << KBC_FIFO_TH_CNT_SHIFT;	/* fifo interrupt threshold */
	val |= KBC_CONTROL_KBC_EN;		/* enable */
	writel(val, &kbc->control);

	config.start_time_ms = get_timer(0);
	config.last_poll_ms = config.next_repeat_ms = get_timer(0);
	config.first_scan = 1;
}

/**
 * Set up the tegra keyboard. This is called by the stdio device handler
 *
 * We want to do this init when the keyboard is actually used rather than
 * at start-up, since keyboard input may not currently be selected.
 *
 * Once the keyboard starts there will be a period during which we must
 * wait for the keyboard to init. We do this only when a key is first
 * read - see kbd_wait_for_fifo_init().
 *
 * @return 0 if ok, -ve on error
 */
static int init_tegra_keyboard(void)
{
	/* check if already created */
	if (config.created)
		return 0;

#ifdef CONFIG_OF_CONTROL
	int	node;

	node = fdtdec_next_compatible(gd->fdt_blob, 0,
					  COMPAT_NVIDIA_TEGRA20_KBC);
	if (node < 0) {
		debug("%s: cannot locate keyboard node\n", __func__);
		return node;
	}
	config.kbc = (struct kbc_tegra *)fdtdec_get_addr(gd->fdt_blob,
		       node, "reg");
	if ((fdt_addr_t)config.kbc == FDT_ADDR_T_NONE) {
		debug("%s: No keyboard register found\n", __func__);
		return -1;
	}
	input_set_delays(&config.input, KBC_REPEAT_DELAY_MS,
			KBC_REPEAT_RATE_MS);

	/* Decode the keyboard matrix information (16 rows, 8 columns) */
	if (key_matrix_init(&config.matrix, 16, 8, 1)) {
		debug("%s: Could not init key matrix\n", __func__);
		return -1;
	}
	if (key_matrix_decode_fdt(&config.matrix, gd->fdt_blob, node)) {
		debug("%s: Could not decode key matrix from fdt\n", __func__);
		return -1;
	}
	if (config.matrix.fn_keycode) {
		if (input_add_table(&config.input, KEY_FN, -1,
				    config.matrix.fn_keycode,
				    config.matrix.key_count))
			return -1;
	}
#else
#error "Tegra keyboard driver requires FDT definitions"
#endif

	/* Set up pin mux and enable the clock */
	funcmux_select(PERIPH_ID_KBC, FUNCMUX_DEFAULT);
	clock_enable(PERIPH_ID_KBC);
	config_kbc_gpio(config.kbc);

	tegra_kbc_open();
	config.created = 1;
	debug("%s: Tegra keyboard ready\n", __func__);

	return 0;
}

int drv_keyboard_init(void)
{
	struct stdio_dev dev;
	char *stdinname = getenv("stdin");
	int error;

	if (input_init(&config.input, 0)) {
		debug("%s: Cannot set up input\n", __func__);
		return -1;
	}
	config.input.read_keys = tegra_kbc_check;

	memset(&dev, '\0', sizeof(dev));
	strcpy(dev.name, "tegra-kbc");
	dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	dev.getc = kbd_getc;
	dev.tstc = kbd_tstc;
	dev.start = init_tegra_keyboard;

	/* Register the device. init_tegra_keyboard() will be called soon */
	error = input_stdio_register(&dev);
	if (error)
		return error;
#ifdef CONFIG_CONSOLE_MUX
	error = iomux_doenv(stdin, stdinname);
	if (error)
		return error;
#endif
	return 0;
}
