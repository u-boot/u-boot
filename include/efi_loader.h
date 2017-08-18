/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <part_efi.h>
#include <efi_api.h>

/* No need for efi loader support in SPL */
#if defined(CONFIG_EFI_LOADER) && !defined(CONFIG_SPL_BUILD)

#include <linux/list.h>

int __efi_entry_check(void);
int __efi_exit_check(void);
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
 * Callback into UEFI world from u-boot:
 */
#define EFI_CALL(exp) do { \
	debug("%sEFI: Call: %s\n", __efi_nesting_inc(), #exp); \
	assert(__efi_exit_check()); \
	exp; \
	assert(__efi_entry_check()); \
	debug("%sEFI: Return From: %s\n", __efi_nesting_dec(), #exp); \
	} while(0)

extern struct efi_runtime_services efi_runtime_services;
extern struct efi_system_table systab;

extern const struct efi_simple_text_output_protocol efi_con_out;
extern struct efi_simple_input_interface efi_con_in;
extern const struct efi_console_control_protocol efi_console_control;
extern const struct efi_device_path_to_text_protocol efi_device_path_to_text;

extern const efi_guid_t efi_guid_console_control;
extern const efi_guid_t efi_guid_device_path;
extern const efi_guid_t efi_guid_loaded_image;
extern const efi_guid_t efi_guid_device_path_to_text_protocol;

extern unsigned int __efi_runtime_start, __efi_runtime_stop;
extern unsigned int __efi_runtime_rel_start, __efi_runtime_rel_stop;

/*
 * When the UEFI payload wants to open a protocol on an object to get its
 * interface (usually a struct with callback functions), this struct maps the
 * protocol GUID to the respective protocol interface */
struct efi_handler {
	const efi_guid_t *guid;
	void *protocol_interface;
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
	/* We support up to 8 "protocols" an object can be accessed through */
	struct efi_handler protocols[8];
	/* The object spawner can either use this for data or as identifier */
	void *handle;
};

#define EFI_PROTOCOL_OBJECT(_guid, _protocol) (struct efi_object){	\
	.protocols = {{							\
		.guid = &(_guid),	 				\
		.protocol_interface = (void *)(_protocol), 		\
	}},								\
	.handle = (void *)(_protocol),					\
}

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
 * @signaled:		The notify function was already called
 */
struct efi_event {
	uint32_t type;
	UINTN notify_tpl;
	void (EFIAPI *notify_function)(struct efi_event *event, void *context);
	void *notify_context;
	u64 trigger_next;
	u64 trigger_time;
	enum efi_timer_delay trigger_type;
	int signaled;
};


/* This list contains all UEFI objects we know of */
extern struct list_head efi_obj_list;

/* Called by bootefi to make console interface available */
int efi_console_register(void);
/* Called by bootefi to make all disk storage accessible as EFI objects */
int efi_disk_register(void);
/* Called by bootefi to make GOP (graphical) interface available */
int efi_gop_register(void);
/* Called by bootefi to make the network interface available */
int efi_net_register(void **handle);
/* Called by bootefi to make SMBIOS tables available */
void efi_smbios_register(void);

/* Called by networking code to memorize the dhcp ack package */
void efi_net_set_dhcp_ack(void *pkt, int len);

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
/* Call this to create an event */
efi_status_t efi_create_event(uint32_t type, UINTN notify_tpl,
			      void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
			      void *notify_context, struct efi_event **event);
/* Call this to set a timer */
efi_status_t efi_set_timer(struct efi_event *event, enum efi_timer_delay type,
			   uint64_t trigger_time);
/* Call this to signal an event */
void efi_signal_event(struct efi_event *event);

/* Generic EFI memory allocator, call this to get memory */
void *efi_alloc(uint64_t len, int memory_type);
/* More specific EFI memory allocator, called by EFI payloads */
efi_status_t efi_allocate_pages(int type, int memory_type, unsigned long pages,
				uint64_t *memory);
/* EFI memory free function. */
efi_status_t efi_free_pages(uint64_t memory, unsigned long pages);
/* EFI memory allocator for small allocations */
efi_status_t efi_allocate_pool(int pool_type, unsigned long size,
			       void **buffer);
/* EFI pool memory free function. */
efi_status_t efi_free_pool(void *buffer);
/* Returns the EFI memory map */
efi_status_t efi_get_memory_map(unsigned long *memory_map_size,
				struct efi_mem_desc *memory_map,
				unsigned long *map_key,
				unsigned long *descriptor_size,
				uint32_t *descriptor_version);
/* Adds a range into the EFI memory map */
uint64_t efi_add_memory_map(uint64_t start, uint64_t pages, int memory_type,
			    bool overlap_only_ram);
/* Called by board init to initialize the EFI memory map */
int efi_memory_init(void);
/* Adds new or overrides configuration table entry to the system table */
efi_status_t efi_install_configuration_table(const efi_guid_t *guid, void *table);

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
extern void *efi_bounce_buffer;
#define EFI_LOADER_BOUNCE_BUFFER_SIZE (64 * 1024 * 1024)
#endif

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

#endif
