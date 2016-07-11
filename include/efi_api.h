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

/* Types and defines for EFI CreateEvent */
enum efi_event_type {
	EFI_TIMER_STOP = 0,
	EFI_TIMER_PERIODIC = 1,
	EFI_TIMER_RELATIVE = 2
};

/* EFI Boot Services table */
struct efi_boot_services {
	struct efi_table_hdr hdr;
	efi_status_t (EFIAPI *raise_tpl)(unsigned long new_tpl);
	void (EFIAPI *restore_tpl)(unsigned long old_tpl);

	efi_status_t (EFIAPI *allocate_pages)(int, int, unsigned long,
					      efi_physical_addr_t *);
	efi_status_t (EFIAPI *free_pages)(efi_physical_addr_t, unsigned long);
	efi_status_t (EFIAPI *get_memory_map)(unsigned long *memory_map_size,
			struct efi_mem_desc *desc, unsigned long *key,
			unsigned long *desc_size, u32 *desc_version);
	efi_status_t (EFIAPI *allocate_pool)(int, unsigned long, void **);
	efi_status_t (EFIAPI *free_pool)(void *);

	efi_status_t (EFIAPI *create_event)(enum efi_event_type type,
			unsigned long notify_tpl,
			void (EFIAPI *notify_function) (void *event,
							void *context),
			void *notify_context, void **event);
	efi_status_t (EFIAPI *set_timer)(void *event, int type,
			uint64_t trigger_time);
	efi_status_t (EFIAPI *wait_for_event)(unsigned long number_of_events,
			void *event, unsigned long *index);
	efi_status_t (EFIAPI *signal_event)(void *event);
	efi_status_t (EFIAPI *close_event)(void *event);
	efi_status_t (EFIAPI *check_event)(void *event);

	efi_status_t (EFIAPI *install_protocol_interface)(
			void **handle, efi_guid_t *protocol,
			int protocol_interface_type, void *protocol_interface);
	efi_status_t (EFIAPI *reinstall_protocol_interface)(
			void *handle, efi_guid_t *protocol,
			void *old_interface, void *new_interface);
	efi_status_t (EFIAPI *uninstall_protocol_interface)(void *handle,
			efi_guid_t *protocol, void *protocol_interface);
	efi_status_t (EFIAPI *handle_protocol)(efi_handle_t, efi_guid_t *,
					       void **);
	void *reserved;
	efi_status_t (EFIAPI *register_protocol_notify)(
			efi_guid_t *protocol, void *event,
			void **registration);
	efi_status_t (EFIAPI *locate_handle)(
			enum efi_locate_search_type search_type,
			efi_guid_t *protocol, void *search_key,
			unsigned long *buffer_size, efi_handle_t *buffer);
	efi_status_t (EFIAPI *locate_device_path)(efi_guid_t *protocol,
			struct efi_device_path **device_path,
			efi_handle_t *device);
	efi_status_t (EFIAPI *install_configuration_table)(
			efi_guid_t *guid, void *table);

	efi_status_t (EFIAPI *load_image)(bool boot_policiy,
			efi_handle_t parent_image,
			struct efi_device_path *file_path, void *source_buffer,
			unsigned long source_size, efi_handle_t *image);
	efi_status_t (EFIAPI *start_image)(efi_handle_t handle,
					   unsigned long *exitdata_size,
					   s16 **exitdata);
	efi_status_t (EFIAPI *exit)(efi_handle_t handle,
				    efi_status_t exit_status,
				    unsigned long exitdata_size, s16 *exitdata);
	efi_status_t (EFIAPI *unload_image)(void *image_handle);
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
	efi_status_t (EFIAPI *disconnect_controller)(void *controller_handle,
			void *driver_image_handle, void *child_handle);
#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020
	efi_status_t (EFIAPI *open_protocol)(efi_handle_t handle,
			efi_guid_t *protocol, void **interface,
			efi_handle_t agent_handle,
			efi_handle_t controller_handle, u32 attributes);
	efi_status_t (EFIAPI *close_protocol)(void *handle,
			efi_guid_t *protocol, void *agent_handle,
			void *controller_handle);
	efi_status_t(EFIAPI *open_protocol_information)(efi_handle_t handle,
			efi_guid_t *protocol,
			struct efi_open_protocol_info_entry **entry_buffer,
			unsigned long *entry_count);
	efi_status_t (EFIAPI *protocols_per_handle)(efi_handle_t handle,
			efi_guid_t ***protocol_buffer,
			unsigned long *protocols_buffer_count);
	efi_status_t (EFIAPI *locate_handle_buffer) (
			enum efi_locate_search_type search_type,
			efi_guid_t *protocol, void *search_key,
			unsigned long *no_handles, efi_handle_t **buffer);
	efi_status_t (EFIAPI *locate_protocol)(efi_guid_t *protocol,
			void *registration, void **protocol_interface);
	efi_status_t (EFIAPI *install_multiple_protocol_interfaces)(
			void **handle, ...);
	efi_status_t (EFIAPI *uninstall_multiple_protocol_interfaces)(
			void *handle, ...);
	efi_status_t (EFIAPI *calculate_crc32)(void *data,
			unsigned long data_size, uint32_t *crc32);
	void (EFIAPI *copy_mem)(void *destination, void *source,
			unsigned long length);
	void (EFIAPI *set_mem)(void *buffer, unsigned long size,
			uint8_t value);
	void *create_event_ex;
};

