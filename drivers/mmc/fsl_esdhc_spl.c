// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <mmc.h>
#include <malloc.h>

/*
 * The environment variables are written to just after the u-boot image
 * on SDCard, so we must read the MBR to get the start address and code
 * length of the u-boot image, then calculate the address of the env.
 */
#define ESDHC_BOOT_SIGNATURE_OFF 0x40
#define ESDHC_BOOT_SIGNATURE	0x424f4f54
#define ESDHC_BOOT_IMAGE_SIZE	0x48
#define ESDHC_BOOT_IMAGE_ADDR	0x50
#define MBRDBR_BOOT_SIG_55	0x1fe
#define MBRDBR_BOOT_SIG_AA	0x1ff


void mmc_spl_load_image(uint32_t offs, unsigned int size, void *vdst)
{
	uint blk_start, blk_cnt, err;

	struct mmc *mmc = find_mmc_device(0);
	if (!mmc) {
		puts("spl: mmc device not found!!\n");
		hang();
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return;
	}

	blk_start = ALIGN(offs, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

	err = mmc->block_dev.block_read(&mmc->block_dev, blk_start, blk_cnt,
					vdst);
	if (err != blk_cnt) {
		puts("spl: mmc read failed!!\n");
		hang();
	}
}

/*
 * The main entry for mmc booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from mmc into SDRAM and starts it from there.
 */

void __noreturn mmc_boot(void)
{
	__attribute__((noreturn)) void (*uboot)(void);
	uint blk_start, blk_cnt, err;
#ifndef CONFIG_FSL_CORENET
	uchar *tmp_buf;
	u32 blklen;
	u32 blk_off;
	uchar val;
#ifndef CONFIG_SPL_FSL_PBL
	u32 val32;
#endif
	uint i, byte_num;
	u32 sector;
#endif
	u32 offset, code_len;
	struct mmc *mmc;

	mmc = find_mmc_device(0);
	if (!mmc) {
		puts("spl: mmc device not found!!\n");
		hang();
	}

	if (mmc_init(mmc)) {
		puts("spl: mmc device init failed!\n");
		hang();
	}

#ifdef CONFIG_FSL_CORENET
	offset = CONFIG_SYS_MMC_U_BOOT_OFFS;
#else
	blklen = mmc->read_bl_len;
	if (blklen < 512)
		blklen = 512;
	tmp_buf = malloc(blklen);
	if (!tmp_buf) {
		puts("spl: malloc memory failed!!\n");
		hang();
	}

	sector = 0;
again:
	memset(tmp_buf, 0, blklen);

	/*
	* Read source addr from sd card
	*/
	blk_start = (sector * 512) / mmc->read_bl_len;
	blk_off = (sector * 512) % mmc->read_bl_len;
	blk_cnt = DIV_ROUND_UP(512,  mmc->read_bl_len);
	err = mmc->block_dev.block_read(&mmc->block_dev, blk_start, blk_cnt, tmp_buf);
	if (err != blk_cnt) {
		puts("spl: mmc read failed!!\n");
		hang();
	}

#ifdef CONFIG_SPL_FSL_PBL
	val = *(tmp_buf + blk_off + MBRDBR_BOOT_SIG_55);
	if (0x55 != val) {
		puts("spl: mmc MBR/DBR signature is not valid!!\n");
		hang();
	}
	val = *(tmp_buf + blk_off + MBRDBR_BOOT_SIG_AA);
	if (0xAA != val) {
		puts("spl: mmc MBR/DBR signature is not valid!!\n");
		hang();
	}
#else
	/*
	 * Booting from On-Chip ROM (eSDHC or eSPI), Document Number: AN3659, Rev. 2, 06/2012.
	 * Pre-PBL BootROMs (MPC8536E, MPC8569E, P2020, P1011, P1012, P1013, P1020, P1021, P1022)
	 * require custom BOOT signature on sector 0 and MBR/DBR signature is not required at all.
	 */
	byte_num = 4;
	val32 = 0;
	for (i = 0; i < byte_num; i++) {
		val = *(tmp_buf + blk_off + ESDHC_BOOT_SIGNATURE_OFF + i);
		val32 = (val32 << 8) + val;
	}
	if (val32 != ESDHC_BOOT_SIGNATURE) {
		/* BOOT signature may be on the first 24 sectors (each being 512 bytes) */
		if (++sector < 24)
			goto again;
		puts("spl: mmc BOOT signature is not valid!!\n");
		hang();
	}
#endif

	byte_num = 4;
	offset = 0;
	for (i = 0; i < byte_num; i++) {
		val = *(tmp_buf + blk_off + ESDHC_BOOT_IMAGE_ADDR + i);
		offset = (offset << 8) + val;
	}
	offset += CONFIG_SYS_MMC_U_BOOT_OFFS;
#endif
	/*
	* Load U-Boot image from mmc into RAM
	*/
	code_len = CONFIG_SYS_MMC_U_BOOT_SIZE;
	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt = ALIGN(code_len, mmc->read_bl_len) / mmc->read_bl_len;
	err = mmc->block_dev.block_read(&mmc->block_dev, blk_start, blk_cnt,
					(uchar *)CONFIG_SYS_MMC_U_BOOT_DST);
	if (err != blk_cnt) {
		puts("spl: mmc read failed!!\n");
#ifndef CONFIG_FSL_CORENET
		free(tmp_buf);
#endif
		hang();
	}

	/*
	* Clean d-cache and invalidate i-cache, to
	* make sure that no stale data is executed.
	*/
	flush_cache(CONFIG_SYS_MMC_U_BOOT_DST, CONFIG_SYS_MMC_U_BOOT_SIZE);

	/*
	* Jump to U-Boot image
	*/
	uboot = (void *)CONFIG_SYS_MMC_U_BOOT_START;
	(*uboot)();
}
