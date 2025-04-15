/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Helpers for ACPI table generation
 *
 * Based on acpi.c from coreboot
 *
 * Copyright 2019 Google LLC
 *
 * Copyright (C) 2015, Saket Sinha <saket.sinha89@gmail.com>
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __ACPI_TABLE_H__
#define __ACPI_TABLE_H__

#include <dm/acpi.h>

#define RSDP_SIG		"RSD PTR "	/* RSDP pointer signature */
#define OEM_ID			"U-BOOT"	/* U-Boot */
#define OEM_TABLE_ID		"U-BOOTBL"	/* U-Boot Table */
#define ASLC_ID			"INTL"		/* Intel ASL Compiler */

/* TODO(sjg@chromium.org): Figure out how to get compiler revision */
#define ASL_REVISION	0

#define ACPI_RSDP_REV_ACPI_1_0	0
#define ACPI_RSDP_REV_ACPI_2_0	2

#if !defined(__ACPI__)

#include <linux/bitops.h>

struct acpi_ctx;

/*
 * RSDP (Root System Description Pointer)
 * Note: ACPI 1.0 didn't have length, xsdt_address, and ext_checksum
 */
struct __packed acpi_rsdp {
	char signature[8];	/* RSDP signature */
	u8 checksum;		/* Checksum of the first 20 bytes */
	char oem_id[6];		/* OEM ID */
	u8 revision;		/* 0 for ACPI 1.0, others 2 */
	u32 rsdt_address;	/* Physical address of RSDT (32 bits) */
	u32 length;		/* Total RSDP length (incl. extended part) */
	u64 xsdt_address;	/* Physical address of XSDT (64 bits) */
	u8 ext_checksum;	/* Checksum of the whole table */
	u8 reserved[3];
};

/* Generic ACPI header, provided by (almost) all tables */
struct __packed acpi_table_header {
	char signature[ACPI_NAME_LEN];	/* ACPI signature (4 ASCII chars) */
	u32 length;		/* Table length in bytes (incl. header) */
	u8 revision;		/* Table version (not ACPI version!) */
	volatile u8 checksum;	/* To make sum of entire table == 0 */
	char oem_id[6];		/* OEM identification */
	char oem_table_id[8];	/* OEM table identification */
	u32 oem_revision;	/* OEM revision number */
	char creator_id[4];	/* ASL compiler vendor ID */
	u32 creator_revision;	/* ASL compiler revision number */
};

/**
 * struct acpi_gen_regaddr - generic address structure (GAS)
 */
struct acpi_gen_regaddr {
	/**
	 * @space_id: address space ID
	 *
	 * See table "Operation Region Address Space Identifiers" in the ACPI
	 * specification.
	 */
	u8 space_id;
	/** @bit_width: size in bits of the register */
	u8 bit_width;
	/** @bit_offset: bit offset of the register */
	u8 bit_offset;
	/**
	 * @access_size: access size
	 *
	 * * 0 - undefined
	 * * 1 - byte access
	 * * 2 - word (2 bytes) access
	 * * 3 - Dword (4 bytes) access
	 * * 4 - Qword (8 bytes) access
	 *
	 * See ACPI_ACCESS_SIZE_*_ACCESS macros.
	 */
	u8 access_size;
	/** @addrl: register address, low 32 bits */
	u32 addrl;
	/** @addrh: register address, high 32 bits */
	u32 addrh;
};

/* A maximum number of 32 ACPI tables ought to be enough for now */
#define MAX_ACPI_TABLES		32

/* RSDT (Root System Description Table) */
struct acpi_rsdt {
	struct acpi_table_header header;
	u32 entry[MAX_ACPI_TABLES];
};

/* XSDT (Extended System Description Table) */
struct __packed acpi_xsdt {
	struct acpi_table_header header;
	u64 entry[MAX_ACPI_TABLES];
};

/**
 * struct acpi_hpet: High Precision Event Timers (HETP)
 *
 * The structure is defined in the
 * "IA-PC HPET (High Precision Event Timers) Specification", rev 1.0a, Oct 2004
 */
struct acpi_hpet {
	/** @header: table header */
	struct acpi_table_header header;
	/** @id hardware ID of Event Timer Block */
	u32 id;
	/** @addr: address of Event Timer Block */
	struct acpi_gen_regaddr addr;
	/** @number: HPET sequence number */
	u8 number;
	/** @min_tick: minimum clock ticks without lost interrupts */
	u16 min_tick;
	/** @attributes: page protection and OEM atttribute */
	u8 attributes;
} __packed;

struct __packed acpi_tpm2 {
	struct acpi_table_header header;
	u16 platform_class;
	u8  reserved[2];
	u64 control_area;
	u32 start_method;
	u8  msp[12];
	u32 laml;
	u64 lasa;
};

struct __packed acpi_tcpa {
	struct acpi_table_header header;
	u16 platform_class;
	u32 laml;
	u64 lasa;
};

/* FADT Preferred Power Management Profile */
enum acpi_pm_profile {
	ACPI_PM_UNSPECIFIED = 0,
	ACPI_PM_DESKTOP,
	ACPI_PM_MOBILE,
	ACPI_PM_WORKSTATION,
	ACPI_PM_ENTERPRISE_SERVER,
	ACPI_PM_SOHO_SERVER,
	ACPI_PM_APPLIANCE_PC,
	ACPI_PM_PERFORMANCE_SERVER,
	ACPI_PM_TABLET
};

/* FADT flags for p_lvl2_lat and p_lvl3_lat */
#define ACPI_FADT_C2_NOT_SUPPORTED	101
#define ACPI_FADT_C3_NOT_SUPPORTED	1001

