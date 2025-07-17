// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <config.h>
#include <errno.h>
#include <imx_container.h>
#include <log.h>
#include <malloc.h>
#include <asm/io.h>
#include <mmc.h>
#include <spi_flash.h>
#include <spl.h>
#include <nand.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>

#define MMC_DEV		0
#define QSPI_DEV	1
#define NAND_DEV	2
#define QSPI_NOR_DEV	3
#define ROM_API_DEV	4

/* The unit of second image offset number which provision by the fuse bits */
#define SND_IMG_OFF_UNIT    (0x100000UL)

/*
 * If num = 0, off = (2 ^ 2) * 1MB
 * else If num = 2, off = (2 ^ 0) * 1MB
 * else off = (2 ^ num) * 1MB
 */
#define SND_IMG_NUM_TO_OFF(num) \
	((1UL << ((0 == (num)) ? 2 : (2 == (num)) ? 0 : (num))) * SND_IMG_OFF_UNIT)

#define GET_SND_IMG_NUM(fuse) (((fuse) >> 24) & 0x1F)

#if defined(CONFIG_IMX8QM)
#define FUSE_IMG_SET_OFF_WORD 464
#elif defined(CONFIG_IMX8QXP)
#define FUSE_IMG_SET_OFF_WORD 720
#endif

#define MAX_V2X_CTNR_IMG_NUM   (4)
#define MIN_V2X_CTNR_IMG_NUM   (2)

#define IMG_FLAGS_IMG_TYPE_SHIFT  (0u)
#define IMG_FLAGS_IMG_TYPE_MASK   (0xfU)
#define IMG_FLAGS_IMG_TYPE(x)     (((x) & IMG_FLAGS_IMG_TYPE_MASK) >> \
								   IMG_FLAGS_IMG_TYPE_SHIFT)

#define IMG_FLAGS_CORE_ID_SHIFT   (4u)
#define IMG_FLAGS_CORE_ID_MASK    (0xf0U)
#define IMG_FLAGS_CORE_ID(x)      (((x) & IMG_FLAGS_CORE_ID_MASK) >> \
								   IMG_FLAGS_CORE_ID_SHIFT)

#define IMG_TYPE_V2X_PRI_FW     (0x0Bu)   /* Primary V2X FW */
#define IMG_TYPE_V2X_SND_FW     (0x0Cu)   /* Secondary V2X FW */

#define CORE_V2X_PRI 9
#define CORE_V2X_SND 10

static bool is_v2x_fw_container(ulong addr)
{
	struct container_hdr *phdr;
	struct boot_img_t *img_entry;

	phdr = (struct container_hdr *)addr;
	if ((phdr->tag != 0x87 && phdr->tag != 0x82) || phdr->version != 0x0) {
		debug("Wrong container header\n");
		return false;
	}

	if (phdr->num_images >= MIN_V2X_CTNR_IMG_NUM && phdr->num_images <= MAX_V2X_CTNR_IMG_NUM) {
		img_entry = (struct boot_img_t *)(addr + sizeof(struct container_hdr));

		if (IMG_FLAGS_IMG_TYPE(img_entry->hab_flags) == IMG_TYPE_V2X_PRI_FW &&
		    IMG_FLAGS_CORE_ID(img_entry->hab_flags) == CORE_V2X_PRI) {
			img_entry++;

			if (IMG_FLAGS_IMG_TYPE(img_entry->hab_flags) == IMG_TYPE_V2X_SND_FW &&
			    IMG_FLAGS_CORE_ID(img_entry->hab_flags) == CORE_V2X_SND)
				return true;
		}
	}

	return false;
}

int get_container_size(ulong addr, u16 *header_length)
{
	struct container_hdr *phdr;
	struct boot_img_t *img_entry;
	struct signature_block_hdr *sign_hdr;
	u8 i = 0;
	u32 max_offset = 0, img_end;

	phdr = (struct container_hdr *)addr;
	if (!valid_container_hdr(phdr)) {
		debug("Wrong container header\n");
		return -EFAULT;
	}

	max_offset = phdr->length_lsb + (phdr->length_msb << 8);
	if (header_length)
		*header_length = max_offset;

	img_entry = (struct boot_img_t *)(addr + sizeof(struct container_hdr));
	for (i = 0; i < phdr->num_images; i++) {
		img_end = img_entry->offset + img_entry->size;
		if (img_end > max_offset)
			max_offset = img_end;

		debug("img[%u], end = 0x%x\n", i, img_end);

		img_entry++;
	}

	if (phdr->sig_blk_offset != 0) {
		sign_hdr = (struct signature_block_hdr *)(addr + phdr->sig_blk_offset);
		u16 len = sign_hdr->length_lsb + (sign_hdr->length_msb << 8);

		if (phdr->sig_blk_offset + len > max_offset)
			max_offset = phdr->sig_blk_offset + len;

		debug("sigblk, end = 0x%x\n", phdr->sig_blk_offset + len);
	}

	return max_offset;
}