/* Types and defines for EFI ResetSystem */
enum efi_reset_type {
	EFI_RESET_COLD = 0,
	EFI_RESET_WARM = 1,
	EFI_RESET_SHUTDOWN = 2
};

/* EFI Runtime Services table */
#define EFI_RUNTIME_SERVICES_SIGNATURE	0x5652453544e5552ULL
#define EFI_RUNTIME_SERVICES_REVISION	0x00010000

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
	efi_status_t (EFIAPI *get_variable)(s16 *variable_name,
			efi_guid_t *vendor, u32 *attributes,
			unsigned long *data_size, void *data);
	efi_status_t (EFIAPI *get_next_variable)(
			unsigned long *variable_name_size,
			s16 *variable_name, efi_guid_t *vendor);
	efi_status_t (EFIAPI *set_variable)(s16 *variable_name,
			efi_guid_t *vendor, u32 attributes,
			unsigned long data_size, void *data);
	efi_status_t (EFIAPI *get_next_high_mono_count)(
			uint32_t *high_count);
	void (EFIAPI *reset_system)(enum efi_reset_type reset_type,
				    efi_status_t reset_status,
				    unsigned long data_size, void *reset_data);
	void *update_capsule;
	void *query_capsule_caps;
	void *query_variable_info;
};

/* EFI Configuration Table and GUID definitions */
#define NULL_GUID \
	EFI_GUID(0x00000000, 0x0000, 0x0000, 0x00, 0x00, \
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)

