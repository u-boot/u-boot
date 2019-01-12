/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Extensible Firmware Interface
 * Based on 'Extensible Firmware Interface Specification' version 0.9,
 * April 30, 1999
 *
 * Copyright (C) 1999 VA Linux Systems
 * Copyright (C) 1999 Walt Drummond <drummond@valinux.com>
 * Copyright (C) 1999, 2002-2003 Hewlett-Packard Co.
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 *	Stephane Eranian <eranian@hpl.hp.com>
 *
 * From include/linux/efi.h in kernel 4.1 with some additions/subtractions
 */

#ifndef _EFI_API_H
#define _EFI_API_H

#include <efi.h>

#ifdef CONFIG_EFI_LOADER
#include <asm/setjmp.h>
#endif

/* UEFI spec version 2.7 */
#define EFI_SPECIFICATION_VERSION (2 << 16 | 70)

/* Types and defines for EFI CreateEvent */
enum efi_timer_delay {
	EFI_TIMER_STOP = 0,
	EFI_TIMER_PERIODIC = 1,
	EFI_TIMER_RELATIVE = 2
};

#define efi_intn_t ssize_t
#define efi_uintn_t size_t
typedef uint16_t *efi_string_t;

#define EVT_TIMER				0x80000000
#define EVT_RUNTIME				0x40000000
#define EVT_NOTIFY_WAIT				0x00000100
#define EVT_NOTIFY_SIGNAL			0x00000200
#define EVT_SIGNAL_EXIT_BOOT_SERVICES		0x00000201
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE	0x60000202

#define TPL_APPLICATION		0x04
#define TPL_CALLBACK		0x08
#define TPL_NOTIFY		0x10
#define TPL_HIGH_LEVEL		0x1F

struct efi_event;

/* EFI Boot Services table */
#define EFI_BOOT_SERVICES_SIGNATURE 0x56524553544f4f42
struct efi_boot_services {
	struct efi_table_hdr hdr;
	efi_status_t (EFIAPI *raise_tpl)(efi_uintn_t new_tpl);
	void (EFIAPI *restore_tpl)(efi_uintn_t old_tpl);

	efi_status_t (EFIAPI *allocate_pages)(int, int, efi_uintn_t,
					      efi_physical_addr_t *);
	efi_status_t (EFIAPI *free_pages)(efi_physical_addr_t, efi_uintn_t);
	efi_status_t (EFIAPI *get_memory_map)(efi_uintn_t *memory_map_size,
					      struct efi_mem_desc *desc,
					      efi_uintn_t *key,
					      efi_uintn_t *desc_size,
					      u32 *desc_version);
	efi_status_t (EFIAPI *allocate_pool)(int, efi_uintn_t, void **);
	efi_status_t (EFIAPI *free_pool)(void *);

	efi_status_t (EFIAPI *create_event)(uint32_t type,
			efi_uintn_t notify_tpl,
			void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
			void *notify_context, struct efi_event **event);
	efi_status_t (EFIAPI *set_timer)(struct efi_event *event,
					 enum efi_timer_delay type,
					 uint64_t trigger_time);
	efi_status_t (EFIAPI *wait_for_event)(efi_uintn_t number_of_events,
					      struct efi_event **event,
					      efi_uintn_t *index);
	efi_status_t (EFIAPI *signal_event)(struct efi_event *event);
	efi_status_t (EFIAPI *close_event)(struct efi_event *event);
	efi_status_t (EFIAPI *check_event)(struct efi_event *event);
#define EFI_NATIVE_INTERFACE	0x00000000
	efi_status_t (EFIAPI *install_protocol_interface)(
			efi_handle_t *handle, const efi_guid_t *protocol,
			int protocol_interface_type, void *protocol_interface);
	efi_status_t (EFIAPI *reinstall_protocol_interface)(
			efi_handle_t handle, const efi_guid_t *protocol,
			void *old_interface, void *new_interface);
	efi_status_t (EFIAPI *uninstall_protocol_interface)(
			efi_handle_t handle, const efi_guid_t *protocol,
			void *protocol_interface);
	efi_status_t (EFIAPI *handle_protocol)(
			efi_handle_t handle, const efi_guid_t *protocol,
			void **protocol_interface);
	void *reserved;
	efi_status_t (EFIAPI *register_protocol_notify)(
			const efi_guid_t *protocol, struct efi_event *event,
			void **registration);
	efi_status_t (EFIAPI *locate_handle)(
			enum efi_locate_search_type search_type,
			const efi_guid_t *protocol, void *search_key,
			efi_uintn_t *buffer_size, efi_handle_t *buffer);
	efi_status_t (EFIAPI *locate_device_path)(const efi_guid_t *protocol,
			struct efi_device_path **device_path,
			efi_handle_t *device);
	efi_status_t (EFIAPI *install_configuration_table)(
			efi_guid_t *guid, void *table);

	efi_status_t (EFIAPI *load_image)(bool boot_policiy,
			efi_handle_t parent_image,
			struct efi_device_path *file_path, void *source_buffer,
			efi_uintn_t source_size, efi_handle_t *image);
	efi_status_t (EFIAPI *start_image)(efi_handle_t handle,
					   efi_uintn_t *exitdata_size,
					   u16 **exitdata);
	efi_status_t (EFIAPI *exit)(efi_handle_t handle,
				    efi_status_t exit_status,
				    efi_uintn_t exitdata_size, u16 *exitdata);
	efi_status_t (EFIAPI *unload_image)(efi_handle_t image_handle);
	efi_status_t (EFIAPI *exit_boot_services)(efi_handle_t, unsigned long);