/* FADT Boot Architecture Flags */
#define ACPI_FADT_LEGACY_FREE		0x00
#define ACPI_FADT_LEGACY_DEVICES	BIT(0)
#define ACPI_FADT_8042			BIT(1)
#define ACPI_FADT_VGA_NOT_PRESENT	BIT(2)
#define ACPI_FADT_MSI_NOT_SUPPORTED	BIT(3)
#define ACPI_FADT_NO_PCIE_ASPM_CONTROL	BIT(4)

/* FADT Feature Flags */
#define ACPI_FADT_WBINVD		BIT(0)
#define ACPI_FADT_WBINVD_FLUSH		BIT(1)
#define ACPI_FADT_C1_SUPPORTED		BIT(2)
#define ACPI_FADT_C2_MP_SUPPORTED	BIT(3)
#define ACPI_FADT_POWER_BUTTON		BIT(4)
#define ACPI_FADT_SLEEP_BUTTON		BIT(5)
#define ACPI_FADT_FIXED_RTC		BIT(6)
#define ACPI_FADT_S4_RTC_WAKE		BIT(7)
#define ACPI_FADT_32BIT_TIMER		BIT(8)
#define ACPI_FADT_DOCKING_SUPPORTED	BIT(9)
#define ACPI_FADT_RESET_REGISTER	BIT(10)
#define ACPI_FADT_SEALED_CASE		BIT(11)
#define ACPI_FADT_HEADLESS		BIT(12)
#define ACPI_FADT_SLEEP_TYPE		BIT(13)
#define ACPI_FADT_PCI_EXPRESS_WAKE	BIT(14)
#define ACPI_FADT_PLATFORM_CLOCK	BIT(15)
#define ACPI_FADT_S4_RTC_VALID		BIT(16)
#define ACPI_FADT_REMOTE_POWER_ON	BIT(17)
#define ACPI_FADT_APIC_CLUSTER		BIT(18)
#define ACPI_FADT_APIC_PHYSICAL		BIT(19)
#define ACPI_FADT_HW_REDUCED_ACPI	BIT(20)
#define ACPI_FADT_LOW_PWR_IDLE_S0	BIT(21)

/* ARM boot flags */
#define ACPI_ARM_PSCI_COMPLIANT		BIT(0)

enum acpi_address_space_type {
	ACPI_ADDRESS_SPACE_MEMORY = 0,	/* System memory */
	ACPI_ADDRESS_SPACE_IO,		/* System I/O */
	ACPI_ADDRESS_SPACE_PCI,		/* PCI config space */
	ACPI_ADDRESS_SPACE_EC,		/* Embedded controller */
	ACPI_ADDRESS_SPACE_SMBUS,	/* SMBus */
	ACPI_ADDRESS_SPACE_PCC = 0x0a,	/* Platform Comm. Channel */
	ACPI_ADDRESS_SPACE_FIXED = 0x7f	/* Functional fixed hardware */
};

enum acpi_address_space_size {
	ACPI_ACCESS_SIZE_UNDEFINED = 0,
	ACPI_ACCESS_SIZE_BYTE_ACCESS,
	ACPI_ACCESS_SIZE_WORD_ACCESS,
	ACPI_ACCESS_SIZE_DWORD_ACCESS,
	ACPI_ACCESS_SIZE_QWORD_ACCESS
};

/* FADT (Fixed ACPI Description Table) */
struct __packed acpi_fadt {
	struct acpi_table_header header;
	u32 firmware_ctrl;
	u32 dsdt;
	u8 res1;
	u8 preferred_pm_profile;
	u16 sci_int;
	u32 smi_cmd;
	u8 acpi_enable;
	u8 acpi_disable;
	u8 s4bios_req;
	u8 pstate_cnt;
	u32 pm1a_evt_blk;
	u32 pm1b_evt_blk;
	u32 pm1a_cnt_blk;
	u32 pm1b_cnt_blk;
	u32 pm2_cnt_blk;
	u32 pm_tmr_blk;
	u32 gpe0_blk;
	u32 gpe1_blk;
	u8 pm1_evt_len;
	u8 pm1_cnt_len;
	u8 pm2_cnt_len;
	u8 pm_tmr_len;
	u8 gpe0_blk_len;
	u8 gpe1_blk_len;
	u8 gpe1_base;
	u8 cst_cnt;
	u16 p_lvl2_lat;
	u16 p_lvl3_lat;
	u16 flush_size;
	u16 flush_stride;
	u8 duty_offset;
	u8 duty_width;
	u8 day_alrm;
	u8 mon_alrm;
	u8 century;
	u16 iapc_boot_arch;
	u8 res2;
	u32 flags;
	struct acpi_gen_regaddr reset_reg;
	u8 reset_value;
	u16 arm_boot_arch;
	u8 minor_revision;
	u64 x_firmware_ctrl;
	u64 x_dsdt;
	struct acpi_gen_regaddr x_pm1a_evt_blk;
	struct acpi_gen_regaddr x_pm1b_evt_blk;
	struct acpi_gen_regaddr x_pm1a_cnt_blk;
	struct acpi_gen_regaddr x_pm1b_cnt_blk;
	struct acpi_gen_regaddr x_pm2_cnt_blk;
	struct acpi_gen_regaddr x_pm_tmr_blk;
	struct acpi_gen_regaddr x_gpe0_blk;
	struct acpi_gen_regaddr x_gpe1_blk;
	struct acpi_gen_regaddr sleep_control_reg;
	struct acpi_gen_regaddr sleep_status_reg;
	u64 hyp_vendor_id;
};

/* FADT TABLE Revision values - note these do not match the ACPI revision */
#define ACPI_FADT_REV_ACPI_1_0		1
#define ACPI_FADT_REV_ACPI_2_0		3
#define ACPI_FADT_REV_ACPI_3_0		4
#define ACPI_FADT_REV_ACPI_4_0		4
#define ACPI_FADT_REV_ACPI_5_0		5
#define ACPI_FADT_REV_ACPI_6_0		6