#define LOADED_IMAGE_PROTOCOL_GUID \
	EFI_GUID(0x5b1b31a1, 0x9562, 0x11d2, 0x8e, 0x3f, \
		 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define EFI_FDT_GUID \
	EFI_GUID(0xb1b621d5, 0xf19c, 0x41a5, \
		 0x83, 0x0b, 0xd9, 0x15, 0x2c, 0x69, 0xaa, 0xe0)

struct efi_configuration_table
{
	efi_guid_t guid;
	void *table;
};

#define EFI_SYSTEM_TABLE_SIGNATURE ((u64)0x5453595320494249ULL)

struct efi_system_table {
	struct efi_table_hdr hdr;
	unsigned long fw_vendor;   /* physical addr of wchar_t vendor string */
	u32 fw_revision;
	unsigned long con_in_handle;
	struct efi_simple_input_interface *con_in;
	unsigned long con_out_handle;
	struct efi_simple_text_output_protocol *con_out;
	unsigned long stderr_handle;
	struct efi_simple_text_output_protocol *std_err;
	struct efi_runtime_services *runtime;
	struct efi_boot_services *boottime;
	unsigned long nr_tables;
	struct efi_configuration_table *tables;
};

#define LOADED_IMAGE_GUID \
	EFI_GUID(0x5b1b31a1, 0x9562, 0x11d2, \
		 0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

struct efi_loaded_image {
	u32 revision;
	void *parent_handle;
	struct efi_system_table *system_table;
	void *device_handle;
	void *file_path;
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
		 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b )

#define DEVICE_PATH_TYPE_END			0x7f
#  define DEVICE_PATH_SUB_TYPE_END		0xff

struct efi_device_path {
	u8 type;
	u8 sub_type;
	u16 length;
};

#define DEVICE_PATH_TYPE_MEDIA_DEVICE		0x04
#  define DEVICE_PATH_SUB_TYPE_FILE_PATH	0x04

struct efi_device_path_file_path {
	struct efi_device_path dp;
	u16 str[32];
};

#define BLOCK_IO_GUID \
	EFI_GUID(0x964e5b21, 0x6459, 0x11d2, \
		 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

struct efi_block_io_media
{
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
};

struct efi_block_io {
	u64 revision;
	struct efi_block_io_media *media;
	efi_status_t (EFIAPI *reset)(struct efi_block_io *this,
			char extended_verification);
	efi_status_t (EFIAPI *read_blocks)(struct efi_block_io *this,
			u32 media_id, u64 lba, unsigned long buffer_size,
			void *buffer);
	efi_status_t (EFIAPI *write_blocks)(struct efi_block_io *this,
			u32 media_id, u64 lba, unsigned long buffer_size,
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

struct efi_simple_text_output_protocol {
	void *reset;
	efi_status_t (EFIAPI *output_string)(
			struct efi_simple_text_output_protocol *this,
			const unsigned short *str);
	efi_status_t (EFIAPI *test_string)(
			struct efi_simple_text_output_protocol *this,
			const unsigned short *str);
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

struct efi_input_key {
	u16 scan_code;
	s16 unicode_char;
};

struct efi_simple_input_interface {
	efi_status_t(EFIAPI *reset)(struct efi_simple_input_interface *this,
			bool ExtendedVerification);
	efi_status_t(EFIAPI *read_key_stroke)(
			struct efi_simple_input_interface *this,
			struct efi_input_key *key);
	void *wait_for_key;
};

#define CONSOLE_CONTROL_GUID \
	EFI_GUID(0xf42f7782, 0x12e, 0x4c12, \
		 0x99, 0x56, 0x49, 0xf9, 0x43, 0x4, 0xf7, 0x21)
#define EFI_CONSOLE_MODE_TEXT	0
#define EFI_CONSOLE_MODE_GFX	1

struct efi_console_control_protocol
{
	efi_status_t (EFIAPI *get_mode)(
			struct efi_console_control_protocol *this, int *mode,
			char *uga_exists, char *std_in_locked);
	efi_status_t (EFIAPI *set_mode)(
			struct efi_console_control_protocol *this, int mode);
	efi_status_t (EFIAPI *lock_std_in)(
			struct efi_console_control_protocol *this,
			uint16_t *password);
};

#define EFI_GOP_GUID \
	EFI_GUID(0x9042a9de, 0x23dc, 0x4a38, \
		 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a)

#define EFI_GOT_RGBA8		0
#define EFI_GOT_BGRA8		1
#define EFI_GOT_BITMASK		2

struct efi_gop_mode_info
{
	u32 version;
	u32 width;
	u32 height;
	u32 pixel_format;
	u32 pixel_bitmask[4];
	u32 pixels_per_scanline;
};

struct efi_gop_mode
{
	u32 max_mode;
	u32 mode;
	struct efi_gop_mode_info *info;
	unsigned long info_size;
	efi_physical_addr_t fb_base;
	unsigned long fb_size;
};

#define EFI_BLT_VIDEO_FILL		0
#define EFI_BLT_VIDEO_TO_BLT_BUFFER	1
#define EFI_BLT_BUFFER_TO_VIDEO		2
#define EFI_BLT_VIDEO_TO_VIDEO		3

struct efi_gop
{
	efi_status_t (EFIAPI *query_mode)(struct efi_gop *this, u32 mode_number,
					  unsigned long *size_of_info,
					  struct efi_gop_mode_info **info);
	efi_status_t (EFIAPI *set_mode)(struct efi_gop *this, u32 mode_number);
	efi_status_t (EFIAPI *blt)(struct efi_gop *this, void *buffer,
				   unsigned long operation, unsigned long sx,
				   unsigned long sy, unsigned long dx,
				   unsigned long dy, unsigned long width,
				   unsigned long height, unsigned long delta);
	struct efi_gop_mode *mode;
};

#endif
