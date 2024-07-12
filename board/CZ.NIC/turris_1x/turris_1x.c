// SPDX-License-Identifier: GPL-2.0+
// (C) 2022 Pali Rohár <pali@kernel.org>

#include <init.h>
#include <env.h>
#include <fdt_support.h>
#include <clock_legacy.h>
#include <image.h>
#include <asm/fsl_law.h>
#include <asm/global_data.h>
#include <asm/mmu.h>
#include <dm/device.h>
#include <dm/ofnode.h>
#include <linux/build_bug.h>
#include <display_options.h>

#include "../turris_atsha_otp.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Reset time cycle register provided by Turris CPLD firmware.
 * Turris CPLD firmware is open source and available at:
 * https://gitlab.nic.cz/turris/hw/turris_cpld/-/blob/master/CZ_NIC_Router_CPLD.v
 */
#define TURRIS_CPLD_RESET_TIME_CYCLE_REG	((void *)CFG_SYS_CPLD_BASE + 0x1f)
#define  TURRIS_CPLD_RESET_TIME_CYCLE_300MS	BIT(0)
#define  TURRIS_CPLD_RESET_TIME_CYCLE_1S	BIT(1)
#define  TURRIS_CPLD_RESET_TIME_CYCLE_2S	BIT(2)
#define  TURRIS_CPLD_RESET_TIME_CYCLE_3S	BIT(3)
#define  TURRIS_CPLD_RESET_TIME_CYCLE_4S	BIT(4)
#define  TURRIS_CPLD_RESET_TIME_CYCLE_5S	BIT(5)
#define  TURRIS_CPLD_RESET_TIME_CYCLE_6S	BIT(6)

#define TURRIS_CPLD_LED_BRIGHTNESS_REG_FIRST	((void *)CFG_SYS_CPLD_BASE + 0x13)
#define TURRIS_CPLD_LED_BRIGHTNESS_REG_LAST	((void *)CFG_SYS_CPLD_BASE + 0x1e)
#define TURRIS_CPLD_LED_SW_OVERRIDE_REG		((void *)CFG_SYS_CPLD_BASE + 0x22)

int dram_init_banksize(void)
{
	phys_size_t size = gd->ram_size;

	static_assert(CONFIG_NR_DRAM_BANKS >= 3);

	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = get_effective_memsize();
	size -= gd->bd->bi_dram[0].size;

	/* Note: This address space is not mapped via TLB entries in U-Boot */

#ifndef CONFIG_SDCARD
	if (size > 0) {
		/*
		 * Setup additional overlapping 1 GB DDR LAW at the end of
		 * 32-bit physical address space. It overlaps with all other
		 * peripherals on P2020 mapped to physical address space.
		 * But this is not issue because documentation says:
		 * P2020 QorIQ Integrated Processor Reference Manual,
		 * section 2.3.1 Precedence of local access windows:
		 * If two local access windows overlap, the lower
		 * numbered window takes precedence.
		 */
		if (set_ddr_laws(0xc0000000, SZ_1G, LAW_TRGT_IF_DDR_1) < 0) {
			printf("Error: Cannot setup DDR LAW for more than 2 GB\n");
			return 0;
		}
	}

	if (size > 0) {
		/* Free space between PCIe bus 3 MEM and NOR */
		gd->bd->bi_dram[1].start = 0xc0200000;
		gd->bd->bi_dram[1].size = min(size, 0xef000000 - gd->bd->bi_dram[1].start);
		size -= gd->bd->bi_dram[1].size;
	}

	if (size > 0) {
		/* Free space between NOR and NAND */
		gd->bd->bi_dram[2].start = 0xf0000000;
		gd->bd->bi_dram[2].size = min(size, 0xff800000 - gd->bd->bi_dram[2].start);
		size -= gd->bd->bi_dram[2].size;
	}
#else
	puts("\n\n!!! TODO: fix sdcard >2GB RAM\n\n\n");
#endif
	return 0;
}

static inline int fdt_setprop_inplace_u32_partial(void *blob, int node,
						  const char *name,
						  u32 idx, u32 val)
{
	val = cpu_to_fdt32(val);

	return fdt_setprop_inplace_namelen_partial(blob, node, name,
						   strlen(name),
						   idx * sizeof(u32),
						   &val, sizeof(u32));
}

