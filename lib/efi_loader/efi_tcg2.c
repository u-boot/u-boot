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
#include <efi_variable.h>
#include <efi_tcg2.h>
#include <log.h>
#include <malloc.h>
#include <smbios.h>
#include <version_string.h>
#include <tpm-v2.h>
#include <tpm_api.h>
#include <u-boot/hash-checksum.h>
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>
#include <u-boot/sha512.h>
#include <linux/unaligned/be_byteshift.h>
#include <linux/unaligned/le_byteshift.h>
#include <linux/unaligned/generic.h>
#include <hexdump.h>

/**
 * struct event_log_buffer - internal eventlog management structure
 *
 * @buffer:		eventlog buffer
 * @final_buffer:	finalevent config table buffer
 * @pos:		current position of 'buffer'
 * @final_pos:		current position of 'final_buffer'
 * @get_event_called:	true if GetEventLog has been invoked at least once
 * @ebs_called:		true if ExitBootServices has been invoked
 * @truncated:		true if the 'buffer' is truncated
 */
struct event_log_buffer {
	void *buffer;
	void *final_buffer;
	size_t pos; /* eventlog position */
	size_t final_pos; /* final events config table position */
	size_t last_event_size;
	bool get_event_called;
	bool ebs_called;
	bool truncated;
};

static struct event_log_buffer event_log;
static bool tcg2_efi_app_invoked;
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

static const efi_guid_t efi_guid_tcg2_protocol = EFI_TCG2_PROTOCOL_GUID;
static const efi_guid_t efi_guid_final_events = EFI_TCG2_FINAL_EVENTS_TABLE_GUID;

struct digest_info {
	u16 hash_alg;
	u32 hash_mask;
	u16 hash_len;
};

static const struct digest_info hash_algo_list[] = {
	{
		TPM2_ALG_SHA1,
		EFI_TCG2_BOOT_HASH_ALG_SHA1,
		TPM2_SHA1_DIGEST_SIZE,
	},
	{
		TPM2_ALG_SHA256,
		EFI_TCG2_BOOT_HASH_ALG_SHA256,
		TPM2_SHA256_DIGEST_SIZE,
	},
	{
		TPM2_ALG_SHA384,
		EFI_TCG2_BOOT_HASH_ALG_SHA384,
		TPM2_SHA384_DIGEST_SIZE,
	},
	{
		TPM2_ALG_SHA512,
		EFI_TCG2_BOOT_HASH_ALG_SHA512,
		TPM2_SHA512_DIGEST_SIZE,
	},
};

struct variable_info {
	const u16	*name;
	bool		accept_empty;
	u32		pcr_index;
};

static struct variable_info secure_variables[] = {
	{u"SecureBoot",		true,	7},
	{u"PK",			true,	7},
	{u"KEK",		true,	7},
	{u"db",			true,	7},
	{u"dbx",		true,	7},
	{u"dbt",		false,	7},
	{u"dbr",		false,	7},
	{u"DeployedMode",	false,	1},
	{u"AuditMode",		false,	1},
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
	size_t i;

	for (i = 0; i < MAX_HASH_COUNT; i++) {
		if (hash_algo_list[i].hash_alg == hash_alg)
			return hash_algo_list[i].hash_mask;
	}

	return 0;
}

/**
 * alg_to_len - Get a TCG hash len for algorithms
 *
 * @hash_alg: TCG defined algorithm
 *
 * @Return: len of chosen algorithm, 0 if the algorithm is not supported
 */
static u16 alg_to_len(u16 hash_alg)
{
	size_t i;

	for (i = 0; i < MAX_HASH_COUNT; i++) {
		if (hash_algo_list[i].hash_alg == hash_alg)
			return hash_algo_list[i].hash_len;
	}

	return 0;
}

static bool is_tcg2_protocol_installed(void)
{
	struct efi_handler *handler;
	efi_status_t ret;

	ret = efi_search_protocol(efi_root, &efi_guid_tcg2_protocol, &handler);
	return ret == EFI_SUCCESS;
}

static u32 tcg_event_final_size(struct tpml_digest_values *digest_list)
{
	u32 len;
	size_t i;

	len = offsetof(struct tcg_pcr_event2, digests);
	len += offsetof(struct tpml_digest_values, digests);
	for (i = 0; i < digest_list->count; i++) {
		u16 hash_alg = digest_list->digests[i].hash_alg;

		len += offsetof(struct tpmt_ha, digest);
		len += alg_to_len(hash_alg);
	}
	len += sizeof(u32); /* tcg_pcr_event2 event_size*/

	return len;
}

/* tcg2_pcr_extend - Extend PCRs for a TPM2 device for a given tpml_digest_values
 *
 * @dev:		device
 * @digest_list:	list of digest algorithms to extend
 *
 * @Return: status code
 */
static efi_status_t tcg2_pcr_extend(struct udevice *dev, u32 pcr_index,
				    struct tpml_digest_values *digest_list)
{
	u32 rc;
	size_t i;

	for (i = 0; i < digest_list->count; i++) {
		u32 alg = digest_list->digests[i].hash_alg;

		rc = tpm2_pcr_extend(dev, pcr_index, alg,
				     (u8 *)&digest_list->digests[i].digest,
				     alg_to_len(alg));
		if (rc) {
			EFI_PRINT("Failed to extend PCR\n");
			return EFI_DEVICE_ERROR;
		}
	}

	return EFI_SUCCESS;
}

/* tcg2_pcr_read - Read PCRs for a TPM2 device for a given tpml_digest_values
 *
 * @dev:		device
 * @pcr_index:		PCR index
 * @digest_list:	list of digest algorithms to extend
 *
 * @Return: status code
 */
static efi_status_t tcg2_pcr_read(struct udevice *dev, u32 pcr_index,
				  struct tpml_digest_values *digest_list)
{
	struct tpm_chip_priv *priv;
	unsigned int updates, pcr_select_min;
	u32 rc;
	size_t i;

	priv = dev_get_uclass_priv(dev);
	if (!priv)
		return EFI_DEVICE_ERROR;

	pcr_select_min = priv->pcr_select_min;

	for (i = 0; i < digest_list->count; i++) {
		u16 hash_alg = digest_list->digests[i].hash_alg;
		u8 *digest = (u8 *)&digest_list->digests[i].digest;

		rc = tpm2_pcr_read(dev, pcr_index, pcr_select_min,
				   hash_alg, digest, alg_to_len(hash_alg),
				   &updates);
		if (rc) {
			EFI_PRINT("Failed to read PCR\n");
			return EFI_DEVICE_ERROR;
		}
	}

	return EFI_SUCCESS;
}

/* put_event - Append an agile event to an eventlog
 *
 * @pcr_index:		PCR index
 * @event_type:		type of event added
 * @digest_list:	list of digest algorithms to add
 * @size:		size of event
 * @event:		event to add
 * @log:		log buffer to append the event
 *
 */
static void put_event(u32 pcr_index, u32 event_type,
		      struct tpml_digest_values *digest_list, u32 size,
		      u8 event[], void *log)
{
	size_t pos;
	size_t i;
	u32 event_size;

	/*
	 * size refers to the length of event[] only, we need to check against
	 * the final tcg_pcr_event2 size
	 */
	event_size = size + tcg_event_final_size(digest_list);

	put_unaligned_le32(pcr_index, log);
	pos = offsetof(struct tcg_pcr_event2, event_type);
	put_unaligned_le32(event_type, (void *)((uintptr_t)log + pos));
	pos = offsetof(struct tcg_pcr_event2, digests); /* count */
	put_unaligned_le32(digest_list->count, (void *)((uintptr_t)log + pos));

	pos += offsetof(struct tpml_digest_values, digests);
	for (i = 0; i < digest_list->count; i++) {
		u16 hash_alg = digest_list->digests[i].hash_alg;
		u8 *digest = (u8 *)&digest_list->digests[i].digest;

		put_unaligned_le16(hash_alg, (void *)((uintptr_t)log + pos));
		pos += offsetof(struct tpmt_ha, digest);
		memcpy((void *)((uintptr_t)log + pos), digest, alg_to_len(hash_alg));
		pos += alg_to_len(hash_alg);
	}

	put_unaligned_le32(size, (void *)((uintptr_t)log + pos));
	pos += sizeof(u32); /* tcg_pcr_event2 event_size*/
	memcpy((void *)((uintptr_t)log + pos), event, size);
	pos += size;

	/*
	 * make sure the calculated buffer is what we checked against
	 * This check should never fail.  It checks the code above is
	 * calculating the right length for the event we are adding
	 */
	if (pos != event_size)
		log_err("Appending to the EventLog failed\n");
}

