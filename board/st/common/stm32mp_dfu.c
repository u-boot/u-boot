// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <dfu.h>
#include <env.h>
#include <log.h>
#include <memalign.h>
#include <misc.h>
#include <mtd.h>
#include <mtd_node.h>
#include <asm/arch/stm32prog.h>

#define DFU_ALT_BUF_LEN SZ_1K

static void board_get_alt_info_mmc(struct udevice *dev, char *buf)
{
	struct disk_partition info;
	int p, len, devnum;
	bool first = true;
	const char *name;
	struct mmc *mmc;
	struct blk_desc *desc;

	mmc = mmc_get_mmc_dev(dev);
	if (!mmc)
		return;

	if (mmc_init(mmc))
		return;

	desc = mmc_get_blk_desc(mmc);
	if (!desc)
		return;

	name = blk_get_if_type_name(desc->if_type);
	devnum = desc->devnum;
	len = strlen(buf);

	if (buf[0] != '\0')
		len += snprintf(buf + len,
				DFU_ALT_BUF_LEN - len, "&");
	len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
			 "%s %d=", name, devnum);

	if (IS_MMC(mmc) && mmc->capacity_boot) {
		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
				"%s%d_boot1 raw 0x0 0x%llx mmcpart 1;",
				name, devnum, mmc->capacity_boot);
		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
				"%s%d_boot2 raw 0x0 0x%llx mmcpart 2",
				name, devnum, mmc->capacity_boot);
		first = false;
	}

	for (p = 1; p < MAX_SEARCH_PARTITIONS; p++) {
		if (part_get_info(desc, p, &info))
			continue;
		if (!first)
			len += snprintf(buf + len, DFU_ALT_BUF_LEN - len, ";");
		first = false;
		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
				"%s%d_%s part %d %d",
				name, devnum, info.name, devnum, p);
	}
}

static void board_get_alt_info_mtd(struct mtd_info *mtd, char *buf)
{
	struct mtd_info *part;
	bool first = true;
	const char *name;
	int len, partnum = 0;

	name = mtd->name;
	len = strlen(buf);

	if (buf[0] != '\0')
		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len, "&");
	len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
			"mtd %s=", name);

	len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
			"%s raw 0x0 0x%llx ",
			name, mtd->size);

	list_for_each_entry(part, &mtd->partitions, node) {
		partnum++;
		if (!first)
			len += snprintf(buf + len, DFU_ALT_BUF_LEN - len, ";");
		first = false;

		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
				"%s_%s part %d",
				name, part->name, partnum);
	}
}

void set_dfu_alt_info(char *interface, char *devstr)
{
	struct udevice *dev;
	struct mtd_info *mtd;

	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);

	if (env_get("dfu_alt_info"))
		return;

	memset(buf, 0, sizeof(buf));

	snprintf(buf, DFU_ALT_BUF_LEN,
		 "ram 0=%s", CONFIG_DFU_ALT_RAM0);

	if (CONFIG_IS_ENABLED(MMC)) {
		if (!uclass_get_device(UCLASS_MMC, 0, &dev))
			board_get_alt_info_mmc(dev, buf);

		if (!uclass_get_device(UCLASS_MMC, 1, &dev))
			board_get_alt_info_mmc(dev, buf);
	}

	if (CONFIG_IS_ENABLED(MTD)) {
		/* probe all MTD devices */
		mtd_probe_devices();

		/* probe SPI flash device on a bus */
		if (!uclass_get_device(UCLASS_SPI_FLASH, 0, &dev)) {
			mtd = get_mtd_device_nm("nor0");
			if (!IS_ERR_OR_NULL(mtd))
				board_get_alt_info_mtd(mtd, buf);
		}

		mtd = get_mtd_device_nm("nand0");
		if (!IS_ERR_OR_NULL(mtd))
			board_get_alt_info_mtd(mtd, buf);

		mtd = get_mtd_device_nm("spi-nand0");
		if (!IS_ERR_OR_NULL(mtd))
			board_get_alt_info_mtd(mtd, buf);
	}

	if (IS_ENABLED(CONFIG_DFU_VIRT) &&
	    IS_ENABLED(CMD_STM32PROG_USB)) {
		strncat(buf, "&virt 0=OTP", DFU_ALT_BUF_LEN);

		if (IS_ENABLED(CONFIG_PMIC_STPMIC1))
			strncat(buf, "&virt 1=PMIC", DFU_ALT_BUF_LEN);
	}

	env_set("dfu_alt_info", buf);
	puts("DFU alt info setting: done\n");
}

#if CONFIG_IS_ENABLED(DFU_VIRT)
#include <dfu.h>
#include <power/stpmic1.h>

static int dfu_otp_read(u64 offset, u8 *buffer, long *size)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, offset + STM32_BSEC_OTP_OFFSET, buffer, *size);
	if (ret >= 0) {
		*size = ret;
		ret = 0;
	}

	return 0;
}

static int dfu_pmic_read(u64 offset, u8 *buffer, long *size)
{
	int ret;
#ifdef CONFIG_PMIC_STPMIC1
	struct udevice *dev;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stpmic1_nvm),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, 0xF8 + offset, buffer, *size);
	if (ret >= 0) {
		*size = ret;
		ret = 0;
	}
	if (ret == -EACCES) {
		*size = 0;
		ret = 0;
	}
#else
	log_err("PMIC update not supported");
	ret = -EOPNOTSUPP;
#endif

	return ret;
}

int dfu_read_medium_virt(struct dfu_entity *dfu, u64 offset,
			 void *buf, long *len)
{
	switch (dfu->data.virt.dev_num) {
	case 0x0:
		return dfu_otp_read(offset, buf, len);
	case 0x1:
		return dfu_pmic_read(offset, buf, len);
	}

	if (IS_ENABLED(CONFIG_CMD_STM32PROG_USB) &&
	    dfu->data.virt.dev_num >= STM32PROG_VIRT_FIRST_DEV_NUM)
		return stm32prog_read_medium_virt(dfu, offset, buf, len);

	*len = 0;
	return 0;
}

int dfu_write_medium_virt(struct dfu_entity *dfu, u64 offset,
			  void *buf, long *len)
{
	if (IS_ENABLED(CONFIG_CMD_STM32PROG_USB) &&
	    dfu->data.virt.dev_num >= STM32PROG_VIRT_FIRST_DEV_NUM)
		return stm32prog_write_medium_virt(dfu, offset, buf, len);

	return -EOPNOTSUPP;
}

int __weak dfu_get_medium_size_virt(struct dfu_entity *dfu, u64 *size)
{
	if (IS_ENABLED(CONFIG_CMD_STM32PROG_USB) &&
	    dfu->data.virt.dev_num >= STM32PROG_VIRT_FIRST_DEV_NUM)
		return stm32prog_get_medium_size_virt(dfu, size);

	*size = SZ_1K;

	return 0;
}

#endif