	efi_status_t (EFIAPI *get_next_monotonic_count)(u64 *count);
	efi_status_t (EFIAPI *stall)(unsigned long usecs);
	efi_status_t (EFIAPI *set_watchdog_timer)(unsigned long timeout,
			uint64_t watchdog_code, unsigned long data_size,
			uint16_t *watchdog_data);
	efi_status_t(EFIAPI *connect_controller)(efi_handle_t controller_handle,
			efi_handle_t *driver_image_handle,
			struct efi_device_path *remaining_device_path,
			bool recursive);
	efi_status_t (EFIAPI *disconnect_controller)(
			efi_handle_t controller_handle,
			efi_handle_t driver_image_handle,
			efi_handle_t child_handle);
#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020
	efi_status_t (EFIAPI *open_protocol)(efi_handle_t handle,
			const efi_guid_t *protocol, void **interface,
			efi_handle_t agent_handle,
			efi_handle_t controller_handle, u32 attributes);
	efi_status_t (EFIAPI *close_protocol)(
			efi_handle_t handle, const efi_guid_t *protocol,
			efi_handle_t agent_handle,
			efi_handle_t controller_handle);
	efi_status_t(EFIAPI *open_protocol_information)(efi_handle_t handle,
			const efi_guid_t *protocol,
			struct efi_open_protocol_info_entry **entry_buffer,
			efi_uintn_t *entry_count);
	efi_status_t (EFIAPI *protocols_per_handle)(efi_handle_t handle,
			efi_guid_t ***protocol_buffer,
			efi_uintn_t *protocols_buffer_count);
	efi_status_t (EFIAPI *locate_handle_buffer) (
			enum efi_locate_search_type search_type,
			const efi_guid_t *protocol, void *search_key,
			efi_uintn_t *no_handles, efi_handle_t **buffer);
	efi_status_t (EFIAPI *locate_protocol)(const efi_guid_t *protocol,
			void *registration, void **protocol_interface);
	efi_status_t (EFIAPI *install_multiple_protocol_interfaces)(
			efi_handle_t *handle, ...);
	efi_status_t (EFIAPI *uninstall_multiple_protocol_interfaces)(
			efi_handle_t handle, ...);
	efi_status_t (EFIAPI *calculate_crc32)(const void *data,
					       efi_uintn_t data_size,
					       u32 *crc32);
	void (EFIAPI *copy_mem)(void *destination, const void *source,
			size_t length);
	void (EFIAPI *set_mem)(void *buffer, size_t size, uint8_t value);
	efi_status_t (EFIAPI *create_event_ex)(
				uint32_t type, efi_uintn_t notify_tpl,
				void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
				void *notify_context,
				efi_guid_t *event_group,
				struct efi_event **event);
};

/* Types and defines for EFI ResetSystem */
enum efi_reset_type {
	EFI_RESET_COLD = 0,
	EFI_RESET_WARM = 1,
	EFI_RESET_SHUTDOWN = 2,
	EFI_RESET_PLATFORM_SPECIFIC = 3,
};

/* EFI Runtime Services table */
#define EFI_RUNTIME_SERVICES_SIGNATURE	0x56524553544e5552ULL

#define CAPSULE_FLAGS_PERSIST_ACROSS_RESET	0x00010000
#define CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE	0x00020000
#define CAPSULE_FLAGS_INITIATE_RESET		0x00040000

struct efi_capsule_header {
	efi_guid_t *capsule_guid;
	u32 header_size;
	u32 flags;
	u32 capsule_image_size;
};

struct efi_runtime_services {
	struct efi_table_hdr hdr;
	efi_status_t (EFIAPI *get_time)(struct efi_time *time,
			struct efi_time_cap *capabilities);
	efi_status_t (EFIAPI *set_time)(struct efi_time *time);
	efi_status_t (EFIAPI *get_wakeup_time)(char *enabled, char *pending,
			struct efi_time *time);
	efi_status_t (EFIAPI *set_wakeup_time)(char enabled,
			struct efi_time *time);
	efi_status_t (EFIAPI *set_virtual_address_map)(
			unsigned long memory_map_size,
			unsigned long descriptor_size,
			uint32_t descriptor_version,
			struct efi_mem_desc *virtmap);
	efi_status_t (*convert_pointer)(unsigned long dbg, void **address);
	efi_status_t (EFIAPI *get_variable)(u16 *variable_name,
					    const efi_guid_t *vendor,
					    u32 *attributes,
					    efi_uintn_t *data_size, void *data);
	efi_status_t (EFIAPI *get_next_variable_name)(
			efi_uintn_t *variable_name_size,
			u16 *variable_name, const efi_guid_t *vendor);
	efi_status_t (EFIAPI *set_variable)(u16 *variable_name,
					    const efi_guid_t *vendor,
					    u32 attributes,
					    efi_uintn_t data_size,
					    const void *data);
	efi_status_t (EFIAPI *get_next_high_mono_count)(
			uint32_t *high_count);
	void (EFIAPI *reset_system)(enum efi_reset_type reset_type,
				    efi_status_t reset_status,
				    unsigned long data_size, void *reset_data);
	efi_status_t (EFIAPI *update_capsule)(
			struct efi_capsule_header **capsule_header_array,
			efi_uintn_t capsule_count,
			u64 scatter_gather_list);
	efi_status_t (EFIAPI *query_capsule_caps)(
			struct efi_capsule_header **capsule_header_array,
			efi_uintn_t capsule_count,
			u64 *maximum_capsule_size,
			u32 *reset_type);
	efi_status_t (EFIAPI *query_variable_info)(
			u32 attributes,
			u64 *maximum_variable_storage_size,
			u64 *remaining_variable_storage_size,
			u64 *maximum_variable_size);
};

