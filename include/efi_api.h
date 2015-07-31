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

/* EFI Boot Services table */
struct efi_boot_services {
	struct efi_table_hdr hdr;
	void *raise_tpl;
	void *restore_tpl;

	efi_status_t (EFIAPI *allocate_pages)(int, int, unsigned long,
					      efi_physical_addr_t *);
	efi_status_t (EFIAPI *free_pages)(efi_physical_addr_t, unsigned long);
	efi_status_t (EFIAPI *get_memory_map)(unsigned long *memory_map_size,
			struct efi_mem_desc *desc, unsigned long *key,
			unsigned long *desc_size, u32 *desc_version);
	efi_status_t (EFIAPI *allocate_pool)(int, unsigned long, void **);
	efi_status_t (EFIAPI *free_pool)(void *);

	void *create_event;
	void *set_timer;
	efi_status_t(EFIAPI *wait_for_event)(unsigned long number_of_events,
					     void *event, unsigned long *index);
	void *signal_event;
	void *close_event;
	void *check_event;

	void *install_protocol_interface;
	void *reinstall_protocol_interface;
	void *uninstall_protocol_interface;
	efi_status_t (EFIAPI *handle_protocol)(efi_handle_t, efi_guid_t *,
					       void **);
	void *reserved;
	void *register_protocol_notify;
	efi_status_t (EFIAPI *locate_handle)(
			enum efi_locate_search_type search_type,
			efi_guid_t *protocol, void *search_key,
			unsigned long *buffer_size, efi_handle_t *buffer);
	efi_status_t (EFIAPI *locate_device_path)(efi_guid_t *protocol,
			struct efi_device_path **device_path,
			efi_handle_t *device);
	void *install_configuration_table;

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
	void *unload_image;
	efi_status_t (EFIAPI *exit_boot_services)(efi_handle_t, unsigned long);

	efi_status_t (EFIAPI *get_next_monotonic_count)(u64 *count);
	efi_status_t (EFIAPI *stall)(unsigned long usecs);
	void *set_watchdog_timer;
	efi_status_t(EFIAPI *connect_controller)(efi_handle_t controller_handle,
			efi_handle_t *driver_image_handle,
			struct efi_device_path *remaining_device_path,
			bool recursive);
	void *disconnect_controller;
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
	void *close_protocol;
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
	void *locate_protocol;
	void *install_multiple_protocol_interfaces;
	void *uninstall_multiple_protocol_interfaces;
	void *calculate_crc32;
	void *copy_mem;
	void *set_mem;
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
	void *get_time;
	void *set_time;
	void *get_wakeup_time;
	void *set_wakeup_time;
	void *set_virtual_address_map;
	void *convert_pointer;
	efi_status_t (EFIAPI *get_variable)(s16 *variable_name,
			efi_guid_t *vendor, u32 *attributes,
			unsigned long *data_size, void *data);
	efi_status_t (EFIAPI *get_next_variable)(
			unsigned long *variable_name_size,
			s16 *variable_name, efi_guid_t *vendor);
	efi_status_t (EFIAPI *set_variable)(s16 *variable_name,
			efi_guid_t *vendor, u32 attributes,
			unsigned long data_size, void *data);
	void *get_next_high_mono_count;
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

struct efi_system_table {
	struct efi_table_hdr hdr;
	unsigned long fw_vendor;   /* physical addr of wchar_t vendor string */
	u32 fw_revision;
	unsigned long con_in_handle;
	struct efi_simple_input_interface *con_in;
	unsigned long con_out_handle;
	struct efi_simple_text_output_protocol *con_out;
	unsigned long stderr_handle;
	unsigned long std_err;
	struct efi_runtime_services *runtime;
	struct efi_boot_services *boottime;
	unsigned long nr_tables;
	unsigned long tables;
};

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

struct efi_device_path {
	u8 type;
	u8 sub_type;
	u16 length;
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
	void *test_string;

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
	efi_status_t(EFIAPI *enable_cursor)(void *, bool enable);
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

#endif
