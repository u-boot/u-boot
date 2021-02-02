/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Defines data structures and APIs that allow an OS to interact with UEFI
 * firmware to query information about the device
 *
 * Copyright (c) 2020, Linaro Limited
 */

#if !defined _EFI_TCG2_PROTOCOL_H_
#define _EFI_TCG2_PROTOCOL_H_

#include <tpm-v2.h>

#define EFI_TCG2_PROTOCOL_GUID \
	EFI_GUID(0x607f766c, 0x7455, 0x42be, 0x93, \
		 0x0b, 0xe4, 0xd7, 0x6d, 0xb2, 0x72, 0x0f)

/* TPMV2 only */
#define TCG2_EVENT_LOG_FORMAT_TCG_2 0x00000002

/* Algorithm Registry */
#define EFI_TCG2_BOOT_HASH_ALG_SHA1    0x00000001
#define EFI_TCG2_BOOT_HASH_ALG_SHA256  0x00000002
#define EFI_TCG2_BOOT_HASH_ALG_SHA384  0x00000004
#define EFI_TCG2_BOOT_HASH_ALG_SHA512  0x00000008
#define EFI_TCG2_BOOT_HASH_ALG_SM3_256 0x00000010

typedef u32 efi_tcg_event_log_bitmap;
typedef u32 efi_tcg_event_log_format;
typedef u32 efi_tcg_event_algorithm_bitmap;

struct efi_tcg2_version {
	u8 major;
	u8 minor;
};

struct efi_tcg2_event_header {
	u32 header_size;
	u16 header_version;
	u32 pcr_index;
	u32 event_type;
} __packed;

struct efi_tcg2_event {
	u32 size;
	struct efi_tcg2_event_header header;
	u8 event[];
} __packed;

struct efi_tcg2_boot_service_capability {
	u8 size;
	struct efi_tcg2_version structure_version;
	struct efi_tcg2_version protocol_version;
	efi_tcg_event_algorithm_bitmap hash_algorithm_bitmap;
	efi_tcg_event_log_bitmap supported_event_logs;
	u8 tpm_present_flag;
	u16 max_command_size;
	u16 max_response_size;
	u32 manufacturer_id;
	u32 number_of_pcr_banks;
	efi_tcg_event_algorithm_bitmap active_pcr_banks;
};

#define boot_service_capability_min \
	sizeof(struct efi_tcg2_boot_service_capability) - \
	offsetof(struct efi_tcg2_boot_service_capability, number_of_pcr_banks)

struct efi_tcg2_protocol {
	efi_status_t (EFIAPI * get_capability)(struct efi_tcg2_protocol *this,
					       struct efi_tcg2_boot_service_capability *capability);
	efi_status_t (EFIAPI * get_eventlog)(struct efi_tcg2_protocol *this,
					     efi_tcg_event_log_format log_format,
					     u64 *event_log_location, u64 *event_log_last_entry,
					     bool *event_log_truncated);
	efi_status_t (EFIAPI * hash_log_extend_event)(struct efi_tcg2_protocol *this,
						      u64 flags, u64 data_to_hash,
						      u64 data_to_hash_len,
						      struct efi_tcg2_event *efi_tcg_event);
	efi_status_t (EFIAPI * submit_command)(struct efi_tcg2_protocol *this,
					       u32 input_parameter_block_size,
					       u8 *input_parameter_block,
					       u32 output_parameter_block_size,
					       u8 *output_parameter_block);
	efi_status_t (EFIAPI * get_active_pcr_banks)(struct efi_tcg2_protocol *this,
						     u32 *active_pcr_banks);
	efi_status_t (EFIAPI * set_active_pcr_banks)(struct efi_tcg2_protocol *this,
						     u32 active_pcr_banks);
	efi_status_t (EFIAPI * get_result_of_set_active_pcr_banks)(struct efi_tcg2_protocol *this,
								   u32 *operation_present,
								   u32 *response);
};
#endif
