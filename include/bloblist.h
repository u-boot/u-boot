/* SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause */
/*
 * This provides a standard way of passing information between boot phases
 * (TPL -> SPL -> U-Boot proper.)
 *
 * A list of blobs of data, tagged with their owner. The list resides in memory
 * and can be updated by SPL, U-Boot, etc.
 *
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __BLOBLIST_H
#define __BLOBLIST_H

#include <mapmem.h>

enum {
	BLOBLIST_VERSION	= 0,
	BLOBLIST_MAGIC		= 0xb00757a3,
	BLOBLIST_ALIGN		= 16,
};

/* Supported tags - add new ones to tag_name in bloblist.c */
enum bloblist_tag_t {
	BLOBLISTT_NONE = 0,

	/*
	 * Standard area to allocate blobs used across firmware components, for
	 * things that are very commonly used, particularly in multiple
	 * projects.
	 */
	BLOBLISTT_AREA_FIRMWARE_TOP = 0x1,

	/* Standard area to allocate blobs used across firmware components */
	BLOBLISTT_AREA_FIRMWARE = 0x100,
	/*
	 * Advanced Configuration and Power Interface Global Non-Volatile
	 * Sleeping table. This forms part of the ACPI tables passed to Linux.
	 */
	BLOBLISTT_ACPI_GNVS = 0x100,
	BLOBLISTT_INTEL_VBT = 0x101,	/* Intel Video-BIOS table */
	BLOBLISTT_TPM2_TCG_LOG = 0x102,	/* TPM v2 log space */
	BLOBLISTT_TCPA_LOG = 0x103,	/* TPM log space */
	BLOBLISTT_ACPI_TABLES = 0x104,	/* ACPI tables for x86 */
	BLOBLISTT_SMBIOS_TABLES = 0x105, /* SMBIOS tables for x86 */
	BLOBLISTT_VBOOT_CTX = 0x106,	/* Chromium OS verified boot context */

	/*
	 * Project-specific tags are permitted here. Projects can be open source
	 * or not, but the format of the data must be fuily documented in an
	 * open source project, including all fields, bits, etc. Naming should
	 * be: BLOBLISTT_<project>_<purpose_here>
	 */
	BLOBLISTT_PROJECT_AREA = 0x8000,
	BLOBLISTT_U_BOOT_SPL_HANDOFF = 0x8000, /* Hand-off info from SPL */

	/*
	 * Vendor-specific tags are permitted here. Projects can be open source
	 * or not, but the format of the data must be fuily documented in an
	 * open source project, including all fields, bits, etc. Naming should
	 * be BLOBLISTT_<vendor>_<purpose_here>
	 */
	BLOBLISTT_VENDOR_AREA = 0xc000,

	/* Tags after this are not allocated for now */
	BLOBLISTT_EXPANSION = 0x10000,

	/*
	 * Tags from here are on reserved for private use within a single
	 * firmware binary (i.e. a single executable or phase of a project).
	 * These tags can be passed between binaries within a local
	 * implementation, but cannot be used in upstream code. Allocate a
	 * tag in one of the areas above if you want that.
	 *
	 * This area may move in future.
	 */
	BLOBLISTT_PRIVATE_AREA = 0xffff0000,
};

/**
 * struct bloblist_hdr - header for the bloblist
 *
 * This is stored at the start of the bloblist which is always on a 16-byte
 * boundary. Records follow this header. The bloblist normally stays in the
 * same place in memory as SPL and U-Boot execute, but it can be safely moved
 * around.
 *
 * None of the bloblist headers themselves contain pointers but it is possible
 * to put pointers inside a bloblist record if desired. This is not encouraged,
 * since it can make part of the bloblist inaccessible if the pointer is
 * no-longer valid. It is better to just store all the data inside a bloblist
 * record.
 *
 * Each bloblist record is aligned to a 16-byte boundary and follows immediately
 * from the last.
 *
 * @magic: BLOBLIST_MAGIC
 * @version: BLOBLIST_VERSION
 * @hdr_size: Size of this header, normally sizeof(struct bloblist_hdr). The
 *	first bloblist_rec starts at this offset from the start of the header
 * @flags: Space for BLOBLISTF... flags (none yet)
 * @size: Total size of the bloblist (non-zero if valid) including this header.
 *	The bloblist extends for this many bytes from the start of this header.
 *	When adding new records, the bloblist can grow up to this size.
 * @alloced: Total size allocated so far for this bloblist. This starts out as
 *	sizeof(bloblist_hdr) since we need at least that much space to store a
 *	valid bloblist
 * @spare: Spare space (for future use)
 * @chksum: CRC32 for the entire bloblist allocated area. Since any of the
 *	blobs can be altered after being created, this checksum is only valid
 *	when the bloblist is finalised before jumping to the next stage of boot.
 *	Note that chksum is last to make it easier to exclude it from the
 *	checksum calculation.
 */
