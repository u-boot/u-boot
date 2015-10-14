/*
 * Copyright (C) 2015  Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <linux/err.h>
#include <linux/list.h>
#include <dm/device.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(PINCTRL_FULL)
/**
 * pinctrl_config_one() - apply pinctrl settings for a single node
 *
 * @config: pin configuration node
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_config_one(struct udevice *config)
{
	struct udevice *pctldev;
	const struct pinctrl_ops *ops;

	pctldev = config;
	for (;;) {
		pctldev = dev_get_parent(pctldev);
		if (!pctldev) {
			dev_err(config, "could not find pctldev\n");
			return -EINVAL;
		}
		if (pctldev->uclass->uc_drv->id == UCLASS_PINCTRL)
			break;
	}

	ops = pinctrl_get_ops(pctldev);
	return ops->set_state(pctldev, config);
}

/**
 * pinctrl_select_state_full() - full implementation of pinctrl_select_state
 *
 * @dev: peripheral device
 * @statename: state name, like "default"
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_select_state_full(struct udevice *dev, const char *statename)
{
	const void *fdt = gd->fdt_blob;
	int node = dev->of_offset;
	char propname[32]; /* long enough */
	const fdt32_t *list;
	uint32_t phandle;
	int config_node;
	struct udevice *config;
	int state, size, i, ret;

	state = fdt_find_string(fdt, node, "pinctrl-names", statename);
	if (state < 0) {
		char *end;
		/*
		 * If statename is not found in "pinctrl-names",
		 * assume statename is just the integer state ID.
		 */
		state = simple_strtoul(statename, &end, 10);
		if (*end)
			return -EINVAL;
	}

	snprintf(propname, sizeof(propname), "pinctrl-%d", state);
	list = fdt_getprop(fdt, node, propname, &size);
	if (!list)
		return -EINVAL;

	size /= sizeof(*list);
	for (i = 0; i < size; i++) {
		phandle = fdt32_to_cpu(*list++);

		config_node = fdt_node_offset_by_phandle(fdt, phandle);
		if (config_node < 0) {
			dev_err(dev, "prop %s index %d invalid phandle\n",
				propname, i);
			return -EINVAL;
		}
		ret = uclass_get_device_by_of_offset(UCLASS_PINCONFIG,
						     config_node, &config);
		if (ret)
			return ret;

		ret = pinctrl_config_one(config);
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * pinconfig_post-bind() - post binding for PINCONFIG uclass
 * Recursively bind its children as pinconfig devices.
 *
 * @dev: pinconfig device
 * @return: 0 on success, or negative error code on failure
 */
static int pinconfig_post_bind(struct udevice *dev)
{
	const void *fdt = gd->fdt_blob;
	int offset = dev->of_offset;
	const char *name;
	int ret;

	for (offset = fdt_first_subnode(fdt, offset);
	     offset > 0;
	     offset = fdt_next_subnode(fdt, offset)) {
		/*
		 * If this node has "compatible" property, this is not
		 * a pin configuration node, but a normal device. skip.
		 */
		fdt_get_property(fdt, offset, "compatible", &ret);
		if (ret >= 0)
			continue;

		if (ret != -FDT_ERR_NOTFOUND)
			return ret;

		name = fdt_get_name(fdt, offset, NULL);
		if (!name)
			return -EINVAL;
		ret = device_bind_driver_to_node(dev, "pinconfig", name,
						 offset, NULL);
		if (ret)
			return ret;
	}

	return 0;
}

UCLASS_DRIVER(pinconfig) = {
	.id = UCLASS_PINCONFIG,
	.post_bind = pinconfig_post_bind,
	.name = "pinconfig",
};

U_BOOT_DRIVER(pinconfig_generic) = {
	.name = "pinconfig",
	.id = UCLASS_PINCONFIG,
};

#else
static int pinctrl_select_state_full(struct udevice *dev, const char *statename)
{
	return -ENODEV;
}

static int pinconfig_post_bind(struct udevice *dev)
{
	return 0;
}
#endif

/**
 * pinctrl_select_state_simple() - simple implementation of pinctrl_select_state
 *
 * @dev: peripheral device
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_select_state_simple(struct udevice *dev)
{
	struct udevice *pctldev;
	struct pinctrl_ops *ops;
	int ret;

	/*
	 * For simplicity, assume the first device of PINCTRL uclass
	 * is the correct one.  This is most likely OK as there is
	 * usually only one pinctrl device on the system.
	 */
	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pctldev);
	if (ret)
		return ret;

	ops = pinctrl_get_ops(pctldev);
	if (!ops->set_state_simple) {
		dev_dbg(dev, "set_state_simple op missing\n");
		return -ENOSYS;
	}

	return ops->set_state_simple(pctldev, dev);
}

int pinctrl_select_state(struct udevice *dev, const char *statename)
{
	/*
	 * Try full-implemented pinctrl first.
	 * If it fails or is not implemented, try simple one.
	 */
	if (pinctrl_select_state_full(dev, statename))
		return pinctrl_select_state_simple(dev);

	return 0;
}

int pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (!ops->request)
		return -ENOSYS;

	return ops->request(dev, func, flags);
}

int pinctrl_request_noflags(struct udevice *dev, int func)
{
	return pinctrl_request(dev, func, 0);
}

int pinctrl_get_periph_id(struct udevice *dev, struct udevice *periph)
{
	struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (!ops->get_periph_id)
		return -ENOSYS;

	return ops->get_periph_id(dev, periph);
}

/**
 * pinconfig_post-bind() - post binding for PINCTRL uclass
 * Recursively bind child nodes as pinconfig devices in case of full pinctrl.
 *
 * @dev: pinctrl device
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_post_bind(struct udevice *dev)
{
	const struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (!ops) {
		dev_dbg(dev, "ops is not set.  Do not bind.\n");
		return -EINVAL;
	}

	/*
	 * If set_state callback is set, we assume this pinctrl driver is the
	 * full implementation.  In this case, its child nodes should be bound
	 * so that peripheral devices can easily search in parent devices
	 * during later DT-parsing.
	 */
	if (ops->set_state)
		return pinconfig_post_bind(dev);

	return 0;
}

UCLASS_DRIVER(pinctrl) = {
	.id = UCLASS_PINCTRL,
	.post_bind = pinctrl_post_bind,
	.name = "pinctrl",
};
