/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	Intel
 */

#include <common.h>
#include <asm/arch/fsp/fsp_support.h>
#include <asm/post.h>

/**
 * Reads a 64-bit value from memory that may be unaligned.
 *
 * This function returns the 64-bit value pointed to by buf. The function
 * guarantees that the read operation does not produce an alignment fault.
 *
 * If the buf is NULL, then ASSERT().
 *
 * @buf: Pointer to a 64-bit value that may be unaligned.
 *
 * @return: The 64-bit value read from buf.
 */
static u64 read_unaligned64(const u64 *buf)
{
	ASSERT(buf != NULL);

	return *buf;
}

/**
 * Compares two GUIDs
 *
 * If the GUIDs are identical then TRUE is returned.
 * If there are any bit differences in the two GUIDs, then FALSE is returned.
 *
 * If guid1 is NULL, then ASSERT().
 * If guid2 is NULL, then ASSERT().
 *
 * @guid1:        A pointer to a 128 bit GUID.
 * @guid2:        A pointer to a 128 bit GUID.
 *
 * @retval TRUE:  guid1 and guid2 are identical.
 * @retval FALSE: guid1 and guid2 are not identical.
 */
static unsigned char compare_guid(const struct efi_guid_t *guid1,
				  const struct efi_guid_t *guid2)
{
	u64 guid1_low;
	u64 guid2_low;
	u64 guid1_high;
	u64 guid2_high;

	guid1_low  = read_unaligned64((const u64 *)guid1);
	guid2_low  = read_unaligned64((const u64 *)guid2);
	guid1_high = read_unaligned64((const u64 *)guid1 + 1);
	guid2_high = read_unaligned64((const u64 *)guid2 + 1);

	return (unsigned char)(guid1_low == guid2_low && guid1_high == guid2_high);
}

u32 __attribute__((optimize("O0"))) find_fsp_header(void)
{
	volatile register u8 *fsp asm("eax");

	/* Initalize the FSP base */
	fsp = (u8 *)CONFIG_FSP_LOCATION;

	/* Check the FV signature, _FVH */
	if (((struct fv_header_t *)fsp)->sign == 0x4856465F) {
		/* Go to the end of the FV header and align the address */
		fsp += ((struct fv_header_t *)fsp)->ext_hdr_off;
		fsp += ((struct fv_ext_header_t *)fsp)->ext_hdr_size;
		fsp  = (u8 *)(((u32)fsp + 7) & 0xFFFFFFF8);
	} else {
		fsp  = 0;
	}

	/* Check the FFS GUID */
	if (fsp &&
	    (((u32 *)&(((struct ffs_file_header_t *)fsp)->name))[0] == 0x912740BE) &&
	    (((u32 *)&(((struct ffs_file_header_t *)fsp)->name))[1] == 0x47342284) &&
	    (((u32 *)&(((struct ffs_file_header_t *)fsp)->name))[2] == 0xB08471B9) &&
	    (((u32 *)&(((struct ffs_file_header_t *)fsp)->name))[3] == 0x0C3F3527)) {
		/* Add the FFS header size to find the raw section header */
		fsp += sizeof(struct ffs_file_header_t);
	} else {
		fsp = 0;
	}

	if (fsp &&
	    ((struct raw_section_t *)fsp)->type == EFI_SECTION_RAW) {
		/* Add the raw section header size to find the FSP header */
		fsp += sizeof(struct raw_section_t);
	} else {
		fsp = 0;
	}

	return (u32)fsp;
}

void fsp_continue(struct shared_data_t *shared_data, u32 status, void *hob_list)
{
	u32 stack_len;
	u32 stack_base;
	u32 stack_top;

	post_code(POST_MRC);

	ASSERT(status == 0);

	/* Get the migrated stack in normal memory */
	stack_base = (u32)get_bootloader_tmp_mem(hob_list, &stack_len);
	ASSERT(stack_base != 0);
	stack_top  = stack_base + stack_len - sizeof(u32);

	/*
	 * Old stack base is stored at the very end of the stack top,
	 * use it to calculate the migrated shared data base
	 */
	shared_data = (struct shared_data_t *)(stack_base +
			((u32)shared_data - *(u32 *)stack_top));

	/* The boot loader main function entry */
	fsp_init_done(hob_list);
}

