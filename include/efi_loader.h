/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _EFI_LOADER_H
#define _EFI_LOADER_H 1

#include <common.h>
#include <part_efi.h>
#include <efi_api.h>

/* No need for efi loader support in SPL */
#if defined(CONFIG_EFI_LOADER) && !defined(CONFIG_SPL_BUILD)

#include <linux/list.h>

int __efi_entry_check(void);
int __efi_exit_check(void);
const char *__efi_nesting(void);
const char *__efi_nesting_inc(void);
const char *__efi_nesting_dec(void);

/*
 * Enter the u-boot world from UEFI:
 */
#define EFI_ENTRY(format, ...) do { \
	assert(__efi_entry_check()); \
	debug("%sEFI: Entry %s(" format ")\n", __efi_nesting_inc(), \
		__func__, ##__VA_ARGS__); \
	} while(0)

/*
 * Exit the u-boot world back to UEFI:
 */
#define EFI_EXIT(ret) ({ \
	typeof(ret) _r = ret; \
	debug("%sEFI: Exit: %s: %u\n", __efi_nesting_dec(), \
		__func__, (u32)((uintptr_t) _r & ~EFI_ERROR_MASK)); \
	assert(__efi_exit_check()); \
	_r; \
	})

/*
 * Call non-void UEFI function from u-boot and retrieve return value:
 */
#define EFI_CALL(exp) ({ \
	debug("%sEFI: Call: %s\n", __efi_nesting_inc(), #exp); \
	assert(__efi_exit_check()); \
	typeof(exp) _r = exp; \
	assert(__efi_entry_check()); \
	debug("%sEFI: %lu returned by %s\n", __efi_nesting_dec(), \
	      (unsigned long)((uintptr_t)_r & ~EFI_ERROR_MASK), #exp); \
	_r; \
})

/*
 * Call void UEFI function from u-boot:
 */
#define EFI_CALL_VOID(exp) do { \
	debug("%sEFI: Call: %s\n", __efi_nesting_inc(), #exp); \
	assert(__efi_exit_check()); \
	exp; \
	assert(__efi_entry_check()); \
	debug("%sEFI: Return From: %s\n", __efi_nesting_dec(), #exp); \
	} while(0)

/*
 * Write an indented message with EFI prefix
 */
#define EFI_PRINT(format, ...) ({ \
	debug("%sEFI: " format, __efi_nesting(), \
		##__VA_ARGS__); \
	})

extern struct efi_runtime_services efi_runtime_services;
extern struct efi_system_table systab;

extern struct efi_simple_text_output_protocol efi_con_out;
extern struct efi_simple_input_interface efi_con_in;
extern struct efi_console_control_protocol efi_console_control;
extern const struct efi_device_path_to_text_protocol efi_device_path_to_text;

uint16_t *efi_dp_str(struct efi_device_path *dp);

/* GUID of the EFI_BLOCK_IO_PROTOCOL */
extern const efi_guid_t efi_block_io_guid;
extern const efi_guid_t efi_global_variable_guid;
extern const efi_guid_t efi_guid_console_control;
extern const efi_guid_t efi_guid_device_path;
/* GUID of the EFI_DRIVER_BINDING_PROTOCOL */
extern const efi_guid_t efi_guid_driver_binding_protocol;
extern const efi_guid_t efi_guid_loaded_image;
extern const efi_guid_t efi_guid_device_path_to_text_protocol;
extern const efi_guid_t efi_simple_file_system_protocol_guid;
extern const efi_guid_t efi_file_info_guid;

extern unsigned int __efi_runtime_start, __efi_runtime_stop;
extern unsigned int __efi_runtime_rel_start, __efi_runtime_rel_stop;

/*
 * When a protocol is opened a open protocol info entry is created.
 * These are maintained in a list.
 */
struct efi_open_protocol_info_item {
	/* Link to the list of open protocol info entries of a protocol */
	struct list_head link;
	struct efi_open_protocol_info_entry info;
};

