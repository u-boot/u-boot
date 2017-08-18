/*
 *  EFI application boot time services
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <libfdt_env.h>
#include <u-boot/crc.h>
#include <bootm.h>
#include <inttypes.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

/* This list contains all the EFI objects our payload has access to */
LIST_HEAD(efi_obj_list);

/*
 * If we're running on nasty systems (32bit ARM booting into non-EFI Linux)
 * we need to do trickery with caches. Since we don't want to break the EFI
 * aware boot path, only apply hacks when loading exiting directly (breaking
 * direct Linux EFI booting along the way - oh well).
 */
static bool efi_is_direct_boot = true;

/*
 * EFI can pass arbitrary additional "tables" containing vendor specific
 * information to the payload. One such table is the FDT table which contains
 * a pointer to a flattened device tree blob.
 *
 * In most cases we want to pass an FDT to the payload, so reserve one slot of
 * config table space for it. The pointer gets populated by do_bootefi_exec().
 */
static struct efi_configuration_table __efi_runtime_data efi_conf_table[2];

#ifdef CONFIG_ARM
/*
 * The "gd" pointer lives in a register on ARM and AArch64 that we declare
 * fixed when compiling U-Boot. However, the payload does not know about that
 * restriction so we need to manually swap its and our view of that register on
 * EFI callback entry/exit.
 */
static volatile void *efi_gd, *app_gd;
#endif

static int entry_count;
static int nesting_level;

/* Called on every callback entry */
int __efi_entry_check(void)
{
	int ret = entry_count++ == 0;
#ifdef CONFIG_ARM
	assert(efi_gd);
	app_gd = gd;
	gd = efi_gd;
#endif
	return ret;
}

/* Called on every callback exit */
int __efi_exit_check(void)
{
	int ret = --entry_count == 0;
#ifdef CONFIG_ARM
	gd = app_gd;
#endif
	return ret;
}

/* Called from do_bootefi_exec() */
void efi_save_gd(void)
{
#ifdef CONFIG_ARM
	efi_gd = gd;
#endif
}

/*
 * Special case handler for error/abort that just forces things back
 * to u-boot world so we can dump out an abort msg, without any care
 * about returning back to UEFI world.
 */
void efi_restore_gd(void)
{
#ifdef CONFIG_ARM
	/* Only restore if we're already in EFI context */
	if (!efi_gd)
		return;
	gd = efi_gd;
#endif
}

/*
 * Two spaces per indent level, maxing out at 10.. which ought to be
 * enough for anyone ;-)
 */
static const char *indent_string(int level)
{
	const char *indent = "                    ";
	const int max = strlen(indent);
	level = min(max, level * 2);
	return &indent[max - level];
}

const char *__efi_nesting_inc(void)
{
	return indent_string(nesting_level++);
}

const char *__efi_nesting_dec(void)
{
	return indent_string(--nesting_level);
}

/* Low 32 bit */
#define EFI_LOW32(a) (a & 0xFFFFFFFFULL)
/* High 32 bit */
#define EFI_HIGH32(a) (a >> 32)

/*
 * 64bit division by 10 implemented as multiplication by 1 / 10
 *
 * Decimals of one tenth: 0x1 / 0xA = 0x0.19999...
 */
#define EFI_TENTH 0x199999999999999A
static u64 efi_div10(u64 a)
{
	u64 prod;
	u64 rem;
	u64 ret;

	ret  = EFI_HIGH32(a) * EFI_HIGH32(EFI_TENTH);
	prod = EFI_HIGH32(a) * EFI_LOW32(EFI_TENTH);
	rem  = EFI_LOW32(prod);
	ret += EFI_HIGH32(prod);
	prod = EFI_LOW32(a) * EFI_HIGH32(EFI_TENTH);
	rem += EFI_LOW32(prod);
	ret += EFI_HIGH32(prod);
	prod = EFI_LOW32(a) * EFI_LOW32(EFI_TENTH);
	rem += EFI_HIGH32(prod);
	ret += EFI_HIGH32(rem);
	/* Round to nearest integer */
	if (rem >= (1 << 31))
		++ret;
	return ret;
}

void efi_signal_event(struct efi_event *event)
{
	if (event->signaled)
		return;
	event->signaled = 1;
	if (event->type & EVT_NOTIFY_SIGNAL) {
		EFI_CALL(event->notify_function(event, event->notify_context));
	}
}

static efi_status_t efi_unsupported(const char *funcname)
{
	debug("EFI: App called into unimplemented function %s\n", funcname);
	return EFI_EXIT(EFI_UNSUPPORTED);
}

static unsigned long EFIAPI efi_raise_tpl(UINTN new_tpl)
{
	EFI_ENTRY("0x%zx", new_tpl);
	return EFI_EXIT(0);
}