void fsp_init(u32 stack_top, u32 boot_mode, void *nvs_buf)
{
	struct shared_data_t shared_data;
	fsp_init_f init;
	struct fsp_init_params_t params;
	struct fspinit_rtbuf_t rt_buf;
	struct vpd_region_t *fsp_vpd;
	struct fsp_header_t *fsp_hdr;
	struct fsp_init_params_t *params_ptr;
	struct upd_region_t *fsp_upd;

	fsp_hdr = (struct fsp_header_t *)find_fsp_header();
	if (fsp_hdr == NULL) {
		/* No valid FSP info header was found */
		ASSERT(FALSE);
	}

	fsp_upd = (struct upd_region_t *)&shared_data.fsp_upd;
	memset((void *)&rt_buf, 0, sizeof(struct fspinit_rtbuf_t));

	/* Reserve a gap in stack top */
	rt_buf.common.stack_top = (u32 *)stack_top - 32;
	rt_buf.common.boot_mode = boot_mode;
	rt_buf.common.upd_data = (struct upd_region_t *)fsp_upd;

	/* Get VPD region start */
	fsp_vpd = (struct vpd_region_t *)(fsp_hdr->img_base +
			fsp_hdr->cfg_region_off);

	/* Verifify the VPD data region is valid */
	ASSERT((fsp_vpd->img_rev == VPD_IMAGE_REV) &&
	       (fsp_vpd->sign == VPD_IMAGE_ID));

	/* Copy default data from Flash */
	memcpy(fsp_upd, (void *)(fsp_hdr->img_base + fsp_vpd->upd_offset),
	       sizeof(struct upd_region_t));

	/* Verifify the UPD data region is valid */
	ASSERT(fsp_upd->terminator == 0x55AA);

	/* Override any UPD setting if required */
	update_fsp_upd(fsp_upd);

	memset((void *)&params, 0, sizeof(struct fsp_init_params_t));
	params.nvs_buf = nvs_buf;
	params.rt_buf = (struct fspinit_rtbuf_t *)&rt_buf;
	params.continuation = (fsp_continuation_f)asm_continuation;

	init = (fsp_init_f)(fsp_hdr->img_base + fsp_hdr->fsp_init);
	params_ptr = &params;

	shared_data.fsp_hdr = fsp_hdr;
	shared_data.stack_top = (u32 *)stack_top;

	post_code(POST_PRE_MRC);

	/*
	 * Use ASM code to ensure the register value in EAX & ECX
	 * will be passed into BlContinuationFunc
	 */
	asm volatile (
		"pushl	%0;"
		"call	*%%eax;"
		".global asm_continuation;"
		"asm_continuation:;"
		"movl	%%ebx, %%eax;"		/* shared_data */
		"movl	4(%%esp), %%edx;"	/* status */
		"movl	8(%%esp), %%ecx;"	/* hob_list */
		"jmp	fsp_continue;"
		: : "m"(params_ptr), "a"(init), "b"(&shared_data)
	);

	/*
	 * Should never get here.
	 * Control will continue from romstage_main_continue_asm.
	 * This line below is to prevent the compiler from optimizing
	 * structure intialization.
	 */
	init(&params);

	/*
	 * Should never return.
	 * Control will continue from ContinuationFunc
	 */
	ASSERT(FALSE);
}

u32 fsp_notify(struct fsp_header_t *fsp_hdr, u32 phase)
{
	fsp_notify_f notify;
	struct fsp_notify_params_t params;
	struct fsp_notify_params_t *params_ptr;
	u32 status;

	if (!fsp_hdr)
		fsp_hdr = (struct fsp_header_t *)find_fsp_header();

	if (fsp_hdr == NULL) {
		/* No valid FSP info header */
		ASSERT(FALSE);
	}

	notify = (fsp_notify_f)(fsp_hdr->img_base + fsp_hdr->fsp_notify);
	params.phase = phase;
	params_ptr = &params;

	/*
	 * Use ASM code to ensure correct parameter is on the stack for
	 * FspNotify as U-Boot is using different ABI from FSP
	 */
	asm volatile (
		"pushl	%1;"		/* push notify phase */
		"call	*%%eax;"	/* call FspNotify */
		"addl	$4, %%esp;"	/* clean up the stack */
		: "=a"(status) : "m"(params_ptr), "a"(notify), "m"(*params_ptr)
	);

	return status;
}

u32 get_usable_lowmem_top(const void *hob_list)
{
	union hob_pointers_t hob;
	phys_addr_t phys_start;
	u32 top;

	/* Get the HOB list for processing */
	hob.raw = (void *)hob_list;

	/* * Collect memory ranges */
	top = 0x100000;
	while (!END_OF_HOB(hob)) {
		if (hob.hdr->type == HOB_TYPE_RES_DESC) {
			if (hob.res_desc->type == RES_SYS_MEM) {
				phys_start = hob.res_desc->phys_start;
				/* Need memory above 1MB to be collected here */
				if (phys_start >= 0x100000 &&
				    phys_start < (phys_addr_t)0x100000000)
					top += (u32)(hob.res_desc->len);
			}
		}
		hob.raw = GET_NEXT_HOB(hob);
	}

	return top;
}

