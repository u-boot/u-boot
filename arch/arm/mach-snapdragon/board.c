// SPDX-License-Identifier: GPL-2.0+
/*
 * Common initialisation for Qualcomm Snapdragon boards.
 *
 * Copyright (c) 2024 Linaro Ltd.
 * Author: Caleb Connolly <caleb.connolly@linaro.org>
 */

#include "time.h"
#include <asm/armv8/mmu.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/psci.h>
#include <asm/system.h>
#include <dm/device.h>
#include <dm/pinctrl.h>
#include <dm/uclass-internal.h>
#include <dm/read.h>
#include <env.h>
#include <init.h>
#include <linux/arm-smccc.h>
#include <linux/bug.h>
#include <linux/psci.h>
#include <linux/sizes.h>
#include <lmb.h>
#include <malloc.h>
#include <fdt_support.h>
#include <usb.h>
#include <sort.h>

#include "qcom-priv.h"

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region rbx_mem_map[CONFIG_NR_DRAM_BANKS + 2] = { { 0 } };

struct mm_region *mem_map = rbx_mem_map;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

static int ddr_bank_cmp(const void *v1, const void *v2)
{
	const struct {
		phys_addr_t start;
		phys_size_t size;
	} *res1 = v1, *res2 = v2;

	if (!res1->size)
		return 1;
	if (!res2->size)
		return -1;

	return (res1->start >> 24) - (res2->start >> 24);
}

int dram_init_banksize(void)
{
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret < 0)
		return ret;

	if (CONFIG_NR_DRAM_BANKS < 2)
		return 0;

	/* Sort our RAM banks -_- */
	qsort(gd->bd->bi_dram, CONFIG_NR_DRAM_BANKS, sizeof(gd->bd->bi_dram[0]), ddr_bank_cmp);

	return 0;
}

static void show_psci_version(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(ARM_PSCI_0_2_FN_PSCI_VERSION, 0, 0, 0, 0, 0, 0, 0, &res);

	debug("PSCI:  v%ld.%ld\n",
	      PSCI_VERSION_MAJOR(res.a0),
	      PSCI_VERSION_MINOR(res.a0));
}

void *board_fdt_blob_setup(int *err)
{
	phys_addr_t fdt;
	/* Return DTB pointer passed by ABL */
	*err = 0;
	fdt = get_prev_bl_fdt_addr();

	/*
	 * If we bail then the board will simply not boot, instead let's
	 * try and use the FDT built into U-Boot if there is one...
	 * This avoids having a hard dependency on the previous stage bootloader
	 */

	if (IS_ENABLED(CONFIG_OF_SEPARATE) && (!fdt || fdt != ALIGN(fdt, SZ_4K) ||
					       fdt_check_header((void *)fdt))) {
		debug("%s: Using built in FDT, bootloader gave us %#llx\n", __func__, fdt);
		return (void *)gd->fdt_blob;
	}

	return (void *)fdt;
}

void reset_cpu(void)
{
	psci_system_reset();
}

/*
 * Some Qualcomm boards require GPIO configuration when switching USB modes.
 * Support setting this configuration via pinctrl state.
 */
int board_usb_init(int index, enum usb_init_type init)
{
	struct udevice *usb;
	int ret = 0;

	/* USB device */
	ret = uclass_find_device_by_seq(UCLASS_USB, index, &usb);
	if (ret) {
		printf("Cannot find USB device\n");
		return ret;
	}

	ret = dev_read_stringlist_search(usb, "pinctrl-names",
					 "device");
	/* No "device" pinctrl state, so just bail */
	if (ret < 0)
		return 0;

	/* Select "default" or "device" pinctrl */
	switch (init) {
	case USB_INIT_HOST:
		pinctrl_select_state(usb, "default");
		break;
	case USB_INIT_DEVICE:
		pinctrl_select_state(usb, "device");
		break;
	default:
		debug("Unknown usb_init_type %d\n", init);
		break;
	}

	return 0;
}

/*
 * Some boards still need board specific init code, they can implement that by
 * overriding this function.
 *
 * FIXME: get rid of board specific init code
 */
void __weak qcom_board_init(void)
{
}

int board_init(void)
{
	show_psci_version();
	qcom_of_fixup_nodes();
	qcom_board_init();
	return 0;
}

/* Sets up the "board", and "soc" environment variables as well as constructing the devicetree
 * path, with a few quirks to handle non-standard dtb filenames. This is not meant to be a
 * comprehensive solution to automatically picking the DTB, but aims to be correct for the
 * majority case. For most devices it should be possible to make this algorithm work by
 * adjusting the root compatible property in the U-Boot DTS. Handling devices with multiple
 * variants that are all supported by a single U-Boot image will require implementing device-
 * specific detection.
 */
