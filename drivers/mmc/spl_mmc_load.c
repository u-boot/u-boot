/*
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

static void mmc_load_image(struct mmc *mmc)
{
	s32 err;
	void (*uboot)(void) __noreturn;

	err = mmc->block_dev.block_read(0, CONFIG_SYS_MMC_U_BOOT_OFFS,
			CONFIG_SYS_MMC_U_BOOT_SIZE/512,
			(u32 *)CONFIG_SYS_TEXT_BASE);

	if (err <= 0) {
		printf("spl: error reading image %s, err - %d\n",
			"u-boot.img", err);
		hang();
	}
	uboot = (void *) CONFIG_SYS_TEXT_BASE;
	(*uboot)();
}

void spl_mmc_load(void)
{
	struct mmc *mmc;
	int err;
	void (mmc_load_image)(struct mmc *mmc) __noreturn;

	mmc_initialize(gd->bd);
	mmc = find_mmc_device(0);
	if (!mmc) {
		puts("spl: mmc device not found!!\n");
		hang();
	} else {
		puts("spl: mmc device found\n");
	}
	err = mmc_init(mmc);
	if (err) {
		printf("spl: mmc init failed: err - %d\n", err);
		hang();
	}
	mmc_load_image(mmc);
}
