/*
 * Extensible Firmware Interface
 * Based on 'Extensible Firmware Interface Specification' version 0.9,
 * April 30, 1999
 *
 * Copyright (C) 1999 VA Linux Systems
 * Copyright (C) 1999 Walt Drummond <drummond@valinux.com>
 * Copyright (C) 1999, 2002-2003 Hewlett-Packard Co.
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 *	Stephane Eranian <eranian@hpl.hp.com>
 *
 * From include/linux/efi.h in kernel 4.1 with some additions/subtractions
 */

#ifndef _EFI_H
#define _EFI_H

#include <linux/linkage.h>
#include <linux/string.h>
#include <linux/types.h>

#ifdef CONFIG_EFI_STUB_64BIT
/* EFI uses the Microsoft ABI which is not the default for GCC */
#define EFIAPI __attribute__((ms_abi))
#else
#define EFIAPI asmlinkage
#endif

struct efi_device_path;

#define EFI_BITS_PER_LONG	BITS_PER_LONG

/*
 * With 64-bit EFI stub, EFI_BITS_PER_LONG has to be 64. EFI_STUB is set
 * in lib/efi/Makefile, when building the stub.
 */
#if defined(CONFIG_EFI_STUB_64BIT) && defined(EFI_STUB)
#undef EFI_BITS_PER_LONG
#define EFI_BITS_PER_LONG	64
#endif

#define EFI_SUCCESS		0
#define EFI_LOAD_ERROR		(1 | (1UL << (EFI_BITS_PER_LONG - 1)))
#define EFI_INVALID_PARAMETER	(2 | (1UL << (EFI_BITS_PER_LONG - 1)))
#define EFI_UNSUPPORTED		(3 | (1UL << (EFI_BITS_PER_LONG - 1)))
#define EFI_BAD_BUFFER_SIZE	(4 | (1UL << (EFI_BITS_PER_LONG - 1)))
#define EFI_BUFFER_TOO_SMALL	(5 | (1UL << (EFI_BITS_PER_LONG - 1)))
#define EFI_NOT_READY		(6 | (1UL << (EFI_BITS_PER_LONG - 1)))
#define EFI_DEVICE_ERROR	(7 | (1UL << (EFI_BITS_PER_LONG - 1)))
#define EFI_WRITE_PROTECTED	(8 | (1UL << (EFI_BITS_PER_LONG - 1)))
#define EFI_OUT_OF_RESOURCES	(9 | (1UL << (EFI_BITS_PER_LONG - 1)))
#define EFI_NOT_FOUND		(14 | (1UL << (EFI_BITS_PER_LONG - 1)))
#define EFI_ACCESS_DENIED	(15 | (1UL << (EFI_BITS_PER_LONG - 1)))
#define EFI_SECURITY_VIOLATION	(26 | (1UL << (EFI_BITS_PER_LONG - 1)))

typedef unsigned long efi_status_t;
typedef u64 efi_physical_addr_t;
typedef u64 efi_virtual_addr_t;
typedef void *efi_handle_t;

#define EFI_GUID(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7) \
	((efi_guid_t) \
	{{ (a) & 0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, \
		((a) >> 24) & 0xff, \
		(b) & 0xff, ((b) >> 8) & 0xff, \
		(c) & 0xff, ((c) >> 8) & 0xff, \
		(d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) } })

/* Generic EFI table header */
struct efi_table_hdr {
	u64 signature;
	u32 revision;
	u32 headersize;
	u32 crc32;
	u32 reserved;
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

	EFI_MAX_MEMORY_TYPE,
	EFI_TABLE_END,	/* For efi_build_mem_table() */
};

/* Attribute values */
enum {
	EFI_MEMORY_UC_SHIFT	= 0,	/* uncached */
	EFI_MEMORY_WC_SHIFT	= 1,	/* write-coalescing */
	EFI_MEMORY_WT_SHIFT	= 2,	/* write-through */
	EFI_MEMORY_WB_SHIFT	= 3,	/* write-back */
	EFI_MEMORY_UCE_SHIFT	= 4,	/* uncached, exported */
	EFI_MEMORY_WP_SHIFT	= 12,	/* write-protect */
	EFI_MEMORY_RP_SHIFT	= 13,	/* read-protect */
	EFI_MEMORY_XP_SHIFT	= 14,	/* execute-protect */
	EFI_MEMORY_RUNTIME_SHIFT = 63,	/* range requires runtime mapping */

	EFI_MEMORY_RUNTIME = 1ULL << EFI_MEMORY_RUNTIME_SHIFT,
	EFI_MEM_DESC_VERSION	= 1,
};

#define EFI_PAGE_SHIFT		12
#define EFI_PAGE_SIZE		(1UL << EFI_PAGE_SHIFT)
#define EFI_PAGE_MASK		(EFI_PAGE_SIZE - 1)

struct efi_mem_desc {
	u32 type;
	u32 reserved;
	efi_physical_addr_t physical_start;
	efi_virtual_addr_t virtual_start;
	u64 num_pages;
	u64 attribute;
};

#define EFI_MEMORY_DESCRIPTOR_VERSION 1

/* Allocation types for calls to boottime->allocate_pages*/
#define EFI_ALLOCATE_ANY_PAGES		0
#define EFI_ALLOCATE_MAX_ADDRESS	1
#define EFI_ALLOCATE_ADDRESS		2
#define EFI_MAX_ALLOCATE_TYPE		3

/* Types and defines for Time Services */
#define EFI_TIME_ADJUST_DAYLIGHT 0x1
#define EFI_TIME_IN_DAYLIGHT     0x2
#define EFI_UNSPECIFIED_TIMEZONE 0x07ff

