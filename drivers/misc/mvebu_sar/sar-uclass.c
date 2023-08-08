/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0
 * https://spdx.org/licenses
 */

/* #define DEBUG */
#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <sar-uclass.h>

DECLARE_GLOBAL_DATA_PTR;

#define SAR_MAX_CHIP	4

UCLASS_DRIVER(sar) = {
	.name = "sar",
	.id = UCLASS_SAR,
};

struct udevice *__attribute__((section(".data")))soc_sar_info[SAR_MAX_IDX];

int mvebu_sar_id_register(struct udevice *dev, u32 sar_id)
{
	if (soc_sar_info[sar_id]) {
		pr_err("sar %d was already registered.\n", sar_id);
		return -EBUSY;
	}
	soc_sar_info[sar_id] = dev;

	return 0;
}

/* find all drivers for sar device and initialize each driver */
int mvebu_sar_init(void)
{
	int ret, i;
	int node, chip_count, sar_list[SAR_MAX_CHIP];
	const void *blob = gd->fdt_blob;
	struct udevice *sar_chip[SAR_MAX_CHIP];
	struct dm_sar_pdata *priv;
	const struct sar_ops *ops;
	struct udevice *parent;
	struct driver_info sar_drivers[SAR_MAX_CHIP];
	void *sar_base = NULL;
	const char *sar_driver, *sar_name;

	chip_count = fdtdec_find_aliases_for_id(blob, "sar-reg",
						COMPAT_MVEBU_SAR_REG_COMMON,
						sar_list, SAR_MAX_CHIP);

	if (chip_count <= 0) {
		pr_err("Cannot find sample-at-reset dt entry (%d).\n",
		       chip_count);
		return -ENODEV;
	}
	uclass_get_device_by_name(UCLASS_ROOT, "root_driver", &parent);
	memset(soc_sar_info, 0, sizeof(soc_sar_info));

	for (i = 0; i < chip_count ; i++) {
		node = sar_list[i];
		if (node <= 0)
			continue;

		/* Skip if Node is disabled */
		if (!fdtdec_get_is_enabled(blob, node))
			continue;
		/* Binding stage */
		sar_driver = fdt_getprop(blob, node, "sar-driver", NULL);
		sar_drivers[i].name = sar_driver;
		ret = device_bind_by_name(parent, false,
					  &sar_drivers[i], &sar_chip[i]);

		if (!sar_chip[i]) {
			pr_err("SAR driver binding failed\n");
			return 0;
		}

		/* fetch driver info from device-tree */
		sar_base = (void *)fdtdec_get_addr_size_auto_noparent(blob,
				node, "reg", 0, NULL, true);
		if (!sar_base) {
			pr_err("SAR address isn't found in the device-tree\n");
			return 0;
		}
		sar_name = fdt_getprop(blob, node, "sar-name", NULL);
		/* Initialize driver priv struct */
		device_probe(sar_chip[i]);
		priv = dev_get_priv(sar_chip[i]);
		priv->sar_base = sar_base;
		priv->sar_name = sar_name;
		ops = device_get_ops(sar_chip[i]);
		if (!ops->sar_init_func)
			return -EINVAL;

		ret = ops->sar_init_func(sar_chip[i]);
		if (ret) {
			pr_err("sar_init failed (%d).\n", ret);
			return ret;
		}
	}

	return 0;
}

int mvebu_sar_value_get(enum mvebu_sar_opts opt, struct sar_val *val)
{
	const struct sar_ops *ops;

	if (soc_sar_info[opt]) {
		ops = device_get_ops(soc_sar_info[opt]);
		return ops->sar_value_get_func(soc_sar_info[opt], opt, val);
	}

	pr_err("SAR - No chip registered on sar %d.\n", opt);
	return -ENODEV;
}

char *mvebu_sar_bootsrc_to_name(enum mvebu_bootsrc_type src)
{
	switch (src) {
	case(BOOTSRC_NAND):
		return "nand";
	case(BOOTSRC_SPI):
	case(BOOTSRC_AP_SPI):
		return "spi";
	case(BOOTSRC_SD_EMMC):
	case(BOOTSRC_AP_SD_EMMC):
		return "mmc";
	case(BOOTSRC_NOR):
		return "nor";
	default:
		return "unknown";
	}
}

void mvebu_sar_dump(struct udevice *dev)
{
	const struct sar_ops *ops;

	ops = device_get_ops(dev);

	ops->sar_dump_func(dev);
}