/* Setup correct size of PCIe controller MEM in DT "ranges" property recursively */
static void fdt_fixup_pcie_mem_size(void *blob, int node, phys_size_t pcie1_mem,
				    phys_size_t pcie2_mem, phys_size_t pcie3_mem)
{
	int pci_cells, cpu_cells, size_cells;
	const u32 *ranges;
	int pnode;
	int i, len;
	u32 pci_flags;
	u64 cpu_addr;
	u64 size;
	u64 new_size;
	int pcie_id;
	int idx;
	int subnode;
	int ret;

	if (!fdtdec_get_is_enabled(blob, node))
		return;

	ranges = fdt_getprop(blob, node, "ranges", &len);
	if (!ranges || !len || len % sizeof(u32))
		return;

	/*
	 * The "ranges" property is an array of
	 *   { <PCI address> <CPU address> <size in PCI address space> }
	 * where number of PCI address cells and size cells is stored in the
	 * "#address-cells" and "#size-cells" properties of the same node
	 * containing the "ranges" property and number of CPU address cells
	 * is stored in the parent's "#address-cells" property.
	 *
	 * All 3 elements can span a different number of cells. Fetch them.
	 */
	pnode = fdt_parent_offset(blob, node);
	pci_cells = fdt_address_cells(blob, node);
	cpu_cells = fdt_address_cells(blob, pnode);
	size_cells = fdt_size_cells(blob, node);

	/* PCI addresses always use 3 cells */
	if (pci_cells != 3)
		return;

	/* CPU addresses and sizes on P2020 may be 32-bit (1 cell) or 64-bit (2 cells) */
	if (cpu_cells != 1 && cpu_cells != 2)
		return;
	if (size_cells != 1 && size_cells != 2)
		return;

	for (i = 0; i < len / sizeof(u32); i += pci_cells + cpu_cells + size_cells) {
		/* PCI address consists of 3 cells: flags, addr.hi, addr.lo */
		pci_flags = fdt32_to_cpu(ranges[i]);

		cpu_addr = fdt32_to_cpu(ranges[i + pci_cells]);
		if (cpu_cells == 2) {
			cpu_addr <<= 32;
			cpu_addr |= fdt32_to_cpu(ranges[i + pci_cells + 1]);
		}

		size = fdt32_to_cpu(ranges[i + pci_cells + cpu_cells]);
		if (size_cells == 2) {
			size <<= 32;
			size |= fdt32_to_cpu(ranges[i + pci_cells + cpu_cells + 1]);
		}

		/*
		 * Bits [25:24] of PCI flags defines space code
		 * 0b10 is 32-bit MEM and 0b11 is 64-bit MEM.
		 * Check for any type of PCIe MEM mapping.
		 */
		if (!(pci_flags & 0x02000000))
			continue;

		if (cpu_addr == CFG_SYS_PCIE1_MEM_PHYS && size > pcie1_mem) {
			pcie_id = 1;
			new_size = pcie1_mem;
		} else if (cpu_addr == CFG_SYS_PCIE2_MEM_PHYS && size > pcie2_mem) {
			pcie_id = 2;
			new_size = pcie2_mem;
		} else if (cpu_addr == CFG_SYS_PCIE3_MEM_PHYS && size > pcie3_mem) {
			pcie_id = 3;
			new_size = pcie3_mem;
		} else {
			continue;
		}

		printf("Decreasing PCIe MEM %d size from ", pcie_id);
		print_size(size, " to ");
		print_size(new_size, "\n");
		idx = i + pci_cells + cpu_cells;
		if (size_cells == 2) {
			ret = fdt_setprop_inplace_u32_partial(blob, node,
							      "ranges", idx, 0);
			if (ret)
				goto err;
			idx++;
		}
		ret = fdt_setprop_inplace_u32_partial(blob, node,
						      "ranges", idx, SZ_2M);
		if (ret)
			goto err;
	}

	/* Recursively fix also all subnodes */
	fdt_for_each_subnode(subnode, blob, node)
		fdt_fixup_pcie_mem_size(blob, subnode, pcie1_mem, pcie2_mem, pcie3_mem);

	return;

err:
	printf("Error: Cannot update \"ranges\" property\n");
}

