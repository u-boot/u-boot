// SPDX-License-Identifier: GPL-2.0+
/*
 * UEFI runtime variable services
 *
 * Copyright (c) 2017 Rob Clark
 */

#include <common.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <env.h>
#include <env_internal.h>
#include <hexdump.h>
#include <malloc.h>
#include <rtc.h>
#include <search.h>
#include <uuid.h>
#include <crypto/pkcs7_parser.h>
#include <linux/compat.h>
#include <u-boot/crc.h>

enum efi_secure_mode {
	EFI_MODE_SETUP,
	EFI_MODE_USER,
	EFI_MODE_AUDIT,
	EFI_MODE_DEPLOYED,
};

static bool efi_secure_boot;
static enum efi_secure_mode efi_secure_mode;
static u8 efi_vendor_keys;

/**
 * efi_set_secure_state - modify secure boot state variables
 * @secure_boot:	value of SecureBoot
 * @setup_mode:		value of SetupMode
 * @audit_mode:		value of AuditMode
 * @deployed_mode:	value of DeployedMode
 *
 * Modify secure boot status related variables as indicated.
 *
 * Return:		status code
 */
static efi_status_t efi_set_secure_state(u8 secure_boot, u8 setup_mode,
					 u8 audit_mode, u8 deployed_mode)
{
	efi_status_t ret;
	const u32 attributes_ro = EFI_VARIABLE_BOOTSERVICE_ACCESS |
				  EFI_VARIABLE_RUNTIME_ACCESS |
				  EFI_VARIABLE_READ_ONLY;
	const u32 attributes_rw = EFI_VARIABLE_BOOTSERVICE_ACCESS |
				  EFI_VARIABLE_RUNTIME_ACCESS;

	efi_secure_boot = secure_boot;

	ret = efi_set_variable_int(L"SecureBoot", &efi_global_variable_guid,
				   attributes_ro, sizeof(secure_boot),
				   &secure_boot, false);
	if (ret != EFI_SUCCESS)
		goto err;

	ret = efi_set_variable_int(L"SetupMode", &efi_global_variable_guid,
				   attributes_ro, sizeof(setup_mode),
				   &setup_mode, false);
	if (ret != EFI_SUCCESS)
		goto err;

	ret = efi_set_variable_int(L"AuditMode", &efi_global_variable_guid,
				   audit_mode || setup_mode ?
				   attributes_ro : attributes_rw,
				   sizeof(audit_mode), &audit_mode, false);
	if (ret != EFI_SUCCESS)
		goto err;

	ret = efi_set_variable_int(L"DeployedMode",
				   &efi_global_variable_guid,
				   audit_mode || deployed_mode || setup_mode ?
				   attributes_ro : attributes_rw,
				   sizeof(deployed_mode), &deployed_mode,
				   false);
err:
	return ret;
}

/**
 * efi_transfer_secure_state - handle a secure boot state transition
 * @mode:	new state
 *
 * Depending on @mode, secure boot related variables are updated.
 * Those variables are *read-only* for users, efi_set_variable_int()
 * is called here.
 *
 * Return:	status code
 */
static efi_status_t efi_transfer_secure_state(enum efi_secure_mode mode)
{
	efi_status_t ret;

	EFI_PRINT("Switching secure state from %d to %d\n", efi_secure_mode,
		  mode);

	if (mode == EFI_MODE_DEPLOYED) {
		ret = efi_set_secure_state(1, 0, 0, 1);
		if (ret != EFI_SUCCESS)
			goto err;
	} else if (mode == EFI_MODE_AUDIT) {
		ret = efi_set_variable_int(L"PK", &efi_global_variable_guid,
					   EFI_VARIABLE_BOOTSERVICE_ACCESS |
					   EFI_VARIABLE_RUNTIME_ACCESS,
					   0, NULL, false);
		if (ret != EFI_SUCCESS)
			goto err;

		ret = efi_set_secure_state(0, 1, 1, 0);
		if (ret != EFI_SUCCESS)
			goto err;
	} else if (mode == EFI_MODE_USER) {
		ret = efi_set_secure_state(1, 0, 0, 0);
		if (ret != EFI_SUCCESS)
			goto err;
	} else if (mode == EFI_MODE_SETUP) {
		ret = efi_set_secure_state(0, 1, 0, 0);
		if (ret != EFI_SUCCESS)
			goto err;
	} else {
		return EFI_INVALID_PARAMETER;
	}

	efi_secure_mode = mode;

	return EFI_SUCCESS;

err:
	/* TODO: What action should be taken here? */
	printf("ERROR: Secure state transition failed\n");
	return ret;
}