/* MADT TABLE Revision values - note these do not match the ACPI revision */
#define ACPI_MADT_REV_ACPI_3_0		2
#define ACPI_MADT_REV_ACPI_4_0		3
#define ACPI_MADT_REV_ACPI_5_0		3
#define ACPI_MADT_REV_ACPI_6_2		4
#define ACPI_MADT_REV_ACPI_6_3		5

#define ACPI_MCFG_REV_ACPI_3_0		1

/* IVRS Revision Field */
#define IVRS_FORMAT_FIXED	0x01	/* Type 10h & 11h only */
#define IVRS_FORMAT_MIXED	0x02	/* Type 10h, 11h, & 40h */

/* FACS flags */
#define ACPI_FACS_S4BIOS_F		BIT(0)
#define ACPI_FACS_64BIT_WAKE_F		BIT(1)

/* FACS (Firmware ACPI Control Structure) */
struct acpi_facs {
	char signature[ACPI_NAME_LEN];	/* "FACS" */
	u32 length;			/* Length in bytes (>= 64) */
	u32 hardware_signature;		/* Hardware signature */
	u32 firmware_waking_vector;	/* Firmware waking vector */
	u32 global_lock;		/* Global lock */
	u32 flags;			/* FACS flags */
	u32 x_firmware_waking_vector_l;	/* X FW waking vector, low */
	u32 x_firmware_waking_vector_h;	/* X FW waking vector, high */
	u8 version;			/* Version 2 */
	u8 res1[3];
	u32 ospm_flags;			/* OSPM enabled flags */
	u8 res2[24];
};

/* MADT flags */
#define ACPI_MADT_PCAT_COMPAT	BIT(0)

/* MADT (Multiple APIC Description Table) */
struct acpi_madt {
	struct acpi_table_header header;
	u32 lapic_addr;			/* Local APIC address */
	u32 flags;			/* Multiple APIC flags */
};

/* MADT: APIC Structure Type*/
enum acpi_apic_types {
	ACPI_APIC_LAPIC	= 0,		/* Processor local APIC */
	ACPI_APIC_IOAPIC,		/* I/O APIC */
	ACPI_APIC_IRQ_SRC_OVERRIDE,	/* Interrupt source override */
	ACPI_APIC_NMI_SRC,		/* NMI source */
	ACPI_APIC_LAPIC_NMI,		/* Local APIC NMI */
	ACPI_APIC_LAPIC_ADDR_OVERRIDE,	/* Local APIC address override */
	ACPI_APIC_IOSAPIC,		/* I/O SAPIC */
	ACPI_APIC_LSAPIC,		/* Local SAPIC */
	ACPI_APIC_PLATFORM_IRQ_SRC,	/* Platform interrupt sources */
	ACPI_APIC_LX2APIC,		/* Processor local x2APIC */
	ACPI_APIC_LX2APIC_NMI,		/* Local x2APIC NMI */
	ACPI_APIC_GICC,			/* Generic Interrupt Ctlr CPU i/f */
	ACPI_APIC_GICD,			/* Generic Interrupt Ctlr Distributor */
	ACPI_APIC_MSI_FRAME,		/* Generic Interrupt MSI Frame */
	ACPI_APIC_GICR,			/* Generic Interrupt Ctlr Redistributor */
	ACPI_APIC_ITS,			/* Interrupt Translation Service */
};

/* MADT: Processor Local APIC Structure */

#define LOCAL_APIC_FLAG_ENABLED		BIT(0)

struct acpi_madt_lapic {
	u8 type;		/* Type (0) */
	u8 length;		/* Length in bytes (8) */
	u8 processor_id;	/* ACPI processor ID */
	u8 apic_id;		/* Local APIC ID */
	u32 flags;		/* Local APIC flags */
};

/* MADT: I/O APIC Structure */
struct acpi_madt_ioapic {
	u8 type;		/* Type (1) */
	u8 length;		/* Length in bytes (12) */
	u8 ioapic_id;		/* I/O APIC ID */
	u8 reserved;
	u32 ioapic_addr;	/* I/O APIC address */
	u32 gsi_base;		/* Global system interrupt base */
};

/* MADT: Interrupt Source Override Structure */
struct __packed acpi_madt_irqoverride {
	u8 type;		/* Type (2) */
	u8 length;		/* Length in bytes (10) */
	u8 bus;			/* ISA (0) */
	u8 source;		/* Bus-relative int. source (IRQ) */
	u32 gsirq;		/* Global system interrupt */
	u16 flags;		/* MPS INTI flags */
};

/* MADT: Local APIC NMI Structure */
struct __packed acpi_madt_lapic_nmi {
	u8 type;		/* Type (4) */
	u8 length;		/* Length in bytes (6) */
	u8 processor_id;	/* ACPI processor ID */
	u16 flags;		/* MPS INTI flags */
	u8 lint;		/* Local APIC LINT# */
};

/* flags for acpi_madt_gicc flags word */
enum {
	ACPI_MADTF_ENABLED	= BIT(0),
	ACPI_MADTF_PERF		= BIT(1),
	ACPI_MADTF_VGIC		= BIT(2),
};

/**
 * struct __packed acpi_madt_gicc - GIC CPU interface (type 0xb)
 *
 * This holds information about the Generic Interrupt Controller (GIC) CPU
 * interface. See ACPI Spec v6.3 section 5.2.12.14
 */
struct acpi_madt_gicc {
	u8 type;
	u8 length;
	u16 reserved;
	u32 cpu_if_num;
	u32 processor_id;
	u32 flags;
	u32 parking_proto;
	u32 perf_gsiv;
	u64 parked_addr;
	u64 phys_base;
	u64 gicv;
	u64 gich;
	u32 vgic_maint_irq;
	u64 gicr_base;
	u64 mpidr;
	u8 efficiency;
	u8 reserved2;
	u16 spi_overflow_irq;
} __packed;

