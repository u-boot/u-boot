/*
 * Copyright (C) 2014-2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <power/pmic.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

int regulator_mode(struct udevice *dev, struct dm_regulator_mode **modep)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	*modep = NULL;

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata)
		return -ENXIO;

	*modep = uc_pdata->mode;
	return uc_pdata->mode_count;
}

int regulator_get_value(struct udevice *dev)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->get_value)
		return -ENOSYS;

	return ops->get_value(dev);
}

int regulator_set_value(struct udevice *dev, int uV)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->set_value)
		return -ENOSYS;

	return ops->set_value(dev, uV);
}

int regulator_get_current(struct udevice *dev)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->get_current)
		return -ENOSYS;

	return ops->get_current(dev);
}

int regulator_set_current(struct udevice *dev, int uA)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->set_current)
		return -ENOSYS;

	return ops->set_current(dev, uA);
}

bool regulator_get_enable(struct udevice *dev)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->get_enable)
		return -ENOSYS;

	return ops->get_enable(dev);
}

int regulator_set_enable(struct udevice *dev, bool enable)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->set_enable)
		return -ENOSYS;

	return ops->set_enable(dev, enable);
}

int regulator_get_mode(struct udevice *dev)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->get_mode)
		return -ENOSYS;

	return ops->get_mode(dev);
}

int regulator_set_mode(struct udevice *dev, int mode)
{
	const struct dm_regulator_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->set_mode)
		return -ENOSYS;

	return ops->set_mode(dev, mode);
}

int regulator_get_by_platname(const char *plat_name, struct udevice **devp)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct udevice *dev;
	int ret;

	*devp = NULL;

	for (ret = uclass_find_first_device(UCLASS_REGULATOR, &dev); dev;
	     ret = uclass_find_next_device(&dev)) {
		if (ret)
			continue;

		uc_pdata = dev_get_uclass_platdata(dev);
		if (!uc_pdata || strcmp(plat_name, uc_pdata->name))
			continue;

		return uclass_get_device_tail(dev, 0, devp);
	}

	debug("%s: can't find: %s\n", __func__, plat_name);

	return -ENODEV;
}

int regulator_get_by_devname(const char *devname, struct udevice **devp)
{
	return uclass_get_device_by_name(UCLASS_REGULATOR, devname, devp);
}

static int failed(int ret, bool verbose, const char *fmt, ...)
{
	va_list args;
	char buf[64];

	if (verbose == false)
		return ret;

	va_start(args, fmt);
	vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	printf(buf);

	if (!ret)
		return 0;

	printf(" (ret: %d)", ret);

	return ret;
}

int regulator_autoset(const char *platname,
		      struct udevice **devp,
		      bool verbose)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct udevice *dev;
	int ret;

	if (devp)
		*devp = NULL;

	ret = regulator_get_by_platname(platname, &dev);
	if (ret) {
		error("Can get the regulator: %s!", platname);
		return ret;
	}

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata) {
		error("Can get the regulator %s uclass platdata!", platname);
		return -ENXIO;
	}

	if (!uc_pdata->always_on && !uc_pdata->boot_on)
		goto retdev;

	if (verbose)
		printf("%s@%s: ", dev->name, uc_pdata->name);

	/* Those values are optional (-ENODATA if unset) */
	if ((uc_pdata->min_uV != -ENODATA) &&
	    (uc_pdata->max_uV != -ENODATA) &&
	    (uc_pdata->min_uV == uc_pdata->max_uV)) {
		ret = regulator_set_value(dev, uc_pdata->min_uV);
		if (failed(ret, verbose, "set %d uV", uc_pdata->min_uV))
			goto exit;
	}

	/* Those values are optional (-ENODATA if unset) */
	if ((uc_pdata->min_uA != -ENODATA) &&
	    (uc_pdata->max_uA != -ENODATA) &&
	    (uc_pdata->min_uA == uc_pdata->max_uA)) {
		ret = regulator_set_current(dev, uc_pdata->min_uA);
		if (failed(ret, verbose, "; set %d uA", uc_pdata->min_uA))
			goto exit;
	}

	ret = regulator_set_enable(dev, true);
	if (failed(ret, verbose, "; enabling", uc_pdata->min_uA))
		goto exit;

retdev:
	if (devp)
		*devp = dev;
exit:
	if (verbose)
		printf("\n");

	return ret;
}

int regulator_list_autoset(const char *list_platname[],
			   struct udevice *list_devp[],
			   bool verbose)
{
	struct udevice *dev;
	int error = 0, i = 0, ret;

	while (list_platname[i]) {
		ret = regulator_autoset(list_platname[i], &dev, verbose);
		if (ret & !error)
			error = ret;

		if (list_devp)
			list_devp[i] = dev;

		i++;
	}

	return error;
}

static bool regulator_name_is_unique(struct udevice *check_dev,
				     const char *check_name)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct udevice *dev;
	int check_len = strlen(check_name);
	int ret;
	int len;

	for (ret = uclass_find_first_device(UCLASS_REGULATOR, &dev); dev;
	     ret = uclass_find_next_device(&dev)) {
		if (ret || dev == check_dev)
			continue;

		uc_pdata = dev_get_uclass_platdata(dev);
		len = strlen(uc_pdata->name);
		if (len != check_len)
			continue;

		if (!strcmp(uc_pdata->name, check_name))
			return false;
	}

	return true;
}

static int regulator_post_bind(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	int offset = dev->of_offset;
	const void *blob = gd->fdt_blob;
	const char *property = "regulator-name";

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata)
		return -ENXIO;

	/* Regulator's mandatory constraint */
	uc_pdata->name = fdt_getprop(blob, offset, property, NULL);
	if (!uc_pdata->name) {
		debug("%s: dev: %s has no property 'regulator-name'\n",
		      __func__, dev->name);
		return -EINVAL;
	}

	if (regulator_name_is_unique(dev, uc_pdata->name))
		return 0;

	error("\"%s\" of dev: \"%s\", has nonunique value: \"%s\"",
	      property, dev->name, uc_pdata->name);

	return -EINVAL;
}

static int regulator_pre_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	int offset = dev->of_offset;

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata)
		return -ENXIO;

	/* Regulator's optional constraints */
	uc_pdata->min_uV = fdtdec_get_int(gd->fdt_blob, offset,
					  "regulator-min-microvolt", -ENODATA);
	uc_pdata->max_uV = fdtdec_get_int(gd->fdt_blob, offset,
					  "regulator-max-microvolt", -ENODATA);
	uc_pdata->min_uA = fdtdec_get_int(gd->fdt_blob, offset,
					  "regulator-min-microamp", -ENODATA);
	uc_pdata->max_uA = fdtdec_get_int(gd->fdt_blob, offset,
					  "regulator-max-microamp", -ENODATA);
	uc_pdata->always_on = fdtdec_get_bool(gd->fdt_blob, offset,
					      "regulator-always-on");
	uc_pdata->boot_on = fdtdec_get_bool(gd->fdt_blob, offset,
					    "regulator-boot-on");

	return 0;
}

UCLASS_DRIVER(regulator) = {
	.id		= UCLASS_REGULATOR,
	.name		= "regulator",
	.post_bind	= regulator_post_bind,
	.pre_probe	= regulator_pre_probe,
	.per_device_platdata_auto_alloc_size =
				sizeof(struct dm_regulator_uclass_platdata),
};