/**
 * efi_init_secure_state - initialize secure boot state
 *
 * Return:	status code
 */
static efi_status_t efi_init_secure_state(void)
{
	enum efi_secure_mode mode = EFI_MODE_SETUP;
	efi_uintn_t size = 0;
	efi_status_t ret;

	ret = efi_get_variable_int(L"PK", &efi_global_variable_guid,
				   NULL, &size, NULL, NULL);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		if (IS_ENABLED(CONFIG_EFI_SECURE_BOOT))
			mode = EFI_MODE_USER;
	}

	ret = efi_transfer_secure_state(mode);
	if (ret != EFI_SUCCESS)
		return ret;

	/* As we do not provide vendor keys this variable is always 0. */
	ret = efi_set_variable_int(L"VendorKeys",
				   &efi_global_variable_guid,
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS |
				   EFI_VARIABLE_READ_ONLY,
				   sizeof(efi_vendor_keys),
				   &efi_vendor_keys, false);
	return ret;
}

/**
 * efi_secure_boot_enabled - return if secure boot is enabled or not
 *
 * Return:	true if enabled, false if disabled
 */
bool efi_secure_boot_enabled(void)
{
	return efi_secure_boot;
}

#ifdef CONFIG_EFI_SECURE_BOOT
static u8 pkcs7_hdr[] = {
	/* SEQUENCE */
	0x30, 0x82, 0x05, 0xc7,
	/* OID: pkcs7-signedData */
	0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02,
	/* Context Structured? */
	0xa0, 0x82, 0x05, 0xb8,
};

/**
 * efi_variable_parse_signature - parse a signature in variable
 * @buf:	Pointer to variable's value
 * @buflen:	Length of @buf
 *
 * Parse a signature embedded in variable's value and instantiate
 * a pkcs7_message structure. Since pkcs7_parse_message() accepts only
 * pkcs7's signedData, some header needed be prepended for correctly
 * parsing authentication data, particularly for variable's.
 *
 * Return:	Pointer to pkcs7_message structure on success, NULL on error
 */
static struct pkcs7_message *efi_variable_parse_signature(const void *buf,
							  size_t buflen)
{
	u8 *ebuf;
	size_t ebuflen, len;
	struct pkcs7_message *msg;

	/*
	 * This is the best assumption to check if the binary is
	 * already in a form of pkcs7's signedData.
	 */
	if (buflen > sizeof(pkcs7_hdr) &&
	    !memcmp(&((u8 *)buf)[4], &pkcs7_hdr[4], 11)) {
		msg = pkcs7_parse_message(buf, buflen);
		goto out;
	}

	/*
	 * Otherwise, we should add a dummy prefix sequence for pkcs7
	 * message parser to be able to process.
	 * NOTE: EDK2 also uses similar hack in WrapPkcs7Data()
	 * in CryptoPkg/Library/BaseCryptLib/Pk/CryptPkcs7VerifyCommon.c
	 * TODO:
	 * The header should be composed in a more refined manner.
	 */
	EFI_PRINT("Makeshift prefix added to authentication data\n");
	ebuflen = sizeof(pkcs7_hdr) + buflen;
	if (ebuflen <= 0x7f) {
		EFI_PRINT("Data is too short\n");
		return NULL;
	}

	ebuf = malloc(ebuflen);
	if (!ebuf) {
		EFI_PRINT("Out of memory\n");
		return NULL;
	}

	memcpy(ebuf, pkcs7_hdr, sizeof(pkcs7_hdr));
	memcpy(ebuf + sizeof(pkcs7_hdr), buf, buflen);
	len = ebuflen - 4;
	ebuf[2] = (len >> 8) & 0xff;
	ebuf[3] = len & 0xff;
	len = ebuflen - 0x13;
	ebuf[0x11] = (len >> 8) & 0xff;
	ebuf[0x12] = len & 0xff;

	msg = pkcs7_parse_message(ebuf, ebuflen);

	free(ebuf);

out:
	if (IS_ERR(msg))
		return NULL;

	return msg;
}