struct bloblist_hdr {
	u32 magic;
	u32 version;
	u32 hdr_size;
	u32 flags;

	u32 size;
	u32 alloced;
	u32 spare;
	u32 chksum;
};

/**
 * struct bloblist_rec - record for the bloblist
 *
 * The bloblist contains a number of records each consisting of this record
 * structure followed by the data contained. Each records is 16-byte aligned.
 *
 * NOTE: Only exported for testing purposes. Do not use this struct.
 *
 * @tag: Tag indicating what the record contains
 * @hdr_size: Size of this header, normally sizeof(struct bloblist_rec). The
 *	record's data starts at this offset from the start of the record
 * @size: Size of record in bytes, excluding the header size. This does not
 *	need to be aligned (e.g. 3 is OK).
 * @spare: Spare space for other things
 */
struct bloblist_rec {
	u32 tag;
	u32 hdr_size;
	u32 size;
	u32 spare;
};

/**
 * bloblist_check_magic() - return a bloblist if the magic matches
 *
 * @addr: Address to check
 * Return: pointer to bloblist, if the magic matches, else NULL
 */
static inline void *bloblist_check_magic(ulong addr)
{
	u32 *ptr;

	if (!addr)
		return NULL;
	ptr = map_sysmem(addr, 0);
	if (*ptr != BLOBLIST_MAGIC)
		return NULL;

	return ptr;
}

/**
 * bloblist_find() - Find a blob
 *
 * Searches the bloblist and returns the blob with the matching tag
 *
 * @tag:	Tag to search for (enum bloblist_tag_t)
 * @size:	Expected size of the blob, or 0 for any size
 * Return: pointer to blob if found, or NULL if not found, or a blob was found
 * but it is the wrong size
 */
void *bloblist_find(uint tag, int size);

/**
 * bloblist_add() - Add a new blob
 *
 * Add a new blob to the bloblist
 *
 * This should only be called if you konw there is no existing blob for a
 * particular tag. It is typically safe to call in the first phase of U-Boot
 * (e.g. TPL or SPL). After that, bloblist_ensure() should be used instead.
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @size:	Size of the blob
 * @align:	Alignment of the blob (in bytes), 0 for default
 * Return: pointer to the newly added block, or NULL if there is not enough
 * space for the blob
 */
void *bloblist_add(uint tag, int size, int align);

/**
 * bloblist_ensure_size() - Find or add a blob
 *
 * Find an existing blob, or add a new one if not found
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @size:	Size of the blob
 * @blobp:	Returns a pointer to blob on success
 * @align:	Alignment of the blob (in bytes), 0 for default
 * Return: 0 if OK, -ENOSPC if it is missing and could not be added due to lack
 * of space, or -ESPIPE it exists but has the wrong size
 */
int bloblist_ensure_size(uint tag, int size, int align, void **blobp);

/**
 * bloblist_ensure() - Find or add a blob
 *
 * Find an existing blob, or add a new one if not found
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @size:	Size of the blob
 * Return: pointer to blob, or NULL if it is missing and could not be added due
 * to lack of space, or it exists but has the wrong size
 */
void *bloblist_ensure(uint tag, int size);

/**
 * bloblist_ensure_size_ret() - Find or add a blob
 *
 * Find an existing blob, or add a new one if not found
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @sizep:	Size of the blob to create; returns size of actual blob
 * @blobp:	Returns a pointer to blob on success
 * Return: 0 if OK, -ENOSPC if it is missing and could not be added due to lack
 * of space
 */
