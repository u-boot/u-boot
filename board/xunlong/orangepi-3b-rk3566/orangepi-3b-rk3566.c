// SPDX-License-Identifier: GPL-2.0+

#include <env.h>
#include <asm/gpio.h>

struct board_model {
	int value;
	const char *fdtfile;
	const char *config;
};

static const struct board_model board_models[] = {
	{ 0, "rockchip/rk3566-orangepi-3b-v1.1.dtb", "rk3566-orangepi-3b-v1.1.dtb" },
	{ 1, "rockchip/rk3566-orangepi-3b-v2.1.dtb", "rk3566-orangepi-3b-v2.1.dtb" },
};

static int get_board_value(void)
{
	struct gpio_desc desc;
	int ret;

	/*
	 * GPIO4_C4 (E20):
	 * v1.1.1: x (internal pull-down)
	 * v2.1:   PHY_RESET (external pull-up)
	 */
	ret = dm_gpio_lookup_name("E20", &desc);
	if (ret)
		return ret;

	ret = dm_gpio_request(&desc, "phy_reset");
	if (ret && ret != -EBUSY)
		return ret;

	dm_gpio_set_dir_flags(&desc, GPIOD_IS_IN);
	ret = dm_gpio_get_value(&desc);
	dm_gpio_free(desc.dev, &desc);

	return ret;
}

static const struct board_model *get_board_model(void)
{
	int i, val;

	val = get_board_value();
	if (val < 0)
		return NULL;

	for (i = 0; i < ARRAY_SIZE(board_models); i++) {
		if (val == board_models[i].value)
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

	if (model && (!strcmp(name, model->fdtfile) ||
	              !strcmp(name, model->config)))
		return 0;

	return -EINVAL;
}