/* tcg2_agile_log_append - Append an agile event to an eventlog
 *
 * @pcr_index:		PCR index
 * @event_type:		type of event added
 * @digest_list:	list of digest algorithms to add
 * @size:		size of event
 * @event:		event to add
 * @log:		log buffer to append the event
 *
 * @Return: status code
 */
static efi_status_t tcg2_agile_log_append(u32 pcr_index, u32 event_type,
					  struct tpml_digest_values *digest_list,
					  u32 size, u8 event[])
{
	void *log = (void *)((uintptr_t)event_log.buffer + event_log.pos);
	u32 event_size = size + tcg_event_final_size(digest_list);
	struct efi_tcg2_final_events_table *final_event;
	efi_status_t ret = EFI_SUCCESS;

	/* if ExitBootServices hasn't been called update the normal log */
	if (!event_log.ebs_called) {
		if (event_log.truncated ||
		    event_log.pos + event_size > TPM2_EVENT_LOG_SIZE) {
			event_log.truncated = true;
			return EFI_VOLUME_FULL;
		}
		put_event(pcr_index, event_type, digest_list, size, event, log);
		event_log.pos += event_size;
		event_log.last_event_size = event_size;
	}

	if (!event_log.get_event_called)
		return ret;

	/* if GetEventLog has been called update FinalEventLog as well */
	if (event_log.final_pos + event_size > TPM2_EVENT_LOG_SIZE)
		return EFI_VOLUME_FULL;

	log = (void *)((uintptr_t)event_log.final_buffer + event_log.final_pos);
	put_event(pcr_index, event_type, digest_list, size, event, log);

	final_event = event_log.final_buffer;
	final_event->number_of_events++;
	event_log.final_pos += event_size;

	return ret;
}

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
 * platform_get_eventlog() - retrieve the eventlog address and size
 *
 * This function retrieves the eventlog address and size if the underlying
 * firmware has done some measurements and passed them.
 *
 * This function may be overridden based on platform specific method of
 * passing the eventlog address and size.
 *
 * @dev:	udevice
 * @addr:	eventlog address
 * @sz:		eventlog size
 * Return:	status code
 */
__weak efi_status_t platform_get_eventlog(struct udevice *dev, u64 *addr,
					  u32 *sz)
{
	const u64 *basep;
	const u32 *sizep;

	basep = dev_read_prop(dev, "tpm_event_log_addr", NULL);
	if (!basep)
		return EFI_NOT_FOUND;

	*addr = be64_to_cpup((__force __be64 *)basep);

	sizep = dev_read_prop(dev, "tpm_event_log_size", NULL);
	if (!sizep)
		return EFI_NOT_FOUND;

	*sz = be32_to_cpup((__force __be32 *)sizep);
	if (*sz == 0) {
		log_debug("event log empty\n");
		return EFI_NOT_FOUND;
	}

	return EFI_SUCCESS;
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
static bool is_active_pcr(struct tpms_pcr_selection *selection)
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
	size_t i;
	int tpm_ret;

	*supported_pcr = 0;
	*active_pcr = 0;
	*pcr_banks = 0;
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
 * __get_active_pcr_banks() - returns the currently active PCR banks
 *
 * @active_pcr_banks:		pointer for receiving the bitmap of currently
 *				active PCR banks
 *
 * Return:	status code
 */
static efi_status_t __get_active_pcr_banks(u32 *active_pcr_banks)
{
	struct udevice *dev;
	u32 active = 0, supported = 0, pcr_banks = 0;
	efi_status_t ret;
	int err;

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS)
		goto out;

	err = tpm2_get_pcr_info(dev, &supported, &active, &pcr_banks);
	if (err) {
		ret = EFI_DEVICE_ERROR;
		goto out;
	}

	*active_pcr_banks = active;

out:
	return ret;
}

/* tcg2_create_digest - create a list of digests of the supported PCR banks
 *			for a given memory range
 *
 * @input:		input memory
 * @length:		length of buffer to calculate the digest
 * @digest_list:	list of digests to fill in
 *
 * Return:		status code
 */
static efi_status_t tcg2_create_digest(const u8 *input, u32 length,
				       struct tpml_digest_values *digest_list)
{
	sha1_context ctx;
	sha256_context ctx_256;
	sha512_context ctx_512;
	u8 final[TPM2_SHA512_DIGEST_SIZE];
	efi_status_t ret;
	u32 active;
	size_t i;

	ret = __get_active_pcr_banks(&active);
	if (ret != EFI_SUCCESS)
		return ret;

	digest_list->count = 0;
	for (i = 0; i < MAX_HASH_COUNT; i++) {
		u16 hash_alg = hash_algo_list[i].hash_alg;

		if (!(active & alg_to_mask(hash_alg)))
			continue;
		switch (hash_alg) {
		case TPM2_ALG_SHA1:
			sha1_starts(&ctx);
			sha1_update(&ctx, input, length);
			sha1_finish(&ctx, final);
			break;
		case TPM2_ALG_SHA256:
			sha256_starts(&ctx_256);
			sha256_update(&ctx_256, input, length);
			sha256_finish(&ctx_256, final);
			break;
		case TPM2_ALG_SHA384:
			sha384_starts(&ctx_512);
			sha384_update(&ctx_512, input, length);
			sha384_finish(&ctx_512, final);
			break;
		case TPM2_ALG_SHA512:
			sha512_starts(&ctx_512);
			sha512_update(&ctx_512, input, length);
			sha512_finish(&ctx_512, final);
			break;
		default:
			EFI_PRINT("Unsupported algorithm %x\n", hash_alg);
			return EFI_INVALID_PARAMETER;
		}
		digest_list->digests[digest_list->count].hash_alg = hash_alg;
		memcpy(&digest_list->digests[digest_list->count].digest, final,
		       (u32)alg_to_len(hash_alg));
		digest_list->count++;
	}

	return EFI_SUCCESS;
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

	if (capability->size < BOOT_SERVICE_CAPABILITY_MIN) {
		capability->size = BOOT_SERVICE_CAPABILITY_MIN;
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
	efi_status_t ret = EFI_SUCCESS;
	struct udevice *dev;

	EFI_ENTRY("%p, %u, %p, %p,  %p", this, log_format, event_log_location,
		  event_log_last_entry, event_log_truncated);

	if (!this || !event_log_location || !event_log_last_entry ||
	    !event_log_truncated) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Only support TPMV2 */
	if (log_format != TCG2_EVENT_LOG_FORMAT_TCG_2) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS) {
		event_log_location = NULL;
		event_log_last_entry = NULL;
		*event_log_truncated = false;
		ret = EFI_SUCCESS;
		goto out;
	}
	*event_log_location = (uintptr_t)event_log.buffer;
	*event_log_last_entry = (uintptr_t)(event_log.buffer + event_log.pos -
					    event_log.last_event_size);
	*event_log_truncated = event_log.truncated;
	event_log.get_event_called = true;

out:
	return EFI_EXIT(ret);
}

/**
 * tcg2_hash_pe_image() - calculate PE/COFF image hash
 *
 * @efi:		pointer to the EFI binary
 * @efi_size:		size of @efi binary
 * @digest_list:	list of digest algorithms to extend
 *
 * Return:	status code
 */
