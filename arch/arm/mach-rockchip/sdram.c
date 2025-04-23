// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 */

#include <config.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch-rockchip/sdram.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

#define TRUST_PARAMETER_OFFSET    (34 * 1024 * 1024)

struct tos_parameter_t {
	u32 version;
	u32 checksum;
	struct {
		char name[8];
		s64 phy_addr;
		u32 size;
		u32 flags;
	} tee_mem;
	struct {
		char name[8];
		s64 phy_addr;
		u32 size;
		u32 flags;
	} drm_mem;
	s64 reserve[8];
};

#ifdef CONFIG_ARM64
/* Tag size and offset */
#define ATAGS_SIZE		SZ_8K
#define ATAGS_OFFSET		(SZ_2M - ATAGS_SIZE)
#define ATAGS_PHYS_BASE		(CFG_SYS_SDRAM_BASE + ATAGS_OFFSET)
#define ATAGS_PHYS_END		(ATAGS_PHYS_BASE + ATAGS_SIZE)

/* ATAGS memory structures */

enum tag_magic {
	ATAG_NONE,
	ATAG_CORE = 0x54410001,
	ATAG_SERIAL = 0x54410050,
	ATAG_DDR_MEM = 0x54410052,
	ATAG_MAX = 0x544100ff,
};

/*
 * An ATAG contains the following data:
 *  - header
 *    u32 size // sizeof(header + tag data) / sizeof(u32)
 *    u32 magic
 *  - tag data
 */

struct tag_header {
	u32 size;
	u32 magic;
} __packed;

/*
 * DDR_MEM tag bank is storing data this way:
 *  - address0
 *  - address1
 *  - [...]
 *  - addressX
 *  - size0
 *  - size1
 *  - [...]
 *  - sizeX
 *
 *  with X being tag_ddr_mem.count - 1.
 */
struct tag_ddr_mem {
	u32 count;
	u32 version;
	u64 bank[20];
	u32 flags;
	u32 data[2];
	u32 hash;
} __packed;

static u32 js_hash(const void *buf, u32 len)
{
	u32 i, hash = 0x47C6A7E6;

	if (!buf || !len)
		return hash;

	for (i = 0; i < len; i++)
		hash ^= ((hash << 5) + ((const char *)buf)[i] + (hash >> 2));

	return hash;
}

