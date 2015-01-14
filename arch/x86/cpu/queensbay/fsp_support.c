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
 * Compares two GUIDs
 *
 * If the GUIDs are identical then true is returned.
 * If there are any bit differences in the two GUIDs, then false is returned.
 *
 * @guid1:        A pointer to a 128 bit GUID.
 * @guid2:        A pointer to a 128 bit GUID.
 *
 * @retval true:  guid1 and guid2 are identical.
 * @retval false: guid1 and guid2 are not identical.
 */
static bool compare_guid(const struct efi_guid *guid1,
			 const struct efi_guid *guid2)
{
	if (memcmp(guid1, guid2, sizeof(struct efi_guid)) == 0)
		return true;
	else
		return false;
}

u32 __attribute__((optimize("O0"))) find_fsp_header(void)
{
	/*
	 * This function may be called before the a stack is established,
	 * so special care must be taken. First, it cannot declare any local
	 * variable using stack. Only register variable can be used here.
	 * Secondly, some compiler version will add prolog or epilog code
	 * for the C function. If so the function call may not work before
	 * stack is ready.
	 *
	 * GCC 4.8.1 has been verified to be working for the following codes.
	 */
	volatile register u8 *fsp asm("eax");

	/* Initalize the FSP base */
	fsp = (u8 *)CONFIG_FSP_ADDR;

	/* Check the FV signature, _FVH */
	if (((struct fv_header *)fsp)->sign == EFI_FVH_SIGNATURE) {
		/* Go to the end of the FV header and align the address */
		fsp += ((struct fv_header *)fsp)->ext_hdr_off;
		fsp += ((struct fv_ext_header *)fsp)->ext_hdr_size;
		fsp  = (u8 *)(((u32)fsp + 7) & 0xFFFFFFF8);
	} else {
		fsp  = 0;
	}

	/* Check the FFS GUID */
	if (fsp &&
	    ((struct ffs_file_header *)fsp)->name.data1 == FSP_GUID_DATA1 &&
	    ((struct ffs_file_header *)fsp)->name.data2 == FSP_GUID_DATA2 &&
	    ((struct ffs_file_header *)fsp)->name.data3 == FSP_GUID_DATA3 &&
	    ((struct ffs_file_header *)fsp)->name.data4[0] == FSP_GUID_DATA4_0 &&
	    ((struct ffs_file_header *)fsp)->name.data4[1] == FSP_GUID_DATA4_1 &&
	    ((struct ffs_file_header *)fsp)->name.data4[2] == FSP_GUID_DATA4_2 &&
	    ((struct ffs_file_header *)fsp)->name.data4[3] == FSP_GUID_DATA4_3 &&
	    ((struct ffs_file_header *)fsp)->name.data4[4] == FSP_GUID_DATA4_4 &&
	    ((struct ffs_file_header *)fsp)->name.data4[5] == FSP_GUID_DATA4_5 &&
	    ((struct ffs_file_header *)fsp)->name.data4[6] == FSP_GUID_DATA4_6 &&
	    ((struct ffs_file_header *)fsp)->name.data4[7] == FSP_GUID_DATA4_7) {
		/* Add the FFS header size to find the raw section header */
		fsp += sizeof(struct ffs_file_header);
	} else {
		fsp = 0;
	}

	if (fsp &&
	    ((struct raw_section *)fsp)->type == EFI_SECTION_RAW) {
		/* Add the raw section header size to find the FSP header */
		fsp += sizeof(struct raw_section);
	} else {
		fsp = 0;
	}

	return (u32)fsp;
}

void fsp_continue(struct shared_data *shared_data, u32 status, void *hob_list)
{
	u32 stack_len;
	u32 stack_base;
	u32 stack_top;

	post_code(POST_MRC);

	assert(status == 0);

	/* Get the migrated stack in normal memory */
	stack_base = (u32)fsp_get_bootloader_tmp_mem(hob_list, &stack_len);
	assert(stack_base != 0);
	stack_top  = stack_base + stack_len - sizeof(u32);

	/*
	 * Old stack base is stored at the very end of the stack top,
	 * use it to calculate the migrated shared data base
	 */
	shared_data = (struct shared_data *)(stack_base +
			((u32)shared_data - *(u32 *)stack_top));

	/* The boot loader main function entry */
	fsp_init_done(hob_list);
}

