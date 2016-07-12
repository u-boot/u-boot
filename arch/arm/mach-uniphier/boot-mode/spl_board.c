/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <linux/io.h>
#include <asm/processor.h>

#include "../soc-info.h"

void spl_board_announce_boot_device(void)
{
	printf("eMMC");
}

struct uniphier_romfunc_table {
	void *mmc_send_cmd;
	void *mmc_card_blockaddr;
	void *mmc_switch_part;
	void *mmc_load_image;
};

static const struct uniphier_romfunc_table uniphier_ld11_romfunc_table = {
	.mmc_send_cmd = (void *)0x20d8,
	.mmc_card_blockaddr = (void *)0x1b68,
	.mmc_switch_part = (void *)0x1c38,
	.mmc_load_image = (void *)0x2e48,
};

static const struct uniphier_romfunc_table uniphier_ld20_romfunc_table = {
	.mmc_send_cmd = (void *)0x2130,
	.mmc_card_blockaddr = (void *)0x1ba0,
	.mmc_switch_part = (void *)0x1c70,
	.mmc_load_image = (void *)0x2ef0,
};

int uniphier_rom_get_mmc_funcptr(int (**send_cmd)(u32, u32),
				 int (**card_blockaddr)(u32),
				 int (**switch_part)(int),
				 int (**load_image)(u32, uintptr_t, u32))
{
	const struct uniphier_romfunc_table *table;

	switch (uniphier_get_soc_type()) {
	case SOC_UNIPHIER_LD11:
		table = &uniphier_ld11_romfunc_table;
		break;
	case SOC_UNIPHIER_LD20:
		table = &uniphier_ld20_romfunc_table;
		break;
	default:
		printf("unsupported SoC\n");
		return -EINVAL;
	}

	*send_cmd = table->mmc_send_cmd;
	*card_blockaddr = table->mmc_card_blockaddr;
	*switch_part = table->mmc_switch_part;
	*load_image = table->mmc_load_image;

	return 0;
}

int spl_board_load_image(void)
{
	int (*send_cmd)(u32 cmd, u32 arg);
	int (*card_blockaddr)(u32 rca);
	int (*switch_part)(int part);
	int (*load_image)(u32 dev_addr, uintptr_t load_addr, u32 block_cnt);
	u32 dev_addr = CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
	const u32 rca = 0x1000; /* RCA assigned by Boot ROM */
	int ret;

	ret = uniphier_rom_get_mmc_funcptr(&send_cmd, &card_blockaddr,
					   &switch_part, &load_image);
	if (ret)
		return ret;

	/*
	 * deselect card before SEND_CSD command.
	 * Do not check the return code.  It fails, but it is OK.
	 */
	(*send_cmd)(0x071a0000, 0); /* CMD7 (arg=0) */

	/* reset CMD Line */
	writeb(0x6, 0x5a00022f);
	while (readb(0x5a00022f))
		cpu_relax();

	ret = (*card_blockaddr)(rca);
	if (ret) {
		debug("card is block addressing\n");
	} else {
		debug("card is byte addressing\n");
		dev_addr *= 512;
	}

	ret = (*send_cmd)(0x071a0000, rca << 16); /* CMD7: select card again */
	if (ret)
		printf("failed to select card\n");

	ret = (*switch_part)(1); /* Switch to Boot Partition 1 */
	if (ret)
		printf("failed to switch partition\n");

	ret = (*load_image)(dev_addr, CONFIG_SYS_TEXT_BASE, 1);
	if (ret) {
		printf("failed to load image\n");
		return ret;
	}

	ret = spl_parse_image_header((void *)CONFIG_SYS_TEXT_BASE);
	if (ret)
		return ret;

	ret = (*load_image)(dev_addr, spl_image.load_addr,
			    spl_image.size / 512);
	if (ret) {
		printf("failed to load image\n");
		return ret;
	}

	return 0;
}