/**
 * efi_variable_authenticate - authenticate a variable
 * @variable:	Variable name in u16
 * @vendor:	Guid of variable
 * @data_size:	Size of @data
 * @data:	Pointer to variable's value
 * @given_attr:	Attributes to be given at SetVariable()
 * @env_attr:	Attributes that an existing variable holds
 * @time:	signed time that an existing variable holds
 *
 * Called by efi_set_variable() to verify that the input is correct.
 * Will replace the given data pointer with another that points to
 * the actual data to store in the internal memory.
 * On success, @data and @data_size will be replaced with variable's
 * actual data, excluding authentication data, and its size, and variable's
 * attributes and signed time will also be returned in @env_attr and @time,
 * respectively.
 *
 * Return:	status code
 */
static efi_status_t efi_variable_authenticate(u16 *variable,
					      const efi_guid_t *vendor,
					      efi_uintn_t *data_size,
					      const void **data, u32 given_attr,
					      u32 *env_attr, u64 *time)
{
	const struct efi_variable_authentication_2 *auth;
	struct efi_signature_store *truststore, *truststore2;
	struct pkcs7_message *var_sig;
	struct efi_image_regions *regs;
	struct efi_time timestamp;
	struct rtc_time tm;
	u64 new_time;
	efi_status_t ret;

	var_sig = NULL;
	truststore = NULL;
	truststore2 = NULL;
	regs = NULL;
	ret = EFI_SECURITY_VIOLATION;

	if (*data_size < sizeof(struct efi_variable_authentication_2))
		goto err;

	/* authentication data */
	auth = *data;
	if (*data_size < (sizeof(auth->time_stamp)
				+ auth->auth_info.hdr.dwLength))
		goto err;

	if (guidcmp(&auth->auth_info.cert_type, &efi_guid_cert_type_pkcs7))
		goto err;

	memcpy(&timestamp, &auth->time_stamp, sizeof(timestamp));
	if (timestamp.pad1 || timestamp.nanosecond || timestamp.timezone ||
	    timestamp.daylight || timestamp.pad2)
		goto err;

	*data += sizeof(auth->time_stamp) + auth->auth_info.hdr.dwLength;
	*data_size -= (sizeof(auth->time_stamp)
				+ auth->auth_info.hdr.dwLength);

	memset(&tm, 0, sizeof(tm));
	tm.tm_year = timestamp.year;
	tm.tm_mon = timestamp.month;
	tm.tm_mday = timestamp.day;
	tm.tm_hour = timestamp.hour;
	tm.tm_min = timestamp.minute;
	tm.tm_sec = timestamp.second;
	new_time = rtc_mktime(&tm);

	if (!efi_secure_boot_enabled()) {
		/* finished checking */
		*time = new_time;
		return EFI_SUCCESS;
	}

	if (new_time <= *time)
		goto err;

	/* data to be digested */
	regs = calloc(sizeof(*regs) + sizeof(struct image_region) * 5, 1);
	if (!regs)
		goto err;
	regs->max = 5;
	efi_image_region_add(regs, (uint8_t *)variable,
			     (uint8_t *)variable
				+ u16_strlen(variable) * sizeof(u16), 1);
	efi_image_region_add(regs, (uint8_t *)vendor,
			     (uint8_t *)vendor + sizeof(*vendor), 1);
	efi_image_region_add(regs, (uint8_t *)&given_attr,
			     (uint8_t *)&given_attr + sizeof(given_attr), 1);
	efi_image_region_add(regs, (uint8_t *)&timestamp,
			     (uint8_t *)&timestamp + sizeof(timestamp), 1);
	efi_image_region_add(regs, (uint8_t *)*data,
			     (uint8_t *)*data + *data_size, 1);

	/* variable's signature list */
	if (auth->auth_info.hdr.dwLength < sizeof(auth->auth_info))
		goto err;
	var_sig = efi_variable_parse_signature(auth->auth_info.cert_data,
					       auth->auth_info.hdr.dwLength
						   - sizeof(auth->auth_info));
	if (!var_sig) {
		EFI_PRINT("Parsing variable's signature failed\n");
		goto err;
	}

	/* signature database used for authentication */
	if (u16_strcmp(variable, L"PK") == 0 ||
	    u16_strcmp(variable, L"KEK") == 0) {
		/* with PK */
		truststore = efi_sigstore_parse_sigdb(L"PK");
		if (!truststore)
			goto err;
	} else if (u16_strcmp(variable, L"db") == 0 ||
		   u16_strcmp(variable, L"dbx") == 0) {
		/* with PK and KEK */
		truststore = efi_sigstore_parse_sigdb(L"KEK");
		truststore2 = efi_sigstore_parse_sigdb(L"PK");

		if (!truststore) {
			if (!truststore2)
				goto err;

			truststore = truststore2;
			truststore2 = NULL;
		}
	} else {
		/* TODO: support private authenticated variables */
		goto err;
	}

	/* verify signature */
	if (efi_signature_verify_with_sigdb(regs, var_sig, truststore, NULL)) {
		EFI_PRINT("Verified\n");
	} else {
		if (truststore2 &&
		    efi_signature_verify_with_sigdb(regs, var_sig,
						    truststore2, NULL)) {
			EFI_PRINT("Verified\n");
		} else {
			EFI_PRINT("Verifying variable's signature failed\n");
			goto err;
		}
	}

	/* finished checking */
	*time = new_time;
	ret = EFI_SUCCESS;

err:
	efi_sigstore_free(truststore);
	efi_sigstore_free(truststore2);
	pkcs7_free_message(var_sig);
	free(regs);

	return ret;
}
#else
static efi_status_t efi_variable_authenticate(u16 *variable,
					      const efi_guid_t *vendor,
					      efi_uintn_t *data_size,
					      const void **data, u32 given_attr,
					      u32 *env_attr, u64 *time)
{
	return EFI_SUCCESS;
}
#endif /* CONFIG_EFI_SECURE_BOOT */