struct efi_time {
	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
	u8 second;
	u8 pad1;
	u32 nanosecond;
	s16 timezone;
	u8 daylight;
	u8 pad2;
};

struct efi_time_cap {
	u32 resolution;
	u32 accuracy;
	u8 sets_to_zero;
};

enum efi_locate_search_type {
	all_handles,
	by_register_notify,
	by_protocol
};

struct efi_open_protocol_info_entry {
	efi_handle_t agent_handle;
	efi_handle_t controller_handle;
	u32 attributes;
	u32 open_count;
};

enum efi_entry_t {
	EFIET_END,	/* Signals this is the last (empty) entry */
	EFIET_MEMORY_MAP,

	/* Number of entries */
	EFIET_MEMORY_COUNT,
};

#define EFI_TABLE_VERSION	1

/**
 * struct efi_info_hdr - Header for the EFI info table
 *
 * @version:	EFI_TABLE_VERSION
 * @hdr_size:	Size of this struct in bytes
 * @total_size:	Total size of this header plus following data
 * @spare:	Spare space for expansion
 */
struct efi_info_hdr {
	u32 version;
	u32 hdr_size;
	u32 total_size;
	u32 spare[5];
};

/**
 * struct efi_entry_hdr - Header for a table entry
 *
 * @type:	enum eft_entry_t
 * @size	size of entry bytes excluding header and padding
 * @addr:	address of this entry (0 if it follows the header )
 * @link:	size of entry including header and padding
 * @spare1:	Spare space for expansion
 * @spare2:	Spare space for expansion
 */
struct efi_entry_hdr {
	u32 type;
	u32 size;
	u64 addr;
	u32 link;
	u32 spare1;
	u64 spare2;
};

/**
 * struct efi_entry_memmap - a memory map table passed to U-Boot
 *
 * @version:	EFI's memory map table version
 * @desc_size:	EFI's size of each memory descriptor
 * @spare:	Spare space for expansion
 * @desc:	An array of descriptors, each @desc_size bytes apart
 */
struct efi_entry_memmap {
	u32 version;
	u32 desc_size;
	u64 spare;
	struct efi_mem_desc desc[];
};

static inline struct efi_mem_desc *efi_get_next_mem_desc(
		struct efi_entry_memmap *map, struct efi_mem_desc *desc)
{
	return (struct efi_mem_desc *)((ulong)desc + map->desc_size);
}

struct efi_priv {
	efi_handle_t parent_image;
	struct efi_device_path *device_path;
	struct efi_system_table *sys_table;
	struct efi_boot_services *boot;
	struct efi_runtime_services *run;
	bool use_pool_for_malloc;
	unsigned long ram_base;
	unsigned int image_data_type;
	struct efi_info_hdr *info;
	unsigned int info_size;
	void *next_hdr;
};

/* Base address of the EFI image */
extern char image_base[];

/* Start and end of U-Boot image (for payload) */
extern char _binary_u_boot_bin_start[], _binary_u_boot_bin_end[];

/**
 * efi_get_sys_table() - Get access to the main EFI system table
 *
 * @return pointer to EFI system table
 */

struct efi_system_table *efi_get_sys_table(void);

/**
 * efi_get_ram_base() - Find the base of RAM
 *
 * This is used when U-Boot is built as an EFI application.
 *
 * @return the base of RAM as known to U-Boot
 */
unsigned long efi_get_ram_base(void);

/**
 * efi_init() - Set up ready for use of EFI boot services
 *
 * @priv:	Pointer to our private EFI structure to fill in
 * @banner:	Banner to display when starting
 * @image:	The image handle passed to efi_main()
 * @sys_table:	The EFI system table pointer passed to efi_main()
 */
int efi_init(struct efi_priv *priv, const char *banner, efi_handle_t image,
	     struct efi_system_table *sys_table);

/**
 * efi_malloc() - Allocate some memory from EFI
 *
 * @priv:	Pointer to private EFI structure
 * @size:	Number of bytes to allocate
 * @retp:	Return EFI status result
 * @return pointer to memory allocated, or NULL on error
 */
void *efi_malloc(struct efi_priv *priv, int size, efi_status_t *retp);

/**
 * efi_free() - Free memory allocated from EFI
 *
 * @priv:	Pointer to private EFI structure
 * @ptr:	Pointer to memory to free
 */
void efi_free(struct efi_priv *priv, void *ptr);

/**
 * efi_puts() - Write out a string to the EFI console
 *
 * @priv:	Pointer to private EFI structure
 * @str:	String to write (note this is a ASCII, not unicode)
 */
void efi_puts(struct efi_priv *priv, const char *str);

/**
 * efi_putc() - Write out a character to the EFI console
 *
 * @priv:	Pointer to private EFI structure
 * @ch:		Character to write (note this is not unicode)
 */
void efi_putc(struct efi_priv *priv, const char ch);

/**
 * efi_info_get() - get an entry from an EFI table
 *
 * @type:	Entry type to search for
 * @datap:	Returns pointer to entry data
 * @sizep:	Returns pointer to entry size
 * @return 0 if OK, -ENODATA if there is no table, -ENOENT if there is no entry
 * of the requested type, -EPROTONOSUPPORT if the table has the wrong version
 */
int efi_info_get(enum efi_entry_t type, void **datap, int *sizep);

/**
 * efi_build_mem_table() - make a sorted copy of the memory table
 *
 * @map:	Pointer to EFI memory map table
 * @size:	Size of table in bytes
 * @skip_bs:	True to skip boot-time memory and merge it with conventional
 *		memory. This will significantly reduce the number of table
 *		entries.
 * @return pointer to the new table. It should be freed with free() by the
 *	   caller
 */
void *efi_build_mem_table(struct efi_entry_memmap *map, int size, bool skip_bs);

#endif /* _LINUX_EFI_H */
