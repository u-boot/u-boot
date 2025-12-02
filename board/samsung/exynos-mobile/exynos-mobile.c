// SPDX-License-Identifier: GPL-2.0
/*
 * Samsung Exynos Generic Board Source (for mobile devices)
 *
 * Copyright (c) 2025 Kaustabh Chakraborty <kauschluss@disroot.org>
 */

#include <asm/armv8/mmu.h>
#include <blk.h>
#include <bootflow.h>
#include <ctype.h>
#include <dm/ofnode.h>
#include <env.h>
#include <errno.h>
#include <init.h>
#include <limits.h>
#include <linux/sizes.h>
#include <lmb.h>
#include <part.h>
#include <stdbool.h>

DECLARE_GLOBAL_DATA_PTR;

#define lmb_alloc(size, addr) \
	lmb_alloc_mem(LMB_MEM_ALLOC_ANY, SZ_2M, addr, size, LMB_NONE)

struct exynos_board_info {
	const char *name;
	const char *chip;
	const u64 *const dram_bank_bases;

	char serial[64];

	int (*const match)(struct exynos_board_info *);
	const char *match_model;
	const u8 match_max_rev;
};

/*
 * The memory mapping includes all DRAM banks, along with the
 * peripheral block, and a sentinel at the end. This is filled in
 * dynamically.
 */
static struct mm_region exynos_mem_map[CONFIG_NR_DRAM_BANKS + 2] = {
	{
		/* Peripheral MMIO block */
		.virt = 0x10000000UL,
		.phys = 0x10000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	},
};

struct mm_region *mem_map = exynos_mem_map;

static const u64 exynos7870_common_dram_bank_bases[CONFIG_NR_DRAM_BANKS] = {
	0x40000000, 0x80000000, 0x100000000,
};

static const char *exynos_prev_bl_get_bootargs(void)
{
	void *prev_bl_fdt_base = (void *)get_prev_bl_fdt_addr();
	int chosen_node_offset, ret;
	const struct fdt_property *bootargs_prop;

	ret = fdt_check_header(prev_bl_fdt_base);
	if (ret < 0) {
		log_err("%s: FDT is invalid (FDT_ERR %d)\n", __func__, ret);
		return NULL;
	}

	ret = fdt_path_offset(prev_bl_fdt_base, "/chosen");
	chosen_node_offset = ret;
	if (ret < 0) {
		log_err("%s: /chosen node not found (FDT_ERR %d)\n", __func__,
			ret);
		return NULL;
	}

	bootargs_prop = fdt_get_property(prev_bl_fdt_base, chosen_node_offset,
					 "bootargs", &ret);
	if (!bootargs_prop) {
		log_err("%s: /chosen/bootargs property not found (FDT_ERR %d)\n",
			__func__, ret);
		return NULL;
	}

	return bootargs_prop->data;
}

static int exynos7870_fdt_match(struct exynos_board_info *board_info)
{
	const char *prev_bl_bootargs;
	int val, ret;

	prev_bl_bootargs = exynos_prev_bl_get_bootargs();
	if (!prev_bl_bootargs)
		return -1;

	/*
	 * Read the cmdline property which stores the
	 * bootloader/firmware version. An example value of the option
	 * can be: "J600GDXU3ARH5". This can be used to verify the model
	 * of the device.
	 */
	ret = cmdline_get_arg(prev_bl_bootargs, "androidboot.bootloader", &val);
	if (ret < 0) {
		log_err("%s: unable to find property for bootloader version (%d)\n",
			__func__, ret);
		return -1;
	}

	if (strncmp(prev_bl_bootargs + val, board_info->match_model,
		    strlen(board_info->match_model)))
		return -1;

	/*
	 * Read the cmdline property which stores the hardware revision.
	 * This is required to allow selecting one of multiple dtbs
	 * available of a single device, varying in hardware changes in
	 * different revisions.
	 */
	ret = cmdline_get_arg(prev_bl_bootargs, "androidboot.revision", &val);
	if (ret < 0)
		ret = cmdline_get_arg(prev_bl_bootargs, "androidboot.hw_rev", &val);
	if (ret < 0) {
		log_err("%s: unable to find property for bootloader revision (%d)\n",
			__func__, ret);
		return -1;
	}

	if (strtoul(prev_bl_bootargs + val, NULL, 10) > board_info->match_max_rev)
		return -1;

	/*
	 * Read the cmdline property which stores the serial number.
	 * Store this in the board info struct.
	 */
	ret = cmdline_get_arg(prev_bl_bootargs, "androidboot.serialno", &val);
	if (ret > 0)
		strlcpy(board_info->serial, prev_bl_bootargs + val, ret);

	return 0;
}

