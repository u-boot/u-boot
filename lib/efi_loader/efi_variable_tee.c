// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI variable service via OP-TEE
 *
 *  Copyright (C) 2019 Linaro Ltd. <sughosh.ganu@linaro.org>
 *  Copyright (C) 2019 Linaro Ltd. <ilias.apalodimas@linaro.org>
 */

#include <common.h>
#include <efi.h>
#include <efi_api.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <tee.h>
#include <malloc.h>
#include <mm_communication.h>

#define OPTEE_PAGE_SIZE BIT(12)
extern struct efi_var_file __efi_runtime_data *efi_var_buf;
static efi_uintn_t max_buffer_size;	/* comm + var + func + data */
static efi_uintn_t max_payload_size;	/* func + data */

struct mm_connection {
	struct udevice *tee;
	u32 session;
};

/**
 * get_connection() - Retrieve OP-TEE session for a specific UUID.
 *
 * @conn:   session buffer to fill
 * Return:  status code
 */
static int get_connection(struct mm_connection *conn)
{
	static const struct tee_optee_ta_uuid uuid = PTA_STMM_UUID;
	struct udevice *tee = NULL;
	struct tee_open_session_arg arg;
	int rc = -ENODEV;

	tee = tee_find_device(tee, NULL, NULL, NULL);
	if (!tee)
		goto out;

	memset(&arg, 0, sizeof(arg));
	tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
	rc = tee_open_session(tee, &arg, 0, NULL);
	if (rc)
		goto out;

	/* Check the internal OP-TEE result */
	if (arg.ret != TEE_SUCCESS) {
		rc = -EIO;
		goto out;
	}

	conn->tee = tee;
	conn->session = arg.session;

	return 0;
out:
	return rc;
}

/**
 * optee_mm_communicate() - Pass a buffer to StandaloneMM running in OP-TEE
 *
 * @comm_buf:		locally allocted communcation buffer
 * @dsize:		buffer size
 * Return:		status code
 */
static efi_status_t optee_mm_communicate(void *comm_buf, ulong dsize)
{
	ulong buf_size;
	efi_status_t ret;
	struct efi_mm_communicate_header *mm_hdr;
	struct mm_connection conn = { NULL, 0 };
	struct tee_invoke_arg arg;
	struct tee_param param[2];
	struct tee_shm *shm = NULL;
	int rc;

	if (!comm_buf)
		return EFI_INVALID_PARAMETER;

	mm_hdr = (struct efi_mm_communicate_header *)comm_buf;
	buf_size = mm_hdr->message_len + sizeof(efi_guid_t) + sizeof(size_t);

	if (dsize != buf_size)
		return EFI_INVALID_PARAMETER;

	rc = get_connection(&conn);
	if (rc) {
		log_err("Unable to open OP-TEE session (err=%d)\n", rc);
		return EFI_UNSUPPORTED;
	}

	if (tee_shm_register(conn.tee, comm_buf, buf_size, 0, &shm)) {
		log_err("Unable to register shared memory\n");
		tee_close_session(conn.tee, conn.session);
		return EFI_UNSUPPORTED;
	}

	memset(&arg, 0, sizeof(arg));
	arg.func = PTA_STMM_CMDID_COMMUNICATE;
	arg.session = conn.session;

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[0].u.memref.size = buf_size;
	param[0].u.memref.shm = shm;
	param[1].attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT;

	rc = tee_invoke_func(conn.tee, &arg, 2, param);
	tee_shm_free(shm);
	tee_close_session(conn.tee, conn.session);
	if (rc || arg.ret != TEE_SUCCESS)
		return EFI_DEVICE_ERROR;

	switch (param[1].u.value.a) {
	case ARM_SVC_SPM_RET_SUCCESS:
		ret = EFI_SUCCESS;
		break;

	case ARM_SVC_SPM_RET_INVALID_PARAMS:
		ret = EFI_INVALID_PARAMETER;
		break;

	case ARM_SVC_SPM_RET_DENIED:
		ret = EFI_ACCESS_DENIED;
		break;

	case ARM_SVC_SPM_RET_NO_MEMORY:
		ret = EFI_OUT_OF_RESOURCES;
		break;

	default:
		ret = EFI_ACCESS_DENIED;
	}

	return ret;
}