efi_status_t __efi_runtime
efi_get_variable_int(u16 *variable_name, const efi_guid_t *vendor,
		     u32 *attributes, efi_uintn_t *data_size, void *data,
		     u64 *timep)
{
	efi_uintn_t old_size;
	struct efi_var_entry *var;
	u16 *pdata;

	if (!variable_name || !vendor || !data_size)
		return EFI_INVALID_PARAMETER;
	var = efi_var_mem_find(vendor, variable_name, NULL);
	if (!var)
		return EFI_NOT_FOUND;

	if (attributes)
		*attributes = var->attr;
	if (timep)
		*timep = var->time;

	old_size = *data_size;
	*data_size = var->length;
	if (old_size < var->length)
		return EFI_BUFFER_TOO_SMALL;

	if (!data)
		return EFI_INVALID_PARAMETER;

	for (pdata = var->name; *pdata; ++pdata)
		;
	++pdata;

	efi_memcpy_runtime(data, pdata, var->length);

	return EFI_SUCCESS;
}

efi_status_t __efi_runtime
efi_get_next_variable_name_int(efi_uintn_t *variable_name_size,
			       u16 *variable_name, efi_guid_t *vendor)
{
	struct efi_var_entry *var;
	efi_uintn_t old_size;
	u16 *pdata;

	if (!variable_name_size || !variable_name || !vendor)
		return EFI_INVALID_PARAMETER;

	efi_var_mem_find(vendor, variable_name, &var);

	if (!var)
		return EFI_NOT_FOUND;

	for (pdata = var->name; *pdata; ++pdata)
		;
	++pdata;

	old_size = *variable_name_size;
	*variable_name_size = (uintptr_t)pdata - (uintptr_t)var->name;

	if (old_size < *variable_name_size)
		return EFI_BUFFER_TOO_SMALL;

	efi_memcpy_runtime(variable_name, var->name, *variable_name_size);
	efi_memcpy_runtime(vendor, &var->guid, sizeof(efi_guid_t));

	return EFI_SUCCESS;
}