static efi_status_t tcg2_hash_pe_image(void *efi, u64 efi_size,
				       struct tpml_digest_values *digest_list)
{
	WIN_CERTIFICATE *wincerts = NULL;
	size_t wincerts_len;
	struct efi_image_regions *regs = NULL;
	void *new_efi = NULL;
	u8 hash[TPM2_SHA512_DIGEST_SIZE];
	efi_status_t ret;
	u32 active;
	int i;

	new_efi = efi_prepare_aligned_image(efi, &efi_size);
	if (!new_efi)
		return EFI_OUT_OF_RESOURCES;

	if (!efi_image_parse(new_efi, efi_size, &regs, &wincerts,
			     &wincerts_len)) {
		log_err("Parsing PE executable image failed\n");
		ret = EFI_UNSUPPORTED;
		goto out;
	}

	ret = __get_active_pcr_banks(&active);
	if (ret != EFI_SUCCESS) {
		goto out;
	}

	digest_list->count = 0;
	for (i = 0; i < MAX_HASH_COUNT; i++) {
		u16 hash_alg = hash_algo_list[i].hash_alg;

		if (!(active & alg_to_mask(hash_alg)))
			continue;
		switch (hash_alg) {
		case TPM2_ALG_SHA1:
			hash_calculate("sha1", regs->reg, regs->num, hash);
			break;
		case TPM2_ALG_SHA256:
			hash_calculate("sha256", regs->reg, regs->num, hash);
			break;
		case TPM2_ALG_SHA384:
			hash_calculate("sha384", regs->reg, regs->num, hash);
			break;
		case TPM2_ALG_SHA512:
			hash_calculate("sha512", regs->reg, regs->num, hash);
			break;
		default:
			EFI_PRINT("Unsupported algorithm %x\n", hash_alg);
			return EFI_INVALID_PARAMETER;
		}
		digest_list->digests[digest_list->count].hash_alg = hash_alg;
		memcpy(&digest_list->digests[digest_list->count].digest, hash,
		       (u32)alg_to_len(hash_alg));
		digest_list->count++;
	}

out:
	if (new_efi != efi)
		free(new_efi);
	free(regs);

	return ret;
}

/**
 * tcg2_measure_pe_image() - measure PE/COFF image
 *
 * @efi:		pointer to the EFI binary
 * @efi_size:		size of @efi binary
 * @handle:		loaded image handle
 * @loaded_image:	loaded image protocol
 *
 * Return:	status code
 */
efi_status_t tcg2_measure_pe_image(void *efi, u64 efi_size,
				   struct efi_loaded_image_obj *handle,
				   struct efi_loaded_image *loaded_image)
{
	struct tpml_digest_values digest_list;
	efi_status_t ret;
	struct udevice *dev;
	u32 pcr_index, event_type, event_size;
	struct uefi_image_load_event *image_load_event;
	struct efi_device_path *device_path;
	u32 device_path_length;
	IMAGE_DOS_HEADER *dos;
	IMAGE_NT_HEADERS32 *nt;
	struct efi_handler *handler;

	if (!is_tcg2_protocol_installed())
		return EFI_SUCCESS;

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS)
		return EFI_SECURITY_VIOLATION;

	switch (handle->image_type) {
	case IMAGE_SUBSYSTEM_EFI_APPLICATION:
		pcr_index = 4;
		event_type = EV_EFI_BOOT_SERVICES_APPLICATION;
		break;
	case IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
		pcr_index = 2;
		event_type = EV_EFI_BOOT_SERVICES_DRIVER;
		break;
	case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
		pcr_index = 2;
		event_type = EV_EFI_RUNTIME_SERVICES_DRIVER;
		break;
	default:
		return EFI_UNSUPPORTED;
	}

	ret = tcg2_hash_pe_image(efi, efi_size, &digest_list);
	if (ret != EFI_SUCCESS)
		return ret;

	ret = tcg2_pcr_extend(dev, pcr_index, &digest_list);
	if (ret != EFI_SUCCESS)
		return ret;

	ret = efi_search_protocol(&handle->header,
				  &efi_guid_loaded_image_device_path, &handler);
	if (ret != EFI_SUCCESS)
		return ret;

	device_path = handler->protocol_interface;
	device_path_length = efi_dp_size(device_path);
	if (device_path_length > 0) {
		/* add end node size */
		device_path_length += sizeof(struct efi_device_path);
	}
	event_size = sizeof(struct uefi_image_load_event) + device_path_length;
	image_load_event = calloc(1, event_size);
	if (!image_load_event)
		return EFI_OUT_OF_RESOURCES;

	image_load_event->image_location_in_memory = (uintptr_t)efi;
	image_load_event->image_length_in_memory = efi_size;
	image_load_event->length_of_device_path = device_path_length;

	dos = (IMAGE_DOS_HEADER *)efi;
	nt = (IMAGE_NT_HEADERS32 *)(efi + dos->e_lfanew);
	if (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
		IMAGE_NT_HEADERS64 *nt64 = (IMAGE_NT_HEADERS64 *)nt;

		image_load_event->image_link_time_address =
				nt64->OptionalHeader.ImageBase;
	} else if (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
		image_load_event->image_link_time_address =
				nt->OptionalHeader.ImageBase;
	} else {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* device_path_length might be zero */
	memcpy(image_load_event->device_path, device_path, device_path_length);

	ret = tcg2_agile_log_append(pcr_index, event_type, &digest_list,
				    event_size, (u8 *)image_load_event);

out:
	free(image_load_event);

	return ret;
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
	struct udevice *dev;
	efi_status_t ret;
	u32 event_type, pcr_index, event_size;
	struct tpml_digest_values digest_list;

	EFI_ENTRY("%p, %llu, %llu, %llu, %p", this, flags, data_to_hash,
		  data_to_hash_len, efi_tcg_event);

	if (!this || !data_to_hash || !efi_tcg_event) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS)
		goto out;

	if (efi_tcg_event->size < efi_tcg_event->header.header_size +
	    sizeof(u32)) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (efi_tcg_event->header.pcr_index > EFI_TCG2_MAX_PCR_INDEX) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/*
	 * if PE_COFF_IMAGE is set we need to make sure the image is not
	 * corrupted, verify it and hash the PE/COFF image in accordance with
	 * the procedure specified in "Calculating the PE Image Hash"
	 * section of the "Windows Authenticode Portable Executable Signature
	 * Format"
	 */
	if (flags & PE_COFF_IMAGE) {
		IMAGE_NT_HEADERS32 *nt;

		ret = efi_check_pe((void *)(uintptr_t)data_to_hash,
				   data_to_hash_len, (void **)&nt);
		if (ret != EFI_SUCCESS) {
			log_err("Not a valid PE-COFF file\n");
			ret = EFI_UNSUPPORTED;
			goto out;
		}
		ret = tcg2_hash_pe_image((void *)(uintptr_t)data_to_hash,
					 data_to_hash_len, &digest_list);
	} else {
		ret = tcg2_create_digest((u8 *)(uintptr_t)data_to_hash,
					 data_to_hash_len, &digest_list);
	}

	if (ret != EFI_SUCCESS)
		goto out;

	pcr_index = efi_tcg_event->header.pcr_index;
	event_type = efi_tcg_event->header.event_type;

	ret = tcg2_pcr_extend(dev, pcr_index, &digest_list);
	if (ret != EFI_SUCCESS)
		goto out;

	if (flags & EFI_TCG2_EXTEND_ONLY) {
		if (event_log.truncated)
			ret = EFI_VOLUME_FULL;
		goto out;
	}

	/*
	 * The efi_tcg_event size includes the size component and the
	 * headersize
	 */
	event_size = efi_tcg_event->size - sizeof(efi_tcg_event->size) -
		efi_tcg_event->header.header_size;
	ret = tcg2_agile_log_append(pcr_index, event_type, &digest_list,
				    event_size, efi_tcg_event->event);
