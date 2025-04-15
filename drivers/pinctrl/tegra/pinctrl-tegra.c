// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2023
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <stdlib.h>

#include <asm/arch/pinmux.h>

static void tegra_pinctrl_set_drive(struct udevice *config, int drvcnt)
{
	struct pmux_drvgrp_config *drive_group;
	int i, ret, pad_id, count = 0;
	const char **pads;

	drive_group = kmalloc_array(drvcnt, sizeof(*drive_group), GFP_KERNEL);
	if (!drive_group) {
		log_debug("%s: cannot allocate drive group array\n", __func__);
		return;
	}

	drive_group[0].slwf = dev_read_u32_default(config, "nvidia,slew-rate-falling", PMUX_SLWF_NONE);
	drive_group[0].slwr = dev_read_u32_default(config, "nvidia,slew-rate-rising", PMUX_SLWR_NONE);
	drive_group[0].drvup = dev_read_u32_default(config, "nvidia,pull-up-strength", PMUX_DRVUP_NONE);
	drive_group[0].drvdn = dev_read_u32_default(config, "nvidia,pull-down-strength", PMUX_DRVDN_NONE);
#ifdef TEGRA_PMX_GRPS_HAVE_LPMD
	drive_group[0].lpmd = dev_read_u32_default(config, "nvidia,low-power-mode", PMUX_LPMD_NONE);
#endif
#ifdef TEGRA_PMX_GRPS_HAVE_SCHMT
	drive_group[0].schmt = dev_read_u32_default(config, "nvidia,schmitt", PMUX_SCHMT_NONE);
#endif
#ifdef TEGRA_PMX_GRPS_HAVE_HSM
	drive_group[0].hsm = dev_read_u32_default(config, "nvidia,high-speed-mode", PMUX_HSM_NONE);
#endif

	for (i = 1; i < drvcnt; i++)
		memcpy(&drive_group[i], &drive_group[0], sizeof(drive_group[0]));

	ret = dev_read_string_list(config, "nvidia,pins", &pads);
	if (ret < 0) {
		log_debug("%s: could not parse property nvidia,pins\n", __func__);
		goto exit;
	}

	/*
	 * i goes through all drive instances defined, while
	 * count is increased only if a valid configuration is found
	 */
	for (i = 0; i < drvcnt; i++) {
		for (pad_id = 0; pad_id < PMUX_DRVGRP_COUNT; pad_id++)
			if (tegra_pinctrl_to_drvgrp[pad_id])
				if (!strcmp(pads[i], tegra_pinctrl_to_drvgrp[pad_id])) {
					drive_group[count].drvgrp = pad_id;
					break;
				}

		if (pad_id == PMUX_DRVGRP_COUNT) {
			log_debug("%s: drive %s is not valid\n", __func__, pads[i]);
			continue;
		}

		log_debug("%s(%d) drvmap: %d, %d, %d, %d, %d\n", pads[count], count,
			  drive_group[count].drvgrp, drive_group[count].slwf,
			  drive_group[count].slwr, drive_group[count].drvup,
			  drive_group[count].drvdn);

		count++;
	}

	pinmux_config_drvgrp_table(drive_group, count);

	free(pads);
exit:
	kfree(drive_group);
}