/**
 * struct __packed acpi_madt_gicc - GIC distributor (type 0xc)
 *
 * This holds information about the Generic Interrupt Controller (GIC)
 * Distributor interface. See ACPI Spec v6.3 section 5.2.12.15
 */
struct acpi_madt_gicd {
	u8 type;
	u8 length;
	u16 reserved;
	u32 gic_id;
	u64 phys_base;
	u32 reserved2;
	u8 gic_version;
	u8 reserved3[3];
} __packed;

/**
 * struct __packed acpi_madt_gicr - GIC Redistributor (type 0xe)
 *
 * This holds information about the Generic Interrupt Controller (GIC)
 * Redistributor interface. See ACPI Spec v6.3 section 5.2.12.17
 */
struct acpi_madt_gicr {
	u8 type;
	u8 length;
	u16 reserved;
	u64 discovery_range_base_address;
	u32 discovery_range_length;
} __packed;

/**
 * struct __packed acpi_madt_its - GIC Interrupt Translation Service (type 0xf)
 *
 * This holds information about the Interrupt Translation Service (ITS)
 * Structure. See ACPI Spec v6.3 section 5.2.12.18
 */
struct acpi_madt_its {
	u8 type;
	u8 length;
	u16 reserved;
	u32 gic_its_id;
	u64 physical_base_address;
	u32 reserved2;
} __packed;

/* MCFG (PCI Express MMIO config space BAR description table) */
struct acpi_mcfg {
	struct acpi_table_header header;
	u8 reserved[8];
};

struct acpi_mcfg_mmconfig {
	u32 base_address_l;
	u32 base_address_h;
	u16 pci_segment_group_number;
	u8 start_bus_number;
	u8 end_bus_number;
	u8 reserved[4];
};

/* PM1_CNT bit defines */
#define PM1_CNT_SCI_EN		BIT(0)

/* ACPI global NVS structure */
struct acpi_global_nvs;

/* CSRT (Core System Resource Table) */
struct acpi_csrt {
	struct acpi_table_header header;
};

/**
 * struct acpi_csrt_group - header for a group within the CSRT
 *
 * The CSRT consists of one or more groups and this is the header for each
 *
 * See Core System Resources Table (CSRT), March 13, 2017, Microsoft Corporation
 * for details
 *
 * https://uefi.org/sites/default/files/resources/CSRT%20v2.pdf
 *
 * @shared_info_length indicates the number of shared-info bytes following this
 * struct (which may be 0)
 */
struct acpi_csrt_group {
	u32 length;
	u32 vendor_id;
	u32 subvendor_id;
	u16 device_id;
	u16 subdevice_id;
	u16 revision;
	u16 reserved;
	u32 shared_info_length;
};

/**
 * struct acpi_csrt_descriptor - describes the information that follows
 *
 * See the spec as above for details
 */
struct acpi_csrt_descriptor {
	u32 length;
	u16 type;
	u16 subtype;
	u32 uid;
};

/**
 * struct acpi_csrt_shared_info - shared info for Intel tangier
 *
 * This provides the shared info for this particular board. Notes that the CSRT
 * does not describe the format of data, so this format may not be used by any
 * other board.
 */
struct acpi_csrt_shared_info {
	u16 major_version;
	u16 minor_version;
	u32 mmio_base_low;
	u32 mmio_base_high;
	u32 gsi_interrupt;
	u8 interrupt_polarity;
	u8 interrupt_mode;
	u8 num_channels;
	u8 dma_address_width;
	u16 base_request_line;
	u16 num_handshake_signals;
	u32 max_block_size;
};

/* Port types for ACPI _UPC object */
enum acpi_upc_type {
	UPC_TYPE_A,
	UPC_TYPE_MINI_AB,
	UPC_TYPE_EXPRESSCARD,
	UPC_TYPE_USB3_A,
	UPC_TYPE_USB3_B,
	UPC_TYPE_USB3_MICRO_B,
	UPC_TYPE_USB3_MICRO_AB,
	UPC_TYPE_USB3_POWER_B,
	UPC_TYPE_C_USB2_ONLY,
	UPC_TYPE_C_USB2_SS_SWITCH,
	UPC_TYPE_C_USB2_SS,
	UPC_TYPE_PROPRIETARY = 0xff,
	/*
	 * The following types are not directly defined in the ACPI
	 * spec but are used by coreboot to identify a USB device type.
	 */
	UPC_TYPE_INTERNAL = 0xff,
	UPC_TYPE_UNUSED,
	UPC_TYPE_HUB
};

enum dev_scope_type {
	SCOPE_PCI_ENDPOINT = 1,
	SCOPE_PCI_SUB = 2,
	SCOPE_IOAPIC = 3,
	SCOPE_MSI_HPET = 4,
	SCOPE_ACPI_NAMESPACE_DEVICE = 5
};

struct __packed dev_scope {
	u8 type;
	u8 length;
	u8 reserved[2];
	u8 enumeration;
	u8 start_bus;
	struct {
		u8 dev;
		u8 fn;
	} __packed path[0];
};

enum dmar_type {
	DMAR_DRHD = 0,
	DMAR_RMRR = 1,
	DMAR_ATSR = 2,
	DMAR_RHSA = 3,
	DMAR_ANDD = 4
};

enum {
	DRHD_INCLUDE_PCI_ALL = BIT(0)
};

enum dmar_flags {
	DMAR_INTR_REMAP			= BIT(0),
	DMAR_X2APIC_OPT_OUT		= BIT(1),
	DMAR_CTRL_PLATFORM_OPT_IN_FLAG	= BIT(2),
};

