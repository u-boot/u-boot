// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024-2026 NXP
 */
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <errno.h>
#include <imx_container.h>
#include <linux/bitfield.h>
#include <mmc.h>
#include <spi_flash.h>
#include <spl.h>
#include <stdlib.h>
#include <u-boot/crc.h>

#include <asm/arch/ddr.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/sys_proto.h>

#define QB_STATE_LOAD_SIZE		SZ_64K

#define BLK_DEV				0
#define SPI_DEV				1

#define IMG_FLAGS_IMG_TYPE_MASK		0xF
#define IMG_FLAGS_IMG_TYPE(x)		FIELD_GET(IMG_FLAGS_IMG_TYPE_MASK, (x))

#define IMG_TYPE_DDR_TDATA_DUMMY	0xD /* dummy DDR training data image */

static const struct {
	const char *ifname;
	const char *dev;
} imx_boot_devs[] = {
	[BOOT_DEVICE_MMC1] = { "mmc", "0" },
	[BOOT_DEVICE_MMC2] = { "mmc", "1" },
	[BOOT_DEVICE_SPI] = { "spi", "" },
};

static int imx_qb_get_board_boot_device(void)
{
	switch (get_boot_device()) {
	case SD1_BOOT:
	case MMC1_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC2;
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	case QSPI_BOOT:
		return BOOT_DEVICE_SPI;
	default:
		return BOOT_DEVICE_NONE;
	}
}

static int imx_qb_get_boot_dev_str(const char **ifname, const char **dev)
{
	int boot_dev;

	if (IS_ENABLED(CONFIG_XPL_BUILD))
		boot_dev = spl_boot_device();
	else
		boot_dev = imx_qb_get_board_boot_device();

	if (boot_dev == BOOT_DEVICE_NONE || boot_dev == BOOT_DEVICE_BOARD)
		return -EINVAL;

	*ifname = imx_boot_devs[boot_dev].ifname;
	*dev = imx_boot_devs[boot_dev].dev;

	return 0;
}

bool imx_qb_check(void)
{
	struct ddrphy_qb_state *qb_state;
	u32 size, crc;

	/*
	 * Ensure CRC is not empty, the reason is that
	 * the data is invalidated after first save run
	 * or after it is overwritten.
	 */
	qb_state = (struct ddrphy_qb_state *)CONFIG_QB_SAVED_STATE_BASE;
	size = sizeof(struct ddrphy_qb_state) - sizeof(qb_state->crc);
	crc = crc32(0, (u8 *)qb_state->mac, size);

	if (!qb_state->crc || crc != qb_state->crc)
		return false;

	return true;
}

static int imx_qb_get_blk_boot_part(const char * const ifname,
				    const char * const dev,
				    struct blk_desc **bdesc)
{
	struct udevice *udev;
	struct disk_partition info;
	struct mmc *mmc;
	int part;
	int ret;

	if (!IS_ENABLED(CONFIG_XPL_BUILD))
		return blk_get_device_part_str(ifname, dev, bdesc, &info, 1);

	/*
	 * SPL does not have access to part_get_info,
	 * so get the partition manually. Currently only
	 * supporting MMC devices.
	 */
	ret = blk_get_device_by_str(ifname, dev, bdesc);

	if (ret < 0)
		return -ENODEV;

	if ((*bdesc)->uclass_id != UCLASS_MMC)
		return -EOPNOTSUPP;

	udev = dev_get_parent((*bdesc)->bdev);
	mmc = mmc_get_mmc_dev(udev);

	if (IS_SD(mmc) || mmc->part_config == MMCPART_NOAVAILABLE)
		return 0;

	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);

	if (part == EMMC_BOOT_PART_BOOT1 || part == EMMC_BOOT_PART_BOOT2)
		return part;

	return 0;
}

static ulong imx_qb_get_boot_device_offset(void *dev, int dev_type)
{
	struct blk_desc *bdesc;

	switch (dev_type) {
	case BLK_DEV:
		bdesc = dev;

		/* eMMC boot partition */
		if (bdesc->hwpart)
			return CONTAINER_HDR_EMMC_OFFSET;

		return CONTAINER_HDR_MMCSD_OFFSET;
	case SPI_DEV:
		return CONTAINER_HDR_QSPI_OFFSET;
	default:
		return -EOPNOTSUPP;
	}
}

static int imx_qb_parse_container(void *addr, u64 *qb_data_off)
{
	struct container_hdr *phdr;
	struct boot_img_t *img_entry;
	u32 img_type, img_end;
	int i;

	phdr = addr;
	if (phdr->tag != 0x87 || (phdr->version != 0x0 && phdr->version != 0x2))
		return -EINVAL;

	img_entry = addr + sizeof(struct container_hdr);
	for (i = 0; i < phdr->num_images; i++) {
		img_type = IMG_FLAGS_IMG_TYPE(img_entry->hab_flags);
		if (img_type == IMG_TYPE_DDR_TDATA_DUMMY && img_entry->size == 0) {
			/* Image entry pointing to DDR Training Data */
			*qb_data_off = img_entry->offset;
			return 0;
		}

		img_end = img_entry->offset + img_entry->size;
		if (i + 1 < phdr->num_images) {
			img_entry++;
			if (img_end + QB_STATE_LOAD_SIZE == img_entry->offset) {
				/* hole detected */
				*qb_data_off = img_end;
				return 0;
			}
		}
	}

	return -EINVAL;
}

