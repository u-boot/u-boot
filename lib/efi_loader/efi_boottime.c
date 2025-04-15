// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI application boot time services
 *
 * Copyright (c) 2016 Alexander Graf
 */

#define LOG_CATEGORY LOGC_EFI

#include <bootm.h>
#include <div64.h>
#include <dm/device.h>
#include <dm/root.h>
#include <efi_loader.h>
#include <irq_func.h>
#include <log.h>
#include <malloc.h>
#include <net-common.h>
#include <pe.h>
#include <time.h>
#include <u-boot/crc.h>
#include <usb.h>
#include <watchdog.h>
#include <asm/global_data.h>
#include <linux/libfdt_env.h>

DECLARE_GLOBAL_DATA_PTR;

/* Task priority level */
static efi_uintn_t efi_tpl = TPL_APPLICATION;

/* This list contains all the EFI objects our payload has access to */
LIST_HEAD(efi_obj_list);

/* List of all events */
__efi_runtime_data LIST_HEAD(efi_events);

/* List of queued events */
static LIST_HEAD(efi_event_queue);

/* Flag to disable timer activity in ExitBootServices() */
static bool timers_enabled = true;

/* Flag used by the selftest to avoid detaching devices in ExitBootServices() */
bool efi_st_keep_devices;

/* List of all events registered by RegisterProtocolNotify() */
static LIST_HEAD(efi_register_notify_events);

/* Handle of the currently executing image */
static efi_handle_t current_image;

#if defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * The "gd" pointer lives in a register on ARM and RISC-V that we declare
 * fixed when compiling U-Boot. However, the payload does not know about that
 * restriction so we need to manually swap its and our view of that register on
 * EFI callback entry/exit.
 */
static volatile gd_t *efi_gd, *app_gd;
#endif

efi_status_t efi_uninstall_protocol
		(efi_handle_t handle, const efi_guid_t *protocol,
		 void *protocol_interface, bool preserve);

/* 1 if inside U-Boot code, 0 if inside EFI payload code */
static int entry_count = 1;
static int nesting_level;
/* GUID of the device tree table */
const efi_guid_t efi_guid_fdt = EFI_FDT_GUID;
/* GUID of the EFI_DRIVER_BINDING_PROTOCOL */
const efi_guid_t efi_guid_driver_binding_protocol =
			EFI_DRIVER_BINDING_PROTOCOL_GUID;

/* event group ExitBootServices() invoked */
const efi_guid_t efi_guid_event_group_exit_boot_services =
			EFI_EVENT_GROUP_EXIT_BOOT_SERVICES;
/* event group before ExitBootServices() invoked */
const efi_guid_t efi_guid_event_group_before_exit_boot_services =
			EFI_EVENT_GROUP_BEFORE_EXIT_BOOT_SERVICES;
/* event group SetVirtualAddressMap() invoked */
const efi_guid_t efi_guid_event_group_virtual_address_change =
			EFI_EVENT_GROUP_VIRTUAL_ADDRESS_CHANGE;
/* event group memory map changed */
const efi_guid_t efi_guid_event_group_memory_map_change =
			EFI_EVENT_GROUP_MEMORY_MAP_CHANGE;
/* event group boot manager about to boot */
const efi_guid_t efi_guid_event_group_ready_to_boot =
			EFI_EVENT_GROUP_READY_TO_BOOT;
/* event group ResetSystem() invoked (before ExitBootServices) */
const efi_guid_t efi_guid_event_group_reset_system =
			EFI_EVENT_GROUP_RESET_SYSTEM;
/* event group return to efibootmgr */
const efi_guid_t efi_guid_event_group_return_to_efibootmgr =
			EFI_EVENT_GROUP_RETURN_TO_EFIBOOTMGR;
/* GUIDs of the Load File and Load File2 protocols */
const efi_guid_t efi_guid_load_file_protocol = EFI_LOAD_FILE_PROTOCOL_GUID;
const efi_guid_t efi_guid_load_file2_protocol = EFI_LOAD_FILE2_PROTOCOL_GUID;
/* GUID of the SMBIOS table */
const efi_guid_t smbios_guid = SMBIOS_TABLE_GUID;

efi_status_t EFIAPI efi_disconnect_controller(
					efi_handle_t controller_handle,
					efi_handle_t driver_image_handle,
					efi_handle_t child_handle);

efi_status_t EFIAPI efi_connect_controller(efi_handle_t controller_handle,
					   efi_handle_t *driver_image_handle,
					   struct efi_device_path *remain_device_path,
					   bool recursive);

/* Called on every callback entry */
int __efi_entry_check(void)
{
	int ret = entry_count++ == 0;
#if defined(CONFIG_ARM) || defined(CONFIG_RISCV)
	assert(efi_gd);
	app_gd = gd;
	set_gd(efi_gd);
#endif
	return ret;
}

/* Called on every callback exit */
int __efi_exit_check(void)
{
	int ret = --entry_count == 0;
#if defined(CONFIG_ARM) || defined(CONFIG_RISCV)
	set_gd(app_gd);
#endif
	return ret;
}

/**
 * efi_save_gd() - save global data register
 *
 * On the ARM and RISC-V architectures gd is mapped to a fixed register.
 * As this register may be overwritten by an EFI payload we save it here
 * and restore it on every callback entered.
 *
 * This function is called after relocation from initr_reloc_global_data().
 */
void efi_save_gd(void)
{
#if defined(CONFIG_ARM) || defined(CONFIG_RISCV)
	efi_gd = gd;
#endif
}

/**
 * efi_restore_gd() - restore global data register
 *
 * On the ARM and RISC-V architectures gd is mapped to a fixed register.
 * Restore it after returning from the UEFI world to the value saved via
 * efi_save_gd().
 */
void efi_restore_gd(void)
{
#if defined(CONFIG_ARM) || defined(CONFIG_RISCV)
	/* Only restore if we're already in EFI context */
	if (!efi_gd)
		return;
	set_gd(efi_gd);
#endif
}

/**
 * indent_string() - returns a string for indenting with two spaces per level
 * @level: indent level
 *
 * A maximum of ten indent levels is supported. Higher indent levels will be
 * truncated.
 *
 * Return: A string for indenting with two spaces per level is
 *         returned.
 */
static const char *indent_string(int level)
{
	const char *indent = "                    ";
	const int max = strlen(indent);

	level = min(max, level * 2);
	return &indent[max - level];
}

const char *__efi_nesting(void)
{
	return indent_string(nesting_level);
}

const char *__efi_nesting_inc(void)
{
	return indent_string(nesting_level++);
}

const char *__efi_nesting_dec(void)
{
	return indent_string(--nesting_level);
}

/**
 * efi_event_is_queued() - check if an event is queued
 *
 * @event:	event
 * Return:	true if event is queued
 */
static bool efi_event_is_queued(struct efi_event *event)
{
	return !!event->queue_link.next;
}

/**
 * efi_purge_handle() - Clean the deleted handle from the various lists
 * @handle: handle to remove
 *
 * Return: status code
 */
static efi_status_t efi_purge_handle(efi_handle_t handle)
{
	struct efi_register_notify_event *item;

	if (!list_empty(&handle->protocols))
		return EFI_ACCESS_DENIED;
	/* The handle is about to be freed. Remove it from events */
	list_for_each_entry(item, &efi_register_notify_events, link) {
		struct efi_protocol_notification *hitem, *hnext;

		list_for_each_entry_safe(hitem, hnext, &item->handles, link) {
			if (handle == hitem->handle) {
				list_del(&hitem->link);
				free(hitem);
			}
		}
	}
	/* The last protocol has been removed, delete the handle. */
	list_del(&handle->link);
	free(handle);

	return EFI_SUCCESS;
}

/**
 * efi_process_event_queue() - process event queue
 */
static void efi_process_event_queue(void)
{
	while (!list_empty(&efi_event_queue)) {
		struct efi_event *event;
		efi_uintn_t old_tpl;

		event = list_first_entry(&efi_event_queue, struct efi_event,
					 queue_link);
		if (efi_tpl >= event->notify_tpl)
			return;
		list_del(&event->queue_link);
		event->queue_link.next = NULL;
		event->queue_link.prev = NULL;
		/* Events must be executed at the event's TPL */
		old_tpl = efi_tpl;
		efi_tpl = event->notify_tpl;
		EFI_CALL_VOID(event->notify_function(event,
						     event->notify_context));
		efi_tpl = old_tpl;
		if (event->type == EVT_NOTIFY_SIGNAL)
			event->is_signaled = 0;
	}
}

/**
 * efi_queue_event() - queue an EFI event
 * @event:     event to signal
 *
 * This function queues the notification function of the event for future
 * execution.
 *
 */
static void efi_queue_event(struct efi_event *event)
{
	struct efi_event *item;

	if (!event->notify_function)
		return;

	if (!efi_event_is_queued(event)) {
		/*
		 * Events must be notified in order of decreasing task priority
		 * level. Insert the new event accordingly.
		 */
		list_for_each_entry(item, &efi_event_queue, queue_link) {
			if (item->notify_tpl < event->notify_tpl) {
				list_add_tail(&event->queue_link,
					      &item->queue_link);
				event = NULL;
				break;
			}
		}
		if (event)
			list_add_tail(&event->queue_link, &efi_event_queue);
		efi_process_event_queue();
	}
}

/**
 * is_valid_tpl() - check if the task priority level is valid
 *
 * @tpl:		TPL level to check
 * Return:		status code
 */
static efi_status_t is_valid_tpl(efi_uintn_t tpl)
{
	switch (tpl) {
	case TPL_APPLICATION:
	case TPL_CALLBACK:
	case TPL_NOTIFY:
		return EFI_SUCCESS;
	default:
		return EFI_INVALID_PARAMETER;
	}
}

/**
 * efi_signal_event() - signal an EFI event
 * @event:     event to signal
 *
 * This function signals an event. If the event belongs to an event group, all
 * events of the group are signaled. If they are of type EVT_NOTIFY_SIGNAL,
 * their notification function is queued.
 *
 * For the SignalEvent service see efi_signal_event_ext.
 */
void efi_signal_event(struct efi_event *event)
{
	if (event->is_signaled)
		return;
	if (event->group) {
		struct efi_event *evt;

		/*
		 * The signaled state has to set before executing any
		 * notification function
		 */
		list_for_each_entry(evt, &efi_events, link) {
			if (!evt->group || guidcmp(evt->group, event->group))
				continue;
			if (evt->is_signaled)
				continue;
			evt->is_signaled = true;
		}
		list_for_each_entry(evt, &efi_events, link) {
			if (!evt->group || guidcmp(evt->group, event->group))
				continue;
			efi_queue_event(evt);
		}
	} else {
		event->is_signaled = true;
		efi_queue_event(event);
	}
}

/**
 * efi_raise_tpl() - raise the task priority level
 * @new_tpl: new value of the task priority level
 *
 * This function implements the RaiseTpl service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: old value of the task priority level
 */
static unsigned long EFIAPI efi_raise_tpl(efi_uintn_t new_tpl)
{
	efi_uintn_t old_tpl = efi_tpl;

	EFI_ENTRY("0x%zx", new_tpl);

	if (new_tpl < efi_tpl)
		EFI_PRINT("WARNING: new_tpl < current_tpl in %s\n", __func__);
	efi_tpl = new_tpl;
	if (efi_tpl > TPL_HIGH_LEVEL)
		efi_tpl = TPL_HIGH_LEVEL;

	EFI_EXIT(EFI_SUCCESS);
	return old_tpl;
}

/**
 * efi_restore_tpl() - lower the task priority level
 * @old_tpl: value of the task priority level to be restored
 *
 * This function implements the RestoreTpl service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 */