/* EFI event group GUID definitions */
#define EFI_EVENT_GROUP_EXIT_BOOT_SERVICES \
	EFI_GUID(0x27abf055, 0xb1b8, 0x4c26, 0x80, 0x48, \
		 0x74, 0x8f, 0x37, 0xba, 0xa2, 0xdf)

#define EFI_EVENT_GROUP_VIRTUAL_ADDRESS_CHANGE \
	EFI_GUID(0x13fa7698, 0xc831, 0x49c7, 0x87, 0xea, \
		 0x8f, 0x43, 0xfc, 0xc2, 0x51, 0x96)

#define EFI_EVENT_GROUP_MEMORY_MAP_CHANGE \
	EFI_GUID(0x78bee926, 0x692f, 0x48fd, 0x9e, 0xdb, \
		 0x01, 0x42, 0x2e, 0xf0, 0xd7, 0xab)

#define EFI_EVENT_GROUP_READY_TO_BOOT \
	EFI_GUID(0x7ce88fb3, 0x4bd7, 0x4679, 0x87, 0xa8, \
		 0xa8, 0xd8, 0xde, 0xe5, 0x0d, 0x2b)

#define EFI_EVENT_GROUP_RESET_SYSTEM \
	EFI_GUID(0x62da6a56, 0x13fb, 0x485a, 0xa8, 0xda, \
		 0xa3, 0xdd, 0x79, 0x12, 0xcb, 0x6b)

/* EFI Configuration Table and GUID definitions */
#define NULL_GUID \
	EFI_GUID(0x00000000, 0x0000, 0x0000, 0x00, 0x00, \
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)

#define EFI_GLOBAL_VARIABLE_GUID \
	EFI_GUID(0x8be4df61, 0x93ca, 0x11d2, 0xaa, 0x0d, \
		 0x00, 0xe0, 0x98, 0x03, 0x2b, 0x8c)

