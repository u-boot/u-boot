/*
 *  EFI application runtime services
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <efi_loader.h>
#include <rtc.h>
#include <asm/global_data.h>

/* For manual relocation support */
DECLARE_GLOBAL_DATA_PTR;

struct efi_runtime_mmio_list {
	struct list_head link;
	void **ptr;
	u64 paddr;
	u64 len;
};

/* This list contains all runtime available mmio regions */
LIST_HEAD(efi_runtime_mmio);

static efi_status_t __efi_runtime EFIAPI efi_unimplemented(void);
static efi_status_t __efi_runtime EFIAPI efi_device_error(void);
static efi_status_t __efi_runtime EFIAPI efi_invalid_parameter(void);

#ifdef CONFIG_SYS_CACHELINE_SIZE
#define EFI_CACHELINE_SIZE CONFIG_SYS_CACHELINE_SIZE
#else
/* Just use the greatest cache flush alignment requirement I'm aware of */
#define EFI_CACHELINE_SIZE 128
#endif

#if defined(CONFIG_ARM64)
#define R_RELATIVE	1027
#define R_MASK		0xffffffffULL
#define IS_RELA		1
#elif defined(CONFIG_ARM)
#define R_RELATIVE	23
#define R_MASK		0xffULL
#elif defined(CONFIG_X86)
#include <asm/elf.h>
#define R_RELATIVE	R_386_RELATIVE
#define R_MASK		0xffULL
#else
#error Need to add relocation awareness
#endif

struct elf_rel {
	ulong *offset;
	ulong info;
};

struct elf_rela {
	ulong *offset;
	ulong info;
	long addend;
};

/*
 * EFI Runtime code lives in 2 stages. In the first stage, U-Boot and an EFI
 * payload are running concurrently at the same time. In this mode, we can
 * handle a good number of runtime callbacks
 */

static void EFIAPI efi_reset_system_boottime(
			enum efi_reset_type reset_type,
			efi_status_t reset_status,
			unsigned long data_size, void *reset_data)
{
	EFI_ENTRY("%d %lx %lx %p", reset_type, reset_status, data_size,
		  reset_data);

	switch (reset_type) {
	case EFI_RESET_COLD:
	case EFI_RESET_WARM:
		do_reset(NULL, 0, 0, NULL);
		break;
	case EFI_RESET_SHUTDOWN:
		/* We don't have anything to map this to */
		break;
	}

	while (1) { }
}

static efi_status_t EFIAPI efi_get_time_boottime(
			struct efi_time *time,
			struct efi_time_cap *capabilities)
{
#if defined(CONFIG_CMD_DATE) && defined(CONFIG_DM_RTC)
	struct rtc_time tm;
	int r;
	struct udevice *dev;

	EFI_ENTRY("%p %p", time, capabilities);

	r = uclass_get_device(UCLASS_RTC, 0, &dev);
	if (r)
		return EFI_EXIT(EFI_DEVICE_ERROR);

	r = dm_rtc_get(dev, &tm);
	if (r)
		return EFI_EXIT(EFI_DEVICE_ERROR);

	memset(time, 0, sizeof(*time));
	time->year = tm.tm_year;
	time->month = tm.tm_mon;
	time->day = tm.tm_mday;
	time->hour = tm.tm_hour;
	time->minute = tm.tm_min;
	time->daylight = tm.tm_isdst;

	return EFI_EXIT(EFI_SUCCESS);
#else
	return EFI_DEVICE_ERROR;
#endif
}

/* Boards may override the helpers below to implement RTS functionality */

void __weak __efi_runtime EFIAPI efi_reset_system(
			enum efi_reset_type reset_type,
			efi_status_t reset_status,
			unsigned long data_size, void *reset_data)
{
	/* Nothing we can do */
	while (1) { }
}

void __weak efi_reset_system_init(void)
{
}

efi_status_t __weak __efi_runtime EFIAPI efi_get_time(
			struct efi_time *time,
			struct efi_time_cap *capabilities)
{
	/* Nothing we can do */
	return EFI_DEVICE_ERROR;
}

void __weak efi_get_time_init(void)
{
}

struct efi_runtime_detach_list_struct {
	void *ptr;
	void *patchto;
};

