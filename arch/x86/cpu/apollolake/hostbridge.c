// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
#include <spl.h>
#include <asm/intel_pinctrl.h>
#include <asm/intel_regs.h>
#include <asm/pci.h>
#include <asm/arch/systemagent.h>

/**
 * struct apl_hostbridge_platdata - platform data for hostbridge
 *
 * @dtplat: Platform data for of-platdata
 * @early_pads: Early pad data to set up, each (pad, cfg0, cfg1)
 * @early_pads_count: Number of pads to process
 * @pciex_region_size: BAR length in bytes
 * @bdf: Bus/device/function of hostbridge
 */
struct apl_hostbridge_platdata {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_intel_apl_hostbridge dtplat;
#endif
	u32 *early_pads;
	int early_pads_count;
	uint pciex_region_size;
	pci_dev_t bdf;
};

enum {
	PCIEXBAR		= 0x60,
	PCIEXBAR_LENGTH_256MB	= 0,
	PCIEXBAR_LENGTH_128MB,
	PCIEXBAR_LENGTH_64MB,

	PCIEXBAR_PCIEXBAREN	= 1 << 0,

	TSEG			= 0xb8,  /* TSEG base */
};

static int apl_hostbridge_early_init_pinctrl(struct udevice *dev)
{
	struct apl_hostbridge_platdata *plat = dev_get_platdata(dev);
	struct udevice *pinctrl;
	int ret;

	ret = uclass_first_device_err(UCLASS_PINCTRL, &pinctrl);
	if (ret)
		return log_msg_ret("no hostbridge pinctrl", ret);

	return pinctrl_config_pads(pinctrl, plat->early_pads,
				   plat->early_pads_count);
}

static int apl_hostbridge_early_init(struct udevice *dev)
{
	struct apl_hostbridge_platdata *plat = dev_get_platdata(dev);
	u32 region_size;
	ulong base;
	u32 reg;
	int ret;

	/* Set up the MCHBAR */
	pci_x86_read_config(plat->bdf, MCHBAR, &base, PCI_SIZE_32);
	base = MCH_BASE_ADDRESS;
	pci_x86_write_config(plat->bdf, MCHBAR, base | 1, PCI_SIZE_32);

	/*
	 * The PCIEXBAR is assumed to live in the memory mapped IO space under
	 * 4GiB
	 */
	pci_x86_write_config(plat->bdf, PCIEXBAR + 4, 0, PCI_SIZE_32);

	switch (plat->pciex_region_size >> 20) {
	default:
	case 256:
		region_size = PCIEXBAR_LENGTH_256MB;
		break;
	case 128:
		region_size = PCIEXBAR_LENGTH_128MB;
		break;
	case 64:
		region_size = PCIEXBAR_LENGTH_64MB;
		break;
	}

	reg = CONFIG_MMCONF_BASE_ADDRESS | (region_size << 1)
				| PCIEXBAR_PCIEXBAREN;
	pci_x86_write_config(plat->bdf, PCIEXBAR, reg, PCI_SIZE_32);

	/*
	 * TSEG defines the base of SMM range. BIOS determines the base
	 * of TSEG memory which must be at or below Graphics base of GTT
	 * Stolen memory, hence its better to clear TSEG register early
	 * to avoid power on default non-zero value (if any).
	 */
	pci_x86_write_config(plat->bdf, TSEG, 0, PCI_SIZE_32);

	ret = apl_hostbridge_early_init_pinctrl(dev);
	if (ret)
		return log_msg_ret("pinctrl", ret);

	return 0;
}

static int apl_hostbridge_ofdata_to_platdata(struct udevice *dev)
{
	struct apl_hostbridge_platdata *plat = dev_get_platdata(dev);
	struct udevice *pinctrl;
	int ret;

	/*
	 * The host bridge holds the early pad data needed to get through TPL.
	 * This is a small amount of data, enough to fit in TPL, so we keep it
	 * separate from the full pad data, stored in the fsp-s subnode. That
	 * subnode is not present in TPL, to save space.
	 */
	ret = uclass_first_device_err(UCLASS_PINCTRL, &pinctrl);
	if (ret)
		return log_msg_ret("no hostbridge PINCTRL", ret);
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	int root;

	/* Get length of PCI Express Region */
	plat->pciex_region_size = dev_read_u32_default(dev, "pciex-region-size",
						       256 << 20);

	root = pci_get_devfn(dev);
	if (root < 0)
		return log_msg_ret("Cannot get host-bridge PCI address", root);
	plat->bdf = root;

	ret = pinctrl_read_pads(pinctrl, dev_ofnode(dev), "early-pads",
				&plat->early_pads, &plat->early_pads_count);
	if (ret)
		return log_msg_ret("early-pads", ret);
#else
	struct dtd_intel_apl_hostbridge *dtplat = &plat->dtplat;
	int size;

	plat->pciex_region_size = dtplat->pciex_region_size;
	plat->bdf = pci_ofplat_get_devfn(dtplat->reg[0]);

	/* Assume that if everything is 0, it is empty */
	plat->early_pads = dtplat->early_pads;
	size = ARRAY_SIZE(dtplat->early_pads);
	plat->early_pads_count = pinctrl_count_pads(pinctrl, plat->early_pads,
						    size);

#endif

	return 0;
}

static int apl_hostbridge_probe(struct udevice *dev)
{
	if (spl_phase() == PHASE_TPL)
		return apl_hostbridge_early_init(dev);

	return 0;
}

static const struct udevice_id apl_hostbridge_ids[] = {
	{ .compatible = "intel,apl-hostbridge" },
	{ }
};

U_BOOT_DRIVER(apl_hostbridge_drv) = {
	.name		= "intel_apl_hostbridge",
	.id		= UCLASS_NORTHBRIDGE,
	.of_match	= apl_hostbridge_ids,
	.ofdata_to_platdata = apl_hostbridge_ofdata_to_platdata,
	.probe		= apl_hostbridge_probe,
	.platdata_auto_alloc_size = sizeof(struct apl_hostbridge_platdata),
};