/**
 * mm_communicate() - Adjust the cmonnucation buffer to StandAlonneMM and send
 * it to OP-TEE
 *
 * @comm_buf:		locally allocted communcation buffer
 * @dsize:		buffer size
 * Return:		status code
 */
static efi_status_t mm_communicate(u8 *comm_buf, efi_uintn_t dsize)
{
	efi_status_t ret;
	struct efi_mm_communicate_header *mm_hdr;
	struct smm_variable_communicate_header *var_hdr;

	dsize += MM_COMMUNICATE_HEADER_SIZE + MM_VARIABLE_COMMUNICATE_SIZE;
	mm_hdr = (struct efi_mm_communicate_header *)comm_buf;
	var_hdr = (struct smm_variable_communicate_header *)mm_hdr->data;

	ret = optee_mm_communicate(comm_buf, dsize);
	if (ret != EFI_SUCCESS) {
		log_err("%s failed!\n", __func__);
		return ret;
	}

	return var_hdr->ret_status;
}

/**
 * setup_mm_hdr() -	Allocate a buffer for StandAloneMM and initialize the
 *			header data.
 *
 * @dptr:		pointer address of the corresponding StandAloneMM
 *			function
 * @payload_size:	buffer size
 * @func:		standAloneMM function number
 * @ret:		EFI return code
 * Return:		buffer or NULL
 */
static u8 *setup_mm_hdr(void **dptr, efi_uintn_t payload_size,
			efi_uintn_t func, efi_status_t *ret)
{
	const efi_guid_t mm_var_guid = EFI_MM_VARIABLE_GUID;
	struct efi_mm_communicate_header *mm_hdr;
	struct smm_variable_communicate_header *var_hdr;
	u8 *comm_buf;

	/* In the init function we initialize max_buffer_size with
	 * get_max_payload(). So skip the test if max_buffer_size is initialized
	 * StandAloneMM will perform similar checks and drop the buffer if it's
	 * too long
	 */
	if (max_buffer_size && max_buffer_size <
			(MM_COMMUNICATE_HEADER_SIZE +
			 MM_VARIABLE_COMMUNICATE_SIZE +
			 payload_size)) {
		*ret = EFI_INVALID_PARAMETER;
		return NULL;
	}

	comm_buf = calloc(1, MM_COMMUNICATE_HEADER_SIZE +
			  MM_VARIABLE_COMMUNICATE_SIZE +
			  payload_size);
	if (!comm_buf) {
		*ret = EFI_OUT_OF_RESOURCES;
		return NULL;
	}

	mm_hdr = (struct efi_mm_communicate_header *)comm_buf;
	guidcpy(&mm_hdr->header_guid, &mm_var_guid);
	mm_hdr->message_len = MM_VARIABLE_COMMUNICATE_SIZE + payload_size;

	var_hdr = (struct smm_variable_communicate_header *)mm_hdr->data;
	var_hdr->function = func;
	if (dptr)
		*dptr = var_hdr->data;
	*ret = EFI_SUCCESS;

	return comm_buf;
}

/**
 * get_max_payload() - Get variable payload size from StandAloneMM.
 *
 * @size:    size of the variable in storage
 * Return:   status code
 */