void fsp_init(u32 stack_top, u32 boot_mode, void *nvs_buf)
{
	struct shared_data shared_data;
	fsp_init_f init;
	struct fsp_init_params params;
	struct fspinit_rtbuf rt_buf;
	struct vpd_region *fsp_vpd;
	struct fsp_header *fsp_hdr;
	struct fsp_init_params *params_ptr;
	struct upd_region *fsp_upd;

	fsp_hdr = (struct fsp_header *)find_fsp_header();
	if (fsp_hdr == NULL) {
		/* No valid FSP info header was found */
		panic("Invalid FSP header");
	}

	fsp_upd = (struct upd_region *)&shared_data.fsp_upd;
	memset(&rt_buf, 0, sizeof(struct fspinit_rtbuf));

	/* Reserve a gap in stack top */
	rt_buf.common.stack_top = (u32 *)stack_top - 32;
	rt_buf.common.boot_mode = boot_mode;
	rt_buf.common.upd_data = (struct upd_region *)fsp_upd;

	/* Get VPD region start */
	fsp_vpd = (struct vpd_region *)(fsp_hdr->img_base +
			fsp_hdr->cfg_region_off);

	/* Verifify the VPD data region is valid */
	assert((fsp_vpd->img_rev == VPD_IMAGE_REV) &&
	       (fsp_vpd->sign == VPD_IMAGE_ID));

	/* Copy default data from Flash */
	memcpy(fsp_upd, (void *)(fsp_hdr->img_base + fsp_vpd->upd_offset),
	       sizeof(struct upd_region));

	/* Verifify the UPD data region is valid */
	assert(fsp_upd->terminator == UPD_TERMINATOR);

	/* Override any UPD setting if required */
	update_fsp_upd(fsp_upd);

	memset(&params, 0, sizeof(struct fsp_init_params));
	params.nvs_buf = nvs_buf;
	params.rt_buf = (struct fspinit_rtbuf *)&rt_buf;
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
	 * Control will continue from fsp_continue.
	 * This line below is to prevent the compiler from optimizing
	 * structure intialization.
	 *
	 * DO NOT REMOVE!
	 */
	init(&params);
}