static int get_dev_container_size(void *dev, int dev_type, unsigned long offset, u16 *header_length, bool *v2x_cntr)
{
	u8 *buf = malloc(CONTAINER_HDR_ALIGNMENT);
	int ret = 0;

	if (!buf) {
		printf("Malloc buffer failed\n");
		return -ENOMEM;
	}

#ifdef CONFIG_SPL_MMC
	if (dev_type == MMC_DEV) {
		unsigned long count = 0;
		struct mmc *mmc = (struct mmc *)dev;

		count = blk_dread(mmc_get_blk_desc(mmc),
				  offset / mmc->read_bl_len,
				  CONTAINER_HDR_ALIGNMENT / mmc->read_bl_len,
				  buf);
		if (count == 0) {
			printf("Read container image from MMC/SD failed\n");
			return -EIO;
		}
	}
#endif

#ifdef CONFIG_SPL_SPI_LOAD
	if (dev_type == QSPI_DEV) {
		struct spi_flash *flash = (struct spi_flash *)dev;

		ret = spi_flash_read(flash, offset,
				     CONTAINER_HDR_ALIGNMENT, buf);
		if (ret != 0) {
			printf("Read container image from QSPI failed\n");
			return -EIO;
		}
	}
#endif

#ifdef CONFIG_SPL_NAND_SUPPORT
	if (dev_type == NAND_DEV) {
		ret = nand_spl_load_image(offset, CONTAINER_HDR_ALIGNMENT,
					  buf);
		if (ret != 0) {
			printf("Read container image from NAND failed\n");
			return -EIO;
		}
	}
#endif

#ifdef CONFIG_SPL_NOR_SUPPORT
	if (dev_type == QSPI_NOR_DEV)
		memcpy(buf, (const void *)offset, CONTAINER_HDR_ALIGNMENT);
#endif

#ifdef CONFIG_SPL_BOOTROM_SUPPORT
	if (dev_type == ROM_API_DEV) {
		ret = spl_romapi_raw_seekable_read(offset, CONTAINER_HDR_ALIGNMENT, buf);
		if (!ret) {
			printf("Read container image from ROM API failed\n");
			return -EIO;
		}
	}
#endif

	ret = get_container_size((ulong)buf, header_length);

	if (v2x_cntr)
		*v2x_cntr = is_v2x_fw_container((ulong)buf);

	free(buf);

	return ret;
}

static bool check_secondary_cnt_set(unsigned long *set_off)
{
#if IS_ENABLED(CONFIG_ARCH_IMX8)
	int ret;
	u8 set_id = 1;
	u32 fuse_val = 0;

	if (!(is_imx8qxp() && is_soc_rev(CHIP_REV_B))) {
		ret = sc_misc_get_boot_container(-1, &set_id);
		if (ret)
			return false;
		/* Secondary boot */
		if (set_id == 2) {
			ret = sc_misc_otp_fuse_read(-1, FUSE_IMG_SET_OFF_WORD, &fuse_val);
			if (!ret) {
				if (set_off)
					*set_off = SND_IMG_NUM_TO_OFF(GET_SND_IMG_NUM(fuse_val));
				return true;
			}
		}
	}
#endif

	return false;
}

static unsigned long get_boot_device_offset(void *dev, int dev_type)
{
	unsigned long offset = 0, sec_set_off = 0;
	bool sec_boot = false;

	if (dev_type == ROM_API_DEV) {
		offset = (unsigned long)dev;
		return offset;
	}

	sec_boot = check_secondary_cnt_set(&sec_set_off);
	if (sec_boot)
		printf("Secondary set selected\n");
	else
		printf("Primary set selected\n");

	if (dev_type == MMC_DEV) {
		struct mmc *mmc = (struct mmc *)dev;

		if (IS_SD(mmc) || mmc->part_config == MMCPART_NOAVAILABLE) {
			offset = sec_boot ? sec_set_off : CONTAINER_HDR_MMCSD_OFFSET;
		} else {
			u8 part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);

			if (part == EMMC_BOOT_PART_BOOT1 || part == EMMC_BOOT_PART_BOOT2) {
				if (is_imx8qxp() && is_soc_rev(CHIP_REV_B))
					offset = CONTAINER_HDR_MMCSD_OFFSET;
				else
					offset = CONTAINER_HDR_EMMC_OFFSET;
			} else {
				offset = sec_boot ? sec_set_off : CONTAINER_HDR_MMCSD_OFFSET;
			}
		}
	} else if (dev_type == QSPI_DEV) {
		offset = sec_boot ? (sec_set_off + CONTAINER_HDR_QSPI_OFFSET) :
			CONTAINER_HDR_QSPI_OFFSET;
	} else if (dev_type == NAND_DEV) {
		offset = sec_boot ? (sec_set_off + CONTAINER_HDR_NAND_OFFSET) :
			CONTAINER_HDR_NAND_OFFSET;
	} else if (dev_type == QSPI_NOR_DEV) {
		offset = CONTAINER_HDR_QSPI_OFFSET + 0x08000000;
	} else {
		printf("Not supported dev_type: %d\n", dev_type);
	}

	debug("container set offset 0x%lx\n", offset);

	return offset;
}