static const struct efi_runtime_detach_list_struct efi_runtime_detach_list[] = {
	{
		/* do_reset is gone */
		.ptr = &efi_runtime_services.reset_system,
		.patchto = efi_reset_system,
	}, {
		/* invalidate_*cache_all are gone */
		.ptr = &efi_runtime_services.set_virtual_address_map,
		.patchto = &efi_invalid_parameter,
	}, {
		/* RTC accessors are gone */
		.ptr = &efi_runtime_services.get_time,
		.patchto = &efi_get_time,
	}, {
		/* Clean up system table */
		.ptr = &systab.con_in,
		.patchto = NULL,
	}, {
		/* Clean up system table */
		.ptr = &systab.con_out,
		.patchto = NULL,
	}, {
		/* Clean up system table */
		.ptr = &systab.std_err,
		.patchto = NULL,
	}, {
		/* Clean up system table */
		.ptr = &systab.boottime,
		.patchto = NULL,
	}, {
		.ptr = &efi_runtime_services.get_variable,
		.patchto = &efi_device_error,
	}, {
		.ptr = &efi_runtime_services.get_next_variable,
		.patchto = &efi_device_error,
	}, {
		.ptr = &efi_runtime_services.set_variable,
		.patchto = &efi_device_error,
	}
};

static bool efi_runtime_tobedetached(void *p)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(efi_runtime_detach_list); i++)
		if (efi_runtime_detach_list[i].ptr == p)
			return true;

	return false;
}

static void efi_runtime_detach(ulong offset)
{
	int i;
	ulong patchoff = offset - (ulong)gd->relocaddr;

	for (i = 0; i < ARRAY_SIZE(efi_runtime_detach_list); i++) {
		ulong patchto = (ulong)efi_runtime_detach_list[i].patchto;
		ulong *p = efi_runtime_detach_list[i].ptr;
		ulong newaddr = patchto ? (patchto + patchoff) : 0;

		debug("%s: Setting %p to %lx\n", __func__, p, newaddr);
		*p = newaddr;
	}
}

/* Relocate EFI runtime to uboot_reloc_base = offset */
void efi_runtime_relocate(ulong offset, struct efi_mem_desc *map)
{
#ifdef IS_RELA
	struct elf_rela *rel = (void*)&__efi_runtime_rel_start;
#else
	struct elf_rel *rel = (void*)&__efi_runtime_rel_start;
	static ulong lastoff = CONFIG_SYS_TEXT_BASE;
#endif

	debug("%s: Relocating to offset=%lx\n", __func__, offset);
	for (; (ulong)rel < (ulong)&__efi_runtime_rel_stop; rel++) {
		ulong base = CONFIG_SYS_TEXT_BASE;
		ulong *p;
		ulong newaddr;

		p = (void*)((ulong)rel->offset - base) + gd->relocaddr;

		if ((rel->info & R_MASK) != R_RELATIVE) {
			continue;
		}

#ifdef IS_RELA
		newaddr = rel->addend + offset - CONFIG_SYS_TEXT_BASE;
#else
		newaddr = *p - lastoff + offset;
#endif

		/* Check if the relocation is inside bounds */
		if (map && ((newaddr < map->virtual_start) ||
		    newaddr > (map->virtual_start +
			      (map->num_pages << EFI_PAGE_SHIFT)))) {
			if (!efi_runtime_tobedetached(p))
				printf("U-Boot EFI: Relocation at %p is out of "
				       "range (%lx)\n", p, newaddr);
			continue;
		}

		debug("%s: Setting %p to %lx\n", __func__, p, newaddr);
		*p = newaddr;
		flush_dcache_range((ulong)p & ~(EFI_CACHELINE_SIZE - 1),
			ALIGN((ulong)&p[1], EFI_CACHELINE_SIZE));
	}

#ifndef IS_RELA
	lastoff = offset;
#endif

        invalidate_icache_all();
}

static efi_status_t EFIAPI efi_set_virtual_address_map(
			unsigned long memory_map_size,
			unsigned long descriptor_size,
			uint32_t descriptor_version,
			struct efi_mem_desc *virtmap)
{
	ulong runtime_start = (ulong)&__efi_runtime_start &
			      ~(ulong)EFI_PAGE_MASK;
	int n = memory_map_size / descriptor_size;
	int i;

	EFI_ENTRY("%lx %lx %x %p", memory_map_size, descriptor_size,
		  descriptor_version, virtmap);

