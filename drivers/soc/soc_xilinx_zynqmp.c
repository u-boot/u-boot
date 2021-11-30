// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx ZynqMP SOC driver
 *
 * Copyright (C) 2021 Xilinx, Inc.
 */

#include <common.h>
#include <dm.h>
#include <asm/cache.h>
#include <soc.h>
#include <zynqmp_firmware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>

/*
 * Zynqmp has 4 silicon revisions
 * v0 -> 0(XCZU9EG-ES1)
 * v1 -> 1(XCZU3EG-ES1, XCZU15EG-ES1)
 * v2 -> 2(XCZU7EV-ES1, XCZU9EG-ES2, XCZU19EG-ES1)
 * v3 -> 3(Production Level)
 */
static const char zynqmp_family[] = "ZynqMP";

struct soc_xilinx_zynqmp_priv {
	const char *family;
	char revision;
};

static int soc_xilinx_zynqmp_get_family(struct udevice *dev, char *buf, int size)
{
	struct soc_xilinx_zynqmp_priv *priv = dev_get_priv(dev);

	return snprintf(buf, size, "%s", priv->family);
}

static int soc_xilinx_zynqmp_get_revision(struct udevice *dev, char *buf, int size)
{
	struct soc_xilinx_zynqmp_priv *priv = dev_get_priv(dev);

	return snprintf(buf, size, "v%d", priv->revision);
}

static const struct soc_ops soc_xilinx_zynqmp_ops = {
	.get_family = soc_xilinx_zynqmp_get_family,
	.get_revision = soc_xilinx_zynqmp_get_revision,
};

static int soc_xilinx_zynqmp_probe(struct udevice *dev)
{
	struct soc_xilinx_zynqmp_priv *priv = dev_get_priv(dev);
	u32 ret_payload[4];
	int ret;

	priv->family = zynqmp_family;

	if (IS_ENABLED(CONFIG_SPL_BUILD) || current_el() == 3 ||
	    !IS_ENABLED(CONFIG_ZYNQMP_FIRMWARE))
		ret = zynqmp_mmio_read(ZYNQMP_PS_VERSION, &ret_payload[2]);
	else
		ret = xilinx_pm_request(PM_GET_CHIPID, 0, 0, 0, 0,
					ret_payload);
	if (ret < 0)
		return ret;

	priv->revision = ret_payload[2] & ZYNQMP_PS_VER_MASK;

	return 0;
}

U_BOOT_DRIVER(soc_xilinx_zynqmp) = {
	.name		= "soc_xilinx_zynqmp",
	.id		= UCLASS_SOC,
	.ops		= &soc_xilinx_zynqmp_ops,
	.probe		= soc_xilinx_zynqmp_probe,
	.priv_auto	= sizeof(struct soc_xilinx_zynqmp_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};