static void EFIAPI efi_restore_tpl(efi_uintn_t old_tpl)
{
	EFI_ENTRY("0x%zx", old_tpl);

	if (old_tpl > efi_tpl)
		EFI_PRINT("WARNING: old_tpl > current_tpl in %s\n", __func__);
	efi_tpl = old_tpl;
	if (efi_tpl > TPL_HIGH_LEVEL)
		efi_tpl = TPL_HIGH_LEVEL;

	/*
	 * Lowering the TPL may have made queued events eligible for execution.
	 */
	efi_timer_check();

	EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_allocate_pages_ext() - allocate memory pages
 * @type:        type of allocation to be performed
 * @memory_type: usage type of the allocated memory
 * @pages:       number of pages to be allocated
 * @memory:      allocated memory
 *
 * This function implements the AllocatePages service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_allocate_pages_ext(int type, int memory_type,
						  efi_uintn_t pages,
						  uint64_t *memory)
{
	efi_status_t r;

	EFI_ENTRY("%d, %d, 0x%zx, %p", type, memory_type, pages, memory);
	r = efi_allocate_pages(type, memory_type, pages, memory);
	return EFI_EXIT(r);
}

/**
 * efi_free_pages_ext() - Free memory pages.
 * @memory: start of the memory area to be freed
 * @pages:  number of pages to be freed
 *
 * This function implements the FreePages service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_free_pages_ext(uint64_t memory,
					      efi_uintn_t pages)
{
	efi_status_t r;

	EFI_ENTRY("%llx, 0x%zx", memory, pages);
	r = efi_free_pages(memory, pages);
	return EFI_EXIT(r);
}

/**
 * efi_get_memory_map_ext() - get map describing memory usage
 * @memory_map_size:    on entry the size, in bytes, of the memory map buffer,
 *                      on exit the size of the copied memory map
 * @memory_map:         buffer to which the memory map is written
 * @map_key:            key for the memory map
 * @descriptor_size:    size of an individual memory descriptor
 * @descriptor_version: version number of the memory descriptor structure
 *
 * This function implements the GetMemoryMap service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_get_memory_map_ext(
					efi_uintn_t *memory_map_size,
					struct efi_mem_desc *memory_map,
					efi_uintn_t *map_key,
					efi_uintn_t *descriptor_size,
					uint32_t *descriptor_version)
{
	efi_status_t r;

	EFI_ENTRY("%p, %p, %p, %p, %p", memory_map_size, memory_map,
		  map_key, descriptor_size, descriptor_version);
	r = efi_get_memory_map(memory_map_size, memory_map, map_key,
			       descriptor_size, descriptor_version);
	return EFI_EXIT(r);
}

/**
 * efi_allocate_pool_ext() - allocate memory from pool
 * @pool_type: type of the pool from which memory is to be allocated
 * @size:      number of bytes to be allocated
 * @buffer:    allocated memory
 *
 * This function implements the AllocatePool service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_allocate_pool_ext(int pool_type,
						 efi_uintn_t size,
						 void **buffer)
{
	efi_status_t r;

	EFI_ENTRY("%d, %zu, %p", pool_type, size, buffer);
	r = efi_allocate_pool(pool_type, size, buffer);
	return EFI_EXIT(r);
}

/**
 * efi_free_pool_ext() - free memory from pool
 * @buffer: start of memory to be freed
 *
 * This function implements the FreePool service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_free_pool_ext(void *buffer)
{
	efi_status_t r;

	EFI_ENTRY("%p", buffer);
	r = efi_free_pool(buffer);
	return EFI_EXIT(r);
}

/**
 * efi_add_handle() - add a new handle to the object list
 *
 * @handle:	handle to be added
 *
 * The protocols list is initialized. The handle is added to the list of known
 * UEFI objects.
 */
void efi_add_handle(efi_handle_t handle)
{
	if (!handle)
		return;
	INIT_LIST_HEAD(&handle->protocols);
	list_add_tail(&handle->link, &efi_obj_list);
}

/**
 * efi_create_handle() - create handle
 * @handle: new handle
 *
 * Return: status code
 */
efi_status_t efi_create_handle(efi_handle_t *handle)
{
	struct efi_object *obj;

	obj = calloc(1, sizeof(struct efi_object));
	if (!obj)
		return EFI_OUT_OF_RESOURCES;

	efi_add_handle(obj);
	*handle = obj;

	return EFI_SUCCESS;
}

/**
 * efi_search_protocol() - find a protocol on a handle.
 * @handle:        handle
 * @protocol_guid: GUID of the protocol
 * @handler:       reference to the protocol
 *
 * Return: status code
 */
efi_status_t efi_search_protocol(const efi_handle_t handle,
				 const efi_guid_t *protocol_guid,
				 struct efi_handler **handler)
{
	struct efi_object *efiobj;
	struct list_head *lhandle;

	if (!handle || !protocol_guid)
		return EFI_INVALID_PARAMETER;
	efiobj = efi_search_obj(handle);
	if (!efiobj)
		return EFI_INVALID_PARAMETER;
	list_for_each(lhandle, &efiobj->protocols) {
		struct efi_handler *protocol;

		protocol = list_entry(lhandle, struct efi_handler, link);
		if (!guidcmp(&protocol->guid, protocol_guid)) {
			if (handler)
				*handler = protocol;
			return EFI_SUCCESS;
		}
	}
	return EFI_NOT_FOUND;
}

/**
 * efi_remove_protocol() - delete protocol from a handle
 * @handle:             handle from which the protocol shall be deleted
 * @protocol:           GUID of the protocol to be deleted
 * @protocol_interface: interface of the protocol implementation
 *
 * Return: status code
 */
static efi_status_t efi_remove_protocol(const efi_handle_t handle,
					const efi_guid_t *protocol,
					void *protocol_interface)
{
	struct efi_handler *handler;
	efi_status_t ret;

	ret = efi_search_protocol(handle, protocol, &handler);
	if (ret != EFI_SUCCESS)
		return ret;
	if (handler->protocol_interface != protocol_interface)
		return EFI_NOT_FOUND;
	list_del(&handler->link);
	free(handler);
	return EFI_SUCCESS;
}

/**
 * efi_remove_all_protocols() - delete all protocols from a handle
 * @handle: handle from which the protocols shall be deleted
 *
 * Return: status code
 */
static efi_status_t efi_remove_all_protocols(const efi_handle_t handle)
{
	struct efi_object *efiobj;
	struct efi_handler *protocol;
	struct efi_handler *pos;

	efiobj = efi_search_obj(handle);
	if (!efiobj)
		return EFI_INVALID_PARAMETER;
	list_for_each_entry_safe(protocol, pos, &efiobj->protocols, link) {
		efi_status_t ret;

		ret = efi_uninstall_protocol(handle, &protocol->guid,
					     protocol->protocol_interface, true);
		if (ret != EFI_SUCCESS)
			return ret;
	}
	return EFI_SUCCESS;
}

/**
 * efi_delete_handle() - delete handle
 *
 * @handle: handle to delete
 *
 * Return: status code
 */
efi_status_t efi_delete_handle(efi_handle_t handle)
{
	efi_status_t ret;

	ret = efi_remove_all_protocols(handle);
	if (ret != EFI_SUCCESS) {
		log_err("Handle %p has protocols installed. Unable to delete\n", handle);
		return ret;
	}

	return efi_purge_handle(handle);
}

/**
 * efi_is_event() - check if a pointer is a valid event
 * @event: pointer to check
 *
 * Return: status code
 */
static efi_status_t efi_is_event(const struct efi_event *event)
{
	const struct efi_event *evt;

	if (!event)
		return EFI_INVALID_PARAMETER;
	list_for_each_entry(evt, &efi_events, link) {
		if (evt == event)
			return EFI_SUCCESS;
	}
	return EFI_INVALID_PARAMETER;
}

/**
 * efi_create_event() - create an event
 *
 * @type:            type of the event to create
 * @notify_tpl:      task priority level of the event
 * @notify_function: notification function of the event
 * @notify_context:  pointer passed to the notification function
 * @group:           event group
 * @event:           created event
 *
 * This function is used inside U-Boot code to create an event.
 *
 * For the API function implementing the CreateEvent service see
 * efi_create_event_ext.
 *
 * Return: status code
 */
efi_status_t efi_create_event(uint32_t type, efi_uintn_t notify_tpl,
			      void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
			      void *notify_context, const efi_guid_t *group,
			      struct efi_event **event)
{
	struct efi_event *evt;
	efi_status_t ret;
	int pool_type;

	if (event == NULL)
		return EFI_INVALID_PARAMETER;

	switch (type) {
	case 0:
	case EVT_TIMER:
	case EVT_NOTIFY_SIGNAL:
	case EVT_TIMER | EVT_NOTIFY_SIGNAL:
	case EVT_NOTIFY_WAIT:
	case EVT_TIMER | EVT_NOTIFY_WAIT:
	case EVT_SIGNAL_EXIT_BOOT_SERVICES:
		pool_type = EFI_BOOT_SERVICES_DATA;
		break;
	case EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE:
		pool_type = EFI_RUNTIME_SERVICES_DATA;
		break;
	default:
		return EFI_INVALID_PARAMETER;
	}

	/*
	 * The UEFI specification requires event notification levels to be
	 * > TPL_APPLICATION and <= TPL_HIGH_LEVEL.
	 *
	 * Parameter NotifyTpl should not be checked if it is not used.
	 */
	if ((type & (EVT_NOTIFY_WAIT | EVT_NOTIFY_SIGNAL)) &&
	    (!notify_function || is_valid_tpl(notify_tpl) != EFI_SUCCESS ||
	     notify_tpl == TPL_APPLICATION))
		return EFI_INVALID_PARAMETER;

	ret = efi_allocate_pool(pool_type, sizeof(struct efi_event),
				(void **)&evt);
	if (ret != EFI_SUCCESS)
		return ret;
	memset(evt, 0, sizeof(struct efi_event));
	evt->type = type;
	evt->notify_tpl = notify_tpl;
	evt->notify_function = notify_function;
	evt->notify_context = notify_context;
	evt->group = group;
	/* Disable timers on boot up */
	evt->trigger_next = -1ULL;
	list_add_tail(&evt->link, &efi_events);
	*event = evt;
	return EFI_SUCCESS;
}

/*
 * efi_create_event_ex() - create an event in a group
 *
 * @type:            type of the event to create
 * @notify_tpl:      task priority level of the event
 * @notify_function: notification function of the event
 * @notify_context:  pointer passed to the notification function
 * @event:           created event
 * @event_group:     event group
 *
 * This function implements the CreateEventEx service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static
efi_status_t EFIAPI efi_create_event_ex(uint32_t type, efi_uintn_t notify_tpl,
					void (EFIAPI *notify_function) (
							struct efi_event *event,
							void *context),
					void *notify_context,
					const efi_guid_t *event_group,
					struct efi_event **event)
{
	efi_status_t ret;

	EFI_ENTRY("%d, 0x%zx, %p, %p, %pUs", type, notify_tpl, notify_function,
		  notify_context, event_group);

	/*
	 * The allowable input parameters are the same as in CreateEvent()
	 * except for the following two disallowed event types.
	 */
	switch (type) {
	case EVT_SIGNAL_EXIT_BOOT_SERVICES:
	case EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE:
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = efi_create_event(type, notify_tpl, notify_function,
			       notify_context, event_group, event);
out:
	return EFI_EXIT(ret);
}

/**
 * efi_create_event_ext() - create an event
 * @type:            type of the event to create
 * @notify_tpl:      task priority level of the event
 * @notify_function: notification function of the event
 * @notify_context:  pointer passed to the notification function
 * @event:           created event
 *
 * This function implements the CreateEvent service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_create_event_ext(
			uint32_t type, efi_uintn_t notify_tpl,
			void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
			void *notify_context, struct efi_event **event)
{
	EFI_ENTRY("%d, 0x%zx, %p, %p", type, notify_tpl, notify_function,
		  notify_context);
	return EFI_EXIT(efi_create_event(type, notify_tpl, notify_function,
					 notify_context, NULL, event));
}

/**
 * efi_timer_check() - check if a timer event has occurred
 *
 * Check if a timer event has occurred or a queued notification function should
 * be called.
 *
 * Our timers have to work without interrupts, so we check whenever keyboard
 * input or disk accesses happen if enough time elapsed for them to fire.
 */
void efi_timer_check(void)
{
	struct efi_event *evt;
	u64 now = timer_get_us();

	list_for_each_entry(evt, &efi_events, link) {
		if (!timers_enabled)
			continue;
		if (!(evt->type & EVT_TIMER) || now < evt->trigger_next)
			continue;
		switch (evt->trigger_type) {
		case EFI_TIMER_RELATIVE:
			evt->trigger_type = EFI_TIMER_STOP;
			break;
		case EFI_TIMER_PERIODIC:
			evt->trigger_next += evt->trigger_time;
			break;
		default:
			continue;
		}
		evt->is_signaled = false;
		efi_signal_event(evt);
	}
	efi_process_event_queue();
	schedule();
}

/**
 * efi_set_timer() - set the trigger time for a timer event or stop the event
 * @event:        event for which the timer is set
 * @type:         type of the timer
 * @trigger_time: trigger period in multiples of 100 ns
 *
 * This is the function for internal usage in U-Boot. For the API function
 * implementing the SetTimer service see efi_set_timer_ext.
 *
 * Return: status code
 */
efi_status_t efi_set_timer(struct efi_event *event, enum efi_timer_delay type,
			   uint64_t trigger_time)
{
	/* Check that the event is valid */
	if (efi_is_event(event) != EFI_SUCCESS || !(event->type & EVT_TIMER))
		return EFI_INVALID_PARAMETER;

	/*
	 * The parameter defines a multiple of 100 ns.
	 * We use multiples of 1000 ns. So divide by 10.
	 */
	do_div(trigger_time, 10);

	switch (type) {
	case EFI_TIMER_STOP:
		event->trigger_next = -1ULL;
		break;
	case EFI_TIMER_PERIODIC:
	case EFI_TIMER_RELATIVE:
		event->trigger_next = timer_get_us() + trigger_time;
		break;
	default:
		return EFI_INVALID_PARAMETER;
	}
	event->trigger_type = type;
	event->trigger_time = trigger_time;
	event->is_signaled = false;
	return EFI_SUCCESS;
}

/**
 * efi_set_timer_ext() - Set the trigger time for a timer event or stop the
 *                       event
 * @event:        event for which the timer is set
 * @type:         type of the timer
 * @trigger_time: trigger period in multiples of 100 ns
 *
 * This function implements the SetTimer service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_set_timer_ext(struct efi_event *event,
					     enum efi_timer_delay type,
					     uint64_t trigger_time)
{
	EFI_ENTRY("%p, %d, %llx", event, type, trigger_time);
	return EFI_EXIT(efi_set_timer(event, type, trigger_time));
}

/**
 * efi_wait_for_event() - wait for events to be signaled
 * @num_events: number of events to be waited for
 * @event:      events to be waited for
 * @index:      index of the event that was signaled
 *
 * This function implements the WaitForEvent service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_wait_for_event(efi_uintn_t num_events,
					      struct efi_event **event,
					      efi_uintn_t *index)
{
	int i;

	EFI_ENTRY("%zu, %p, %p", num_events, event, index);

	/* Check parameters */
	if (!num_events || !event)
		return EFI_EXIT(EFI_INVALID_PARAMETER);
	/* Check TPL */
	if (efi_tpl != TPL_APPLICATION)
		return EFI_EXIT(EFI_UNSUPPORTED);
	for (i = 0; i < num_events; ++i) {
		if (efi_is_event(event[i]) != EFI_SUCCESS)
			return EFI_EXIT(EFI_INVALID_PARAMETER);
		if (!event[i]->type || event[i]->type & EVT_NOTIFY_SIGNAL)
			return EFI_EXIT(EFI_INVALID_PARAMETER);
		if (!event[i]->is_signaled)
			efi_queue_event(event[i]);
	}

	/* Wait for signal */
	for (;;) {
		for (i = 0; i < num_events; ++i) {
			if (event[i]->is_signaled)
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
	event[i]->is_signaled = false;
	if (index)
		*index = i;

	return EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_signal_event_ext() - signal an EFI event
 * @event: event to signal
 *
 * This function implements the SignalEvent service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * This functions sets the signaled state of the event and queues the
 * notification function for execution.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_signal_event_ext(struct efi_event *event)
{
	EFI_ENTRY("%p", event);
	if (efi_is_event(event) != EFI_SUCCESS)
		return EFI_EXIT(EFI_INVALID_PARAMETER);
	efi_signal_event(event);
	return EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_close_event() - close an EFI event
 * @event: event to close
 *
 * This function implements the CloseEvent service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_close_event(struct efi_event *event)
{
	struct efi_register_notify_event *item, *next;

	EFI_ENTRY("%p", event);
	if (efi_is_event(event) != EFI_SUCCESS)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	/* Remove protocol notify registrations for the event */
	list_for_each_entry_safe(item, next, &efi_register_notify_events,
				 link) {
		if (event == item->event) {
			struct efi_protocol_notification *hitem, *hnext;

			/* Remove signaled handles */
			list_for_each_entry_safe(hitem, hnext, &item->handles,
						 link) {
				list_del(&hitem->link);
				free(hitem);
			}
			list_del(&item->link);
			free(item);
		}
	}
	/* Remove event from queue */
	if (efi_event_is_queued(event))
		list_del(&event->queue_link);

	list_del(&event->link);
	efi_free_pool(event);
	return EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_check_event() - check if an event is signaled
 * @event: event to check
 *
 * This function implements the CheckEvent service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * If an event is not signaled yet, the notification function is queued. The
 * signaled state is cleared.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_check_event(struct efi_event *event)
{
	EFI_ENTRY("%p", event);
	efi_timer_check();
	if (efi_is_event(event) != EFI_SUCCESS ||
	    event->type & EVT_NOTIFY_SIGNAL)
		return EFI_EXIT(EFI_INVALID_PARAMETER);
	if (!event->is_signaled)
		efi_queue_event(event);
	if (event->is_signaled) {
		event->is_signaled = false;
		return EFI_EXIT(EFI_SUCCESS);
	}
	return EFI_EXIT(EFI_NOT_READY);
}

/**
 * efi_search_obj() - find the internal EFI object for a handle
 * @handle: handle to find
 *
 * Return: EFI object
 */
struct efi_object *efi_search_obj(const efi_handle_t handle)
{
	struct efi_object *efiobj;

	if (!handle)
		return NULL;

	list_for_each_entry(efiobj, &efi_obj_list, link) {
		if (efiobj == handle)
			return efiobj;
	}
	return NULL;
}

/**
 * efi_open_protocol_info_entry() - create open protocol info entry and add it
 *                                  to a protocol
 * @handler: handler of a protocol
 *
 * Return: open protocol info entry
 */
static struct efi_open_protocol_info_entry *efi_create_open_info(
			struct efi_handler *handler)
{
	struct efi_open_protocol_info_item *item;

	item = calloc(1, sizeof(struct efi_open_protocol_info_item));
	if (!item)
		return NULL;
	/* Append the item to the open protocol info list. */
	list_add_tail(&item->link, &handler->open_infos);

	return &item->info;
}

/**
 * efi_delete_open_info() - remove an open protocol info entry from a protocol
 * @item: open protocol info entry to delete
 *
 * Return: status code
 */
static efi_status_t efi_delete_open_info(
			struct efi_open_protocol_info_item *item)
{
	list_del(&item->link);
	free(item);
	return EFI_SUCCESS;
}

/**
 * efi_add_protocol() - install new protocol on a handle
 * @handle:             handle on which the protocol shall be installed
 * @protocol:           GUID of the protocol to be installed
 * @protocol_interface: interface of the protocol implementation
 *
 * Return: status code
 */
efi_status_t efi_add_protocol(const efi_handle_t handle,
			      const efi_guid_t *protocol,
			      void *protocol_interface)
{
	struct efi_object *efiobj;
	struct efi_handler *handler;
	efi_status_t ret;
	struct efi_register_notify_event *event;

	efiobj = efi_search_obj(handle);
	if (!efiobj)
		return EFI_INVALID_PARAMETER;
	ret = efi_search_protocol(handle, protocol, NULL);
	if (ret != EFI_NOT_FOUND)
		return EFI_INVALID_PARAMETER;
	handler = calloc(1, sizeof(struct efi_handler));
	if (!handler)
		return EFI_OUT_OF_RESOURCES;
	memcpy((void *)&handler->guid, protocol, sizeof(efi_guid_t));
	handler->protocol_interface = protocol_interface;
	INIT_LIST_HEAD(&handler->open_infos);
	list_add_tail(&handler->link, &efiobj->protocols);

	/* Notify registered events */
	list_for_each_entry(event, &efi_register_notify_events, link) {
		if (!guidcmp(protocol, &event->protocol)) {
			struct efi_protocol_notification *notif;

			notif = calloc(1, sizeof(*notif));
			if (!notif) {
				list_del(&handler->link);
				free(handler);
				return EFI_OUT_OF_RESOURCES;
			}
			notif->handle = handle;
			list_add_tail(&notif->link, &event->handles);
			event->event->is_signaled = false;
			efi_signal_event(event->event);
		}
	}

	if (!guidcmp(&efi_guid_device_path, protocol))
		EFI_PRINT("installed device path '%pD'\n", protocol_interface);
	return EFI_SUCCESS;
}

/**
 * efi_install_protocol_interface() - install protocol interface
 * @handle:                  handle on which the protocol shall be installed
 * @protocol:                GUID of the protocol to be installed
 * @protocol_interface_type: type of the interface to be installed,
 *                           always EFI_NATIVE_INTERFACE
 * @protocol_interface:      interface of the protocol implementation
 *
 * This function implements the InstallProtocolInterface service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_install_protocol_interface(
			efi_handle_t *handle, const efi_guid_t *protocol,
			int protocol_interface_type, void *protocol_interface)
{
	efi_status_t r;

	EFI_ENTRY("%p, %pUs, %d, %p", handle, protocol, protocol_interface_type,
		  protocol_interface);

	if (!handle || !protocol ||
	    protocol_interface_type != EFI_NATIVE_INTERFACE) {
		r = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Create new handle if requested. */
	if (!*handle) {
		r = efi_create_handle(handle);
		if (r != EFI_SUCCESS)
			goto out;
		EFI_PRINT("new handle %p\n", *handle);
	} else {
		EFI_PRINT("handle %p\n", *handle);
	}
	/* Add new protocol */
	r = efi_add_protocol(*handle, protocol, protocol_interface);
out:
	return EFI_EXIT(r);
}

/**
 * efi_get_drivers() - get all drivers associated to a controller
 * @handle:               handle of the controller
 * @protocol:             protocol GUID (optional)
 * @number_of_drivers:    number of child controllers
 * @driver_handle_buffer: handles of the the drivers
 *
 * The allocated buffer has to be freed with free().
 *
 * Return: status code
 */
static efi_status_t efi_get_drivers(efi_handle_t handle,
				    const efi_guid_t *protocol,
				    efi_uintn_t *number_of_drivers,
				    efi_handle_t **driver_handle_buffer)
{
	struct efi_handler *handler;
	struct efi_open_protocol_info_item *item;
	efi_uintn_t count = 0, i;
	bool duplicate;

	/* Count all driver associations */
	list_for_each_entry(handler, &handle->protocols, link) {
		if (protocol && guidcmp(&handler->guid, protocol))
			continue;
		list_for_each_entry(item, &handler->open_infos, link) {
			if (item->info.attributes &
			    EFI_OPEN_PROTOCOL_BY_DRIVER)
				++count;
		}
	}
	*number_of_drivers = 0;
	if (!count) {
		*driver_handle_buffer = NULL;
		return EFI_SUCCESS;
	}
	/*
	 * Create buffer. In case of duplicate driver assignments the buffer
	 * will be too large. But that does not harm.
	 */
	*driver_handle_buffer = calloc(count, sizeof(efi_handle_t));
	if (!*driver_handle_buffer)
		return EFI_OUT_OF_RESOURCES;
	/* Collect unique driver handles */
	list_for_each_entry(handler, &handle->protocols, link) {
		if (protocol && guidcmp(&handler->guid, protocol))
			continue;
		list_for_each_entry(item, &handler->open_infos, link) {
			if (item->info.attributes &
			    EFI_OPEN_PROTOCOL_BY_DRIVER) {
				/* Check this is a new driver */
				duplicate = false;
				for (i = 0; i < *number_of_drivers; ++i) {
					if ((*driver_handle_buffer)[i] ==
					    item->info.agent_handle)
						duplicate = true;
				}
				/* Copy handle to buffer */
				if (!duplicate) {
					i = (*number_of_drivers)++;
					(*driver_handle_buffer)[i] =
						item->info.agent_handle;
				}
			}
		}
	}
	return EFI_SUCCESS;
}

/**
 * efi_disconnect_all_drivers() - disconnect all drivers from a controller
 * @handle:       handle of the controller
 * @protocol:     protocol GUID (optional)
 * @child_handle: handle of the child to destroy
 *
 * This function implements the DisconnectController service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t efi_disconnect_all_drivers
				(efi_handle_t handle,
				 const efi_guid_t *protocol,
				 efi_handle_t child_handle)
{
	efi_uintn_t number_of_drivers;
	efi_handle_t *driver_handle_buffer;
	efi_status_t r, ret;

	ret = efi_get_drivers(handle, protocol, &number_of_drivers,
			      &driver_handle_buffer);
	if (ret != EFI_SUCCESS)
		return ret;
	if (!number_of_drivers)
		return EFI_SUCCESS;

	while (number_of_drivers) {
		r = EFI_CALL(efi_disconnect_controller(
				handle,
				driver_handle_buffer[--number_of_drivers],
				child_handle));
		if (r != EFI_SUCCESS)
			ret = r;
	}

	free(driver_handle_buffer);
	return ret;
}

/**
 * efi_uninstall_protocol() - uninstall protocol interface
 *
 * @handle:             handle from which the protocol shall be removed
 * @protocol:           GUID of the protocol to be removed
 * @protocol_interface: interface to be removed
 * @preserve:		preserve or delete the handle and remove it from any
 *			list it participates if no protocols remain
 *
 * This function DOES NOT delete a handle without installed protocol.
 *
 * Return: status code
 */
efi_status_t efi_uninstall_protocol
		(efi_handle_t handle, const efi_guid_t *protocol,
			void *protocol_interface, bool preserve)
{
	struct efi_handler *handler;
	struct efi_open_protocol_info_item *item;
	struct efi_open_protocol_info_item *pos;
	efi_status_t r;

	/* Find the protocol on the handle */
	r = efi_search_protocol(handle, protocol, &handler);
	if (r != EFI_SUCCESS)
		goto out;
	if (handler->protocol_interface != protocol_interface)
		return EFI_NOT_FOUND;
	/* Disconnect controllers */
	r = efi_disconnect_all_drivers(handle, protocol, NULL);
	if (r != EFI_SUCCESS) {
		r = EFI_ACCESS_DENIED;
		/*
		 * This will reconnect all controllers of the handle, even ones
		 * that were not connected before. This can be done better
		 * but we are following the EDKII implementation on this for
		 * now
		 */
		EFI_CALL(efi_connect_controller(handle, NULL, NULL, true));
		goto out;
	}
	/* Close protocol */
	list_for_each_entry_safe(item, pos, &handler->open_infos, link) {
		if (item->info.attributes ==
			EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL ||
		    item->info.attributes == EFI_OPEN_PROTOCOL_GET_PROTOCOL ||
		    item->info.attributes == EFI_OPEN_PROTOCOL_TEST_PROTOCOL)
			efi_delete_open_info(item);
	}
	/* if agents didn't close the protocols properly */
	if (!list_empty(&handler->open_infos)) {
		r =  EFI_ACCESS_DENIED;
		EFI_CALL(efi_connect_controller(handle, NULL, NULL, true));
		goto out;
	}
	r = efi_remove_protocol(handle, protocol, protocol_interface);
	if (r != EFI_SUCCESS)
		return r;
	/*
	 * We don't care about the return value here since the
	 * handle might have more protocols installed
	 */
	if (!preserve)
		efi_purge_handle(handle);
out:
	return r;
}

/**
 * efi_uninstall_protocol_interface() - uninstall protocol interface
 * @handle:             handle from which the protocol shall be removed
 * @protocol:           GUID of the protocol to be removed
 * @protocol_interface: interface to be removed
 *
 * This function implements the UninstallProtocolInterface service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_uninstall_protocol_interface
			(efi_handle_t handle, const efi_guid_t *protocol,
			 void *protocol_interface)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %pUs, %p", handle, protocol, protocol_interface);

	ret = efi_uninstall_protocol(handle, protocol, protocol_interface, false);
	if (ret != EFI_SUCCESS)
		goto out;

out:
	return EFI_EXIT(ret);
}

/**
 * efi_register_protocol_notify() - register an event for notification when a
 *                                  protocol is installed.
 * @protocol:     GUID of the protocol whose installation shall be notified
 * @event:        event to be signaled upon installation of the protocol
 * @registration: key for retrieving the registration information
 *
 * This function implements the RegisterProtocolNotify service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_register_protocol_notify(const efi_guid_t *protocol,
						 struct efi_event *event,
						 void **registration)
{
	struct efi_register_notify_event *item;
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%pUs, %p, %p", protocol, event, registration);

	if (!protocol || !event || !registration) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	item = calloc(1, sizeof(struct efi_register_notify_event));
	if (!item) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

	item->event = event;
	guidcpy(&item->protocol, protocol);
	INIT_LIST_HEAD(&item->handles);

	list_add_tail(&item->link, &efi_register_notify_events);

	*registration = item;
out:
	return EFI_EXIT(ret);
}

/**
 * efi_search() - determine if an EFI handle implements a protocol
 *
 * @search_type: selection criterion
 * @protocol:    GUID of the protocol
 * @handle:      handle
 *
 * See the documentation of the LocateHandle service in the UEFI specification.
 *
 * Return: 0 if the handle implements the protocol
 */
static int efi_search(enum efi_locate_search_type search_type,
		      const efi_guid_t *protocol, efi_handle_t handle)
{
	efi_status_t ret;

	switch (search_type) {
	case ALL_HANDLES:
		return 0;
	case BY_PROTOCOL:
		ret = efi_search_protocol(handle, protocol, NULL);
		return (ret != EFI_SUCCESS);
	default:
		/* Invalid search type */
		return -1;
	}
}

/**
 * efi_check_register_notify_event() - check if registration key is valid
 *
 * Check that a pointer is a valid registration key as returned by
 * RegisterProtocolNotify().
 *
 * @key:	registration key
 * Return:	valid registration key or NULL
 */
static struct efi_register_notify_event *efi_check_register_notify_event
								(void *key)
{
	struct efi_register_notify_event *event;

	list_for_each_entry(event, &efi_register_notify_events, link) {
		if (event == (struct efi_register_notify_event *)key)
			return event;
	}
	return NULL;
}

/**
 * efi_locate_handle() - locate handles implementing a protocol
 *
 * @search_type:	selection criterion
 * @protocol:		GUID of the protocol
 * @search_key:		registration key
 * @buffer_size:	size of the buffer to receive the handles in bytes
 * @buffer:		buffer to receive the relevant handles
 *
 * This function is meant for U-Boot internal calls. For the API implementation
 * of the LocateHandle service see efi_locate_handle_ext.
 *
 * Return: status code
 */
static efi_status_t efi_locate_handle(
			enum efi_locate_search_type search_type,
			const efi_guid_t *protocol, void *search_key,
			efi_uintn_t *buffer_size, efi_handle_t *buffer)
{
	struct efi_object *efiobj;
	efi_uintn_t size = 0;
	struct efi_register_notify_event *event;
	struct efi_protocol_notification *handle = NULL;

	/* Check parameters */
	switch (search_type) {
	case ALL_HANDLES:
		break;
	case BY_REGISTER_NOTIFY:
		if (!search_key)
			return EFI_INVALID_PARAMETER;
		/* Check that the registration key is valid */
		event = efi_check_register_notify_event(search_key);
		if (!event)
			return EFI_INVALID_PARAMETER;
		break;
	case BY_PROTOCOL:
		if (!protocol)
			return EFI_INVALID_PARAMETER;
		break;
	default:
		return EFI_INVALID_PARAMETER;
	}

	/* Count how much space we need */
	if (search_type == BY_REGISTER_NOTIFY) {
		if (list_empty(&event->handles))
			return EFI_NOT_FOUND;
		handle = list_first_entry(&event->handles,
					  struct efi_protocol_notification,
					  link);
		efiobj = handle->handle;
		size += sizeof(void *);
	} else {
		list_for_each_entry(efiobj, &efi_obj_list, link) {
			if (!efi_search(search_type, protocol, efiobj))
				size += sizeof(void *);
		}
		if (size == 0)
			return EFI_NOT_FOUND;
	}

	if (!buffer_size)
		return EFI_INVALID_PARAMETER;

	if (*buffer_size < size) {
		*buffer_size = size;
		return EFI_BUFFER_TOO_SMALL;
	}

	*buffer_size = size;

	/* The buffer size is sufficient but there is no buffer */
	if (!buffer)
		return EFI_INVALID_PARAMETER;

	/* Then fill the array */
	if (search_type == BY_REGISTER_NOTIFY) {
		*buffer = efiobj;
		list_del(&handle->link);
	} else {
		list_for_each_entry(efiobj, &efi_obj_list, link) {
			if (!efi_search(search_type, protocol, efiobj))
				*buffer++ = efiobj;
		}
	}

	return EFI_SUCCESS;
}

/**
 * efi_locate_handle_ext() - locate handles implementing a protocol.
 * @search_type: selection criterion
 * @protocol:    GUID of the protocol
 * @search_key:  registration key
 * @buffer_size: size of the buffer to receive the handles in bytes
 * @buffer:      buffer to receive the relevant handles
 *
 * This function implements the LocateHandle service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: 0 if the handle implements the protocol
 */
static efi_status_t EFIAPI efi_locate_handle_ext(
			enum efi_locate_search_type search_type,
			const efi_guid_t *protocol, void *search_key,
			efi_uintn_t *buffer_size, efi_handle_t *buffer)
{
	EFI_ENTRY("%d, %pUs, %p, %p, %p", search_type, protocol, search_key,
		  buffer_size, buffer);

	return EFI_EXIT(efi_locate_handle(search_type, protocol, search_key,
			buffer_size, buffer));
}

/**
 * efi_remove_configuration_table() - collapses configuration table entries,
 *                                    removing index i
 *
 * @i: index of the table entry to be removed
 */
static void efi_remove_configuration_table(int i)
{
	struct efi_configuration_table *this = &systab.tables[i];
	struct efi_configuration_table *next = &systab.tables[i + 1];
	struct efi_configuration_table *end = &systab.tables[systab.nr_tables];

	memmove(this, next, (ulong)end - (ulong)next);
	systab.nr_tables--;
}

/**
 * efi_install_configuration_table() - adds, updates, or removes a
 *                                     configuration table
 * @guid:  GUID of the installed table
 * @table: table to be installed
 *
 * This function is used for internal calls. For the API implementation of the
 * InstallConfigurationTable service see efi_install_configuration_table_ext.
 *
 * Return: status code
 */
efi_status_t efi_install_configuration_table(const efi_guid_t *guid,
					     void *table)
{
	struct efi_event *evt;
	int i;

	if (!guid)
		return EFI_INVALID_PARAMETER;

	/* Check for GUID override */
	for (i = 0; i < systab.nr_tables; i++) {
		if (!guidcmp(guid, &systab.tables[i].guid)) {
			if (table)
				systab.tables[i].table = table;
			else
				efi_remove_configuration_table(i);
			goto out;
		}
	}

	if (!table)
		return EFI_NOT_FOUND;

	/* No override, check for overflow */
	if (i >= EFI_MAX_CONFIGURATION_TABLES)
		return EFI_OUT_OF_RESOURCES;

	/* Add a new entry */
	guidcpy(&systab.tables[i].guid, guid);
	systab.tables[i].table = table;
	systab.nr_tables = i + 1;

out:
	/* systab.nr_tables may have changed. So we need to update the CRC32 */
	efi_update_table_header_crc32(&systab.hdr);

	/* Notify that the configuration table was changed */
	list_for_each_entry(evt, &efi_events, link) {
		if (evt->group && !guidcmp(evt->group, guid)) {
			efi_signal_event(evt);
			break;
		}
	}

	return EFI_SUCCESS;
}

/**
 * efi_install_configuration_table_ex() - Adds, updates, or removes a
 *                                        configuration table.
 * @guid:  GUID of the installed table
 * @table: table to be installed
 *
 * This function implements the InstallConfigurationTable service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t
EFIAPI efi_install_configuration_table_ext(const efi_guid_t *guid,
					   void *table)
{
	EFI_ENTRY("%pUs, %p", guid, table);
	return EFI_EXIT(efi_install_configuration_table(guid, table));
}

/**
 * efi_setup_loaded_image() - initialize a loaded image
 *
 * Initialize a loaded_image_info and loaded_image_info object with correct
 * protocols, boot-device, etc.
 *
 * In case of an error \*handle_ptr and \*info_ptr are set to NULL and an error
 * code is returned.
 *
 * @device_path:	device path of the loaded image
 * @file_path:		file path of the loaded image
 * @handle_ptr:		handle of the loaded image
 * @info_ptr:		loaded image protocol
 * Return:		status code
 */
efi_status_t efi_setup_loaded_image(struct efi_device_path *device_path,
				    struct efi_device_path *file_path,
				    struct efi_loaded_image_obj **handle_ptr,
				    struct efi_loaded_image **info_ptr)
{
	efi_status_t ret;
	struct efi_loaded_image *info = NULL;
	struct efi_loaded_image_obj *obj = NULL;
	struct efi_device_path *dp;

	/* In case of EFI_OUT_OF_RESOURCES avoid illegal free by caller. */
	*handle_ptr = NULL;
	*info_ptr = NULL;

	info = calloc(1, sizeof(*info));
	if (!info)
		return EFI_OUT_OF_RESOURCES;
	obj = calloc(1, sizeof(*obj));
	if (!obj) {
		free(info);
		return EFI_OUT_OF_RESOURCES;
	}
	obj->header.type = EFI_OBJECT_TYPE_LOADED_IMAGE;

	/* Add internal object to object list */
	efi_add_handle(&obj->header);

	info->revision =  EFI_LOADED_IMAGE_PROTOCOL_REVISION;
	info->file_path = file_path;
	info->system_table = &systab;

	if (device_path) {
		info->device_handle = efi_dp_find_obj(device_path, NULL, NULL);

		dp = efi_dp_concat(device_path, file_path, 0);
		if (!dp) {
			ret = EFI_OUT_OF_RESOURCES;
			goto failure;
		}
	} else {
		dp = NULL;
	}
	ret = efi_add_protocol(&obj->header,
			       &efi_guid_loaded_image_device_path, dp);
	if (ret != EFI_SUCCESS)
		goto failure;

	/*
	 * When asking for the loaded_image interface, just
	 * return handle which points to loaded_image_info
	 */
	ret = efi_add_protocol(&obj->header,
			       &efi_guid_loaded_image, info);
	if (ret != EFI_SUCCESS)
		goto failure;

	*info_ptr = info;
	*handle_ptr = obj;

	return ret;
failure:
	printf("ERROR: Failure to install protocols for loaded image\n");
	efi_delete_handle(&obj->header);
	free(info);
	return ret;
}

/**
 * efi_locate_device_path() - Get the device path and handle of an device
 *                            implementing a protocol
 * @protocol:    GUID of the protocol
 * @device_path: device path
 * @device:      handle of the device
 *
 * This function implements the LocateDevicePath service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_locate_device_path(const efi_guid_t *protocol,
					   struct efi_device_path **device_path,
					   efi_handle_t *device)
{
	struct efi_device_path *dp;
	size_t i;
	struct efi_handler *handler;
	efi_handle_t *handles;
	size_t len, len_dp;
	size_t len_best = 0;
	efi_uintn_t no_handles;
	u8 *remainder;
	efi_status_t ret;

	EFI_ENTRY("%pUs, %p, %p", protocol, device_path, device);

	if (!protocol || !device_path || !*device_path) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Find end of device path */
	len = efi_dp_instance_size(*device_path);

	/* Get all handles implementing the protocol */
	ret = EFI_CALL(efi_locate_handle_buffer(BY_PROTOCOL, protocol, NULL,
						&no_handles, &handles));
	if (ret != EFI_SUCCESS)
		goto out;

	for (i = 0; i < no_handles; ++i) {
		/* Find the device path protocol */
		ret = efi_search_protocol(handles[i], &efi_guid_device_path,
					  &handler);
		if (ret != EFI_SUCCESS)
			continue;
		dp = (struct efi_device_path *)handler->protocol_interface;
		len_dp = efi_dp_instance_size(dp);
		/*
		 * This handle can only be a better fit
		 * if its device path length is longer than the best fit and
		 * if its device path length is shorter of equal the searched
		 * device path.
		 */
		if (len_dp <= len_best || len_dp > len)
			continue;
		/* Check if dp is a subpath of device_path */
		if (memcmp(*device_path, dp, len_dp))
			continue;
		if (!device) {
			ret = EFI_INVALID_PARAMETER;
			goto out;
		}
		*device = handles[i];
		len_best = len_dp;
	}
	if (len_best) {
		remainder = (u8 *)*device_path + len_best;
		*device_path = (struct efi_device_path *)remainder;
		ret = EFI_SUCCESS;
	} else {
		ret = EFI_NOT_FOUND;
	}
out:
	return EFI_EXIT(ret);
}

/**
 * efi_load_image_from_file() - load an image from file system
 *
 * Read a file into a buffer allocated as EFI_BOOT_SERVICES_DATA. It is the
 * callers obligation to update the memory type as needed.
 *
 * @file_path:		the path of the image to load
 * @buffer:		buffer containing the loaded image
 * @size:		size of the loaded image
 * Return:		status code
 */
static
efi_status_t efi_load_image_from_file(struct efi_device_path *file_path,
				      void **buffer, efi_uintn_t *size)
{
	struct efi_file_handle *f;
	efi_status_t ret;
	u64 addr;
	efi_uintn_t bs;

	/* Open file */
	f = efi_file_from_path(file_path);
	if (!f)
		return EFI_NOT_FOUND;

	ret = efi_file_size(f, &bs);
	if (ret != EFI_SUCCESS)
		goto error;

	/*
	 * When reading the file we do not yet know if it contains an
	 * application, a boottime driver, or a runtime driver. So here we
	 * allocate a buffer as EFI_BOOT_SERVICES_DATA. The caller has to
	 * update the reservation according to the image type.
	 */
	ret = efi_allocate_pages(EFI_ALLOCATE_ANY_PAGES,
				 EFI_BOOT_SERVICES_DATA,
				 efi_size_in_pages(bs), &addr);
	if (ret != EFI_SUCCESS) {
		ret = EFI_OUT_OF_RESOURCES;
		goto error;
	}

	/* Read file */
	EFI_CALL(ret = f->read(f, &bs, (void *)(uintptr_t)addr));
	if (ret != EFI_SUCCESS)
		efi_free_pages(addr, efi_size_in_pages(bs));
	*buffer = (void *)(uintptr_t)addr;
	*size = bs;
error:
	EFI_CALL(f->close(f));
	return ret;
}

/**
 * efi_load_image_from_path() - load an image using a file path
 *
 * Read a file into a buffer allocated as EFI_BOOT_SERVICES_DATA. It is the
 * callers obligation to update the memory type as needed.
 *
 * @boot_policy:	true for request originating from the boot manager
 * @file_path:		the path of the image to load
 * @buffer:		buffer containing the loaded image
 * @size:		size of the loaded image
 * Return:		status code
 */
efi_status_t efi_load_image_from_path(bool boot_policy,
				      struct efi_device_path *file_path,
				      void **buffer, efi_uintn_t *size)
{
	efi_handle_t device;
	efi_status_t ret;
	struct efi_device_path *dp, *rem;
	struct efi_load_file_protocol *load_file_protocol = NULL;
	efi_uintn_t buffer_size;
	uint64_t addr, pages;
	const efi_guid_t *guid;
	struct efi_handler *handler;

	/* In case of failure nothing is returned */
	*buffer = NULL;
	*size = 0;

	dp = file_path;
	device = efi_dp_find_obj(dp, NULL, &rem);
	ret = efi_search_protocol(device, &efi_simple_file_system_protocol_guid,
				  NULL);
	if (ret == EFI_SUCCESS)
		return efi_load_image_from_file(file_path, buffer, size);

	ret = efi_search_protocol(device, &efi_guid_load_file_protocol, NULL);
	if (ret == EFI_SUCCESS) {
		guid = &efi_guid_load_file_protocol;
	} else if (!boot_policy) {
		guid = &efi_guid_load_file2_protocol;
		ret = efi_search_protocol(device, guid, NULL);
	}
	if (ret != EFI_SUCCESS)
		return EFI_NOT_FOUND;
	ret = efi_search_protocol(device, guid, &handler);
	if (ret != EFI_SUCCESS)
		return EFI_NOT_FOUND;
	buffer_size = 0;
	load_file_protocol = handler->protocol_interface;
	ret = EFI_CALL(load_file_protocol->load_file(
					load_file_protocol, rem, boot_policy,
					&buffer_size, NULL));
	if (ret != EFI_BUFFER_TOO_SMALL)
		goto out;
	pages = efi_size_in_pages(buffer_size);
	ret = efi_allocate_pages(EFI_ALLOCATE_ANY_PAGES, EFI_BOOT_SERVICES_DATA,
				 pages, &addr);
	if (ret != EFI_SUCCESS) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	ret = EFI_CALL(load_file_protocol->load_file(
					load_file_protocol, rem, boot_policy,
					&buffer_size, (void *)(uintptr_t)addr));
	if (ret != EFI_SUCCESS)
		efi_free_pages(addr, pages);
out:
	efi_close_protocol(device, guid, efi_root, NULL);
	if (ret == EFI_SUCCESS) {
		*buffer = (void *)(uintptr_t)addr;
		*size = buffer_size;
	}

	return ret;
}

/**
 * efi_load_image() - load an EFI image into memory
 * @boot_policy:   true for request originating from the boot manager
 * @parent_image:  the caller's image handle
 * @file_path:     the path of the image to load
 * @source_buffer: memory location from which the image is installed
 * @source_size:   size of the memory area from which the image is installed
 * @image_handle:  handle for the newly installed image
 *
 * This function implements the LoadImage service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_load_image(bool boot_policy,
				   efi_handle_t parent_image,
				   struct efi_device_path *file_path,
				   void *source_buffer,
				   efi_uintn_t source_size,
				   efi_handle_t *image_handle)
{
	struct efi_device_path *dp, *fp;
	struct efi_loaded_image *info = NULL;
	struct efi_loaded_image_obj **image_obj =
		(struct efi_loaded_image_obj **)image_handle;
	efi_status_t ret;
	void *dest_buffer;

	EFI_ENTRY("%d, %p, %pD, %p, %zu, %p", boot_policy, parent_image,
		  file_path, source_buffer, source_size, image_handle);

	if (!image_handle || (!source_buffer && !file_path) ||
	    !efi_search_obj(parent_image) ||
	    /* The parent image handle must refer to a loaded image */
	    !parent_image->type) {
		ret = EFI_INVALID_PARAMETER;
		goto error;
	}

	if (!source_buffer) {
		ret = efi_load_image_from_path(boot_policy, file_path,
					       &dest_buffer, &source_size);
		if (ret != EFI_SUCCESS)
			goto error;
	} else {
		dest_buffer = source_buffer;
	}
	/* split file_path which contains both the device and file parts */
	efi_dp_split_file_path(file_path, &dp, &fp);
	ret = efi_setup_loaded_image(dp, fp, image_obj, &info);
	if (ret == EFI_SUCCESS)
		ret = efi_load_pe(*image_obj, dest_buffer, source_size, info);
	if (!source_buffer)
		/* Release buffer to which file was loaded */
		efi_free_pages((uintptr_t)dest_buffer,
			       efi_size_in_pages(source_size));
	if (ret == EFI_SUCCESS || ret == EFI_SECURITY_VIOLATION) {
		info->system_table = &systab;
		info->parent_handle = parent_image;
	} else {
		/* The image is invalid. Release all associated resources. */
		efi_delete_handle(*image_handle);
		*image_handle = NULL;
		free(info);
	}
error:
	return EFI_EXIT(ret);
}

/**
 * efi_exit_caches() - fix up caches for EFI payloads if necessary
 */
static void efi_exit_caches(void)
{
#if defined(CONFIG_EFI_GRUB_ARM32_WORKAROUND)
	/*
	 * Boooting Linux via GRUB prior to version 2.04 fails on 32bit ARM if
	 * caches are enabled.
	 *
	 * TODO:
	 * According to the UEFI spec caches that can be managed via CP15
	 * operations should be enabled. Caches requiring platform information
	 * to manage should be disabled. This should not happen in
	 * ExitBootServices() but before invoking any UEFI binary is invoked.
	 *
	 * We want to keep the current workaround while GRUB prior to version
	 * 2.04 is still in use.
	 */
	cleanup_before_linux();
#endif
}

/**
 * efi_exit_boot_services() - stop all boot services
 * @image_handle: handle of the loaded image
 * @map_key:      key of the memory map
 *
 * This function implements the ExitBootServices service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * All timer events are disabled. For exit boot services events the
 * notification function is called. The boot services are disabled in the
 * system table.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_exit_boot_services(efi_handle_t image_handle,
						  efi_uintn_t map_key)
{
	struct efi_event *evt, *next_event;
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %zx", image_handle, map_key);

	/* Check that the caller has read the current memory map */
	if (map_key != efi_memory_map_key) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Check if ExitBootServices has already been called */
	if (!systab.boottime)
		goto out;

	/* Notify EFI_EVENT_GROUP_BEFORE_EXIT_BOOT_SERVICES event group. */
	list_for_each_entry(evt, &efi_events, link) {
		if (evt->group &&
		    !guidcmp(evt->group,
			     &efi_guid_event_group_before_exit_boot_services)) {
			efi_signal_event(evt);
			break;
		}
	}

	/* Stop all timer related activities */
	timers_enabled = false;

	/* Add related events to the event group */
	list_for_each_entry(evt, &efi_events, link) {
		if (evt->type == EVT_SIGNAL_EXIT_BOOT_SERVICES)
			evt->group = &efi_guid_event_group_exit_boot_services;
	}
	/* Notify that ExitBootServices is invoked. */
	list_for_each_entry(evt, &efi_events, link) {
		if (evt->group &&
		    !guidcmp(evt->group,
			     &efi_guid_event_group_exit_boot_services)) {
			efi_signal_event(evt);
			break;
		}
	}

	/* Make sure that notification functions are not called anymore */
	efi_tpl = TPL_HIGH_LEVEL;

	/* Notify variable services */
	efi_variables_boot_exit_notify();

	/* Remove all events except EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE */
	list_for_each_entry_safe(evt, next_event, &efi_events, link) {
		if (evt->type != EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE)
			list_del(&evt->link);
	}

	if (!efi_st_keep_devices) {
		bootm_disable_interrupts();
		if (IS_ENABLED(CONFIG_DM_ETH))
			eth_halt();
		board_quiesce_devices();
		dm_remove_devices_active();
	}

	/* Patch out unsupported runtime function */
	efi_runtime_detach();

	/* Fix up caches for EFI payloads if necessary */
	efi_exit_caches();

	/* Disable boot time services */
	systab.con_in_handle = NULL;
	systab.con_in = NULL;
	systab.con_out_handle = NULL;
	systab.con_out = NULL;
	systab.stderr_handle = NULL;
	systab.std_err = NULL;
	systab.boottime = NULL;

	/* Recalculate CRC32 */
	efi_update_table_header_crc32(&systab.hdr);

	/* Give the payload some time to boot */
	efi_set_watchdog(0);
	schedule();
out:
	if (IS_ENABLED(CONFIG_EFI_TCG2_PROTOCOL)) {
		if (ret != EFI_SUCCESS)
			efi_tcg2_notify_exit_boot_services_failed();
	}

	return EFI_EXIT(ret);
}

