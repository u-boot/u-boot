// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

/* #define DEBUG */
#include <common.h>
#include <malloc.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch-mvebu/thermal.h>
#include <dm.h>
#include <thermal.h>

DECLARE_GLOBAL_DATA_PTR;

static struct thermal_unit_config *init_thermal_config(struct udevice *dev)
{
	struct thermal_unit_config *thermal_cfg = dev_get_priv(dev);
	void *blob = (void *)gd->fdt_blob;
	int node = dev_of_offset(dev);

	/* Read register base */
	thermal_cfg->regs_base = (void *)devfdt_get_addr_index(dev, 0);

	if (thermal_cfg->regs_base == 0) {
		pr_err("thermal: base address is missing in device-tree\n");
		return NULL;
	}

	/* Register functions (according to compatible) */
	if (device_is_compatible(dev, "marvell,thermal-sensor")) {
		thermal_cfg->ptr_thermal_sensor_probe =
			mvebu_thermal_sensor_probe;
		thermal_cfg->ptr_thermal_sensor_read =
			mvebu_thermal_sensor_read;
	}

	if (device_is_compatible(dev, "marvell,thermal-ext-sensor")) {
		thermal_cfg->ptr_thermal_sensor_probe =
			mvebu_thermal_ext_sensor_probe;
		thermal_cfg->ptr_thermal_sensor_read =
			mvebu_thermal_ext_sensor_read;
	}

	if (!thermal_cfg->ptr_thermal_sensor_probe ||
	    !thermal_cfg->ptr_thermal_sensor_read) {
		pr_err("thermal.%lx: compatible is not supported\n",
		       (uintptr_t)thermal_cfg->regs_base);
		return NULL;
	}

	/* Read temperature calculation parameters */
	thermal_cfg->tsen_gain = fdtdec_get_int(blob, node, "gain", 0);
	if (thermal_cfg->tsen_gain <= 0) {
		pr_err("thermal%lx: gain is missing in device-tree\n",
		       (uintptr_t)thermal_cfg->regs_base);
		return NULL;
	}

	thermal_cfg->tsen_offset = fdtdec_get_int(blob, node, "offset", 0);
	if (thermal_cfg->tsen_offset <= 0) {
		pr_err("thermal%lx: offset is missing in device-tree\n",
		       (uintptr_t)thermal_cfg->regs_base);
		return NULL;
	}

	thermal_cfg->tsen_divisor = fdtdec_get_int(blob, node, "divisor", 0);
	if (thermal_cfg->tsen_divisor <= 0) {
		pr_err("thermal%lx: divisor is missing in device-tree\n",
		       (uintptr_t)thermal_cfg->regs_base);
		return NULL;
	}

	return thermal_cfg;
}

int mvebu_thermal_read(struct udevice *dev, int *temp)
{
	struct thermal_unit_config *thermal_cfg;

	if (!temp) {
		pr_err("NULL pointer for temperature read\n");
		return -1;
	}

	thermal_cfg = dev_get_priv(dev);

	if (thermal_cfg->ptr_thermal_sensor_read == 0 ||
	    thermal_cfg->tsen_ready == 0) {
		debug("Thermal unit was not initialized\n");
		return -1;
	}

	return thermal_cfg->ptr_thermal_sensor_read(thermal_cfg, temp);
}

static const struct dm_thermal_ops mvebu_thermal_ops = {
	.get_temp	= mvebu_thermal_read,
};

static int mvebu_thermal_probe(struct udevice *dev)
{
	struct thermal_unit_config *thermal_cfg = dev_get_priv(dev);

	/* Init Sensor data structure */
	thermal_cfg = init_thermal_config(dev);
	if (!thermal_cfg) {
		pr_err("Thermal: failed to initialize thermal data structure\n");
		return -1;
	}

	/* set flag to indicate that Thermal Sensor is not ready */
	thermal_cfg->tsen_ready = 0;

	/* Sensor init */
	if (thermal_cfg->ptr_thermal_sensor_probe(thermal_cfg)) {
		pr_err("thermal.%lx: failed to initialize thermal info\n",
		       (uintptr_t)thermal_cfg->regs_base);
		return -1; /* initialization failed */
	}

	/* Thermal Sensor is ready to use */
	thermal_cfg->tsen_ready = 1;

	debug("thermal.%lx: Initialized\n", (uintptr_t)thermal_cfg->regs_base);

	return 0;
}

static const struct udevice_id mvebu_thermal_ids[] = {
	{ .compatible = "marvell,mvebu-thermal" },
	{ }
};

U_BOOT_DRIVER(mvebu_thermal) = {
	.name	= "mvebu-thermal",
	.id	= UCLASS_THERMAL,
	.ops	= &mvebu_thermal_ops,
	.of_match = mvebu_thermal_ids,
	.probe	= mvebu_thermal_probe,
	.priv_auto_alloc_size = sizeof(struct thermal_unit_config),
};
