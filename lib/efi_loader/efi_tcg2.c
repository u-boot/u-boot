// SPDX-License-Identifier: GPL-2.0+
/*
 * Defines APIs that allow an OS to interact with UEFI firmware to query
 * information about the device.
 * https://trustedcomputinggroup.org/resource/tcg-efi-protocol-specification/
 *
 * Copyright (c) 2020, Linaro Limited
 */

#define LOG_CATEGORY LOGC_EFI
#include <common.h>
#include <dm.h>
#include <efi_loader.h>
#include <efi_tcg2.h>
#include <log.h>
#include <tpm-v2.h>
#include <linux/unaligned/access_ok.h>
#include <linux/unaligned/generic.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * When requesting TPM2_CAP_TPM_PROPERTIES the value is on a standard offset.
 * Since the current tpm2_get_capability() response buffers starts at
 * 'union tpmu_capabilities data' of 'struct tpms_capability_data', calculate
 * the response size and offset once for all consumers
 */
#define TPM2_RESPONSE_BUFFER_SIZE (sizeof(struct tpms_capability_data) - \
				   offsetof(struct tpms_capability_data, data))
#define properties_offset (offsetof(struct tpml_tagged_tpm_property, tpm_property) + \
			   offsetof(struct tpms_tagged_property, value))

struct {
	u16 hash_alg;
	u32 hash_mask;
} hash_algo_list[] = {
	{
		TPM2_ALG_SHA1,
		EFI_TCG2_BOOT_HASH_ALG_SHA1,
	},
	{
		TPM2_ALG_SHA256,
		EFI_TCG2_BOOT_HASH_ALG_SHA256,
	},
	{
		TPM2_ALG_SHA384,
		EFI_TCG2_BOOT_HASH_ALG_SHA384,
	},
	{
		TPM2_ALG_SHA512,
		EFI_TCG2_BOOT_HASH_ALG_SHA512,
	},
	{
		TPM2_ALG_SM3_256,
		EFI_TCG2_BOOT_HASH_ALG_SM3_256,
	},
};

#define MAX_HASH_COUNT ARRAY_SIZE(hash_algo_list)
/**
 * alg_to_mask - Get a TCG hash mask for algorithms
 *
 * @hash_alg: TCG defined algorithm
 *
 * @Return: TCG hashing algorithm bitmaps, 0 if the algorithm is not supported
 */
static u32 alg_to_mask(u16 hash_alg)
{
	int i;

	for (i = 0; i < MAX_HASH_COUNT; i++) {
		if (hash_algo_list[i].hash_alg == hash_alg)
			return hash_algo_list[i].hash_mask;
	}

	return 0;
}

const efi_guid_t efi_guid_tcg2_protocol = EFI_TCG2_PROTOCOL_GUID;

/**
 * platform_get_tpm_device() - retrieve TPM device
 *
 * This function retrieves the udevice implementing a TPM
 *
 * This function may be overridden if special initialization is needed.
 *
 * @dev:	udevice
 * Return:	status code
 */
__weak efi_status_t platform_get_tpm2_device(struct udevice **dev)
{
	for_each_tpm_device(*dev) {
		/* Only support TPMv2 devices */
		if (tpm_get_version(*dev) == TPM_V2)
			return EFI_SUCCESS;
	}

	return EFI_NOT_FOUND;
}

/**
 * tpm2_get_max_command_size() - get the supported max command size
 *
 * @dev:		TPM device
 * @max_command_size:	output buffer for the size
 *
 * Return: 0 on success, -1 on error
 */
static int tpm2_get_max_command_size(struct udevice *dev, u16 *max_command_size)
{
	u8 response[TPM2_RESPONSE_BUFFER_SIZE];
	u32 ret;

	memset(response, 0, sizeof(response));
	ret = tpm2_get_capability(dev, TPM2_CAP_TPM_PROPERTIES,
				  TPM2_PT_MAX_COMMAND_SIZE, response, 1);
	if (ret)
		return -1;

	*max_command_size = (uint16_t)get_unaligned_be32(response +
							 properties_offset);

	return 0;
}

/**
 * tpm2_get_max_response_size() - get the supported max response size
 *
 * @dev:		TPM device
 * @max_response_size:	output buffer for the size
 *
 * Return: 0 on success, -1 on error
 */