/**
 * efi_get_next_monotonic_count() - get next value of the counter
 * @count: returned value of the counter
 *
 * This function implements the NextMonotonicCount service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_get_next_monotonic_count(uint64_t *count)
{
	static uint64_t mono;
	efi_status_t ret;

	EFI_ENTRY("%p", count);
	if (!count) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	*count = mono++;
	ret = EFI_SUCCESS;
out:
	return EFI_EXIT(ret);
}

/**
 * efi_stall() - sleep
 * @microseconds: period to sleep in microseconds
 *
 * This function implements the Stall service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return:  status code
 */
static efi_status_t EFIAPI efi_stall(unsigned long microseconds)
{
	u64 end_tick;

	EFI_ENTRY("%ld", microseconds);

	end_tick = get_ticks() + usec_to_tick(microseconds);
	while (get_ticks() < end_tick)
		efi_timer_check();

	return EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_set_watchdog_timer() - reset the watchdog timer
 * @timeout:       seconds before reset by watchdog
 * @watchdog_code: code to be logged when resetting
 * @data_size:     size of buffer in bytes
 * @watchdog_data: buffer with data describing the reset reason
 *
 * This function implements the SetWatchdogTimer service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_set_watchdog_timer(unsigned long timeout,
						  uint64_t watchdog_code,
						  unsigned long data_size,
						  uint16_t *watchdog_data)
{
	EFI_ENTRY("%ld, 0x%llx, %ld, %p", timeout, watchdog_code,
		  data_size, watchdog_data);
	return EFI_EXIT(efi_set_watchdog(timeout));
}

/**
 * efi_close_protocol() - close a protocol
 * @handle:            handle on which the protocol shall be closed
 * @protocol:          GUID of the protocol to close
 * @agent_handle:      handle of the driver
 * @controller_handle: handle of the controller
 *
 * This is the function implementing the CloseProtocol service is for internal
 * usage in U-Boot. For API usage wrapper efi_close_protocol_ext() is provided.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
efi_status_t efi_close_protocol(efi_handle_t handle, const efi_guid_t *protocol,
				efi_handle_t agent_handle,
				efi_handle_t controller_handle)
{
	struct efi_handler *handler;
	struct efi_open_protocol_info_item *item;
	struct efi_open_protocol_info_item *pos;
	efi_status_t ret;

	if (!efi_search_obj(agent_handle) ||
	    (controller_handle && !efi_search_obj(controller_handle)))
		return EFI_INVALID_PARAMETER;
	ret = efi_search_protocol(handle, protocol, &handler);
	if (ret != EFI_SUCCESS)
		return ret;

	ret = EFI_NOT_FOUND;
	list_for_each_entry_safe(item, pos, &handler->open_infos, link) {
		if (item->info.agent_handle == agent_handle &&
		    item->info.controller_handle == controller_handle) {
			efi_delete_open_info(item);
			ret = EFI_SUCCESS;
		}
	}

	return ret;
}

/**
 * efi_close_protocol_ext() - close a protocol
 * @handle:            handle on which the protocol shall be closed
 * @protocol:          GUID of the protocol to close
 * @agent_handle:      handle of the driver
 * @controller_handle: handle of the controller
 *
 * This function implements the CloseProtocol service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI
efi_close_protocol_ext(efi_handle_t handle, const efi_guid_t *protocol,
		       efi_handle_t agent_handle,
		       efi_handle_t controller_handle)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %pUs, %p, %p", handle, protocol, agent_handle,
		  controller_handle);

	ret = efi_close_protocol(handle, protocol,
				 agent_handle, controller_handle);

	return EFI_EXIT(ret);
}

/**
 * efi_open_protocol_information() - provide information about then open status
 *                                   of a protocol on a handle
 * @handle:       handle for which the information shall be retrieved
 * @protocol:     GUID of the protocol
 * @entry_buffer: buffer to receive the open protocol information
 * @entry_count:  number of entries available in the buffer
 *
 * This function implements the OpenProtocolInformation service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_open_protocol_information(
			efi_handle_t handle, const efi_guid_t *protocol,
			struct efi_open_protocol_info_entry **entry_buffer,
			efi_uintn_t *entry_count)
{
	unsigned long buffer_size;
	unsigned long count;
	struct efi_handler *handler;
	struct efi_open_protocol_info_item *item;
	efi_status_t r;

	EFI_ENTRY("%p, %pUs, %p, %p", handle, protocol, entry_buffer,
		  entry_count);

	/* Check parameters */
	if (!entry_buffer) {
		r = EFI_INVALID_PARAMETER;
		goto out;
	}
	r = efi_search_protocol(handle, protocol, &handler);
	if (r != EFI_SUCCESS)
		goto out;

	/* Count entries */
	count = 0;
	list_for_each_entry(item, &handler->open_infos, link) {
		if (item->info.open_count)
			++count;
	}
	*entry_count = count;
	*entry_buffer = NULL;
	if (!count) {
		r = EFI_SUCCESS;
		goto out;
	}

	/* Copy entries */
	buffer_size = count * sizeof(struct efi_open_protocol_info_entry);
	r = efi_allocate_pool(EFI_BOOT_SERVICES_DATA, buffer_size,
			      (void **)entry_buffer);
	if (r != EFI_SUCCESS)
		goto out;
	list_for_each_entry_reverse(item, &handler->open_infos, link) {
		if (item->info.open_count)
			(*entry_buffer)[--count] = item->info;
	}
out:
	return EFI_EXIT(r);
}