out:
	return EFI_EXIT(ret);
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
static efi_status_t EFIAPI
efi_tcg2_submit_command(struct efi_tcg2_protocol *this,
			u32 input_param_block_size,
			u8 *input_param_block,
			u32 output_param_block_size,
			u8 *output_param_block)
{
	struct udevice *dev;
	efi_status_t ret;
	u32 rc;
	size_t resp_buf_size = output_param_block_size;

	EFI_ENTRY("%p, %u, %p, %u, %p", this, input_param_block_size,
		  input_param_block, output_param_block_size, output_param_block);

	if (!this || !input_param_block || !input_param_block_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS)
		goto out;

	rc = tpm2_submit_command(dev, input_param_block,
				 output_param_block, &resp_buf_size);
	if (rc) {
		ret = (rc == -ENOSPC) ? EFI_OUT_OF_RESOURCES : EFI_DEVICE_ERROR;

		goto out;
	}

out:
	return EFI_EXIT(ret);
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
static efi_status_t EFIAPI
efi_tcg2_get_active_pcr_banks(struct efi_tcg2_protocol *this,
			      u32 *active_pcr_banks)
{
	efi_status_t ret;

	if (!this || !active_pcr_banks) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	EFI_ENTRY("%p, %p", this, active_pcr_banks);
	ret = __get_active_pcr_banks(active_pcr_banks);

out:
	return EFI_EXIT(ret);
}

/**
 * efi_tcg2_set_active_pcr_banks() - sets the currently active PCR banks
 *
 * @this:			TCG2 protocol instance
 * @active_pcr_banks:		bitmap of the requested active PCR banks
 *
 * Return:	status code
 */
static efi_status_t EFIAPI
efi_tcg2_set_active_pcr_banks(__maybe_unused struct efi_tcg2_protocol *this,
			      u32 __maybe_unused active_pcr_banks)
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
static efi_status_t EFIAPI
efi_tcg2_get_result_of_set_active_pcr_banks(__maybe_unused struct efi_tcg2_protocol *this,
					    u32 __maybe_unused *operation_present,
					    u32 __maybe_unused *response)
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
 * parse_event_log_header() -  Parse and verify the event log header fields
 *
 * @buffer:			Pointer to the start of the eventlog
 * @size:			Size of the eventlog
 * @pos:			Return offset of the next event in buffer right
 *				after the event header i.e specID
 *
 * Return:	status code
 */
static efi_status_t parse_event_log_header(void *buffer, u32 size, u32 *pos)
{
	struct tcg_pcr_event *event_header = (struct tcg_pcr_event *)buffer;
	int i = 0;

	if (size < sizeof(*event_header))
		return EFI_COMPROMISED_DATA;

	if (get_unaligned_le32(&event_header->pcr_index) != 0 ||
	    get_unaligned_le32(&event_header->event_type) != EV_NO_ACTION)
		return EFI_COMPROMISED_DATA;

	for (i = 0; i < sizeof(event_header->digest); i++) {
		if (event_header->digest[i])
			return EFI_COMPROMISED_DATA;
	}

	*pos += sizeof(*event_header);

	return EFI_SUCCESS;
}

/**
 * parse_specid_event() -  Parse and verify the specID Event in the eventlog
 *
 * @dev:		udevice
 * @buffer:		Pointer to the start of the eventlog
 * @log_size:		Size of the eventlog
 * @pos:		[in] Offset of specID event in the eventlog buffer
 *			[out] Return offset of the next event in the buffer
 *			after the specID
 * @digest_list:	list of digests in the event
 *
 * Return:		status code
 * @pos			Offset in the eventlog where the specID event ends
 * @digest_list:	list of digests in the event
 */
static efi_status_t parse_specid_event(struct udevice *dev, void *buffer,
				       u32 log_size, u32 *pos,
				       struct tpml_digest_values *digest_list)
{
	struct tcg_efi_spec_id_event *spec_event;
	struct tcg_pcr_event *event_header = (struct tcg_pcr_event *)buffer;
	size_t spec_event_size;
	u32 active = 0, supported = 0, pcr_count = 0, alg_count = 0;
	u32 spec_active = 0;
	u16 hash_alg;
	u8 vendor_sz;
	int err, i;

	if (*pos >= log_size || (*pos + sizeof(*spec_event)) > log_size)
		return EFI_COMPROMISED_DATA;

	/* Check specID event data */
	spec_event = (struct tcg_efi_spec_id_event *)((uintptr_t)buffer + *pos);
	/* Check for signature */
	if (memcmp(spec_event->signature, TCG_EFI_SPEC_ID_EVENT_SIGNATURE_03,
		   sizeof(TCG_EFI_SPEC_ID_EVENT_SIGNATURE_03))) {
		log_err("specID Event: Signature mismatch\n");
		return EFI_COMPROMISED_DATA;
	}

	if (spec_event->spec_version_minor !=
			TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MINOR_TPM2 ||
	    spec_event->spec_version_major !=
			TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MAJOR_TPM2)
		return EFI_COMPROMISED_DATA;

	if (spec_event->number_of_algorithms > MAX_HASH_COUNT ||
	    spec_event->number_of_algorithms < 1) {
		log_err("specID Event: Number of algorithms incorrect\n");
		return EFI_COMPROMISED_DATA;
	}

	alg_count = spec_event->number_of_algorithms;

	err = tpm2_get_pcr_info(dev, &supported, &active, &pcr_count);
	if (err)
		return EFI_DEVICE_ERROR;

	digest_list->count = 0;
	/*
	 * We have to take care that the sequence of algorithms that we record
	 * in digest_list matches the sequence in eventlog.
	 */
	for (i = 0; i < alg_count; i++) {
		hash_alg =
		  get_unaligned_le16(&spec_event->digest_sizes[i].algorithm_id);

		if (!(supported & alg_to_mask(hash_alg))) {
			log_err("specID Event: Unsupported algorithm\n");
			return EFI_COMPROMISED_DATA;
		}
		digest_list->digests[digest_list->count++].hash_alg = hash_alg;

		spec_active |= alg_to_mask(hash_alg);
	}

	/*
	 * TCG specification expects the event log to have hashes for all
	 * active PCR's
	 */
	if (spec_active != active) {
		/*
		 * Previous stage bootloader should know all the active PCR's
		 * and use them in the Eventlog.
		 */
		log_err("specID Event: All active hash alg not present\n");
		return EFI_COMPROMISED_DATA;
	}

	/*
	 * the size of the spec event and placement of vendor_info_size
	 * depends on supported algoriths
	 */
	spec_event_size =
		offsetof(struct tcg_efi_spec_id_event, digest_sizes) +
		alg_count * sizeof(spec_event->digest_sizes[0]);

	if (*pos + spec_event_size >= log_size)
		return EFI_COMPROMISED_DATA;

	vendor_sz = *(uint8_t *)((uintptr_t)buffer + *pos + spec_event_size);

	spec_event_size += sizeof(vendor_sz) + vendor_sz;
	*pos += spec_event_size;

	if (get_unaligned_le32(&event_header->event_size) != spec_event_size) {
		log_err("specID event: header event size mismatch\n");
		/* Right way to handle this can be to call SetActive PCR's */
		return EFI_COMPROMISED_DATA;
	}

	return EFI_SUCCESS;
}

/**
 * tcg2_parse_event() -  Parse the event in the eventlog
 *
 * @dev:		udevice
 * @buffer:		Pointer to the start of the eventlog
 * @log_size:		Size of the eventlog
 * @offset:		[in] Offset of the event in the eventlog buffer
 *			[out] Return offset of the next event in the buffer
 * @digest_list:	list of digests in the event
 * @pcr			Index of the PCR in the event
 *
 * Return:		status code
 */
static efi_status_t tcg2_parse_event(struct udevice *dev, void *buffer,
				     u32 log_size, u32 *offset,
				     struct tpml_digest_values *digest_list,
				     u32 *pcr)
{
	struct tcg_pcr_event2 *event = NULL;
	u32 count, size, event_size;
	size_t pos;

	event_size = tcg_event_final_size(digest_list);
	if (*offset >= log_size || *offset + event_size > log_size) {
		log_err("Event exceeds log size\n");
		return EFI_COMPROMISED_DATA;
	}

	event = (struct tcg_pcr_event2 *)((uintptr_t)buffer + *offset);
	*pcr = get_unaligned_le32(&event->pcr_index);

	/* get the count */
	count = get_unaligned_le32(&event->digests.count);
	if (count != digest_list->count)
		return EFI_COMPROMISED_DATA;

	pos = offsetof(struct tcg_pcr_event2, digests);
	pos += offsetof(struct tpml_digest_values, digests);

	for (int i = 0; i < digest_list->count; i++) {
		u16 alg;
		u16 hash_alg = digest_list->digests[i].hash_alg;
		u8 *digest = (u8 *)&digest_list->digests[i].digest;

		alg = get_unaligned_le16((void *)((uintptr_t)event + pos));

		if (alg != hash_alg)
			return EFI_COMPROMISED_DATA;

		pos += offsetof(struct tpmt_ha, digest);
		memcpy(digest, (void *)((uintptr_t)event + pos), alg_to_len(hash_alg));
		pos += alg_to_len(hash_alg);
	}

	size = get_unaligned_le32((void *)((uintptr_t)event + pos));
	event_size += size;
	pos += sizeof(u32); /* tcg_pcr_event2 event_size*/
	pos += size;

	/* make sure the calculated buffer is what we checked against */
	if (pos != event_size)
		return EFI_COMPROMISED_DATA;

	if (pos > log_size)
		return EFI_COMPROMISED_DATA;

	*offset += pos;

	return EFI_SUCCESS;
}

/**
 * tcg2_get_fw_eventlog() -  Get the eventlog address and size
 *
 * If the previous firmware has passed some eventlog, this function get it's
 * location and check for it's validity.
 *
 * @dev:		udevice
 * @log_buffer:		eventlog address
 * @log_sz:		eventlog size
 *
 * Return:	status code
 */
static efi_status_t tcg2_get_fw_eventlog(struct udevice *dev, void *log_buffer,
					 size_t *log_sz)
{
	struct tpml_digest_values digest_list;
	void *buffer;
	efi_status_t ret;
	u32 pcr, pos;
	u64 base;
	u32 sz;
	bool extend_pcr = false;
	int i;

	ret = platform_get_eventlog(dev, &base, &sz);
	if (ret != EFI_SUCCESS)
		return ret;

	if (sz > TPM2_EVENT_LOG_SIZE)
		return EFI_VOLUME_FULL;

	buffer = (void *)(uintptr_t)base;
	pos = 0;
	/* Parse the eventlog to check for its validity */
	ret = parse_event_log_header(buffer, sz, &pos);
	if (ret)
		return ret;

	ret = parse_specid_event(dev, buffer, sz, &pos, &digest_list);
	if (ret) {
		log_err("Error parsing SPEC ID Event\n");
		return ret;
	}

	ret = tcg2_pcr_read(dev, 0, &digest_list);
	if (ret) {
		log_err("Error reading PCR 0\n");
		return ret;
	}

	/*
	 * If PCR0 is 0, previous firmware didn't have the capability
	 * to extend the PCR. In this scenario, extend the PCR as
	 * the eventlog is parsed.
	 */
	for (i = 0; i < digest_list.count; i++) {
		u8 hash_buf[TPM2_SHA512_DIGEST_SIZE] = { 0 };
		u16 hash_alg = digest_list.digests[i].hash_alg;

		if (!memcmp((u8 *)&digest_list.digests[i].digest, hash_buf,
			    alg_to_len(hash_alg)))
			extend_pcr = true;
	}

	while (pos < sz) {
		ret = tcg2_parse_event(dev, buffer, sz, &pos, &digest_list,
				       &pcr);
		if (ret) {
			log_err("Error parsing event\n");
			return ret;
		}
		if (extend_pcr) {
			ret = tcg2_pcr_extend(dev, pcr, &digest_list);
			if (ret != EFI_SUCCESS) {
				log_err("Error in extending PCR\n");
				return ret;
			}

			/* Clear the digest for next event */
			for (i = 0; i < digest_list.count; i++) {
				u16 hash_alg = digest_list.digests[i].hash_alg;
				u8 *digest =
				   (u8 *)&digest_list.digests[i].digest;

				memset(digest, 0, alg_to_len(hash_alg));
			}
		}
	}

	memcpy(log_buffer, buffer, sz);
	*log_sz = sz;

	return ret;
}

/**
 * create_specid_event() - Create the first event in the eventlog
 *
 * @dev:			tpm device
 * @event_header:		Pointer to the final event header
 * @event_size:			final spec event size
 *
 * Return:	status code
 */
static efi_status_t create_specid_event(struct udevice *dev, void *buffer,
					size_t *event_size)
{
	struct tcg_efi_spec_id_event *spec_event;
	size_t spec_event_size;
	efi_status_t ret = EFI_DEVICE_ERROR;
	u32 active = 0, supported = 0, pcr_count = 0, alg_count = 0;
	int err;
	size_t i;

	/*
	 * Create Spec event. This needs to be the first event in the log
	 * according to the TCG EFI protocol spec
	 */

	/* Setup specID event data */
	spec_event = (struct tcg_efi_spec_id_event *)buffer;
	memcpy(spec_event->signature, TCG_EFI_SPEC_ID_EVENT_SIGNATURE_03,
	       sizeof(spec_event->signature));
	put_unaligned_le32(0, &spec_event->platform_class); /* type client */
	spec_event->spec_version_minor =
		TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MINOR_TPM2;
	spec_event->spec_version_major =
		TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MAJOR_TPM2;
	spec_event->spec_errata =
		TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_ERRATA_TPM2;
	spec_event->uintn_size = sizeof(efi_uintn_t) / sizeof(u32);

	err = tpm2_get_pcr_info(dev, &supported, &active, &pcr_count);

	if (err)
		goto out;

	for (i = 0; i < pcr_count; i++) {
		u16 hash_alg = hash_algo_list[i].hash_alg;
		u16 hash_len = hash_algo_list[i].hash_len;

		if (active & alg_to_mask(hash_alg)) {
			put_unaligned_le16(hash_alg,
					   &spec_event->digest_sizes[alg_count].algorithm_id);
			put_unaligned_le16(hash_len,
					   &spec_event->digest_sizes[alg_count].digest_size);
			alg_count++;
		}
	}

	spec_event->number_of_algorithms = alg_count;
	if (spec_event->number_of_algorithms > MAX_HASH_COUNT ||
	    spec_event->number_of_algorithms < 1)
		goto out;

	/*
	 * the size of the spec event and placement of vendor_info_size
	 * depends on supported algoriths
	 */
	spec_event_size =
		offsetof(struct tcg_efi_spec_id_event, digest_sizes) +
		spec_event->number_of_algorithms * sizeof(spec_event->digest_sizes[0]);
	/* no vendor info for us */
	memset(buffer + spec_event_size, 0, 1);
	/* add a byte for vendor_info_size in the spec event */
	spec_event_size += 1;
	*event_size = spec_event_size;

	return EFI_SUCCESS;

out:
	return ret;
}

/**
 * tcg2_uninit - remove the final event table and free efi memory on failures
 */
void tcg2_uninit(void)
{
	efi_status_t ret;

	ret = efi_install_configuration_table(&efi_guid_final_events, NULL);
	if (ret != EFI_SUCCESS)
		log_err("Failed to delete final events config table\n");

	efi_free_pool(event_log.buffer);
	event_log.buffer = NULL;
	efi_free_pool(event_log.final_buffer);
	event_log.final_buffer = NULL;

	if (!is_tcg2_protocol_installed())
		return;

	ret = efi_remove_protocol(efi_root, &efi_guid_tcg2_protocol,
				  (void *)&efi_tcg2_protocol);
	if (ret != EFI_SUCCESS)
		log_err("Failed to remove EFI TCG2 protocol\n");
}

/**
 * create_final_event() - Create the final event and install the config
 *			defined by the TCG EFI spec
 */
static efi_status_t create_final_event(void)
{
	struct efi_tcg2_final_events_table *final_event;
	efi_status_t ret;

	/*
	 * All events generated after the invocation of
	 * EFI_TCG2_GET_EVENT_LOGS need to be stored in an instance of an
	 * EFI_CONFIGURATION_TABLE
	 */
	ret = efi_allocate_pool(EFI_ACPI_MEMORY_NVS, TPM2_EVENT_LOG_SIZE,
				&event_log.final_buffer);
	if (ret != EFI_SUCCESS)
		goto out;

	memset(event_log.final_buffer, 0xff, TPM2_EVENT_LOG_SIZE);
	final_event = event_log.final_buffer;
	final_event->number_of_events = 0;
	final_event->version = EFI_TCG2_FINAL_EVENTS_TABLE_VERSION;
	event_log.final_pos = sizeof(*final_event);
	ret = efi_install_configuration_table(&efi_guid_final_events,
					      final_event);
	if (ret != EFI_SUCCESS) {
		efi_free_pool(event_log.final_buffer);
		event_log.final_buffer = NULL;
	}

out:
	return ret;
}

/**
 * tcg2_measure_event() - common function to add event log and extend PCR
 *
 * @dev:		TPM device
 * @pcr_index:		PCR index
 * @event_type:		type of event added
 * @size:		event size
 * @event:		event data
 *
 * Return:	status code
 */
static efi_status_t
tcg2_measure_event(struct udevice *dev, u32 pcr_index, u32 event_type,
		   u32 size, u8 event[])
{
	struct tpml_digest_values digest_list;
	efi_status_t ret;

	ret = tcg2_create_digest(event, size, &digest_list);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = tcg2_pcr_extend(dev, pcr_index, &digest_list);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = tcg2_agile_log_append(pcr_index, event_type, &digest_list,
				    size, event);

out:
	return ret;
}

/**
 * efi_append_scrtm_version - Append an S-CRTM EV_S_CRTM_VERSION event on the
 *			      eventlog and extend the PCRs
 *
 * @dev:	TPM device
 *
 * @Return:	status code
 */
static efi_status_t efi_append_scrtm_version(struct udevice *dev)
{
	efi_status_t ret;

	ret = tcg2_measure_event(dev, 0, EV_S_CRTM_VERSION,
				 strlen(version_string) + 1,
				 (u8 *)version_string);

	return ret;
}

/**
 * efi_init_event_log() - initialize an eventlog
 *
 * Return:		status code
 */
static efi_status_t efi_init_event_log(void)
{
	/*
	 * vendor_info_size is currently set to 0, we need to change the length
	 * and allocate the flexible array member if this changes
	 */
	struct tcg_pcr_event *event_header = NULL;
	struct udevice *dev;
	size_t spec_event_size;
	efi_status_t ret;

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS)
		return ret;

	ret = efi_allocate_pool(EFI_BOOT_SERVICES_DATA, TPM2_EVENT_LOG_SIZE,
				(void **)&event_log.buffer);
	if (ret != EFI_SUCCESS)
		return ret;

	/*
	 * initialize log area as 0xff so the OS can easily figure out the
	 * last log entry
	 */
	memset(event_log.buffer, 0xff, TPM2_EVENT_LOG_SIZE);

	/*
	 * The log header is defined to be in SHA1 event log entry format.
	 * Setup event header
	 */
	event_header =  (struct tcg_pcr_event *)event_log.buffer;
	event_log.pos = 0;
	event_log.last_event_size = 0;
	event_log.get_event_called = false;
	event_log.ebs_called = false;
	event_log.truncated = false;

	/*
	 * Check if earlier firmware have passed any eventlog. Different
	 * platforms can use different ways to do so.
	 */
	ret = tcg2_get_fw_eventlog(dev, event_log.buffer, &event_log.pos);
	/*
	 * If earlier firmware hasn't passed any eventlog, go ahead and
	 * create the eventlog header.
	 */
	if (ret == EFI_NOT_FOUND) {
		put_unaligned_le32(0, &event_header->pcr_index);
		put_unaligned_le32(EV_NO_ACTION, &event_header->event_type);
		memset(&event_header->digest, 0, sizeof(event_header->digest));
		ret = create_specid_event(dev,
					  (void *)((uintptr_t)event_log.buffer +
						   sizeof(*event_header)),
					  &spec_event_size);
		if (ret != EFI_SUCCESS)
			goto free_pool;
		put_unaligned_le32(spec_event_size, &event_header->event_size);
		event_log.pos = spec_event_size + sizeof(*event_header);
		event_log.last_event_size = event_log.pos;

		/*
		 * Add SCRTM version to the log if previous firmmware
		 * doesn't pass an eventlog.
		 */
		ret = efi_append_scrtm_version(dev);
	}

	if (ret != EFI_SUCCESS)
		goto free_pool;

	ret = create_final_event();
	if (ret != EFI_SUCCESS)
		goto free_pool;

	return ret;

free_pool:
	efi_free_pool(event_log.buffer);
	event_log.buffer = NULL;
	return ret;
}