static void EFIAPI efi_restore_tpl(UINTN old_tpl)
{
	EFI_ENTRY("0x%zx", old_tpl);
	efi_unsupported(__func__);
}

static efi_status_t EFIAPI efi_allocate_pages_ext(int type, int memory_type,
						  unsigned long pages,
						  uint64_t *memory)
{
	efi_status_t r;

	EFI_ENTRY("%d, %d, 0x%lx, %p", type, memory_type, pages, memory);
	r = efi_allocate_pages(type, memory_type, pages, memory);
	return EFI_EXIT(r);
}

static efi_status_t EFIAPI efi_free_pages_ext(uint64_t memory,
					      unsigned long pages)
{
	efi_status_t r;

	EFI_ENTRY("%"PRIx64", 0x%lx", memory, pages);
	r = efi_free_pages(memory, pages);
	return EFI_EXIT(r);
}

static efi_status_t EFIAPI efi_get_memory_map_ext(
					unsigned long *memory_map_size,
					struct efi_mem_desc *memory_map,
					unsigned long *map_key,
					unsigned long *descriptor_size,
					uint32_t *descriptor_version)
{
	efi_status_t r;

	EFI_ENTRY("%p, %p, %p, %p, %p", memory_map_size, memory_map,
		  map_key, descriptor_size, descriptor_version);
	r = efi_get_memory_map(memory_map_size, memory_map, map_key,
			       descriptor_size, descriptor_version);
	return EFI_EXIT(r);
}

static efi_status_t EFIAPI efi_allocate_pool_ext(int pool_type,
						 unsigned long size,
						 void **buffer)
{
	efi_status_t r;

	EFI_ENTRY("%d, %ld, %p", pool_type, size, buffer);
	r = efi_allocate_pool(pool_type, size, buffer);
	return EFI_EXIT(r);
}

static efi_status_t EFIAPI efi_free_pool_ext(void *buffer)
{
	efi_status_t r;

	EFI_ENTRY("%p", buffer);
	r = efi_free_pool(buffer);
	return EFI_EXIT(r);
}

/*
 * Our event capabilities are very limited. Only a small limited
 * number of events is allowed to coexist.
 */
static struct efi_event efi_events[16];

efi_status_t efi_create_event(uint32_t type, UINTN notify_tpl,
			      void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
			      void *notify_context, struct efi_event **event)
{
	int i;

	if (event == NULL)
		return EFI_INVALID_PARAMETER;

	if ((type & EVT_NOTIFY_SIGNAL) && (type & EVT_NOTIFY_WAIT))
		return EFI_INVALID_PARAMETER;

	if ((type & (EVT_NOTIFY_SIGNAL|EVT_NOTIFY_WAIT)) &&
	    notify_function == NULL)
		return EFI_INVALID_PARAMETER;

	for (i = 0; i < ARRAY_SIZE(efi_events); ++i) {
		if (efi_events[i].type)
			continue;
		efi_events[i].type = type;
		efi_events[i].notify_tpl = notify_tpl;
		efi_events[i].notify_function = notify_function;
		efi_events[i].notify_context = notify_context;
		/* Disable timers on bootup */
		efi_events[i].trigger_next = -1ULL;
		efi_events[i].signaled = 0;
		*event = &efi_events[i];
		return EFI_SUCCESS;
	}
	return EFI_OUT_OF_RESOURCES;
}

static efi_status_t EFIAPI efi_create_event_ext(
			uint32_t type, UINTN notify_tpl,
			void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
			void *notify_context, struct efi_event **event)
{
	EFI_ENTRY("%d, 0x%zx, %p, %p", type, notify_tpl, notify_function,
		  notify_context);
	return EFI_EXIT(efi_create_event(type, notify_tpl, notify_function,
					 notify_context, event));
}


/*
 * Our timers have to work without interrupts, so we check whenever keyboard
 * input or disk accesses happen if enough time elapsed for it to fire.
 */
void efi_timer_check(void)
{
	int i;
	u64 now = timer_get_us();

	for (i = 0; i < ARRAY_SIZE(efi_events); ++i) {
		if (!efi_events[i].type ||
		    !(efi_events[i].type & EVT_TIMER) ||
		    efi_events[i].trigger_type == EFI_TIMER_STOP ||
		    now < efi_events[i].trigger_next)
			continue;
		if (efi_events[i].trigger_type == EFI_TIMER_PERIODIC) {
			efi_events[i].trigger_next +=
				efi_events[i].trigger_time;
			efi_events[i].signaled = 0;
		}
		efi_signal_event(&efi_events[i]);
	}
	WATCHDOG_RESET();
}

