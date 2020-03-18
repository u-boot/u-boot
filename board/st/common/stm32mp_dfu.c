// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dfu.h>
#include <env.h>
#include <memalign.h>
#include <misc.h>
#include <mtd.h>
#include <mtd_node.h>

#define DFU_ALT_BUF_LEN SZ_1K

static void board_get_alt_info(const char *dev, char *buff)
{
	char var_name[32] = "dfu_alt_info_";
	int ret;

	ALLOC_CACHE_ALIGN_BUFFER(char, tmp_alt, DFU_ALT_BUF_LEN);

	/* name of env variable to read = dfu_alt_info_<dev> */
	strcat(var_name, dev);
	ret = env_get_f(var_name, tmp_alt, DFU_ALT_BUF_LEN);
	if (ret) {
		if (buff[0] != '\0')
			strcat(buff, "&");
		strncat(buff, tmp_alt, DFU_ALT_BUF_LEN);
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

	/* probe all MTD devices */
	mtd_probe_devices();

	board_get_alt_info("ram", buf);

	if (!uclass_get_device(UCLASS_MMC, 0, &dev))
		board_get_alt_info("mmc0", buf);

	if (!uclass_get_device(UCLASS_MMC, 1, &dev))
		board_get_alt_info("mmc1", buf);

	if (!uclass_get_device(UCLASS_SPI_FLASH, 0, &dev))
		board_get_alt_info("nor0", buf);

	mtd = get_mtd_device_nm("nand0");
	if (!IS_ERR_OR_NULL(mtd))
		board_get_alt_info("nand0", buf);

	mtd = get_mtd_device_nm("spi-nand0");
	if (!IS_ERR_OR_NULL(mtd))
		board_get_alt_info("spi-nand0", buf);

#ifdef CONFIG_DFU_VIRT
	strncat(buf, "&virt 0=OTP", DFU_ALT_BUF_LEN);

	if (IS_ENABLED(CONFIG_PMIC_STPMIC1))
		strncat(buf, "&virt 1=PMIC", DFU_ALT_BUF_LEN);
#endif

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
					  DM_GET_DRIVER(stm32mp_bsec),
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
					  DM_GET_DRIVER(stpmic1_nvm),
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
	pr_err("PMIC update not supported");
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
	*len = 0;
	return 0;
}

int __weak dfu_get_medium_size_virt(struct dfu_entity *dfu, u64 *size)
{
	*size = SZ_1K;

	return 0;
}

#endif
