// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 PHYTEC America, LLC - https://www.phytec.com
 * Author: Matt McKee <mmckee@phytec.com>
 *
 * Copyright (C) 2022 - 2024 PHYTEC Messtechnik GmbH
 * Author: Wadim Egorov <w.egorov@phytec.de>
 */

#include <asm/io.h>
#include <env.h>
#include <env_internal.h>
#include <spl.h>
#include <asm/arch/k3-ddr.h>
#include <fdt_support.h>
#include <asm/arch/hardware.h>

#include "../common/am6_som_detection.h"

DECLARE_GLOBAL_DATA_PTR;

static u8 phytec_get_am64_ddr_size_default(void)
{
	int ret;
	struct phytec_eeprom_data data;

	if (IS_ENABLED(CONFIG_PHYCORE_AM64X_RAM_SIZE_FIX)) {
		if (IS_ENABLED(CONFIG_PHYCORE_AM64X_RAM_SIZE_1GB))
			return EEPROM_RAM_SIZE_1GB;
		else if (IS_ENABLED(CONFIG_PHYCORE_AM64X_RAM_SIZE_2GB))
			return EEPROM_RAM_SIZE_2GB;
	}

	ret = phytec_eeprom_data_setup(&data, 0, EEPROM_ADDR);
	if (!ret && data.valid)
		return phytec_get_am6_ddr_size(&data);

	/* Default DDR size is 2GB */
	return EEPROM_RAM_SIZE_2GB;
}

int dram_init(void)
{
	u8 ram_size;

	if (!IS_ENABLED(CONFIG_CPU_V7R))
		return fdtdec_setup_mem_size_base();

	ram_size = phytec_get_am64_ddr_size_default();

	switch (ram_size) {
	case EEPROM_RAM_SIZE_1GB:
		gd->ram_size = 0x40000000;
		break;
	case EEPROM_RAM_SIZE_2GB:
		gd->ram_size = 0x80000000;
		break;
	default:
		gd->ram_size = 0x80000000;
	}

	return 0;
}

int dram_init_banksize(void)
{
	u8 ram_size;

	memset(gd->bd->bi_dram, 0, sizeof(gd->bd->bi_dram[0]) * CONFIG_NR_DRAM_BANKS);

	if (!IS_ENABLED(CONFIG_CPU_V7R))
		return fdtdec_setup_memory_banksize();

	ram_size = phytec_get_am64_ddr_size_default();
	switch (ram_size) {
	case EEPROM_RAM_SIZE_1GB:
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = 0x40000000;
		gd->ram_size = 0x40000000;
		break;

	case EEPROM_RAM_SIZE_2GB:
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = 0x80000000;
		gd->ram_size = 0x80000000;
		break;

	default:
		/* Continue with default 2GB setup */
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = 0x80000000;
		gd->ram_size = 0x80000000;
		printf("DDR size %d is not supported\n", ram_size);
	}

	return 0;
}

#if defined(CONFIG_K3_DDRSS)
int do_board_detect(void)
{
	void *fdt = (void *)gd->fdt_blob;
	int bank;

	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	dram_init();
	dram_init_banksize();

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start[bank] = gd->bd->bi_dram[bank].start;
		size[bank] = gd->bd->bi_dram[bank].size;
	}

	return fdt_fixup_memory_banks(fdt, start, size, CONFIG_NR_DRAM_BANKS);
}
#endif

#if IS_ENABLED(CONFIG_XPL_BUILD)
void spl_perform_fixups(struct spl_image_info *spl_image)
{
	if (IS_ENABLED(CONFIG_K3_DDRSS) && IS_ENABLED(CONFIG_K3_INLINE_ECC))
		fixup_ddr_driver_for_ecc(spl_image);
	else
		fixup_memory_node(spl_image);
}
#endif

#define CTRLMMR_USB0_PHY_CTRL	0x43004008
#define CORE_VOLTAGE		0x80000000

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void)
{
	u32 val;

	/* Set USB PHY core voltage to 0.85V */
	val = readl(CTRLMMR_USB0_PHY_CTRL);
	val &= ~(CORE_VOLTAGE);
	writel(val, CTRLMMR_USB0_PHY_CTRL);

	/* Init DRAM size for R5/A53 SPL */
	dram_init_banksize();
}
#endif
