// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <acpi_s3.h>
#include <dm.h>
#include <errno.h>
#include <rtc.h>
#include <asm/cmos_layout.h>
#include <asm/early_cmos.h>
#include <asm/io.h>
#include <asm/mrccache.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/fsp/fsp_support.h>

DECLARE_GLOBAL_DATA_PTR;

int checkcpu(void)
{
	return 0;
}

int print_cpuinfo(void)
{
	post_code(POST_CPU_INFO);
	return default_print_cpuinfo();
}

int fsp_init_phase_pci(void)
{
	u32 status;

	/* call into FspNotify */
	debug("Calling into FSP (notify phase INIT_PHASE_PCI): ");
	status = fsp_notify(NULL, INIT_PHASE_PCI);
	if (status)
		debug("fail, error code %x\n", status);
	else
		debug("OK\n");

	return status ? -EPERM : 0;
}

void board_final_cleanup(void)
{
	u32 status;

	/* call into FspNotify */
	debug("Calling into FSP (notify phase INIT_PHASE_BOOT): ");
	status = fsp_notify(NULL, INIT_PHASE_BOOT);
	if (status)
		debug("fail, error code %x\n", status);
	else
		debug("OK\n");
}

void *fsp_prepare_mrc_cache(void)
{
	struct mrc_data_container *cache;
	struct mrc_region entry;
	int ret;

	ret = mrccache_get_region(NULL, &entry);
	if (ret)
		return NULL;

	cache = mrccache_find_current(&entry);
	if (!cache)
		return NULL;

	debug("%s: mrc cache at %p, size %x checksum %04x\n", __func__,
	      cache->data, cache->data_size, cache->checksum);

	return cache->data;
}

#ifdef CONFIG_HAVE_ACPI_RESUME
int fsp_save_s3_stack(void)
{
	struct udevice *dev;
	int ret;

	if (gd->arch.prev_sleep_state == ACPI_S3)
		return 0;

	ret = uclass_get_device(UCLASS_RTC, 0, &dev);
	if (ret) {
		debug("Cannot find RTC: err=%d\n", ret);
		return -ENODEV;
	}

	/* Save the stack address to CMOS */
	ret = rtc_write32(dev, CMOS_FSP_STACK_ADDR, gd->start_addr_sp);
	if (ret) {
		debug("Save stack address to CMOS: err=%d\n", ret);
		return -EIO;
	}

	return 0;
}
#endif