static inline phys_size_t get_law_size(phys_addr_t addr, enum law_trgt_if id)
{
	struct law_entry e;

	e = find_law_by_addr_id(addr, id);
	if (e.index < 0)
		return 0;

	return 2ULL << e.size;
}

void ft_memory_setup(void *blob, struct bd_info *bd)
{
	phys_size_t pcie1_mem, pcie2_mem, pcie3_mem;
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int count;
	int node;

	if (!env_get("bootm_low") && !env_get("bootm_size")) {
		for (count = 0; count < CONFIG_NR_DRAM_BANKS; count++) {
			start[count] = gd->bd->bi_dram[count].start;
			size[count] = gd->bd->bi_dram[count].size;
			if (!size[count])
				break;
		}
		fdt_fixup_memory_banks(blob, start, size, count);
	} else {
		fdt_fixup_memory(blob, env_get_bootm_low(), env_get_bootm_size());
	}

	pcie1_mem = get_law_size(CFG_SYS_PCIE1_MEM_PHYS, LAW_TRGT_IF_PCIE_1);
	pcie2_mem = get_law_size(CFG_SYS_PCIE2_MEM_PHYS, LAW_TRGT_IF_PCIE_2);
	pcie3_mem = get_law_size(CFG_SYS_PCIE3_MEM_PHYS, LAW_TRGT_IF_PCIE_3);

	fdt_for_each_node_by_compatible(node, blob, -1, "fsl,mpc8548-pcie")
		fdt_fixup_pcie_mem_size(blob, node, pcie1_mem, pcie2_mem, pcie3_mem);
}

static int detect_model_serial(const char **model, char serial[17])
{
	u32 version_num;
	int err;

	err = turris_atsha_otp_get_serial_number(serial);
	if (err) {
		*model = "Turris 1.x";
		strcpy(serial, "unknown");
		return -1;
	}

	version_num = simple_strtoull(serial, NULL, 16) >> 32;

	/*
	 * Turris 1.0 boards (RTRS01) have version_num 0x5.
	 * Turris 1.1 boards (RTRS02) have version_num 0x6, 0x7, 0x8 and 0x9.
	 */
	if (be32_to_cpu(version_num) >= 0x6) {
		*model = "Turris 1.1 (RTRS02)";
		return 1;
	}

	*model = "Turris 1.0 (RTRS01)";
	return 0;
}

void p1_p2_rdb_pc_fix_fdt_model(void *blob)
{
	const char *model;
	char serial[17];
	int len;
	int off;
	int rev;
	char c;

	rev = detect_model_serial(&model, serial);
	if (rev < 0)
		return;

	/* Turris 1.0 boards (RTRS01) do not have third PCIe controller */
	if (rev == 0) {
		off = fdt_path_offset(blob, "pci2");
		if (off >= 0)
			fdt_del_node(blob, off);
	}

	/* Fix model string only in case it is generic "Turris 1.x" */
	model = fdt_getprop(blob, 0, "model", &len);
	if (len < sizeof("Turris 1.x") - 1)
		return;
	if (memcmp(model, "Turris 1.x", sizeof("Turris 1.x") - 1) != 0)
		return;

	c = '0' + rev;
	fdt_setprop_inplace_namelen_partial(blob, 0, "model", sizeof("model") - 1,
					    sizeof("Turris 1.") - 1, &c, 1);
}

int misc_init_r(void)
{
	turris_atsha_otp_init_mac_addresses(0);
	turris_atsha_otp_init_serial_number();
	return 0;
}

/* This comes from ../../freescale/p1_p2_rdb_pc/p1_p2_rdb_pc.c */
extern int checkboard_p1_p2(void);