/**
 * efi_protocols_per_handle() - get protocols installed on a handle
 * @handle:                handle for which the information is retrieved
 * @protocol_buffer:       buffer with protocol GUIDs
 * @protocol_buffer_count: number of entries in the buffer
 *
 * This function implements the ProtocolsPerHandleService.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_protocols_per_handle(
			efi_handle_t handle, efi_guid_t ***protocol_buffer,
			efi_uintn_t *protocol_buffer_count)
{
	unsigned long buffer_size;
	struct efi_object *efiobj;
	struct list_head *protocol_handle;
	efi_status_t r;

	EFI_ENTRY("%p, %p, %p", handle, protocol_buffer,
		  protocol_buffer_count);

	if (!handle || !protocol_buffer || !protocol_buffer_count)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	*protocol_buffer = NULL;

	efiobj = efi_search_obj(handle);
	if (!efiobj)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	*protocol_buffer_count = list_count_nodes(&efiobj->protocols);

	/* Copy GUIDs */
	if (*protocol_buffer_count) {
		size_t j = 0;

		buffer_size = sizeof(efi_guid_t *) * *protocol_buffer_count;
		r = efi_allocate_pool(EFI_BOOT_SERVICES_DATA, buffer_size,
				      (void **)protocol_buffer);
		if (r != EFI_SUCCESS)
			return EFI_EXIT(r);
		list_for_each(protocol_handle, &efiobj->protocols) {
			struct efi_handler *protocol;

			protocol = list_entry(protocol_handle,
					      struct efi_handler, link);
			(*protocol_buffer)[j] = (void *)&protocol->guid;
			++j;
		}
	}

	return EFI_EXIT(EFI_SUCCESS);
}

