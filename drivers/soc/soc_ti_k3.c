// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Texas Instruments Incorporated - http://www.ti.com/
 *	Dave Gerlach <d-gerlach@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <soc.h>

#include <asm/io.h>

#define AM65X			0xbb5a
#define J721E			0xbb64
#define J7200			0xbb6d
#define AM64X			0xbb38
#define J721S2			0xbb75

#define JTAG_ID_VARIANT_SHIFT	28
#define JTAG_ID_VARIANT_MASK	(0xf << 28)
#define JTAG_ID_PARTNO_SHIFT	12
#define JTAG_ID_PARTNO_MASK	(0xffff << 12)

struct soc_ti_k3_plat {
	const char *family;
	const char *revision;
};

static const char *get_family_string(u32 idreg)
{
	const char *family;
	u32 soc;

	soc = (idreg & JTAG_ID_PARTNO_MASK) >> JTAG_ID_PARTNO_SHIFT;

	switch (soc) {
	case AM65X:
		family = "AM65X";
		break;
	case J721E:
		family = "J721E";
		break;
	case J7200:
		family = "J7200";
		break;
	case AM64X:
		family = "AM64X";
		break;
	case J721S2:
		family = "J721S2";
		break;
	default:
		family = "Unknown Silicon";
	};

	return family;
}

static char *j721e_rev_string_map[] = {
	"1.0", "1.1",
};

static char *am65x_rev_string_map[] = {
	"1.0", "2.0",
};

static const char *get_rev_string(u32 idreg)
{
	u32 rev;
	u32 soc;

	rev = (idreg & JTAG_ID_VARIANT_MASK) >> JTAG_ID_VARIANT_SHIFT;
	soc = (idreg & JTAG_ID_PARTNO_MASK) >> JTAG_ID_PARTNO_SHIFT;

	switch (soc) {
	case J721E:
		if (rev > ARRAY_SIZE(j721e_rev_string_map))
			goto bail;
		return j721e_rev_string_map[rev];

	case AM65X:
		if (rev > ARRAY_SIZE(am65x_rev_string_map))
			goto bail;
		return am65x_rev_string_map[rev];

	case AM64X:
	case J7200:
	default:
		if (!rev)
			return "1.0";
	};

bail:
	return "Unknown Revision";
}

static int soc_ti_k3_get_family(struct udevice *dev, char *buf, int size)
{
	struct soc_ti_k3_plat *plat = dev_get_plat(dev);

	snprintf(buf, size, "%s", plat->family);

	return 0;
}

static int soc_ti_k3_get_revision(struct udevice *dev, char *buf, int size)
{
	struct soc_ti_k3_plat *plat = dev_get_plat(dev);

	snprintf(buf, size, "SR%s", plat->revision);

	return 0;
}

static const struct soc_ops soc_ti_k3_ops = {
	.get_family = soc_ti_k3_get_family,
	.get_revision = soc_ti_k3_get_revision,
};

int soc_ti_k3_probe(struct udevice *dev)
{
	struct soc_ti_k3_plat *plat = dev_get_plat(dev);
	u32 idreg;
	void *idreg_addr;

	idreg_addr = dev_read_addr_ptr(dev);
	if (!idreg_addr)
		return -EINVAL;

	idreg = readl(idreg_addr);

	plat->family = get_family_string(idreg);
	plat->revision = get_rev_string(idreg);

	return 0;
}

static const struct udevice_id soc_ti_k3_ids[] = {
	{ .compatible = "ti,am654-chipid" },
	{ }
};

U_BOOT_DRIVER(soc_ti_k3) = {
	.name           = "soc_ti_k3",
	.id             = UCLASS_SOC,
	.ops		= &soc_ti_k3_ops,
	.of_match       = soc_ti_k3_ids,
	.probe          = soc_ti_k3_probe,
	.plat_auto	= sizeof(struct soc_ti_k3_plat),
};
