// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016-2018, NVIDIA CORPORATION.
 */

#include <stdlib.h>
#include <common.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <asm/arch/tegra.h>
#include <asm/armv8/mmu.h>

extern unsigned long nvtboot_boot_x0;

/*
 * The following few functions run late during the boot process and dynamically
 * calculate the load address of various binaries. To keep track of multiple
 * allocations, some writable list of RAM banks must be used. tegra_mem_map[]
 * is used for this purpose to avoid making yet another copy of the list of RAM
 * banks. This is safe because tegra_mem_map[] is only used once during very
 * early boot to create U-Boot's page tables, long before this code runs. If
 * this assumption becomes invalid later, we can just fix the code to copy the
 * list of RAM banks into some private data structure before running.
 */

extern struct mm_region tegra_mem_map[];

static char *gen_varname(const char *var, const char *ext)
{
	size_t len_var = strlen(var);
	size_t len_ext = strlen(ext);
	size_t len = len_var + len_ext + 1;
	char *varext = malloc(len);

	if (!varext)
		return 0;
	strcpy(varext, var);
	strcpy(varext + len_var, ext);
	return varext;
}

static void mark_ram_allocated(int bank, u64 allocated_start, u64 allocated_end)
{
	u64 bank_start = tegra_mem_map[bank].virt;
	u64 bank_size = tegra_mem_map[bank].size;
	u64 bank_end = bank_start + bank_size;
	bool keep_front = allocated_start != bank_start;
	bool keep_tail = allocated_end != bank_end;

	if (keep_front && keep_tail) {
		/*
		 * There are CONFIG_NR_DRAM_BANKS DRAM entries in the array,
		 * starting at index 1 (index 0 is MMIO). So, we are at DRAM
		 * entry "bank" not "bank - 1" as for a typical 0-base array.
		 * The number of remaining DRAM entries is therefore
		 * "CONFIG_NR_DRAM_BANKS - bank". We want to duplicate the
		 * current entry and shift up the remaining entries, dropping
		 * the last one. Thus, we must copy one fewer entry than the
		 * number remaining.
		 */
		memmove(&tegra_mem_map[bank + 1], &tegra_mem_map[bank],
			CONFIG_NR_DRAM_BANKS - bank - 1);
		tegra_mem_map[bank].size = allocated_start - bank_start;
		bank++;
		tegra_mem_map[bank].virt = allocated_end;
		tegra_mem_map[bank].phys = allocated_end;
		tegra_mem_map[bank].size = bank_end - allocated_end;
	} else if (keep_front) {
		tegra_mem_map[bank].size = allocated_start - bank_start;
	} else if (keep_tail) {
		tegra_mem_map[bank].virt = allocated_end;
		tegra_mem_map[bank].phys = allocated_end;
		tegra_mem_map[bank].size = bank_end - allocated_end;
	} else {
		/*
		 * We could move all subsequent banks down in the array but
		 * that's not necessary for subsequent allocations to work, so
		 * we skip doing so.
		 */
		tegra_mem_map[bank].size = 0;
	}
}

static void reserve_ram(u64 start, u64 size)
{
	int bank;
	u64 end = start + size;

	for (bank = 1; bank <= CONFIG_NR_DRAM_BANKS; bank++) {
		u64 bank_start = tegra_mem_map[bank].virt;
		u64 bank_size = tegra_mem_map[bank].size;
		u64 bank_end = bank_start + bank_size;

		if (end <= bank_start || start > bank_end)
			continue;
		mark_ram_allocated(bank, start, end);
		break;
	}
}

static u64 alloc_ram(u64 size, u64 align, u64 offset)
{
	int bank;

	for (bank = 1; bank <= CONFIG_NR_DRAM_BANKS; bank++) {
		u64 bank_start = tegra_mem_map[bank].virt;
		u64 bank_size = tegra_mem_map[bank].size;
		u64 bank_end = bank_start + bank_size;
		u64 allocated = ROUND(bank_start, align) + offset;
		u64 allocated_end = allocated + size;

		if (allocated_end > bank_end)
			continue;
		mark_ram_allocated(bank, allocated, allocated_end);
		return allocated;
	}
	return 0;
}

static void set_calculated_aliases(char *aliases, u64 address)
{
	char *tmp, *alias;
	int err;

	aliases = strdup(aliases);
	if (!aliases) {
		pr_err("strdup(aliases) failed");
		return;
	}

	tmp = aliases;
	while (true) {
		alias = strsep(&tmp, " ");
		if (!alias)
			break;
		debug("%s: alias: %s\n", __func__, alias);
		err = env_set_hex(alias, address);
		if (err)
			pr_err("Could not set %s\n", alias);
	}

	free(aliases);
}

