// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 - 2023 PHYTEC Messtechnik GmbH
 * Author: Wadim Egorov <w.egorov@phytec.de>
 */

#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <spl.h>
#include <asm/arch/k3-ddr.h>
#include <fdt_support.h>

#include "phycore-ddr-data.h"
#include "../common/k3/k3_ddrss_patch.h"
#include "../common/am6_som_detection.h"

#define AM64_DDRSS_SS_BASE	0x0F300000
#define DDRSS_V2A_CTL_REG	0x0020

DECLARE_GLOBAL_DATA_PTR;

static u8 phytec_get_am62_ddr_size_default(void)
{
	int ret;
	struct phytec_eeprom_data data;

	if (IS_ENABLED(CONFIG_PHYCORE_AM62X_RAM_SIZE_FIX)) {
		if (IS_ENABLED(CONFIG_PHYCORE_AM62X_RAM_SIZE_1GB))
			return EEPROM_RAM_SIZE_1GB;
		else if (IS_ENABLED(CONFIG_PHYCORE_AM62X_RAM_SIZE_2GB))
			return EEPROM_RAM_SIZE_2GB;
		else if (IS_ENABLED(CONFIG_PHYCORE_AM62X_RAM_SIZE_4GB))
			return EEPROM_RAM_SIZE_4GB;
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

	ram_size = phytec_get_am62_ddr_size_default();

	/*
	 * HACK: ddrss driver support 2GB RAM by default
	 * V2A_CTL_REG should be updated to support other RAM size
	 */
	if (IS_ENABLED(CONFIG_K3_AM64_DDRSS))
		if (ram_size == EEPROM_RAM_SIZE_4GB)
			writel(0x00000210, AM64_DDRSS_SS_BASE + DDRSS_V2A_CTL_REG);

	switch (ram_size) {
	case EEPROM_RAM_SIZE_1GB:
		gd->ram_size = 0x40000000;
		break;
	case EEPROM_RAM_SIZE_2GB:
		gd->ram_size = 0x80000000;
		break;
	case EEPROM_RAM_SIZE_4GB:
#ifdef CONFIG_PHYS_64BIT
		gd->ram_size = 0x100000000;
#else
		gd->ram_size = 0x80000000;
#endif
		break;
	default:
		gd->ram_size = 0x80000000;
	}

	return 0;
}

phys_size_t board_get_usable_ram_top(phys_size_t total_size)
{
#ifdef CONFIG_PHYS_64BIT
	/* Limit RAM used by U-Boot to the DDR low region */
	if (gd->ram_top > 0x100000000)
		return 0x100000000;
#endif
	return gd->ram_top;
}

int dram_init_banksize(void)
{
	u8 ram_size;

	memset(gd->bd->bi_dram, 0, sizeof(gd->bd->bi_dram[0]) * CONFIG_NR_DRAM_BANKS);

	if (!IS_ENABLED(CONFIG_CPU_V7R))
		return fdtdec_setup_memory_banksize();

	ram_size = phytec_get_am62_ddr_size_default();
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

	case EEPROM_RAM_SIZE_4GB:
		/* Bank 0 declares the memory available in the DDR low region */
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = 0x80000000;
		gd->ram_size = 0x80000000;

#ifdef CONFIG_PHYS_64BIT
		/* Bank 1 declares the memory available in the DDR upper region */
		gd->bd->bi_dram[1].start = 0x880000000;
		gd->bd->bi_dram[1].size = 0x80000000;
		gd->ram_size = 0x100000000;
#endif
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
int update_ddrss_timings(void)
{
	int ret;
	u8 ram_size;
	struct ddrss *ddr_patch = NULL;
	void *fdt = (void *)gd->fdt_blob;

	ram_size = phytec_get_am62_ddr_size_default();
	switch (ram_size) {
	case EEPROM_RAM_SIZE_1GB:
		ddr_patch = &phycore_ddrss_data[PHYCORE_1GB];
		break;
	case EEPROM_RAM_SIZE_2GB:
		ddr_patch = NULL;
		break;
	case EEPROM_RAM_SIZE_4GB:
		ddr_patch = &phycore_ddrss_data[PHYCORE_4GB];
		break;
	default:
		break;
	}

	/* Nothing to patch */
	if (!ddr_patch)
		return 0;

	debug("Applying DDRSS timings patch for ram_size %d\n", ram_size);

	ret = fdt_apply_ddrss_timings_patch(fdt, ddr_patch);
	if (ret < 0) {
		printf("Failed to apply ddrs timings patch %d\n", ret);
		return ret;
	}

	return 0;
}

int do_board_detect(void)
{
	int ret;
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

	ret = fdt_fixup_memory_banks(fdt, start, size, CONFIG_NR_DRAM_BANKS);
	if (ret)
		return ret;

	return update_ddrss_timings();
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

#define CTRLMMR_USB0_PHY_CTRL   0x43004008
#define CTRLMMR_USB1_PHY_CTRL   0x43004018
#define CORE_VOLTAGE            0x80000000

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void)
{
	u32 val;

	/* Set USB0 PHY core voltage to 0.85V */
	val = readl(CTRLMMR_USB0_PHY_CTRL);
	val &= ~(CORE_VOLTAGE);
	writel(val, CTRLMMR_USB0_PHY_CTRL);

	/* Set USB1 PHY core voltage to 0.85V */
	val = readl(CTRLMMR_USB1_PHY_CTRL);
	val &= ~(CORE_VOLTAGE);
	writel(val, CTRLMMR_USB1_PHY_CTRL);

	/* We have 32k crystal, so lets enable it */
	val = readl(MCU_CTRL_LFXOSC_CTRL);
	val &= ~(MCU_CTRL_LFXOSC_32K_DISABLE_VAL);
	writel(val, MCU_CTRL_LFXOSC_CTRL);
	/* Add any TRIM needed for the crystal here.. */
	/* Make sure to mux up to take the SoC 32k from the crystal */
	writel(MCU_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL,
	       MCU_CTRL_DEVICE_CLKOUT_32K_CTRL);
}
#endif