efi_status_t efi_locate_handle_buffer_int(enum efi_locate_search_type search_type,
					  const efi_guid_t *protocol, void *search_key,
					  efi_uintn_t *no_handles, efi_handle_t **buffer)
{
	efi_status_t r;
	efi_uintn_t buffer_size = 0;

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
	r = efi_allocate_pool(EFI_BOOT_SERVICES_DATA, buffer_size,
			      (void **)buffer);
	if (r != EFI_SUCCESS)
		goto out;
	r = efi_locate_handle(search_type, protocol, search_key, &buffer_size,
			      *buffer);
	if (r == EFI_SUCCESS)
		*no_handles = buffer_size / sizeof(efi_handle_t);
out:
	return r;
}

/**
 * efi_locate_handle_buffer() - locate handles implementing a protocol
 * @search_type: selection criterion
 * @protocol:    GUID of the protocol
 * @search_key:  registration key
 * @no_handles:  number of returned handles
 * @buffer:      buffer with the returned handles
 *
 * This function implements the LocateHandleBuffer service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_locate_handle_buffer(
			enum efi_locate_search_type search_type,
			const efi_guid_t *protocol, void *search_key,
			efi_uintn_t *no_handles, efi_handle_t **buffer)
{
	efi_status_t r;

	EFI_ENTRY("%d, %pUs, %p, %p, %p", search_type, protocol, search_key,
		  no_handles, buffer);

	r = efi_locate_handle_buffer_int(search_type, protocol, search_key,
					 no_handles, buffer);

	return EFI_EXIT(r);
}

/**
 * efi_locate_protocol() - find an interface implementing a protocol
 * @protocol:           GUID of the protocol
 * @registration:       registration key passed to the notification function
 * @protocol_interface: interface implementing the protocol
 *
 * This function implements the LocateProtocol service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_locate_protocol(const efi_guid_t *protocol,
					       void *registration,
					       void **protocol_interface)
{
	struct efi_handler *handler;
	efi_status_t ret;
	struct efi_object *efiobj;

	EFI_ENTRY("%pUs, %p, %p", protocol, registration, protocol_interface);

	/*
	 * The UEFI spec explicitly requires a protocol even if a registration
	 * key is provided. This differs from the logic in LocateHandle().
	 */
	if (!protocol || !protocol_interface)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	if (registration) {
		struct efi_register_notify_event *event;
		struct efi_protocol_notification *handle;

		event = efi_check_register_notify_event(registration);
		if (!event)
			return EFI_EXIT(EFI_INVALID_PARAMETER);
		/*
		 * The UEFI spec requires to return EFI_NOT_FOUND if no
		 * protocol instance matches protocol and registration.
		 * So let's do the same for a mismatch between protocol and
		 * registration.
		 */
		if (guidcmp(&event->protocol, protocol))
			goto not_found;
		if (list_empty(&event->handles))
			goto not_found;
		handle = list_first_entry(&event->handles,
					  struct efi_protocol_notification,
					  link);
		efiobj = handle->handle;
		list_del(&handle->link);
		free(handle);
		ret = efi_search_protocol(efiobj, protocol, &handler);
		if (ret == EFI_SUCCESS)
			goto found;
	} else {
		list_for_each_entry(efiobj, &efi_obj_list, link) {
			ret = efi_search_protocol(efiobj, protocol, &handler);
			if (ret == EFI_SUCCESS)
				goto found;
		}
	}
