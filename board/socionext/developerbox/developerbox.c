// SPDX-License-Identifier: GPL-2.0
/*
 * u-boot/board/socionext/developerbox/developerbox.c
 *
 * Copyright (C) 2016-2017 Socionext Inc.
 * Copyright (C) 2021 Linaro Ltd.
 */
#include <asm/types.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <common.h>
#include <efi.h>
#include <efi_loader.h>
#include <env_internal.h>
#include <fdt_support.h>
#include <log.h>

#include <linux/kernel.h>

#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
	{
		.image_type_id = DEVELOPERBOX_UBOOT_IMAGE_GUID,
		.fw_name = u"DEVELOPERBOX-UBOOT",
		.image_index = 1,
	},
	{
		.image_type_id = DEVELOPERBOX_FIP_IMAGE_GUID,
		.fw_name = u"DEVELOPERBOX-FIP",
		.image_index = 2,
	},
	{
		.image_type_id = DEVELOPERBOX_OPTEE_IMAGE_GUID,
		.fw_name = u"DEVELOPERBOX-OPTEE",
		.image_index = 3,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mtd nor1=u-boot.bin raw 200000 100000;"
			"fip.bin raw 180000 78000;"
			"optee.bin raw 500000 100000",
	.images = fw_images,
};

u8 num_image_type_guids = ARRAY_SIZE(fw_images);
#endif /* EFI_HAVE_CAPSULE_SUPPORT */

static struct mm_region sc2a11_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		/* 1st DDR block */
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = PHYS_SDRAM_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		/* 2nd DDR place holder */
		0,
	}, {
		/* 3rd DDR place holder */
		0,
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = sc2a11_mem_map;

#define DDR_REGION_INDEX(i)	(1 + (i))
#define MAX_DDR_REGIONS		3

struct draminfo_entry {
	u64	base;
	u64	size;
};

struct draminfo {
	u32	nr_regions;
	u32	reserved;
	struct draminfo_entry	entry[3];
};

struct draminfo *synquacer_draminfo = (void *)SQ_DRAMINFO_BASE;

DECLARE_GLOBAL_DATA_PTR;

#define LOAD_OFFSET 0x100

/* SCBM System MMU is used for eMMC and NETSEC */
#define SCBM_SMMU_ADDR				(0x52e00000UL)
#define SMMU_SCR0_OFFS				(0x0)
#define SMMU_SCR0_SHCFG_INNER			(0x2 << 22)
#define SMMU_SCR0_MTCFG				(0x1 << 20)
#define SMMU_SCR0_MEMATTR_INNER_OUTER_WB	(0xf << 16)

static void synquacer_setup_scbm_smmu(void)
{
	writel(SMMU_SCR0_SHCFG_INNER | SMMU_SCR0_MTCFG | SMMU_SCR0_MEMATTR_INNER_OUTER_WB,
	       SCBM_SMMU_ADDR + SMMU_SCR0_OFFS);
}

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_LOAD_ADDR + LOAD_OFFSET;

	gd->env_addr = (ulong)&default_environment[0];

	synquacer_setup_scbm_smmu();

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	/* Remove SPI NOR and I2C0 for making DT compatible with EDK2 */
	fdt_del_node_and_alias(blob, "spi_nor");
	fdt_del_node_and_alias(blob, "i2c0");

	return 0;
}

/*
 * DRAM configuration
 */

int dram_init(void)
{
	struct draminfo_entry *ent = synquacer_draminfo->entry;
	struct mm_region *mr;
	int i, ri;

	if (synquacer_draminfo->nr_regions < 1) {
		log_err("Failed to get correct DRAM information\n");
		return -1;
	}

	/*
	 * U-Boot RAM size must be under the first DRAM region so that it doesn't
	 * access secure memory which is at the end of the first DRAM region.
	 */
	gd->ram_size = ent[0].size;

	/* Update memory region maps */
	for (i = 0; i < synquacer_draminfo->nr_regions; i++) {
		if (i >= MAX_DDR_REGIONS)
			break;

		ri = DDR_REGION_INDEX(i);
		mem_map[ri].phys = ent[i].base;
		mem_map[ri].size = ent[i].size;
		if (i == 0)
			continue;

		mr = &mem_map[DDR_REGION_INDEX(0)];
		mem_map[ri].virt = mr->virt + mr->size;
		mem_map[ri].attrs = mr->attrs;
	}

	return 0;
}

int dram_init_banksize(void)
{
	struct draminfo_entry *ent = synquacer_draminfo->entry;
	int i;

	for (i = 0; i < ARRAY_SIZE(gd->bd->bi_dram); i++) {
		if (i < synquacer_draminfo->nr_regions) {
			debug("%s: dram[%d] = %llx@%llx\n", __func__, i, ent[i].size, ent[i].base);
			gd->bd->bi_dram[i].start = ent[i].base;
			gd->bd->bi_dram[i].size = ent[i].size;
		}
	}

	return 0;
}

int print_cpuinfo(void)
{
	printf("CPU:   SC2A11:Cortex-A53 MPCore 24cores\n");
	return 0;
}