#define LOADED_IMAGE_PROTOCOL_GUID \
	EFI_GUID(0x5b1b31a1, 0x9562, 0x11d2, 0x8e, 0x3f, \
		 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define EFI_FDT_GUID \
	EFI_GUID(0xb1b621d5, 0xf19c, 0x41a5, \
		 0x83, 0x0b, 0xd9, 0x15, 0x2c, 0x69, 0xaa, 0xe0)

#define EFI_ACPI_TABLE_GUID \
	EFI_GUID(0x8868e871, 0xe4f1, 0x11d3, \
		 0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81)

#define SMBIOS_TABLE_GUID \
	EFI_GUID(0xeb9d2d31, 0x2d88, 0x11d3,  \
		 0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d)

struct efi_configuration_table {
	efi_guid_t guid;
	void *table;
} __packed;

#define EFI_SYSTEM_TABLE_SIGNATURE ((u64)0x5453595320494249ULL)

struct efi_system_table {
	struct efi_table_hdr hdr;
	u16 *fw_vendor;   /* physical addr of wchar_t vendor string */
	u32 fw_revision;
	efi_handle_t con_in_handle;
	struct efi_simple_text_input_protocol *con_in;
	efi_handle_t con_out_handle;
	struct efi_simple_text_output_protocol *con_out;
	efi_handle_t stderr_handle;
	struct efi_simple_text_output_protocol *std_err;
	struct efi_runtime_services *runtime;
	struct efi_boot_services *boottime;
	efi_uintn_t nr_tables;
	struct efi_configuration_table *tables;
};

#define LOADED_IMAGE_GUID \
	EFI_GUID(0x5b1b31a1, 0x9562, 0x11d2, \
		 0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define EFI_LOADED_IMAGE_PROTOCOL_REVISION 0x1000

struct efi_loaded_image {
	u32 revision;
	void *parent_handle;
	struct efi_system_table *system_table;
	efi_handle_t device_handle;
	struct efi_device_path *file_path;
	void *reserved;
	u32 load_options_size;
	void *load_options;
	void *image_base;
	aligned_u64 image_size;
	unsigned int image_code_type;
	unsigned int image_data_type;
	unsigned long unload;
};

#define DEVICE_PATH_GUID \
	EFI_GUID(0x09576e91, 0x6d3f, 0x11d2, \
		 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define DEVICE_PATH_TYPE_END			0x7f
#  define DEVICE_PATH_SUB_TYPE_INSTANCE_END	0x01
#  define DEVICE_PATH_SUB_TYPE_END		0xff

struct efi_device_path {
	u8 type;
	u8 sub_type;
	u16 length;
} __packed;

struct efi_mac_addr {
	u8 addr[32];
} __packed;

#define DEVICE_PATH_TYPE_HARDWARE_DEVICE	0x01
#  define DEVICE_PATH_SUB_TYPE_MEMORY		0x03
#  define DEVICE_PATH_SUB_TYPE_VENDOR		0x04

struct efi_device_path_memory {
	struct efi_device_path dp;
	u32 memory_type;
	u64 start_address;
	u64 end_address;
} __packed;

struct efi_device_path_vendor {
	struct efi_device_path dp;
	efi_guid_t guid;
	u8 vendor_data[];
} __packed;

#define DEVICE_PATH_TYPE_ACPI_DEVICE		0x02
#  define DEVICE_PATH_SUB_TYPE_ACPI_DEVICE	0x01

#define EFI_PNP_ID(ID)				(u32)(((ID) << 16) | 0x41D0)
#define EISA_PNP_ID(ID)				EFI_PNP_ID(ID)
#define EISA_PNP_NUM(ID)			((ID) >> 16)

struct efi_device_path_acpi_path {
	struct efi_device_path dp;
	u32 hid;
	u32 uid;
} __packed;

#define DEVICE_PATH_TYPE_MESSAGING_DEVICE	0x03
#  define DEVICE_PATH_SUB_TYPE_MSG_ATAPI	0x01
#  define DEVICE_PATH_SUB_TYPE_MSG_SCSI		0x02
#  define DEVICE_PATH_SUB_TYPE_MSG_USB		0x05
#  define DEVICE_PATH_SUB_TYPE_MSG_MAC_ADDR	0x0b
#  define DEVICE_PATH_SUB_TYPE_MSG_USB_CLASS	0x0f
#  define DEVICE_PATH_SUB_TYPE_MSG_SD		0x1a
#  define DEVICE_PATH_SUB_TYPE_MSG_MMC		0x1d

struct efi_device_path_atapi {
	struct efi_device_path dp;
	u8 primary_secondary;
	u8 slave_master;
	u16 logical_unit_number;
} __packed;

struct efi_device_path_scsi {
	struct efi_device_path dp;
	u16 target_id;
	u16 logical_unit_number;
} __packed;

struct efi_device_path_usb {
	struct efi_device_path dp;
	u8 parent_port_number;
	u8 usb_interface;
} __packed;

struct efi_device_path_mac_addr {
	struct efi_device_path dp;
	struct efi_mac_addr mac;
	u8 if_type;
} __packed;

struct efi_device_path_usb_class {
	struct efi_device_path dp;
	u16 vendor_id;
	u16 product_id;
	u8 device_class;
	u8 device_subclass;
	u8 device_protocol;
} __packed;

struct efi_device_path_sd_mmc_path {
	struct efi_device_path dp;
	u8 slot_number;
} __packed;

#define DEVICE_PATH_TYPE_MEDIA_DEVICE		0x04
#  define DEVICE_PATH_SUB_TYPE_HARD_DRIVE_PATH	0x01
#  define DEVICE_PATH_SUB_TYPE_CDROM_PATH	0x02
#  define DEVICE_PATH_SUB_TYPE_FILE_PATH	0x04

struct efi_device_path_hard_drive_path {
	struct efi_device_path dp;
	u32 partition_number;
	u64 partition_start;
	u64 partition_end;
	u8 partition_signature[16];
	u8 partmap_type;
	u8 signature_type;
} __packed;

struct efi_device_path_cdrom_path {
	struct efi_device_path dp;
	u32 boot_entry;
	u64 partition_start;
	u64 partition_end;
} __packed;

struct efi_device_path_file_path {
	struct efi_device_path dp;
	u16 str[];
} __packed;

#define BLOCK_IO_GUID \
	EFI_GUID(0x964e5b21, 0x6459, 0x11d2, \
		 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

struct efi_block_io_media {
	u32 media_id;
	char removable_media;
	char media_present;
	char logical_partition;
	char read_only;
	char write_caching;
	u8 pad[3];
	u32 block_size;
	u32 io_align;
	u8 pad2[4];
	u64 last_block;
	/* Added in revision 2 of the protocol */
	u64 lowest_aligned_lba;
	u32 logical_blocks_per_physical_block;
	/* Added in revision 3 of the protocol */
	u32 optimal_transfer_length_granualarity;
};

#define EFI_BLOCK_IO_PROTOCOL_REVISION2	0x00020001
#define EFI_BLOCK_IO_PROTOCOL_REVISION3	0x0002001f

struct efi_block_io {
	u64 revision;
	struct efi_block_io_media *media;
	efi_status_t (EFIAPI *reset)(struct efi_block_io *this,
			char extended_verification);
	efi_status_t (EFIAPI *read_blocks)(struct efi_block_io *this,
			u32 media_id, u64 lba, efi_uintn_t buffer_size,
			void *buffer);
	efi_status_t (EFIAPI *write_blocks)(struct efi_block_io *this,
			u32 media_id, u64 lba, efi_uintn_t buffer_size,
			void *buffer);
	efi_status_t (EFIAPI *flush_blocks)(struct efi_block_io *this);
};

struct simple_text_output_mode {
	s32 max_mode;
	s32 mode;
	s32 attribute;
	s32 cursor_column;
	s32 cursor_row;
	bool cursor_visible;
};

#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID \
	EFI_GUID(0x387477c2, 0x69c7, 0x11d2, \
		 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define EFI_BLACK                0x00
#define EFI_BLUE                 0x01
#define EFI_GREEN                0x02
#define EFI_CYAN                 0x03
#define EFI_RED                  0x04
#define EFI_MAGENTA              0x05
#define EFI_BROWN                0x06
#define EFI_LIGHTGRAY            0x07
#define EFI_BRIGHT               0x08
#define EFI_DARKGRAY             0x08
#define EFI_LIGHTBLUE            0x09
#define EFI_LIGHTGREEN           0x0a
#define EFI_LIGHTCYAN            0x0b
#define EFI_LIGHTRED             0x0c
#define EFI_LIGHTMAGENTA         0x0d
#define EFI_YELLOW               0x0e
#define EFI_WHITE                0x0f
#define EFI_BACKGROUND_BLACK     0x00
#define EFI_BACKGROUND_BLUE      0x10
#define EFI_BACKGROUND_GREEN     0x20
#define EFI_BACKGROUND_CYAN      0x30
#define EFI_BACKGROUND_RED       0x40
#define EFI_BACKGROUND_MAGENTA   0x50
#define EFI_BACKGROUND_BROWN     0x60
#define EFI_BACKGROUND_LIGHTGRAY 0x70

/* extract foreground color from EFI attribute */
#define EFI_ATTR_FG(attr)        ((attr) & 0x07)
/* treat high bit of FG as bright/bold (similar to edk2) */
#define EFI_ATTR_BOLD(attr)      (((attr) >> 3) & 0x01)
/* extract background color from EFI attribute */
#define EFI_ATTR_BG(attr)        (((attr) >> 4) & 0x7)

struct efi_simple_text_output_protocol {
	void *reset;
	efi_status_t (EFIAPI *output_string)(
			struct efi_simple_text_output_protocol *this,
			const efi_string_t str);
	efi_status_t (EFIAPI *test_string)(
			struct efi_simple_text_output_protocol *this,
			const efi_string_t str);
	efi_status_t(EFIAPI *query_mode)(
			struct efi_simple_text_output_protocol *this,
			unsigned long mode_number, unsigned long *columns,
			unsigned long *rows);
	efi_status_t(EFIAPI *set_mode)(
			struct efi_simple_text_output_protocol *this,
			unsigned long mode_number);
	efi_status_t(EFIAPI *set_attribute)(
			struct efi_simple_text_output_protocol *this,
			unsigned long attribute);
	efi_status_t(EFIAPI *clear_screen) (
			struct efi_simple_text_output_protocol *this);
	efi_status_t(EFIAPI *set_cursor_position) (
			struct efi_simple_text_output_protocol *this,
			unsigned long column, unsigned long row);
	efi_status_t(EFIAPI *enable_cursor)(
			struct efi_simple_text_output_protocol *this,
			bool enable);
	struct simple_text_output_mode *mode;
};

#define EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID \
	EFI_GUID(0xdd9e7534, 0x7762, 0x4698, \
		 0x8c, 0x14, 0xf5, 0x85, 0x17, 0xa6, 0x25, 0xaa)

struct efi_input_key {
	u16 scan_code;
	s16 unicode_char;
};

#define EFI_SHIFT_STATE_INVALID		0x00000000
#define EFI_RIGHT_SHIFT_PRESSED		0x00000001
#define EFI_LEFT_SHIFT_PRESSED		0x00000002
#define EFI_RIGHT_CONTROL_PRESSED	0x00000004
#define EFI_LEFT_CONTROL_PRESSED	0x00000008
#define EFI_RIGHT_ALT_PRESSED		0x00000010
#define EFI_LEFT_ALT_PRESSED		0x00000020
#define EFI_RIGHT_LOGO_PRESSED		0x00000040
#define EFI_LEFT_LOGO_PRESSED		0x00000080
#define EFI_MENU_KEY_PRESSED		0x00000100
#define EFI_SYS_REQ_PRESSED		0x00000200
#define EFI_SHIFT_STATE_VALID		0x80000000

#define EFI_TOGGLE_STATE_INVALID	0x00
#define EFI_SCROLL_LOCK_ACTIVE		0x01
#define EFI_NUM_LOCK_ACTIVE		0x02
#define EFI_CAPS_LOCK_ACTIVE		0x04
#define EFI_KEY_STATE_EXPOSED		0x40
#define EFI_TOGGLE_STATE_VALID		0x80

struct efi_key_state {
	u32 key_shift_state;
	u8 key_toggle_state;
};

struct efi_key_data {
	struct efi_input_key key;
	struct efi_key_state key_state;
};

struct efi_simple_text_input_ex_protocol {
	efi_status_t (EFIAPI *reset) (
		struct efi_simple_text_input_ex_protocol *this,
		bool extended_verification);
	efi_status_t (EFIAPI *read_key_stroke_ex) (
		struct efi_simple_text_input_ex_protocol *this,
		struct efi_key_data *key_data);
	struct efi_event *wait_for_key_ex;
	efi_status_t (EFIAPI *set_state) (
		struct efi_simple_text_input_ex_protocol *this,
		u8 key_toggle_state);
	efi_status_t (EFIAPI *register_key_notify) (
		struct efi_simple_text_input_ex_protocol *this,
		struct efi_key_data *key_data,
		efi_status_t (EFIAPI *key_notify_function)(
			struct efi_key_data *key_data),
		void **notify_handle);
	efi_status_t (EFIAPI *unregister_key_notify) (
		struct efi_simple_text_input_ex_protocol *this,
		void *notification_handle);
};

#define EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID \
	EFI_GUID(0x387477c1, 0x69c7, 0x11d2, \
		 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

struct efi_simple_text_input_protocol {
	efi_status_t(EFIAPI *reset)(struct efi_simple_text_input_protocol *this,
				    bool extended_verification);
	efi_status_t(EFIAPI *read_key_stroke)(
			struct efi_simple_text_input_protocol *this,
			struct efi_input_key *key);
	struct efi_event *wait_for_key;
};

#define EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID \
	EFI_GUID(0x8b843e20, 0x8132, 0x4852, \
		 0x90, 0xcc, 0x55, 0x1a, 0x4e, 0x4a, 0x7f, 0x1c)

struct efi_device_path_to_text_protocol {
	uint16_t *(EFIAPI *convert_device_node_to_text)(
			struct efi_device_path *device_node,
			bool display_only,
			bool allow_shortcuts);
	uint16_t *(EFIAPI *convert_device_path_to_text)(
			struct efi_device_path *device_path,
			bool display_only,
			bool allow_shortcuts);
};

#define EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID \
	EFI_GUID(0x0379be4e, 0xd706, 0x437d, \
		 0xb0, 0x37, 0xed, 0xb8, 0x2f, 0xb7, 0x72, 0xa4)

struct efi_device_path_utilities_protocol {
	efi_uintn_t (EFIAPI *get_device_path_size)(
		const struct efi_device_path *device_path);
	struct efi_device_path *(EFIAPI *duplicate_device_path)(
		const struct efi_device_path *device_path);
	struct efi_device_path *(EFIAPI *append_device_path)(
		const struct efi_device_path *src1,
		const struct efi_device_path *src2);
	struct efi_device_path *(EFIAPI *append_device_node)(
		const struct efi_device_path *device_path,
		const struct efi_device_path *device_node);
	struct efi_device_path *(EFIAPI *append_device_path_instance)(
		const struct efi_device_path *device_path,
		const struct efi_device_path *device_path_instance);
	struct efi_device_path *(EFIAPI *get_next_device_path_instance)(
		struct efi_device_path **device_path_instance,
		efi_uintn_t *device_path_instance_size);
	bool (EFIAPI *is_device_path_multi_instance)(
		const struct efi_device_path *device_path);
	struct efi_device_path *(EFIAPI *create_device_node)(
		uint8_t node_type,
		uint8_t node_sub_type,
		uint16_t node_length);
};

#define EFI_GOP_GUID \
	EFI_GUID(0x9042a9de, 0x23dc, 0x4a38, \
		 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a)

#define EFI_GOT_RGBA8		0
#define EFI_GOT_BGRA8		1
#define EFI_GOT_BITMASK		2

struct efi_gop_mode_info {
	u32 version;
	u32 width;
	u32 height;
	u32 pixel_format;
	u32 pixel_bitmask[4];
	u32 pixels_per_scanline;
};

struct efi_gop_mode {
	u32 max_mode;
	u32 mode;
	struct efi_gop_mode_info *info;
	unsigned long info_size;
	efi_physical_addr_t fb_base;
	unsigned long fb_size;
};

struct efi_gop_pixel {
	u8 blue;
	u8 green;
	u8 red;
	u8 reserved;
};

#define EFI_BLT_VIDEO_FILL		0
#define EFI_BLT_VIDEO_TO_BLT_BUFFER	1
#define EFI_BLT_BUFFER_TO_VIDEO		2
#define EFI_BLT_VIDEO_TO_VIDEO		3

struct efi_gop {
	efi_status_t (EFIAPI *query_mode)(struct efi_gop *this, u32 mode_number,
					  efi_uintn_t *size_of_info,
					  struct efi_gop_mode_info **info);
	efi_status_t (EFIAPI *set_mode)(struct efi_gop *this, u32 mode_number);
	efi_status_t (EFIAPI *blt)(struct efi_gop *this,
				   struct efi_gop_pixel *buffer,
				   u32 operation, efi_uintn_t sx,
				   efi_uintn_t sy, efi_uintn_t dx,
				   efi_uintn_t dy, efi_uintn_t width,
				   efi_uintn_t height, efi_uintn_t delta);
	struct efi_gop_mode *mode;
};

#define EFI_SIMPLE_NETWORK_GUID \
	EFI_GUID(0xa19832b9, 0xac25, 0x11d3, \
		 0x9a, 0x2d, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d)

struct efi_mac_address {
	char mac_addr[32];
};

struct efi_ip_address {
	u8 ip_addr[16];
} __attribute__((aligned(4)));

enum efi_simple_network_state {
	EFI_NETWORK_STOPPED,
	EFI_NETWORK_STARTED,
	EFI_NETWORK_INITIALIZED,
};

struct efi_simple_network_mode {
	enum efi_simple_network_state state;
	u32 hwaddr_size;
	u32 media_header_size;
	u32 max_packet_size;
	u32 nvram_size;
	u32 nvram_access_size;
	u32 receive_filter_mask;
	u32 receive_filter_setting;
	u32 max_mcast_filter_count;
	u32 mcast_filter_count;
	struct efi_mac_address mcast_filter[16];
	struct efi_mac_address current_address;
	struct efi_mac_address broadcast_address;
	struct efi_mac_address permanent_address;
	u8 if_type;
	u8 mac_changeable;
	u8 multitx_supported;
	u8 media_present_supported;
	u8 media_present;
};

/* receive_filters bit mask */
#define EFI_SIMPLE_NETWORK_RECEIVE_UNICAST               0x01
#define EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST             0x02
#define EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST             0x04
#define EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS           0x08
#define EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST 0x10

/* interrupt status bit mask */
#define EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT	0x01
#define EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT	0x02
#define EFI_SIMPLE_NETWORK_COMMAND_INTERRUPT	0x04
#define EFI_SIMPLE_NETWORK_SOFTWARE_INTERRUPT	0x08

/* revision of the simple network protocol */
#define EFI_SIMPLE_NETWORK_PROTOCOL_REVISION	0x00010000

struct efi_simple_network {
	u64 revision;
	efi_status_t (EFIAPI *start)(struct efi_simple_network *this);
	efi_status_t (EFIAPI *stop)(struct efi_simple_network *this);
	efi_status_t (EFIAPI *initialize)(struct efi_simple_network *this,
			ulong extra_rx, ulong extra_tx);
	efi_status_t (EFIAPI *reset)(struct efi_simple_network *this,
			int extended_verification);
	efi_status_t (EFIAPI *shutdown)(struct efi_simple_network *this);
	efi_status_t (EFIAPI *receive_filters)(struct efi_simple_network *this,
			u32 enable, u32 disable, int reset_mcast_filter,
			ulong mcast_filter_count,
			struct efi_mac_address *mcast_filter);
	efi_status_t (EFIAPI *station_address)(struct efi_simple_network *this,
			int reset, struct efi_mac_address *new_mac);
	efi_status_t (EFIAPI *statistics)(struct efi_simple_network *this,
			int reset, ulong *stat_size, void *stat_table);
	efi_status_t (EFIAPI *mcastiptomac)(struct efi_simple_network *this,
			int ipv6, struct efi_ip_address *ip,
			struct efi_mac_address *mac);
	efi_status_t (EFIAPI *nvdata)(struct efi_simple_network *this,
			int read_write, ulong offset, ulong buffer_size,
			char *buffer);
	efi_status_t (EFIAPI *get_status)(struct efi_simple_network *this,
			u32 *int_status, void **txbuf);
	efi_status_t (EFIAPI *transmit)(struct efi_simple_network *this,
			size_t header_size, size_t buffer_size, void *buffer,
			struct efi_mac_address *src_addr,
			struct efi_mac_address *dest_addr, u16 *protocol);
	efi_status_t (EFIAPI *receive)(struct efi_simple_network *this,
			size_t *header_size, size_t *buffer_size, void *buffer,
			struct efi_mac_address *src_addr,
			struct efi_mac_address *dest_addr, u16 *protocol);
	struct efi_event *wait_for_packet;
	struct efi_simple_network_mode *mode;
};

#define EFI_PXE_GUID \
	EFI_GUID(0x03c4e603, 0xac28, 0x11d3, \
		 0x9a, 0x2d, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d)

struct efi_pxe_packet {
	u8 packet[1472];
};

struct efi_pxe_mode {
	u8 started;
	u8 ipv6_available;
	u8 ipv6_supported;
	u8 using_ipv6;
	u8 bis_supported;
	u8 bis_detected;
	u8 auto_arp;
	u8 send_guid;
	u8 dhcp_discover_valid;
	u8 dhcp_ack_received;
	u8 proxy_offer_received;
	u8 pxe_discover_valid;
	u8 pxe_reply_received;
	u8 pxe_bis_reply_received;
	u8 icmp_error_received;
	u8 tftp_error_received;
	u8 make_callbacks;
	u8 ttl;
	u8 tos;
	u8 pad;
	struct efi_ip_address station_ip;
	struct efi_ip_address subnet_mask;
	struct efi_pxe_packet dhcp_discover;
	struct efi_pxe_packet dhcp_ack;
	struct efi_pxe_packet proxy_offer;
	struct efi_pxe_packet pxe_discover;
	struct efi_pxe_packet pxe_reply;
};

struct efi_pxe {
	u64 rev;
	void (EFIAPI *start)(void);
	void (EFIAPI *stop)(void);
	void (EFIAPI *dhcp)(void);
	void (EFIAPI *discover)(void);
	void (EFIAPI *mftp)(void);
	void (EFIAPI *udpwrite)(void);
	void (EFIAPI *udpread)(void);
	void (EFIAPI *setipfilter)(void);
	void (EFIAPI *arp)(void);
	void (EFIAPI *setparams)(void);
	void (EFIAPI *setstationip)(void);
	void (EFIAPI *setpackets)(void);
	struct efi_pxe_mode *mode;
};

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
	EFI_GUID(0x964e5b22, 0x6459, 0x11d2, \
		 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b)
#define EFI_FILE_PROTOCOL_REVISION 0x00010000

struct efi_file_handle {
	u64 rev;
	efi_status_t (EFIAPI *open)(struct efi_file_handle *file,
			struct efi_file_handle **new_handle,
			u16 *file_name, u64 open_mode, u64 attributes);
	efi_status_t (EFIAPI *close)(struct efi_file_handle *file);
	efi_status_t (EFIAPI *delete)(struct efi_file_handle *file);
	efi_status_t (EFIAPI *read)(struct efi_file_handle *file,
			efi_uintn_t *buffer_size, void *buffer);
	efi_status_t (EFIAPI *write)(struct efi_file_handle *file,
			efi_uintn_t *buffer_size, void *buffer);
	efi_status_t (EFIAPI *getpos)(struct efi_file_handle *file,
				      u64 *pos);
	efi_status_t (EFIAPI *setpos)(struct efi_file_handle *file,
				      u64 pos);
	efi_status_t (EFIAPI *getinfo)(struct efi_file_handle *file,
			const efi_guid_t *info_type, efi_uintn_t *buffer_size,
			void *buffer);
	efi_status_t (EFIAPI *setinfo)(struct efi_file_handle *file,
			const efi_guid_t *info_type, efi_uintn_t buffer_size,
			void *buffer);
	efi_status_t (EFIAPI *flush)(struct efi_file_handle *file);
};

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
	EFI_GUID(0x964e5b22, 0x6459, 0x11d2, \
		 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b)
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION 0x00010000

struct efi_simple_file_system_protocol {
	u64 rev;
	efi_status_t (EFIAPI *open_volume)(struct efi_simple_file_system_protocol *this,
			struct efi_file_handle **root);
};

#define EFI_FILE_INFO_GUID \
	EFI_GUID(0x9576e92, 0x6d3f, 0x11d2, \
		 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define EFI_FILE_SYSTEM_INFO_GUID \
	EFI_GUID(0x09576e93, 0x6d3f, 0x11d2, \
		 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define EFI_FILE_MODE_READ	0x0000000000000001
#define EFI_FILE_MODE_WRITE	0x0000000000000002
#define EFI_FILE_MODE_CREATE	0x8000000000000000

#define EFI_FILE_READ_ONLY	0x0000000000000001
#define EFI_FILE_HIDDEN		0x0000000000000002
#define EFI_FILE_SYSTEM		0x0000000000000004
#define EFI_FILE_RESERVED	0x0000000000000008
#define EFI_FILE_DIRECTORY	0x0000000000000010
#define EFI_FILE_ARCHIVE	0x0000000000000020
#define EFI_FILE_VALID_ATTR	0x0000000000000037

struct efi_file_info {
	u64 size;
	u64 file_size;
	u64 physical_size;
	struct efi_time create_time;
	struct efi_time last_access_time;
	struct efi_time modification_time;
	u64 attribute;
	u16 file_name[0];
};

struct efi_file_system_info {
	u64 size;
	u8 read_only;
	u64 volume_size;
	u64 free_space;
	u32 block_size;
	u16 volume_label[0];
};

#define EFI_DRIVER_BINDING_PROTOCOL_GUID \
	EFI_GUID(0x18a031ab, 0xb443, 0x4d1a,\
		 0xa5, 0xc0, 0x0c, 0x09, 0x26, 0x1e, 0x9f, 0x71)
struct efi_driver_binding_protocol {
	efi_status_t (EFIAPI * supported)(
			struct efi_driver_binding_protocol *this,
			efi_handle_t controller_handle,
			struct efi_device_path *remaining_device_path);
	efi_status_t (EFIAPI * start)(
			struct efi_driver_binding_protocol *this,
			efi_handle_t controller_handle,
			struct efi_device_path *remaining_device_path);
	efi_status_t (EFIAPI * stop)(
			struct efi_driver_binding_protocol *this,
			efi_handle_t controller_handle,
			efi_uintn_t number_of_children,
			efi_handle_t *child_handle_buffer);
	u32 version;
	efi_handle_t image_handle;
	efi_handle_t driver_binding_handle;
};

#define EFI_UNICODE_COLLATION_PROTOCOL2_GUID \
	EFI_GUID(0xa4c751fc, 0x23ae, 0x4c3e, \
		 0x92, 0xe9, 0x49, 0x64, 0xcf, 0x63, 0xf3, 0x49)
struct efi_unicode_collation_protocol {
	efi_intn_t (EFIAPI *stri_coll)(
		struct efi_unicode_collation_protocol *this, u16 *s1, u16 *s2);
	bool (EFIAPI *metai_match)(struct efi_unicode_collation_protocol *this,
				   const u16 *string, const u16 *patter);
	void (EFIAPI *str_lwr)(struct efi_unicode_collation_protocol
			       *this, u16 *string);
	void (EFIAPI *str_upr)(struct efi_unicode_collation_protocol *this,
			       u16 *string);
	void (EFIAPI *fat_to_str)(struct efi_unicode_collation_protocol *this,
				  efi_uintn_t fat_size, char *fat, u16 *string);
	bool (EFIAPI *str_to_fat)(struct efi_unicode_collation_protocol *this,
				  const u16 *string, efi_uintn_t fat_size,
				  char *fat);
	char *supported_languages;
};

#endif