/*
 * This array is used for matching the models and revisions with the
 * devicetree used by U-Boot. This allows a single U-Boot to work on
 * multiple devices.
 *
 * Entries are kept in lexicographical order of board SoCs, followed by
 * board names.
 */
static struct exynos_board_info exynos_board_info_match[] = {
	{
		/* Samsung Galaxy A2 Core */
		.name = "a2corelte",
		.chip = "exynos7870",
		.dram_bank_bases = exynos7870_common_dram_bank_bases,
		.match = exynos7870_fdt_match,
		.match_model = "A260",
		.match_max_rev = U8_MAX,
	}, {
		/* Samsung Galaxy J6 */
		.name = "j6lte",
		.chip = "exynos7870",
		.dram_bank_bases = exynos7870_common_dram_bank_bases,
		.match = exynos7870_fdt_match,
		.match_model = "J600",
		.match_max_rev = U8_MAX,
	}, {
		/* Samsung Galaxy J7 Prime */
		.name = "on7xelte",
		.chip = "exynos7870",
		.dram_bank_bases = exynos7870_common_dram_bank_bases,
		.match = exynos7870_fdt_match,
		.match_model = "G610",
		.match_max_rev = U8_MAX,
	},
};

static void exynos_parse_dram_banks(const struct exynos_board_info *board_info,
				    const void *fdt_base)
{
	u64 mem_addr, mem_size = 0;
	u32 na, ns, i, j;
	int offset;

	if (fdt_check_header(fdt_base) < 0)
		return;

	/* #address-cells and #size-cells as defined in the fdt root. */
	na = fdt_address_cells(fdt_base, 0);
	ns = fdt_size_cells(fdt_base, 0);

	fdt_for_each_subnode(offset, fdt_base, 0) {
		if (strncmp(fdt_get_name(fdt_base, offset, NULL), "memory", 6))
			continue;

		for (i = 0; ; i++) {
			mem_addr = fdtdec_get_addr_size_fixed(fdt_base, offset,
							      "reg", i, na, ns,
							      &mem_size, false);
			if (mem_addr == FDT_ADDR_T_NONE)
				break;

			if (!mem_size)
				continue;

			for (j = 0; j < CONFIG_NR_DRAM_BANKS; j++) {
				if (board_info->dram_bank_bases[j] != mem_addr)
					continue;

				mem_map[j + 1].phys = mem_addr;
				mem_map[j + 1].virt = mem_addr;
				mem_map[j + 1].size = mem_size;
				mem_map[j + 1].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
						       PTE_BLOCK_INNER_SHARE;
				break;
			}
		}
	}
}

