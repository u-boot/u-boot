/*
 * DWC SATA platform driver
 *
 * (C) Copyright 2016
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * Author: Mugunthan V N <mugunthanvnm@ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <ahci.h>
#include <scsi.h>
#include <sata.h>
#include <asm/arch/sata.h>
#include <asm/io.h>
#include <generic-phy.h>

DECLARE_GLOBAL_DATA_PTR;

struct dwc_ahci_priv {
	void *base;
	void *wrapper_base;
};

static int dwc_ahci_ofdata_to_platdata(struct udevice *dev)
{
	struct dwc_ahci_priv *priv = dev_get_priv(dev);
	struct scsi_platdata *plat = dev_get_uclass_platdata(dev);
	fdt_addr_t addr;

	plat->max_id = fdtdec_get_uint(gd->fdt_blob, dev_of_offset(dev),
				       "max-id", CONFIG_SYS_SCSI_MAX_SCSI_ID);
	plat->max_lun = fdtdec_get_uint(gd->fdt_blob, dev_of_offset(dev),
					"max-lun", CONFIG_SYS_SCSI_MAX_LUN);

	priv->base = map_physmem(devfdt_get_addr(dev), sizeof(void *),
				 MAP_NOCACHE);

	addr = devfdt_get_addr_index(dev, 1);
	if (addr != FDT_ADDR_T_NONE) {
		priv->wrapper_base = map_physmem(addr, sizeof(void *),
						 MAP_NOCACHE);
	} else {
		priv->wrapper_base = NULL;
	}

	return 0;
}

static int dwc_ahci_probe(struct udevice *dev)
{
	struct dwc_ahci_priv *priv = dev_get_priv(dev);
	int ret;
	struct phy phy;

	ret = generic_phy_get_by_name(dev, "sata-phy", &phy);
	if (ret) {
		pr_err("can't get the phy from DT\n");
		return ret;
	}

	ret = generic_phy_init(&phy);
	if (ret) {
		pr_err("unable to initialize the sata phy\n");
		return ret;
	}

	ret = generic_phy_power_on(&phy);
	if (ret) {
		pr_err("unable to power on the sata phy\n");
		return ret;
	}

	if (priv->wrapper_base) {
		u32 val = TI_SATA_IDLE_NO | TI_SATA_STANDBY_NO;

		/* Enable SATA module, No Idle, No Standby */
		writel(val, priv->wrapper_base + TI_SATA_SYSCONFIG);
	}

	ret = ahci_init_dm(dev, priv->base);
	if (ret)
		return ret;

	return achi_start_ports_dm(dev);
}

static const struct udevice_id dwc_ahci_ids[] = {
	{ .compatible = "snps,dwc-ahci" },
	{ }
};

U_BOOT_DRIVER(dwc_ahci) = {
	.name	= "dwc_ahci",
	.id	= UCLASS_SCSI,
	.of_match = dwc_ahci_ids,
	.ofdata_to_platdata = dwc_ahci_ofdata_to_platdata,
	.ops	= &scsi_ops,
	.probe	= dwc_ahci_probe,
	.priv_auto_alloc_size = sizeof(struct dwc_ahci_priv),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