struct dmar_entry {
	u16 type;
	u16 length;
	u8 flags;
	u8 reserved;
	u16 segment;
	u64 bar;
};

struct dmar_rmrr_entry {
	u16 type;
	u16 length;
	u16 reserved;
	u16 segment;
	u64 bar;
	u64 limit;
};

/* DMAR (DMA Remapping Reporting Structure) */
struct __packed acpi_dmar {
	struct acpi_table_header header;
	u8 host_address_width;
	u8 flags;
	u8 reserved[10];
	struct dmar_entry structure[0];
};

/* DBG2 definitions are partially used for SPCR interface_type */

/* Types for port_type field */

#define ACPI_DBG2_SERIAL_PORT		0x8000
#define ACPI_DBG2_1394_PORT		0x8001
#define ACPI_DBG2_USB_PORT		0x8002
#define ACPI_DBG2_NET_PORT		0x8003

/* Subtypes for port_subtype field */

#define ACPI_DBG2_16550_COMPATIBLE	0x0000
#define ACPI_DBG2_16550_SUBSET		0x0001
#define ACPI_DBG2_ARM_PL011		0x0003
#define ACPI_DBG2_ARM_SBSA_32BIT	0x000D
#define ACPI_DBG2_ARM_SBSA_GENERIC	0x000E
#define ACPI_DBG2_ARM_DCC		0x000F
#define ACPI_DBG2_BCM2835		0x0010

#define ACPI_DBG2_1394_STANDARD		0x0000

#define ACPI_DBG2_USB_XHCI		0x0000
#define ACPI_DBG2_USB_EHCI		0x0001

#define ACPI_DBG2_UNKNOWN		0x00FF

/* DBG2: Microsoft Debug Port Table 2 header */
struct __packed acpi_dbg2_header {
	struct acpi_table_header header;
	u32 devices_offset;
	u32 devices_count;
};

/* DBG2: Microsoft Debug Port Table 2 device entry */
struct __packed acpi_dbg2_device {
	u8  revision;
	u16 length;
	u8 address_count;
	u16 namespace_string_length;
	u16 namespace_string_offset;
	u16 oem_data_length;
	u16 oem_data_offset;
	u16 port_type;
	u16 port_subtype;
	u8  reserved[2];
	u16 base_address_offset;
	u16 address_size_offset;
};

/* SPCR (Serial Port Console Redirection table) */
struct __packed acpi_spcr {
	struct acpi_table_header header;
	u8 interface_type;
	u8 reserved[3];
	struct acpi_gen_regaddr serial_port;
	u8 interrupt_type;
	u8 pc_interrupt;
	u32 interrupt;		/* Global system interrupt */
	u8 baud_rate;
	u8 parity;
	u8 stop_bits;
	u8 flow_control;
	u8 terminal_type;
	u8 reserved1;
	u16 pci_device_id;	/* Must be 0xffff if not PCI device */
	u16 pci_vendor_id;	/* Must be 0xffff if not PCI device */
	u8 pci_bus;
	u8 pci_device;
	u8 pci_function;
	u32 pci_flags;
	u8 pci_segment;
	u32 reserved2;
};

/**
 * struct acpi_gtdt - Generic Timer Description Table (GTDT)
 *
 * See ACPI Spec v6.3 section 5.2.24 for details
 */
struct acpi_gtdt {
	struct acpi_table_header header;
	u64 cnt_ctrl_base;
	u32 reserved0;
	u32 sec_el1_gsiv;
	u32 sec_el1_flags;
	u32 el1_gsiv;
	u32 el1_flags;
	u32 virt_el1_gsiv;
	u32 virt_el1_flags;
	u32 el2_gsiv;
	u32 el2_flags;
	u64 cnt_read_base;
	u32 plat_timer_count;
	u32 plat_timer_offset;
	u32 virt_el2_gsiv;
	u32 virt_el2_flags;
} __packed;

#define GTDT_FLAG_INT_ACTIVE_LOW	BIT(1)

/**
 * struct acpi_bgrt -  Boot Graphics Resource Table (BGRT)
 *
 * Optional table that provides a mechanism to indicate that an image was drawn
 * on the screen during boot, and some information about the image.
 *
 * See ACPI Spec v6.3 section 5.2.22 for details
 */
struct acpi_bgrt {
	struct acpi_table_header header;
	u16 version;
	u8 status;
	u8 image_type;
	u64 addr;
	u32 offset_x;
	u32 offset_y;
} __packed;

/* Types for PPTT */
#define ACPI_PPTT_TYPE_PROC		0
#define ACPI_PPTT_TYPE_CACHE		1

/* Flags for PPTT */
#define ACPI_PPTT_PHYSICAL_PACKAGE	BIT(0)
#define ACPI_PPTT_PROC_ID_VALID		BIT(1)
#define ACPI_PPTT_PROC_IS_THREAD	BIT(2)
#define ACPI_PPTT_NODE_IS_LEAF		BIT(3)
#define ACPI_PPTT_CHILDREN_IDENTICAL	BIT(4)

/**
 * struct acpi_pptt_header - Processor Properties Topology Table (PPTT) header
 *
 * Describes the topological structure of processors and their shared resources,
 * such as caches.
 *
 * See ACPI Spec v6.3 section 5.2.29 for details
 */
struct acpi_pptt_header {
	u8 type;	/* ACPI_PPTT_TYPE_... */
	u8 length;
	u16 reserved;
} __packed;

/**
 * struct acpi_pptt_proc - a processor as described by PPTT
 */
struct acpi_pptt_proc {
	struct acpi_pptt_header hdr;
	u32 flags;
	u32 parent;
	u32 proc_id;
	u32 num_resources;
} __packed;