static void configure_env(void)
{
	const char *first_compat, *last_compat;
	char *tmp;
	char buf[32] = { 0 };
	/*
	 * Most DTB filenames follow the scheme: qcom/<soc>-[vendor]-<board>.dtb
	 * The vendor is skipped when it's a Qualcomm reference board, or the
	 * db845c.
	 */
	char dt_path[64] = { 0 };
	int compat_count, ret;
	ofnode root;

	root = ofnode_root();
	/* This is almost always 2, but be explicit that we want the first and last compatibles
	 * not the first and second.
	 */
	compat_count = ofnode_read_string_count(root, "compatible");
	if (compat_count < 2) {
		log_warning("%s: only one root compatible bailing!\n", __func__);
		return;
	}

	/* The most specific device compatible (e.g. "thundercomm,db845c") */
	ret = ofnode_read_string_index(root, "compatible", 0, &first_compat);
	if (ret < 0) {
		log_warning("Can't read first compatible\n");
		return;
	}

	/* The last compatible is always the SoC compatible */
	ret = ofnode_read_string_index(root, "compatible", compat_count - 1, &last_compat);
	if (ret < 0) {
		log_warning("Can't read second compatible\n");
		return;
	}

	/* Copy the second compat (e.g. "qcom,sdm845") into buf */
	strlcpy(buf, last_compat, sizeof(buf) - 1);
	tmp = buf;

	/* strsep() is destructive, it replaces the comma with a \0 */
	if (!strsep(&tmp, ",")) {
		log_warning("second compatible '%s' has no ','\n", buf);
		return;
	}

	/* tmp now points to just the "sdm845" part of the string */
	env_set("soc", tmp);

	/* Now figure out the "board" part from the first compatible */
	memset(buf, 0, sizeof(buf));
	strlcpy(buf, first_compat, sizeof(buf) - 1);
	tmp = buf;

	/* The Qualcomm reference boards (RBx, HDK, etc)  */
	if (!strncmp("qcom", buf, strlen("qcom"))) {
		/*
		 * They all have the first compatible as "qcom,<soc>-<board>"
		 * (e.g. "qcom,qrb5165-rb5"). We extract just the part after
		 * the dash.
		 */
		if (!strsep(&tmp, "-")) {
			log_warning("compatible '%s' has no '-'\n", buf);
			return;
		}
		/* tmp is now "rb5" */
		env_set("board", tmp);
	} else {
		if (!strsep(&tmp, ",")) {
			log_warning("compatible '%s' has no ','\n", buf);
			return;
		}
		/* for thundercomm we just want the bit after the comma (e.g. "db845c"),
		 * for all other boards we replace the comma with a '-' and take both
		 * (e.g. "oneplus-enchilada")
		 */
		if (!strncmp("thundercomm", buf, strlen("thundercomm"))) {
			env_set("board", tmp);
		} else {
			*(tmp - 1) = '-';
			env_set("board", buf);
		}
	}

	/* Now build the full path name */
	snprintf(dt_path, sizeof(dt_path), "qcom/%s-%s.dtb",
		 env_get("soc"), env_get("board"));
	env_set("fdtfile", dt_path);
}

void __weak qcom_late_init(void)
{
}

#define KERNEL_COMP_SIZE	SZ_64M

#define addr_alloc(lmb, size) lmb_alloc(lmb, size, SZ_2M)

/* Stolen from arch/arm/mach-apple/board.c */
int board_late_init(void)
{
	struct lmb lmb;
	u32 status = 0;

	lmb_init_and_reserve(&lmb, gd->bd, (void *)gd->fdt_blob);

	/* We need to be fairly conservative here as we support boards with just 1G of TOTAL RAM */
	status |= env_set_hex("kernel_addr_r", addr_alloc(&lmb, SZ_128M));
	status |= env_set_hex("ramdisk_addr_r", addr_alloc(&lmb, SZ_128M));
	status |= env_set_hex("kernel_comp_addr_r", addr_alloc(&lmb, KERNEL_COMP_SIZE));
	status |= env_set_hex("kernel_comp_size", KERNEL_COMP_SIZE);
	status |= env_set_hex("scriptaddr", addr_alloc(&lmb, SZ_4M));
	status |= env_set_hex("pxefile_addr_r", addr_alloc(&lmb, SZ_4M));
	status |= env_set_hex("fdt_addr_r", addr_alloc(&lmb, SZ_2M));

	if (status)
		log_warning("%s: Failed to set run time variables\n", __func__);

	configure_env();
	qcom_late_init();

	return 0;
}

static void build_mem_map(void)
{
	int i, j;

	/*
	 * Ensure the peripheral block is sized to correctly cover the address range
	 * up to the first memory bank.
	 * Don't map the first page to ensure that we actually trigger an abort on a
	 * null pointer access rather than just hanging.
	 * FIXME: we should probably split this into more precise regions
	 */
	mem_map[0].phys = 0x1000;
	mem_map[0].virt = mem_map[0].phys;
	mem_map[0].size = gd->bd->bi_dram[0].start - mem_map[0].phys;
	mem_map[0].attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN;

	for (i = 1, j = 0; i < ARRAY_SIZE(rbx_mem_map) - 1 && gd->bd->bi_dram[j].size; i++, j++) {
		mem_map[i].phys = gd->bd->bi_dram[j].start;
		mem_map[i].virt = mem_map[i].phys;
		mem_map[i].size = gd->bd->bi_dram[j].size;
		mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | \
				   PTE_BLOCK_INNER_SHARE;
	}

	mem_map[i].phys = UINT64_MAX;
	mem_map[i].size = 0;

#ifdef DEBUG
	debug("Configured memory map:\n");
	for (i = 0; mem_map[i].size; i++)
		debug("  0x%016llx - 0x%016llx: entry %d\n",
		      mem_map[i].phys, mem_map[i].phys + mem_map[i].size, i);
#endif
}