not_found:
	*protocol_interface = NULL;
	return EFI_EXIT(EFI_NOT_FOUND);
found:
	*protocol_interface = handler->protocol_interface;
	return EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_install_multiple_protocol_interfaces_int() - Install multiple protocol
 *                                              interfaces
 * @handle: handle on which the protocol interfaces shall be installed
 * @argptr: va_list of args
 *
 * Core functionality of efi_install_multiple_protocol_interfaces
 * Must not be called directly
 *
 * Return: status code
 */
static efi_status_t EFIAPI
efi_install_multiple_protocol_interfaces_int(efi_handle_t *handle,
					     efi_va_list argptr)
{
	const efi_guid_t *protocol;
	void *protocol_interface;
	efi_handle_t old_handle;
	efi_status_t ret = EFI_SUCCESS;
	int i = 0;
	efi_va_list argptr_copy;

	if (!handle)
		return EFI_INVALID_PARAMETER;

	efi_va_copy(argptr_copy, argptr);
	for (;;) {
		protocol = efi_va_arg(argptr, efi_guid_t*);
		if (!protocol)
			break;
		protocol_interface = efi_va_arg(argptr, void*);
		/* Check that a device path has not been installed before */
		if (!guidcmp(protocol, &efi_guid_device_path)) {
			struct efi_device_path *dp = protocol_interface;

			ret = EFI_CALL(efi_locate_device_path(protocol, &dp,
							      &old_handle));
			if (ret == EFI_SUCCESS &&
			    dp->type == DEVICE_PATH_TYPE_END) {
				EFI_PRINT("Path %pD already installed\n",
					  protocol_interface);
				ret = EFI_ALREADY_STARTED;
				break;
			}
		}
		ret = EFI_CALL(efi_install_protocol_interface(handle, protocol,
							      EFI_NATIVE_INTERFACE,
							      protocol_interface));
		if (ret != EFI_SUCCESS)
			break;
		i++;
	}
	if (ret == EFI_SUCCESS)
		goto out;

	/* If an error occurred undo all changes. */
	for (; i; --i) {
		protocol = efi_va_arg(argptr_copy, efi_guid_t*);
		protocol_interface = efi_va_arg(argptr_copy, void*);
		EFI_CALL(efi_uninstall_protocol_interface(*handle, protocol,
							  protocol_interface));
	}

out:
	efi_va_end(argptr_copy);
	return ret;

}

/**
 * efi_install_multiple_protocol_interfaces() - Install multiple protocol
 *                                              interfaces
 * @handle: handle on which the protocol interfaces shall be installed
 * @...:    NULL terminated argument list with pairs of protocol GUIDS and
 *          interfaces
 *
 *
 * This is the function for internal usage in U-Boot. For the API function
 * implementing the InstallMultipleProtocol service see
 * efi_install_multiple_protocol_interfaces_ext()
 *
 * Return: status code
 */
efi_status_t EFIAPI
efi_install_multiple_protocol_interfaces(efi_handle_t *handle, ...)
{
	efi_status_t ret;
	efi_va_list argptr;

	efi_va_start(argptr, handle);
	ret = efi_install_multiple_protocol_interfaces_int(handle, argptr);
	efi_va_end(argptr);
	return ret;
}

/**
 * efi_install_multiple_protocol_interfaces_ext() - Install multiple protocol
 *                                                  interfaces
 * @handle: handle on which the protocol interfaces shall be installed
 * @...:    NULL terminated argument list with pairs of protocol GUIDS and
 *          interfaces
 *
 * This function implements the MultipleProtocolInterfaces service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI
efi_install_multiple_protocol_interfaces_ext(efi_handle_t *handle, ...)
{
	EFI_ENTRY("%p", handle);
	efi_status_t ret;
	efi_va_list argptr;

	efi_va_start(argptr, handle);
	ret = efi_install_multiple_protocol_interfaces_int(handle, argptr);
	efi_va_end(argptr);
	return EFI_EXIT(ret);
}

/**
 * efi_uninstall_multiple_protocol_interfaces_int() - wrapper for uninstall
 *                                                  multiple protocol
 *                                                  interfaces
 * @handle: handle from which the protocol interfaces shall be removed
 * @argptr: va_list of args
 *
 * Core functionality of efi_uninstall_multiple_protocol_interfaces
 * Must not be called directly
 *
 * Return: status code
 */
static efi_status_t EFIAPI
efi_uninstall_multiple_protocol_interfaces_int(efi_handle_t handle,
					       efi_va_list argptr)
{
	const efi_guid_t *protocol, *next_protocol;
	void *protocol_interface;
	efi_status_t ret = EFI_SUCCESS;
	size_t i = 0;
	efi_va_list argptr_copy;

	if (!handle)
		return EFI_INVALID_PARAMETER;

	efi_va_copy(argptr_copy, argptr);
	protocol = efi_va_arg(argptr, efi_guid_t*);
	for (;;) {
		/*
		 * If efi_uninstall_protocol() fails we need to be able to
		 * reinstall the previously uninstalled protocols on the same
		 * handle.
		 * Instead of calling efi_uninstall_protocol(...,..., false)
		 * and potentially removing the handle, only allow the handle
		 * removal on the last protocol that we requested to uninstall.
		 * That way we can preserve  the handle in case the latter fails
		 */
		bool preserve = true;

		if (!protocol)
			break;
		protocol_interface = efi_va_arg(argptr, void*);
		next_protocol = efi_va_arg(argptr, efi_guid_t*);
		if (!next_protocol)
			preserve = false;
		ret = efi_uninstall_protocol(handle, protocol,
					     protocol_interface, preserve);
		if (ret != EFI_SUCCESS)
			break;
		i++;
		protocol = next_protocol;
	}
	if (ret == EFI_SUCCESS)
		goto out;

	/* If an error occurred undo all changes. */
	for (; i; --i) {
		protocol = efi_va_arg(argptr_copy, efi_guid_t*);
		protocol_interface = efi_va_arg(argptr_copy, void*);
		EFI_CALL(efi_install_protocol_interface(&handle, protocol,
							EFI_NATIVE_INTERFACE,
							protocol_interface));
	}
	/*
	 * If any errors are generated while the protocol interfaces are being
	 * uninstalled, then the protocols uninstalled prior to the error will
	 * be reinstalled using InstallProtocolInterface() and the status code
	 * EFI_INVALID_PARAMETER is returned.
	 */
	ret = EFI_INVALID_PARAMETER;

out:
	efi_va_end(argptr_copy);
	return ret;
}

/**
 * efi_uninstall_multiple_protocol_interfaces() - uninstall multiple protocol
 *                                                interfaces
 * @handle: handle from which the protocol interfaces shall be removed
 * @...:    NULL terminated argument list with pairs of protocol GUIDS and
 *          interfaces
 *
 * This function implements the UninstallMultipleProtocolInterfaces service.
 *
 * This is the function for internal usage in U-Boot. For the API function
 * implementing the UninstallMultipleProtocolInterfaces service see
 * efi_uninstall_multiple_protocol_interfaces_ext()
 *
 * Return: status code
 */
efi_status_t EFIAPI
efi_uninstall_multiple_protocol_interfaces(efi_handle_t handle, ...)
{
	efi_status_t ret;
	efi_va_list argptr;

	efi_va_start(argptr, handle);
	ret = efi_uninstall_multiple_protocol_interfaces_int(handle, argptr);
	efi_va_end(argptr);
	return ret;
}

/**
 * efi_uninstall_multiple_protocol_interfaces_ext() - uninstall multiple protocol
 *                                                    interfaces
 * @handle: handle from which the protocol interfaces shall be removed
 * @...:    NULL terminated argument list with pairs of protocol GUIDS and
 *          interfaces
 *
 * This function implements the UninstallMultipleProtocolInterfaces service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI
efi_uninstall_multiple_protocol_interfaces_ext(efi_handle_t handle, ...)
{
	EFI_ENTRY("%p", handle);
	efi_status_t ret;
	efi_va_list argptr;

	efi_va_start(argptr, handle);
	ret = efi_uninstall_multiple_protocol_interfaces_int(handle, argptr);
	efi_va_end(argptr);
	return EFI_EXIT(ret);
}

/**
 * efi_calculate_crc32() - calculate cyclic redundancy code
 * @data:      buffer with data
 * @data_size: size of buffer in bytes
 * @crc32_p:   cyclic redundancy code
 *
 * This function implements the CalculateCrc32 service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_calculate_crc32(const void *data,
					       efi_uintn_t data_size,
					       u32 *crc32_p)
{
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %zu", data, data_size);
	if (!data || !data_size || !crc32_p) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	*crc32_p = crc32(0, data, data_size);
out:
	return EFI_EXIT(ret);
}

/**
 * efi_copy_mem() - copy memory
 * @destination: destination of the copy operation
 * @source:      source of the copy operation
 * @length:      number of bytes to copy
 *
 * This function implements the CopyMem service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 */