#ifdef TEGRA_PMX_SOC_HAS_MIPI_PAD_CTRL_GRPS
static void tegra_pinctrl_set_mipipad(struct udevice *config, int padcnt)
{
	struct pmux_mipipadctrlgrp_config *mipipad_group;
	int i, ret, pad_id, count = 0;
	const char *function;
	const char **pads;

	mipipad_group = kmalloc_array(padcnt, sizeof(*mipipad_group), GFP_KERNEL);
	if (!mipipad_group) {
		log_debug("%s: cannot allocate mipi pad group array\n", __func__);
		return;
	}

	/* decode function id and fill the first copy of pmux_mipipadctrlgrp_config */
	function = dev_read_string(config, "nvidia,function");
	if (function)
		for (i = 0; i < PMUX_FUNC_COUNT; i++)
			if (tegra_pinctrl_to_func[i])
				if (!strcmp(function, tegra_pinctrl_to_func[i]))
					break;

	if (!function || i == PMUX_FUNC_COUNT) {
		log_debug("%s: pin function is not defined or is not valid\n", __func__);
		goto exit;
	}

	mipipad_group[0].func = i;

	for (i = 1; i < padcnt; i++)
		memcpy(&mipipad_group[i], &mipipad_group[0], sizeof(mipipad_group[0]));

	ret = dev_read_string_list(config, "nvidia,pins", &pads);
	if (ret < 0) {
		log_debug("%s: could not parse property nvidia,pins\n", __func__);
		goto exit;
	}

	/*
	 * i goes through all pin instances defined, while
	 * count is increased only if a valid configuration is found
	 */
	for (i = 0; i < padcnt; i++) {
		for (pad_id = 0; pad_id < PMUX_MIPIPADCTRLGRP_COUNT; pad_id++)
			if (tegra_pinctrl_to_mipipadgrp[pad_id])
				if (!strcmp(pads[i], tegra_pinctrl_to_mipipadgrp[pad_id])) {
					mipipad_group[count].grp = pad_id;
					break;
				}

		if (pad_id == PMUX_MIPIPADCTRLGRP_COUNT) {
			log_debug("%s: drive %s is not valid\n", __func__, pads[i]);
			continue;
		}

		count++;
	}

	pinmux_config_mipipadctrlgrp_table(mipipad_group, count);

	free(pads);
exit:
	kfree(mipipad_group);
}
#else
static void tegra_pinctrl_set_mipipad(struct udevice *config, int padcnt) { }
#endif

static void tegra_pinctrl_set_pin(struct udevice *config, int pincnt)
{
	struct pmux_pingrp_config *pinmux_group;
	int i, ret, pin_id, count = 0;
	const char *function;
	const char **pins;

	pinmux_group = kmalloc_array(pincnt, sizeof(*pinmux_group), GFP_KERNEL);
	if (!pinmux_group) {
		log_debug("%s: cannot allocate pinmux group array\n", __func__);
		return;
	}

	/* decode function id and fill the first copy of pmux_pingrp_config */
	function = dev_read_string(config, "nvidia,function");
	if (function)
		for (i = 0; i < PMUX_FUNC_COUNT; i++)
			if (tegra_pinctrl_to_func[i])
				if (!strcmp(function, tegra_pinctrl_to_func[i]))
					break;

	if (!function || i == PMUX_FUNC_COUNT) {
		log_debug("%s: pin function is not defined or is not valid\n", __func__);
		goto exit;
	}

	pinmux_group[0].func = i;

	pinmux_group[0].pull = dev_read_u32_default(config, "nvidia,pull", PMUX_PULL_NORMAL);
	pinmux_group[0].tristate = dev_read_u32_default(config, "nvidia,tristate", PMUX_TRI_TRISTATE);
#ifdef TEGRA_PMX_PINS_HAVE_E_INPUT
	pinmux_group[0].io = dev_read_u32_default(config, "nvidia,enable-input", PMUX_PIN_NONE);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_LOCK
	pinmux_group[0].lock = dev_read_u32_default(config, "nvidia,lock", PMUX_PIN_LOCK_DEFAULT);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_OD
	pinmux_group[0].od = dev_read_u32_default(config, "nvidia,open-drain", PMUX_PIN_OD_DEFAULT);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_IO_RESET
	pinmux_group[0].ioreset = dev_read_u32_default(config, "nvidia,io-reset", PMUX_PIN_IO_RESET_DEFAULT);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_RCV_SEL
	pinmux_group[0].rcv_sel = dev_read_u32_default(config, "nvidia,rcv-sel", PMUX_PIN_RCV_SEL_DEFAULT);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_E_IO_HV
	pinmux_group[0].e_io_hv = dev_read_u32_default(config, "nvidia,io-hv", PMUX_PIN_E_IO_HV_DEFAULT);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_SCHMT
	pinmux_group[0].schmt = dev_read_u32_default(config, "nvidia,schmitt", PMUX_SCHMT_NONE);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_HSM
	pinmux_group[0].hsm = dev_read_u32_default(config, "nvidia,high-speed-mode", PMUX_HSM_NONE);
#endif

	for (i = 1; i < pincnt; i++)
		memcpy(&pinmux_group[i], &pinmux_group[0], sizeof(pinmux_group[0]));

	ret = dev_read_string_list(config, "nvidia,pins", &pins);
	if (ret < 0) {
		log_debug("%s: could not parse property nvidia,pins\n", __func__);
		goto exit;
	}

	/*
	 * i goes through all pin instances defined, while
	 * count is increased only if a valid configuration is found
	 */
	for (i = 0; i < pincnt; i++) {
		for (pin_id = 0; pin_id < PMUX_PINGRP_COUNT; pin_id++)
			if (tegra_pinctrl_to_pingrp[pin_id])
				if (!strcmp(pins[i], tegra_pinctrl_to_pingrp[pin_id])) {
					pinmux_group[count].pingrp = pin_id;
					break;
				}

		if (pin_id == PMUX_PINGRP_COUNT) {
			log_debug("%s: pin %s is not valid\n", __func__, pins[i]);
			continue;
		}

		log_debug("%s(%d) pinmap: %d, %d, %d, %d\n", pins[count], count,
			  pinmux_group[count].pingrp, pinmux_group[count].func,
			  pinmux_group[count].pull, pinmux_group[count].tristate);

		count++;
	}

	pinmux_config_pingrp_table(pinmux_group, count);

	free(pins);
exit:
	kfree(pinmux_group);
}

