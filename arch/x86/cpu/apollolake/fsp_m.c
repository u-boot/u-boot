// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/iomap.h>
#include <asm/arch/fsp_bindings.h>
#include <asm/fsp2/fsp_internal.h>
#include <dm/uclass-internal.h>

int fspm_update_config(struct udevice *dev, struct fspm_upd *upd)
{
	struct fsp_m_config *cfg = &upd->config;
	struct fspm_arch_upd *arch = &upd->arch;
	ofnode node;

	arch->nvs_buffer_ptr = NULL;
	prepare_mrc_cache(upd);
	arch->stack_base = (void *)0xfef96000;
	arch->boot_loader_tolum_size = 0;
	arch->boot_mode = FSP_BOOT_WITH_FULL_CONFIGURATION;

	node = dev_ofnode(dev);
	if (!ofnode_valid(node))
		return log_msg_ret("fsp-m settings", -ENOENT);

	return fsp_m_update_config_from_dtb(node, cfg);
}

/*
 * The FSP-M binary appears to break the SPI controller. It can be fixed by
 * writing the BAR again, so do that here
 */
int fspm_done(struct udevice *dev)
{
	struct udevice *spi;
	int ret;

	/* Don't probe the device, since that reads the BAR */
	ret = uclass_find_first_device(UCLASS_SPI, &spi);
	if (ret)
		return log_msg_ret("SPI", ret);
	if (!spi)
		return log_msg_ret("no SPI", -ENODEV);

	dm_pci_write_config32(spi, PCI_BASE_ADDRESS_0,
			      IOMAP_SPI_BASE | PCI_BASE_ADDRESS_SPACE_MEMORY);

	return 0;
}