efi_status_t efi_set_timer(struct efi_event *event, enum efi_timer_delay type,
			   uint64_t trigger_time)
{
	int i;

	/*
	 * The parameter defines a multiple of 100ns.
	 * We use multiples of 1000ns. So divide by 10.
	 */
	trigger_time = efi_div10(trigger_time);

	for (i = 0; i < ARRAY_SIZE(efi_events); ++i) {
		if (event != &efi_events[i])
			continue;

		if (!(event->type & EVT_TIMER))
			break;
		switch (type) {
		case EFI_TIMER_STOP:
			event->trigger_next = -1ULL;
			break;
		case EFI_TIMER_PERIODIC:
		case EFI_TIMER_RELATIVE:
			event->trigger_next =
				timer_get_us() + trigger_time;
			break;
		default:
			return EFI_INVALID_PARAMETER;
		}
		event->trigger_type = type;
		event->trigger_time = trigger_time;
		return EFI_SUCCESS;
	}
	return EFI_INVALID_PARAMETER;
}

static efi_status_t EFIAPI efi_set_timer_ext(struct efi_event *event,
					     enum efi_timer_delay type,
					     uint64_t trigger_time)
{
	EFI_ENTRY("%p, %d, %"PRIx64, event, type, trigger_time);
	return EFI_EXIT(efi_set_timer(event, type, trigger_time));
}