static int rockchip_dram_init_banksize(void)
{
	const struct tag_header *tag_h = NULL;
	u32 *addr = (void *)ATAGS_PHYS_BASE;
	struct tag_ddr_mem *ddr_info;
	u32 calc_hash;
	u8 i, j;

	if (!IS_ENABLED(CONFIG_ROCKCHIP_RK3588) &&
	    !IS_ENABLED(CONFIG_ROCKCHIP_RK3576) &&
	    !IS_ENABLED(CONFIG_ROCKCHIP_RK3568) &&
	    !IS_ENABLED(CONFIG_ROCKCHIP_RK3528))
		return -ENOTSUPP;

	if (!IS_ENABLED(CONFIG_ROCKCHIP_EXTERNAL_TPL))
		return -ENOTSUPP;

	/* Find DDR_MEM tag */
	while (addr < (u32 *)ATAGS_PHYS_END) {
		tag_h = (const struct tag_header *)addr;

		if (!tag_h->size) {
			debug("End of ATAGS (0-size tag), no DDR_MEM found\n");
			return -ENODATA;
		}

		if (tag_h->magic == ATAG_DDR_MEM)
			break;

		switch (tag_h->magic) {
		case ATAG_NONE:
		case ATAG_CORE:
		case ATAG_SERIAL ... ATAG_MAX:
			addr += tag_h->size;
			continue;
		default:
			debug("Invalid magic (0x%08x) for ATAG at 0x%p\n",
			      tag_h->magic, addr);
			return -EINVAL;
		}
	}

	if (addr >= (u32 *)ATAGS_PHYS_END ||
	    (tag_h && (addr + tag_h->size > (u32 *)ATAGS_PHYS_END))) {
		debug("End of ATAGS, no DDR_MEM found\n");
		return -ENODATA;
	}

	/* Data is right after the magic member of the tag_header struct */
	ddr_info = (struct tag_ddr_mem *)(&tag_h->magic + 1);
	if (!ddr_info->count || ddr_info->count > CONFIG_NR_DRAM_BANKS) {
		debug("Too many ATAG banks, got (%d) but max allowed (%d)\n",
		      ddr_info->count, CONFIG_NR_DRAM_BANKS);
		return -ENOMEM;
	}

	if (!ddr_info->hash) {
		debug("No hash for tag (0x%08x)\n", tag_h->magic);
	} else {
		calc_hash = js_hash(addr, sizeof(u32) * (tag_h->size - 1));

		if (calc_hash != ddr_info->hash) {
			debug("Incorrect hash for tag (0x%08x), got (0x%08x) expected (0x%08x)\n",
			      tag_h->magic, ddr_info->hash, calc_hash);
			return -EINVAL;
		}
	}

	/*
	 * Rockchip guaranteed DDR_MEM is ordered so no need to worry about
	 * bi_dram order.
	 */
	for (i = 0, j = 0; i < ddr_info->count; i++, j++) {
		phys_size_t size = ddr_info->bank[(i + ddr_info->count)];
		phys_addr_t start_addr = ddr_info->bank[i];
		struct mm_region *tmp_mem_map = mem_map;
		phys_addr_t end_addr;

		/*
		 * BL31 (TF-A) reserves the first 2MB but DDR_MEM tag may not
		 * have it, so force this space as reserved.
		 */
		if (start_addr < CFG_SYS_SDRAM_BASE + SZ_2M) {
			size -= CFG_SYS_SDRAM_BASE + SZ_2M - start_addr;
			start_addr = CFG_SYS_SDRAM_BASE + SZ_2M;
		}

		/*
		 * Put holes for reserved memory areas from mem_map.
		 *
		 * Only check for at most one overlap with one reserved memory
		 * area.
		 */
		while (tmp_mem_map->size) {
			const phys_addr_t rsrv_start = tmp_mem_map->phys;
			const phys_size_t rsrv_size = tmp_mem_map->size;
			const phys_addr_t rsrv_end = rsrv_start + rsrv_size;

			/*
			 * DRAM memories are expected by Arm to be marked as
			 * Normal Write-back cacheable, Inner shareable[1], so
			 * let's filter on that to put holes in non-DRAM areas.
			 *
			 * [1] https://developer.arm.com/documentation/102376/0200/Cacheability-and-shareability-attributes
			 */
			const u64 dram_attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE;
			/*
			 * (AttrIndx | SH) in Lower Attributes of Block
			 * Descriptor[2].
			 * [2] https://developer.arm.com/documentation/102376/0200/Describing-memory-in-AArch64
			 */
			const u64 attrs_mask = PMD_ATTRINDX_MASK | GENMASK(9, 8);

			if ((tmp_mem_map->attrs & attrs_mask) == dram_attrs) {
				tmp_mem_map++;
				continue;
			}

			/*
			 * If the start of the DDR_MEM tag is in a reserved
			 * memory area, move start address and resize.
			 */
			if (start_addr >= rsrv_start && start_addr < rsrv_end) {
				if (rsrv_end - start_addr > size) {
					debug("Would be negative memory size\n");
					return -EINVAL;
				}

				size -= rsrv_end - (start_addr - CFG_SYS_SDRAM_BASE);
				start_addr = rsrv_end;
				break;
			}

			if (start_addr < rsrv_start) {
				end_addr = start_addr + size;

				if (end_addr <= rsrv_start) {
					tmp_mem_map++;
					continue;
				}

				/*
				 * If the memory area overlaps a reserved memory
				 * area with start address outside of reserved
				 * memory area and...
				 *
				 * ... ends in the middle of reserved memory
				 * area, resize.
				 */
				if (end_addr <= rsrv_end) {
					size = rsrv_start - start_addr;
					break;
				}

				/*
				 * ... ends after the reserved memory area,
				 * split the region in two, one for before the
				 * reserved memory area and one for after.
				 */
				gd->bd->bi_dram[j].start = start_addr;
				gd->bd->bi_dram[j].size = rsrv_start - start_addr;

				j++;

				size = end_addr - rsrv_end;
				start_addr = rsrv_end;

				break;
			}

			tmp_mem_map++;
		}

		if (j > CONFIG_NR_DRAM_BANKS) {
			debug("Too many banks, max allowed (%d)\n",
			      CONFIG_NR_DRAM_BANKS);
			return -ENOMEM;
		}

		gd->bd->bi_dram[j].start = start_addr;
		gd->bd->bi_dram[j].size = size;
	}

	return 0;
}
#endif

