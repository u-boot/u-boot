/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Defines APIs and structures that adhere to
 * https://trustedcomputinggroup.org/resource/pc-client-specific-platform-firmware-profile-specification/
 * https://trustedcomputinggroup.org/resource/tcg-efi-protocol-specification/
 *
 * Copyright (c) 2020 Linaro Limited
 */

#ifndef __TPM_TCG_V2_H
#define __TPM_TCG_V2_H

#include <tpm-v2.h>

/*
 * event types, cf.
 * "TCG Server Management Domain Firmware Profile Specification",
 * rev 1.00, 2020-05-01
 */
#define EV_POST_CODE			((u32)0x00000001)
#define EV_NO_ACTION			((u32)0x00000003)
#define EV_SEPARATOR			((u32)0x00000004)
#define EV_ACTION			((u32)0x00000005)
#define EV_TAG				((u32)0x00000006)
#define EV_S_CRTM_CONTENTS		((u32)0x00000007)
#define EV_S_CRTM_VERSION		((u32)0x00000008)
#define EV_CPU_MICROCODE		((u32)0x00000009)
#define EV_PLATFORM_CONFIG_FLAGS	((u32)0x0000000A)
#define EV_TABLE_OF_DEVICES		((u32)0x0000000B)
#define EV_COMPACT_HASH			((u32)0x0000000C)

/*
 * event types, cf.
 * "TCG PC Client Platform Firmware Profile Specification", Family "2.0"
 * Level 00 Version 1.05 Revision 23, May 7, 2021
 */
#define EV_EFI_EVENT_BASE			((u32)0x80000000)
#define EV_EFI_VARIABLE_DRIVER_CONFIG		((u32)0x80000001)
#define EV_EFI_VARIABLE_BOOT			((u32)0x80000002)
#define EV_EFI_BOOT_SERVICES_APPLICATION	((u32)0x80000003)
#define EV_EFI_BOOT_SERVICES_DRIVER		((u32)0x80000004)
#define EV_EFI_RUNTIME_SERVICES_DRIVER		((u32)0x80000005)
#define EV_EFI_GPT_EVENT			((u32)0x80000006)
#define EV_EFI_ACTION				((u32)0x80000007)
#define EV_EFI_PLATFORM_FIRMWARE_BLOB		((u32)0x80000008)
#define EV_EFI_HANDOFF_TABLES			((u32)0x80000009)
#define EV_EFI_PLATFORM_FIRMWARE_BLOB2		((u32)0x8000000A)
#define EV_EFI_HANDOFF_TABLES2			((u32)0x8000000B)
#define EV_EFI_VARIABLE_BOOT2			((u32)0x8000000C)
#define EV_EFI_HCRTM_EVENT			((u32)0x80000010)
#define EV_EFI_VARIABLE_AUTHORITY		((u32)0x800000E0)
#define EV_EFI_SPDM_FIRMWARE_BLOB		((u32)0x800000E1)
#define EV_EFI_SPDM_FIRMWARE_CONFIG		((u32)0x800000E2)

#define EFI_CALLING_EFI_APPLICATION         \
	"Calling EFI Application from Boot Option"
#define EFI_RETURNING_FROM_EFI_APPLICATION  \
	"Returning from EFI Application from Boot Option"
#define EFI_EXIT_BOOT_SERVICES_INVOCATION   \
	"Exit Boot Services Invocation"
#define EFI_EXIT_BOOT_SERVICES_FAILED       \
	"Exit Boot Services Returned with Failure"
#define EFI_EXIT_BOOT_SERVICES_SUCCEEDED    \
	"Exit Boot Services Returned with Success"
#define EFI_DTB_EVENT_STRING \
	"DTB DATA"

/**
 *  struct TCG_EfiSpecIdEventAlgorithmSize - hashing algorithm information
 *
 *  @algorithm_id:	algorithm defined in enum tpm2_algorithms
 *  @digest_size:	size of the algorithm
 */
struct tcg_efi_spec_id_event_algorithm_size {
	u16      algorithm_id;
	u16      digest_size;
} __packed;

/**
 * SHA1 Event Log Entry Format
 *
 * @pcr_index:	PCRIndex event extended to
 * @event_type:	Type of event (see EFI specs)
 * @digest:	Value extended into PCR index
 * @event_size:	Size of event
 * @event:	Event data
 */
struct tcg_pcr_event {
	u32 pcr_index;
	u32 event_type;
	u8 digest[TPM2_SHA1_DIGEST_SIZE];
	u32 event_size;
	u8 event[];
} __packed;

/**
 * Crypto Agile Log Entry Format
 *
 * @pcr_index:	PCRIndex event extended to
 * @event_type:	Type of event
 * @digests:	List of digestsextended to PCR index
 * @event_size: Size of the event data
 * @event:	Event data
 */
struct tcg_pcr_event2 {
	u32 pcr_index;
	u32 event_type;
	struct tpml_digest_values digests;
	u32 event_size;
	u8 event[];
} __packed;

/**
 * struct TCG_EfiSpecIDEventStruct - content of the event log header
 *
 * @signature:			signature, set to Spec ID Event03
 * @platform_class:		class defined in TCG ACPI Specification
 *				Client  Common Header.
 * @spec_version_minor:		minor version
 * @spec_version_major:		major version
 * @spec_version_errata:	major version
 * @uintn_size:			size of the efi_uintn_t fields used in various
 *				data structures used in this specification.
 *				0x01 indicates u32  and 0x02  indicates u64
 * @number_of_algorithms:	hashing algorithms used in this event log
 * @digest_sizes:		array of number_of_algorithms pairs
 *				1st member defines the algorithm id
 *				2nd member defines the algorithm size
 */
struct tcg_efi_spec_id_event {
	u8 signature[16];
	u32 platform_class;
	u8 spec_version_minor;
	u8 spec_version_major;
	u8 spec_errata;
	u8 uintn_size;
	u32 number_of_algorithms;
	struct tcg_efi_spec_id_event_algorithm_size digest_sizes[];
} __packed;

#define TCG_EFI_SPEC_ID_EVENT_SIGNATURE_03 "Spec ID Event03"
#define TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MAJOR_TPM2 2
#define TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MINOR_TPM2 0
#define TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_ERRATA_TPM2 2

#endif /* __TPM_TCG_V2_H */
