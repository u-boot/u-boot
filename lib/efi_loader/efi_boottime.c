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

/* Called from do_bootefi_exec() */
void efi_save_gd(void)
{
#ifdef CONFIG_ARM
	efi_gd = gd;
#endif
}

/* Called on every callback entry */
void efi_restore_gd(void)
{
#ifdef CONFIG_ARM
	/* Only restore if we're already in EFI context */
	if (!efi_gd)
		return;

	if (gd != efi_gd)
		app_gd = gd;
	gd = efi_gd;
#endif
}

/* Called on every callback exit */
efi_status_t efi_exit_func(efi_status_t ret)
{
#ifdef CONFIG_ARM
	gd = app_gd;
#endif

	return ret;
}

static efi_status_t efi_unsupported(const char *funcname)
{
	debug("EFI: App called into unimplemented function %s\n", funcname);
	return EFI_EXIT(EFI_UNSUPPORTED);
}

static int guidcmp(const efi_guid_t *g1, const efi_guid_t *g2)
{
	return memcmp(g1, g2, sizeof(efi_guid_t));
}

static unsigned long EFIAPI efi_raise_tpl(unsigned long new_tpl)
{
	EFI_ENTRY("0x%lx", new_tpl);
	return EFI_EXIT(0);
}

static void EFIAPI efi_restore_tpl(unsigned long old_tpl)
{
	EFI_ENTRY("0x%lx", old_tpl);
	EFI_EXIT(efi_unsupported(__func__));
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
 * Our event capabilities are very limited. Only support a single
 * event to exist, so we don't need to maintain lists.
 */
static struct {
	enum efi_event_type type;
	u32 trigger_type;
	u32 trigger_time;
	u64 trigger_next;
	unsigned long notify_tpl;
	void (EFIAPI *notify_function) (void *event, void *context);
	void *notify_context;
} efi_event = {
	/* Disable timers on bootup */
	.trigger_next = -1ULL,
};

static efi_status_t EFIAPI efi_create_event(
			enum efi_event_type type, ulong notify_tpl,
			void (EFIAPI *notify_function) (void *event,
							void *context),
			void *notify_context, void **event)
{
	EFI_ENTRY("%d, 0x%lx, %p, %p", type, notify_tpl, notify_function,
		  notify_context);
	if (efi_event.notify_function) {
		/* We only support one event at a time */
		return EFI_EXIT(EFI_OUT_OF_RESOURCES);
	}

	if (event == NULL)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	if ((type & EVT_NOTIFY_SIGNAL) && (type & EVT_NOTIFY_WAIT))
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	if ((type & (EVT_NOTIFY_SIGNAL|EVT_NOTIFY_WAIT)) &&
	    notify_function == NULL)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	efi_event.type = type;
	efi_event.notify_tpl = notify_tpl;
	efi_event.notify_function = notify_function;
	efi_event.notify_context = notify_context;
	*event = &efi_event;

	return EFI_EXIT(EFI_SUCCESS);
}

/*
 * Our timers have to work without interrupts, so we check whenever keyboard
 * input or disk accesses happen if enough time elapsed for it to fire.
 */
void efi_timer_check(void)
{
	u64 now = timer_get_us();

	if (now >= efi_event.trigger_next) {
		/* Triggering! */
		if (efi_event.trigger_type == EFI_TIMER_PERIODIC)
			efi_event.trigger_next += efi_event.trigger_time / 10;
		if (efi_event.type & (EVT_NOTIFY_WAIT | EVT_NOTIFY_SIGNAL))
			efi_event.notify_function(&efi_event,
			                          efi_event.notify_context);
	}

	WATCHDOG_RESET();
}

static efi_status_t EFIAPI efi_set_timer(void *event, int type,
					 uint64_t trigger_time)
{
	/* We don't have 64bit division available everywhere, so limit timer
	 * distances to 32bit bits. */
	u32 trigger32 = trigger_time;

	EFI_ENTRY("%p, %d, %"PRIx64, event, type, trigger_time);

	if (trigger32 < trigger_time) {
		printf("WARNING: Truncating timer from %"PRIx64" to %x\n",
		       trigger_time, trigger32);
	}

	if (event != &efi_event) {
		/* We only support one event at a time */
		return EFI_EXIT(EFI_INVALID_PARAMETER);
	}

	switch (type) {
	case EFI_TIMER_STOP:
		efi_event.trigger_next = -1ULL;
		break;
	case EFI_TIMER_PERIODIC:
	case EFI_TIMER_RELATIVE:
		efi_event.trigger_next = timer_get_us() + (trigger32 / 10);
		break;
	default:
		return EFI_EXIT(EFI_INVALID_PARAMETER);
	}
	efi_event.trigger_type = type;
	efi_event.trigger_time = trigger_time;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_wait_for_event(unsigned long num_events,
					      void *event, unsigned long *index)
{
	u64 now;

	EFI_ENTRY("%ld, %p, %p", num_events, event, index);

	now = timer_get_us();
	while (now < efi_event.trigger_next) { }
	efi_timer_check();

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_signal_event(void *event)
{
	EFI_ENTRY("%p", event);
	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_close_event(void *event)
{
	EFI_ENTRY("%p", event);
	efi_event.trigger_next = -1ULL;
	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_check_event(void *event)
{
	EFI_ENTRY("%p", event);
	return EFI_EXIT(EFI_NOT_READY);
}

static efi_status_t EFIAPI efi_install_protocol_interface(void **handle,
			efi_guid_t *protocol, int protocol_interface_type,
			void *protocol_interface)
{
	EFI_ENTRY("%p, %p, %d, %p", handle, protocol, protocol_interface_type,
		  protocol_interface);
	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
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
	EFI_ENTRY("%p, %p, %p", handle, protocol, protocol_interface);
	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI efi_register_protocol_notify(efi_guid_t *protocol,
							void *event,
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

static efi_status_t EFIAPI efi_locate_handle(
			enum efi_locate_search_type search_type,
			efi_guid_t *protocol, void *search_key,
			unsigned long *buffer_size, efi_handle_t *buffer)
{
	struct list_head *lhandle;
	unsigned long size = 0;

	EFI_ENTRY("%d, %p, %p, %p, %p", search_type, protocol, search_key,
		  buffer_size, buffer);

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
		return EFI_EXIT(EFI_BUFFER_TOO_SMALL);
	}

	/* Then fill the array */
	list_for_each(lhandle, &efi_obj_list) {
		struct efi_object *efiobj;
		efiobj = list_entry(lhandle, struct efi_object, link);
		if (!efi_search(search_type, protocol, search_key, efiobj)) {
			*(buffer++) = efiobj->handle;
		}
	}

	*buffer_size = size;
	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_locate_device_path(efi_guid_t *protocol,
			struct efi_device_path **device_path,
			efi_handle_t *device)
{
	EFI_ENTRY("%p, %p, %p", protocol, device_path, device);
	return EFI_EXIT(EFI_NOT_FOUND);
}

efi_status_t efi_install_configuration_table(const efi_guid_t *guid, void *table)
{
	int i;

	/* Check for guid override */
	for (i = 0; i < systab.nr_tables; i++) {
		if (!guidcmp(guid, &efi_conf_table[i].guid)) {
			efi_conf_table[i].table = table;
			return EFI_SUCCESS;
		}
	}

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
				.open = &efi_return_handle,
			},
		},
	};
	struct efi_loaded_image *info;
	struct efi_object *obj;

	EFI_ENTRY("%d, %p, %p, %p, %ld, %p", boot_policy, parent_image,
		  file_path, source_buffer, source_size, image_handle);
	info = malloc(sizeof(*info));
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

	entry(image_handle, &systab);

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
	return EFI_EXIT(efi_unsupported(__func__));
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
	EFI_ENTRY("%p, %p, %p", handle, protocol_buffer,
		  protocol_buffer_count);
	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
}

static efi_status_t EFIAPI efi_locate_handle_buffer(
			enum efi_locate_search_type search_type,
			efi_guid_t *protocol, void *search_key,
			unsigned long *no_handles, efi_handle_t **buffer)
{
	EFI_ENTRY("%d, %p, %p, %p, %p", search_type, protocol, search_key,
		  no_handles, buffer);
	return EFI_EXIT(EFI_NOT_FOUND);
}

static struct efi_class_map efi_class_maps[] = {
	{
		.guid = &efi_guid_console_control,
		.interface = &efi_console_control
	},
};

static efi_status_t EFIAPI efi_locate_protocol(efi_guid_t *protocol,
					       void *registration,
					       void **protocol_interface)
{
	int i;

	EFI_ENTRY("%p, %p, %p", protocol, registration, protocol_interface);
	for (i = 0; i < ARRAY_SIZE(efi_class_maps); i++) {
		struct efi_class_map *curmap = &efi_class_maps[i];
		if (!guidcmp(protocol, curmap->guid)) {
			*protocol_interface = (void*)curmap->interface;
			return EFI_EXIT(EFI_SUCCESS);
		}
	}

	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI efi_install_multiple_protocol_interfaces(
			void **handle, ...)
{
	EFI_ENTRY("%p", handle);
	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
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
	efi_status_t r = EFI_UNSUPPORTED;

	EFI_ENTRY("%p, %p, %p, %p, %p, 0x%x", handle, protocol,
		  protocol_interface, agent_handle, controller_handle,
		  attributes);
	list_for_each(lhandle, &efi_obj_list) {
		struct efi_object *efiobj;
		efiobj = list_entry(lhandle, struct efi_object, link);

		if (efiobj->handle != handle)
			continue;

		for (i = 0; i < ARRAY_SIZE(efiobj->protocols); i++) {
			struct efi_handler *handler = &efiobj->protocols[i];
			const efi_guid_t *hprotocol = handler->guid;
			if (!hprotocol)
				break;
			if (!guidcmp(hprotocol, protocol)) {
				r = handler->open(handle, protocol,
				    protocol_interface, agent_handle,
				    controller_handle, attributes);
				goto out;
			}
		}
	}

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
	.create_event = efi_create_event,
	.set_timer = efi_set_timer,
	.wait_for_event = efi_wait_for_event,
	.signal_event = efi_signal_event,
	.close_event = efi_close_event,
	.check_event = efi_check_event,
	.install_protocol_interface = efi_install_protocol_interface,
	.reinstall_protocol_interface = efi_reinstall_protocol_interface,
	.uninstall_protocol_interface = efi_uninstall_protocol_interface,
	.handle_protocol = efi_handle_protocol,
	.reserved = NULL,
	.register_protocol_notify = efi_register_protocol_notify,
	.locate_handle = efi_locate_handle,
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