efi_status_t efi_set_variable_int(u16 *variable_name, const efi_guid_t *vendor,
				  u32 attributes, efi_uintn_t data_size,
				  const void *data, bool ro_check)
{
	struct efi_var_entry *var;
	efi_uintn_t ret;
	bool append, delete;
	u64 time = 0;

	if (!variable_name || !*variable_name || !vendor ||
	    ((attributes & EFI_VARIABLE_RUNTIME_ACCESS) &&
	     !(attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS)))
		return EFI_INVALID_PARAMETER;

	/* check if a variable exists */
	var = efi_var_mem_find(vendor, variable_name, NULL);
	append = !!(attributes & EFI_VARIABLE_APPEND_WRITE);
	attributes &= ~(u32)EFI_VARIABLE_APPEND_WRITE;
	delete = !append && (!data_size || !attributes);

	/* check attributes */
	if (var) {
		if (ro_check && (var->attr & EFI_VARIABLE_READ_ONLY))
			return EFI_WRITE_PROTECTED;

		/* attributes won't be changed */
		if (!delete &&
		    ((ro_check && var->attr != attributes) ||
		     (!ro_check && ((var->attr & ~(u32)EFI_VARIABLE_READ_ONLY)
				    != (attributes & ~(u32)EFI_VARIABLE_READ_ONLY))))) {
			return EFI_INVALID_PARAMETER;
		}
		time = var->time;
	} else {
		if (delete || append)
			/*
			 * Trying to delete or to update a non-existent
			 * variable.
			 */
			return EFI_NOT_FOUND;
	}

	if (((!u16_strcmp(variable_name, L"PK") ||
	      !u16_strcmp(variable_name, L"KEK")) &&
		!guidcmp(vendor, &efi_global_variable_guid)) ||
	    ((!u16_strcmp(variable_name, L"db") ||
	      !u16_strcmp(variable_name, L"dbx")) &&
		!guidcmp(vendor, &efi_guid_image_security_database))) {
		/* authentication is mandatory */
		if (!(attributes &
		      EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)) {
			EFI_PRINT("%ls: TIME_BASED_AUTHENTICATED_WRITE_ACCESS required\n",
				  variable_name);
			return EFI_INVALID_PARAMETER;
		}
	}

	/* authenticate a variable */
	if (IS_ENABLED(CONFIG_EFI_SECURE_BOOT)) {
		if (attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS)
			return EFI_INVALID_PARAMETER;
		if (attributes &
		    EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) {
			u32 env_attr;

			ret = efi_variable_authenticate(variable_name, vendor,
							&data_size, &data,
							attributes, &env_attr,
							&time);
			if (ret != EFI_SUCCESS)
				return ret;

			/* last chance to check for delete */
			if (!data_size)
				delete = true;
		}
	} else {
		if (attributes &
		    (EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS |
		     EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)) {
			EFI_PRINT("Secure boot is not configured\n");
			return EFI_INVALID_PARAMETER;
		}
	}

	if (delete) {
		/* EFI_NOT_FOUND has been handled before */
		ret = EFI_SUCCESS;
	} else if (append) {
		u16 *old_data = var->name;

		for (; *old_data; ++old_data)
			;
		++old_data;
		ret = efi_var_mem_ins(variable_name, vendor, attributes,
				      var->length, old_data, data_size, data,
				      time);
	} else {
		ret = efi_var_mem_ins(variable_name, vendor, attributes,
				      data_size, data, 0, NULL, time);
	}
	efi_var_mem_del(var);

	if (ret != EFI_SUCCESS)
		return ret;

	if (!u16_strcmp(variable_name, L"PK"))
		ret = efi_init_secure_state();
	else
		ret = EFI_SUCCESS;

	/* Write non-volatile EFI variables to file */
	if (attributes & EFI_VARIABLE_NON_VOLATILE &&
	    ret == EFI_SUCCESS && efi_obj_list_initialized == EFI_SUCCESS)
		efi_var_to_file();

	return EFI_SUCCESS;
}