int checkboard(void)
{
	const char *model;
	char serial[17];
	void *reg;

	/* Disable software control of all Turris LEDs */
	out_8(TURRIS_CPLD_LED_SW_OVERRIDE_REG, 0x00);

	/* Reset colors of all Turris LEDs to their default values */
	for (reg = TURRIS_CPLD_LED_BRIGHTNESS_REG_FIRST;
	     reg <= TURRIS_CPLD_LED_BRIGHTNESS_REG_LAST;
	     reg++)
		out_8(reg, 0xff);

	detect_model_serial(&model, serial);
	printf("Revision: %s\n", model);
	printf("Serial Number: %s\n", serial);

	return checkboard_p1_p2();
}

static void handle_reset_button(void)
{
	const char * const vars[1] = { "bootcmd_rescue", };
	u8 reset_time_raw, reset_time;

	/*
	 * Ensure that bootcmd_rescue has always stock value, so that running
	 *   run bootcmd_rescue
	 * always works correctly.
	 */
	env_set_default_vars(1, (char * const *)vars, 0);

	reset_time_raw = in_8(TURRIS_CPLD_RESET_TIME_CYCLE_REG);
	if (reset_time_raw & TURRIS_CPLD_RESET_TIME_CYCLE_6S)
		reset_time = 6;
	else if (reset_time_raw & TURRIS_CPLD_RESET_TIME_CYCLE_5S)
		reset_time = 5;
	else if (reset_time_raw & TURRIS_CPLD_RESET_TIME_CYCLE_4S)
		reset_time = 4;
	else if (reset_time_raw & TURRIS_CPLD_RESET_TIME_CYCLE_3S)
		reset_time = 3;
	else if (reset_time_raw & TURRIS_CPLD_RESET_TIME_CYCLE_2S)
		reset_time = 2;
	else if (reset_time_raw & TURRIS_CPLD_RESET_TIME_CYCLE_1S)
		reset_time = 1;
	else
		reset_time = 0;

	env_set_ulong("turris_reset", reset_time);

	/* Check if red reset button was hold for at least six seconds. */
	if (reset_time >= 6) {
		const char * const vars[3] = {
			"bootcmd",
			"bootdelay",
			"distro_bootcmd",
		};

		/*
		 * Set the above envs to their default values, in case the user
		 * managed to break them.
		 */
		env_set_default_vars(3, (char * const *)vars, 0);

		/* Ensure bootcmd_rescue is used by distroboot */
		env_set("boot_targets", "rescue");

		printf("RESET button was hold for >= 6s, overwriting boot_targets for system rescue!\n");
	} else {
		/*
		 * In case the user somehow managed to save environment with
		 * boot_targets=rescue, reset boot_targets to default value.
		 * This could happen in subsequent commands if bootcmd_rescue
		 * failed.
		 */
		if (!strcmp(env_get("boot_targets"), "rescue")) {
			const char * const vars[1] = {
				"boot_targets",
			};

			env_set_default_vars(1, (char * const *)vars, 0);
		}

		if (reset_time > 0)
			printf("RESET button was hold for %us.\n", reset_time);
	}
}

static int recalculate_pcie_mem_law(phys_addr_t addr,
				    pci_size_t pcie_size,
				    enum law_trgt_if id,
				    phys_addr_t *free_start,
				    phys_size_t *free_size)
{
	phys_size_t cur_size, new_size;
	struct law_entry e;

	e = find_law_by_addr_id(addr, id);
	if (e.index < 0) {
		*free_start = *free_size = 0;
		return 0;
	}

	cur_size = 2ULL << e.size;
	new_size = roundup_pow_of_two(pcie_size);

	if (new_size >= cur_size) {
		*free_start = *free_size = 0;
		return 0;
	}

	set_law(e.index, addr, law_size_bits(new_size), id);

	*free_start = addr + new_size;
	*free_size = cur_size - new_size;
	return 1;
}

