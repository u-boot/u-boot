/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#ifndef __ASSEMBLY__

enum pei_boot_mode_t {
	PEI_BOOT_NONE = 0,
	PEI_BOOT_SOFT_RESET,
	PEI_BOOT_RESUME,

};

struct memory_area {
	uint64_t start;
	uint64_t size;
};

struct memory_info {
	int num_areas;
	uint64_t total_memory;
	uint64_t total_32bit_memory;
	struct memory_area area[CONFIG_NR_DRAM_BANKS];
};

#define MAX_MTRR_REQUESTS	8

/**
 * A request for a memory region to be set up in a particular way. These
 * requests are processed before board_init_r() is called. They are generally
 * optional and can be ignored with some performance impact.
 */
struct mtrr_request {
	int type;		/* MTRR_TYPE_... */
	uint64_t start;
	uint64_t size;
};

/* Architecture-specific global data */
struct arch_global_data {
	struct global_data *gd_addr;	/* Location of Global Data */
	uint8_t x86;			/* CPU family */
	uint8_t x86_vendor;		/* CPU vendor */
	uint8_t x86_model;
	uint8_t x86_mask;
	uint32_t x86_device;
	uint64_t tsc_base;		/* Initial value returned by rdtsc() */
	uint32_t tsc_base_kclocks;	/* Initial tsc as a kclocks value */
	uint32_t tsc_prev;		/* For show_boot_progress() */
	uint32_t tsc_mhz;		/* TSC frequency in MHz */
	void *new_fdt;			/* Relocated FDT */
	uint32_t bist;			/* Built-in self test value */
	enum pei_boot_mode_t pei_boot_mode;
	const struct pch_gpio_map *gpio_map;	/* board GPIO map */
	struct memory_info meminfo;	/* Memory information */
#ifdef CONFIG_HAVE_FSP
	void *hob_list;			/* FSP HOB list */
#endif
	struct mtrr_request mtrr_req[MAX_MTRR_REQUESTS];
	int mtrr_req_count;
	int has_mtrr;
	/* MRC training data to save for the next boot */
	char *mrc_output;
	unsigned int mrc_output_len;
	void *gdt;			/* Global descriptor table */
};

#endif

#include <asm-generic/global_data.h>

#ifndef __ASSEMBLY__
static inline __attribute__((no_instrument_function)) gd_t *get_fs_gd_ptr(void)
{
	gd_t *gd_ptr;

	asm volatile("fs movl 0, %0\n" : "=r" (gd_ptr));

	return gd_ptr;
}

#define gd	get_fs_gd_ptr()

#endif

/*
 * Our private Global Data Flags
 */
#define GD_FLG_COLD_BOOT	0x00100	/* Cold Boot */
#define GD_FLG_WARM_BOOT	0x00200	/* Warm Boot */

#define DECLARE_GLOBAL_DATA_PTR

#endif /* __ASM_GBL_DATA_H */