static efi_status_t EFIAPI efi_wait_for_event(unsigned long num_events,
					      struct efi_event **event,
					      unsigned long *index)
{
	int i, j;

	EFI_ENTRY("%ld, %p, %p", num_events, event, index);

	/* Check parameters */
	if (!num_events || !event)
		return EFI_EXIT(EFI_INVALID_PARAMETER);
	for (i = 0; i < num_events; ++i) {
		for (j = 0; j < ARRAY_SIZE(efi_events); ++j) {
			if (event[i] == &efi_events[j])
				goto known_event;
		}
		return EFI_EXIT(EFI_INVALID_PARAMETER);
known_event:
		if (!event[i]->type || event[i]->type & EVT_NOTIFY_SIGNAL)
			return EFI_EXIT(EFI_INVALID_PARAMETER);
	}

	/* Wait for signal */
	for (;;) {
		for (i = 0; i < num_events; ++i) {
			if (event[i]->signaled)
				goto out;
		}
		/* Allow events to occur. */
		efi_timer_check();
	}

out:
	/*
	 * Reset the signal which is passed to the caller to allow periodic
	 * events to occur.
	 */
	event[i]->signaled = 0;
	if (index)
		*index = i;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_signal_event_ext(struct efi_event *event)
{
	int i;

	EFI_ENTRY("%p", event);
	for (i = 0; i < ARRAY_SIZE(efi_events); ++i) {
		if (event != &efi_events[i])
			continue;
		efi_signal_event(event);
		break;
	}
	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_close_event(struct efi_event *event)
{
	int i;

	EFI_ENTRY("%p", event);
	for (i = 0; i < ARRAY_SIZE(efi_events); ++i) {
		if (event == &efi_events[i]) {
			event->type = 0;
			event->trigger_next = -1ULL;
			event->signaled = 0;
			return EFI_EXIT(EFI_SUCCESS);
		}
	}
	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

static efi_status_t EFIAPI efi_check_event(struct efi_event *event)
{
	int i;

	EFI_ENTRY("%p", event);
	efi_timer_check();
	for (i = 0; i < ARRAY_SIZE(efi_events); ++i) {
		if (event != &efi_events[i])
			continue;
		if (!event->type || event->type & EVT_NOTIFY_SIGNAL)
			break;
		if (event->signaled)
			return EFI_EXIT(EFI_SUCCESS);
		return EFI_EXIT(EFI_NOT_READY);
	}
	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

static efi_status_t EFIAPI efi_install_protocol_interface(void **handle,
			efi_guid_t *protocol, int protocol_interface_type,
			void *protocol_interface)
{
	struct list_head *lhandle;
	int i;
	efi_status_t r;

	if (!handle || !protocol ||
	    protocol_interface_type != EFI_NATIVE_INTERFACE) {
		r = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Create new handle if requested. */
	if (!*handle) {
		r = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	/* Find object. */
	list_for_each(lhandle, &efi_obj_list) {
		struct efi_object *efiobj;
		efiobj = list_entry(lhandle, struct efi_object, link);

		if (efiobj->handle != *handle)
			continue;
		/* Check if protocol is already installed on the handle. */
		for (i = 0; i < ARRAY_SIZE(efiobj->protocols); i++) {
			struct efi_handler *handler = &efiobj->protocols[i];

			if (!handler->guid)
				continue;
			if (!guidcmp(handler->guid, protocol)) {
				r = EFI_INVALID_PARAMETER;
				goto out;
			}
		}
		/* Install protocol in first empty slot. */
		for (i = 0; i < ARRAY_SIZE(efiobj->protocols); i++) {
			struct efi_handler *handler = &efiobj->protocols[i];

			if (handler->guid)
				continue;

			handler->guid = protocol;
			handler->protocol_interface = protocol_interface;
			r = EFI_SUCCESS;
			goto out;
		}
		r = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	r = EFI_INVALID_PARAMETER;
out:
	return r;
}

static efi_status_t EFIAPI efi_install_protocol_interface_ext(void **handle,
			efi_guid_t *protocol, int protocol_interface_type,
			void *protocol_interface)
{
	EFI_ENTRY("%p, %p, %d, %p", handle, protocol, protocol_interface_type,
		  protocol_interface);

	return EFI_EXIT(efi_install_protocol_interface(handle, protocol,
						       protocol_interface_type,
						       protocol_interface));
}

static efi_status_t EFIAPI efi_reinstall_protocol_interface(void *handle,
			efi_guid_t *protocol, void *old_interface,
			void *new_interface)
{
	EFI_ENTRY("%p, %p, %p, %p", handle, protocol, old_interface,
		  new_interface);
	return EFI_EXIT(EFI_ACCESS_DENIED);
}

static efi_status_t EFIAPI efi_uninstall_protocol_interface(void *handle,
			efi_guid_t *protocol, void *protocol_interface)
{
	struct list_head *lhandle;
	int i;
	efi_status_t r = EFI_NOT_FOUND;

	if (!handle || !protocol) {
		r = EFI_INVALID_PARAMETER;
		goto out;
	}

	list_for_each(lhandle, &efi_obj_list) {
		struct efi_object *efiobj;
		efiobj = list_entry(lhandle, struct efi_object, link);

		if (efiobj->handle != handle)
			continue;

		for (i = 0; i < ARRAY_SIZE(efiobj->protocols); i++) {
			struct efi_handler *handler = &efiobj->protocols[i];
			const efi_guid_t *hprotocol = handler->guid;

			if (!hprotocol)
				continue;
			if (!guidcmp(hprotocol, protocol)) {
				if (handler->protocol_interface) {
					r = EFI_ACCESS_DENIED;
				} else {
					handler->guid = 0;
					r = EFI_SUCCESS;
				}
				goto out;
			}
		}
	}

out:
	return r;
}

static efi_status_t EFIAPI efi_uninstall_protocol_interface_ext(void *handle,
			efi_guid_t *protocol, void *protocol_interface)
{
	EFI_ENTRY("%p, %p, %p", handle, protocol, protocol_interface);

	return EFI_EXIT(efi_uninstall_protocol_interface(handle, protocol,
							 protocol_interface));
}

static efi_status_t EFIAPI efi_register_protocol_notify(efi_guid_t *protocol,
							struct efi_event *event,
							void **registration)
{
	EFI_ENTRY("%p, %p, %p", protocol, event, registration);
	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
}

static int efi_search(enum efi_locate_search_type search_type,
		      efi_guid_t *protocol, void *search_key,
		      struct efi_object *efiobj)
{
	int i;

	switch (search_type) {
	case all_handles:
		return 0;
	case by_register_notify:
		return -1;
	case by_protocol:
		for (i = 0; i < ARRAY_SIZE(efiobj->protocols); i++) {
			const efi_guid_t *guid = efiobj->protocols[i].guid;
			if (guid && !guidcmp(guid, protocol))
				return 0;
		}
		return -1;
	}

	return -1;
}

static efi_status_t efi_locate_handle(
			enum efi_locate_search_type search_type,
			efi_guid_t *protocol, void *search_key,
			unsigned long *buffer_size, efi_handle_t *buffer)
{
	struct list_head *lhandle;
	unsigned long size = 0;

	/* Count how much space we need */
	list_for_each(lhandle, &efi_obj_list) {
		struct efi_object *efiobj;
		efiobj = list_entry(lhandle, struct efi_object, link);
		if (!efi_search(search_type, protocol, search_key, efiobj)) {
			size += sizeof(void*);
		}
	}

	if (*buffer_size < size) {
		*buffer_size = size;
		return EFI_BUFFER_TOO_SMALL;
	}

	*buffer_size = size;
	if (size == 0)
		return EFI_NOT_FOUND;

	/* Then fill the array */
	list_for_each(lhandle, &efi_obj_list) {
		struct efi_object *efiobj;
		efiobj = list_entry(lhandle, struct efi_object, link);
		if (!efi_search(search_type, protocol, search_key, efiobj)) {
			*(buffer++) = efiobj->handle;
		}
	}

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_locate_handle_ext(
			enum efi_locate_search_type search_type,
			efi_guid_t *protocol, void *search_key,
			unsigned long *buffer_size, efi_handle_t *buffer)
{
	EFI_ENTRY("%d, %p, %p, %p, %p", search_type, protocol, search_key,
		  buffer_size, buffer);

	return EFI_EXIT(efi_locate_handle(search_type, protocol, search_key,
			buffer_size, buffer));
}

static efi_status_t EFIAPI efi_locate_device_path(efi_guid_t *protocol,
			struct efi_device_path **device_path,
			efi_handle_t *device)
{
	EFI_ENTRY("%p, %p, %p", protocol, device_path, device);
	return EFI_EXIT(EFI_NOT_FOUND);
}

/* Collapses configuration table entries, removing index i */
static void efi_remove_configuration_table(int i)
{
	struct efi_configuration_table *this = &efi_conf_table[i];
	struct efi_configuration_table *next = &efi_conf_table[i+1];
	struct efi_configuration_table *end = &efi_conf_table[systab.nr_tables];

	memmove(this, next, (ulong)end - (ulong)next);
	systab.nr_tables--;
}

efi_status_t efi_install_configuration_table(const efi_guid_t *guid, void *table)
{
	int i;

	/* Check for guid override */
	for (i = 0; i < systab.nr_tables; i++) {
		if (!guidcmp(guid, &efi_conf_table[i].guid)) {
			if (table)
				efi_conf_table[i].table = table;
			else
				efi_remove_configuration_table(i);
			return EFI_SUCCESS;
		}
	}

	if (!table)
		return EFI_NOT_FOUND;

	/* No override, check for overflow */
	if (i >= ARRAY_SIZE(efi_conf_table))
		return EFI_OUT_OF_RESOURCES;

	/* Add a new entry */
	memcpy(&efi_conf_table[i].guid, guid, sizeof(*guid));
	efi_conf_table[i].table = table;
	systab.nr_tables = i + 1;

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_install_configuration_table_ext(efi_guid_t *guid,
							       void *table)
{
	EFI_ENTRY("%p, %p", guid, table);
	return EFI_EXIT(efi_install_configuration_table(guid, table));
}

static efi_status_t EFIAPI efi_load_image(bool boot_policy,
					  efi_handle_t parent_image,
					  struct efi_device_path *file_path,
					  void *source_buffer,
					  unsigned long source_size,
					  efi_handle_t *image_handle)
{
	static struct efi_object loaded_image_info_obj = {
		.protocols = {
			{
				.guid = &efi_guid_loaded_image,
			},
		},
	};
	struct efi_loaded_image *info;
	struct efi_object *obj;

	EFI_ENTRY("%d, %p, %p, %p, %ld, %p", boot_policy, parent_image,
		  file_path, source_buffer, source_size, image_handle);
	info = malloc(sizeof(*info));
	loaded_image_info_obj.protocols[0].protocol_interface = info;
	obj = malloc(sizeof(loaded_image_info_obj));
	memset(info, 0, sizeof(*info));
	memcpy(obj, &loaded_image_info_obj, sizeof(loaded_image_info_obj));
	obj->handle = info;
	info->file_path = file_path;
	info->reserved = efi_load_pe(source_buffer, info);
	if (!info->reserved) {
		free(info);
		free(obj);
		return EFI_EXIT(EFI_UNSUPPORTED);
	}

	*image_handle = info;
	list_add_tail(&obj->link, &efi_obj_list);

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_start_image(efi_handle_t image_handle,
					   unsigned long *exit_data_size,
					   s16 **exit_data)
{
	ulong (*entry)(void *image_handle, struct efi_system_table *st);
	struct efi_loaded_image *info = image_handle;

	EFI_ENTRY("%p, %p, %p", image_handle, exit_data_size, exit_data);
	entry = info->reserved;

	efi_is_direct_boot = false;

	/* call the image! */
	if (setjmp(&info->exit_jmp)) {
		/* We returned from the child image */
		return EFI_EXIT(info->exit_status);
	}

	__efi_nesting_dec();
	__efi_exit_check();
	entry(image_handle, &systab);
	__efi_entry_check();
	__efi_nesting_inc();

	/* Should usually never get here */
	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_exit(efi_handle_t image_handle,
			efi_status_t exit_status, unsigned long exit_data_size,
			int16_t *exit_data)
{
	struct efi_loaded_image *loaded_image_info = (void*)image_handle;

	EFI_ENTRY("%p, %ld, %ld, %p", image_handle, exit_status,
		  exit_data_size, exit_data);

	loaded_image_info->exit_status = exit_status;
	longjmp(&loaded_image_info->exit_jmp, 1);

	panic("EFI application exited");
}

static struct efi_object *efi_search_obj(void *handle)
{
	struct list_head *lhandle;

	list_for_each(lhandle, &efi_obj_list) {
		struct efi_object *efiobj;
		efiobj = list_entry(lhandle, struct efi_object, link);
		if (efiobj->handle == handle)
			return efiobj;
	}

	return NULL;
}

static efi_status_t EFIAPI efi_unload_image(void *image_handle)
{
	struct efi_object *efiobj;

	EFI_ENTRY("%p", image_handle);
	efiobj = efi_search_obj(image_handle);
	if (efiobj)
		list_del(&efiobj->link);

	return EFI_EXIT(EFI_SUCCESS);
}

static void efi_exit_caches(void)
{
#if defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	/*
	 * Grub on 32bit ARM needs to have caches disabled before jumping into
	 * a zImage, but does not know of all cache layers. Give it a hand.
	 */
	if (efi_is_direct_boot)
		cleanup_before_linux();
#endif
}

static efi_status_t EFIAPI efi_exit_boot_services(void *image_handle,
						  unsigned long map_key)
{
	EFI_ENTRY("%p, %ld", image_handle, map_key);

	board_quiesce_devices();

	/* Fix up caches for EFI payloads if necessary */
	efi_exit_caches();

	/* This stops all lingering devices */
	bootm_disable_interrupts();

	/* Give the payload some time to boot */
	WATCHDOG_RESET();

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_get_next_monotonic_count(uint64_t *count)
{
	static uint64_t mono = 0;
	EFI_ENTRY("%p", count);
	*count = mono++;
	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_stall(unsigned long microseconds)
{
	EFI_ENTRY("%ld", microseconds);
	udelay(microseconds);
	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_set_watchdog_timer(unsigned long timeout,
						  uint64_t watchdog_code,
						  unsigned long data_size,
						  uint16_t *watchdog_data)
{
	EFI_ENTRY("%ld, 0x%"PRIx64", %ld, %p", timeout, watchdog_code,
		  data_size, watchdog_data);
	return efi_unsupported(__func__);
}

static efi_status_t EFIAPI efi_connect_controller(
			efi_handle_t controller_handle,
			efi_handle_t *driver_image_handle,
			struct efi_device_path *remain_device_path,
			bool recursive)
{
	EFI_ENTRY("%p, %p, %p, %d", controller_handle, driver_image_handle,
		  remain_device_path, recursive);
	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI efi_disconnect_controller(void *controller_handle,
						     void *driver_image_handle,
						     void *child_handle)
{
	EFI_ENTRY("%p, %p, %p", controller_handle, driver_image_handle,
		  child_handle);
	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

static efi_status_t EFIAPI efi_close_protocol(void *handle,
					      efi_guid_t *protocol,
					      void *agent_handle,
					      void *controller_handle)
{
	EFI_ENTRY("%p, %p, %p, %p", handle, protocol, agent_handle,
		  controller_handle);
	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI efi_open_protocol_information(efi_handle_t handle,
			efi_guid_t *protocol,
			struct efi_open_protocol_info_entry **entry_buffer,
			unsigned long *entry_count)
{
	EFI_ENTRY("%p, %p, %p, %p", handle, protocol, entry_buffer,
		  entry_count);
	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI efi_protocols_per_handle(void *handle,
			efi_guid_t ***protocol_buffer,
			unsigned long *protocol_buffer_count)
{
	unsigned long buffer_size;
	struct efi_object *efiobj;
	unsigned long i, j;
	struct list_head *lhandle;
	efi_status_t r;

	EFI_ENTRY("%p, %p, %p", handle, protocol_buffer,
		  protocol_buffer_count);

	if (!handle || !protocol_buffer || !protocol_buffer_count)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	*protocol_buffer = NULL;
	*protocol_buffer_count = 0;
	list_for_each(lhandle, &efi_obj_list) {
		efiobj = list_entry(lhandle, struct efi_object, link);

		if (efiobj->handle != handle)
			continue;

		/* Count protocols */
		for (i = 0; i < ARRAY_SIZE(efiobj->protocols); i++) {
			if (efiobj->protocols[i].guid)
				++*protocol_buffer_count;
		}
		/* Copy guids */
		if (*protocol_buffer_count) {
			buffer_size = sizeof(efi_guid_t *) *
					*protocol_buffer_count;
			r = efi_allocate_pool(EFI_ALLOCATE_ANY_PAGES,
					      buffer_size,
					      (void **)protocol_buffer);
			if (r != EFI_SUCCESS)
				return EFI_EXIT(r);
			j = 0;
			for (i = 0; i < ARRAY_SIZE(efiobj->protocols); ++i) {
				if (efiobj->protocols[i].guid) {
					(*protocol_buffer)[j] = (void *)
						efiobj->protocols[i].guid;
					++j;
				}
			}
		}
		break;
	}

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_locate_handle_buffer(
			enum efi_locate_search_type search_type,
			efi_guid_t *protocol, void *search_key,
			unsigned long *no_handles, efi_handle_t **buffer)
{
	efi_status_t r;
	unsigned long buffer_size = 0;

	EFI_ENTRY("%d, %p, %p, %p, %p", search_type, protocol, search_key,
		  no_handles, buffer);

	if (!no_handles || !buffer) {
		r = EFI_INVALID_PARAMETER;
		goto out;
	}
	*no_handles = 0;
	*buffer = NULL;
	r = efi_locate_handle(search_type, protocol, search_key, &buffer_size,
			      *buffer);
	if (r != EFI_BUFFER_TOO_SMALL)
		goto out;
	r = efi_allocate_pool(EFI_ALLOCATE_ANY_PAGES, buffer_size,
			      (void **)buffer);
	if (r != EFI_SUCCESS)
		goto out;
	r = efi_locate_handle(search_type, protocol, search_key, &buffer_size,
			      *buffer);
	if (r == EFI_SUCCESS)
		*no_handles = buffer_size / sizeof(void *);
out:
	return EFI_EXIT(r);
}

static efi_status_t EFIAPI efi_locate_protocol(efi_guid_t *protocol,
					       void *registration,
					       void **protocol_interface)
{
	struct list_head *lhandle;
	int i;

	EFI_ENTRY("%p, %p, %p", protocol, registration, protocol_interface);

	if (!protocol || !protocol_interface)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	list_for_each(lhandle, &efi_obj_list) {
		struct efi_object *efiobj;

		efiobj = list_entry(lhandle, struct efi_object, link);
		for (i = 0; i < ARRAY_SIZE(efiobj->protocols); i++) {
			struct efi_handler *handler = &efiobj->protocols[i];

			if (!handler->guid)
				continue;
			if (!guidcmp(handler->guid, protocol)) {
				*protocol_interface =
					handler->protocol_interface;
				return EFI_EXIT(EFI_SUCCESS);
			}
		}
	}
	*protocol_interface = NULL;

	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI efi_install_multiple_protocol_interfaces(
			void **handle, ...)
{
	EFI_ENTRY("%p", handle);

	va_list argptr;
	efi_guid_t *protocol;
	void *protocol_interface;
	efi_status_t r = EFI_SUCCESS;
	int i = 0;

	if (!handle)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	va_start(argptr, handle);
	for (;;) {
		protocol = va_arg(argptr, efi_guid_t*);
		if (!protocol)
			break;
		protocol_interface = va_arg(argptr, void*);
		r = efi_install_protocol_interface(handle, protocol,
						   EFI_NATIVE_INTERFACE,
						   protocol_interface);
		if (r != EFI_SUCCESS)
			break;
		i++;
	}
	va_end(argptr);
	if (r == EFI_SUCCESS)
		return EFI_EXIT(r);

	/* If an error occured undo all changes. */
	va_start(argptr, handle);
	for (; i; --i) {
		protocol = va_arg(argptr, efi_guid_t*);
		protocol_interface = va_arg(argptr, void*);
		efi_uninstall_protocol_interface(handle, protocol,
						 protocol_interface);
	}
	va_end(argptr);

	return EFI_EXIT(r);
}

static efi_status_t EFIAPI efi_uninstall_multiple_protocol_interfaces(
			void *handle, ...)
{
	EFI_ENTRY("%p", handle);
	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

static efi_status_t EFIAPI efi_calculate_crc32(void *data,
					       unsigned long data_size,
					       uint32_t *crc32_p)
{
	EFI_ENTRY("%p, %ld", data, data_size);
	*crc32_p = crc32(0, data, data_size);
	return EFI_EXIT(EFI_SUCCESS);
}

static void EFIAPI efi_copy_mem(void *destination, void *source,
				unsigned long length)
{
	EFI_ENTRY("%p, %p, %ld", destination, source, length);
	memcpy(destination, source, length);
}

static void EFIAPI efi_set_mem(void *buffer, unsigned long size, uint8_t value)
{
	EFI_ENTRY("%p, %ld, 0x%x", buffer, size, value);
	memset(buffer, value, size);
}

static efi_status_t EFIAPI efi_open_protocol(
			void *handle, efi_guid_t *protocol,
			void **protocol_interface, void *agent_handle,
			void *controller_handle, uint32_t attributes)
{
	struct list_head *lhandle;
	int i;
	efi_status_t r = EFI_INVALID_PARAMETER;

	EFI_ENTRY("%p, %p, %p, %p, %p, 0x%x", handle, protocol,
		  protocol_interface, agent_handle, controller_handle,
		  attributes);

	if (!handle || !protocol ||
	    (!protocol_interface && attributes !=
	     EFI_OPEN_PROTOCOL_TEST_PROTOCOL)) {
		goto out;
	}

	switch (attributes) {
	case EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL:
	case EFI_OPEN_PROTOCOL_GET_PROTOCOL:
	case EFI_OPEN_PROTOCOL_TEST_PROTOCOL:
		break;
	case EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER:
		if (controller_handle == handle)
			goto out;
	case EFI_OPEN_PROTOCOL_BY_DRIVER:
	case EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE:
		if (controller_handle == NULL)
			goto out;
	case EFI_OPEN_PROTOCOL_EXCLUSIVE:
		if (agent_handle == NULL)
			goto out;
		break;
	default:
		goto out;
	}

	list_for_each(lhandle, &efi_obj_list) {
		struct efi_object *efiobj;
		efiobj = list_entry(lhandle, struct efi_object, link);

		if (efiobj->handle != handle)
			continue;

		for (i = 0; i < ARRAY_SIZE(efiobj->protocols); i++) {
			struct efi_handler *handler = &efiobj->protocols[i];
			const efi_guid_t *hprotocol = handler->guid;
			if (!hprotocol)
				continue;
			if (!guidcmp(hprotocol, protocol)) {
				if (attributes !=
				    EFI_OPEN_PROTOCOL_TEST_PROTOCOL) {
					*protocol_interface =
						handler->protocol_interface;
				}
				r = EFI_SUCCESS;
				goto out;
			}
		}
		goto unsupported;
	}

unsupported:
	r = EFI_UNSUPPORTED;
out:
	return EFI_EXIT(r);
}

static efi_status_t EFIAPI efi_handle_protocol(void *handle,
					       efi_guid_t *protocol,
					       void **protocol_interface)
{
	return efi_open_protocol(handle, protocol, protocol_interface, NULL,
				 NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
}

static const struct efi_boot_services efi_boot_services = {
	.hdr = {
		.headersize = sizeof(struct efi_table_hdr),
	},
	.raise_tpl = efi_raise_tpl,
	.restore_tpl = efi_restore_tpl,
	.allocate_pages = efi_allocate_pages_ext,
	.free_pages = efi_free_pages_ext,
	.get_memory_map = efi_get_memory_map_ext,
	.allocate_pool = efi_allocate_pool_ext,
	.free_pool = efi_free_pool_ext,
	.create_event = efi_create_event_ext,
	.set_timer = efi_set_timer_ext,
	.wait_for_event = efi_wait_for_event,
	.signal_event = efi_signal_event_ext,
	.close_event = efi_close_event,
	.check_event = efi_check_event,
	.install_protocol_interface = efi_install_protocol_interface_ext,
	.reinstall_protocol_interface = efi_reinstall_protocol_interface,
	.uninstall_protocol_interface = efi_uninstall_protocol_interface_ext,
	.handle_protocol = efi_handle_protocol,
	.reserved = NULL,
	.register_protocol_notify = efi_register_protocol_notify,
	.locate_handle = efi_locate_handle_ext,
	.locate_device_path = efi_locate_device_path,
	.install_configuration_table = efi_install_configuration_table_ext,
	.load_image = efi_load_image,
	.start_image = efi_start_image,
	.exit = efi_exit,
	.unload_image = efi_unload_image,
	.exit_boot_services = efi_exit_boot_services,
	.get_next_monotonic_count = efi_get_next_monotonic_count,
	.stall = efi_stall,
	.set_watchdog_timer = efi_set_watchdog_timer,
	.connect_controller = efi_connect_controller,
	.disconnect_controller = efi_disconnect_controller,
	.open_protocol = efi_open_protocol,
	.close_protocol = efi_close_protocol,
	.open_protocol_information = efi_open_protocol_information,
	.protocols_per_handle = efi_protocols_per_handle,
	.locate_handle_buffer = efi_locate_handle_buffer,
	.locate_protocol = efi_locate_protocol,
	.install_multiple_protocol_interfaces = efi_install_multiple_protocol_interfaces,
	.uninstall_multiple_protocol_interfaces = efi_uninstall_multiple_protocol_interfaces,
	.calculate_crc32 = efi_calculate_crc32,
	.copy_mem = efi_copy_mem,
	.set_mem = efi_set_mem,
};


static uint16_t __efi_runtime_data firmware_vendor[] =
	{ 'D','a','s',' ','U','-','b','o','o','t',0 };

struct efi_system_table __efi_runtime_data systab = {
	.hdr = {
		.signature = EFI_SYSTEM_TABLE_SIGNATURE,
		.revision = 0x20005, /* 2.5 */
		.headersize = sizeof(struct efi_table_hdr),
	},
	.fw_vendor = (long)firmware_vendor,
	.con_in = (void*)&efi_con_in,
	.con_out = (void*)&efi_con_out,
	.std_err = (void*)&efi_con_out,
	.runtime = (void*)&efi_runtime_services,
	.boottime = (void*)&efi_boot_services,
	.nr_tables = 0,
	.tables = (void*)efi_conf_table,
};
