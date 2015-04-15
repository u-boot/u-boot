/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	Intel
 */

#ifndef __FSP_HOB_H__
#define __FSP_HOB_H__

/* Type of HOB Header */
#define HOB_TYPE_MEM_ALLOC	0x0002
#define HOB_TYPE_RES_DESC	0x0003
#define HOB_TYPE_GUID_EXT	0x0004
#define HOB_TYPE_UNUSED		0xFFFE
#define HOB_TYPE_EOH		0xFFFF

/*
 * Describes the format and size of the data inside the HOB.
 * All HOBs must contain this generic HOB header.
 */
struct hob_header {
	u16	type;		/* HOB type */
	u16	len;		/* HOB length */
	u32	reserved;	/* always zero */
};

/* Enumeration of memory types introduced in UEFI */
enum efi_mem_type {
	EFI_RESERVED_MEMORY_TYPE,
	/*
	 * The code portions of a loaded application.
	 * (Note that UEFI OS loaders are UEFI applications.)
	 */
	EFI_LOADER_CODE,
	/*
	 * The data portions of a loaded application and
	 * the default data allocation type used by an application
	 * to allocate pool memory.
	 */
	EFI_LOADER_DATA,
	/* The code portions of a loaded Boot Services Driver */
	EFI_BOOT_SERVICES_CODE,
	/*
	 * The data portions of a loaded Boot Serves Driver and
	 * the default data allocation type used by a Boot Services
	 * Driver to allocate pool memory.
	 */
	EFI_BOOT_SERVICES_DATA,
	/* The code portions of a loaded Runtime Services Driver */
	EFI_RUNTIME_SERVICES_CODE,
	/*
	 * The data portions of a loaded Runtime Services Driver and
	 * the default data allocation type used by a Runtime Services
	 * Driver to allocate pool memory.
	 */
	EFI_RUNTIME_SERVICES_DATA,
	/* Free (unallocated) memory */
	EFI_CONVENTIONAL_MEMORY,
	/* Memory in which errors have been detected */
	EFI_UNUSABLE_MEMORY,
	/* Memory that holds the ACPI tables */
	EFI_ACPI_RECLAIM_MEMORY,
	/* Address space reserved for use by the firmware */
	EFI_ACPI_MEMORY_NVS,
	/*
	 * Used by system firmware to request that a memory-mapped IO region
	 * be mapped by the OS to a virtual address so it can be accessed by
	 * EFI runtime services.
	 */
	EFI_MMAP_IO,
	/*
	 * System memory-mapped IO region that is used to translate
	 * memory cycles to IO cycles by the processor.
	 */
	EFI_MMAP_IO_PORT,
	/*
	 * Address space reserved by the firmware for code that is
	 * part of the processor.
	 */
	EFI_PAL_CODE,
	EFI_MAX_MEMORY_TYPE
};

/*
 * Describes all memory ranges used during the HOB producer phase that
 * exist outside the HOB list. This HOB type describes how memory is used,
 * not the physical attributes of memory.
 */
struct hob_mem_alloc {
	struct hob_header	hdr;
	/*
	 * A GUID that defines the memory allocation region's type and purpose,
	 * as well as other fields within the memory allocation HOB. This GUID
	 * is used to define the additional data within the HOB that may be
	 * present for the memory allocation HOB. Type efi_guid is defined in
	 * InstallProtocolInterface() in the UEFI 2.0 specification.
	 */
	struct efi_guid		name;
	/*
	 * The base address of memory allocated by this HOB.
	 * Type phys_addr_t is defined in AllocatePages() in the UEFI 2.0
	 * specification.
	 */
	phys_addr_t		mem_base;
	/* The length in bytes of memory allocated by this HOB */
	phys_size_t		mem_len;
	/*
	 * Defines the type of memory allocated by this HOB.
	 * The memory type definition follows the EFI_MEMORY_TYPE definition.
	 * Type EFI_MEMORY_TYPE is defined in AllocatePages() in the UEFI 2.0
	 * specification.
	 */
	enum efi_mem_type	mem_type;
	/* padding */
	u8			reserved[4];
};