int bloblist_ensure_size_ret(uint tag, int *sizep, void **blobp);

/**
 * bloblist_resize() - resize a blob
 *
 * Any blobs above this one are relocated up or down. The resized blob remains
 * in the same place.
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @new_size:	New size of the blob (>0 to expand, <0 to contract)
 * Return: 0 if OK, -ENOSPC if the bloblist does not have enough space, -ENOENT
 * if the tag is not found
 */
int bloblist_resize(uint tag, int new_size);

/**
 * bloblist_new() - Create a new, empty bloblist of a given size
 *
 * @addr: Address of bloblist
 * @size: Initial size for bloblist
 * @flags: Flags to use for bloblist
 * Return: 0 if OK, -EFAULT if addr is not aligned correctly, -ENOSPC is the
 * area is not large enough
 */
int bloblist_new(ulong addr, uint size, uint flags);

/**
 * bloblist_check() - Check if a bloblist exists
 *
 * @addr: Address of bloblist
 * @size: Expected size of blobsize, or 0 to detect the size
 * Return: 0 if OK, -ENOENT if the magic number doesn't match (indicating that
 * there problem is no bloblist at the given address), -EPROTONOSUPPORT
 * if the version does not match, -EIO if the checksum does not match,
 * -EFBIG if the expected size does not match the detected size, -ENOSPC
 * if the size is not large enough to hold the headers
 */
int bloblist_check(ulong addr, uint size);

/**
 * bloblist_finish() - Set up the bloblist for the next U-Boot part
 *
 * This sets the correct checksum for the bloblist. This ensures that the
 * bloblist will be detected correctly by the next phase of U-Boot.
 *
 * Return: 0
 */
int bloblist_finish(void);

/**
 * bloblist_get_stats() - Get information about the bloblist
 *
 * This returns useful information about the bloblist
 *
 * @basep: Returns base address of bloblist
 * @sizep: Returns the number of bytes used in the bloblist
 * @allocedp: Returns the total space allocated to the bloblist
 */
void bloblist_get_stats(ulong *basep, ulong *sizep, ulong *allocedp);

/**
 * bloblist_get_base() - Get the base address of the bloblist
 *
 * Return: base address of bloblist
 */
ulong bloblist_get_base(void);

/**
 * bloblist_get_size() - Get the size of the bloblist
 *
 * Return: the size in bytes
 */
ulong bloblist_get_size(void);

/**
 * bloblist_show_stats() - Show information about the bloblist
 *
 * This shows useful information about the bloblist on the console
 */
void bloblist_show_stats(void);

/**
 * bloblist_show_list() - Show a list of blobs in the bloblist
 *
 * This shows a list of blobs, showing their address, size and tag.
 */
void bloblist_show_list(void);

/**
 * bloblist_tag_name() - Get the name for a tag
 *
 * @tag: Tag to check
 * Return: name of tag, or "invalid" if an invalid tag is provided
 */
const char *bloblist_tag_name(enum bloblist_tag_t tag);

/**
 * bloblist_reloc() - Relocate the bloblist and optionally resize it
 *
 * @to: Pointer to new bloblist location (must not overlap old location)
 * @to_size: New size for bloblist (must be larger than from_size)
 * @from: Pointer to bloblist to relocate
 * @from_size: Size of bloblist to relocate
 */
void bloblist_reloc(void *to, uint to_size, void *from, uint from_size);

/**
 * bloblist_init() - Init the bloblist system with a single bloblist
 *
 * This locates and sets up the blocklist for use.
 *
 * If CONFIG_BLOBLIST_FIXED is selected, it uses CONFIG_BLOBLIST_ADDR and
 * CONFIG_BLOBLIST_SIZE to set up a bloblist for use by U-Boot.
 *
 * If CONFIG_BLOBLIST_ALLOC is selected, it allocates memory for a bloblist of
 * size CONFIG_BLOBLIST_SIZE.
 *
 * If CONFIG_BLOBLIST_PASSAGE is selected, it uses the bloblist in the incoming
 * standard passage. The size is detected automatically so CONFIG_BLOBLIST_SIZE
 * can be 0.
 *
 * Return: 0 if OK, -ve on error
 */
int bloblist_init(void);

#endif /* __BLOBLIST_H */