static void recalculate_used_pcie_mem(void)
{
	phys_addr_t free_start1, free_start2;
	phys_size_t free_size1, free_size2;
	pci_size_t pcie1_used_mem_size;
	pci_size_t pcie2_used_mem_size;
	struct law_entry e;
	phys_size_t size;
	ofnode node;
	int i;

	size = gd->ram_size;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		size -= gd->bd->bi_dram[i].size;

	if (size == 0)
		return;

	e = find_law_by_addr_id(CFG_SYS_PCIE3_MEM_PHYS, LAW_TRGT_IF_PCIE_3);
	if (e.index < 0 && gd->bd->bi_dram[1].size > 0) {
		/*
		 * If there is no LAW for PCIe 3 MEM then 3rd PCIe controller
		 * is inactive, which is the case for Turris 1.0 boards. So
		 * use its reserved 2 MB physical space for DDR RAM.
		 */
		unsigned int bank_size = SZ_2M;

		if (bank_size > size)
			bank_size = size;
		printf("Reserving unused ");
		print_size(bank_size, "");
		printf(" of PCIe 3 MEM for DDR RAM\n");
		gd->bd->bi_dram[1].start -= bank_size;
		gd->bd->bi_dram[1].size += bank_size;
		size -= bank_size;
		if (size == 0)
			return;
	}

#ifdef CONFIG_PCI_PNP
	/*
	 * Detect how much space of PCIe MEM is needed for both PCIe 1 and
	 * PCIe 2 controllers with all connected cards on whole hierarchy.
	 * This works only when U-Boot has enabled PCI PNP code which scans
	 * all PCI devices and calculate required memory for every PCI BAR of
	 * every PCI device.
	 */
	ofnode_for_each_compatible_node(node, "fsl,mpc8548-pcie") {
		struct udevice *dev;

		if (device_find_global_by_ofnode(node, &dev))
			continue;

		struct pci_controller *hose = dev_get_uclass_priv(pci_get_controller(dev));

		if (!hose)
			continue;
		if (!hose->pci_mem)
			continue;
		if (!hose->pci_mem->size)
			continue;

		pci_size_t used_mem_size = hose->pci_mem->bus_lower - hose->pci_mem->bus_start;

		if (hose->pci_mem->phys_start == CFG_SYS_PCIE1_MEM_PHYS)
			pcie1_used_mem_size = used_mem_size;
		else if (hose->pci_mem->phys_start == CFG_SYS_PCIE2_MEM_PHYS)
			pcie2_used_mem_size = used_mem_size;
	}

	if (pcie1_used_mem_size == 0 && pcie2_used_mem_size == 0)
		return;

	e = find_law_by_addr_id(0xc0000000, LAW_TRGT_IF_DDR_1);
	if (e.index < 0) {
		printf("Error: Cannot setup DDR LAW for more than 3 GB of RAM\n");
		return;
	}

	/*
	 * Increase additional overlapping 1 GB DDR LAW from 1GB to 2GB by
	 * moving its left side from 0xc0000000 to 0x80000000. After this
	 * change it would overlap with PCIe MEM 1 and 2 LAWs.
	 */
	set_law(e.index, 0x80000000, LAW_SIZE_2G, LAW_TRGT_IF_DDR_1);

	i = 3;
	static_assert(CONFIG_NR_DRAM_BANKS >= 3 + 2);

	if (recalculate_pcie_mem_law(CFG_SYS_PCIE2_MEM_PHYS,
				     pcie2_used_mem_size, LAW_TRGT_IF_PCIE_2,
				     &free_start2, &free_size2)) {
		printf("Reserving unused ");
		print_size(free_size2, "");
		printf(" of PCIe 2 MEM for DDR RAM\n");
		gd->bd->bi_dram[i].start = free_start2;
		gd->bd->bi_dram[i].size = min(size, free_size2);
		size -= gd->bd->bi_dram[i].start;
		i++;
		if (size == 0)
			return;
	}

	if (recalculate_pcie_mem_law(CFG_SYS_PCIE1_MEM_PHYS,
				     pcie1_used_mem_size, LAW_TRGT_IF_PCIE_1,
				     &free_start1, &free_size1)) {
		printf("Reserving unused ");
		print_size(free_size1, "");
		printf(" of PCIe 1 MEM for DDR RAM\n");
		gd->bd->bi_dram[i].start = free_start1;
		gd->bd->bi_dram[i].size = min(size, free_size1);
		size -= gd->bd->bi_dram[i].size;
		i++;
		if (size == 0)
			return;
	}
#endif
}

int last_stage_init(void)
{
	handle_reset_button();
	recalculate_used_pcie_mem();
	return 0;
}

int get_serial_clock(void)
{
	return get_bus_freq(0);
}
