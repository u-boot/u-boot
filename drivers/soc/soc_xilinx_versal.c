// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx Versal SOC driver
 *
 * Copyright (C) 2021 Xilinx, Inc.
 */

#include <dm.h>
#include <soc.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

/*
 * v1 -> 0x10 - ES1
 * v2 -> 0x20 - Production
 */
static const char versal_family[] = "Versal";

struct soc_xilinx_versal_priv {
	const char *family;
	char revision;
};

static int soc_xilinx_versal_get_family(struct udevice *dev, char *buf, int size)
{
	struct soc_xilinx_versal_priv *priv = dev_get_priv(dev);

	return snprintf(buf, size, "%s", priv->family);
}

static int soc_xilinx_versal_get_revision(struct udevice *dev, char *buf, int size)
{
	struct soc_xilinx_versal_priv *priv = dev_get_priv(dev);

	return snprintf(buf, size, "v%d", priv->revision);
}

static const struct soc_ops soc_xilinx_versal_ops = {
	.get_family = soc_xilinx_versal_get_family,
	.get_revision = soc_xilinx_versal_get_revision,
};

static int soc_xilinx_versal_probe(struct udevice *dev)
{
	struct soc_xilinx_versal_priv *priv = dev_get_priv(dev);
	u32 version;
	int ret;

	ret = xilinx_pm_get_chipid(NULL, &version);
	if (ret)
		return ret;

	priv->family = versal_family;
	priv->revision = version >> VERSAL_PS_VER_SHIFT;

	return 0;
}

U_BOOT_DRIVER(soc_xilinx_versal) = {
	.name		= "soc_xilinx_versal",
	.id		= UCLASS_SOC,
	.ops		= &soc_xilinx_versal_ops,
	.probe		= soc_xilinx_versal_probe,
	.priv_auto	= sizeof(struct soc_xilinx_versal_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};
