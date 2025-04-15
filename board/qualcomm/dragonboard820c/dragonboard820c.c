// SPDX-License-Identifier: GPL-2.0+
/*
 * Board init file for Dragonboard 820C
 *
 * (C) Copyright 2017 Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>
 */

#include <button.h>
#include <cpu_func.h>
#include <init.h>
#include <env.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <linux/arm-smccc.h>
#include <linux/psci.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <asm/psci.h>
#include <asm/gpio.h>

#define TLMM_BASE_ADDR                  (0x1010000)

/* Strength (sdc1) */
#define SDC1_HDRV_PULL_CTL_REG          (TLMM_BASE_ADDR + 0x0012D000)

DECLARE_GLOBAL_DATA_PTR;

static void sdhci_power_init(void)
{
	const u32 TLMM_PULL_MASK = 0x3;
	const u32 TLMM_HDRV_MASK = 0x7;

	struct tlmm_cfg {
		u32 bit;  /* bit in the register      */
		u8 mask;  /* mask clk/dat/cmd control */
		u8 val;
	};

	/* bit offsets in the sdc tlmm register */
	enum {  SDC1_DATA_HDRV = 0,
		SDC1_CMD_HDRV  = 3,
		SDC1_CLK_HDRV  = 6,
		SDC1_DATA_PULL = 9,
		SDC1_CMD_PULL  = 11,
		SDC1_CLK_PULL  = 13,
		SDC1_RCLK_PULL = 15,
	};

	enum {  TLMM_PULL_DOWN	 = 0x1,
		TLMM_PULL_UP   = 0x3,
		TLMM_NO_PULL   = 0x0,
	};

	enum {  TLMM_CUR_VAL_10MA = 0x04,
		TLMM_CUR_VAL_16MA = 0x07,
	};
	int i;

	/* drive strength configs for sdhc pins */
	const struct tlmm_cfg hdrv[] = {

		{ SDC1_CLK_HDRV,  TLMM_CUR_VAL_16MA, TLMM_HDRV_MASK, },
		{ SDC1_CMD_HDRV,  TLMM_CUR_VAL_10MA, TLMM_HDRV_MASK, },
		{ SDC1_DATA_HDRV, TLMM_CUR_VAL_10MA, TLMM_HDRV_MASK, },
	};

	/* pull configs for sdhc pins */
	const struct tlmm_cfg pull[] = {

		{ SDC1_CLK_PULL,  TLMM_NO_PULL, TLMM_PULL_MASK, },
		{ SDC1_CMD_PULL,  TLMM_PULL_UP, TLMM_PULL_MASK, },
		{ SDC1_DATA_PULL, TLMM_PULL_UP, TLMM_PULL_MASK, },
	};

	const struct tlmm_cfg rclk[] = {

		{ SDC1_RCLK_PULL, TLMM_PULL_DOWN, TLMM_PULL_MASK,},
	};

	for (i = 0; i < ARRAY_SIZE(hdrv); i++)
		clrsetbits_le32(SDC1_HDRV_PULL_CTL_REG,
				hdrv[i].mask << hdrv[i].bit,
			hdrv[i].val  << hdrv[i].bit);

	for (i = 0; i < ARRAY_SIZE(pull); i++)
		clrsetbits_le32(SDC1_HDRV_PULL_CTL_REG,
				pull[i].mask << pull[i].bit,
			pull[i].val  << pull[i].bit);

	for (i = 0; i < ARRAY_SIZE(rclk); i++)
		clrsetbits_le32(SDC1_HDRV_PULL_CTL_REG,
				rclk[i].mask << rclk[i].bit,
			rclk[i].val  << rclk[i].bit);
}

void qcom_board_init(void)
{
	sdhci_power_init();
}

/* Check for vol- button - if pressed - stop autoboot */
int misc_init_r(void)
{
	struct udevice *btn;
	int ret;
	enum button_state_t state;

	ret = button_get_by_label("Power Button", &btn);
	if (ret < 0) {
		printf("Couldn't find power button!\n");
		return ret;
	}

	state = button_get_state(btn);
	if (state == BUTTON_ON) {
		env_set("bootdelay", "-1");
		printf("Power button pressed - dropping to console.\n");
	}

	return 0;
}
