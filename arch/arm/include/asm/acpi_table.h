/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __ASM_ACPI_TABLE_H__
#define __ASM_ACPI_TABLE_H__

#ifndef __ACPI__
#ifndef __ASSEMBLY__

void acpi_write_madt_gicc(struct acpi_madt_gicc *gicc, uint cpu_num,
			  uint perf_gsiv, ulong phys_base, ulong gicv,
			  ulong gich, uint vgic_maint_irq, ulong mpidr,
			  uint efficiency);

void acpi_write_madt_gicd(struct acpi_madt_gicd *gicd, uint gic_id,
			  ulong phys_base, uint gic_version);

/* Multi-processor Startup for ARM Platforms */
struct acpi_parking_protocol_page {
	u32 cpu_id;
	u32 reserved;
	u64 jumping_address;
	u8 os_reserved[2032];
	u8 cpu_spinning_code[2048];
} __packed;

/* Architecture specific functions */
/**
 * acpi_parking_protocol_code_size - Spinloop code size *
 */
extern u32 acpi_parking_protocol_code_size;

/**
 * parking_protocol_code_start() - Spinloop code
 *
 * Architectural spinloop code to be installed in each parking protocol
 * page. The spinloop code must be less than 2048 bytes.
 *
 * The spinloop code will be entered after calling
 * acpi_parking_protocol_install().
 *
 */
void acpi_parking_protocol_code_start(void);

/**
 * acpi_parking_protocol_install() - Installs the parking protocol.
 *
 * Installs the reserved memory containing the spinloop code and the
 * OS mailbox as required by the ACPI Multi-processor Startup for
 * ARM Platforms specification.
 *
 * The secondary CPUs will wait for this function to be called in order
 * to enter the spinloop code residing in the tables.
 *
 * @tables: ACPI parking protocol tables.
 * @num_cpus: Number of allocated pages.
 */
void acpi_parking_protocol_install(uintptr_t tables, size_t num_cpus);

#endif /* !__ASSEMBLY__ */
#endif /* !__ACPI__ */

#define ACPI_PP_CPU_ID_INVALID		0xffffffff
#define ACPI_PP_JMP_ADR_INVALID		0
#define ACPI_PP_PAGE_SIZE		4096
#define ACPI_PP_CPU_ID_OFFSET		0
#define ACPI_PP_CPU_JMP_ADDR_OFFSET	8
#define ACPI_PP_CPU_CODE_OFFSET		2048
#define ACPI_PP_VERSION			1

#endif /* __ASM_ACPI_TABLE_H__ */