static void EFIAPI efi_copy_mem(void *destination, const void *source,
				size_t length)
{
	EFI_ENTRY("%p, %p, %ld", destination, source, (unsigned long)length);
	memmove(destination, source, length);
	EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_set_mem() - Fill memory with a byte value.
 * @buffer: buffer to fill
 * @size:   size of buffer in bytes
 * @value:  byte to copy to the buffer
 *
 * This function implements the SetMem service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 */
static void EFIAPI efi_set_mem(void *buffer, size_t size, uint8_t value)
{
	EFI_ENTRY("%p, %ld, 0x%x", buffer, (unsigned long)size, value);
	memset(buffer, value, size);
	EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_protocol_open() - open protocol interface on a handle
 * @handler:            handler of a protocol
 * @protocol_interface: interface implementing the protocol
 * @agent_handle:       handle of the driver
 * @controller_handle:  handle of the controller
 * @attributes:         attributes indicating how to open the protocol
 *
 * Return: status code
 */
efi_status_t efi_protocol_open(
			struct efi_handler *handler,
			void **protocol_interface, void *agent_handle,
			void *controller_handle, uint32_t attributes)
{
	struct efi_open_protocol_info_item *item;
	struct efi_open_protocol_info_entry *match = NULL;
	bool opened_by_driver = false;
	bool opened_exclusive = false;

	/* If there is no agent, only return the interface */
	if (!agent_handle)
		goto out;

	/* For TEST_PROTOCOL ignore interface attribute */
	if (attributes != EFI_OPEN_PROTOCOL_TEST_PROTOCOL)
		*protocol_interface = NULL;

	/*
	 * Check if the protocol is already opened by a driver with the same
	 * attributes or opened exclusively
	 */
	list_for_each_entry(item, &handler->open_infos, link) {
		if (item->info.agent_handle == agent_handle) {
			if ((attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) &&
			    (item->info.attributes == attributes))
				return EFI_ALREADY_STARTED;
		} else {
			if (item->info.attributes &
			    EFI_OPEN_PROTOCOL_BY_DRIVER)
				opened_by_driver = true;
		}
		if (item->info.attributes & EFI_OPEN_PROTOCOL_EXCLUSIVE)
			opened_exclusive = true;
	}

	/* Only one controller can open the protocol exclusively */
	if (attributes & EFI_OPEN_PROTOCOL_EXCLUSIVE) {
		if (opened_exclusive)
			return EFI_ACCESS_DENIED;
	} else if (attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) {
		if (opened_exclusive || opened_by_driver)
			return EFI_ACCESS_DENIED;
	}

	/* Prepare exclusive opening */
	if (attributes & EFI_OPEN_PROTOCOL_EXCLUSIVE) {
		/* Try to disconnect controllers */
disconnect_next:
		opened_by_driver = false;
		list_for_each_entry(item, &handler->open_infos, link) {
			efi_status_t ret;

			if (item->info.attributes ==
					EFI_OPEN_PROTOCOL_BY_DRIVER) {
				ret = EFI_CALL(efi_disconnect_controller(
						item->info.controller_handle,
						item->info.agent_handle,
						NULL));
				if (ret == EFI_SUCCESS)
					/*
					 * Child controllers may have been
					 * removed from the open_infos list. So
					 * let's restart the loop.
					 */
					goto disconnect_next;
				else
					opened_by_driver = true;
			}
		}
		/* Only one driver can be connected */
		if (opened_by_driver)
			return EFI_ACCESS_DENIED;
	}

	/* Find existing entry */
	list_for_each_entry(item, &handler->open_infos, link) {
		if (item->info.agent_handle == agent_handle &&
		    item->info.controller_handle == controller_handle &&
		    item->info.attributes == attributes)
			match = &item->info;
	}
	/* None found, create one */
	if (!match) {
		match = efi_create_open_info(handler);
		if (!match)
			return EFI_OUT_OF_RESOURCES;
	}

	match->agent_handle = agent_handle;
	match->controller_handle = controller_handle;
	match->attributes = attributes;
	match->open_count++;

out:
	/* For TEST_PROTOCOL ignore interface attribute. */
	if (attributes != EFI_OPEN_PROTOCOL_TEST_PROTOCOL)
		*protocol_interface = handler->protocol_interface;

	return EFI_SUCCESS;
}

/**
 * efi_open_protocol() - open protocol interface on a handle
 * @handle:             handle on which the protocol shall be opened
 * @protocol:           GUID of the protocol
 * @protocol_interface: interface implementing the protocol
 * @agent_handle:       handle of the driver
 * @controller_handle:  handle of the controller
 * @attributes:         attributes indicating how to open the protocol
 *
 * This function implements the OpenProtocol interface.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_open_protocol
			(efi_handle_t handle, const efi_guid_t *protocol,
			 void **protocol_interface, efi_handle_t agent_handle,
			 efi_handle_t controller_handle, uint32_t attributes)
{
	struct efi_handler *handler;
	efi_status_t r = EFI_INVALID_PARAMETER;

	EFI_ENTRY("%p, %pUs, %p, %p, %p, 0x%x", handle, protocol,
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
		/* fall-through */
	case EFI_OPEN_PROTOCOL_BY_DRIVER:
	case EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE:
		/* Check that the controller handle is valid */
		if (!efi_search_obj(controller_handle))
			goto out;
		/* fall-through */
	case EFI_OPEN_PROTOCOL_EXCLUSIVE:
		/* Check that the agent handle is valid */
		if (!efi_search_obj(agent_handle))
			goto out;
		break;
	default:
		goto out;
	}

	r = efi_search_protocol(handle, protocol, &handler);
	switch (r) {
	case EFI_SUCCESS:
		break;
	case EFI_NOT_FOUND:
		r = EFI_UNSUPPORTED;
		goto out;
	default:
		goto out;
	}

	r = efi_protocol_open(handler, protocol_interface, agent_handle,
			      controller_handle, attributes);
out:
	return EFI_EXIT(r);
}