static int tegra_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct udevice *child;
	int ret;
	const char *name;

	device_foreach_child(child, config) {
		/* Pinmux node can contain pins and drives */
		ret = dev_read_string_index(child, "nvidia,pins", 0,
					    &name);
		if (ret < 0) {
			log_debug("%s: could not parse property nvidia,pins\n", __func__);
			return ret;
		}

		ret = dev_read_string_count(child, "nvidia,pins");
		if (ret < 0) {
			log_debug("%s: could not count nvidia,pins\n", __func__);
			return ret;
		}

		if (!strncmp(name, "drive_", 6))
			/* Drive node is detected */
			tegra_pinctrl_set_drive(child, ret);
		else if (!strncmp(name, "mipi_pad_ctrl_", 14))
			/* Handle T124 specific pinconfig */
			tegra_pinctrl_set_mipipad(child, ret);
		else
			/* Pin node is detected */
			tegra_pinctrl_set_pin(child, ret);
	}

	return 0;
}

static int tegra_pinctrl_get_pins_count(struct udevice *dev)
{
	return PMUX_PINGRP_COUNT;
}

static const char *tegra_pinctrl_get_pin_name(struct udevice *dev,
					      unsigned int selector)
{
	return tegra_pinctrl_to_pingrp[selector];
}

static int tegra_pinctrl_get_groups_count(struct udevice *dev)
{
	return PMUX_DRVGRP_COUNT;
}

static const char *tegra_pinctrl_get_group_name(struct udevice *dev,
						unsigned int selector)
{
	return tegra_pinctrl_to_drvgrp[selector];
}

static int tegra_pinctrl_get_functions_count(struct udevice *dev)
{
	return PMUX_FUNC_COUNT;
}

static const char *tegra_pinctrl_get_function_name(struct udevice *dev,
						   unsigned int selector)
{
	return tegra_pinctrl_to_func[selector];
}

const struct pinctrl_ops tegra_pinctrl_ops = {
	.get_pins_count = tegra_pinctrl_get_pins_count,
	.get_pin_name = tegra_pinctrl_get_pin_name,
	.get_groups_count = tegra_pinctrl_get_groups_count,
	.get_group_name = tegra_pinctrl_get_group_name,
	.get_functions_count = tegra_pinctrl_get_functions_count,
	.get_function_name = tegra_pinctrl_get_function_name,
	.set_state = tegra_pinctrl_set_state,
};

static int tegra_pinctrl_bind(struct udevice *dev)
{
	/*
	 * Make sure that the pinctrl driver gets probed after binding
	 * to provide initial configuration and assure that further
	 * probed devices are working correctly.
	 */
	dev_or_flags(dev, DM_FLAG_PROBE_AFTER_BIND);

	return 0;
}

static const struct udevice_id tegra_pinctrl_ids[] = {
	{ .compatible = "nvidia,tegra30-pinmux" },
	{ .compatible = "nvidia,tegra114-pinmux" },
	{ .compatible = "nvidia,tegra124-pinmux" },
	{ },
};

U_BOOT_DRIVER(tegra_pinctrl) = {
	.name		= "tegra_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= tegra_pinctrl_ids,
	.bind		= tegra_pinctrl_bind,
	.ops		= &tegra_pinctrl_ops,
};