/**
 * tcg2_measure_variable() - add variable event log and extend PCR
 *
 * @dev:		TPM device
 * @pcr_index:		PCR index
 * @event_type:		type of event added
 * @var_name:		variable name
 * @guid:		guid
 * @data_size:		variable data size
 * @data:		variable data
 *
 * Return:	status code
 */
static efi_status_t tcg2_measure_variable(struct udevice *dev, u32 pcr_index,
					  u32 event_type, const u16 *var_name,
					  const efi_guid_t *guid,
					  efi_uintn_t data_size, u8 *data)
{
	u32 event_size;
	efi_status_t ret;
	struct efi_tcg2_uefi_variable_data *event;

	event_size = sizeof(event->variable_name) +
		     sizeof(event->unicode_name_length) +
		     sizeof(event->variable_data_length) +
		     (u16_strlen(var_name) * sizeof(u16)) + data_size;
	event = malloc(event_size);
	if (!event)
		return EFI_OUT_OF_RESOURCES;

	guidcpy(&event->variable_name, guid);
	event->unicode_name_length = u16_strlen(var_name);
	event->variable_data_length = data_size;
	memcpy(event->unicode_name, var_name,
	       (event->unicode_name_length * sizeof(u16)));
	if (data) {
		memcpy((u16 *)event->unicode_name + event->unicode_name_length,
		       data, data_size);
	}
	ret = tcg2_measure_event(dev, pcr_index, event_type, event_size,
				 (u8 *)event);
	free(event);
	return ret;
}