/**
 * efi_start_image() - call the entry point of an image
 * @image_handle:   handle of the image
 * @exit_data_size: size of the buffer
 * @exit_data:      buffer to receive the exit data of the called image
 *
 * This function implements the StartImage service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_start_image(efi_handle_t image_handle,
				    efi_uintn_t *exit_data_size,
				    u16 **exit_data)
{
	struct efi_loaded_image_obj *image_obj =
		(struct efi_loaded_image_obj *)image_handle;
	efi_status_t ret;
	void *info;
	efi_handle_t parent_image = current_image;
	efi_status_t exit_status;
	jmp_buf exit_jmp;

	EFI_ENTRY("%p, %p, %p", image_handle, exit_data_size, exit_data);

	if (!efi_search_obj(image_handle))
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	/* Check parameters */
	if (image_obj->header.type != EFI_OBJECT_TYPE_LOADED_IMAGE)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	if (image_obj->auth_status != EFI_IMAGE_AUTH_PASSED)
		return EFI_EXIT(EFI_SECURITY_VIOLATION);

	ret = EFI_CALL(efi_open_protocol(image_handle, &efi_guid_loaded_image,
					 &info, NULL, NULL,
					 EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	if (ret != EFI_SUCCESS)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	image_obj->exit_data_size = exit_data_size;
	image_obj->exit_data = exit_data;
	image_obj->exit_status = &exit_status;
	image_obj->exit_jmp = &exit_jmp;

	if (IS_ENABLED(CONFIG_EFI_TCG2_PROTOCOL)) {
		if (image_obj->image_type == IMAGE_SUBSYSTEM_EFI_APPLICATION) {
			ret = efi_tcg2_measure_efi_app_invocation(image_obj);
			if (ret == EFI_SECURITY_VIOLATION) {
				/*
				 * TCG2 Protocol is installed but no TPM device found,
				 * this is not expected.
				 */
				return EFI_EXIT(EFI_SECURITY_VIOLATION);
			}
		}
	}

	/* call the image! */
	if (setjmp(exit_jmp)) {
		/*
		 * We called the entry point of the child image with EFI_CALL
		 * in the lines below. The child image called the Exit() boot
		 * service efi_exit() which executed the long jump that brought
		 * us to the current line. This implies that the second half
		 * of the EFI_CALL macro has not been executed.
		 */
#if defined(CONFIG_ARM) || defined(CONFIG_RISCV)
		/*
		 * efi_exit() called efi_restore_gd(). We have to undo this
		 * otherwise __efi_entry_check() will put the wrong value into
		 * app_gd.
		 */
		set_gd(app_gd);
#endif
		/*
		 * To get ready to call EFI_EXIT below we have to execute the
		 * missed out steps of EFI_CALL.
		 */
		EFI_RETURN(exit_status);

		current_image = parent_image;

		return EFI_EXIT(exit_status);
	}

	current_image = image_handle;
	image_obj->header.type = EFI_OBJECT_TYPE_STARTED_IMAGE;
	EFI_PRINT("Jumping into 0x%p\n", image_obj->entry);
	ret = EFI_CALL(image_obj->entry(image_handle, &systab));

	/*
	 * Control is returned from a started UEFI image either by calling
	 * Exit() (where exit data can be provided) or by simply returning from
	 * the entry point. In the latter case call Exit() on behalf of the
	 * image.
	 */
	return EFI_CALL(systab.boottime->exit(image_handle, ret, 0, NULL));
}

/**
 * efi_delete_image() - delete loaded image from memory)
 *
 * @image_obj:			handle of the loaded image
 * @loaded_image_protocol:	loaded image protocol
 */
static efi_status_t efi_delete_image
			(struct efi_loaded_image_obj *image_obj,
			 struct efi_loaded_image *loaded_image_protocol)
{
	struct efi_object *efiobj;
	efi_status_t r, ret = EFI_SUCCESS;

close_next:
	list_for_each_entry(efiobj, &efi_obj_list, link) {
		struct efi_handler *protocol;

		list_for_each_entry(protocol, &efiobj->protocols, link) {
			struct efi_open_protocol_info_item *info;

			list_for_each_entry(info, &protocol->open_infos, link) {
				if (info->info.agent_handle !=
				    (efi_handle_t)image_obj)
					continue;
				r = efi_close_protocol(
						efiobj, &protocol->guid,
						info->info.agent_handle,
						info->info.controller_handle);
				if (r !=  EFI_SUCCESS)
					ret = r;
				/*
				 * Closing protocols may results in further
				 * items being deleted. To play it safe loop
				 * over all elements again.
				 */
				goto close_next;
			}
		}
	}

	efi_free_pages((uintptr_t)loaded_image_protocol->image_base,
		       efi_size_in_pages(loaded_image_protocol->image_size));
	efi_delete_handle(&image_obj->header);

	return ret;
}

/**
 * efi_unload_image() - unload an EFI image
 * @image_handle: handle of the image to be unloaded
 *
 * This function implements the UnloadImage service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_unload_image(efi_handle_t image_handle)
{
	efi_status_t ret = EFI_SUCCESS;
	struct efi_object *efiobj;
	struct efi_loaded_image *loaded_image_protocol;

	EFI_ENTRY("%p", image_handle);

	efiobj = efi_search_obj(image_handle);
	if (!efiobj) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	/* Find the loaded image protocol */
	ret = EFI_CALL(efi_open_protocol(image_handle, &efi_guid_loaded_image,
					 (void **)&loaded_image_protocol,
					 NULL, NULL,
					 EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	if (ret != EFI_SUCCESS) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	switch (efiobj->type) {
	case EFI_OBJECT_TYPE_STARTED_IMAGE:
		/* Call the unload function */
		if (!loaded_image_protocol->unload) {
			ret = EFI_UNSUPPORTED;
			goto out;
		}
		ret = EFI_CALL(loaded_image_protocol->unload(image_handle));
		if (ret != EFI_SUCCESS)
			goto out;
		break;
	case EFI_OBJECT_TYPE_LOADED_IMAGE:
		break;
	default:
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	efi_delete_image((struct efi_loaded_image_obj *)efiobj,
			 loaded_image_protocol);
out:
	return EFI_EXIT(ret);
}

/**
 * efi_update_exit_data() - fill exit data parameters of StartImage()
 *
 * @image_obj:		image handle
 * @exit_data_size:	size of the exit data buffer
 * @exit_data:		buffer with data returned by UEFI payload
 * Return:		status code
 */
static efi_status_t efi_update_exit_data(struct efi_loaded_image_obj *image_obj,
					 efi_uintn_t exit_data_size,
					 u16 *exit_data)
{
	efi_status_t ret;

	/*
	 * If exit_data is not provided to StartImage(), exit_data_size must be
	 * ignored.
	 */
	if (!image_obj->exit_data)
		return EFI_SUCCESS;
	if (image_obj->exit_data_size)
		*image_obj->exit_data_size = exit_data_size;
	if (exit_data_size && exit_data) {
		ret = efi_allocate_pool(EFI_BOOT_SERVICES_DATA,
					exit_data_size,
					(void **)image_obj->exit_data);
		if (ret != EFI_SUCCESS)
			return ret;
		memcpy(*image_obj->exit_data, exit_data, exit_data_size);
	} else {
		image_obj->exit_data = NULL;
	}
	return EFI_SUCCESS;
}

/**
 * efi_exit() - leave an EFI application or driver
 * @image_handle:   handle of the application or driver that is exiting
 * @exit_status:    status code
 * @exit_data_size: size of the buffer in bytes
 * @exit_data:      buffer with data describing an error
 *
 * This function implements the Exit service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
static efi_status_t EFIAPI efi_exit(efi_handle_t image_handle,
				    efi_status_t exit_status,
				    efi_uintn_t exit_data_size,
				    u16 *exit_data)
{
	/*
	 * TODO: We should call the unload procedure of the loaded
	 *	 image protocol.
	 */
	efi_status_t ret;
	struct efi_loaded_image *loaded_image_protocol;
	struct efi_loaded_image_obj *image_obj =
		(struct efi_loaded_image_obj *)image_handle;
	jmp_buf *exit_jmp;

	EFI_ENTRY("%p, %ld, %zu, %p", image_handle, exit_status,
		  exit_data_size, exit_data);

	/* Check parameters */
	ret = EFI_CALL(efi_open_protocol(image_handle, &efi_guid_loaded_image,
					 (void **)&loaded_image_protocol,
					 NULL, NULL,
					 EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	if (ret != EFI_SUCCESS) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Unloading of unstarted images */
	switch (image_obj->header.type) {
	case EFI_OBJECT_TYPE_STARTED_IMAGE:
		break;
	case EFI_OBJECT_TYPE_LOADED_IMAGE:
		efi_delete_image(image_obj, loaded_image_protocol);
		ret = EFI_SUCCESS;
		goto out;
	default:
		/* Handle does not refer to loaded image */
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	/* A started image can only be unloaded it is the last one started. */
	if (image_handle != current_image) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Exit data is only foreseen in case of failure. */
	if (exit_status != EFI_SUCCESS) {
		ret = efi_update_exit_data(image_obj, exit_data_size,
					   exit_data);
		/* Exiting has priority. Don't return error to caller. */
		if (ret != EFI_SUCCESS)
			EFI_PRINT("%s: out of memory\n", __func__);
	}
	/* efi_delete_image() frees image_obj. Copy before the call. */
	exit_jmp = image_obj->exit_jmp;
	*image_obj->exit_status = exit_status;
	if (image_obj->image_type == IMAGE_SUBSYSTEM_EFI_APPLICATION ||
	    exit_status != EFI_SUCCESS)
		efi_delete_image(image_obj, loaded_image_protocol);

	if (IS_ENABLED(CONFIG_EFI_TCG2_PROTOCOL)) {
		if (image_obj->image_type == IMAGE_SUBSYSTEM_EFI_APPLICATION) {
			ret = efi_tcg2_measure_efi_app_exit();
			if (ret != EFI_SUCCESS)
				log_debug("tcg2 measurement fails (0x%lx)\n",
					  ret);
		}
	}

	/* Make sure entry/exit counts for EFI world cross-overs match */
	EFI_EXIT(exit_status);

	/*
	 * But longjmp out with the U-Boot gd, not the application's, as
	 * the other end is a setjmp call inside EFI context.
	 */
	efi_restore_gd();

	longjmp(*exit_jmp, 1);

	panic("EFI application exited");
out:
	return EFI_EXIT(ret);
}

/**
 * efi_handle_protocol() - get interface of a protocol on a handle
 * @handle:             handle on which the protocol shall be opened
 * @protocol:           GUID of the protocol
 * @protocol_interface: interface implementing the protocol
 *
 * This function implements the HandleProtocol service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_handle_protocol(efi_handle_t handle,
					const efi_guid_t *protocol,
					void **protocol_interface)
{
	return efi_open_protocol(handle, protocol, protocol_interface, efi_root,
				 NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
}

/**
 * efi_bind_controller() - bind a single driver to a controller
 * @controller_handle:   controller handle
 * @driver_image_handle: driver handle
 * @remain_device_path:  remaining path
 *
 * Return: status code
 */
static efi_status_t efi_bind_controller(
			efi_handle_t controller_handle,
			efi_handle_t driver_image_handle,
			struct efi_device_path *remain_device_path)
{
	struct efi_driver_binding_protocol *binding_protocol;
	efi_status_t r;

	r = EFI_CALL(efi_open_protocol(driver_image_handle,
				       &efi_guid_driver_binding_protocol,
				       (void **)&binding_protocol,
				       driver_image_handle, NULL,
				       EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	if (r != EFI_SUCCESS)
		return r;
	r = EFI_CALL(binding_protocol->supported(binding_protocol,
						 controller_handle,
						 remain_device_path));
	if (r == EFI_SUCCESS)
		r = EFI_CALL(binding_protocol->start(binding_protocol,
						     controller_handle,
						     remain_device_path));
	efi_close_protocol(driver_image_handle,
			   &efi_guid_driver_binding_protocol,
			   driver_image_handle, NULL);
	return r;
}

/**
 * efi_connect_single_controller() - connect a single driver to a controller
 * @controller_handle:   controller
 * @driver_image_handle: driver
 * @remain_device_path:  remaining path
 *
 * Return: status code
 */
static efi_status_t efi_connect_single_controller(
			efi_handle_t controller_handle,
			efi_handle_t *driver_image_handle,
			struct efi_device_path *remain_device_path)
{
	efi_handle_t *buffer;
	size_t count;
	size_t i;
	efi_status_t r;
	size_t connected = 0;

	/* Get buffer with all handles with driver binding protocol */
	r = EFI_CALL(efi_locate_handle_buffer(BY_PROTOCOL,
					      &efi_guid_driver_binding_protocol,
					      NULL, &count, &buffer));
	if (r != EFI_SUCCESS)
		return r;

	/* Context Override */
	if (driver_image_handle) {
		for (; *driver_image_handle; ++driver_image_handle) {
			for (i = 0; i < count; ++i) {
				if (buffer[i] == *driver_image_handle) {
					buffer[i] = NULL;
					r = efi_bind_controller(
							controller_handle,
							*driver_image_handle,
							remain_device_path);
					/*
					 * For drivers that do not support the
					 * controller or are already connected
					 * we receive an error code here.
					 */
					if (r == EFI_SUCCESS)
						++connected;
				}
			}
		}
	}

	/*
	 * TODO: Some overrides are not yet implemented:
	 * - Platform Driver Override
	 * - Driver Family Override Search
	 * - Bus Specific Driver Override
	 */

	/* Driver Binding Search */
	for (i = 0; i < count; ++i) {
		if (buffer[i]) {
			r = efi_bind_controller(controller_handle,
						buffer[i],
						remain_device_path);
			if (r == EFI_SUCCESS)
				++connected;
		}
	}

	efi_free_pool(buffer);
	if (!connected)
		return EFI_NOT_FOUND;
	return EFI_SUCCESS;
}

/**
 * efi_connect_controller() - connect a controller to a driver
 * @controller_handle:   handle of the controller
 * @driver_image_handle: handle of the driver
 * @remain_device_path:  device path of a child controller
 * @recursive:           true to connect all child controllers
 *
 * This function implements the ConnectController service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * First all driver binding protocol handles are tried for binding drivers.
 * Afterwards all handles that have opened a protocol of the controller
 * with EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER are connected to drivers.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_connect_controller(
			efi_handle_t controller_handle,
			efi_handle_t *driver_image_handle,
			struct efi_device_path *remain_device_path,
			bool recursive)
{
	efi_status_t r;
	efi_status_t ret = EFI_NOT_FOUND;
	struct efi_object *efiobj;

	EFI_ENTRY("%p, %p, %pD, %d", controller_handle, driver_image_handle,
		  remain_device_path, recursive);

	efiobj = efi_search_obj(controller_handle);
	if (!efiobj) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	r = efi_connect_single_controller(controller_handle,
					  driver_image_handle,
					  remain_device_path);
	if (r == EFI_SUCCESS)
		ret = EFI_SUCCESS;
	if (recursive) {
		struct efi_handler *handler;
		struct efi_open_protocol_info_item *item;

		list_for_each_entry(handler, &efiobj->protocols, link) {
			list_for_each_entry(item, &handler->open_infos, link) {
				if (item->info.attributes &
				    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) {
					r = EFI_CALL(efi_connect_controller(
						item->info.controller_handle,
						driver_image_handle,
						remain_device_path,
						recursive));
					if (r == EFI_SUCCESS)
						ret = EFI_SUCCESS;
				}
			}
		}
	}
	/* Check for child controller specified by end node */
	if (ret != EFI_SUCCESS && remain_device_path &&
	    remain_device_path->type == DEVICE_PATH_TYPE_END)
		ret = EFI_SUCCESS;
out:
	return EFI_EXIT(ret);
}

/**
 * efi_reinstall_protocol_interface() - reinstall protocol interface
 * @handle:        handle on which the protocol shall be reinstalled
 * @protocol:      GUID of the protocol to be installed
 * @old_interface: interface to be removed
 * @new_interface: interface to be installed
 *
 * This function implements the ReinstallProtocolInterface service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * The old interface is uninstalled. The new interface is installed.
 * Drivers are connected.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_reinstall_protocol_interface(
			efi_handle_t handle, const efi_guid_t *protocol,
			void *old_interface, void *new_interface)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %pUs, %p, %p", handle, protocol, old_interface,
		  new_interface);

	/* Uninstall protocol but do not delete handle */
	ret = efi_uninstall_protocol(handle, protocol, old_interface, true);
	if (ret != EFI_SUCCESS)
		goto out;

	/* Install the new protocol */
	ret = efi_add_protocol(handle, protocol, new_interface);
	/*
	 * The UEFI spec does not specify what should happen to the handle
	 * if in case of an error no protocol interface remains on the handle.
	 * So let's do nothing here.
	 */
	if (ret != EFI_SUCCESS)
		goto out;
	/*
	 * The returned status code has to be ignored.
	 * Do not create an error if no suitable driver for the handle exists.
	 */
	EFI_CALL(efi_connect_controller(handle, NULL, NULL, true));
out:
	return EFI_EXIT(ret);
}

/**
 * efi_get_child_controllers() - get all child controllers associated to a driver
 * @efiobj:              handle of the controller
 * @driver_handle:       handle of the driver
 * @number_of_children:  number of child controllers
 * @child_handle_buffer: handles of the the child controllers
 *
 * The allocated buffer has to be freed with free().
 *
 * Return: status code
 */
static efi_status_t efi_get_child_controllers(
				struct efi_object *efiobj,
				efi_handle_t driver_handle,
				efi_uintn_t *number_of_children,
				efi_handle_t **child_handle_buffer)
{
	struct efi_handler *handler;
	struct efi_open_protocol_info_item *item;
	efi_uintn_t count = 0, i;
	bool duplicate;

	/* Count all child controller associations */
	list_for_each_entry(handler, &efiobj->protocols, link) {
		list_for_each_entry(item, &handler->open_infos, link) {
			if (item->info.agent_handle == driver_handle &&
			    item->info.attributes &
			    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER)
				++count;
		}
	}
	/*
	 * Create buffer. In case of duplicate child controller assignments
	 * the buffer will be too large. But that does not harm.
	 */
	*number_of_children = 0;
	if (!count)
		return EFI_SUCCESS;
	*child_handle_buffer = calloc(count, sizeof(efi_handle_t));
	if (!*child_handle_buffer)
		return EFI_OUT_OF_RESOURCES;
	/* Copy unique child handles */
	list_for_each_entry(handler, &efiobj->protocols, link) {
		list_for_each_entry(item, &handler->open_infos, link) {
			if (item->info.agent_handle == driver_handle &&
			    item->info.attributes &
			    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) {
				/* Check this is a new child controller */
				duplicate = false;
				for (i = 0; i < *number_of_children; ++i) {
					if ((*child_handle_buffer)[i] ==
					    item->info.controller_handle)
						duplicate = true;
				}
				/* Copy handle to buffer */
				if (!duplicate) {
					i = (*number_of_children)++;
					(*child_handle_buffer)[i] =
						item->info.controller_handle;
				}
			}
		}
	}
	return EFI_SUCCESS;
}

/**
 * efi_disconnect_controller() - disconnect a controller from a driver
 * @controller_handle:   handle of the controller
 * @driver_image_handle: handle of the driver
 * @child_handle:        handle of the child to destroy
 *
 * This function implements the DisconnectController service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_disconnect_controller(
				efi_handle_t controller_handle,
				efi_handle_t driver_image_handle,
				efi_handle_t child_handle)
{
	struct efi_driver_binding_protocol *binding_protocol;
	efi_handle_t *child_handle_buffer = NULL;
	size_t number_of_children = 0;
	efi_status_t r;
	struct efi_object *efiobj;
	bool sole_child;

	EFI_ENTRY("%p, %p, %p", controller_handle, driver_image_handle,
		  child_handle);

	efiobj = efi_search_obj(controller_handle);
	if (!efiobj) {
		r = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (child_handle && !efi_search_obj(child_handle)) {
		r = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* If no driver handle is supplied, disconnect all drivers */
	if (!driver_image_handle) {
		r = efi_disconnect_all_drivers(efiobj, NULL, child_handle);
		goto out;
	}

	/* Create list of child handles */
	r = efi_get_child_controllers(efiobj,
				      driver_image_handle,
				      &number_of_children,
				      &child_handle_buffer);
	if (r != EFI_SUCCESS)
		return r;
	sole_child = (number_of_children == 1);

	if (child_handle) {
		number_of_children = 1;
		free(child_handle_buffer);
		child_handle_buffer = &child_handle;
	}

	/* Get the driver binding protocol */
	r = EFI_CALL(efi_open_protocol(driver_image_handle,
				       &efi_guid_driver_binding_protocol,
				       (void **)&binding_protocol,
				       driver_image_handle, NULL,
				       EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	if (r != EFI_SUCCESS) {
		r = EFI_INVALID_PARAMETER;
		goto out;
	}
	/* Remove the children */
	if (number_of_children) {
		r = EFI_CALL(binding_protocol->stop(binding_protocol,
						    controller_handle,
						    number_of_children,
						    child_handle_buffer));
		if (r != EFI_SUCCESS) {
			r = EFI_DEVICE_ERROR;
			goto out;
		}
	}
	/* Remove the driver */
	if (!child_handle || sole_child) {
		r = EFI_CALL(binding_protocol->stop(binding_protocol,
						    controller_handle,
						    0, NULL));
		if (r != EFI_SUCCESS) {
			r = EFI_DEVICE_ERROR;
			goto out;
		}
	}
	efi_close_protocol(driver_image_handle,
			   &efi_guid_driver_binding_protocol,
			   driver_image_handle, NULL);
	r = EFI_SUCCESS;
out:
	if (!child_handle)
		free(child_handle_buffer);
	return EFI_EXIT(r);
}

static struct efi_boot_services efi_boot_services = {
	.hdr = {
		.signature = EFI_BOOT_SERVICES_SIGNATURE,
		.revision = EFI_SPECIFICATION_VERSION,
		.headersize = sizeof(struct efi_boot_services),
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
	.install_protocol_interface = efi_install_protocol_interface,
	.reinstall_protocol_interface = efi_reinstall_protocol_interface,
	.uninstall_protocol_interface = efi_uninstall_protocol_interface,
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
	.close_protocol = efi_close_protocol_ext,
	.open_protocol_information = efi_open_protocol_information,
	.protocols_per_handle = efi_protocols_per_handle,
	.locate_handle_buffer = efi_locate_handle_buffer,
	.locate_protocol = efi_locate_protocol,
	.install_multiple_protocol_interfaces =
			efi_install_multiple_protocol_interfaces_ext,
	.uninstall_multiple_protocol_interfaces =
			efi_uninstall_multiple_protocol_interfaces_ext,
	.calculate_crc32 = efi_calculate_crc32,
	.copy_mem = efi_copy_mem,
	.set_mem = efi_set_mem,
	.create_event_ex = efi_create_event_ex,
};

static u16 __efi_runtime_data firmware_vendor[] = u"Das U-Boot";

struct efi_system_table __efi_runtime_data systab = {
	.hdr = {
		.signature = EFI_SYSTEM_TABLE_SIGNATURE,
		.revision = EFI_SPECIFICATION_VERSION,
		.headersize = sizeof(struct efi_system_table),
	},
	.fw_vendor = firmware_vendor,
	.fw_revision = FW_VERSION << 16 | FW_PATCHLEVEL << 8,
	.runtime = &efi_runtime_services,
	.nr_tables = 0,
	.tables = NULL,
};

/**
 * efi_initialize_system_table() - Initialize system table
 *
 * Return:	status code
 */
efi_status_t efi_initialize_system_table(void)
{
	efi_status_t ret;

	/* Allocate configuration table array */
	ret = efi_allocate_pool(EFI_RUNTIME_SERVICES_DATA,
				EFI_MAX_CONFIGURATION_TABLES *
				sizeof(struct efi_configuration_table),
				(void **)&systab.tables);

	/*
	 * These entries will be set to NULL in ExitBootServices(). To avoid
	 * relocation in SetVirtualAddressMap(), set them dynamically.
	 */
	systab.con_in_handle = efi_root;
	systab.con_in = &efi_con_in;
	systab.con_out_handle = efi_root;
	systab.con_out = &efi_con_out;
	systab.stderr_handle = efi_root;
	systab.std_err = &efi_con_out;
	systab.boottime = &efi_boot_services;

	/* Set CRC32 field in table headers */
	efi_update_table_header_crc32(&systab.hdr);
	efi_update_table_header_crc32(&efi_runtime_services.hdr);
	efi_update_table_header_crc32(&efi_boot_services.hdr);

	return ret;
}
