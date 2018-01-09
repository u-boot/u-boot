/*
 *  EFI application boot time services
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <div64.h>
#include <efi_loader.h>
#include <environment.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <libfdt_env.h>
#include <u-boot/crc.h>
#include <bootm.h>
#include <inttypes.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

/* Task priority level */
static efi_uintn_t efi_tpl = TPL_APPLICATION;

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

/*
 * Queue an EFI event.
 *
 * This function queues the notification function of the event for future
 * execution.
 *
 * The notification function is called if the task priority level of the
 * event is higher than the current task priority level.
 *
 * For the SignalEvent service see efi_signal_event_ext.
 *
 * @event	event to signal
 */
void efi_signal_event(struct efi_event *event)
{
	if (event->notify_function) {
		event->is_queued = true;
		/* Check TPL */
		if (efi_tpl >= event->notify_tpl)
			return;
		EFI_CALL_VOID(event->notify_function(event,
						     event->notify_context));
	}
	event->is_queued = false;
}

/*
 * Raise the task priority level.
 *
 * This function implements the RaiseTpl service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @new_tpl	new value of the task priority level
 * @return	old value of the task priority level
 */
static unsigned long EFIAPI efi_raise_tpl(efi_uintn_t new_tpl)
{
	efi_uintn_t old_tpl = efi_tpl;

	EFI_ENTRY("0x%zx", new_tpl);

	if (new_tpl < efi_tpl)
		debug("WARNING: new_tpl < current_tpl in %s\n", __func__);
	efi_tpl = new_tpl;
	if (efi_tpl > TPL_HIGH_LEVEL)
		efi_tpl = TPL_HIGH_LEVEL;

	EFI_EXIT(EFI_SUCCESS);
	return old_tpl;
}

/*
 * Lower the task priority level.
 *
 * This function implements the RestoreTpl service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @old_tpl	value of the task priority level to be restored
 */
static void EFIAPI efi_restore_tpl(efi_uintn_t old_tpl)
{
	EFI_ENTRY("0x%zx", old_tpl);

	if (old_tpl > efi_tpl)
		debug("WARNING: old_tpl > current_tpl in %s\n", __func__);
	efi_tpl = old_tpl;
	if (efi_tpl > TPL_HIGH_LEVEL)
		efi_tpl = TPL_HIGH_LEVEL;

	EFI_EXIT(EFI_SUCCESS);
}

/*
 * Allocate memory pages.
 *
 * This function implements the AllocatePages service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @type		type of allocation to be performed
 * @memory_type		usage type of the allocated memory
 * @pages		number of pages to be allocated
 * @memory		allocated memory
 * @return		status code
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

/*
 * Free memory pages.
 *
 * This function implements the FreePages service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @memory	start of the memory area to be freed
 * @pages	number of pages to be freed
 * @return	status code
 */
static efi_status_t EFIAPI efi_free_pages_ext(uint64_t memory,
					      efi_uintn_t pages)
{
	efi_status_t r;

	EFI_ENTRY("%"PRIx64", 0x%zx", memory, pages);
	r = efi_free_pages(memory, pages);
	return EFI_EXIT(r);
}

/*
 * Get map describing memory usage.
 *
 * This function implements the GetMemoryMap service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @memory_map_size	on entry the size, in bytes, of the memory map buffer,
 *			on exit the size of the copied memory map
 * @memory_map		buffer to which the memory map is written
 * @map_key		key for the memory map
 * @descriptor_size	size of an individual memory descriptor
 * @descriptor_version	version number of the memory descriptor structure
 * @return		status code
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

/*
 * Allocate memory from pool.
 *
 * This function implements the AllocatePool service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @pool_type	type of the pool from which memory is to be allocated
 * @size	number of bytes to be allocated
 * @buffer	allocated memory
 * @return	status code
 */
static efi_status_t EFIAPI efi_allocate_pool_ext(int pool_type,
						 efi_uintn_t size,
						 void **buffer)
{
	efi_status_t r;

	EFI_ENTRY("%d, %zd, %p", pool_type, size, buffer);
	r = efi_allocate_pool(pool_type, size, buffer);
	return EFI_EXIT(r);
}

/*
 * Free memory from pool.
 *
 * This function implements the FreePool service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @buffer	start of memory to be freed
 * @return	status code
 */
static efi_status_t EFIAPI efi_free_pool_ext(void *buffer)
{
	efi_status_t r;

	EFI_ENTRY("%p", buffer);
	r = efi_free_pool(buffer);
	return EFI_EXIT(r);
}

/*
 * Add a new object to the object list.
 *
 * The protocols list is initialized.
 * The object handle is set.
 *
 * @obj	object to be added
 */
void efi_add_handle(struct efi_object *obj)
{
	if (!obj)
		return;
	INIT_LIST_HEAD(&obj->protocols);
	obj->handle = obj;
	list_add_tail(&obj->link, &efi_obj_list);
}

/*
 * Create handle.
 *
 * @handle	new handle
 * @return	status code
 */
efi_status_t efi_create_handle(void **handle)
{
	struct efi_object *obj;
	efi_status_t r;

	r = efi_allocate_pool(EFI_ALLOCATE_ANY_PAGES,
			      sizeof(struct efi_object),
			      (void **)&obj);
	if (r != EFI_SUCCESS)
		return r;
	efi_add_handle(obj);
	*handle = obj->handle;
	return r;
}

/*
 * Find a protocol on a handle.
 *
 * @handle		handle
 * @protocol_guid	GUID of the protocol
 * @handler		reference to the protocol
 * @return		status code
 */
efi_status_t efi_search_protocol(const void *handle,
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
		if (!guidcmp(protocol->guid, protocol_guid)) {
			if (handler)
				*handler = protocol;
			return EFI_SUCCESS;
		}
	}
	return EFI_NOT_FOUND;
}