/**
 * tcg2_measure_boot_variable() - measure boot variables
 *
 * @dev:	TPM device
 *
 * Return:	status code
 */
static efi_status_t tcg2_measure_boot_variable(struct udevice *dev)
{
	u16 *boot_order;
	u16 *boot_index;
	u16 var_name[] = u"BootOrder";
	u16 boot_name[] = u"Boot####";
	u8 *bootvar;
	efi_uintn_t var_data_size;
	u32 count, i;
	efi_status_t ret;

	boot_order = efi_get_var(var_name, &efi_global_variable_guid,
				 &var_data_size);
	if (!boot_order) {
		/* If "BootOrder" is not defined, skip the boot variable measurement */
		return EFI_SUCCESS;
	}

	ret = tcg2_measure_variable(dev, 1, EV_EFI_VARIABLE_BOOT2, var_name,
				    &efi_global_variable_guid, var_data_size,
				    (u8 *)boot_order);
	if (ret != EFI_SUCCESS)
		goto error;

	count = var_data_size / sizeof(*boot_order);
	boot_index = boot_order;
	for (i = 0; i < count; i++) {
		efi_create_indexed_name(boot_name, sizeof(boot_name),
					"Boot", *boot_index++);

		bootvar = efi_get_var(boot_name, &efi_global_variable_guid,
				      &var_data_size);

		if (!bootvar) {
			log_debug("%ls not found\n", boot_name);
			continue;
		}

		ret = tcg2_measure_variable(dev, 1, EV_EFI_VARIABLE_BOOT2,
					    boot_name,
					    &efi_global_variable_guid,
					    var_data_size, bootvar);
		free(bootvar);
		if (ret != EFI_SUCCESS)
			goto error;
	}

error:
	free(boot_order);
	return ret;
}