u32 fsp_notify(struct fsp_header *fsp_hdr, u32 phase)
{
	fsp_notify_f notify;
	struct fsp_notify_params params;
	struct fsp_notify_params *params_ptr;
	u32 status;

	if (!fsp_hdr)
		fsp_hdr = (struct fsp_header *)find_fsp_header();

	if (fsp_hdr == NULL) {
		/* No valid FSP info header */
		panic("Invalid FSP header");
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

u32 fsp_get_usable_lowmem_top(const void *hob_list)
{
	union hob_pointers hob;
	phys_addr_t phys_start;
	u32 top;

	/* Get the HOB list for processing */
	hob.raw = (void *)hob_list;

	/* * Collect memory ranges */
	top = FSP_LOWMEM_BASE;
	while (!end_of_hob(hob)) {
		if (get_hob_type(hob) == HOB_TYPE_RES_DESC) {
			if (hob.res_desc->type == RES_SYS_MEM) {
				phys_start = hob.res_desc->phys_start;
				/* Need memory above 1MB to be collected here */
				if (phys_start >= FSP_LOWMEM_BASE &&
				    phys_start < (phys_addr_t)FSP_HIGHMEM_BASE)
					top += (u32)(hob.res_desc->len);
			}
		}
		hob.raw = get_next_hob(hob);
	}

	return top;
}

u64 fsp_get_usable_highmem_top(const void *hob_list)
{
	union hob_pointers hob;
	phys_addr_t phys_start;
	u64 top;

	/* Get the HOB list for processing */
	hob.raw = (void *)hob_list;

	/* Collect memory ranges */
	top = FSP_HIGHMEM_BASE;
	while (!end_of_hob(hob)) {
		if (get_hob_type(hob) == HOB_TYPE_RES_DESC) {
			if (hob.res_desc->type == RES_SYS_MEM) {
				phys_start = hob.res_desc->phys_start;
				/* Need memory above 1MB to be collected here */
				if (phys_start >= (phys_addr_t)FSP_HIGHMEM_BASE)
					top += (u32)(hob.res_desc->len);
			}
		}
		hob.raw = get_next_hob(hob);
	}

	return top;
}

u64 fsp_get_reserved_mem_from_guid(const void *hob_list, u64 *len,
				   struct efi_guid *guid)
{
	union hob_pointers hob;

	/* Get the HOB list for processing */
	hob.raw = (void *)hob_list;

	/* Collect memory ranges */
	while (!end_of_hob(hob)) {
		if (get_hob_type(hob) == HOB_TYPE_RES_DESC) {
			if (hob.res_desc->type == RES_MEM_RESERVED) {
				if (compare_guid(&hob.res_desc->owner, guid)) {
					if (len)
						*len = (u32)(hob.res_desc->len);

					return (u64)(hob.res_desc->phys_start);
				}
			}
		}
		hob.raw = get_next_hob(hob);
	}

	return 0;
}

u32 fsp_get_fsp_reserved_mem(const void *hob_list, u32 *len)
{
	const struct efi_guid guid = FSP_HOB_RESOURCE_OWNER_FSP_GUID;
	u64 length;
	u32 base;

	base = (u32)fsp_get_reserved_mem_from_guid(hob_list,
			&length, (struct efi_guid *)&guid);
	if ((len != 0) && (base != 0))
		*len = (u32)length;

	return base;
}

u32 fsp_get_tseg_reserved_mem(const void *hob_list, u32 *len)
{
	const struct efi_guid guid = FSP_HOB_RESOURCE_OWNER_TSEG_GUID;
	u64 length;
	u32 base;

	base = (u32)fsp_get_reserved_mem_from_guid(hob_list,
			&length, (struct efi_guid *)&guid);
	if ((len != 0) && (base != 0))
		*len = (u32)length;

	return base;
}

void *fsp_get_next_hob(u16 type, const void *hob_list)
{
	union hob_pointers hob;

	assert(hob_list != NULL);

	hob.raw = (u8 *)hob_list;

	/* Parse the HOB list until end of list or matching type is found */
	while (!end_of_hob(hob)) {
		if (get_hob_type(hob) == type)
			return hob.raw;

		hob.raw = get_next_hob(hob);
	}

	return NULL;
}

void *fsp_get_next_guid_hob(const struct efi_guid *guid, const void *hob_list)
{
	union hob_pointers hob;

	hob.raw = (u8 *)hob_list;
	while ((hob.raw = fsp_get_next_hob(HOB_TYPE_GUID_EXT,
			hob.raw)) != NULL) {
		if (compare_guid(guid, &hob.guid->name))
			break;
		hob.raw = get_next_hob(hob);
	}

	return hob.raw;
}

void *fsp_get_guid_hob_data(const void *hob_list, u32 *len,
			    struct efi_guid *guid)
{
	u8 *guid_hob;

	guid_hob = fsp_get_next_guid_hob(guid, hob_list);
	if (guid_hob == NULL) {
		return NULL;
	} else {
		if (len)
			*len = get_guid_hob_data_size(guid_hob);

		return get_guid_hob_data(guid_hob);
	}
}

void *fsp_get_nvs_data(const void *hob_list, u32 *len)
{
	const struct efi_guid guid = FSP_NON_VOLATILE_STORAGE_HOB_GUID;

	return fsp_get_guid_hob_data(hob_list, len, (struct efi_guid *)&guid);
}

void *fsp_get_bootloader_tmp_mem(const void *hob_list, u32 *len)
{
	const struct efi_guid guid = FSP_BOOTLOADER_TEMP_MEM_HOB_GUID;

	return fsp_get_guid_hob_data(hob_list, len, (struct efi_guid *)&guid);
}
