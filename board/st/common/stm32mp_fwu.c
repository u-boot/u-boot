// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2026 Amarula Solutions, Dario Binacchi <dario.binacchi@amarulasolutions.com>
 */

#include <fwu.h>
#include <part_efi.h>
#include <asm/io.h>
/**
 * fwu_plat_get_bootidx() - Get the value of the boot index
 * @boot_idx: Boot index value
 *
 * Get the value of the bank(partition) from which the platform
 * has booted. This value is passed to U-Boot from the earlier
 * stage bootloader which loads and boots all the relevant
 * firmware images
 *
 */
void fwu_plat_get_bootidx(uint *boot_idx)
{
	*boot_idx = (readl(TAMP_FWU_BOOT_INFO_REG) >>
		    TAMP_FWU_BOOT_IDX_OFFSET) & TAMP_FWU_BOOT_IDX_MASK;
}

int fwu_platform_hook(struct udevice *dev, struct fwu_data *data)
{
	uint boot_idx;
	efi_guid_t boot_uuid, root_uuid;
	const efi_guid_t boot_type_guid = PARTITION_XBOOTLDR;
	const efi_guid_t root_type_guid =
		PARTITION_LINUX_FILE_SYSTEM_DATA_GUID;
	char uuidbuf[UUID_STR_LEN + 1];
	int retb, retr;

	fwu_plat_get_bootidx(&boot_idx);

	retb = fwu_mdata_get_image_guid(&boot_uuid, &boot_type_guid, boot_idx);
	retr = fwu_mdata_get_image_guid(&root_uuid, &root_type_guid, boot_idx);

	if (!retb && !retr) {
		uuid_bin_to_str(boot_uuid.b, uuidbuf, UUID_STR_FORMAT_GUID);
		env_set("boot_partuuid", uuidbuf);

		uuid_bin_to_str(root_uuid.b, uuidbuf, UUID_STR_FORMAT_GUID);
		env_set("root_partuuid", uuidbuf);
	} else if (!retb && retr) {
		log_warning("%s: found boot GUID but missing root GUID (%d)\n",
			    __func__, retr);
	} else if (!retr && retb) {
		log_warning("%s: found root GUID but missing boot GUID (%d)\n",
			    __func__, retb);
	}

	return 0;
}