static int imx_qb_get_dev_qbdata_offset(void *dev, int dev_type, ulong offset,
					u64 *qbdata_offset)
{
	struct blk_desc *bdesc;
	u8 *buf;
	ulong count;
	int ret;

	buf = malloc(CONTAINER_HDR_ALIGNMENT);
	if (!buf)
		return -ENOMEM;

	switch (dev_type) {
	case BLK_DEV:
		bdesc = dev;

		count = blk_dread(bdesc,
				  offset / bdesc->blksz,
				  CONTAINER_HDR_ALIGNMENT / bdesc->blksz,
				  buf);
		if (count == 0) {
			printf("Read container image from MMC/SD failed\n");
			ret = -EIO;
			goto imx_qb_get_dev_qbdata_offset_exit;
		}
		break;
	case SPI_DEV:
		if (!CONFIG_IS_ENABLED(SPI)) {
			ret = -EOPNOTSUPP;
			goto imx_qb_get_dev_qbdata_offset_exit;
		}

		ret = spi_flash_read_dm(dev, offset,
					CONTAINER_HDR_ALIGNMENT, buf);
		if (ret) {
			printf("Read container header from SPI failed\n");
			ret = -EIO;
			goto imx_qb_get_dev_qbdata_offset_exit;
		}
		break;
	default:
		printf("Support for device %d not enabled\n", dev_type);
		ret = -EOPNOTSUPP;
		goto imx_qb_get_dev_qbdata_offset_exit;
	}

	ret = imx_qb_parse_container(buf, qbdata_offset);

imx_qb_get_dev_qbdata_offset_exit:
	free(buf);

	return ret;
}

static int imx_qb_get_qbdata_offset(void *dev, int dev_type,
				    u64 *qbdata_offset)
{
	u64 cont_offset;
	int ret, i;

	cont_offset = imx_qb_get_boot_device_offset(dev, dev_type);

	for (i = 0; i < 3; i++) {
		ret = imx_qb_get_dev_qbdata_offset(dev, dev_type, cont_offset,
						   qbdata_offset);
		if (ret == 0) {
			(*qbdata_offset) += cont_offset;
			break;
		}

		cont_offset += CONTAINER_HDR_ALIGNMENT;
	}

	return ret;
}

static int imx_qb_blk(const char * const ifname,
		      const char * const dev, bool save)
{
	struct blk_desc *bdesc;
	u64 offset;
	u64 load_size;
	int part, orig_part;
	int ret;

	part = imx_qb_get_blk_boot_part(ifname, dev, &bdesc);

	if (part < 0) {
		printf("Failed to find %s %s\n", ifname, dev);
		return -ENODEV;
	}

	orig_part = bdesc->hwpart;

	ret = blk_dselect_hwpart(bdesc, part);
	if (ret && ret != -EMEDIUMTYPE) {
		printf("Failed to select hwpart, ret %d\n", ret);
		return ret;
	}

	ret = imx_qb_get_qbdata_offset(bdesc, BLK_DEV, &offset);
	if (ret) {
		printf("get_qbdata_offset failed, ret = %d\n", ret);
		return ret;
	}

	offset /= bdesc->blksz;
	load_size = QB_STATE_LOAD_SIZE / bdesc->blksz;

	if (save) {
		/* QB data is stored in DDR -> can use it as buf */
		ret = blk_dwrite(bdesc, offset, load_size,
				 (const void *)CONFIG_QB_SAVED_STATE_BASE);
	} else {
		/* erase */
		ret = blk_derase(bdesc, offset, load_size);
	}

	if (ret != load_size) {
		printf("Failed to %s block device\n", save ? "write to" : "erase");
		return -EIO;
	}

	/* Return to original partition */
	ret = blk_dselect_hwpart(bdesc, orig_part);
	if (ret && ret != -EMEDIUMTYPE) {
		printf("Failed to select hwpart, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static int imx_qb_spi(bool save)
{
	struct udevice *flash;
	u64 offset;
	int ret;

	if (!CONFIG_IS_ENABLED(SPI)) {
		printf("SPI not enabled\n");
		return -EOPNOTSUPP;
	}

	ret = uclass_first_device_err(UCLASS_SPI_FLASH, &flash);
	if (ret) {
		printf("SPI flash not found.\n");
		return -ENODEV;
	}

	ret = imx_qb_get_qbdata_offset(flash, SPI_DEV, &offset);
	if (ret) {
		printf("get_qbdata_offset failed, ret = %d\n", ret);
		return ret;
	}

	ret = spi_flash_erase_dm(flash, offset, QB_STATE_LOAD_SIZE);

	if (ret)
		return ret;

	if (!save)
		return 0;

	/* QB data is stored in DDR -> can use it as buf */
	ret = spi_flash_write_dm(flash, offset,
				 QB_STATE_LOAD_SIZE,
				 (const void *)CONFIG_QB_SAVED_STATE_BASE);

	return ret;
}

int imx_qb(const char *ifname, const char *dev, bool save)
{
	int ret;

	ret = 0;

	/* Try to use boot device */
	if (!strcmp(ifname, "auto"))
		ret = imx_qb_get_boot_dev_str(&ifname, &dev);

	if (ret)
		return ret;

	if (save && !imx_qb_check())
		return -EINVAL;

	if (!strcmp(ifname, "spi"))
		ret = imx_qb_spi(save);
	else
		ret = imx_qb_blk(ifname, dev, save);

	if (ret)
		return ret;

	if (!save)
		return 0;

	/*
	 * invalidate qb_state mem so that at next boot
	 * the check function will fail and save won't happen
	 */
	memset((void *)CONFIG_QB_SAVED_STATE_BASE, 0,
	       sizeof(struct ddrphy_qb_state));

	return 0;
}

void spl_imx_qb_save(void)
{
	/* Save QB data on current boot device */
	if (imx_qb("auto", "", true))
		printf("QB save failed\n");
}