static int tpm2_get_max_response_size(struct udevice *dev,
				      u16 *max_response_size)
{
	u8 response[TPM2_RESPONSE_BUFFER_SIZE];
	u32 ret;

	memset(response, 0, sizeof(response));
	ret = tpm2_get_capability(dev, TPM2_CAP_TPM_PROPERTIES,
				  TPM2_PT_MAX_RESPONSE_SIZE, response, 1);
	if (ret)
		return -1;

	*max_response_size = (uint16_t)get_unaligned_be32(response +
							  properties_offset);

	return 0;
}

/**
 * tpm2_get_manufacturer_id() - get the manufacturer ID
 *
 * @dev:		TPM device
 * @manufacturer_id:	output buffer for the id
 *
 * Return: 0 on success, -1 on error
 */
static int tpm2_get_manufacturer_id(struct udevice *dev, u32 *manufacturer_id)
{
	u8 response[TPM2_RESPONSE_BUFFER_SIZE];
	u32 ret;

	memset(response, 0, sizeof(response));
	ret = tpm2_get_capability(dev, TPM2_CAP_TPM_PROPERTIES,
				  TPM2_PT_MANUFACTURER, response, 1);
	if (ret)
		return -1;

	*manufacturer_id = get_unaligned_be32(response + properties_offset);

	return 0;
}

/**
 * tpm2_get_num_pcr() - get the number of PCRs
 *
 * @dev:		TPM device
 * @manufacturer_id:	output buffer for the number
 *
 * Return: 0 on success, -1 on error
 */
static int tpm2_get_num_pcr(struct udevice *dev, u32 *num_pcr)
{
	u8 response[TPM2_RESPONSE_BUFFER_SIZE];
	u32 ret;

	memset(response, 0, sizeof(response));
	ret = tpm2_get_capability(dev, TPM2_CAP_TPM_PROPERTIES,
				  TPM2_PT_PCR_COUNT, response, 1);
	if (ret)
		return -1;

	*num_pcr = get_unaligned_be32(response + properties_offset);
	if (*num_pcr > TPM2_MAX_PCRS)
		return -1;

	return 0;
}

/**
 * is_active_pcr() - Check if a supported algorithm is active
 *
 * @dev:		TPM device
 * @selection:		struct of PCR information
 *
 * Return: true if PCR is active
 */
bool is_active_pcr(struct tpms_pcr_selection *selection)
{
	int i;
	/*
	 * check the pcr_select. If at least one of the PCRs supports the
	 * algorithm add it on the active ones
	 */
	for (i = 0; i < selection->size_of_select; i++) {
		if (selection->pcr_select[i])
			return true;
	}

	return false;
}

/**
 * tpm2_get_pcr_info() - get the supported, active PCRs and number of banks
 *
 * @dev:		TPM device
 * @supported_pcr:	bitmask with the algorithms supported
 * @active_pcr:		bitmask with the active algorithms
 * @pcr_banks:		number of PCR banks
 *
 * Return: 0 on success, -1 on error
 */
static int tpm2_get_pcr_info(struct udevice *dev, u32 *supported_pcr,
			     u32 *active_pcr, u32 *pcr_banks)
{
	u8 response[TPM2_RESPONSE_BUFFER_SIZE];
	struct tpml_pcr_selection pcrs;
	u32 ret, num_pcr;
	int i, tpm_ret;

	memset(response, 0, sizeof(response));
	ret = tpm2_get_capability(dev, TPM2_CAP_PCRS, 0, response, 1);
	if (ret)
		goto out;

	pcrs.count = get_unaligned_be32(response);
	/*
	 * We only support 5 algorithms for now so check against that
	 * instead of TPM2_NUM_PCR_BANKS
	 */
	if (pcrs.count > MAX_HASH_COUNT || pcrs.count < 1)
		goto out;

	tpm_ret = tpm2_get_num_pcr(dev, &num_pcr);
	if (tpm_ret)
		goto out;

	for (i = 0; i < pcrs.count; i++) {
		/*
		 * Definition of TPMS_PCR_SELECTION Structure
		 * hash: u16
		 * size_of_select: u8
		 * pcr_select: u8 array
		 *
		 * The offsets depend on the number of the device PCRs
		 * so we have to calculate them based on that
		 */
		u32 hash_offset = offsetof(struct tpml_pcr_selection, selection) +
			i * offsetof(struct tpms_pcr_selection, pcr_select) +
			i * ((num_pcr + 7) / 8);
		u32 size_select_offset =
			hash_offset + offsetof(struct tpms_pcr_selection,
					       size_of_select);
		u32 pcr_select_offset =
			hash_offset + offsetof(struct tpms_pcr_selection,
					       pcr_select);

		pcrs.selection[i].hash =
			get_unaligned_be16(response + hash_offset);
		pcrs.selection[i].size_of_select =
			__get_unaligned_be(response + size_select_offset);
		if (pcrs.selection[i].size_of_select > TPM2_PCR_SELECT_MAX)
			goto out;
		/* copy the array of pcr_select */
		memcpy(pcrs.selection[i].pcr_select, response + pcr_select_offset,
		       pcrs.selection[i].size_of_select);
	}

	for (i = 0; i < pcrs.count; i++) {
		u32 hash_mask = alg_to_mask(pcrs.selection[i].hash);

		if (hash_mask) {
			*supported_pcr |= hash_mask;
			if (is_active_pcr(&pcrs.selection[i]))
				*active_pcr |= hash_mask;
		} else {
			EFI_PRINT("Unknown algorithm %x\n", pcrs.selection[i].hash);
		}
	}

	*pcr_banks = pcrs.count;

	return 0;
out:
	return -1;
}