static void set_calculated_env_var(const char *var)
{
	char *var_size;
	char *var_align;
	char *var_offset;
	char *var_aliases;
	u64 size;
	u64 align;
	u64 offset;
	char *aliases;
	u64 address;
	int err;

	var_size = gen_varname(var, "_size");
	if (!var_size)
		return;
	var_align = gen_varname(var, "_align");
	if (!var_align)
		goto out_free_var_size;
	var_offset = gen_varname(var, "_offset");
	if (!var_offset)
		goto out_free_var_align;
	var_aliases = gen_varname(var, "_aliases");
	if (!var_aliases)
		goto out_free_var_offset;

	size = env_get_hex(var_size, 0);
	if (!size) {
		pr_err("%s not set or zero\n", var_size);
		goto out_free_var_aliases;
	}
	align = env_get_hex(var_align, 1);
	/* Handle extant variables, but with a value of 0 */
	if (!align)
		align = 1;
	offset = env_get_hex(var_offset, 0);
	aliases = env_get(var_aliases);

	debug("%s: Calc var %s; size=%llx, align=%llx, offset=%llx\n",
	      __func__, var, size, align, offset);
	if (aliases)
		debug("%s: Aliases: %s\n", __func__, aliases);

	address = alloc_ram(size, align, offset);
	if (!address) {
		pr_err("Could not allocate %s\n", var);
		goto out_free_var_aliases;
	}
	debug("%s: Address %llx\n", __func__, address);

	err = env_set_hex(var, address);
	if (err)
		pr_err("Could not set %s\n", var);
	if (aliases)
		set_calculated_aliases(aliases, address);

out_free_var_aliases:
	free(var_aliases);
out_free_var_offset:
	free(var_offset);
out_free_var_align:
	free(var_align);
out_free_var_size:
	free(var_size);
}

#ifdef DEBUG
static void dump_ram_banks(void)
{
	int bank;

	for (bank = 1; bank <= CONFIG_NR_DRAM_BANKS; bank++) {
		u64 bank_start = tegra_mem_map[bank].virt;
		u64 bank_size = tegra_mem_map[bank].size;
		u64 bank_end = bank_start + bank_size;

		if (!bank_size)
			continue;
		printf("%d: %010llx..%010llx (+%010llx)\n", bank - 1,
		       bank_start, bank_end, bank_size);
	}
}
#endif

static void set_calculated_env_vars(void)
{
	char *vars, *tmp, *var;

#ifdef DEBUG
	printf("RAM banks before any calculated env. var.s:\n");
	dump_ram_banks();
#endif

	reserve_ram(nvtboot_boot_x0, fdt_totalsize(nvtboot_boot_x0));

#ifdef DEBUG
	printf("RAM after reserving cboot DTB:\n");
	dump_ram_banks();
#endif

	vars = env_get("calculated_vars");
	if (!vars) {
		debug("%s: No env var calculated_vars\n", __func__);
		return;
	}

	vars = strdup(vars);
	if (!vars) {
		pr_err("strdup(calculated_vars) failed");
		return;
	}

	tmp = vars;
	while (true) {
		var = strsep(&tmp, " ");
		if (!var)
			break;
		debug("%s: var: %s\n", __func__, var);
		set_calculated_env_var(var);
#ifdef DEBUG
		printf("RAM banks affter allocating %s:\n", var);
		dump_ram_banks();
#endif
	}

	free(vars);
}

static int set_fdt_addr(void)
{
	int ret;

	ret = env_set_hex("fdt_addr", nvtboot_boot_x0);
	if (ret) {
		printf("Failed to set fdt_addr to point at DTB: %d\n", ret);
		return ret;
	}

	return 0;
}

/*
 * Attempt to use /chosen/nvidia,ether-mac in the nvtboot DTB to U-Boot's
 * ethaddr environment variable if possible.
 */
static int set_ethaddr_from_nvtboot(void)
{
	const void *nvtboot_blob = (void *)nvtboot_boot_x0;
	int ret, node, len;
	const u32 *prop;

	/* Already a valid address in the environment? If so, keep it */
	if (env_get("ethaddr"))
		return 0;

	node = fdt_path_offset(nvtboot_blob, "/chosen");
	if (node < 0) {
		printf("Can't find /chosen node in nvtboot DTB\n");
		return node;
	}
	prop = fdt_getprop(nvtboot_blob, node, "nvidia,ether-mac", &len);
	if (!prop) {
		printf("Can't find nvidia,ether-mac property in nvtboot DTB\n");
		return -ENOENT;
	}

	ret = env_set("ethaddr", (void *)prop);
	if (ret) {
		printf("Failed to set ethaddr from nvtboot DTB: %d\n", ret);
		return ret;
	}

	return 0;
}

int tegra_soc_board_init_late(void)
{
	set_calculated_env_vars();
	/*
	 * Ignore errors here; the value may not be used depending on
	 * extlinux.conf or boot script content.
	 */
	set_fdt_addr();
	/* Ignore errors here; not all cases care about Ethernet addresses */
	set_ethaddr_from_nvtboot();

	return 0;
}