/*
 * Delete protocol from a handle.
 *
 * @handle			handle from which the protocol shall be deleted
 * @protocol			GUID of the protocol to be deleted
 * @protocol_interface		interface of the protocol implementation
 * @return			status code
 */
efi_status_t efi_remove_protocol(const void *handle, const efi_guid_t *protocol,
				 void *protocol_interface)
{
	struct efi_handler *handler;
	efi_status_t ret;

	ret = efi_search_protocol(handle, protocol, &handler);
	if (ret != EFI_SUCCESS)
		return ret;
	if (guidcmp(handler->guid, protocol))
		return EFI_INVALID_PARAMETER;
	list_del(&handler->link);
	free(handler);
	return EFI_SUCCESS;
}

/*
 * Delete all protocols from a handle.
 *
 * @handle	handle from which the protocols shall be deleted
 * @return	status code
 */
efi_status_t efi_remove_all_protocols(const void *handle)
{
	struct efi_object *efiobj;
	struct list_head *lhandle;
	struct list_head *pos;

	efiobj = efi_search_obj(handle);
	if (!efiobj)
		return EFI_INVALID_PARAMETER;
	list_for_each_safe(lhandle, pos, &efiobj->protocols) {
		struct efi_handler *protocol;
		efi_status_t ret;

		protocol = list_entry(lhandle, struct efi_handler, link);

		ret = efi_remove_protocol(handle, protocol->guid,
					  protocol->protocol_interface);
		if (ret != EFI_SUCCESS)
			return ret;
	}
	return EFI_SUCCESS;
}

/*
 * Delete handle.
 *
 * @handle	handle to delete
 */
void efi_delete_handle(struct efi_object *obj)
{
	if (!obj)
		return;
	efi_remove_all_protocols(obj->handle);
	list_del(&obj->link);
	free(obj);
}

/*
 * Our event capabilities are very limited. Only a small limited
 * number of events is allowed to coexist.
 */
static struct efi_event efi_events[16];

/*
 * Create an event.
 *
 * This function is used inside U-Boot code to create an event.
 *
 * For the API function implementing the CreateEvent service see
 * efi_create_event_ext.
 *
 * @type		type of the event to create
 * @notify_tpl		task priority level of the event
 * @notify_function	notification function of the event
 * @notify_context	pointer passed to the notification function
 * @event		created event
 * @return		status code
 */
efi_status_t efi_create_event(uint32_t type, efi_uintn_t notify_tpl,
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
		efi_events[i].is_queued = false;
		efi_events[i].is_signaled = false;
		*event = &efi_events[i];
		return EFI_SUCCESS;
	}
	return EFI_OUT_OF_RESOURCES;
}

/*
 * Create an event.
 *
 * This function implements the CreateEvent service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @type		type of the event to create
 * @notify_tpl		task priority level of the event
 * @notify_function	notification function of the event
 * @notify_context	pointer passed to the notification function
 * @event		created event
 * @return		status code
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
					 notify_context, event));
}


/*
 * Check if a timer event has occurred or a queued notification function should
 * be called.
 *
 * Our timers have to work without interrupts, so we check whenever keyboard
 * input or disk accesses happen if enough time elapsed for them to fire.
 */
void efi_timer_check(void)
{
	int i;
	u64 now = timer_get_us();

	for (i = 0; i < ARRAY_SIZE(efi_events); ++i) {
		if (!efi_events[i].type)
			continue;
		if (efi_events[i].is_queued)
			efi_signal_event(&efi_events[i]);
		if (!(efi_events[i].type & EVT_TIMER) ||
		    now < efi_events[i].trigger_next)
			continue;
		switch (efi_events[i].trigger_type) {
		case EFI_TIMER_RELATIVE:
			efi_events[i].trigger_type = EFI_TIMER_STOP;
			break;
		case EFI_TIMER_PERIODIC:
			efi_events[i].trigger_next +=
				efi_events[i].trigger_time;
			break;
		default:
			continue;
		}
		efi_events[i].is_signaled = true;
		efi_signal_event(&efi_events[i]);
	}
	WATCHDOG_RESET();
}

/*
 * Set the trigger time for a timer event or stop the event.
 *
 * This is the function for internal usage in U-Boot. For the API function
 * implementing the SetTimer service see efi_set_timer_ext.
 *
 * @event		event for which the timer is set
 * @type		type of the timer
 * @trigger_time	trigger period in multiples of 100ns
 * @return		status code
 */
efi_status_t efi_set_timer(struct efi_event *event, enum efi_timer_delay type,
			   uint64_t trigger_time)
{
	int i;

	/*
	 * The parameter defines a multiple of 100ns.
	 * We use multiples of 1000ns. So divide by 10.
	 */
	do_div(trigger_time, 10);

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
		event->is_signaled = false;
		return EFI_SUCCESS;
	}
	return EFI_INVALID_PARAMETER;
}

/*
 * Set the trigger time for a timer event or stop the event.
 *
 * This function implements the SetTimer service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @event		event for which the timer is set
 * @type		type of the timer
 * @trigger_time	trigger period in multiples of 100ns
 * @return		status code
 */
static efi_status_t EFIAPI efi_set_timer_ext(struct efi_event *event,
					     enum efi_timer_delay type,
					     uint64_t trigger_time)
{
	EFI_ENTRY("%p, %d, %"PRIx64, event, type, trigger_time);
	return EFI_EXIT(efi_set_timer(event, type, trigger_time));
}

/*
 * Wait for events to be signaled.
 *
 * This function implements the WaitForEvent service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @num_events	number of events to be waited for
 * @events	events to be waited for
 * @index	index of the event that was signaled
 * @return	status code
 */
