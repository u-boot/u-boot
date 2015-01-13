/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>
#ifdef CONFIG_PPC
#include <asm/fsl_law.h>
#endif
#include <div64.h>

#include <fsl_ddr.h>
#include <fsl_immap.h>
#include <asm/io.h>

/* To avoid 64-bit full-divides, we factor this here */
#define ULL_2E12 2000000000000ULL
#define UL_5POW12 244140625UL
#define UL_2POW13 (1UL << 13)

#define ULL_8FS 0xFFFFFFFFULL

u32 fsl_ddr_get_version(void)
{
	struct ccsr_ddr __iomem *ddr;
	u32 ver_major_minor_errata;

	ddr = (void *)_DDR_ADDR;
	ver_major_minor_errata = (ddr_in32(&ddr->ip_rev1) & 0xFFFF) << 8;
	ver_major_minor_errata |= (ddr_in32(&ddr->ip_rev2) & 0xFF00) >> 8;

	return ver_major_minor_errata;
}

/*
 * Round up mclk_ps to nearest 1 ps in memory controller code
 * if the error is 0.5ps or more.
 *
 * If an imprecise data rate is too high due to rounding error
 * propagation, compute a suitably rounded mclk_ps to compute
 * a working memory controller configuration.
 */
unsigned int get_memory_clk_period_ps(void)
{
	unsigned int data_rate = get_ddr_freq(0);
	unsigned int result;

	/* Round to nearest 10ps, being careful about 64-bit multiply/divide */
	unsigned long long rem, mclk_ps = ULL_2E12;

	/* Now perform the big divide, the result fits in 32-bits */
	rem = do_div(mclk_ps, data_rate);
	result = (rem >= (data_rate >> 1)) ? mclk_ps + 1 : mclk_ps;

	return result;
}

/* Convert picoseconds into DRAM clock cycles (rounding up if needed). */
unsigned int picos_to_mclk(unsigned int picos)
{
	unsigned long long clks, clks_rem;
	unsigned long data_rate = get_ddr_freq(0);

	/* Short circuit for zero picos */
	if (!picos)
		return 0;

	/* First multiply the time by the data rate (32x32 => 64) */
	clks = picos * (unsigned long long)data_rate;
	/*
	 * Now divide by 5^12 and track the 32-bit remainder, then divide
	 * by 2*(2^12) using shifts (and updating the remainder).
	 */
	clks_rem = do_div(clks, UL_5POW12);
	clks_rem += (clks & (UL_2POW13-1)) * UL_5POW12;
	clks >>= 13;

	/* If we had a remainder greater than the 1ps error, then round up */
	if (clks_rem > data_rate)
		clks++;

	/* Clamp to the maximum representable value */
	if (clks > ULL_8FS)
		clks = ULL_8FS;
	return (unsigned int) clks;
}

unsigned int mclk_to_picos(unsigned int mclk)
{
	return get_memory_clk_period_ps() * mclk;
}

#ifdef CONFIG_PPC
void
__fsl_ddr_set_lawbar(const common_timing_params_t *memctl_common_params,
			   unsigned int law_memctl,
			   unsigned int ctrl_num)
{
	unsigned long long base = memctl_common_params->base_address;
	unsigned long long size = memctl_common_params->total_mem;

	/*
	 * If no DIMMs on this controller, do not proceed any further.
	 */
	if (!memctl_common_params->ndimms_present) {
		return;
	}

#if !defined(CONFIG_PHYS_64BIT)
	if (base >= CONFIG_MAX_MEM_MAPPED)
		return;
	if ((base + size) >= CONFIG_MAX_MEM_MAPPED)
		size = CONFIG_MAX_MEM_MAPPED - base;
#endif
	if (set_ddr_laws(base, size, law_memctl) < 0) {
		printf("%s: ERROR (ctrl #%d, TRGT ID=%x)\n", __func__, ctrl_num,
			law_memctl);
		return ;
	}
	debug("setup ddr law base = 0x%llx, size 0x%llx, TRGT_ID 0x%x\n",
		base, size, law_memctl);
}

__attribute__((weak, alias("__fsl_ddr_set_lawbar"))) void
fsl_ddr_set_lawbar(const common_timing_params_t *memctl_common_params,
			 unsigned int memctl_interleaved,
			 unsigned int ctrl_num);
#endif

void fsl_ddr_set_intl3r(const unsigned int granule_size)
{
#ifdef CONFIG_E6500
	u32 *mcintl3r = (void *) (CONFIG_SYS_IMMR + 0x18004);
	*mcintl3r = 0x80000000 | (granule_size & 0x1f);
	debug("Enable MCINTL3R with granule size 0x%x\n", granule_size);
#endif
}

u32 fsl_ddr_get_intl3r(void)
{
	u32 val = 0;
#ifdef CONFIG_E6500
	u32 *mcintl3r = (void *) (CONFIG_SYS_IMMR + 0x18004);
	val = *mcintl3r;
#endif
	return val;
}