int dram_init_banksize(void)
{
	size_t ram_top = (unsigned long)(gd->ram_size + CFG_SYS_SDRAM_BASE);
	size_t top = min((unsigned long)ram_top, (unsigned long)(gd->ram_top));

#ifdef CONFIG_ARM64
	int ret = rockchip_dram_init_banksize();

	if (!ret)
		return ret;

	debug("Couldn't use ATAG (%d) to detect DDR layout, falling back...\n",
	      ret);

	/* Reserve 2M for ATF bl31 */
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE + SZ_2M;
	gd->bd->bi_dram[0].size = top - gd->bd->bi_dram[0].start;

	/* Add usable memory beyond the blob of space for peripheral near 4GB */
	if (ram_top > SZ_4G && top < SZ_4G) {
		gd->bd->bi_dram[1].start = SZ_4G;
		gd->bd->bi_dram[1].size = ram_top - gd->bd->bi_dram[1].start;
	} else if (ram_top > SZ_4G && top == SZ_4G) {
		gd->bd->bi_dram[0].size = ram_top - gd->bd->bi_dram[0].start;
	}
#else
#ifdef CONFIG_SPL_OPTEE_IMAGE
	struct tos_parameter_t *tos_parameter;

	tos_parameter = (struct tos_parameter_t *)(CFG_SYS_SDRAM_BASE +
			TRUST_PARAMETER_OFFSET);

	if (tos_parameter->tee_mem.flags == 1) {
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = tos_parameter->tee_mem.phy_addr
					- CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[1].start = tos_parameter->tee_mem.phy_addr +
					tos_parameter->tee_mem.size;
		gd->bd->bi_dram[1].size = top - gd->bd->bi_dram[1].start;
	} else {
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = 0x8400000;
		/* Reserve 32M for OPTEE with TA */
		gd->bd->bi_dram[1].start = CFG_SYS_SDRAM_BASE
					+ gd->bd->bi_dram[0].size + 0x2000000;
		gd->bd->bi_dram[1].size = top - gd->bd->bi_dram[1].start;
	}
#else
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = top - gd->bd->bi_dram[0].start;
#endif
#endif

	return 0;
}