	/* Rebind mmio pointers */
	for (i = 0; i < n; i++) {
		struct efi_mem_desc *map = (void*)virtmap +
					   (descriptor_size * i);
		struct list_head *lhandle;
		efi_physical_addr_t map_start = map->physical_start;
		efi_physical_addr_t map_len = map->num_pages << EFI_PAGE_SHIFT;
		efi_physical_addr_t map_end = map_start + map_len;

		/* Adjust all mmio pointers in this region */
		list_for_each(lhandle, &efi_runtime_mmio) {
			struct efi_runtime_mmio_list *lmmio;

			lmmio = list_entry(lhandle,
					   struct efi_runtime_mmio_list,
					   link);
			if ((map_start <= lmmio->paddr) &&
			    (map_end >= lmmio->paddr)) {
				u64 off = map->virtual_start - map_start;
				uintptr_t new_addr = lmmio->paddr + off;
				*lmmio->ptr = (void *)new_addr;
			}
		}
	}

	/* Move the actual runtime code over */
	for (i = 0; i < n; i++) {
		struct efi_mem_desc *map;

		map = (void*)virtmap + (descriptor_size * i);
		if (map->type == EFI_RUNTIME_SERVICES_CODE) {
			ulong new_offset = map->virtual_start -
					   (runtime_start - gd->relocaddr);

			efi_runtime_relocate(new_offset, map);
			/* Once we're virtual, we can no longer handle
			   complex callbacks */
			efi_runtime_detach(new_offset);
			return EFI_EXIT(EFI_SUCCESS);
		}
	}

	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

void efi_add_runtime_mmio(void *mmio_ptr, u64 len)
{
	struct efi_runtime_mmio_list *newmmio;

	u64 pages = (len + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT;
	efi_add_memory_map(*(uintptr_t *)mmio_ptr, pages, EFI_MMAP_IO, false);

	newmmio = calloc(1, sizeof(*newmmio));
	newmmio->ptr = mmio_ptr;
	newmmio->paddr = *(uintptr_t *)mmio_ptr;
	newmmio->len = len;
	list_add_tail(&newmmio->link, &efi_runtime_mmio);
}

/*
 * In the second stage, U-Boot has disappeared. To isolate our runtime code
 * that at this point still exists from the rest, we put it into a special
 * section.
 *
 *        !!WARNING!!
 *
 * This means that we can not rely on any code outside of this file in any
 * function or variable below this line.
 *
 * Please keep everything fully self-contained and annotated with
 * __efi_runtime and __efi_runtime_data markers.
 */

/*
 * Relocate the EFI runtime stub to a different place. We need to call this
 * the first time we expose the runtime interface to a user and on set virtual
 * address map calls.
 */

static efi_status_t __efi_runtime EFIAPI efi_unimplemented(void)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t __efi_runtime EFIAPI efi_device_error(void)
{
	return EFI_DEVICE_ERROR;
}

static efi_status_t __efi_runtime EFIAPI efi_invalid_parameter(void)
{
	return EFI_INVALID_PARAMETER;
}

efi_status_t __efi_runtime EFIAPI efi_update_capsule(
			struct efi_capsule_header **capsule_header_array,
			efi_uintn_t capsule_count,
			u64 scatter_gather_list)
{
	return EFI_UNSUPPORTED;
}

efi_status_t __efi_runtime EFIAPI efi_query_capsule_caps(
			struct efi_capsule_header **capsule_header_array,
			efi_uintn_t capsule_count,
			u64 maximum_capsule_size,
			u32 reset_type)
{
	return EFI_UNSUPPORTED;
}

efi_status_t __efi_runtime EFIAPI efi_query_variable_info(
			u32 attributes,
			u64 maximum_variable_storage_size,
			u64 remaining_variable_storage_size,
			u64 maximum_variable_size)
{
	return EFI_UNSUPPORTED;
}

struct efi_runtime_services __efi_runtime_data efi_runtime_services = {
	.hdr = {
		.signature = EFI_RUNTIME_SERVICES_SIGNATURE,
		.revision = EFI_RUNTIME_SERVICES_REVISION,
		.headersize = sizeof(struct efi_table_hdr),
	},
	.get_time = &efi_get_time_boottime,
	.set_time = (void *)&efi_device_error,
	.get_wakeup_time = (void *)&efi_unimplemented,
	.set_wakeup_time = (void *)&efi_unimplemented,
	.set_virtual_address_map = &efi_set_virtual_address_map,
	.convert_pointer = (void *)&efi_invalid_parameter,
	.get_variable = efi_get_variable,
	.get_next_variable = efi_get_next_variable,
	.set_variable = efi_set_variable,
	.get_next_high_mono_count = (void *)&efi_device_error,
	.reset_system = &efi_reset_system_boottime,
	.update_capsule = efi_update_capsule,
	.query_capsule_caps = efi_query_capsule_caps,
	.query_variable_info = efi_query_variable_info,
};