/* Cache flags for acpi_pptt_cache */
#define ACPI_PPTT_SIZE_VALID		BIT(0)
#define ACPI_PPTT_SETS_VALID		BIT(1)
#define ACPI_PPTT_ASSOC_VALID		BIT(2)
#define ACPI_PPTT_ALLOC_TYPE_VALID	BIT(3)
#define ACPI_PPTT_CACHE_TYPE_VALID	BIT(4)
#define ACPI_PPTT_WRITE_POLICY_VALID	BIT(5)
#define ACPI_PPTT_LINE_SIZE_VALID	BIT(6)

#define ACPI_PPTT_ALL_VALID		0x7f
#define ACPI_PPTT_ALL_BUT_WRITE_POL	0x5f

#define ACPI_PPTT_READ_ALLOC		BIT(0)
#define ACPI_PPTT_WRITE_ALLOC		BIT(1)
#define ACPI_PPTT_CACHE_TYPE_SHIFT	2
#define ACPI_PPTT_CACHE_TYPE_MASK	(3 << ACPI_PPTT_CACHE_TYPE_SHIFT)
#define ACPI_PPTT_CACHE_TYPE_DATA	0
#define ACPI_PPTT_CACHE_TYPE_INSTR	1
#define ACPI_PPTT_CACHE_TYPE_UNIFIED	2
#define ACPI_PPTT_CACHE_TYPE_DATA	0
#define ACPI_PPTT_WRITE_THROUGH		BIT(4)

/**
 * struct acpi_pptt_cache - a cache as described by PPTT
 */
struct acpi_pptt_cache {
	struct acpi_pptt_header hdr;
	u32 flags;
	u32 next_cache_level;
	u32 size;
	u32 sets;
	u8 assoc;
	u8 attributes;
	u16 line_size;
} __packed;

/** IORT - IO Remapping Table revision 6
 * Document number: ARM DEN 0049E.e, Sep 2022
 */
struct acpi_table_iort {
	struct acpi_table_header header;
	u32 node_count;
	u32 node_offset;
	u32 reserved;
} __packed;

/*
 * IORT subtables
 */
struct acpi_iort_node {
	u8 type;
	u16 length;
	u8 revision;
	u32 identifier;
	u32 mapping_count;
	u32 mapping_offset;
	char node_data[];
} __packed;

/* Values for subtable Type above */
enum acpi_iort_node_type {
	ACPI_IORT_NODE_ITS_GROUP = 0x00,
	ACPI_IORT_NODE_NAMED_COMPONENT = 0x01,
	ACPI_IORT_NODE_PCI_ROOT_COMPLEX = 0x02,
	ACPI_IORT_NODE_SMMU = 0x03,
	ACPI_IORT_NODE_SMMU_V3 = 0x04,
	ACPI_IORT_NODE_PMCG = 0x05,
	ACPI_IORT_NODE_RMR = 0x06,
};

/* ITS Group revision 1 */
struct acpi_iort_its_group {
	u32 its_count;
	u32 identifiers[];	/* GIC ITS identifier array */
} __packed;

/* PCI root complex node revision 2 */
struct acpi_iort_rc {
	u64 mem_access_properties;
	u32 ats_attributes;
	u32 pci_segment_number;
	u8 memory_address_size_limit;
	u8 reserved[3];
} __packed;

/* SMMUv3 revision 5 */
struct acpi_iort_smmu_v3 {
	u64 base_address;	/* SMMUv3 base address */
	u32 flags;
	u32 reserved;
	u64 vatos_address;
	u32 model;
	u32 event_gsiv;
	u32 pri_gsiv;
	u32 gerr_gsiv;
	u32 sync_gsiv;
	u32 pxm;
	u32 id_mapping_index;
} __packed;

/* Masks for Flags field above */
#define ACPI_IORT_SMMU_V3_COHACC_OVERRIDE   (1)
#define ACPI_IORT_SMMU_V3_HTTU_OVERRIDE     (3 << 1)
#define ACPI_IORT_SMMU_V3_PXM_VALID         (1 << 3)
#define ACPI_IORT_SMMU_V3_DEVICEID_VALID    (1 << 4)

struct acpi_iort_id_mapping {
	u32 input_base;		/* Lowest value in input range */
	u32 id_count;		/* Number of IDs */
	u32 output_base;	/* Lowest value in output range */
	u32 output_reference;	/* A reference to the output node */
	u32 flags;
} __packed;

/* Masks for Flags field above for IORT subtable */
#define ACPI_IORT_ID_SINGLE_MAPPING (1)

/* Named Component revision 4 */
struct acpi_iort_named_component {
	u32 node_flags;
	u64 memory_properties;	/* Memory access properties */
	u8 memory_address_limit;	/* Memory address size limit */
	char device_name[];	/* Path of namespace object */
} __packed;

/* Masks for Flags field above */
#define ACPI_IORT_NC_STALL_SUPPORTED    (1)
#define ACPI_IORT_NC_PASID_BITS         (31 << 1)

struct acpi_iort_root_complex {
	u64 memory_properties;	/* Memory access properties */
	u32 ats_attribute;
	u32 pci_segment_number;
	u8 memory_address_limit;/* Memory address size limit */
	u16 pasid_capabilities;	/* PASID Capabilities */
	u8 reserved;		/* Reserved, must be zero */
	u32 flags;		/* Flags */
} __packed;

/* Masks for ats_attribute field above */
#define ACPI_IORT_ATS_SUPPORTED         (1)		/* The root complex ATS support */
#define ACPI_IORT_PRI_SUPPORTED         (1 << 1)	/* The root complex PRI support */
#define ACPI_IORT_PASID_FWD_SUPPORTED   (1 << 2)	/* The root complex PASID forward support */

/* Masks for pasid_capabilities field above */
#define ACPI_IORT_PASID_MAX_WIDTH       (0x1F)	/* Bits 0-4 */

