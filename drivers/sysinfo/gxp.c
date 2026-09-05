// SPDX-License-Identifier: GPL-2.0+
/*
 * HPE GXP sysinfo driver
 *
 * Copyright (C) 2026 9elements GmbH
 */

#include <dm.h>
#include <nvmem.h>
#include <sysinfo.h>
#include <dm/device_compat.h>

#define GXP_SYSINFO_MODEL		"GXP"
#define GXP_SYSINFO_MANUFACTURER	"Hewlett Packard Enterprise"

/**
 * struct gxp_sysinfo_priv - GXP sysinfo private data
 * @serial_cell: NVMEM cell for system serial number
 * @part_cell: NVMEM cell for system part number
 * @pca_serial_cell: NVMEM cell for baseboard (PCA) serial number
 * @pca_part_cell: NVMEM cell for baseboard (PCA) part number
 */
struct gxp_sysinfo_priv {
	struct nvmem_cell serial_cell;
	struct nvmem_cell part_cell;
	struct nvmem_cell pca_serial_cell;
	struct nvmem_cell pca_part_cell;
};

static int gxp_sysinfo_detect(struct udevice *dev)
{
	return 0;
}

static int gxp_sysinfo_get_str(struct udevice *dev, int id, size_t size,
			       char *val)
{
	struct gxp_sysinfo_priv *priv = dev_get_priv(dev);
	struct nvmem_cell *cell;
	int ret;

	switch (id) {
	case SYSID_BOARD_MODEL:
		strlcpy(val, GXP_SYSINFO_MODEL, size);
		return 0;
	case SYSID_BOARD_MANUFACTURER:
		strlcpy(val, GXP_SYSINFO_MANUFACTURER, size);
		return 0;
	case SYSID_SM_SYSTEM_SERIAL:
		if (!priv->serial_cell.nvmem)
			return -ENODEV;
		cell = &priv->serial_cell;
		break;
	case SYSID_SM_SYSTEM_PRODUCT:
		if (!priv->part_cell.nvmem)
			return -ENODEV;
		cell = &priv->part_cell;
		break;
	case SYSID_SM_BASEBOARD_SERIAL:
		if (!priv->pca_serial_cell.nvmem)
			return -ENODEV;
		cell = &priv->pca_serial_cell;
		break;
	case SYSID_SM_BASEBOARD_PRODUCT:
		if (!priv->pca_part_cell.nvmem)
			return -ENODEV;
		cell = &priv->pca_part_cell;
		break;
	default:
		return -EINVAL;
	}

	if (size <= cell->size)
		return -ENOSPC;

	ret = nvmem_cell_read(cell, val, cell->size);
	if (ret)
		return ret;

	val[cell->size] = '\0';

	return 0;
}

static int gxp_sysinfo_probe(struct udevice *dev)
{
	struct gxp_sysinfo_priv *priv = dev_get_priv(dev);

	nvmem_cell_get_by_name(dev, "serial-number", &priv->serial_cell);
	nvmem_cell_get_by_name(dev, "part-number", &priv->part_cell);
	nvmem_cell_get_by_name(dev, "pca-serial-number", &priv->pca_serial_cell);
	nvmem_cell_get_by_name(dev, "pca-part-number", &priv->pca_part_cell);

	return 0;
}

static const struct sysinfo_ops gxp_sysinfo_ops = {
	.detect = gxp_sysinfo_detect,
	.get_str = gxp_sysinfo_get_str,
};

static const struct udevice_id gxp_sysinfo_ids[] = {
	{ .compatible = "hpe,gxp-sysinfo" },
	{ }
};

U_BOOT_DRIVER(gxp_sysinfo) = {
	.name		= "gxp_sysinfo",
	.id		= UCLASS_SYSINFO,
	.of_match	= gxp_sysinfo_ids,
	.probe		= gxp_sysinfo_probe,
	.ops		= &gxp_sysinfo_ops,
	.priv_auto	= sizeof(struct gxp_sysinfo_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};