size_t rockchip_sdram_size(phys_addr_t reg)
{
	u32 rank, cs0_col, bk, cs0_row, cs1_row, bw, row_3_4;
	size_t chipsize_mb = 0;
	size_t size_mb = 0;
	u32 ch;
	u32 cs1_col = 0;
	u32 bg = 0;
	u32 dbw, dram_type;
	u32 sys_reg2 = readl(reg);
	u32 sys_reg3 = readl(reg + 4);
	u32 ch_num = 1 + ((sys_reg2 >> SYS_REG_NUM_CH_SHIFT)
		       & SYS_REG_NUM_CH_MASK);
	u32 version = (sys_reg3 >> SYS_REG_VERSION_SHIFT) &
		      SYS_REG_VERSION_MASK;

	dram_type = (sys_reg2 >> SYS_REG_DDRTYPE_SHIFT) & SYS_REG_DDRTYPE_MASK;
	if (version >= 3)
		dram_type |= ((sys_reg3 >> SYS_REG_EXTEND_DDRTYPE_SHIFT) &
			      SYS_REG_EXTEND_DDRTYPE_MASK) << 3;
	debug("%s %x %x\n", __func__, (u32)reg, sys_reg2);
	debug("%s %x %x\n", __func__, (u32)reg + 4, sys_reg3);
	for (ch = 0; ch < ch_num; ch++) {
		rank = 1 + (sys_reg2 >> SYS_REG_RANK_SHIFT(ch) &
			SYS_REG_RANK_MASK);
		cs0_col = 9 + (sys_reg2 >> SYS_REG_COL_SHIFT(ch) &
			  SYS_REG_COL_MASK);
		cs1_col = cs0_col;
		if (dram_type == LPDDR5)
			/* LPDDR5: 0:8bank(bk=3), 1:16bank(bk=4) */
			bk = 3 + ((sys_reg2 >> SYS_REG_BK_SHIFT(ch)) &
			SYS_REG_BK_MASK);
		else
			/* Other: 0:8bank(bk=3), 1:4bank(bk=2) */
			bk = 3 - ((sys_reg2 >> SYS_REG_BK_SHIFT(ch)) &
			SYS_REG_BK_MASK);
		if (version >= 2) {
			cs1_col = 9 + (sys_reg3 >> SYS_REG_CS1_COL_SHIFT(ch) &
				  SYS_REG_CS1_COL_MASK);
			if (((sys_reg3 >> SYS_REG_EXTEND_CS0_ROW_SHIFT(ch) &
			    SYS_REG_EXTEND_CS0_ROW_MASK) << 2) + (sys_reg2 >>
			    SYS_REG_CS0_ROW_SHIFT(ch) &
			    SYS_REG_CS0_ROW_MASK) == 7)
				cs0_row = 12;
			else
				cs0_row = 13 + (sys_reg2 >>
					  SYS_REG_CS0_ROW_SHIFT(ch) &
					  SYS_REG_CS0_ROW_MASK) +
					  ((sys_reg3 >>
					  SYS_REG_EXTEND_CS0_ROW_SHIFT(ch) &
					  SYS_REG_EXTEND_CS0_ROW_MASK) << 2);
			if (((sys_reg3 >> SYS_REG_EXTEND_CS1_ROW_SHIFT(ch) &
			    SYS_REG_EXTEND_CS1_ROW_MASK) << 2) + (sys_reg2 >>
			    SYS_REG_CS1_ROW_SHIFT(ch) &
			    SYS_REG_CS1_ROW_MASK) == 7)
				cs1_row = 12;
			else
				cs1_row = 13 + (sys_reg2 >>
					  SYS_REG_CS1_ROW_SHIFT(ch) &
					  SYS_REG_CS1_ROW_MASK) +
					  ((sys_reg3 >>
					  SYS_REG_EXTEND_CS1_ROW_SHIFT(ch) &
					  SYS_REG_EXTEND_CS1_ROW_MASK) << 2);
		} else {
			cs0_row = 13 + (sys_reg2 >> SYS_REG_CS0_ROW_SHIFT(ch) &
				SYS_REG_CS0_ROW_MASK);
			cs1_row = 13 + (sys_reg2 >> SYS_REG_CS1_ROW_SHIFT(ch) &
				SYS_REG_CS1_ROW_MASK);
		}
		bw = (2 >> ((sys_reg2 >> SYS_REG_BW_SHIFT(ch)) &
			SYS_REG_BW_MASK));
		row_3_4 = sys_reg2 >> SYS_REG_ROW_3_4_SHIFT(ch) &
			SYS_REG_ROW_3_4_MASK;
		if (dram_type == DDR4) {
			dbw = (sys_reg2 >> SYS_REG_DBW_SHIFT(ch)) &
				SYS_REG_DBW_MASK;
			bg = (dbw == 2) ? 2 : 1;
		}
		chipsize_mb = (1 << (cs0_row + cs0_col + bk + bg + bw - 20));

		if (rank > 1)
			chipsize_mb += chipsize_mb >> ((cs0_row - cs1_row) +
				       (cs0_col - cs1_col));
		if (row_3_4)
			chipsize_mb = chipsize_mb * 3 / 4;
		size_mb += chipsize_mb;
		if (rank > 1)
			debug("rank %d cs0_col %d cs1_col %d bk %d cs0_row %d\
			       cs1_row %d bw %d row_3_4 %d\n",
			       rank, cs0_col, cs1_col, bk, cs0_row,
			       cs1_row, bw, row_3_4);
		else
			debug("rank %d cs0_col %d bk %d cs0_row %d\
			       bw %d row_3_4 %d\n",
			       rank, cs0_col, bk, cs0_row,
			       bw, row_3_4);
	}

	/*
	 * This is workaround for issue we can't get correct size for 4GB ram
	 * in 32bit system and available before we really need ram space
	 * out of 4GB, eg.enable ARM LAPE(rk3288 supports 8GB ram).
	 * The size of 4GB is '0x1 00000000', and this value will be truncated
	 * to 0 in 32bit system, and system can not get correct ram size.
	 * Rockchip SoCs reserve a blob of space for peripheral near 4GB,
	 * and we are now setting SDRAM_MAX_SIZE as max available space for
	 * ram in 4GB, so we can use this directly to workaround the issue.
	 * TODO:
	 *   1. update correct value for SDRAM_MAX_SIZE as what dram
	 *   controller sees.
	 *   2. update board_get_usable_ram_top() and dram_init_banksize()
	 *   to reserve memory for peripheral space after previous update.
	 */
	if (!IS_ENABLED(CONFIG_ARM64) && size_mb > (SDRAM_MAX_SIZE >> 20))
		size_mb = (SDRAM_MAX_SIZE >> 20);

	return (size_t)size_mb << 20;
}

int dram_init(void)
{
	struct ram_info ram;
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return ret;
	}
	ret = ram_get_info(dev, &ram);
	if (ret) {
		debug("Cannot get DRAM size: %d\n", ret);
		return ret;
	}
	gd->ram_base = ram.base;
	gd->ram_size = ram.size;
	debug("SDRAM base=%lx, size=%lx\n",
	      (unsigned long)ram.base, (unsigned long)ram.size);

	return 0;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	/* Make sure U-Boot only uses the space below the 4G address boundary */
	u64 top = min_t(u64, CFG_SYS_SDRAM_BASE + SDRAM_MAX_SIZE, SZ_4G);

	return (gd->ram_top > top) ? top : gd->ram_top;
}