efi_status_t EFIAPI get_max_payload(efi_uintn_t *size)
{
	struct smm_variable_payload_size *var_payload = NULL;
	efi_uintn_t payload_size;
	u8 *comm_buf = NULL;
	efi_status_t ret;

	if (!size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	payload_size = sizeof(*var_payload);
	comm_buf = setup_mm_hdr((void **)&var_payload, payload_size,
				SMM_VARIABLE_FUNCTION_GET_PAYLOAD_SIZE, &ret);
	if (!comm_buf)
		goto out;

	ret = mm_communicate(comm_buf, payload_size);
	if (ret != EFI_SUCCESS)
		goto out;

	/* Make sure the buffer is big enough for storing variables */
	if (var_payload->size < MM_VARIABLE_ACCESS_HEADER_SIZE + 0x20) {
		ret = EFI_DEVICE_ERROR;
		goto out;
	}
	*size = var_payload->size;
	/*
	 * Although the max payload is configurable on StMM, we only share a
	 * single page from OP-TEE for the non-secure buffer used to communicate
	 * with StMM. Since OP-TEE will reject to map anything bigger than that,
	 * make sure we are in bounds.
	 */
	if (*size > OPTEE_PAGE_SIZE)
		*size = OPTEE_PAGE_SIZE - MM_COMMUNICATE_HEADER_SIZE  -
			MM_VARIABLE_COMMUNICATE_SIZE;
	/*
	 * There seems to be a bug in EDK2 miscalculating the boundaries and
	 * size checks, so deduct 2 more bytes to fulfill this requirement. Fix
	 * it up here to ensure backwards compatibility with older versions
	 * (cf. StandaloneMmPkg/Drivers/StandaloneMmCpu/AArch64/EventHandle.c.
	 * sizeof (EFI_MM_COMMUNICATE_HEADER) instead the size minus the
	 * flexible array member).
	 *
	 * size is guaranteed to be > 2 due to checks on the beginning.
	 */
	*size -= 2;
out:
	free(comm_buf);
	return ret;
}

/*
 * StMM can store internal attributes and properties for variables, i.e enabling
 * R/O variables
 */
static efi_status_t set_property_int(u16 *variable_name, efi_uintn_t name_size,
				     const efi_guid_t *vendor,
				     struct var_check_property *var_property)
{
	struct smm_variable_var_check_property *smm_property;
	efi_uintn_t payload_size;
	u8 *comm_buf = NULL;
	efi_status_t ret;

	payload_size = sizeof(*smm_property) + name_size;
	if (payload_size > max_payload_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	comm_buf = setup_mm_hdr((void **)&smm_property, payload_size,
				SMM_VARIABLE_FUNCTION_VAR_CHECK_VARIABLE_PROPERTY_SET,
				&ret);
	if (!comm_buf)
		goto out;

	guidcpy(&smm_property->guid, vendor);
	smm_property->name_size = name_size;
	memcpy(&smm_property->property, var_property,
	       sizeof(smm_property->property));
	memcpy(smm_property->name, variable_name, name_size);

	ret = mm_communicate(comm_buf, payload_size);

out:
	free(comm_buf);
	return ret;
}

static efi_status_t get_property_int(u16 *variable_name, efi_uintn_t name_size,
				     const efi_guid_t *vendor,
				     struct var_check_property *var_property)
{
	struct smm_variable_var_check_property *smm_property;
	efi_uintn_t payload_size;
	u8 *comm_buf = NULL;
	efi_status_t ret;

	memset(var_property, 0, sizeof(*var_property));
	payload_size = sizeof(*smm_property) + name_size;
	if (payload_size > max_payload_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	comm_buf = setup_mm_hdr((void **)&smm_property, payload_size,
				SMM_VARIABLE_FUNCTION_VAR_CHECK_VARIABLE_PROPERTY_GET,
				&ret);
	if (!comm_buf)
		goto out;

	guidcpy(&smm_property->guid, vendor);
	smm_property->name_size = name_size;
	memcpy(smm_property->name, variable_name, name_size);

	ret = mm_communicate(comm_buf, payload_size);
	/*
	 * Currently only R/O property is supported in StMM.
	 * Variables that are not set to R/O will not set the property in StMM
	 * and the call will return EFI_NOT_FOUND. We are setting the
	 * properties to 0x0 so checking against that is enough for the
	 * EFI_NOT_FOUND case.
	 */
	if (ret == EFI_NOT_FOUND)
		ret = EFI_SUCCESS;
	if (ret != EFI_SUCCESS)
		goto out;
	memcpy(var_property, &smm_property->property, sizeof(*var_property));

out:
	free(comm_buf);
	return ret;
}

efi_status_t efi_get_variable_int(u16 *variable_name, const efi_guid_t *vendor,
				  u32 *attributes, efi_uintn_t *data_size,
				  void *data, u64 *timep)
{
	struct var_check_property var_property;
	struct smm_variable_access *var_acc;
	efi_uintn_t payload_size;
	efi_uintn_t name_size;
	efi_uintn_t tmp_dsize;
	u8 *comm_buf = NULL;
	efi_status_t ret;

	if (!variable_name || !vendor || !data_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Check payload size */
	name_size = u16_strsize(variable_name);
	if (name_size > max_payload_size - MM_VARIABLE_ACCESS_HEADER_SIZE) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Trim output buffer size */
	tmp_dsize = *data_size;
	if (name_size + tmp_dsize >
			max_payload_size - MM_VARIABLE_ACCESS_HEADER_SIZE) {
		tmp_dsize = max_payload_size -
				MM_VARIABLE_ACCESS_HEADER_SIZE -
				name_size;
	}

	/* Get communication buffer and initialize header */
	payload_size = MM_VARIABLE_ACCESS_HEADER_SIZE + name_size + tmp_dsize;
	comm_buf = setup_mm_hdr((void **)&var_acc, payload_size,
				SMM_VARIABLE_FUNCTION_GET_VARIABLE, &ret);
	if (!comm_buf)
		goto out;

	/* Fill in contents */
	guidcpy(&var_acc->guid, vendor);
	var_acc->data_size = tmp_dsize;
	var_acc->name_size = name_size;
	var_acc->attr = attributes ? *attributes : 0;
	memcpy(var_acc->name, variable_name, name_size);

	/* Communicate */
	ret = mm_communicate(comm_buf, payload_size);
	if (ret == EFI_SUCCESS || ret == EFI_BUFFER_TOO_SMALL) {
		/* Update with reported data size for trimmed case */
		*data_size = var_acc->data_size;
	}
	if (ret != EFI_SUCCESS)
		goto out;

	ret = get_property_int(variable_name, name_size, vendor, &var_property);
	if (ret != EFI_SUCCESS)
		goto out;

	if (attributes) {
		*attributes = var_acc->attr;
		if (var_property.property & VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY)
			*attributes |= EFI_VARIABLE_READ_ONLY;
	}

	if (data)
		memcpy(data, (u8 *)var_acc->name + var_acc->name_size,
		       var_acc->data_size);
	else
		ret = EFI_INVALID_PARAMETER;

out:
	free(comm_buf);
	return ret;
}

efi_status_t efi_get_next_variable_name_int(efi_uintn_t *variable_name_size,
					    u16 *variable_name,
					    efi_guid_t *guid)
{
	struct smm_variable_getnext *var_getnext;
	efi_uintn_t payload_size;
	efi_uintn_t out_name_size;
	efi_uintn_t in_name_size;
	u8 *comm_buf = NULL;
	efi_status_t ret;

	if (!variable_name_size || !variable_name || !guid) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	out_name_size = *variable_name_size;
	in_name_size = u16_strsize(variable_name);

	if (out_name_size < in_name_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (in_name_size > max_payload_size - MM_VARIABLE_GET_NEXT_HEADER_SIZE) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Trim output buffer size */
	if (out_name_size > max_payload_size - MM_VARIABLE_GET_NEXT_HEADER_SIZE)
		out_name_size = max_payload_size - MM_VARIABLE_GET_NEXT_HEADER_SIZE;

	payload_size = MM_VARIABLE_GET_NEXT_HEADER_SIZE + out_name_size;
	comm_buf = setup_mm_hdr((void **)&var_getnext, payload_size,
				SMM_VARIABLE_FUNCTION_GET_NEXT_VARIABLE_NAME,
				&ret);
	if (!comm_buf)
		goto out;

	/* Fill in contents */
	guidcpy(&var_getnext->guid, guid);
	var_getnext->name_size = out_name_size;
	memcpy(var_getnext->name, variable_name, in_name_size);
	memset((u8 *)var_getnext->name + in_name_size, 0x0,
	       out_name_size - in_name_size);

	/* Communicate */
	ret = mm_communicate(comm_buf, payload_size);
	if (ret == EFI_SUCCESS || ret == EFI_BUFFER_TOO_SMALL) {
		/* Update with reported data size for trimmed case */
		*variable_name_size = var_getnext->name_size;
	}
	if (ret != EFI_SUCCESS)
		goto out;

	guidcpy(guid, &var_getnext->guid);
	memcpy(variable_name, var_getnext->name, var_getnext->name_size);

out:
	free(comm_buf);
	return ret;
}

efi_status_t efi_set_variable_int(u16 *variable_name, const efi_guid_t *vendor,
				  u32 attributes, efi_uintn_t data_size,
				  const void *data, bool ro_check)
{
	efi_status_t ret, alt_ret = EFI_SUCCESS;
	struct var_check_property var_property;
	struct smm_variable_access *var_acc;
	efi_uintn_t payload_size;
	efi_uintn_t name_size;
	u8 *comm_buf = NULL;
	bool ro;

	if (!variable_name || variable_name[0] == 0 || !vendor) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	if (data_size > 0 && !data) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	/* Check payload size */
	name_size = u16_strsize(variable_name);
	payload_size = MM_VARIABLE_ACCESS_HEADER_SIZE + name_size + data_size;
	if (payload_size > max_payload_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/*
	 * Allocate the buffer early, before switching to RW (if needed)
	 * so we won't need to account for any failures in reading/setting
	 * the properties, if the allocation fails
	 */
	comm_buf = setup_mm_hdr((void **)&var_acc, payload_size,
				SMM_VARIABLE_FUNCTION_SET_VARIABLE, &ret);
	if (!comm_buf)
		goto out;

	ro = !!(attributes & EFI_VARIABLE_READ_ONLY);
	attributes &= EFI_VARIABLE_MASK;

	/*
	 * The API has the ability to override RO flags. If no RO check was
	 * requested switch the variable to RW for the duration of this call
	 */
	ret = get_property_int(variable_name, name_size, vendor,
			       &var_property);
	if (ret != EFI_SUCCESS)
		goto out;

	if (var_property.property & VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY) {
		/* Bypass r/o check */
		if (!ro_check) {
			var_property.property &= ~VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY;
			ret = set_property_int(variable_name, name_size, vendor, &var_property);
			if (ret != EFI_SUCCESS)
				goto out;
		} else {
			ret = EFI_WRITE_PROTECTED;
			goto out;
		}
	}

	/* Fill in contents */
	guidcpy(&var_acc->guid, vendor);
	var_acc->data_size = data_size;
	var_acc->name_size = name_size;
	var_acc->attr = attributes;
	memcpy(var_acc->name, variable_name, name_size);
	memcpy((u8 *)var_acc->name + name_size, data, data_size);

	/* Communicate */
	ret = mm_communicate(comm_buf, payload_size);
	if (ret != EFI_SUCCESS)
		alt_ret = ret;

	if (ro && !(var_property.property & VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY)) {
		var_property.revision = VAR_CHECK_VARIABLE_PROPERTY_REVISION;
		var_property.property |= VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY;
		var_property.attributes = attributes;
		var_property.minsize = 1;
		var_property.maxsize = var_acc->data_size;
		ret = set_property_int(variable_name, name_size, vendor, &var_property);
	}

	if (alt_ret != EFI_SUCCESS)
		goto out;

	if (!u16_strcmp(variable_name, L"PK"))
		alt_ret = efi_init_secure_state();
out:
	free(comm_buf);
	return alt_ret == EFI_SUCCESS ? ret : alt_ret;
}

efi_status_t efi_query_variable_info_int(u32 attributes,
					 u64 *max_variable_storage_size,
					 u64 *remain_variable_storage_size,
					 u64 *max_variable_size)
{
	struct smm_variable_query_info *mm_query_info;
	efi_uintn_t payload_size;
	efi_status_t ret;
	u8 *comm_buf;

	payload_size = sizeof(*mm_query_info);
	comm_buf = setup_mm_hdr((void **)&mm_query_info, payload_size,
				SMM_VARIABLE_FUNCTION_QUERY_VARIABLE_INFO,
				&ret);
	if (!comm_buf)
		goto out;

	mm_query_info->attr = attributes;
	ret = mm_communicate(comm_buf, payload_size);
	if (ret != EFI_SUCCESS)
		goto out;
	*max_variable_storage_size = mm_query_info->max_variable_storage;
	*remain_variable_storage_size =
			mm_query_info->remaining_variable_storage;
	*max_variable_size = mm_query_info->max_variable_size;

out:
	free(comm_buf);
	return ret;
}

/**
 * efi_query_variable_info() - get information about EFI variables
 *
 * This function implements the QueryVariableInfo() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @attributes:				bitmask to select variables to be
 *					queried
 * @maximum_variable_storage_size:	maximum size of storage area for the
 *					selected variable types
 * @remaining_variable_storage_size:	remaining size of storage are for the
 *					selected variable types
 * @maximum_variable_size:		maximum size of a variable of the
 *					selected type
 * Return:				status code
 */
efi_status_t EFIAPI __efi_runtime
efi_query_variable_info_runtime(u32 attributes, u64 *max_variable_storage_size,
				u64 *remain_variable_storage_size,
				u64 *max_variable_size)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_set_variable_runtime() - runtime implementation of SetVariable()
 *
 * @variable_name:	name of the variable
 * @guid:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer with the variable value
 * @data:		buffer with the variable value
 * Return:		status code
 */
static efi_status_t __efi_runtime EFIAPI
efi_set_variable_runtime(u16 *variable_name, const efi_guid_t *guid,
			 u32 attributes, efi_uintn_t data_size,
			 const void *data)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_variables_boot_exit_notify() - notify ExitBootServices() is called
 */
void efi_variables_boot_exit_notify(void)
{
	efi_status_t ret;
	u8 *comm_buf;
	loff_t len;
	struct efi_var_file *var_buf;

	comm_buf = setup_mm_hdr(NULL, 0,
				SMM_VARIABLE_FUNCTION_EXIT_BOOT_SERVICE, &ret);
	if (comm_buf)
		ret = mm_communicate(comm_buf, 0);
	else
		ret = EFI_NOT_FOUND;

	if (ret != EFI_SUCCESS)
		log_err("Unable to notify StMM for ExitBootServices\n");
	free(comm_buf);

	/*
	 * Populate the list for runtime variables.
	 * asking EFI_VARIABLE_RUNTIME_ACCESS is redundant, since
	 * efi_var_mem_notify_exit_boot_services will clean those, but that's fine
	 */
	ret = efi_var_collect(&var_buf, &len, EFI_VARIABLE_RUNTIME_ACCESS);
	if (ret != EFI_SUCCESS)
		log_err("Can't populate EFI variables. No runtime variables will be available\n");
	else
		memcpy(efi_var_buf, var_buf, len);
	free(var_buf);

	/* Update runtime service table */
	efi_runtime_services.query_variable_info =
			efi_query_variable_info_runtime;
	efi_runtime_services.get_variable = efi_get_variable_runtime;
	efi_runtime_services.get_next_variable_name =
			efi_get_next_variable_name_runtime;
	efi_runtime_services.set_variable = efi_set_variable_runtime;
	efi_update_table_header_crc32(&efi_runtime_services.hdr);
}

/**
 * efi_init_variables() - initialize variable services
 *
 * Return:	status code
 */
efi_status_t efi_init_variables(void)
{
	efi_status_t ret;

	/* Create a cached copy of the variables that will be enabled on ExitBootServices() */
	ret = efi_var_mem_init();
	if (ret != EFI_SUCCESS)
		return ret;

	ret = get_max_payload(&max_payload_size);
	if (ret != EFI_SUCCESS)
		return ret;

	max_buffer_size = MM_COMMUNICATE_HEADER_SIZE +
			  MM_VARIABLE_COMMUNICATE_SIZE +
			  max_payload_size;

	ret = efi_init_secure_state();
	if (ret != EFI_SUCCESS)
		return ret;

	return EFI_SUCCESS;
}
