// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 * Copyright 2019 Google Inc
 */

#include <common.h>
#include <dm.h>
#include "designware_i2c.h"

/* BayTrail HCNT/LCNT/SDA hold time */
static struct dw_scl_sda_cfg byt_config = {
	.ss_hcnt = 0x200,
	.fs_hcnt = 0x55,
	.ss_lcnt = 0x200,
	.fs_lcnt = 0x99,
	.sda_hold = 0x6,
};

static int designware_i2c_pci_probe(struct udevice *dev)
{
	struct dw_i2c *priv = dev_get_priv(dev);

	/* Save base address from PCI BAR */
	priv->regs = (struct i2c_regs *)
		dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, PCI_REGION_MEM);
	if (IS_ENABLED(CONFIG_INTEL_BAYTRAIL))
		/* Use BayTrail specific timing values */
		priv->scl_sda_cfg = &byt_config;

	return designware_i2c_probe(dev);
}

static int designware_i2c_pci_bind(struct udevice *dev)
{
	char name[20];

	/*
	 * Create a unique device name for PCI type devices
	 * ToDo:
	 * Setting req_seq in the driver is probably not recommended.
	 * But without a DT alias the number is not configured. And
	 * using this driver is impossible for PCIe I2C devices.
	 * This can be removed, once a better (correct) way for this
	 * is found and implemented.
	 *
	 * TODO(sjg@chromium.org): Perhaps if uclasses had platdata this would
	 * be possible. We cannot use static data in drivers since they may be
	 * used in SPL or before relocation.
	 */
	dev->req_seq = gd->arch.dw_i2c_num_cards++;
	sprintf(name, "i2c_designware#%u", dev->req_seq);
	device_set_name(dev, name);

	return 0;
}

U_BOOT_DRIVER(i2c_designware_pci) = {
	.name	= "i2c_designware_pci",
	.id	= UCLASS_I2C,
	.bind	= designware_i2c_pci_bind,
	.probe	= designware_i2c_pci_probe,
	.priv_auto_alloc_size = sizeof(struct dw_i2c),
	.remove = designware_i2c_remove,
	.flags = DM_FLAG_OS_PREPARE,
	.ops	= &designware_i2c_ops,
};

static struct pci_device_id designware_pci_supported[] = {
	/* Intel BayTrail has 7 I2C controller located on the PCI bus */
	{ PCI_VDEVICE(INTEL, 0x0f41) },
	{ PCI_VDEVICE(INTEL, 0x0f42) },
	{ PCI_VDEVICE(INTEL, 0x0f43) },
	{ PCI_VDEVICE(INTEL, 0x0f44) },
	{ PCI_VDEVICE(INTEL, 0x0f45) },
	{ PCI_VDEVICE(INTEL, 0x0f46) },
	{ PCI_VDEVICE(INTEL, 0x0f47) },
	{},
};

U_BOOT_PCI_DEVICE(i2c_designware_pci, designware_pci_supported);