/* Tables defined/reserved by ACPI and generated by U-Boot */
enum acpi_tables {
	ACPITAB_BERT,
	ACPITAB_DBG2,
	ACPITAB_DMAR,
	ACPITAB_DSDT,
	ACPITAB_ECDT,
	ACPITAB_FACS,
	ACPITAB_FADT,
	ACPITAB_GTDT,
	ACPITAB_HEST,
	ACPITAB_HPET,
	ACPITAB_IVRS,
	ACPITAB_MADT,
	ACPITAB_MCFG,
	ACPITAB_NHLT,
	ACPITAB_PPTT,
	ACPITAB_RSDP,
	ACPITAB_RSDT,
	ACPITAB_SLIT,
	ACPITAB_SPCR,
	ACPITAB_SPMI,
	ACPITAB_SRAT,
	ACPITAB_SSDT,
	ACPITAB_TCPA,
	ACPITAB_TPM2,
	ACPITAB_VFCT,
	ACPITAB_XSDT,

	ACPITAB_COUNT,
};

/**
 * acpi_get_table_revision() - Get the revision number generated for a table
 *
 * This keeps the version-number information in one place
 *
 * @table: ACPI table to check
 * Return: version number that U-Boot generates
 */
int acpi_get_table_revision(enum acpi_tables table);

/**
 * acpi_create_dmar() - Create a DMA Remapping Reporting (DMAR) table
 *
 * @dmar: Place to put the table
 * @flags: DMAR flags to use
 * Return: 0 if OK, -ve on error
 */
int acpi_create_dmar(struct acpi_dmar *dmar, enum dmar_flags flags);

/**
 * acpi_create_mcfg_mmconfig() - Create a MCFG table entry
 *
 * @mmconfig: Place to put the table
 * @base: Base address of the ECAM space
 * @seg_nr: PCI segment number
 * @start: PCI bus start number
 * @end: PCI bus end number
 * Return: size of data written in bytes
 */
int acpi_create_mcfg_mmconfig(struct acpi_mcfg_mmconfig *mmconfig, u32 base,
			      u16 seg_nr, u8 start, u8 end);

/**
 * acpi_create_dbg2() - Create a DBG2 table
 *
 * This table describes how to access the debug UART
 *
 * @dbg2: Place to put information
 * @port_type: Serial port type (see ACPI_DBG2_...)
 * @port_subtype: Serial port sub-type (see ACPI_DBG2_...)
 * @address: ACPI address of port
 * @address_size: Size of address space
 * @device_path: Path of device (created using acpi_device_path())
 */
void acpi_create_dbg2(struct acpi_dbg2_header *dbg2,
		      int port_type, int port_subtype,
		      struct acpi_gen_regaddr *address, uint32_t address_size,
		      const char *device_path);

/**
 * acpi_align() - Align the ACPI output pointer to a 16-byte boundary
 *
 * @ctx: ACPI context
 */
void acpi_align(struct acpi_ctx *ctx);

/**
 * acpi_align64() - Align the ACPI output pointer to a 64-byte boundary
 *
 * @ctx: ACPI context
 */
void acpi_align64(struct acpi_ctx *ctx);

/**
 * acpi_inc() - Increment the ACPI output pointer by a bit
 *
 * The pointer is NOT aligned afterwards.
 *
 * @ctx: ACPI context
 * @amount: Amount to increment by
 */
void acpi_inc(struct acpi_ctx *ctx, uint amount);

/**
 * acpi_inc_align() - Increment the ACPI output pointer by a bit and align
 *
 * The pointer is aligned afterwards to a 16-byte boundary
 *
 * @ctx: ACPI context
 * @amount: Amount to increment by
 */
void acpi_inc_align(struct acpi_ctx *ctx, uint amount);

/**
 * acpi_add_table() - Add a new table to the RSDP and XSDT
 *
 * @ctx: ACPI context
 * @table: Table to add
 * Return: 0 if OK, -E2BIG if too many tables
 */
int acpi_add_table(struct acpi_ctx *ctx, void *table);

static inline int acpi_add_fadt(struct acpi_ctx *ctx, struct acpi_fadt *fadt)
{
	acpi_add_table(ctx, fadt);
	acpi_inc(ctx, sizeof(struct acpi_fadt));
	return 0;
}

/**
 * acpi_write_dbg2_pci_uart() - Write out a DBG2 table
 *
 * @ctx: Current ACPI context
 * @dev: Debug UART device to describe
 * @access_size: Access size for UART (e.g. ACPI_ACCESS_SIZE_DWORD_ACCESS)
 * Return: 0 if OK, -ve on error
 */
int acpi_write_dbg2_pci_uart(struct acpi_ctx *ctx, struct udevice *dev,
			     uint access_size);

/**
 * acpi_write_rsdp() - Write out an RSDP indicating where the ACPI tables are
 *
 * @rsdp: Address to write RSDP
 * @rsdt: Address of RSDT
 * @xsdt: Address of XSDT
 */
void acpi_write_rsdp(struct acpi_rsdp *rsdp, struct acpi_rsdt *rsdt,
		     struct acpi_xsdt *xsdt);

/**
 * acpi_fill_header() - Set up a table header
 *
 * @header: Pointer to header to set up
 * @signature: 4-character signature to use (e.g. "FACS")
 */
void acpi_fill_header(struct acpi_table_header *header, char *signature);

/**
 * acpi_fill_csrt() - Fill out the body of the CSRT
 *
 * This should write the contents of the Core System Resource Table (CSRT)
 * to the context. The header (struct acpi_table_header) has already been
 * written.
 *
 * @ctx: ACPI context to write to
 * @return 0 if OK, -ve on error
 */
int acpi_fill_csrt(struct acpi_ctx *ctx);