/* Value of ResourceType in HOB_RES_DESC */
#define RES_SYS_MEM		0x00000000
#define RES_MMAP_IO		0x00000001
#define RES_IO			0x00000002
#define RES_FW_DEVICE		0x00000003
#define RES_MMAP_IO_PORT	0x00000004
#define RES_MEM_RESERVED	0x00000005
#define RES_IO_RESERVED		0x00000006
#define RES_MAX_MEM_TYPE	0x00000007

/*
 * These types can be ORed together as needed.
 *
 * The first three enumerations describe settings
 * The rest of the settings describe capabilities
 */
#define RES_ATTR_PRESENT			0x00000001
#define RES_ATTR_INITIALIZED			0x00000002
#define RES_ATTR_TESTED				0x00000004
#define RES_ATTR_SINGLE_BIT_ECC			0x00000008
#define RES_ATTR_MULTIPLE_BIT_ECC		0x00000010
#define RES_ATTR_ECC_RESERVED_1			0x00000020
#define RES_ATTR_ECC_RESERVED_2			0x00000040
#define RES_ATTR_READ_PROTECTED			0x00000080
#define RES_ATTR_WRITE_PROTECTED		0x00000100
#define RES_ATTR_EXECUTION_PROTECTED		0x00000200
#define RES_ATTR_UNCACHEABLE			0x00000400
#define RES_ATTR_WRITE_COMBINEABLE		0x00000800
#define RES_ATTR_WRITE_THROUGH_CACHEABLE	0x00001000
#define RES_ATTR_WRITE_BACK_CACHEABLE		0x00002000
#define RES_ATTR_16_BIT_IO			0x00004000
#define RES_ATTR_32_BIT_IO			0x00008000
#define RES_ATTR_64_BIT_IO			0x00010000
#define RES_ATTR_UNCACHED_EXPORTED		0x00020000

/*
 * Describes the resource properties of all fixed, nonrelocatable resource
 * ranges found on the processor host bus during the HOB producer phase.
 */
struct hob_res_desc {
	struct hob_header	hdr;
	/*
	 * A GUID representing the owner of the resource. This GUID is
	 * used by HOB consumer phase components to correlate device
	 * ownership of a resource.
	 */
	struct efi_guid		owner;
	u32			type;
	u32			attr;
	/* The physical start address of the resource region */
	phys_addr_t		phys_start;
	/* The number of bytes of the resource region */
	phys_size_t		len;
};

/*
 * Allows writers of executable content in the HOB producer phase to
 * maintain and manage HOBs with specific GUID.
 */
struct hob_guid {
	struct hob_header	hdr;
	/* A GUID that defines the contents of this HOB */
	struct efi_guid		name;
	/* GUID specific data goes here */
};

/**
 * get_next_hob() - return a pointer to the next HOB in the HOB list
 *
 * This macro returns a pointer to HOB that follows the HOB specified by hob
 * in the HOB List.
 *
 * @hdr:    A pointer to a HOB.
 *
 * @return: A pointer to the next HOB in the HOB list.
 */
static inline const struct hob_header *get_next_hob(const struct hob_header *hdr)
{
	return (const struct hob_header *)((u32)hdr + hdr->len);
}

/**
 * end_of_hob() - determine if a HOB is the last HOB in the HOB list
 *
 * This macro determine if the HOB specified by hob is the last HOB in the
 * HOB list.  If hob is last HOB in the HOB list, then true is returned.
 * Otherwise, false is returned.
 *
 * @hdr:          A pointer to a HOB.
 *
 * @retval true:  The HOB specified by hdr is the last HOB in the HOB list.
 * @retval false: The HOB specified by hdr is not the last HOB in the HOB list.
 */