/**
 * tcg2_measure_smbios() - measure smbios table
 *
 * @dev:	TPM device
 * @entry:	pointer to the smbios_entry structure
 *
 * Return:	status code
 */
static efi_status_t
tcg2_measure_smbios(struct udevice *dev,
		    const struct smbios_entry *entry)
{
	efi_status_t ret;
	struct smbios_header *smbios_copy;
	struct smbios_handoff_table_pointers2 *event = NULL;
	u32 event_size;

	/*
	 * TCG PC Client PFP Spec says
	 * "SMBIOS structures that contain static configuration information
	 * (e.g. Platform Manufacturer Enterprise Number assigned by IANA,
	 * platform model number, Vendor and Device IDs for each SMBIOS table)
	 * that is relevant to the security of the platform MUST be measured".
	 * Device dependent parameters such as serial number are cleared to
	 * zero or spaces for the measurement.
	 */
	event_size = sizeof(struct smbios_handoff_table_pointers2) +
		     FIELD_SIZEOF(struct efi_configuration_table, guid) +
		     entry->struct_table_length;
	event = calloc(1, event_size);
	if (!event) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

	event->table_description_size = sizeof(SMBIOS_HANDOFF_TABLE_DESC);
	memcpy(event->table_description, SMBIOS_HANDOFF_TABLE_DESC,
	       sizeof(SMBIOS_HANDOFF_TABLE_DESC));
	put_unaligned_le64(1, &event->number_of_tables);
	guidcpy(&event->table_entry[0].guid, &smbios_guid);
	smbios_copy = (struct smbios_header *)((uintptr_t)&event->table_entry[0].table);
	memcpy(&event->table_entry[0].table,
	       (void *)((uintptr_t)entry->struct_table_address),
	       entry->struct_table_length);

	smbios_prepare_measurement(entry, smbios_copy);

	ret = tcg2_measure_event(dev, 1, EV_EFI_HANDOFF_TABLES2, event_size,
				 (u8 *)event);
	if (ret != EFI_SUCCESS)
		goto out;

out:
	free(event);

	return ret;
}

/**
 * find_smbios_table() - find smbios table
 *
 * Return:	pointer to the smbios table
 */
static void *find_smbios_table(void)
{
	u32 i;

	for (i = 0; i < systab.nr_tables; i++) {
		if (!guidcmp(&smbios_guid, &systab.tables[i].guid))
			return systab.tables[i].table;
	}

	return NULL;
}

/**
 * tcg2_measure_gpt_table() - measure gpt table
 *
 * @dev:		TPM device
 * @loaded_image:	handle to the loaded image
 *
 * Return:	status code
 */
static efi_status_t
tcg2_measure_gpt_data(struct udevice *dev,
		      struct efi_loaded_image_obj *loaded_image)
{
	efi_status_t ret;
	efi_handle_t handle;
	struct efi_handler *dp_handler;
	struct efi_device_path *orig_device_path;
	struct efi_device_path *device_path;
	struct efi_device_path *dp;
	struct efi_block_io *block_io;
	struct efi_gpt_data *event = NULL;
	efi_guid_t null_guid = NULL_GUID;
	gpt_header *gpt_h;
	gpt_entry *entry = NULL;
	gpt_entry *gpt_e;
	u32 num_of_valid_entry = 0;
	u32 event_size;
	u32 i;
	u32 total_gpt_entry_size;

	ret = efi_search_protocol(&loaded_image->header,
				  &efi_guid_loaded_image_device_path,
				  &dp_handler);
	if (ret != EFI_SUCCESS)
		return ret;

	orig_device_path = dp_handler->protocol_interface;
	if (!orig_device_path) /* no device path, skip GPT measurement */
		return EFI_SUCCESS;

	device_path = efi_dp_dup(orig_device_path);
	if (!device_path)
		return EFI_OUT_OF_RESOURCES;

	dp = search_gpt_dp_node(device_path);
	if (!dp) {
		/* no GPT device path node found, skip GPT measurement */
		ret = EFI_SUCCESS;
		goto out1;
	}

	/* read GPT header */
	dp->type = DEVICE_PATH_TYPE_END;
	dp->sub_type = DEVICE_PATH_SUB_TYPE_END;
	dp = device_path;
	ret = EFI_CALL(systab.boottime->locate_device_path(&efi_block_io_guid,
							   &dp, &handle));
	if (ret != EFI_SUCCESS)
		goto out1;

	ret = EFI_CALL(efi_handle_protocol(handle,
					   &efi_block_io_guid, (void **)&block_io));
	if (ret != EFI_SUCCESS)
		goto out1;

	gpt_h = memalign(block_io->media->io_align, block_io->media->block_size);
	if (!gpt_h) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out2;
	}

	ret = block_io->read_blocks(block_io, block_io->media->media_id, 1,
				    block_io->media->block_size, gpt_h);
	if (ret != EFI_SUCCESS)
		goto out2;

	/* read GPT entry */
	total_gpt_entry_size = gpt_h->num_partition_entries *
			       gpt_h->sizeof_partition_entry;
	entry = memalign(block_io->media->io_align, total_gpt_entry_size);
	if (!entry) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out2;
	}

	ret = block_io->read_blocks(block_io, block_io->media->media_id,
				    gpt_h->partition_entry_lba,
				    total_gpt_entry_size, entry);
	if (ret != EFI_SUCCESS)
		goto out2;

	/* count valid GPT entry */
	gpt_e = entry;
	for (i = 0; i < gpt_h->num_partition_entries; i++) {
		if (guidcmp(&null_guid, &gpt_e->partition_type_guid))
			num_of_valid_entry++;

		gpt_e = (gpt_entry *)((u8 *)gpt_e + gpt_h->sizeof_partition_entry);
	}

	/* prepare event data for measurement */
	event_size = sizeof(struct efi_gpt_data) +
		(num_of_valid_entry * gpt_h->sizeof_partition_entry);
	event = calloc(1, event_size);
	if (!event) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out2;
	}
	memcpy(event, gpt_h, sizeof(gpt_header));
	put_unaligned_le64(num_of_valid_entry, &event->number_of_partitions);

	/* copy valid GPT entry */
	gpt_e = entry;
	num_of_valid_entry = 0;
	for (i = 0; i < gpt_h->num_partition_entries; i++) {
		if (guidcmp(&null_guid, &gpt_e->partition_type_guid)) {
			memcpy((u8 *)event->partitions +
			       (num_of_valid_entry * gpt_h->sizeof_partition_entry),
			       gpt_e, gpt_h->sizeof_partition_entry);
			num_of_valid_entry++;
		}

		gpt_e = (gpt_entry *)((u8 *)gpt_e + gpt_h->sizeof_partition_entry);
	}

	ret = tcg2_measure_event(dev, 5, EV_EFI_GPT_EVENT, event_size, (u8 *)event);
	if (ret != EFI_SUCCESS)
		goto out2;

out2:
	EFI_CALL(efi_close_protocol((efi_handle_t)block_io, &efi_block_io_guid,
				    NULL, NULL));
	free(gpt_h);
	free(entry);
	free(event);
out1:
	efi_free_pool(device_path);

	return ret;
}

/**
 * efi_tcg2_measure_efi_app_invocation() - measure efi app invocation
 *
 * Return:	status code
 */
