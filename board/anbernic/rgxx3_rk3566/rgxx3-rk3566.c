// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Chris Morgan <macromorgan@hotmail.com>
 */

#include <abuf.h>
#include <adc.h>
#include <asm/io.h>
#include <dm.h>
#include <linux/delay.h>
#include <pwm.h>
#include <rng.h>
#include <stdlib.h>
#include <mmc.h>
#include <env.h>

#define GPIO0_BASE		0xfdd60000
#define GPIO_SWPORT_DR_H	0x0004
#define GPIO_SWPORT_DDR_H	0x000c
#define GPIO_A5			BIT(5)
#define GPIO_A6			BIT(6)

#define GPIO_WRITEMASK(bits)	((bits) << 16)

#define DTB_DIR			"rockchip/"

struct rg3xx_model {
	const char *board;
	const char *board_name;
	const char *fdtfile;
};

enum rgxx3_device_id {
	RG353M,
	RG353P,
	RG353V,
	RG353VS,
	RG503,
};

static const struct rg3xx_model rg3xx_model_details[] = {
	[RG353M] = {
		"rk3566-anbernic-rg353m",
		"RG353M",
		DTB_DIR "rk3566-anbernic-rg353m.dtb",
	},
	[RG353P] = {
		"rk3566-anbernic-rg353p",
		"RG353P",
		DTB_DIR "rk3566-anbernic-rg353p.dtb",
	},
	[RG353V] = {
		"rk3566-anbernic-rg353v",
		"RG353V",
		DTB_DIR "rk3566-anbernic-rg353v.dtb",
	},
	[RG353VS] = {
		"rk3566-anbernic-rg353vs",
		"RG353VS",
		DTB_DIR "rk3566-anbernic-rg353vs.dtb",
	},
	[RG503] = {
		"rk3566-anbernic-rg503",
		"RG503",
		DTB_DIR "rk3566-anbernic-rg503.dtb",
	},
};

/*
 * Start LED very early so user knows device is on. Set color
 * to amber.
 */
void spl_board_init(void)
{
	/* Set GPIO0_A5 and GPIO0_A6 to output. */
	writel(GPIO_WRITEMASK(GPIO_A6 | GPIO_A5) | (GPIO_A6 | GPIO_A5),
	       (GPIO0_BASE + GPIO_SWPORT_DDR_H));
	/* Set GPIO0_A5 to 0 and GPIO0_A6 to 1. */
	writel(GPIO_WRITEMASK(GPIO_A6 | GPIO_A5) | GPIO_A6,
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
 * presence of an eMMC on mmc0, and querying the DSI panel (TODO).
 */
int rgxx3_detect_device(void)
{
	u32 adc_info;
	int ret;
	int board_id = -ENXIO;
	struct mmc *mmc;

	ret = adc_channel_single_shot("saradc@fe720000", 1, &adc_info);
	if (ret) {
		printf("Read SARADC failed with error %d\n", ret);
		return ret;
	}

	/* Observed value 517. */
	if (adc_info > 505 && adc_info < 530)
		board_id = RG353M;
	/* Observed value 695. */
	if (adc_info > 680 && adc_info < 710)
		board_id = RG353V;
	/* Documented value 860. */
	if (adc_info > 850 && adc_info < 870)
		board_id = RG353P;
	/* Observed value 1023. */
	if (adc_info > 1010)
		board_id = RG503;

	/*
	 * Try to access the eMMC on an RG353V. If it's missing, it's
	 * an RG353VS. Note we could also check for a touchscreen at
	 * 0x1a on i2c2.
	 */
	if (board_id == RG353V) {
		mmc = find_mmc_device(0);
		if (mmc) {
			ret = mmc_init(mmc);
			if (ret)
				board_id = RG353VS;
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

	/* Turn off orange LED and turn on green LED. */
	writel(GPIO_WRITEMASK(GPIO_A6 | GPIO_A5) | GPIO_A5,
	       (GPIO0_BASE + GPIO_SWPORT_DR_H));

	ret = rgxx3_detect_device();
	if (ret) {
		printf("Unable to detect device type: %d\n", ret);
		return ret;
	}

	if (IS_ENABLED(CONFIG_DM_PWM))
		startup_buzz();

	return 0;
}