static inline bool end_of_hob(const struct hob_header *hdr)
{
	return hdr->type == HOB_TYPE_EOH;
}

/**
 * get_guid_hob_data() - return a pointer to data buffer from a HOB of
 *                       type HOB_TYPE_GUID_EXT
 *
 * This macro returns a pointer to the data buffer in a HOB specified by hob.
 * hob is assumed to be a HOB of type HOB_TYPE_GUID_EXT.
 *
 * @hdr:    A pointer to a HOB.
 *
 * @return: A pointer to the data buffer in a HOB.
 */
static inline void *get_guid_hob_data(const struct hob_header *hdr)
{
	return (void *)((u32)hdr + sizeof(struct hob_guid));
}

/**
 * get_guid_hob_data_size() - return the size of the data buffer from a HOB
 *                            of type HOB_TYPE_GUID_EXT
 *
 * This macro returns the size, in bytes, of the data buffer in a HOB
 * specified by hob. hob is assumed to be a HOB of type HOB_TYPE_GUID_EXT.
 *
 * @hdr:    A pointer to a HOB.
 *
 * @return: The size of the data buffer.
 */
static inline u16 get_guid_hob_data_size(const struct hob_header *hdr)
{
	return hdr->len - sizeof(struct hob_guid);
}

/* FSP specific GUID HOB definitions */
#define FSP_GUID_DATA1		0x912740be
#define FSP_GUID_DATA2		0x2284
#define FSP_GUID_DATA3		0x4734
#define FSP_GUID_DATA4_0	0xb9
#define FSP_GUID_DATA4_1	0x71
#define FSP_GUID_DATA4_2	0x84
#define FSP_GUID_DATA4_3	0xb0
#define FSP_GUID_DATA4_4	0x27
#define FSP_GUID_DATA4_5	0x35
#define FSP_GUID_DATA4_6	0x3f
#define FSP_GUID_DATA4_7	0x0c

#define FSP_HEADER_GUID \
	{ \
	FSP_GUID_DATA1, FSP_GUID_DATA2, FSP_GUID_DATA3, \
	{ FSP_GUID_DATA4_0, FSP_GUID_DATA4_1, FSP_GUID_DATA4_2, \
	  FSP_GUID_DATA4_3, FSP_GUID_DATA4_4, FSP_GUID_DATA4_5, \
	  FSP_GUID_DATA4_6, FSP_GUID_DATA4_7 } \
	}

#define FSP_NON_VOLATILE_STORAGE_HOB_GUID \
	{ \
	0x721acf02, 0x4d77, 0x4c2a, \
	{ 0xb3, 0xdc, 0x27, 0xb, 0x7b, 0xa9, 0xe4, 0xb0 } \
	}

#define FSP_BOOTLOADER_TEMP_MEM_HOB_GUID \
	{ \
	0xbbcff46c, 0xc8d3, 0x4113, \
	{ 0x89, 0x85, 0xb9, 0xd4, 0xf3, 0xb3, 0xf6, 0x4e } \
	}

#define FSP_HOB_RESOURCE_OWNER_FSP_GUID \
	{ \
	0x69a79759, 0x1373, 0x4367, \
	{ 0xa6, 0xc4, 0xc7, 0xf5, 0x9e, 0xfd, 0x98, 0x6e } \
	}

#define FSP_HOB_RESOURCE_OWNER_TSEG_GUID \
	{ \
	0xd038747c, 0xd00c, 0x4980, \
	{ 0xb3, 0x19, 0x49, 0x01, 0x99, 0xa4, 0x7d, 0x55 } \
	}

#define FSP_HOB_RESOURCE_OWNER_GRAPHICS_GUID \
	{ \
	0x9c7c3aa7, 0x5332, 0x4917, \
	{ 0x82, 0xb9, 0x56, 0xa5, 0xf3, 0xe6, 0x2a, 0x07 } \
	}

#endif