efi_status_t efi_query_variable_info_int(u32 attributes,
					 u64 *maximum_variable_storage_size,
					 u64 *remaining_variable_storage_size,
					 u64 *maximum_variable_size)
{
	*maximum_variable_storage_size = EFI_VAR_BUF_SIZE -
					 sizeof(struct efi_var_file);
	*remaining_variable_storage_size = efi_var_mem_free();
	*maximum_variable_size = EFI_VAR_BUF_SIZE -
				 sizeof(struct efi_var_file) -
				 sizeof(struct efi_var_entry);
	return EFI_SUCCESS;
}

/**
 * efi_query_variable_info_runtime() - runtime implementation of
 *				       QueryVariableInfo()
 *
 * @attributes:				bitmask to select variables to be
 *					queried
 * @maximum_variable_storage_size:	maximum size of storage area for the
 *					selected variable types
 * @remaining_variable_storage_size:	remaining size of storage are for the
 *					selected variable types
 * @maximum_variable_size:		maximum size of a variable of the
 *					selected type
 * Returns:				status code
 */
efi_status_t __efi_runtime EFIAPI efi_query_variable_info_runtime(
			u32 attributes,
			u64 *maximum_variable_storage_size,
			u64 *remaining_variable_storage_size,
			u64 *maximum_variable_size)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_get_variable_runtime() - runtime implementation of GetVariable()
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer to which the variable value is copied
 * @data:		buffer to which the variable value is copied
 * Return:		status code
 */
static efi_status_t __efi_runtime EFIAPI
efi_get_variable_runtime(u16 *variable_name, const efi_guid_t *vendor,
			 u32 *attributes, efi_uintn_t *data_size, void *data)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_get_next_variable_name_runtime() - runtime implementation of
 *					  GetNextVariable()
 *
 * @variable_name_size:	size of variable_name buffer in byte
 * @variable_name:	name of uefi variable's name in u16
 * @vendor:		vendor's guid
 * Return: status code
 */
static efi_status_t __efi_runtime EFIAPI
efi_get_next_variable_name_runtime(efi_uintn_t *variable_name_size,
				   u16 *variable_name, efi_guid_t *vendor)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_set_variable_runtime() - runtime implementation of SetVariable()
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer with the variable value
 * @data:		buffer with the variable value
 * Return:		status code
 */
static efi_status_t __efi_runtime EFIAPI
efi_set_variable_runtime(u16 *variable_name, const efi_guid_t *vendor,
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
	/* Switch variable services functions to runtime version */
	efi_runtime_services.get_variable = efi_get_variable_runtime;
	efi_runtime_services.get_next_variable_name =
				efi_get_next_variable_name_runtime;
	efi_runtime_services.set_variable = efi_set_variable_runtime;
	efi_runtime_services.query_variable_info =
				efi_query_variable_info_runtime;
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

	ret = efi_var_mem_init();
	if (ret != EFI_SUCCESS)
		return ret;

	ret = efi_init_secure_state();
	if (ret != EFI_SUCCESS)
		return ret;

	return efi_var_from_file();
}