static efi_status_t EFIAPI efi_wait_for_event(efi_uintn_t num_events,
					      struct efi_event **event,
					      efi_uintn_t *index)
{
	int i, j;

	EFI_ENTRY("%zd, %p, %p", num_events, event, index);

	/* Check parameters */
	if (!num_events || !event)
		return EFI_EXIT(EFI_INVALID_PARAMETER);
	/* Check TPL */
	if (efi_tpl != TPL_APPLICATION)
		return EFI_EXIT(EFI_UNSUPPORTED);
	for (i = 0; i < num_events; ++i) {
		for (j = 0; j < ARRAY_SIZE(efi_events); ++j) {
			if (event[i] == &efi_events[j])
				goto known_event;
		}
		return EFI_EXIT(EFI_INVALID_PARAMETER);
known_event:
		if (!event[i]->type || event[i]->type & EVT_NOTIFY_SIGNAL)
			return EFI_EXIT(EFI_INVALID_PARAMETER);
		if (!event[i]->is_signaled)
			efi_signal_event(event[i]);
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

/*
 * Signal an EFI event.
 *
 * This function implements the SignalEvent service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * This functions sets the signaled state of the event and queues the
 * notification function for execution.
 *
 * @event	event to signal
 * @return	status code
 */
static efi_status_t EFIAPI efi_signal_event_ext(struct efi_event *event)
{
	int i;

	EFI_ENTRY("%p", event);
	for (i = 0; i < ARRAY_SIZE(efi_events); ++i) {
		if (event != &efi_events[i])
			continue;
		if (event->is_signaled)
			break;
		event->is_signaled = true;
		if (event->type & EVT_NOTIFY_SIGNAL)
			efi_signal_event(event);
		break;
	}
	return EFI_EXIT(EFI_SUCCESS);
}

/*
 * Close an EFI event.
 *
 * This function implements the CloseEvent service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @event	event to close
 * @return	status code
 */
static efi_status_t EFIAPI efi_close_event(struct efi_event *event)
{
	int i;

	EFI_ENTRY("%p", event);
	for (i = 0; i < ARRAY_SIZE(efi_events); ++i) {
		if (event == &efi_events[i]) {
			event->type = 0;
			event->trigger_next = -1ULL;
			event->is_queued = false;
			event->is_signaled = false;
			return EFI_EXIT(EFI_SUCCESS);
		}
	}
	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

/*
 * Check if an event is signaled.
 *
 * This function implements the CheckEvent service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * If an event is not signaled yet the notification function is queued.
 *
 * @event	event to check
 * @return	status code
 */
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
		if (!event->is_signaled)
			efi_signal_event(event);
		if (event->is_signaled)
			return EFI_EXIT(EFI_SUCCESS);
		return EFI_EXIT(EFI_NOT_READY);
	}
	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

/*
 * Find the internal EFI object for a handle.
 *
 * @handle	handle to find
 * @return	EFI object
 */
struct efi_object *efi_search_obj(const void *handle)
{
	struct efi_object *efiobj;

	list_for_each_entry(efiobj, &efi_obj_list, link) {
		if (efiobj->handle == handle)
			return efiobj;
	}

	return NULL;
}

/*
 * Install new protocol on a handle.
 *
 * @handle			handle on which the protocol shall be installed
 * @protocol			GUID of the protocol to be installed
 * @protocol_interface		interface of the protocol implementation
 * @return			status code
 */
efi_status_t efi_add_protocol(const void *handle, const efi_guid_t *protocol,
			      void *protocol_interface)
{
	struct efi_object *efiobj;
	struct efi_handler *handler;
	efi_status_t ret;

	efiobj = efi_search_obj(handle);
	if (!efiobj)
		return EFI_INVALID_PARAMETER;
	ret = efi_search_protocol(handle, protocol, NULL);
	if (ret != EFI_NOT_FOUND)
		return EFI_INVALID_PARAMETER;
	handler = calloc(1, sizeof(struct efi_handler));
	if (!handler)
		return EFI_OUT_OF_RESOURCES;
	handler->guid = protocol;
	handler->protocol_interface = protocol_interface;
	list_add_tail(&handler->link, &efiobj->protocols);
	return EFI_SUCCESS;
}

/*
 * Install protocol interface.
 *
 * This function implements the InstallProtocolInterface service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @handle			handle on which the protocol shall be installed
 * @protocol			GUID of the protocol to be installed
 * @protocol_interface_type	type of the interface to be installed,
 *				always EFI_NATIVE_INTERFACE
 * @protocol_interface		interface of the protocol implementation
 * @return			status code
 */
static efi_status_t EFIAPI efi_install_protocol_interface(
			void **handle, const efi_guid_t *protocol,
			int protocol_interface_type, void *protocol_interface)
{
	efi_status_t r;

	EFI_ENTRY("%p, %pUl, %d, %p", handle, protocol, protocol_interface_type,
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
		debug("%sEFI: new handle %p\n", indent_string(nesting_level),
		      *handle);
	} else {
		debug("%sEFI: handle %p\n", indent_string(nesting_level),
		      *handle);
	}
	/* Add new protocol */
	r = efi_add_protocol(*handle, protocol, protocol_interface);
out:
	return EFI_EXIT(r);
}

/*
 * Reinstall protocol interface.
 *
 * This function implements the ReinstallProtocolInterface service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @handle			handle on which the protocol shall be
 *				reinstalled
 * @protocol			GUID of the protocol to be installed
 * @old_interface		interface to be removed
 * @new_interface		interface to be installed
 * @return			status code
 */
static efi_status_t EFIAPI efi_reinstall_protocol_interface(void *handle,
			const efi_guid_t *protocol, void *old_interface,
			void *new_interface)
{
	EFI_ENTRY("%p, %pUl, %p, %p", handle, protocol, old_interface,
		  new_interface);
	return EFI_EXIT(EFI_ACCESS_DENIED);
}

/*
 * Uninstall protocol interface.
 *
 * This function implements the UninstallProtocolInterface service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @handle			handle from which the protocol shall be removed
 * @protocol			GUID of the protocol to be removed
 * @protocol_interface		interface to be removed
 * @return			status code
 */
static efi_status_t EFIAPI efi_uninstall_protocol_interface(
				void *handle, const efi_guid_t *protocol,
				void *protocol_interface)
{
	struct efi_handler *handler;
	efi_status_t r;

	EFI_ENTRY("%p, %pUl, %p", handle, protocol, protocol_interface);

	if (!handle || !protocol) {
		r = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Find the protocol on the handle */
	r = efi_search_protocol(handle, protocol, &handler);
	if (r != EFI_SUCCESS)
		goto out;
	if (handler->protocol_interface) {
		/* TODO disconnect controllers */
		r =  EFI_ACCESS_DENIED;
	} else {
		r = efi_remove_protocol(handle, protocol, protocol_interface);
	}
out:
	return EFI_EXIT(r);
}

/*
 * Register an event for notification when a protocol is installed.
 *
 * This function implements the RegisterProtocolNotify service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @protocol		GUID of the protocol whose installation shall be
 *			notified
 * @event		event to be signaled upon installation of the protocol
 * @registration	key for retrieving the registration information
 * @return		status code
 */
static efi_status_t EFIAPI efi_register_protocol_notify(
						const efi_guid_t *protocol,
						struct efi_event *event,
						void **registration)
{
	EFI_ENTRY("%pUl, %p, %p", protocol, event, registration);
	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
}

/*
 * Determine if an EFI handle implements a protocol.
 *
 * See the documentation of the LocateHandle service in the UEFI specification.
 *
 * @search_type		selection criterion
 * @protocol		GUID of the protocol
 * @search_key		registration key
 * @efiobj		handle
 * @return		0 if the handle implements the protocol
 */
static int efi_search(enum efi_locate_search_type search_type,
		      const efi_guid_t *protocol, void *search_key,
		      struct efi_object *efiobj)
{
	efi_status_t ret;

	switch (search_type) {
	case ALL_HANDLES:
		return 0;
	case BY_REGISTER_NOTIFY:
		/* TODO: RegisterProtocolNotify is not implemented yet */
		return -1;
	case BY_PROTOCOL:
		ret = efi_search_protocol(efiobj->handle, protocol, NULL);
		return (ret != EFI_SUCCESS);
	default:
		/* Invalid search type */
		return -1;
	}
}

/*
 * Locate handles implementing a protocol.
 *
 * This function is meant for U-Boot internal calls. For the API implementation
 * of the LocateHandle service see efi_locate_handle_ext.
 *
 * @search_type		selection criterion
 * @protocol		GUID of the protocol
 * @search_key		registration key
 * @buffer_size		size of the buffer to receive the handles in bytes
 * @buffer		buffer to receive the relevant handles
 * @return		status code
 */
static efi_status_t efi_locate_handle(
			enum efi_locate_search_type search_type,
			const efi_guid_t *protocol, void *search_key,
			efi_uintn_t *buffer_size, efi_handle_t *buffer)
{
	struct efi_object *efiobj;
	efi_uintn_t size = 0;

	/* Check parameters */
	switch (search_type) {
	case ALL_HANDLES:
		break;
	case BY_REGISTER_NOTIFY:
		if (!search_key)
			return EFI_INVALID_PARAMETER;
		/* RegisterProtocolNotify is not implemented yet */
		return EFI_UNSUPPORTED;
	case BY_PROTOCOL:
		if (!protocol)
			return EFI_INVALID_PARAMETER;
		break;
	default:
		return EFI_INVALID_PARAMETER;
	}

	/*
	 * efi_locate_handle_buffer uses this function for
	 * the calculation of the necessary buffer size.
	 * So do not require a buffer for buffersize == 0.
	 */
	if (!buffer_size || (*buffer_size && !buffer))
		return EFI_INVALID_PARAMETER;

	/* Count how much space we need */
	list_for_each_entry(efiobj, &efi_obj_list, link) {
		if (!efi_search(search_type, protocol, search_key, efiobj))
			size += sizeof(void*);
	}

	if (*buffer_size < size) {
		*buffer_size = size;
		return EFI_BUFFER_TOO_SMALL;
	}

	*buffer_size = size;
	if (size == 0)
		return EFI_NOT_FOUND;

	/* Then fill the array */
	list_for_each_entry(efiobj, &efi_obj_list, link) {
		if (!efi_search(search_type, protocol, search_key, efiobj))
			*buffer++ = efiobj->handle;
	}

	return EFI_SUCCESS;
}

/*
 * Locate handles implementing a protocol.
 *
 * This function implements the LocateHandle service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @search_type		selection criterion
 * @protocol		GUID of the protocol
 * @search_key		registration key
 * @buffer_size		size of the buffer to receive the handles in bytes
 * @buffer		buffer to receive the relevant handles
 * @return		0 if the handle implements the protocol
 */
static efi_status_t EFIAPI efi_locate_handle_ext(
			enum efi_locate_search_type search_type,
			const efi_guid_t *protocol, void *search_key,
			efi_uintn_t *buffer_size, efi_handle_t *buffer)
{
	EFI_ENTRY("%d, %pUl, %p, %p, %p", search_type, protocol, search_key,
		  buffer_size, buffer);

	return EFI_EXIT(efi_locate_handle(search_type, protocol, search_key,
			buffer_size, buffer));
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

/*
 * Adds, updates, or removes a configuration table.
 *
 * This function is used for internal calls. For the API implementation of the
 * InstallConfigurationTable service see efi_install_configuration_table_ext.
 *
 * @guid		GUID of the installed table
 * @table		table to be installed
 * @return		status code
 */
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

/*
 * Adds, updates, or removes a configuration table.
 *
 * This function implements the InstallConfigurationTable service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @guid		GUID of the installed table
 * @table		table to be installed
 * @return		status code
 */
static efi_status_t EFIAPI efi_install_configuration_table_ext(efi_guid_t *guid,
							       void *table)
{
	EFI_ENTRY("%pUl, %p", guid, table);
	return EFI_EXIT(efi_install_configuration_table(guid, table));
}

/*
 * Initialize a loaded_image_info + loaded_image_info object with correct
 * protocols, boot-device, etc.
 *
 * @info		loaded image info to be passed to the entry point of the
 *			image
 * @obj			internal object associated with the loaded image
 * @device_path		device path of the loaded image
 * @file_path		file path of the loaded image
 * @return		status code
 */
efi_status_t efi_setup_loaded_image(
			struct efi_loaded_image *info, struct efi_object *obj,
			struct efi_device_path *device_path,
			struct efi_device_path *file_path)
{
	efi_status_t ret;

	/* Add internal object to object list */
	efi_add_handle(obj);
	/* efi_exit() assumes that the handle points to the info */
	obj->handle = info;

	info->file_path = file_path;
	if (device_path)
		info->device_handle = efi_dp_find_obj(device_path, NULL);

	/*
	 * When asking for the device path interface, return
	 * bootefi_device_path
	 */
	ret = efi_add_protocol(obj->handle, &efi_guid_device_path, device_path);
	if (ret != EFI_SUCCESS)
		goto failure;

	/*
	 * When asking for the loaded_image interface, just
	 * return handle which points to loaded_image_info
	 */
	ret = efi_add_protocol(obj->handle, &efi_guid_loaded_image, info);
	if (ret != EFI_SUCCESS)
		goto failure;

	ret = efi_add_protocol(obj->handle, &efi_guid_console_control,
			       (void *)&efi_console_control);
	if (ret != EFI_SUCCESS)
		goto failure;

	ret = efi_add_protocol(obj->handle,
			       &efi_guid_device_path_to_text_protocol,
			       (void *)&efi_device_path_to_text);
	if (ret != EFI_SUCCESS)
		goto failure;

	return ret;
failure:
	printf("ERROR: Failure to install protocols for loaded image\n");
	return ret;
}

/*
 * Load an image using a file path.
 *
 * @file_path		the path of the image to load
 * @buffer		buffer containing the loaded image
 * @return		status code
 */
efi_status_t efi_load_image_from_path(struct efi_device_path *file_path,
				      void **buffer)
{
	struct efi_file_info *info = NULL;
	struct efi_file_handle *f;
	static efi_status_t ret;
	uint64_t bs;

	f = efi_file_from_path(file_path);
	if (!f)
		return EFI_DEVICE_ERROR;

	bs = 0;
	EFI_CALL(ret = f->getinfo(f, (efi_guid_t *)&efi_file_info_guid,
				  &bs, info));
	if (ret == EFI_BUFFER_TOO_SMALL) {
		info = malloc(bs);
		EFI_CALL(ret = f->getinfo(f, (efi_guid_t *)&efi_file_info_guid,
					  &bs, info));
	}
	if (ret != EFI_SUCCESS)
		goto error;

	ret = efi_allocate_pool(EFI_LOADER_DATA, info->file_size, buffer);
	if (ret)
		goto error;

	EFI_CALL(ret = f->read(f, &info->file_size, *buffer));

error:
	free(info);
	EFI_CALL(f->close(f));

	if (ret != EFI_SUCCESS) {
		efi_free_pool(*buffer);
		*buffer = NULL;
	}

	return ret;
}

/*
 * Load an EFI image into memory.
 *
 * This function implements the LoadImage service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @boot_policy		true for request originating from the boot manager
 * @parent_image	the calles's image handle
 * @file_path		the path of the image to load
 * @source_buffer	memory location from which the image is installed
 * @source_size		size of the memory area from which the image is
 *			installed
 * @image_handle	handle for the newly installed image
 * @return		status code
 */
static efi_status_t EFIAPI efi_load_image(bool boot_policy,
					  efi_handle_t parent_image,
					  struct efi_device_path *file_path,
					  void *source_buffer,
					  unsigned long source_size,
					  efi_handle_t *image_handle)
{
	struct efi_loaded_image *info;
	struct efi_object *obj;
	efi_status_t ret;

	EFI_ENTRY("%d, %p, %p, %p, %ld, %p", boot_policy, parent_image,
		  file_path, source_buffer, source_size, image_handle);

	info = calloc(1, sizeof(*info));
	obj = calloc(1, sizeof(*obj));

	if (!source_buffer) {
		struct efi_device_path *dp, *fp;

		ret = efi_load_image_from_path(file_path, &source_buffer);
		if (ret != EFI_SUCCESS)
			goto failure;
		/*
		 * split file_path which contains both the device and
		 * file parts:
		 */
		efi_dp_split_file_path(file_path, &dp, &fp);
		ret = efi_setup_loaded_image(info, obj, dp, fp);
		if (ret != EFI_SUCCESS)
			goto failure;
	} else {
		/* In this case, file_path is the "device" path, ie.
		 * something like a HARDWARE_DEVICE:MEMORY_MAPPED
		 */
		ret = efi_setup_loaded_image(info, obj, file_path, NULL);
		if (ret != EFI_SUCCESS)
			goto failure;
	}
	info->reserved = efi_load_pe(source_buffer, info);
	if (!info->reserved) {
		ret = EFI_UNSUPPORTED;
		goto failure;
	}
	info->system_table = &systab;
	info->parent_handle = parent_image;
	*image_handle = obj->handle;
	return EFI_EXIT(EFI_SUCCESS);
failure:
	free(info);
	efi_delete_handle(obj);
	return EFI_EXIT(ret);
}

/*
 * Call the entry point of an image.
 *
 * This function implements the StartImage service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @image_handle	handle of the image
 * @exit_data_size	size of the buffer
 * @exit_data		buffer to receive the exit data of the called image
 * @return		status code
 */
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

/*
 * Leave an EFI application or driver.
 *
 * This function implements the Exit service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @image_handle	handle of the application or driver that is exiting
 * @exit_status		status code
 * @exit_data_size	size of the buffer in bytes
 * @exit_data		buffer with data describing an error
 * @return		status code
 */
static efi_status_t EFIAPI efi_exit(efi_handle_t image_handle,
			efi_status_t exit_status, unsigned long exit_data_size,
			int16_t *exit_data)
{
	/*
	 * We require that the handle points to the original loaded
	 * image protocol interface.
	 *
	 * For getting the longjmp address this is safer than locating
	 * the protocol because the protocol may have been reinstalled
	 * pointing to another memory location.
	 *
	 * TODO: We should call the unload procedure of the loaded
	 *	 image protocol.
	 */
	struct efi_loaded_image *loaded_image_info = (void*)image_handle;

	EFI_ENTRY("%p, %ld, %ld, %p", image_handle, exit_status,
		  exit_data_size, exit_data);

	/* Make sure entry/exit counts for EFI world cross-overs match */
	__efi_exit_check();

	/*
	 * But longjmp out with the U-Boot gd, not the application's, as
	 * the other end is a setjmp call inside EFI context.
	 */
	efi_restore_gd();

	loaded_image_info->exit_status = exit_status;
	longjmp(&loaded_image_info->exit_jmp, 1);

	panic("EFI application exited");
}

/*
 * Unload an EFI image.
 *
 * This function implements the UnloadImage service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @image_handle	handle of the image to be unloaded
 * @return		status code
 */
static efi_status_t EFIAPI efi_unload_image(void *image_handle)
{
	struct efi_object *efiobj;

	EFI_ENTRY("%p", image_handle);
	efiobj = efi_search_obj(image_handle);
	if (efiobj)
		list_del(&efiobj->link);

	return EFI_EXIT(EFI_SUCCESS);
}

/*
 * Fix up caches for EFI payloads if necessary.
 */
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

/*
 * Stop boot services.
 *
 * This function implements the ExitBootServices service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @image_handle	handle of the loaded image
 * @map_key		key of the memory map
 * @return		status code
 */
static efi_status_t EFIAPI efi_exit_boot_services(void *image_handle,
						  unsigned long map_key)
{
	int i;

	EFI_ENTRY("%p, %ld", image_handle, map_key);

	/* Notify that ExitBootServices is invoked. */
	for (i = 0; i < ARRAY_SIZE(efi_events); ++i) {
		if (efi_events[i].type != EVT_SIGNAL_EXIT_BOOT_SERVICES)
			continue;
		efi_signal_event(&efi_events[i]);
	}
	/* Make sure that notification functions are not called anymore */
	efi_tpl = TPL_HIGH_LEVEL;

	/* XXX Should persist EFI variables here */

	board_quiesce_devices();

	/* Fix up caches for EFI payloads if necessary */
	efi_exit_caches();

	/* This stops all lingering devices */
	bootm_disable_interrupts();

	/* Give the payload some time to boot */
	efi_set_watchdog(0);
	WATCHDOG_RESET();

	return EFI_EXIT(EFI_SUCCESS);
}

/*
 * Get next value of the counter.
 *
 * This function implements the NextMonotonicCount service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @count	returned value of the counter
 * @return	status code
 */
static efi_status_t EFIAPI efi_get_next_monotonic_count(uint64_t *count)
{
	static uint64_t mono = 0;
	EFI_ENTRY("%p", count);
	*count = mono++;
	return EFI_EXIT(EFI_SUCCESS);
}

/*
 * Sleep.
 *
 * This function implements the Stall sercive.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @microseconds	period to sleep in microseconds
 * @return		status code
 */
static efi_status_t EFIAPI efi_stall(unsigned long microseconds)
{
	EFI_ENTRY("%ld", microseconds);
	udelay(microseconds);
	return EFI_EXIT(EFI_SUCCESS);
}

/*
 * Reset the watchdog timer.
 *
 * This function implements the SetWatchdogTimer service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @timeout		seconds before reset by watchdog
 * @watchdog_code	code to be logged when resetting
 * @data_size		size of buffer in bytes
 * @watchdog_data	buffer with data describing the reset reason
 * @return		status code
 */
static efi_status_t EFIAPI efi_set_watchdog_timer(unsigned long timeout,
						  uint64_t watchdog_code,
						  unsigned long data_size,
						  uint16_t *watchdog_data)
{
	EFI_ENTRY("%ld, 0x%"PRIx64", %ld, %p", timeout, watchdog_code,
		  data_size, watchdog_data);
	return EFI_EXIT(efi_set_watchdog(timeout));
}

/*
 * Connect a controller to a driver.
 *
 * This function implements the ConnectController service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @controller_handle	handle of the controller
 * @driver_image_handle	handle of the driver
 * @remain_device_path	device path of a child controller
 * @recursive		true to connect all child controllers
 * @return		status code
 */
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

/*
 * Disconnect a controller from a driver.
 *
 * This function implements the DisconnectController service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @controller_handle	handle of the controller
 * @driver_image_handle handle of the driver
 * @child_handle	handle of the child to destroy
 * @return		status code
 */
static efi_status_t EFIAPI efi_disconnect_controller(void *controller_handle,
						     void *driver_image_handle,
						     void *child_handle)
{
	EFI_ENTRY("%p, %p, %p", controller_handle, driver_image_handle,
		  child_handle);
	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

/*
 * Close a protocol.
 *
 * This function implements the CloseProtocol service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @handle		handle on which the protocol shall be closed
 * @protocol		GUID of the protocol to close
 * @agent_handle	handle of the driver
 * @controller_handle	handle of the controller
 * @return		status code
 */
static efi_status_t EFIAPI efi_close_protocol(void *handle,
					      const efi_guid_t *protocol,
					      void *agent_handle,
					      void *controller_handle)
{
	EFI_ENTRY("%p, %pUl, %p, %p", handle, protocol, agent_handle,
		  controller_handle);
	return EFI_EXIT(EFI_NOT_FOUND);
}

/*
 * Provide information about then open status of a protocol on a handle
 *
 * This function implements the OpenProtocolInformation service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @handle		handle for which the information shall be retrieved
 * @protocol		GUID of the protocol
 * @entry_buffer	buffer to receive the open protocol information
 * @entry_count		number of entries available in the buffer
 * @return		status code
 */
static efi_status_t EFIAPI efi_open_protocol_information(efi_handle_t handle,
			const efi_guid_t *protocol,
			struct efi_open_protocol_info_entry **entry_buffer,
			efi_uintn_t *entry_count)
{
	EFI_ENTRY("%p, %pUl, %p, %p", handle, protocol, entry_buffer,
		  entry_count);
	return EFI_EXIT(EFI_NOT_FOUND);
}

/*
 * Get protocols installed on a handle.
 *
 * This function implements the ProtocolsPerHandleService.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @handle			handle for which the information is retrieved
 * @protocol_buffer		buffer with protocol GUIDs
 * @protocol_buffer_count	number of entries in the buffer
 * @return			status code
 */
static efi_status_t EFIAPI efi_protocols_per_handle(void *handle,
			efi_guid_t ***protocol_buffer,
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
	*protocol_buffer_count = 0;

	efiobj = efi_search_obj(handle);
	if (!efiobj)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	/* Count protocols */
	list_for_each(protocol_handle, &efiobj->protocols) {
		++*protocol_buffer_count;
	}

	/* Copy guids */
	if (*protocol_buffer_count) {
		size_t j = 0;

		buffer_size = sizeof(efi_guid_t *) * *protocol_buffer_count;
		r = efi_allocate_pool(EFI_ALLOCATE_ANY_PAGES, buffer_size,
				      (void **)protocol_buffer);
		if (r != EFI_SUCCESS)
			return EFI_EXIT(r);
		list_for_each(protocol_handle, &efiobj->protocols) {
			struct efi_handler *protocol;

			protocol = list_entry(protocol_handle,
					      struct efi_handler, link);
			(*protocol_buffer)[j] = (void *)protocol->guid;
			++j;
		}
	}

	return EFI_EXIT(EFI_SUCCESS);
}

/*
 * Locate handles implementing a protocol.
 *
 * This function implements the LocateHandleBuffer service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @search_type		selection criterion
 * @protocol		GUID of the protocol
 * @search_key		registration key
 * @no_handles		number of returned handles
 * @buffer		buffer with the returned handles
 * @return		status code
 */
static efi_status_t EFIAPI efi_locate_handle_buffer(
			enum efi_locate_search_type search_type,
			const efi_guid_t *protocol, void *search_key,
			efi_uintn_t *no_handles, efi_handle_t **buffer)
{
	efi_status_t r;
	efi_uintn_t buffer_size = 0;

	EFI_ENTRY("%d, %pUl, %p, %p, %p", search_type, protocol, search_key,
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

/*
 * Find an interface implementing a protocol.
 *
 * This function implements the LocateProtocol service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @protocol		GUID of the protocol
 * @registration	registration key passed to the notification function
 * @protocol_interface	interface implementing the protocol
 * @return		status code
 */
static efi_status_t EFIAPI efi_locate_protocol(const efi_guid_t *protocol,
					       void *registration,
					       void **protocol_interface)
{
	struct list_head *lhandle;
	efi_status_t ret;

	EFI_ENTRY("%pUl, %p, %p", protocol, registration, protocol_interface);

	if (!protocol || !protocol_interface)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	list_for_each(lhandle, &efi_obj_list) {
		struct efi_object *efiobj;
		struct efi_handler *handler;

		efiobj = list_entry(lhandle, struct efi_object, link);

		ret = efi_search_protocol(efiobj->handle, protocol, &handler);
		if (ret == EFI_SUCCESS) {
			*protocol_interface = handler->protocol_interface;
			return EFI_EXIT(EFI_SUCCESS);
		}
	}
	*protocol_interface = NULL;

	return EFI_EXIT(EFI_NOT_FOUND);
}

/*
 * Get the device path and handle of an device implementing a protocol.
 *
 * This function implements the LocateDevicePath service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @protocol		GUID of the protocol
 * @device_path		device path
 * @device		handle of the device
 * @return		status code
 */
static efi_status_t EFIAPI efi_locate_device_path(
			const efi_guid_t *protocol,
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

	EFI_ENTRY("%pUl, %p, %p", protocol, device_path, device);

	if (!protocol || !device_path || !*device_path || !device) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Find end of device path */
	len = efi_dp_size(*device_path);

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
		len_dp = efi_dp_size(dp);
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

/*
 * Install multiple protocol interfaces.
 *
 * This function implements the MultipleProtocolInterfaces service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @handle	handle on which the protocol interfaces shall be installed
 * @...		NULL terminated argument list with pairs of protocol GUIDS and
 *		interfaces
 * @return	status code
 */
static efi_status_t EFIAPI efi_install_multiple_protocol_interfaces(
			void **handle, ...)
{
	EFI_ENTRY("%p", handle);

	va_list argptr;
	const efi_guid_t *protocol;
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
		r = EFI_CALL(efi_install_protocol_interface(
						handle, protocol,
						EFI_NATIVE_INTERFACE,
						protocol_interface));
		if (r != EFI_SUCCESS)
			break;
		i++;
	}
	va_end(argptr);
	if (r == EFI_SUCCESS)
		return EFI_EXIT(r);

	/* If an error occurred undo all changes. */
	va_start(argptr, handle);
	for (; i; --i) {
		protocol = va_arg(argptr, efi_guid_t*);
		protocol_interface = va_arg(argptr, void*);
		EFI_CALL(efi_uninstall_protocol_interface(handle, protocol,
							  protocol_interface));
	}
	va_end(argptr);

	return EFI_EXIT(r);
}

/*
 * Uninstall multiple protocol interfaces.
 *
 * This function implements the UninstallMultipleProtocolInterfaces service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @handle	handle from which the protocol interfaces shall be removed
 * @...		NULL terminated argument list with pairs of protocol GUIDS and
 *		interfaces
 * @return	status code
 */
static efi_status_t EFIAPI efi_uninstall_multiple_protocol_interfaces(
			void *handle, ...)
{
	EFI_ENTRY("%p", handle);

	va_list argptr;
	const efi_guid_t *protocol;
	void *protocol_interface;
	efi_status_t r = EFI_SUCCESS;
	size_t i = 0;

	if (!handle)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	va_start(argptr, handle);
	for (;;) {
		protocol = va_arg(argptr, efi_guid_t*);
		if (!protocol)
			break;
		protocol_interface = va_arg(argptr, void*);
		r = EFI_CALL(efi_uninstall_protocol_interface(
						handle, protocol,
						protocol_interface));
		if (r != EFI_SUCCESS)
			break;
		i++;
	}
	va_end(argptr);
	if (r == EFI_SUCCESS)
		return EFI_EXIT(r);

	/* If an error occurred undo all changes. */
	va_start(argptr, handle);
	for (; i; --i) {
		protocol = va_arg(argptr, efi_guid_t*);
		protocol_interface = va_arg(argptr, void*);
		EFI_CALL(efi_install_protocol_interface(&handle, protocol,
							EFI_NATIVE_INTERFACE,
							protocol_interface));
	}
	va_end(argptr);

	return EFI_EXIT(r);
}

/*
 * Calculate cyclic redundancy code.
 *
 * This function implements the CalculateCrc32 service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @data	buffer with data
 * @data_size	size of buffer in bytes
 * @crc32_p	cyclic redundancy code
 * @return	status code
 */
static efi_status_t EFIAPI efi_calculate_crc32(void *data,
					       unsigned long data_size,
					       uint32_t *crc32_p)
{
	EFI_ENTRY("%p, %ld", data, data_size);
	*crc32_p = crc32(0, data, data_size);
	return EFI_EXIT(EFI_SUCCESS);
}

/*
 * Copy memory.
 *
 * This function implements the CopyMem service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @destination		destination of the copy operation
 * @source		source of the copy operation
 * @length		number of bytes to copy
 */
static void EFIAPI efi_copy_mem(void *destination, const void *source,
				size_t length)
{
	EFI_ENTRY("%p, %p, %ld", destination, source, (unsigned long)length);
	memcpy(destination, source, length);
	EFI_EXIT(EFI_SUCCESS);
}

/*
 * Fill memory with a byte value.
 *
 * This function implements the SetMem service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @buffer		buffer to fill
 * @size		size of buffer in bytes
 * @value		byte to copy to the buffer
 */
static void EFIAPI efi_set_mem(void *buffer, size_t size, uint8_t value)
{
	EFI_ENTRY("%p, %ld, 0x%x", buffer, (unsigned long)size, value);
	memset(buffer, value, size);
	EFI_EXIT(EFI_SUCCESS);
}

/*
 * Open protocol interface on a handle.
 *
 * This function implements the OpenProtocol interface.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @handle		handle on which the protocol shall be opened
 * @protocol		GUID of the protocol
 * @protocol_interface	interface implementing the protocol
 * @agent_handle	handle of the driver
 * @controller_handle	handle of the controller
 * @attributes		attributes indicating how to open the protocol
 * @return		status code
 */
static efi_status_t EFIAPI efi_open_protocol(
			void *handle, const efi_guid_t *protocol,
			void **protocol_interface, void *agent_handle,
			void *controller_handle, uint32_t attributes)
{
	struct efi_handler *handler;
	efi_status_t r = EFI_INVALID_PARAMETER;

	EFI_ENTRY("%p, %pUl, %p, %p, %p, 0x%x", handle, protocol,
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

	r = efi_search_protocol(handle, protocol, &handler);
	if (r != EFI_SUCCESS)
		goto out;

	if (attributes != EFI_OPEN_PROTOCOL_TEST_PROTOCOL)
		*protocol_interface = handler->protocol_interface;
out:
	return EFI_EXIT(r);
}

/*
 * Get interface of a protocol on a handle.
 *
 * This function implements the HandleProtocol service.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @handle		handle on which the protocol shall be opened
 * @protocol		GUID of the protocol
 * @protocol_interface  interface implementing the protocol
 * @return		status code
 */
static efi_status_t EFIAPI efi_handle_protocol(void *handle,
					       const efi_guid_t *protocol,
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
