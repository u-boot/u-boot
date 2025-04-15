/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __ASM_ACPI_TABLE_H__
#define __ASM_ACPI_TABLE_H__

#ifndef __ACPI__
#ifndef __ASSEMBLY__

#include <acpi/acpi_table.h>

/**
 * acpi_write_madt_gicc() - Write out a MADT GICC sub-table
 *
 * Write out the GIC CPU Interface sub-table.
 *
 * @gicc:             Pointer to place to put the sub-table
 * @cpu_num:          GIC's CPU Interface Number
 * @perf_gsiv:        The GSIV used for Performance Monitoring Interrupts
 * @phys_base:        Address at which the processor can access this
 *                    GIC CPU Interface
 * @gicv:             Address of the GIC virtual CPU interface registers
 * @gich:             Address of the GIC virtual interface control block
 *                    registers
 * @vgic_maint_irq:   GSIV for Virtual GIC maintenance interrupt
 * @gicr_base:        Physical address of the associated Redistributor
 * @mpidr:            MPIDR as defined by ARM architecture
 * @efficiency:       Describes the relative power efficiency
 */
void acpi_write_madt_gicc(struct acpi_madt_gicc *gicc, uint cpu_num,
			  uint perf_gsiv, ulong phys_base, ulong gicv,
			  ulong gich, uint vgic_maint_irq, u64 gicr_base,
			  ulong mpidr, uint efficiency);

/**
 * acpi_write_madt_gicd() - Write out a MADT GICD sub-table
 *
 * Write out the GIC Distributor sub-table.
 *
 * @gicd:            Pointer to place to put the sub-table
 * @gic_id:          This GIC Distributor's hardware ID
 * @phys_base:       The 64-bit physical address for this Distributor
 * @gic_version:     GIC version
 */
void acpi_write_madt_gicd(struct acpi_madt_gicd *gicd, uint gic_id,
			  ulong phys_base, uint gic_version);

/**
 * acpi_write_madt_gicr() - Write out a MADT GICR sub-table
 *
 * Write out the GIC Redistributor sub-table.
 *
 * @gicr:                            Pointer to place to put the sub-table
 * @discovery_range_base_address:    Physical address of a page range
 *                                   containing all GIC Redistributors
 * @discovery_range_length:          Length of the GIC Redistributor
 *                                   Discovery page range
 */
void acpi_write_madt_gicr(struct acpi_madt_gicr *gicr,
			  u64 discovery_range_base_address,
			  u32 discovery_range_length);

/**
 * acpi_write_madt_its() - Write out a MADT ITS sub-table
 *
 * Write out the GIC Interrupt Translation Service sub-table.
 *
 * @its:                    Pointer to place to put the sub-table
 * @its_id:                 Uniqueue GIC ITS ID
 * @physical_base_address:  Physical address for the Interrupt
 *                          Translation Service
 */
void acpi_write_madt_its(struct acpi_madt_its *its,
			 u32 its_id,
			 u64 physical_base_address);

/**
 * acpi_pptt_add_proc() - Write out a PPTT processor sub-table
 *
 * Write out the Processor Properties Topology Table processor sub-table.
 *
 * @ctx:                   ACPI context pointer
 * @flags:                 Processor Structure Flags
 * @parent:                Reference to parent processor
 * @proc_id:               ACPI processor ID as defined in MADT
 * @num_resources:         Number of resource structure references
 * @resource_list:         References to other PPTT structures
 * Return: offset from start of PPTT in bytes
 */
int acpi_pptt_add_proc(struct acpi_ctx *ctx, const u32 flags, const u32 parent,
		       const u32 proc_id, const u32 num_resources,
		       const u32 *resource_list);

/**
 * acpi_pptt_add_cache() - Write out a PPTT cache sub-table
 *
 * Write out the Processor Properties Topology Table cache sub-table.
 *
 * @ctx: ACPI context pointer
 * @flags:                Cache Structure Flags
 * @next_cache_level:     Reference to next level of cache
 * @size:                 Size of the cache in bytes
 * @sets:                 Number of sets in the cache
 * @assoc:                Integer number of ways
 * @attributes:           Allocation type, Cache type, policy
 * @line_size:            Line size in bytes
 * Return: offset from start of PPTT in bytes
 */
int acpi_pptt_add_cache(struct acpi_ctx *ctx, const u32 flags,
			const u32 next_cache_level, const u32 size,
			const u32 sets, const u8 assoc, const u8 attributes,
			const u16 line_size);

/* Multi-processor Startup for ARM Platforms */
/**
 * struct acpi_pp_page - MP startup handshake mailbox
 *
 * Defines a 4096 byte memory region that is used for starting secondary CPUs on
 * an Arm system that follows the "Multi-processor Startup for ARM Platforms" spec.
 *
 * @cpu_id:           MPIDR as returned by the Multiprocessor Affinity Register.
 *                    On 32bit Arm systems the upper bits are unused.
 * @jumping_address:  On 32bit Arm systems the address must be below 4 GiB
 * @os_reserved:      Reserved for OS use. Firmware must not access this memory.
 * @spinning_code:    Reserved for firmware use. OS must not access this memory.
 *                    The spinning code will be installed by firmware and the secondary
 *                    CPUs will enter it before the control is handed over to the OS.
 */
struct acpi_pp_page {
	u64 cpu_id;
	u64 jumping_address;
	u8 os_reserved[2032];
	u8 spinning_code[2048];
} __packed;

#endif /* !__ASSEMBLY__ */
#endif /* !__ACPI__ */

/* Multi-processor Startup for ARM Platforms defines */
#define ACPI_PP_CPU_ID_INVALID		0xffffffff
#define ACPI_PP_JMP_ADR_INVALID		0
#define ACPI_PP_PAGE_SIZE		4096
#define ACPI_PP_CPU_ID_OFFSET		0
#define ACPI_PP_CPU_JMP_OFFSET		8
#define ACPI_PP_CPU_CODE_OFFSET		2048
#define ACPI_PP_VERSION			1

#endif /* __ASM_ACPI_TABLE_H__ */