efi_status_t efi_tcg2_measure_efi_app_invocation(struct efi_loaded_image_obj *handle)
{
	efi_status_t ret;
	u32 pcr_index;
	struct udevice *dev;
	u32 event = 0;
	struct smbios_entry *entry;

	if (!is_tcg2_protocol_installed())
		return EFI_SUCCESS;

	if (tcg2_efi_app_invoked)
		return EFI_SUCCESS;

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS)
		return EFI_SECURITY_VIOLATION;

	ret = tcg2_measure_boot_variable(dev);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = tcg2_measure_event(dev, 4, EV_EFI_ACTION,
				 strlen(EFI_CALLING_EFI_APPLICATION),
				 (u8 *)EFI_CALLING_EFI_APPLICATION);
	if (ret != EFI_SUCCESS)
		goto out;

	entry = (struct smbios_entry *)find_smbios_table();
	if (entry) {
		ret = tcg2_measure_smbios(dev, entry);
		if (ret != EFI_SUCCESS)
			goto out;
	}

	ret = tcg2_measure_gpt_data(dev, handle);
	if (ret != EFI_SUCCESS)
		goto out;

	for (pcr_index = 0; pcr_index <= 7; pcr_index++) {
		ret = tcg2_measure_event(dev, pcr_index, EV_SEPARATOR,
					 sizeof(event), (u8 *)&event);
		if (ret != EFI_SUCCESS)
			goto out;
	}

	tcg2_efi_app_invoked = true;
out:
	return ret;
}

/**
 * efi_tcg2_measure_efi_app_exit() - measure efi app exit
 *
 * Return:	status code
 */
efi_status_t efi_tcg2_measure_efi_app_exit(void)
{
	efi_status_t ret;
	struct udevice *dev;

	if (!is_tcg2_protocol_installed())
		return EFI_SUCCESS;

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS)
		return ret;

	ret = tcg2_measure_event(dev, 4, EV_EFI_ACTION,
				 strlen(EFI_RETURNING_FROM_EFI_APPLICATION),
				 (u8 *)EFI_RETURNING_FROM_EFI_APPLICATION);
	return ret;
}

/**
 * efi_tcg2_notify_exit_boot_services() - ExitBootService callback
 *
 * @event:	callback event
 * @context:	callback context
 */
static void EFIAPI
efi_tcg2_notify_exit_boot_services(struct efi_event *event, void *context)
{
	efi_status_t ret;
	struct udevice *dev;

	EFI_ENTRY("%p, %p", event, context);

	event_log.ebs_called = true;

	if (!is_tcg2_protocol_installed()) {
		ret = EFI_SUCCESS;
		goto out;
	}

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = tcg2_measure_event(dev, 5, EV_EFI_ACTION,
				 strlen(EFI_EXIT_BOOT_SERVICES_INVOCATION),
				 (u8 *)EFI_EXIT_BOOT_SERVICES_INVOCATION);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = tcg2_measure_event(dev, 5, EV_EFI_ACTION,
				 strlen(EFI_EXIT_BOOT_SERVICES_SUCCEEDED),
				 (u8 *)EFI_EXIT_BOOT_SERVICES_SUCCEEDED);

out:
	EFI_EXIT(ret);
}

/**
 * efi_tcg2_notify_exit_boot_services_failed()
 *  - notify ExitBootServices() is failed
 *
 * Return:	status code
 */
efi_status_t efi_tcg2_notify_exit_boot_services_failed(void)
{
	struct udevice *dev;
	efi_status_t ret;

	if (!is_tcg2_protocol_installed())
		return EFI_SUCCESS;

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = tcg2_measure_event(dev, 5, EV_EFI_ACTION,
				 strlen(EFI_EXIT_BOOT_SERVICES_INVOCATION),
				 (u8 *)EFI_EXIT_BOOT_SERVICES_INVOCATION);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = tcg2_measure_event(dev, 5, EV_EFI_ACTION,
				 strlen(EFI_EXIT_BOOT_SERVICES_FAILED),
				 (u8 *)EFI_EXIT_BOOT_SERVICES_FAILED);

out:
	return ret;
}

/**
 * tcg2_measure_secure_boot_variable() - measure secure boot variables
 *
 * @dev:	TPM device
 *
 * Return:	status code
 */
static efi_status_t tcg2_measure_secure_boot_variable(struct udevice *dev)
{
	u8 *data;
	efi_uintn_t data_size;
	u32 count, i;
	efi_status_t ret;
	u8 deployed_mode;
	efi_uintn_t size;
	u32 deployed_audit_pcr_index = 1;

	size = sizeof(deployed_mode);
	ret = efi_get_variable_int(u"DeployedMode", &efi_global_variable_guid,
				   NULL, &size, &deployed_mode, NULL);
	if (ret != EFI_SUCCESS || !deployed_mode)
		deployed_audit_pcr_index = 7;

	count = ARRAY_SIZE(secure_variables);
	for (i = 0; i < count; i++) {
		const efi_guid_t *guid;

		guid = efi_auth_var_get_guid(secure_variables[i].name);

		data = efi_get_var(secure_variables[i].name, guid, &data_size);
		if (!data && !secure_variables[i].accept_empty)
			continue;

		if (u16_strcmp(u"DeployedMode", secure_variables[i].name))
			secure_variables[i].pcr_index = deployed_audit_pcr_index;
		if (u16_strcmp(u"AuditMode", secure_variables[i].name))
			secure_variables[i].pcr_index = deployed_audit_pcr_index;

		ret = tcg2_measure_variable(dev, secure_variables[i].pcr_index,
					    EV_EFI_VARIABLE_DRIVER_CONFIG,
					    secure_variables[i].name, guid,
					    data_size, data);
		free(data);
		if (ret != EFI_SUCCESS)
			goto error;
	}

error:
	return ret;
}

/**
 * efi_tcg2_do_initial_measurement() - do initial measurement
 *
 * Return:	status code
 */
efi_status_t efi_tcg2_do_initial_measurement(void)
{
	efi_status_t ret;
	struct udevice *dev;

	if (!is_tcg2_protocol_installed())
		return EFI_SUCCESS;

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS)
		return EFI_SECURITY_VIOLATION;

	ret = tcg2_measure_secure_boot_variable(dev);
	if (ret != EFI_SUCCESS)
		goto out;

out:
	return ret;
}

/**
 * efi_tcg2_register() - register EFI_TCG2_PROTOCOL
 *
 * If a TPM2 device is available, the TPM TCG2 Protocol is registered
 *
 * Return:	status code
 */
efi_status_t efi_tcg2_register(void)
{
	efi_status_t ret = EFI_SUCCESS;
	struct udevice *dev;
	struct efi_event *event;
	u32 err;

	ret = platform_get_tpm2_device(&dev);
	if (ret != EFI_SUCCESS) {
		log_warning("Unable to find TPMv2 device\n");
		return EFI_SUCCESS;
	}

	/* initialize the TPM as early as possible. */
	err = tpm_startup(dev, TPM_ST_CLEAR);
	if (err) {
		log_err("TPM startup failed\n");
		goto fail;
	}

	ret = efi_init_event_log();
	if (ret != EFI_SUCCESS) {
		tcg2_uninit();
		goto fail;
	}

	ret = efi_add_protocol(efi_root, &efi_guid_tcg2_protocol,
			       (void *)&efi_tcg2_protocol);
	if (ret != EFI_SUCCESS) {
		tcg2_uninit();
		goto fail;
	}

	ret = efi_create_event(EVT_SIGNAL_EXIT_BOOT_SERVICES, TPL_CALLBACK,
			       efi_tcg2_notify_exit_boot_services, NULL,
			       NULL, &event);
	if (ret != EFI_SUCCESS) {
		tcg2_uninit();
		goto fail;
	}

	return ret;

fail:
	log_err("Cannot install EFI_TCG2_PROTOCOL\n");
	return ret;
}