/*
 * When the UEFI payload wants to open a protocol on an object to get its
 * interface (usually a struct with callback functions), this struct maps the
 * protocol GUID to the respective protocol interface
 */
struct efi_handler {
	/* Link to the list of protocols of a handle */
	struct list_head link;
	const efi_guid_t *guid;
	void *protocol_interface;
	/* Link to the list of open protocol info items */
	struct list_head open_infos;
};

/*
 * UEFI has a poor man's OO model where one "object" can be polymorphic and have
 * multiple different protocols (classes) attached to it.
 *
 * This struct is the parent struct for all of our actual implementation objects
 * that can include it to make themselves an EFI object
 */
struct efi_object {
	/* Every UEFI object is part of a global object list */
	struct list_head link;
	/* The list of protocols */
	struct list_head protocols;
	/* The object spawner can either use this for data or as identifier */
	void *handle;
};

/**
 * struct efi_event
 *
 * @type:		Type of event, see efi_create_event
 * @notify_tpl:		Task priority level of notifications
 * @trigger_time:	Period of the timer
 * @trigger_next:	Next time to trigger the timer
 * @nofify_function:	Function to call when the event is triggered
 * @notify_context:	Data to be passed to the notify function
 * @trigger_type:	Type of timer, see efi_set_timer
 * @queued:		The notification function is queued
 * @signaled:		The event occurred. The event is in the signaled state.
 */
struct efi_event {
	uint32_t type;
	efi_uintn_t notify_tpl;
	void (EFIAPI *notify_function)(struct efi_event *event, void *context);
	void *notify_context;
	u64 trigger_next;
	u64 trigger_time;
	enum efi_timer_delay trigger_type;
	bool is_queued;
	bool is_signaled;
};


/* This list contains all UEFI objects we know of */
extern struct list_head efi_obj_list;

/* Called by bootefi to make console interface available */
int efi_console_register(void);
/* Called by bootefi to make all disk storage accessible as EFI objects */
efi_status_t efi_disk_register(void);
/* Create handles and protocols for the partitions of a block device */
int efi_disk_create_partitions(efi_handle_t parent, struct blk_desc *desc,
			       const char *if_typename, int diskid,
			       const char *pdevname);
/* Called by bootefi to make GOP (graphical) interface available */
int efi_gop_register(void);
/* Called by bootefi to make the network interface available */
int efi_net_register(void);
/* Called by bootefi to make the watchdog available */
int efi_watchdog_register(void);
/* Called by bootefi to make SMBIOS tables available */
void efi_smbios_register(void);

struct efi_simple_file_system_protocol *
efi_fs_from_path(struct efi_device_path *fp);

/* Called by networking code to memorize the dhcp ack package */
void efi_net_set_dhcp_ack(void *pkt, int len);
/* Called by efi_set_watchdog_timer to reset the timer */
efi_status_t efi_set_watchdog(unsigned long timeout);

/* Called from places to check whether a timer expired */
void efi_timer_check(void);
/* PE loader implementation */
void *efi_load_pe(void *efi, struct efi_loaded_image *loaded_image_info);
/* Called once to store the pristine gd pointer */
void efi_save_gd(void);
/* Special case handler for error/abort that just tries to dtrt to get
 * back to u-boot world */
void efi_restore_gd(void);
/* Call this to relocate the runtime section to an address space */
void efi_runtime_relocate(ulong offset, struct efi_mem_desc *map);
/* Call this to set the current device name */
void efi_set_bootdev(const char *dev, const char *devnr, const char *path);
/* Add a new object to the object list. */
void efi_add_handle(struct efi_object *obj);
/* Create handle */
efi_status_t efi_create_handle(efi_handle_t *handle);
/* Delete handle */
void efi_delete_handle(struct efi_object *obj);
/* Call this to validate a handle and find the EFI object for it */
struct efi_object *efi_search_obj(const efi_handle_t handle);
/* Find a protocol on a handle */
efi_status_t efi_search_protocol(const efi_handle_t handle,
				 const efi_guid_t *protocol_guid,
				 struct efi_handler **handler);
