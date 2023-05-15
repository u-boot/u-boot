// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Chris Morgan <macromorgan@hotmail.com>
 */

#include <abuf.h>
#include <adc.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/lists.h>
#include <env.h>
#include <fdt_support.h>
#include <linux/delay.h>
#include <mmc.h>
#include <pwm.h>
#include <rng.h>
#include <stdlib.h>

#define GPIO0_BASE		0xfdd60000
#define GPIO_SWPORT_DR_L	0x0000
#define GPIO_SWPORT_DR_H	0x0004
#define GPIO_SWPORT_DDR_L	0x0008
#define GPIO_SWPORT_DDR_H	0x000c
#define GPIO_C5			BIT(5)
#define GPIO_C6			BIT(6)
#define GPIO_C7			BIT(7)

#define GPIO_WRITEMASK(bits)	((bits) << 16)

#define DTB_DIR			"rockchip/"

struct rg3xx_model {
	const u16 adc_value;
	const char *board;
	const char *board_name;
	const char *fdtfile;
};

enum rgxx3_device_id {
	RG353M,
	RG353P,
	RG353V,
	RG503,
	/* Devices with duplicate ADC value */
	RG353PS,
	RG353VS,
};

static const struct rg3xx_model rg3xx_model_details[] = {
	[RG353M] = {
		517, /* Observed average from device */
		"rk3566-anbernic-rg353m",
		"RG353M",
		DTB_DIR "rk3566-anbernic-rg353p.dtb", /* Identical devices */
	},
	[RG353P] = {
		860, /* Documented value of 860 */
		"rk3566-anbernic-rg353p",
		"RG353P",
		DTB_DIR "rk3566-anbernic-rg353p.dtb",
	},
	[RG353V] = {
		695, /* Observed average from device */
		"rk3566-anbernic-rg353v",
		"RG353V",
		DTB_DIR "rk3566-anbernic-rg353v.dtb",
	},
	[RG503] = {
		1023, /* Observed average from device */
		"rk3566-anbernic-rg503",
		"RG503",
		DTB_DIR "rk3566-anbernic-rg503.dtb",
	},
	/* Devices with duplicate ADC value */
	[RG353PS] = {
		860, /* Observed average from device */
		"rk3566-anbernic-rg353ps",
		"RG353PS",
		DTB_DIR "rk3566-anbernic-rg353ps.dtb",
	},
	[RG353VS] = {
		695, /* Gathered from second hand information */
		"rk3566-anbernic-rg353vs",
		"RG353VS",
		DTB_DIR "rk3566-anbernic-rg353vs.dtb",
	},
};

/*
 * Start LED very early so user knows device is on. Set color
 * to red.
 */
void spl_board_init(void)
{
	/* Set GPIO0_C5, GPIO0_C6, and GPIO0_C7 to output. */
	writel(GPIO_WRITEMASK(GPIO_C7 | GPIO_C6 | GPIO_C5) | \
	       (GPIO_C7 | GPIO_C6 | GPIO_C5),
	       (GPIO0_BASE + GPIO_SWPORT_DDR_H));
	/* Set GPIO0_C5 and GPIO_C6 to 0 and GPIO0_C7 to 1. */
	writel(GPIO_WRITEMASK(GPIO_C7 | GPIO_C6 | GPIO_C5) | GPIO_C7,
	       (GPIO0_BASE + GPIO_SWPORT_DR_H));
}

/* Use hardware rng to seed Linux random. */
int board_rng_seed(struct abuf *buf)
{
	struct udevice *dev;
	size_t len = 0x8;
	u64 *data;

	data = malloc(len);
	if (!data) {
		printf("Out of memory\n");
		return -ENOMEM;
	}

	if (uclass_get_device(UCLASS_RNG, 0, &dev) || !dev) {
		printf("No RNG device\n");
		return -ENODEV;
	}

	if (dm_rng_read(dev, data, len)) {
		printf("Reading RNG failed\n");
		return -EIO;
	}

	abuf_init_set(buf, data, len);

	return 0;
}

/*
 * Buzz the buzzer so the user knows something is going on. Make it
 * optional in case PWM is disabled.
 */
void __maybe_unused startup_buzz(void)
{
	struct udevice *dev;
	int err;

	err = uclass_get_device(UCLASS_PWM, 0, &dev);
	if (err)
		printf("pwm not found\n");

	pwm_set_enable(dev, 0, 1);
	mdelay(200);
	pwm_set_enable(dev, 0, 0);
}

/* Detect which Anbernic RGXX3 device we are using so as to load the
 * correct devicetree for Linux. Set an environment variable once
 * found. The detection depends on the value of ADC channel 1, the
 * presence of an eMMC on mmc0, and querying the DSI panel.
 */
int rgxx3_detect_device(void)
{
	u32 adc_info;
	int ret, i;
	int board_id = -ENXIO;
	struct mmc *mmc;

	ret = adc_channel_single_shot("saradc@fe720000", 1, &adc_info);
	if (ret) {
		printf("Read SARADC failed with error %d\n", ret);
		return ret;
	}

	/*
	 * Get the correct device from the table. The ADC value is
	 * determined by a resistor on ADC channel 0. The hardware
	 * design calls for no more than a 1% variance on the
	 * resistor, so assume a +- value of 15 should be enough.
	 */
	for (i = 0; i < ARRAY_SIZE(rg3xx_model_details); i++) {
		u32 adc_min = rg3xx_model_details[i].adc_value - 15;
		u32 adc_max = rg3xx_model_details[i].adc_value + 15;

		if (adc_min < adc_info && adc_max > adc_info) {
			board_id = i;
			break;
		}
	}

	/*
	 * Try to access the eMMC on an RG353V or RG353P. If it's
	 * missing, it's an RG353VS or RG353PS. Note we could also
	 * check for a touchscreen at 0x1a on i2c2.
	 */
	if (board_id == RG353V || board_id == RG353P) {
		mmc = find_mmc_device(0);
		if (mmc) {
			ret = mmc_init(mmc);
			if (ret) {
				if (board_id == RG353V)
					board_id = RG353VS;
				else
					board_id = RG353PS;
			}
		}
	}

	if (board_id < 0)
		return board_id;

	env_set("board", rg3xx_model_details[board_id].board);
	env_set("board_name",
		rg3xx_model_details[board_id].board_name);
	env_set("fdtfile", rg3xx_model_details[board_id].fdtfile);

	return 0;
}

int rk_board_late_init(void)
{
	int ret;

	ret = rgxx3_detect_device();
	if (ret) {
		printf("Unable to detect device type: %d\n", ret);
		return ret;
	}

	/* Turn off red LED and turn on orange LED. */
	writel(GPIO_WRITEMASK(GPIO_C7 | GPIO_C6 | GPIO_C5) | GPIO_C6,
	       (GPIO0_BASE + GPIO_SWPORT_DR_H));

	if (IS_ENABLED(CONFIG_DM_PWM))
		startup_buzz();

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	char *env;

	/* No fixups necessary for the RG503 */
	env = env_get("board_name");
	if (env && (!strcmp(env, rg3xx_model_details[RG503].board_name)))
		return 0;

	/* Change the model name of the RG353M */
	if (env && (!strcmp(env, rg3xx_model_details[RG353M].board_name)))
		fdt_setprop(blob, 0, "model",
			    rg3xx_model_details[RG353M].board_name,
			    sizeof(rg3xx_model_details[RG353M].board_name));

	return 0;
}