static ulong get_imageset_end(void *dev, int dev_type)
{
	unsigned long offset[3] = {};
	int value_container[3] = {};
	u16 hdr_length;
	bool v2x_fw = false;

	offset[0] = get_boot_device_offset(dev, dev_type);

	value_container[0] = get_dev_container_size(dev, dev_type, offset[0], &hdr_length, NULL);
	if (value_container[0] < 0) {
		printf("Parse seco container failed %d\n", value_container[0]);
		return 0;
	}

	debug("seco container size 0x%x\n", value_container[0]);

	if (is_imx95()) {
		offset[1] = ALIGN(hdr_length, CONTAINER_HDR_ALIGNMENT) + offset[0];

		value_container[1] = get_dev_container_size(dev, dev_type, offset[1], &hdr_length, &v2x_fw);
		if (value_container[1] < 0) {
			printf("Parse v2x container failed %d\n", value_container[1]);
			return value_container[0] + offset[0]; /* return seco container total size */
		}

		if (v2x_fw) {
			debug("v2x container size 0x%x\n", value_container[1]);
			offset[2] = ALIGN(hdr_length, CONTAINER_HDR_ALIGNMENT) + offset[1];
		} else {
			printf("no v2x container included\n");
			offset[2] = offset[1];
		}
	} else {
		/* Skip offset[1] */
		offset[2] = ALIGN(hdr_length, CONTAINER_HDR_ALIGNMENT) + offset[0];
	}

	value_container[2] = get_dev_container_size(dev, dev_type, offset[2], &hdr_length, NULL);
	if (value_container[2] < 0) {
		debug("Parse scu container image failed %d, only seco container\n", value_container[2]);
		if (is_imx95())
			return value_container[1] + offset[1]; /* return seco + v2x container total size */
		else
			return value_container[0] + offset[0]; /* return seco container total size */
	}

	debug("scu container size 0x%x\n", value_container[2]);

	return value_container[2] + offset[2];
}

#ifdef CONFIG_SPL_SPI_LOAD
unsigned int spl_spi_get_uboot_offs(struct spi_flash *flash)
{
	ulong end;

	end = get_imageset_end(flash, QSPI_DEV);
	end = ROUND(end, SZ_1K);

	printf("Load image from QSPI 0x%lx\n", end);

	return end;
}
#endif

#ifdef CONFIG_SPL_MMC
unsigned long arch_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						unsigned long raw_sect)
{
	ulong end;

	end = get_imageset_end(mmc, MMC_DEV);
	end = ROUND(end, SZ_1K);

	printf("Load image from MMC/SD 0x%lx\n", end);

	return end / mmc->read_bl_len;
}

int spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	int part;

	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	if (part == EMMC_BOOT_PART_BOOT1 || part == EMMC_BOOT_PART_BOOT2) {
		unsigned long sec_set_off = 0;
		bool sec_boot = false;

		sec_boot = check_secondary_cnt_set(&sec_set_off);
		if (sec_boot)
			part = (part == EMMC_BOOT_PART_BOOT1) ? EMMC_HWPART_BOOT2 : EMMC_HWPART_BOOT1;
	} else if (part == EMMC_BOOT_PART_USER) {
		part = EMMC_HWPART_DEFAULT;
	}

	return part;
}
#endif

#ifdef CONFIG_SPL_NAND_SUPPORT
uint32_t spl_nand_get_uboot_raw_page(void)
{
	ulong end;

	end = get_imageset_end((void *)NULL, NAND_DEV);
	end = ROUND(end, SZ_16K);

	printf("Load image from NAND 0x%lx\n", end);

	return end;
}
#endif

#ifdef CONFIG_SPL_NOR_SUPPORT
unsigned long spl_nor_get_uboot_base(void)
{
	ulong end;

	/* Calculate the image set end,
	 * if it is less than CFG_SYS_UBOOT_BASE(0x8281000),
	 * we use CFG_SYS_UBOOT_BASE
	 * Otherwise, use the calculated address
	 */
	end = get_imageset_end((void *)NULL, QSPI_NOR_DEV);
	if (end <= CFG_SYS_UBOOT_BASE)
		end = CFG_SYS_UBOOT_BASE;
	else
		end = ROUND(end, SZ_1K);

	printf("Load image from NOR 0x%lx\n", end);

	return end;
}
#endif

#ifdef CONFIG_SPL_BOOTROM_SUPPORT
u32 __weak spl_arch_boot_image_offset(u32 image_offset, u32 rom_bt_dev)
{
	return image_offset;
}

ulong spl_romapi_get_uboot_base(u32 image_offset, u32 rom_bt_dev)
{
	ulong end;

	image_offset = spl_arch_boot_image_offset(image_offset, rom_bt_dev);

	end = get_imageset_end((void *)(ulong)image_offset, ROM_API_DEV);
	end = ROUND(end, SZ_1K);

	printf("Load image from 0x%lx by ROM_API\n", end);

	return end;
}
#endif
