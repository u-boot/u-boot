/*
 * UEFI runtime variable services
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 * Copyright (c) 2020 Linaro Limited, Author: AKASHI Takahiro
 */

#define LOG_CATEGORY LOGC_EFI

#include <efi_loader.h>
#include <efi_variable.h>
#include <stdlib.h>
#include <u-boot/crc.h>

enum efi_secure_mode {
	EFI_MODE_SETUP,
	EFI_MODE_USER,
	EFI_MODE_AUDIT,
	EFI_MODE_DEPLOYED,
};

struct efi_auth_var_name_type {
	const u16 *name;
	const efi_guid_t *guid;
	const enum efi_auth_var_type type;
};

const efi_guid_t efi_guid_image_security_database =
		EFI_IMAGE_SECURITY_DATABASE_GUID;

static const struct efi_auth_var_name_type name_type[] = {
	{u"PK", &efi_global_variable_guid, EFI_AUTH_VAR_PK},
	{u"KEK", &efi_global_variable_guid, EFI_AUTH_VAR_KEK},
	{u"db",  &efi_guid_image_security_database, EFI_AUTH_VAR_DB},
	{u"dbx",  &efi_guid_image_security_database, EFI_AUTH_VAR_DBX},
	{u"dbt",  &efi_guid_image_security_database, EFI_AUTH_VAR_DBT},
	{u"dbr",  &efi_guid_image_security_database, EFI_AUTH_VAR_DBR},
	{u"AuditMode", &efi_global_variable_guid, EFI_AUTH_MODE},
	{u"DeployedMode", &efi_global_variable_guid, EFI_AUTH_MODE},
};

static bool efi_secure_boot;
static enum efi_secure_mode efi_secure_mode;

/**
 * efi_efi_get_variable() - retrieve value of a UEFI variable
 *
 * This function implements the GetVariable runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer to which the variable value is copied
 * @data:		buffer to which the variable value is copied
 * Return:		status code
 */
efi_status_t EFIAPI efi_get_variable(u16 *variable_name,
				     const efi_guid_t *vendor, u32 *attributes,
				     efi_uintn_t *data_size, void *data)
{
	efi_status_t ret;

	EFI_ENTRY("\"%ls\" %pUs %p %p %p", variable_name, vendor, attributes,
		  data_size, data);

	ret = efi_get_variable_int(variable_name, vendor, attributes,
				   data_size, data, NULL);

	/* Remove EFI_VARIABLE_READ_ONLY flag */
	if (attributes)
		*attributes &= EFI_VARIABLE_MASK;

	return EFI_EXIT(ret);
}

/**
 * efi_set_variable() - set value of a UEFI variable
 *
 * This function implements the SetVariable runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer with the variable value
 * @data:		buffer with the variable value
 * Return:		status code
 */
efi_status_t EFIAPI efi_set_variable(u16 *variable_name,
				     const efi_guid_t *vendor, u32 attributes,
				     efi_uintn_t data_size, const void *data)
{
	efi_status_t ret;

	EFI_ENTRY("\"%ls\" %pUs %x %zu %p", variable_name, vendor, attributes,
		  data_size, data);

	/* Make sure that the EFI_VARIABLE_READ_ONLY flag is not set */
	if (attributes & ~EFI_VARIABLE_MASK)
		ret = EFI_INVALID_PARAMETER;
	else
		ret = efi_set_variable_int(variable_name, vendor, attributes,
					   data_size, data, true);

	return EFI_EXIT(ret);
}

/**
 * efi_get_next_variable_name() - enumerate the current variable names
 *
 * @variable_name_size:	size of variable_name buffer in byte
 * @variable_name:	name of uefi variable's name in u16
 * @vendor:		vendor's guid
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_get_next_variable_name(efi_uintn_t *variable_name_size,
					       u16 *variable_name,
					       efi_guid_t *vendor)
{
	efi_status_t ret;

	EFI_ENTRY("%p \"%ls\" %pUs", variable_name_size, variable_name, vendor);

	ret = efi_get_next_variable_name_int(variable_name_size, variable_name,
					     vendor);

	return EFI_EXIT(ret);
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
 * Returns:				status code
 */