void print_ddr_info(unsigned int start_ctrl)
{
	struct ccsr_ddr __iomem *ddr =
		(struct ccsr_ddr __iomem *)(CONFIG_SYS_FSL_DDR_ADDR);

#if	defined(CONFIG_E6500) && (CONFIG_NUM_DDR_CONTROLLERS == 3)
	u32 *mcintl3r = (void *) (CONFIG_SYS_IMMR + 0x18004);
#endif
#if (CONFIG_NUM_DDR_CONTROLLERS > 1)
	uint32_t cs0_config = ddr_in32(&ddr->cs0_config);
#endif
	uint32_t sdram_cfg = ddr_in32(&ddr->sdram_cfg);
	int cas_lat;

#if CONFIG_NUM_DDR_CONTROLLERS >= 2
	if ((!(sdram_cfg & SDRAM_CFG_MEM_EN)) ||
	    (start_ctrl == 1)) {
		ddr = (void __iomem *)CONFIG_SYS_FSL_DDR2_ADDR;
		sdram_cfg = ddr_in32(&ddr->sdram_cfg);
	}
#endif
#if CONFIG_NUM_DDR_CONTROLLERS >= 3
	if ((!(sdram_cfg & SDRAM_CFG_MEM_EN)) ||
	    (start_ctrl == 2)) {
		ddr = (void __iomem *)CONFIG_SYS_FSL_DDR3_ADDR;
		sdram_cfg = ddr_in32(&ddr->sdram_cfg);
	}
#endif

	if (!(sdram_cfg & SDRAM_CFG_MEM_EN)) {
		puts(" (DDR not enabled)\n");
		return;
	}

	puts(" (DDR");
	switch ((sdram_cfg & SDRAM_CFG_SDRAM_TYPE_MASK) >>
		SDRAM_CFG_SDRAM_TYPE_SHIFT) {
	case SDRAM_TYPE_DDR1:
		puts("1");
		break;
	case SDRAM_TYPE_DDR2:
		puts("2");
		break;
	case SDRAM_TYPE_DDR3:
		puts("3");
		break;
	case SDRAM_TYPE_DDR4:
		puts("4");
		break;
	default:
		puts("?");
		break;
	}

	if (sdram_cfg & SDRAM_CFG_32_BE)
		puts(", 32-bit");
	else if (sdram_cfg & SDRAM_CFG_16_BE)
		puts(", 16-bit");
	else
		puts(", 64-bit");

	/* Calculate CAS latency based on timing cfg values */
	cas_lat = ((ddr_in32(&ddr->timing_cfg_1) >> 16) & 0xf);
	if (fsl_ddr_get_version() <= 0x40400)
		cas_lat += 1;
	else
		cas_lat += 2;
	cas_lat += ((ddr_in32(&ddr->timing_cfg_3) >> 12) & 3) << 4;
	printf(", CL=%d", cas_lat >> 1);
	if (cas_lat & 0x1)
		puts(".5");

	if (sdram_cfg & SDRAM_CFG_ECC_EN)
		puts(", ECC on)");
	else
		puts(", ECC off)");

#if (CONFIG_NUM_DDR_CONTROLLERS == 3)
#ifdef CONFIG_E6500
	if (*mcintl3r & 0x80000000) {
		puts("\n");
		puts("       DDR Controller Interleaving Mode: ");
		switch (*mcintl3r & 0x1f) {
		case FSL_DDR_3WAY_1KB_INTERLEAVING:
			puts("3-way 1KB");
			break;
		case FSL_DDR_3WAY_4KB_INTERLEAVING:
			puts("3-way 4KB");
			break;
		case FSL_DDR_3WAY_8KB_INTERLEAVING:
			puts("3-way 8KB");
			break;
		default:
			puts("3-way UNKNOWN");
			break;
		}
	}
#endif
#endif
#if (CONFIG_NUM_DDR_CONTROLLERS >= 2)
	if ((cs0_config & 0x20000000) && (start_ctrl == 0)) {
		puts("\n");
		puts("       DDR Controller Interleaving Mode: ");

		switch ((cs0_config >> 24) & 0xf) {
		case FSL_DDR_256B_INTERLEAVING:
			puts("256B");
			break;
		case FSL_DDR_CACHE_LINE_INTERLEAVING:
			puts("cache line");
			break;
		case FSL_DDR_PAGE_INTERLEAVING:
			puts("page");
			break;
		case FSL_DDR_BANK_INTERLEAVING:
			puts("bank");
			break;
		case FSL_DDR_SUPERBANK_INTERLEAVING:
			puts("super-bank");
			break;
		default:
			puts("invalid");
			break;
		}
	}
#endif

	if ((sdram_cfg >> 8) & 0x7f) {
		puts("\n");
		puts("       DDR Chip-Select Interleaving Mode: ");
		switch(sdram_cfg >> 8 & 0x7f) {
		case FSL_DDR_CS0_CS1_CS2_CS3:
			puts("CS0+CS1+CS2+CS3");
			break;
		case FSL_DDR_CS0_CS1:
			puts("CS0+CS1");
			break;
		case FSL_DDR_CS2_CS3:
			puts("CS2+CS3");
			break;
		case FSL_DDR_CS0_CS1_AND_CS2_CS3:
			puts("CS0+CS1 and CS2+CS3");
			break;
		default:
			puts("invalid");
			break;
		}
	}
}

void __weak detail_board_ddr_info(void)
{
	print_ddr_info(0);
}

void board_add_ram_info(int use_default)
{
	detail_board_ddr_info();
}