u64 get_usable_highmem_top(const void *hob_list)
{
	union hob_pointers_t hob;
	phys_addr_t phys_start;
	u64 top;

	/* Get the HOB list for processing */
	hob.raw = (void *)hob_list;

	/* Collect memory ranges */
	top = 0x100000000;
	while (!END_OF_HOB(hob)) {
		if (hob.hdr->type == HOB_TYPE_RES_DESC) {
			if (hob.res_desc->type == RES_SYS_MEM) {
				phys_start = hob.res_desc->phys_start;
				/* Need memory above 1MB to be collected here */
				if (phys_start >= (phys_addr_t)0x100000000)
					top += (u32)(hob.res_desc->len);
			}
		}
		hob.raw = GET_NEXT_HOB(hob);
	}

	return top;
}

u64 get_fsp_reserved_mem_from_guid(const void *hob_list, u64 *len,
				   struct efi_guid_t *guid)
{
	union hob_pointers_t hob;

	/* Get the HOB list for processing */
	hob.raw = (void *)hob_list;

	/* Collect memory ranges */
	while (!END_OF_HOB(hob)) {
		if (hob.hdr->type == HOB_TYPE_RES_DESC) {
			if (hob.res_desc->type == RES_MEM_RESERVED) {
				if (compare_guid(&hob.res_desc->owner, guid)) {
					if (len)
						*len = (u32)(hob.res_desc->len);

					return (u64)(hob.res_desc->phys_start);
				}
			}
		}
		hob.raw = GET_NEXT_HOB(hob);
	}

	return 0;
}

u32 get_fsp_reserved_mem(const void *hob_list, u32 *len)
{
	const struct efi_guid_t guid = FSP_HOB_RESOURCE_OWNER_FSP_GUID;
	u64 length;
	u32 base;

	base = (u32)get_fsp_reserved_mem_from_guid(hob_list,
			&length, (struct efi_guid_t *)&guid);
	if ((len != 0) && (base != 0))
		*len = (u32)length;

	return base;
}

u32 get_tseg_reserved_mem(const void *hob_list, u32 *len)
{
	const struct efi_guid_t guid = FSP_HOB_RESOURCE_OWNER_TSEG_GUID;
	u64 length;
	u32 base;

	base = (u32)get_fsp_reserved_mem_from_guid(hob_list,
			&length, (struct efi_guid_t *)&guid);
	if ((len != 0) && (base != 0))
		*len = (u32)length;

	return base;
}

void *get_next_hob(u16 type, const void *hob_list)
{
	union hob_pointers_t hob;

	ASSERT(hob_list != NULL);

	hob.raw = (u8 *)hob_list;

	/* Parse the HOB list until end of list or matching type is found */
	while (!END_OF_HOB(hob)) {
		if (hob.hdr->type == type)
			return hob.raw;

		hob.raw = GET_NEXT_HOB(hob);
	}

	return NULL;
}

void *get_next_guid_hob(const struct efi_guid_t *guid, const void *hob_list)
{
	union hob_pointers_t hob;

	hob.raw = (u8 *)hob_list;
	while ((hob.raw = get_next_hob(HOB_TYPE_GUID_EXT,
			hob.raw)) != NULL) {
		if (compare_guid(guid, &hob.guid->name))
			break;
		hob.raw = GET_NEXT_HOB(hob);
	}

	return hob.raw;
}

void *get_guid_hob_data(const void *hob_list, u32 *len, struct efi_guid_t *guid)
{
	u8 *guid_hob;

	guid_hob = get_next_guid_hob(guid, hob_list);
	if (guid_hob == NULL) {
		return NULL;
	} else {
		if (len)
			*len = GET_GUID_HOB_DATA_SIZE(guid_hob);

		return GET_GUID_HOB_DATA(guid_hob);
	}
}

void *get_fsp_nvs_data(const void *hob_list, u32 *len)
{
	const struct efi_guid_t guid = FSP_NON_VOLATILE_STORAGE_HOB_GUID;

	return get_guid_hob_data(hob_list, len, (struct efi_guid_t *)&guid);
}

void *get_bootloader_tmp_mem(const void *hob_list, u32 *len)
{
	const struct efi_guid_t guid = FSP_BOOTLOADER_TEMP_MEM_HOB_GUID;

	return get_guid_hob_data(hob_list, len, (struct efi_guid_t *)&guid);
}
