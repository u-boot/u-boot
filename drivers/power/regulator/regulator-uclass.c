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

int regulator_by_platname(const char *plat_name, struct udevice **devp)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct udevice *dev;

	*devp = NULL;

	for (uclass_find_first_device(UCLASS_REGULATOR, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		uc_pdata = dev_get_uclass_platdata(dev);
		if (!uc_pdata || strcmp(plat_name, uc_pdata->name))
			continue;

		return uclass_get_device_tail(dev, 0, devp);
	}

	debug("%s: can't find: %s\n", __func__, plat_name);

	return -ENODEV;
}

int regulator_by_devname(const char *devname, struct udevice **devp)
{
	return uclass_get_device_by_name(UCLASS_REGULATOR, devname, devp);
}

static int setting_failed(int ret, bool verbose, const char *fmt, ...)
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

int regulator_by_platname_autoset_and_enable(const char *platname,
					     struct udevice **devp,
					     bool verbose)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct udevice *dev;
	bool v = verbose;
	int ret;

	if (devp)
		*devp = NULL;

	ret = regulator_by_platname(platname, &dev);
	if (ret) {
		error("Can get the regulator: %s!", platname);
		return ret;
	}

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata) {
		error("Can get the regulator %s uclass platdata!", platname);
		return -ENXIO;
	}

	if (v)
		printf("%s@%s: ", dev->name, uc_pdata->name);

	/* Those values are optional (-ENODATA if unset) */
	if ((uc_pdata->min_uV != -ENODATA) &&
	    (uc_pdata->max_uV != -ENODATA) &&
	    (uc_pdata->min_uV == uc_pdata->max_uV)) {
		ret = regulator_set_value(dev, uc_pdata->min_uV);
		if (setting_failed(ret, v, "set %d uV", uc_pdata->min_uV))
			goto exit;
	}

	/* Those values are optional (-ENODATA if unset) */
	if ((uc_pdata->min_uA != -ENODATA) &&
	    (uc_pdata->max_uA != -ENODATA) &&
	    (uc_pdata->min_uA == uc_pdata->max_uA)) {
		ret = regulator_set_current(dev, uc_pdata->min_uA);
		if (setting_failed(ret, v, "; set %d uA", uc_pdata->min_uA))
			goto exit;
	}

	if (!uc_pdata->always_on && !uc_pdata->boot_on)
		goto retdev;

	ret = regulator_set_enable(dev, true);
	if (setting_failed(ret, v, "; enabling", uc_pdata->min_uA))
		goto exit;

retdev:
	if (devp)
		*devp = dev;
exit:
	if (v)
		printf("\n");
	return ret;
}

int regulator_by_platname_list_autoset_and_enable(const char *list_platname[],
						  int list_entries,
						  struct udevice *list_devp[],
						  bool verbose)
{
	struct udevice *dev;
	int i, ret, success = 0;

	for (i = 0; i < list_entries; i++) {
		ret = regulator_autoset(list_platname[i], &dev, verbose);
		if (!ret)
			success++;

		if (!list_devp)
			continue;

		if (ret)
			list_devp[i] = NULL;
		else
			list_devp[i] = dev;
	}

	return (success != list_entries);
}

static int regulator_post_bind(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	int offset = dev->of_offset;
	const void *blob = gd->fdt_blob;

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata)
		return -ENXIO;

	/* Regulator's mandatory constraint */
	uc_pdata->name = fdt_getprop(blob, offset, "regulator-name", NULL);
	if (!uc_pdata->name) {
		debug("%s: dev: %s has no property 'regulator-name'\n",
		      __func__, dev->name);
		return -ENXIO;
	}

	return 0;
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