/* Install new protocol on a handle */
efi_status_t efi_add_protocol(const efi_handle_t handle,
			      const efi_guid_t *protocol,
			      void *protocol_interface);
/* Delete protocol from a handle */
efi_status_t efi_remove_protocol(const efi_handle_t handle,
				 const efi_guid_t *protocol,
				 void *protocol_interface);
/* Delete all protocols from a handle */
efi_status_t efi_remove_all_protocols(const efi_handle_t handle);
/* Call this to create an event */
efi_status_t efi_create_event(uint32_t type, efi_uintn_t notify_tpl,
			      void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
			      void *notify_context, struct efi_event **event);
/* Call this to set a timer */
efi_status_t efi_set_timer(struct efi_event *event, enum efi_timer_delay type,
			   uint64_t trigger_time);
/* Call this to signal an event */
void efi_signal_event(struct efi_event *event, bool check_tpl);

/* open file system: */
struct efi_simple_file_system_protocol *efi_simple_file_system(
		struct blk_desc *desc, int part, struct efi_device_path *dp);

/* open file from device-path: */
struct efi_file_handle *efi_file_from_path(struct efi_device_path *fp);


/* Generic EFI memory allocator, call this to get memory */
void *efi_alloc(uint64_t len, int memory_type);
/* More specific EFI memory allocator, called by EFI payloads */
efi_status_t efi_allocate_pages(int type, int memory_type, efi_uintn_t pages,
				uint64_t *memory);
/* EFI memory free function. */
efi_status_t efi_free_pages(uint64_t memory, efi_uintn_t pages);
/* EFI memory allocator for small allocations */
efi_status_t efi_allocate_pool(int pool_type, efi_uintn_t size,
			       void **buffer);
/* EFI pool memory free function. */
efi_status_t efi_free_pool(void *buffer);
/* Returns the EFI memory map */
efi_status_t efi_get_memory_map(efi_uintn_t *memory_map_size,
				struct efi_mem_desc *memory_map,
				efi_uintn_t *map_key,
				efi_uintn_t *descriptor_size,
				uint32_t *descriptor_version);
/* Adds a range into the EFI memory map */
uint64_t efi_add_memory_map(uint64_t start, uint64_t pages, int memory_type,
			    bool overlap_only_ram);
/* Called by board init to initialize the EFI drivers */
efi_status_t efi_driver_init(void);
/* Called by board init to initialize the EFI memory map */
int efi_memory_init(void);
/* Adds new or overrides configuration table entry to the system table */
efi_status_t efi_install_configuration_table(const efi_guid_t *guid, void *table);
/* Sets up a loaded image */
efi_status_t efi_setup_loaded_image(
			struct efi_loaded_image *info, struct efi_object *obj,
			struct efi_device_path *device_path,
			struct efi_device_path *file_path);
efi_status_t efi_load_image_from_path(struct efi_device_path *file_path,
				      void **buffer);

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
extern void *efi_bounce_buffer;
#define EFI_LOADER_BOUNCE_BUFFER_SIZE (64 * 1024 * 1024)
#endif


struct efi_device_path *efi_dp_next(const struct efi_device_path *dp);
int efi_dp_match(const struct efi_device_path *a,
		 const struct efi_device_path *b);
struct efi_object *efi_dp_find_obj(struct efi_device_path *dp,
				   struct efi_device_path **rem);
unsigned efi_dp_size(const struct efi_device_path *dp);
struct efi_device_path *efi_dp_dup(const struct efi_device_path *dp);
struct efi_device_path *efi_dp_append(const struct efi_device_path *dp1,
				      const struct efi_device_path *dp2);
struct efi_device_path *efi_dp_append_node(const struct efi_device_path *dp,
					   const struct efi_device_path *node);