/**
 * efi_tcg2_get_capability() - protocol capability information and state information
 *
 * @this:		TCG2 protocol instance
 * @capability:		caller allocated memory with size field to the size of
 *			the structure allocated

 * Return:	status code
 */
static efi_status_t EFIAPI
efi_tcg2_get_capability(struct efi_tcg2_protocol *this,
			struct efi_tcg2_boot_service_capability *capability)
{
	struct udevice *dev;
	efi_status_t efi_ret;
	int ret;

	EFI_ENTRY("%p, %p", this, capability);

	if (!this || !capability) {
		efi_ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (capability->size < boot_service_capability_min) {
		capability->size = boot_service_capability_min;
		efi_ret = EFI_BUFFER_TOO_SMALL;
		goto out;
	}

	if (capability->size < sizeof(*capability)) {
		capability->size = sizeof(*capability);
		efi_ret = EFI_BUFFER_TOO_SMALL;
		goto out;
	}

	capability->structure_version.major = 1;
	capability->structure_version.minor = 1;
	capability->protocol_version.major = 1;
	capability->protocol_version.minor = 1;

	efi_ret = platform_get_tpm2_device(&dev);
	if (efi_ret != EFI_SUCCESS) {
		capability->supported_event_logs = 0;
		capability->hash_algorithm_bitmap = 0;
		capability->tpm_present_flag = false;
		capability->max_command_size = 0;
		capability->max_response_size = 0;
		capability->manufacturer_id = 0;
		capability->number_of_pcr_banks = 0;
		capability->active_pcr_banks = 0;

		efi_ret = EFI_SUCCESS;
		goto out;
	}

	/* We only allow a TPMv2 device to register the EFI protocol */
	capability->supported_event_logs = TCG2_EVENT_LOG_FORMAT_TCG_2;

	capability->tpm_present_flag = true;

	/* Supported and active PCRs */
	capability->hash_algorithm_bitmap = 0;
	capability->active_pcr_banks = 0;
	ret = tpm2_get_pcr_info(dev, &capability->hash_algorithm_bitmap,
				&capability->active_pcr_banks,
				&capability->number_of_pcr_banks);
	if (ret) {
		efi_ret = EFI_DEVICE_ERROR;
		goto out;
	}

	/* Max command size */
	ret = tpm2_get_max_command_size(dev, &capability->max_command_size);
	if (ret) {
		efi_ret = EFI_DEVICE_ERROR;
		goto out;
	}

	/* Max response size */
	ret = tpm2_get_max_response_size(dev, &capability->max_response_size);
	if (ret) {
		efi_ret = EFI_DEVICE_ERROR;
		goto out;
	}

	/* Manufacturer ID */
	ret = tpm2_get_manufacturer_id(dev, &capability->manufacturer_id);
	if (ret) {
		efi_ret = EFI_DEVICE_ERROR;
		goto out;
	}

	return EFI_EXIT(EFI_SUCCESS);
out:
	return EFI_EXIT(efi_ret);
}

/**
 * efi_tcg2_get_eventlog() -	retrieve the the address of an event log and its
 *				last entry
 *
 * @this:			TCG2 protocol instance
 * @log_format:			type of event log format
 * @event_log_location:		pointer to the memory address of the event log
 * @event_log_last_entry:	pointer to the address of the start of the last
 *				entry in the event log in memory, if log contains
 *				more than 1 entry
 * @event_log_truncated:	set to true, if the Event Log is missing at i
 *				least one entry
 *
 * Return:	status code
 */
static efi_status_t EFIAPI
efi_tcg2_get_eventlog(struct efi_tcg2_protocol *this,
		      efi_tcg_event_log_format log_format,
		      u64 *event_log_location, u64 *event_log_last_entry,
		      bool *event_log_truncated)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_tcg2_hash_log_extend_event() - extend and optionally log events
 *
 * @this:			TCG2 protocol instance
 * @flags:			bitmap providing additional information on the
 *				operation
 * @data_to_hash:		physical address of the start of the data buffer
 *				to be hashed
 * @data_to_hash_len:		the length in bytes of the buffer referenced by
 *				data_to_hash
 * @efi_tcg_event:		pointer to data buffer containing information
 *				about the event
 *
 * Return:	status code
 */
static efi_status_t EFIAPI
efi_tcg2_hash_log_extend_event(struct efi_tcg2_protocol *this, u64 flags,
			       u64 data_to_hash, u64 data_to_hash_len,
			       struct efi_tcg2_event *efi_tcg_event)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_tcg2_submit_command() - Send command to the TPM
 *
 * @this:			TCG2 protocol instance
 * @input_param_block_size:	size of the TPM input parameter block
 * @input_param_block:		pointer to the TPM input parameter block
 * @output_param_block_size:	size of the TPM output parameter block
 * @output_param_block:		pointer to the TPM output parameter block
 *
 * Return:	status code
 */
efi_status_t EFIAPI
efi_tcg2_submit_command(struct efi_tcg2_protocol *this,
			u32 input_param_block_size, u8 *input_param_block,
			u32 output_param_block_size, u8 *output_param_block)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_tcg2_get_active_pcr_banks() - returns the currently active PCR banks
 *
 * @this:			TCG2 protocol instance
 * @active_pcr_banks:		pointer for receiving the bitmap of currently
 *				active PCR banks
 *
 * Return:	status code
 */
efi_status_t EFIAPI
efi_tcg2_get_active_pcr_banks(struct efi_tcg2_protocol *this,
			      u32 *active_pcr_banks)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_tcg2_set_active_pcr_banks() - sets the currently active PCR banks
 *
 * @this:			TCG2 protocol instance
 * @active_pcr_banks:		bitmap of the requested active PCR banks
 *
 * Return:	status code
 */
efi_status_t EFIAPI
efi_tcg2_set_active_pcr_banks(struct efi_tcg2_protocol *this,
			      u32 active_pcr_banks)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_tcg2_get_result_of_set_active_pcr_banks() - retrieve result for previous
 *						   set_active_pcr_banks()
 *
 * @this:			TCG2 protocol instance
 * @operation_present:		non-zero value to indicate a
 *				set_active_pcr_banks operation was
 *				invoked during last boot
 * @response:			result value could be returned
 *
 * Return:	status code
 */
efi_status_t EFIAPI
efi_tcg2_get_result_of_set_active_pcr_banks(struct efi_tcg2_protocol *this,
					    u32 *operation_present, u32 *response)
{
	return EFI_UNSUPPORTED;
}

static const struct efi_tcg2_protocol efi_tcg2_protocol = {
	.get_capability = efi_tcg2_get_capability,
	.get_eventlog = efi_tcg2_get_eventlog,
	.hash_log_extend_event = efi_tcg2_hash_log_extend_event,
	.submit_command = efi_tcg2_submit_command,
	.get_active_pcr_banks = efi_tcg2_get_active_pcr_banks,
	.set_active_pcr_banks = efi_tcg2_set_active_pcr_banks,
	.get_result_of_set_active_pcr_banks = efi_tcg2_get_result_of_set_active_pcr_banks,
};

/**
 * efi_tcg2_register() - register EFI_TCG2_PROTOCOL
 *
 * If a TPM2 device is available, the TPM TCG2 Protocol is registered
 *
 * Return:	An error status is only returned if adding the protocol fails.
 */
efi_status_t efi_tcg2_register(void)
{
	efi_status_t ret;
	struct udevice *dev;

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS) {
		log_warning("Unable to find TPMv2 device\n");
		return EFI_SUCCESS;
	}
	ret = efi_add_protocol(efi_root, &efi_guid_tcg2_protocol,
			       (void *)&efi_tcg2_protocol);
	if (ret != EFI_SUCCESS)
		log_err("Cannot install EFI_TCG2_PROTOCOL\n");

	return ret;
}
