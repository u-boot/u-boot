/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <phy.h>
#ifdef CONFIG_FSL_LSCH3
#include <asm/arch/fdt.h>
#endif
#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif
#ifdef CONFIG_SYS_DPAA_FMAN
#include <fsl_fman.h>
#endif
#ifdef CONFIG_MP
#include <asm/arch/mp.h>
#endif

int fdt_fixup_phy_connection(void *blob, int offset, phy_interface_t phyc)
{
	return fdt_setprop_string(blob, offset, "phy-connection-type",
					 phy_string_for_interface(phyc));
}

#ifdef CONFIG_MP
void ft_fixup_cpu(void *blob)
{
	int off;
	__maybe_unused u64 spin_tbl_addr = (u64)get_spin_tbl_addr();
	fdt32_t *reg;
	int addr_cells;
	u64 val, core_id;
	size_t *boot_code_size = &(__secondary_boot_code_size);

	off = fdt_path_offset(blob, "/cpus");
	if (off < 0) {
		puts("couldn't find /cpus node\n");
		return;
	}
	of_bus_default_count_cells(blob, off, &addr_cells, NULL);

	off = fdt_node_offset_by_prop_value(blob, -1, "device_type", "cpu", 4);
	while (off != -FDT_ERR_NOTFOUND) {
		reg = (fdt32_t *)fdt_getprop(blob, off, "reg", 0);
		if (reg) {
			core_id = of_read_number(reg, addr_cells);
			if (core_id  == 0 || (is_core_online(core_id))) {
				val = spin_tbl_addr;
				val += id_to_core(core_id) *
				       SPIN_TABLE_ELEM_SIZE;
				val = cpu_to_fdt64(val);
				fdt_setprop_string(blob, off, "enable-method",
						   "spin-table");
				fdt_setprop(blob, off, "cpu-release-addr",
					    &val, sizeof(val));
			} else {
				debug("skipping offline core\n");
			}
		} else {
			puts("Warning: found cpu node without reg property\n");
		}
		off = fdt_node_offset_by_prop_value(blob, off, "device_type",
						    "cpu", 4);
	}

	fdt_add_mem_rsv(blob, (uintptr_t)&secondary_boot_code,
			*boot_code_size);
}
#endif

/*
 * the burden is on the the caller to not request a count
 * exceeding the bounds of the stream_ids[] array
 */
void alloc_stream_ids(int start_id, int count, u32 *stream_ids, int max_cnt)
{
	int i;

	if (count > max_cnt) {
		printf("\n%s: ERROR: max per-device stream ID count exceed\n",
		       __func__);
		return;
	}

	for (i = 0; i < count; i++)
		stream_ids[i] = start_id++;
}

/*
 * This function updates the mmu-masters property on the SMMU
 * node as per the SMMU binding-- phandle and list of stream IDs
 * for each MMU master.
 */
void append_mmu_masters(void *blob, const char *smmu_path,
			const char *master_name, u32 *stream_ids, int count)
{
	u32 phandle;
	int smmu_nodeoffset;
	int master_nodeoffset;
	int i;

	/* get phandle of mmu master device */
	master_nodeoffset = fdt_path_offset(blob, master_name);
	if (master_nodeoffset < 0) {
		printf("\n%s: ERROR: master not found\n", __func__);
		return;
	}
	phandle = fdt_get_phandle(blob, master_nodeoffset);
	if (!phandle) { /* if master has no phandle, create one */
		phandle = fdt_create_phandle(blob, master_nodeoffset);
		if (!phandle) {
			printf("\n%s: ERROR: unable to create phandle\n",
			       __func__);
			return;
		}
	}

	/* append it to mmu-masters */
	smmu_nodeoffset = fdt_path_offset(blob, smmu_path);
	if (fdt_appendprop_u32(blob, smmu_nodeoffset, "mmu-masters",
			       phandle) < 0) {
		printf("\n%s: ERROR: unable to update SMMU node\n", __func__);
		return;
	}

	/* for each stream ID, append to mmu-masters */
	for (i = 0; i < count; i++) {
		fdt_appendprop_u32(blob, smmu_nodeoffset, "mmu-masters",
				   stream_ids[i]);
	}

	/* fix up #stream-id-cells with stream ID count */
	if (fdt_setprop_u32(blob, master_nodeoffset, "#stream-id-cells",
			    count) < 0)
		printf("\n%s: ERROR: unable to update #stream-id-cells\n",
		       __func__);
}


/*
 * The info below summarizes how streamID partitioning works
 * for ls2080a and how it is conveyed to the OS via the device tree.
 *
 *  -non-PCI legacy, platform devices (USB, SD/MMC, SATA, DMA)
 *     -all legacy devices get a unique ICID assigned and programmed in
 *      their AMQR registers by u-boot
 *     -u-boot updates the hardware device tree with streamID properties
 *      for each platform/legacy device (smmu-masters property)
 *
 *  -PCIe
 *     -for each PCI controller that is active (as per RCW settings),
 *      u-boot will allocate a range of ICID and convey that to Linux via
 *      the device tree (smmu-masters property)
 *
 *  -DPAA2
 *     -u-boot will allocate a range of ICIDs to be used by the Management
 *      Complex for containers and will set these values in the MC DPC image.
 *     -the MC is responsible for allocating and setting up ICIDs
 *      for all DPAA2 devices.
 *
 */
#ifdef CONFIG_FSL_LSCH3
static void fdt_fixup_smmu(void *blob)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(blob, "/iommu@5000000");
	if (nodeoffset < 0) {
		printf("\n%s: WARNING: no SMMU node found\n", __func__);
		return;
	}

	/* fixup for all PCI controllers */
#ifdef CONFIG_PCI
	fdt_fixup_smmu_pcie(blob);
#endif
}
#endif

void ft_cpu_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_MP
	ft_fixup_cpu(blob);
#endif

#ifdef CONFIG_SYS_NS16550
	do_fixup_by_compat_u32(blob, "fsl,ns16550",
			       "clock-frequency", CONFIG_SYS_NS16550_CLK, 1);
#endif

	do_fixup_by_compat_u32(blob, "fixed-clock",
			       "clock-frequency", CONFIG_SYS_CLK_FREQ, 1);

#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif

#ifdef CONFIG_FSL_ESDHC
	fdt_fixup_esdhc(blob, bd);
#endif

#ifdef CONFIG_FSL_LSCH3
	fdt_fixup_smmu(blob);
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_firmware(blob);
#endif
}