struct efi_device_path *efi_dp_from_dev(struct udevice *dev);
struct efi_device_path *efi_dp_from_part(struct blk_desc *desc, int part);
/* Create a device node for a block device partition. */
struct efi_device_path *efi_dp_part_node(struct blk_desc *desc, int part);
struct efi_device_path *efi_dp_from_file(struct blk_desc *desc, int part,
					 const char *path);
struct efi_device_path *efi_dp_from_eth(void);
struct efi_device_path *efi_dp_from_mem(uint32_t mem_type,
					uint64_t start_address,
					uint64_t end_address);
/* Determine the last device path node that is not the end node. */
const struct efi_device_path *efi_dp_last_node(
			const struct efi_device_path *dp);
efi_status_t efi_dp_split_file_path(struct efi_device_path *full_path,
				    struct efi_device_path **device_path,
				    struct efi_device_path **file_path);

#define EFI_DP_TYPE(_dp, _type, _subtype) \
	(((_dp)->type == DEVICE_PATH_TYPE_##_type) && \
	 ((_dp)->sub_type == DEVICE_PATH_SUB_TYPE_##_subtype))

/* Convert strings from normal C strings to uEFI strings */
static inline void ascii2unicode(u16 *unicode, const char *ascii)
{
	while (*ascii)
		*(unicode++) = *(ascii++);
}

static inline int guidcmp(const efi_guid_t *g1, const efi_guid_t *g2)
{
	return memcmp(g1, g2, sizeof(efi_guid_t));
}

/*
 * Use these to indicate that your code / data should go into the EFI runtime
 * section and thus still be available when the OS is running
 */
#define __efi_runtime_data __attribute__ ((section ("efi_runtime_data")))
#define __efi_runtime __attribute__ ((section ("efi_runtime_text")))

/* Call this with mmio_ptr as the _pointer_ to a pointer to an MMIO region
 * to make it available at runtime */
void efi_add_runtime_mmio(void *mmio_ptr, u64 len);

/* Boards may provide the functions below to implement RTS functionality */

void __efi_runtime EFIAPI efi_reset_system(
			enum efi_reset_type reset_type,
			efi_status_t reset_status,
			unsigned long data_size, void *reset_data);
void efi_reset_system_init(void);

efi_status_t __efi_runtime EFIAPI efi_get_time(
			struct efi_time *time,
			struct efi_time_cap *capabilities);
void efi_get_time_init(void);

#ifdef CONFIG_CMD_BOOTEFI_SELFTEST
/*
 * Entry point for the tests of the EFI API.
 * It is called by 'bootefi selftest'
 */
efi_status_t EFIAPI efi_selftest(efi_handle_t image_handle,
				 struct efi_system_table *systab);
#endif

efi_status_t EFIAPI efi_get_variable(s16 *variable_name,
		efi_guid_t *vendor, u32 *attributes,
		unsigned long *data_size, void *data);
efi_status_t EFIAPI efi_get_next_variable(
		unsigned long *variable_name_size,
		s16 *variable_name, efi_guid_t *vendor);
efi_status_t EFIAPI efi_set_variable(s16 *variable_name,
		efi_guid_t *vendor, u32 attributes,
		unsigned long data_size, void *data);

void *efi_bootmgr_load(struct efi_device_path **device_path,
		       struct efi_device_path **file_path);

#else /* defined(EFI_LOADER) && !defined(CONFIG_SPL_BUILD) */

/* Without CONFIG_EFI_LOADER we don't have a runtime section, stub it out */
#define __efi_runtime_data
#define __efi_runtime
static inline void efi_add_runtime_mmio(void *mmio_ptr, u64 len) { }

/* No loader configured, stub out EFI_ENTRY */
static inline void efi_restore_gd(void) { }
static inline void efi_set_bootdev(const char *dev, const char *devnr,
				   const char *path) { }
static inline void efi_net_set_dhcp_ack(void *pkt, int len) { }

#endif /* CONFIG_EFI_LOADER && !CONFIG_SPL_BUILD */

#endif /* _EFI_LOADER_H */
