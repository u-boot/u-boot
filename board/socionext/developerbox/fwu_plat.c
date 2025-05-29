// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023, Linaro Limited
 */

#include <efi_loader.h>
#include <env.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <memalign.h>
#include <mtd.h>

#define DFU_ALT_BUF_LEN 256

/* Generate dfu_alt_info from partitions */
void set_dfu_alt_info(char *interface, char *devstr)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);
	struct mtd_info *mtd;
	int ret;

	memset(buf, 0, DFU_ALT_BUF_LEN);

	mtd_probe_devices();

	mtd = get_mtd_device_nm("nor1");
	if (IS_ERR_OR_NULL(mtd))
		return;

	ret = fwu_gen_alt_info_from_mtd(buf, DFU_ALT_BUF_LEN, mtd);
	if (ret < 0) {
		log_err("Error: Failed to generate dfu_alt_info. (%d)\n", ret);
		return;
	}
	log_debug("Make dfu_alt_info: '%s'\n", buf);

	env_set("dfu_alt_info", buf);
}

/**
 * fwu_plat_get_bootidx() - Get the value of the boot index
 * @boot_idx: Boot index value
 *
 * Get the value of the bank(partition) from which the platform
 * has booted. This value is passed to U-Boot from the earlier
 * stage bootloader which loads and boots all the relevant
 * firmware images
 */
void fwu_plat_get_bootidx(uint *boot_idx)
{
	int ret;
	u32 buf;
	size_t readlen;
	struct mtd_info *mtd;

	*boot_idx = 0;

	mtd_probe_devices();
	mtd = get_mtd_device_nm("nor1");
	if (IS_ERR_OR_NULL(mtd))
		return;

	ret = mtd_read(mtd, SCB_PLAT_METADATA_OFFSET, sizeof(buf),
		       &readlen, (u_char *)&buf);
	if (ret < 0)
		return;

	*boot_idx = buf;
}