/**
 * acpi_fill_fadt() - Fill out the body of the FADT
 *
 * Must be implemented in SoC specific code or in mainboard code.
 *
 * @fadt: Pointer to FADT to update
 */
void acpi_fill_fadt(struct acpi_fadt *fadt);

/**
 * acpi_fill_iort() - Fill out the body of the IORT table
 *
 * Should be implemented in SoC specific code.
 *
 * @ctx: ACPI context to write to
 * @offset: Offset from the start of the IORT
 */
int acpi_fill_iort(struct acpi_ctx *ctx);

/**
 * acpi_iort_add_its_group() - Add ITS group node to IORT table
 *
 * Called by SoC specific code within acpi_fill_iort().
 *
 * @ctx: ACPI context to write to
 * @its_count: Elements in identifiers
 * @identifiers: The array of ITS identifiers. These IDs must match the value
 *               used in the Multiple APIC Description Table (MADT) GIC ITS
 *               structure for each relevant ITS unit.
 * @return Offset of table within parent
 */
int acpi_iort_add_its_group(struct acpi_ctx *ctx,
			    const u32 its_count,
			    const u32 *identifiers);

/**
 * acpi_iort_add_named_component() - Add named component to IORT table
 *
 * Called by SoC specific code within acpi_fill_iort().
 *
 * @ctx: ACPI context to write to
 * @node_flags: Node flags
 * @memory_properties: Memory properties
 * @memory_address_limit: Memory address limit
 * @device_name: ACPI device path
 * @return Offset of table within parent
 */
int acpi_iort_add_named_component(struct acpi_ctx *ctx,
				  const u32 node_flags,
				  const u64 memory_properties,
				  const u8 memory_address_limit,
				  const char *device_name);

/**
 * acpi_iort_add_rc() - Add PCI root complex node to IORT table
 *
 * Called by SoC specific code within acpi_fill_iort().
 *
 * @ctx: ACPI context to write to
 * @mem_access_properties: Memory access properties
 * @ats_attributes: Support for ATS and its ancillary feature
 * @pci_segment_number: The PCI segment number, as in MCFG
 * @memory_address_size_limit: The number of address bits, starting from LSB
 * @num_mappings: Number of elements in map
 * @map: ID mappings for this node
 * @return Offset of table within parent
 */
int acpi_iort_add_rc(struct acpi_ctx *ctx,
		     const u64 mem_access_properties,
		     const u32 ats_attributes,
		     const u32 pci_segment_number,
		     const u8 memory_address_size_limit,
		     const int num_mappings,
		     const struct acpi_iort_id_mapping *map);

/**
 * acpi_iort_add_smmu_v3() - Add PCI root complex node to IORT table
 *
 * Called by SoC specific code within acpi_fill_iort().
 *
 * @ctx: ACPI context to write to
 * @base_address: Base address of SMMU
 * @flags: SMMUv3 flags
 * @vatos_address: Optional, set to zero if not supported
 * @model: Model ID
 * @event_gsiv: GSIV of the Event interrupt if SPI based
 * @pri_gsiv: GSIV of the PRI interrupt if SPI based
 * @gerr_gsiv: GSIV of the GERR interrupt if GSIV based
 * @sync_gsiv: TGSIV of the Sync interrupt if GSIV based
 * @pxm: Proximity Domain
 * @id_mapping_index: If all the SMMU control interrupts are GSIV based,
 *                    this field is ignored. Index into the array of ID
 *                    mapping otherwise.
 * @num_mappings: Number of elements in map
 * @map: ID mappings for this node
 * @return Offset of table within parent
 */
int acpi_iort_add_smmu_v3(struct acpi_ctx *ctx,
			  const u64 base_address,
			  const u32 flags,
			  const u64 vatos_address,
			  const u32 model,
			  const u32 event_gsiv,
			  const u32 pri_gsiv,
			  const u32 gerr_gsiv,
			  const u32 sync_gsiv,
			  const u32 pxm,
			  const u32 id_mapping_index,
			  const int num_mappings,
			  const struct acpi_iort_id_mapping *map);

/**
 * acpi_fill_madt() - Fill out the body of the MADT
 *
 * Must be implemented in SoC specific code.
 *
 * @madt: The MADT to update
 * @ctx: ACPI context to write MADT sub-tables to
 * @return Pointer to the end of tables, where the next tables can be written
 */
void *acpi_fill_madt(struct acpi_madt *madt, struct acpi_ctx *ctx);

/**
 * acpi_write_park() - Installs the ACPI parking protocol.
 *
 * Sets up the ACPI parking protocol and installs the spinning code for
 * secondary CPUs.
 *
 * @madt: The MADT to update
 */
void acpi_write_park(struct acpi_madt *madt);

/**
 * acpi_get_rsdp_addr() - get ACPI RSDP table address
 *
 * This routine returns the ACPI RSDP table address in the system memory.
 *
 * @return:	ACPI RSDP table address
 */
ulong acpi_get_rsdp_addr(void);

/**
 * write_acpi_tables() - Write out the ACPI tables
 *
 * This writes all ACPI tables to the given address
 *
 * @start: Start address for the tables
 * @return address of end of tables, where the next tables can be written
 */
ulong write_acpi_tables(ulong start);

/**
 * acpi_find_table() - Look up an ACPI table
 *
 * @sig: Signature of table (4 characters, upper case)
 * Return: pointer to table header, or NULL if not found
 */
struct acpi_table_header *acpi_find_table(const char *sig);

/**
 * acpi_update_checksum() - update ACPI table checksum
 *
 * @header - header of an ACPI table
 */
void acpi_update_checksum(struct acpi_table_header *header);

#endif /* !__ACPI__*/

#include <asm/acpi_table.h>

#endif /* __ACPI_TABLE_H__ */
