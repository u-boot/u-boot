// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <errno.h>
#include <image.h>
#include <linux/libfdt.h>
#include <spl.h>

#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

static int is_boot_from_stream_device(u32 boot)
{
	u32 interface;

	interface = boot >> 16;
	if (interface >= BT_DEV_TYPE_USB)
		return 1;

	if (interface == BT_DEV_TYPE_MMC && (boot & 1))
		return 1;

	return 0;
}

static ulong spl_romapi_read_seekable(struct spl_load_info *load,
				      ulong sector, ulong count,
				      void *buf)
{
	u32 pagesize = *(u32 *)load->priv;
	volatile gd_t *pgd = gd;
	ulong byte = count * pagesize;
	int ret;
	u32 offset;

	offset = sector * pagesize;

	debug("ROM API load from 0x%x, size 0x%x\n", offset, (u32)byte);

	ret = g_rom_api->download_image(buf, offset, byte,
					((uintptr_t)buf) ^ offset ^ byte);
	gd = pgd;

	if (ret == ROM_API_OKAY)
		return count;

	printf("ROM API Failure when load 0x%x\n", offset);

	return 0;
}

static int spl_romapi_load_image_seekable(struct spl_image_info *spl_image,
					  struct spl_boot_device *bootdev,
					  u32 rom_bt_dev)
{
	volatile gd_t *pgd = gd;
	int ret;
	u32 offset;
	u32 pagesize, size;
	struct image_header *header;
	u32 image_offset;

	ret = g_rom_api->query_boot_infor(QUERY_IVT_OFF, &offset,
					  ((uintptr_t)&offset) ^ QUERY_IVT_OFF);
	ret |= g_rom_api->query_boot_infor(QUERY_PAGE_SZ, &pagesize,
					   ((uintptr_t)&pagesize) ^ QUERY_PAGE_SZ);
	ret |= g_rom_api->query_boot_infor(QUERY_IMG_OFF, &image_offset,
					   ((uintptr_t)&image_offset) ^ QUERY_IMG_OFF);

	gd = pgd;

	if (ret != ROM_API_OKAY) {
		puts("ROMAPI: Failure query boot infor pagesize/offset\n");
		return -1;
	}

	header = (struct image_header *)(CONFIG_SPL_IMX_ROMAPI_LOADADDR);

	printf("image offset 0x%x, pagesize 0x%x, ivt offset 0x%x\n",
	       image_offset, pagesize, offset);

	if (((rom_bt_dev >> 16) & 0xff) ==  BT_DEV_TYPE_FLEXSPINOR)
		offset = CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512;
	else
		offset = image_offset +
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512 - 0x8000;

	size = ALIGN(sizeof(struct image_header), pagesize);
	ret = g_rom_api->download_image((u8 *)header, offset, size,
					((uintptr_t)header) ^ offset ^ size);
	gd = pgd;

	if (ret != ROM_API_OKAY) {
		printf("ROMAPI: download failure offset 0x%x size 0x%x\n",
		       offset, size);
		return -1;
	}

	if (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		struct spl_load_info load;

		memset(&load, 0, sizeof(load));
		load.bl_len = pagesize;
		load.read = spl_romapi_read_seekable;
		load.priv = &pagesize;
		return spl_load_simple_fit(spl_image, &load,
					   offset / pagesize, header);
	} else {
		/* TODO */
		puts("Can't support legacy image\n");
		return -1;
	}

	return 0;
}

static ulong spl_ram_load_read(struct spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	memcpy(buf, (void *)(sector), count);

	if (load->priv) {
		ulong *p = (ulong *)load->priv;
		ulong total = sector + count;

		if (total > *p)
			*p = total;
	}

	return count;
}

static ulong get_fit_image_size(void *fit)
{
	struct spl_image_info spl_image;
	struct spl_load_info spl_load_info;
	ulong last = (ulong)fit;

	memset(&spl_load_info, 0, sizeof(spl_load_info));
	spl_load_info.bl_len = 1;
	spl_load_info.read = spl_ram_load_read;
	spl_load_info.priv = &last;

	spl_load_simple_fit(&spl_image, &spl_load_info,
			    (uintptr_t)fit, fit);

	return last - (ulong)fit;
}