u64 get_page_table_size(void)
{
	return SZ_64K;
}

static int fdt_cmp_res(const void *v1, const void *v2)
{
	const struct fdt_resource *res1 = v1, *res2 = v2;

	return res1->start - res2->start;
}

#define N_RESERVED_REGIONS 32

/* Mark all no-map regions as PTE_TYPE_FAULT to prevent speculative access.
 * On some platforms this is enough to trigger a security violation and trap
 * to EL3.
 */
static void carve_out_reserved_memory(void)
{
	static struct fdt_resource res[N_RESERVED_REGIONS] = { 0 };
	int parent, rmem, count, i = 0;
	phys_addr_t start;
	size_t size;

	/* Some reserved nodes must be carved out, as the cache-prefetcher may otherwise
	 * attempt to access them, causing a security exception.
	 */
	parent = fdt_path_offset(gd->fdt_blob, "/reserved-memory");
	if (parent <= 0) {
		log_err("No reserved memory regions found\n");
		return;
	}

	/* Collect the reserved memory regions */
	fdt_for_each_subnode(rmem, gd->fdt_blob, parent) {
		const fdt32_t *ptr;
		int len;
		if (!fdt_getprop(gd->fdt_blob, rmem, "no-map", NULL))
			continue;

		if (i == N_RESERVED_REGIONS) {
			log_err("Too many reserved regions!\n");
			break;
		}

		/* Read the address and size out from the reg property. Doing this "properly" with
		 * fdt_get_resource() takes ~70ms on SDM845, but open-coding the happy path here
		 * takes <1ms... Oh the woes of no dcache.
		 */
		ptr = fdt_getprop(gd->fdt_blob, rmem, "reg", &len);
		if (ptr) {
			/* Qualcomm devices use #address/size-cells = <2> but all reserved regions are within
			 * the 32-bit address space. So we can cheat here for speed.
			 */
			res[i].start = fdt32_to_cpu(ptr[1]);
			res[i].end = res[i].start + fdt32_to_cpu(ptr[3]);
			i++;
		}
	}

	/* Sort the reserved memory regions by address */
	count = i;
	qsort(res, count, sizeof(struct fdt_resource), fdt_cmp_res);

	/* Now set the right attributes for them. Often a lot of the regions are tightly packed together
	 * so we can optimise the number of calls to mmu_change_region_attr() by combining adjacent
	 * regions.
	 */
	start = ALIGN_DOWN(res[0].start, SZ_2M);
	size = ALIGN(res[0].end - start, SZ_2M);
	for (i = 1; i <= count; i++) {
		/* We ideally want to 2M align everything for more efficient pagetables, but we must avoid
		 * overwriting reserved memory regions which shouldn't be mapped as FAULT (like those with
		 * compatible properties).
		 * If within 2M of the previous region, bump the size to include this region. Otherwise
		 * start a new region.
		 */
		if (i == count || start + size < res[i].start - SZ_2M) {
			debug("  0x%016llx - 0x%016llx: reserved\n",
			      start, start + size);
			mmu_change_region_attr(start, size, PTE_TYPE_FAULT);
			/* If this is the final region then quit here before we index
			 * out of bounds...
			 */
			if (i == count)
				break;
			start = ALIGN_DOWN(res[i].start, SZ_2M);
			size = ALIGN(res[i].end - start, SZ_2M);
		} else {
			/* Bump size if this region is immediately after the previous one */
			size = ALIGN(res[i].end - start, SZ_2M);
		}
	}
}

/* This function open-codes setup_all_pgtables() so that we can
 * insert additional mappings *before* turning on the MMU.
 */
void enable_caches(void)
{
	u64 tlb_addr = gd->arch.tlb_addr;
	u64 tlb_size = gd->arch.tlb_size;
	u64 pt_size;
	ulong carveout_start;

	gd->arch.tlb_fillptr = tlb_addr;

	build_mem_map();

	icache_enable();

	/* Create normal system page tables */
	setup_pgtables();

	pt_size = (uintptr_t)gd->arch.tlb_fillptr -
		  (uintptr_t)gd->arch.tlb_addr;
	debug("Primary pagetable size: %lluKiB\n", pt_size / 1024);

	/* Create emergency page tables */
	gd->arch.tlb_size -= pt_size;
	gd->arch.tlb_addr = gd->arch.tlb_fillptr;
	setup_pgtables();
	gd->arch.tlb_emerg = gd->arch.tlb_addr;
	gd->arch.tlb_addr = tlb_addr;
	gd->arch.tlb_size = tlb_size;

	carveout_start = get_timer(0);
	/* Takes ~20-50ms on SDM845 */
	carve_out_reserved_memory();
	debug("carveout time: %lums\n", get_timer(carveout_start));

	dcache_enable();
}
