// SPDX-License-Identifier: GPL-2.0
/*
 * AMD Versal Gen 2 SOC driver
 *
 * Copyright (C) 2022 - 2024, Advanced Micro Devices, Inc.
 */

#include <dm.h>
#include <soc.h>
#include <zynqmp_firmware.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

#include <linux/bitfield.h>

/*
 * v1 -> 0x10 - ES1
 * v2 -> 0x20 - Production
 */
static const char versal2_family[] = "Versal Gen 2";

struct soc_amd_versal2_priv {
	const char *family;
	char revision;
};

static int soc_amd_versal2_get_family(struct udevice *dev, char *buf, int size)
{
	struct soc_amd_versal2_priv *priv = dev_get_priv(dev);

	return snprintf(buf, size, "%s", priv->family);
}

static int soc_amd_versal2_get_revision(struct udevice *dev, char *buf, int size)
{
	struct soc_amd_versal2_priv *priv = dev_get_priv(dev);

	return snprintf(buf, size, "v%d.%d",
			(u32)FIELD_GET(PS_VERSION_MAJOR, priv->revision),
			(u32)FIELD_GET(PS_VERSION_MINOR, priv->revision));
}

static const struct soc_ops soc_amd_versal2_ops = {
	.get_family = soc_amd_versal2_get_family,
	.get_revision = soc_amd_versal2_get_revision,
};

static int soc_amd_versal2_probe(struct udevice *dev)
{
	struct soc_amd_versal2_priv *priv = dev_get_priv(dev);
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	priv->family = versal2_family;

	if (IS_ENABLED(CONFIG_ZYNQMP_FIRMWARE)) {
		ret = xilinx_pm_request(PM_GET_CHIPID, 0, 0, 0, 0,
					ret_payload);
		if (ret)
			return ret;
	} else {
		ret_payload[2] = readl(PMC_TAP_VERSION);
		if (!ret_payload[2])
			return -EINVAL;
	}

	priv->revision = FIELD_GET(PS_VERSION_MASK, ret_payload[2]);

	return 0;
}

U_BOOT_DRIVER(soc_amd_versal2) = {
	.name		= "soc_amd_versal2",
	.id		= UCLASS_SOC,
	.ops		= &soc_amd_versal2_ops,
	.probe		= soc_amd_versal2_probe,
	.priv_auto	= sizeof(struct soc_amd_versal2_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};
