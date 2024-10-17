// SPDX-License-Identifier: GPL-2.0+

#include <adc.h>
#include <env.h>
#include <linux/errno.h>
#include <linux/kernel.h>

#define HW_ID_CHANNEL	5

struct board_model {
	unsigned int low;
	unsigned int high;
	const char *fdtfile;
};

static const struct board_model board_models[] = {
	{  348,  528, "rockchip/rk3588-nanopc-t6.dtb" },
	{ 1957, 2137, "rockchip/rk3588-nanopc-t6-lts.dtb" },
};

static const struct board_model *get_board_model(void)
{
	unsigned int val;
	int i, ret;

	ret = adc_channel_single_shot("adc@fec10000", HW_ID_CHANNEL, &val);
	if (ret)
		return NULL;

	for (i = 0; i < ARRAY_SIZE(board_models); i++) {
		unsigned int min = board_models[i].low;
		unsigned int max = board_models[i].high;

		if (min <= val && val <= max)
			return &board_models[i];
	}

	return NULL;
}

int rk_board_late_init(void)
{
	const struct board_model *model = get_board_model();

	if (model)
		env_set("fdtfile", model->fdtfile);

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	const struct board_model *model = get_board_model();

	if (model && !strcmp(name, model->fdtfile))
		return 0;

	return -EINVAL;
}