static int exynos_fastboot_setup(void)
{
	struct blk_desc *blk_dev;
	struct disk_partition info = {0};
	char buf[128];
	phys_addr_t addr;
	int offset, i, j;

	/* Allocate and define buffer address for fastboot interface. */
	if (lmb_alloc(CONFIG_FASTBOOT_BUF_SIZE, &addr)) {
		log_err("%s: failed to allocate fastboot buffer\n", __func__);
		return -ENOMEM;
	}
	env_set_hex("fastboot_addr_r", addr);

	blk_dev = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
	if (!blk_dev) {
		log_err("%s: required mmc device not available\n", __func__);
		return -ENODEV;
	}

	strcpy(buf, "fastboot_partition_alias_");
	offset = strlen(buf);

	for (i = 1; i < CONFIG_EFI_PARTITION_ENTRIES_NUMBERS; i++) {
		if (part_get_info(blk_dev, i, &info))
			continue;

		/*
		 * The partition name must be lowercase (stored in buf[]),
		 * as is expected in all fastboot partitions ...
		 */
		strlcpy(buf + offset, info.name, sizeof(buf) - offset);
		for (j = offset; buf[j]; j++)
			buf[j] = tolower(buf[j]);
		if (!strcmp(buf + offset, info.name))
			continue;
		/*
		 * ... However, if that isn't the case, a fastboot
		 * partition alias must be defined to establish it.
		 */
		env_set(buf, info.name);
	}

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	struct exynos_board_info *board_info;
	char buf[128];
	unsigned int i;
	int ret;

	/*
	 * Iterate over exynos_board_info_match[] to select the
	 * appropriate board info struct. If not found, exit.
	 */
	for (i = 0; i < ARRAY_SIZE(exynos_board_info_match); i++) {
		board_info = exynos_board_info_match + i;
		snprintf(buf, sizeof(buf), "%s-%s", board_info->chip,
			 board_info->name);

		if (!strcmp(name, buf))
			break;
	}
	if (i == ARRAY_SIZE(exynos_board_info_match))
		return -1;

	/*
	 * Execute match logic for the target board. This is separated
	 * as the process may be different for multiple boards.
	 */
	ret = board_info->match(board_info);
	if (ret)
		return ret;

	/*
	 * Store the correct board info struct in gd->board_type to
	 * allow other functions to access it.
	 */
	gd->board_type = (ulong)board_info;
	log_debug("%s: device detected: %s\n", __func__, name);

	return 0;
}

int timer_init(void)
{
	ofnode timer_node;

	/*
	 * In a lot of Exynos devices, the previous bootloader does not
	 * set CNTFRQ_EL0 properly. However, the timer node in
	 * devicetree has the correct frequency, use that instead.
	 */
	timer_node = ofnode_by_compatible(ofnode_null(), "arm,armv8-timer");
	gd->arch.timer_rate_hz = ofnode_read_u32_default(timer_node,
							 "clock-frequency", 0);

	return 0;
}

int board_early_init_f(void)
{
	const struct exynos_board_info *board_info;

	if (!gd->board_type)
		return -ENODATA;
	board_info = (const struct exynos_board_info *)gd->board_type;

	exynos_parse_dram_banks(board_info, gd->fdt_blob);
	/*
	 * Some devices have multiple variants based on the amount of
	 * memory and internal storage. The lowest bank base has been
	 * observed to have the same memory range in all board variants.
	 * For variants with more memory, the previous bootloader should
	 * overlay the devicetree with the required extra memory ranges.
	 */
	exynos_parse_dram_banks(board_info, (const void *)get_prev_bl_fdt_addr());

	return 0;
}

int dram_init(void)
{
	unsigned int i;

	/* Select the largest RAM bank for U-Boot. */
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		if (gd->ram_size < mem_map[i + 1].size) {
			gd->ram_base = mem_map[i + 1].phys;
			gd->ram_size = mem_map[i + 1].size;
		}
	}

	return 0;
}

int dram_init_banksize(void)
{
	unsigned int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->bd->bi_dram[i].start = mem_map[i + 1].phys;
		gd->bd->bi_dram[i].size = mem_map[i + 1].size;
	}

	return 0;
}

int board_init(void)
{
	return 0;
}

int misc_init_r(void)
{
	const struct exynos_board_info *board_info;
	char buf[128];

	if (!gd->board_type)
		return -ENODATA;
	board_info = (const struct exynos_board_info *)gd->board_type;

	env_set("platform", board_info->chip);
	env_set("board", board_info->name);

	if (strlen(board_info->serial))
		env_set("serial#", board_info->serial);

	/* EFI booting requires the path to correct dtb, specify it here. */
	snprintf(buf, sizeof(buf), "exynos/%s-%s.dtb", board_info->chip,
		 board_info->name);
	env_set("fdtfile", buf);

	return exynos_fastboot_setup();
}