u8 *search_fit_header(u8 *p, int size)
{
	int i;

	for (i = 0; i < size; i += 4)
		if (genimg_get_format(p + i) == IMAGE_FORMAT_FIT)
			return p + i;

	return NULL;
}

static int spl_romapi_load_image_stream(struct spl_image_info *spl_image,
					struct spl_boot_device *bootdev)
{
	struct spl_load_info load;
	volatile gd_t *pgd = gd;
	u32 pagesize, pg;
	int ret;
	int i = 0;
	u8 *p = (u8 *)CONFIG_SPL_IMX_ROMAPI_LOADADDR;
	u8 *pfit = NULL;
	int imagesize;
	int total;

	ret = g_rom_api->query_boot_infor(QUERY_PAGE_SZ, &pagesize,
					  ((uintptr_t)&pagesize) ^ QUERY_PAGE_SZ);
	gd = pgd;

	if (ret != ROM_API_OKAY)
		puts("failure at query_boot_info\n");

	pg = pagesize;
	if (pg < 1024)
		pg = 1024;

	for (i = 0; i < 640; i++) {
		ret = g_rom_api->download_image(p, 0, pg,
						((uintptr_t)p) ^ pg);
		gd = pgd;

		if (ret != ROM_API_OKAY) {
			puts("Steam(USB) download failure\n");
			return -1;
		}

		pfit = search_fit_header(p, pg);
		p += pg;

		if (pfit)
			break;
	}

	if (!pfit) {
		puts("Can't found uboot FIT image in 640K range \n");
		return -1;
	}

	if (p - pfit < sizeof(struct fdt_header)) {
		ret = g_rom_api->download_image(p, 0, pg,  ((uintptr_t)p) ^ pg);
		gd = pgd;

		if (ret != ROM_API_OKAY) {
			puts("Steam(USB) download failure\n");
			return -1;
		}

		p += pg;
	}

	imagesize = fit_get_size(pfit);
	printf("Find FIT header 0x&%p, size %d\n", pfit, imagesize);

	if (p - pfit < imagesize) {
		imagesize -= p - pfit;
		/*need pagesize hear after ROM fix USB problme*/
		imagesize += pg - 1;
		imagesize /= pg;
		imagesize *= pg;

		printf("Need continue download %d\n", imagesize);

		ret = g_rom_api->download_image(p, 0, imagesize,
						((uintptr_t)p) ^ imagesize);
		gd = pgd;

		p += imagesize;

		if (ret != ROM_API_OKAY) {
			printf("Failure download %d\n", imagesize);
			return -1;
		}
	}

	total = get_fit_image_size(pfit);
	total += 3;
	total &= ~0x3;

	imagesize = total - (p - pfit);

	imagesize += pagesize - 1;
	imagesize /= pagesize;
	imagesize *= pagesize;

	printf("Download %d, total fit %d\n", imagesize, total);

	ret = g_rom_api->download_image(p, 0, imagesize,
					((uintptr_t)p) ^ imagesize);
	if (ret != ROM_API_OKAY)
		printf("ROM download failure %d\n", imagesize);

	memset(&load, 0, sizeof(load));
	load.bl_len = 1;
	load.read = spl_ram_load_read;

	return spl_load_simple_fit(spl_image, &load, (ulong)pfit, pfit);
}

int board_return_to_bootrom(struct spl_image_info *spl_image,
			    struct spl_boot_device *bootdev)
{
	volatile gd_t *pgd = gd;
	int ret;
	u32 boot;

	ret = g_rom_api->query_boot_infor(QUERY_BT_DEV, &boot,
					  ((uintptr_t)&boot) ^ QUERY_BT_DEV);
	gd =  pgd;

	if (ret != ROM_API_OKAY) {
		puts("ROMAPI: failure at query_boot_info\n");
		return -1;
	}

	if (is_boot_from_stream_device(boot))
		return spl_romapi_load_image_stream(spl_image, bootdev);

	return spl_romapi_load_image_seekable(spl_image, bootdev, boot);
}