efi_status_t EFIAPI efi_query_variable_info(
			u32 attributes, u64 *maximum_variable_storage_size,
			u64 *remaining_variable_storage_size,
			u64 *maximum_variable_size)
{
	efi_status_t ret;

	EFI_ENTRY("%x %p %p %p", attributes, maximum_variable_storage_size,
		  remaining_variable_storage_size, maximum_variable_size);

	ret = efi_query_variable_info_int(attributes,
					  maximum_variable_storage_size,
					  remaining_variable_storage_size,
					  maximum_variable_size);

	return EFI_EXIT(ret);
}

efi_status_t __efi_runtime EFIAPI
efi_get_variable_runtime(u16 *variable_name, const efi_guid_t *guid,
			 u32 *attributes, efi_uintn_t *data_size, void *data)
{
	efi_status_t ret;

	ret = efi_get_variable_mem(variable_name, guid, attributes, data_size,
				   data, NULL, EFI_VARIABLE_RUNTIME_ACCESS);

	/* Remove EFI_VARIABLE_READ_ONLY flag */
	if (attributes)
		*attributes &= EFI_VARIABLE_MASK;

	return ret;
}

efi_status_t __efi_runtime EFIAPI
efi_get_next_variable_name_runtime(efi_uintn_t *variable_name_size,
				   u16 *variable_name, efi_guid_t *guid)
{
	return efi_get_next_variable_name_mem(variable_name_size, variable_name,
					      guid, EFI_VARIABLE_RUNTIME_ACCESS);
}

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

	ret = efi_set_variable_int(u"SecureBoot", &efi_global_variable_guid,
				   attributes_ro, sizeof(secure_boot),
				   &secure_boot, false);
	if (ret != EFI_SUCCESS)
		goto err;

	ret = efi_set_variable_int(u"SetupMode", &efi_global_variable_guid,
				   attributes_ro, sizeof(setup_mode),
				   &setup_mode, false);
	if (ret != EFI_SUCCESS)
		goto err;

	ret = efi_set_variable_int(u"AuditMode", &efi_global_variable_guid,
				   audit_mode || setup_mode ?
				   attributes_ro : attributes_rw,
				   sizeof(audit_mode), &audit_mode, false);
	if (ret != EFI_SUCCESS)
		goto err;

	ret = efi_set_variable_int(u"DeployedMode",
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
		ret = efi_set_variable_int(u"PK", &efi_global_variable_guid,
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

efi_status_t efi_init_secure_state(void)
{
	enum efi_secure_mode mode;
	u8 efi_vendor_keys = 0;
	efi_uintn_t size;
	efi_status_t ret;
	u8 deployed_mode = 0;
	u8 audit_mode = 0;
	u8 setup_mode = 1;

	if (IS_ENABLED(CONFIG_EFI_SECURE_BOOT)) {
		size = sizeof(deployed_mode);
		ret = efi_get_variable_int(u"DeployedMode", &efi_global_variable_guid,
					   NULL, &size, &deployed_mode, NULL);
		size = sizeof(audit_mode);
		ret = efi_get_variable_int(u"AuditMode", &efi_global_variable_guid,
					   NULL, &size, &audit_mode, NULL);
		size = 0;
		ret = efi_get_variable_int(u"PK", &efi_global_variable_guid,
					   NULL, &size, NULL, NULL);
		if (ret == EFI_BUFFER_TOO_SMALL) {
			setup_mode = 0;
			audit_mode = 0;
		} else {
			setup_mode = 1;
			deployed_mode = 0;
		}
	}
	if (deployed_mode)
		mode = EFI_MODE_DEPLOYED;
	else if (audit_mode)
		mode = EFI_MODE_AUDIT;
	else if (setup_mode)
		mode = EFI_MODE_SETUP;
	else
		mode = EFI_MODE_USER;

	ret = efi_transfer_secure_state(mode);
	if (ret != EFI_SUCCESS)
		return ret;

	/* As we do not provide vendor keys this variable is always 0. */
	ret = efi_set_variable_int(u"VendorKeys",
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

enum efi_auth_var_type efi_auth_var_get_type(const u16 *name,
					     const efi_guid_t *guid)
{
	for (size_t i = 0; i < ARRAY_SIZE(name_type); ++i) {
		if (!u16_strcmp(name, name_type[i].name) &&
		    !guidcmp(guid, name_type[i].guid))
			return name_type[i].type;
	}
	return EFI_AUTH_VAR_NONE;
}

const efi_guid_t *efi_auth_var_get_guid(const u16 *name)
{
	for (size_t i = 0; i < ARRAY_SIZE(name_type); ++i) {
		if (!u16_strcmp(name, name_type[i].name))
			return name_type[i].guid;
	}
	return &efi_global_variable_guid;
}

/**
 * efi_get_var() - read value of an EFI variable
 *
 * @name:	variable name
 * @start:	vendor GUID
 * @size:	size of allocated buffer
 *
 * Return:	buffer with variable data or NULL
 */
void *efi_get_var(const u16 *name, const efi_guid_t *vendor, efi_uintn_t *size)
{
	efi_status_t ret;
	void *buf = NULL;

	*size = 0;
	ret = efi_get_variable_int(name, vendor, NULL, size, buf, NULL);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		buf = malloc(*size);
		if (!buf)
			return NULL;
		ret = efi_get_variable_int(name, vendor, NULL, size, buf, NULL);
	}

	if (ret != EFI_SUCCESS) {
		free(buf);
		*size = 0;
		return NULL;
	}

	return buf;
}

/**
 * efi_var_collect() - Copy EFI variables matching attributes mask
 *
 * @bufp:	buffer containing variable collection
 * @lenp:	buffer length
 * @attr_mask:	mask of matched attributes
 *
 * Return:	Status code
 */
efi_status_t __maybe_unused efi_var_collect(struct efi_var_file **bufp, loff_t *lenp,
					    u32 check_attr_mask)
{
	size_t len = EFI_VAR_BUF_SIZE;
	struct efi_var_file *buf;
	struct efi_var_entry *var, *old_var;
	size_t old_var_name_length = 2;

	*bufp = NULL; /* Avoid double free() */
	buf = calloc(1, len);
	if (!buf)
		return EFI_OUT_OF_RESOURCES;
	var = buf->var;
	old_var = var;
	for (;;) {
		efi_uintn_t data_length, var_name_length;
		u8 *data;
		efi_status_t ret;

		if ((uintptr_t)buf + len <=
		    (uintptr_t)var->name + old_var_name_length)
			return EFI_BUFFER_TOO_SMALL;

		var_name_length = (uintptr_t)buf + len - (uintptr_t)var->name;
		memcpy(var->name, old_var->name, old_var_name_length);
		guidcpy(&var->guid, &old_var->guid);
		ret = efi_get_next_variable_name_int(
				&var_name_length, var->name, &var->guid);
		if (ret == EFI_NOT_FOUND)
			break;
		if (ret != EFI_SUCCESS) {
			free(buf);
			return ret;
		}
		old_var_name_length = var_name_length;
		old_var = var;

		data = (u8 *)var->name + old_var_name_length;
		data_length = (uintptr_t)buf + len - (uintptr_t)data;
		ret = efi_get_variable_int(var->name, &var->guid,
					   &var->attr, &data_length, data,
					   &var->time);
		if (ret != EFI_SUCCESS) {
			free(buf);
			return ret;
		}
		if ((var->attr & check_attr_mask) == check_attr_mask) {
			var->length = data_length;
			var = (struct efi_var_entry *)ALIGN((uintptr_t)data + data_length, 8);
		}
	}

	buf->reserved = 0;
	buf->magic = EFI_VAR_FILE_MAGIC;
	len = (uintptr_t)var - (uintptr_t)buf;
	buf->crc32 = crc32(0, (u8 *)buf->var,
			   len - sizeof(struct efi_var_file));
	buf->length = len;
	*bufp = buf;
	*lenp = len;

	return EFI_SUCCESS;
}
