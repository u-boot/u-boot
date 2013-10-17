/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <malloc.h>

/*
 * The environment variables are written to just after the u-boot image
 * on SDCard, so we must read the MBR to get the start address and code
 * length of the u-boot image, then calculate the address of the env.
 */
#define ESDHC_BOOT_IMAGE_SIZE	0x48
#define ESDHC_BOOT_IMAGE_ADDR	0x50

int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
	u8 *tmp_buf;
	u32 blklen, code_offset, code_len, n;

	blklen = mmc->read_bl_len;
	tmp_buf = malloc(blklen);
	if (!tmp_buf)
		return 1;

	/* read out the first block, get the config data information */
	n = mmc->block_dev.block_read(mmc->block_dev.dev, 0, 1, tmp_buf);
	if (!n) {
		free(tmp_buf);
		return 1;
	}

	/* Get the Source Address, from offset 0x50 */
	code_offset = *(u32 *)(tmp_buf + ESDHC_BOOT_IMAGE_ADDR);

	/* Get the code size from offset 0x48 */
	code_len = *(u32 *)(tmp_buf + ESDHC_BOOT_IMAGE_SIZE);

	*env_addr = code_offset + code_len;

	free(tmp_buf);

	return 0;
}
