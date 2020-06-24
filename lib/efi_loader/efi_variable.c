// SPDX-License-Identifier: GPL-2.0+
/*
 * UEFI runtime variable services
 *
 * Copyright (c) 2017 Rob Clark
 */

#include <common.h>
#include <efi_loader.h>
#include <env.h>
#include <env_internal.h>
#include <hexdump.h>
#include <malloc.h>
#include <rtc.h>
#include <search.h>
#include <uuid.h>
#include <crypto/pkcs7_parser.h>
#include <linux/bitops.h>
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

#define READ_ONLY BIT(31)

static efi_status_t efi_get_variable_common(u16 *variable_name,
					    const efi_guid_t *vendor,
					    u32 *attributes,
					    efi_uintn_t *data_size, void *data);

static efi_status_t efi_set_variable_common(u16 *variable_name,
					    const efi_guid_t *vendor,
					    u32 attributes,
					    efi_uintn_t data_size,
					    const void *data,
					    bool ro_check);

/*
 * Mapping between EFI variables and u-boot variables:
 *
 *   efi_$guid_$varname = {attributes}(type)value
 *
 * For example:
 *
 *   efi_8be4df61-93ca-11d2-aa0d-00e098032b8c_OsIndicationsSupported=
 *      "{ro,boot,run}(blob)0000000000000000"
 *   efi_8be4df61-93ca-11d2-aa0d-00e098032b8c_BootOrder=
 *      "(blob)00010000"
 *
 * The attributes are a comma separated list of these possible
 * attributes:
 *
 *   + ro   - read-only
 *   + boot - boot-services access
 *   + run  - runtime access
 *
 * NOTE: with current implementation, no variables are available after
 * ExitBootServices, and all are persisted (if possible).
 *
 * If not specified, the attributes default to "{boot}".
 *
 * The required type is one of:
 *
 *   + utf8 - raw utf8 string
 *   + blob - arbitrary length hex string
 *
 * Maybe a utf16 type would be useful to for a string value to be auto
 * converted to utf16?
 */

#define PREFIX_LEN (strlen("efi_xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx_"))

/**
 * efi_to_native() - convert the UEFI variable name and vendor GUID to U-Boot
 *		     variable name
 *
 * The U-Boot variable name is a concatenation of prefix 'efi', the hexstring
 * encoded vendor GUID, and the UTF-8 encoded UEFI variable name separated by
 * underscores, e.g. 'efi_8be4df61-93ca-11d2-aa0d-00e098032b8c_BootOrder'.
 *
 * @native:		pointer to pointer to U-Boot variable name
 * @variable_name:	UEFI variable name
 * @vendor:		vendor GUID
 * Return:		status code
 */
static efi_status_t efi_to_native(char **native, const u16 *variable_name,
				  const efi_guid_t *vendor)
{
	size_t len;
	char *pos;

	len = PREFIX_LEN + utf16_utf8_strlen(variable_name) + 1;
	*native = malloc(len);
	if (!*native)
		return EFI_OUT_OF_RESOURCES;

	pos = *native;
	pos += sprintf(pos, "efi_%pUl_", vendor);
	utf16_utf8_strcpy(&pos, variable_name);

	return EFI_SUCCESS;
}

/**
 * prefix() - skip over prefix
 *
 * Skip over a prefix string.
 *
 * @str:	string with prefix
 * @prefix:	prefix string
 * Return:	string without prefix, or NULL if prefix not found
 */
static const char *prefix(const char *str, const char *prefix)
{
	size_t n = strlen(prefix);
	if (!strncmp(prefix, str, n))
		return str + n;
	return NULL;
}

/**
 * parse_attr() - decode attributes part of variable value
 *
 * Convert the string encoded attributes of a UEFI variable to a bit mask.
 * TODO: Several attributes are not supported.
 *
 * @str:	value of U-Boot variable
 * @attrp:	pointer to UEFI attributes
 * @timep:	pointer to time attribute
 * Return:	pointer to remainder of U-Boot variable value
 */
static const char *parse_attr(const char *str, u32 *attrp, u64 *timep)
{
	u32 attr = 0;
	char sep = '{';

	if (*str != '{') {
		*attrp = EFI_VARIABLE_BOOTSERVICE_ACCESS;
		return str;
	}

	while (*str == sep) {
		const char *s;

		str++;

		if ((s = prefix(str, "ro"))) {
			attr |= READ_ONLY;
		} else if ((s = prefix(str, "nv"))) {
			attr |= EFI_VARIABLE_NON_VOLATILE;
		} else if ((s = prefix(str, "boot"))) {
			attr |= EFI_VARIABLE_BOOTSERVICE_ACCESS;
		} else if ((s = prefix(str, "run"))) {
			attr |= EFI_VARIABLE_RUNTIME_ACCESS;
		} else if ((s = prefix(str, "time="))) {
			attr |= EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
			hex2bin((u8 *)timep, s, sizeof(*timep));
			s += sizeof(*timep) * 2;
		} else if (*str == '}') {
			break;
		} else {
			printf("invalid attribute: %s\n", str);
			break;
		}

		str = s;
		sep = ',';
	}

	str++;

	*attrp = attr;

	return str;
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
	u32 attributes;
	efi_status_t ret;

	attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS |
		     EFI_VARIABLE_RUNTIME_ACCESS |
		     READ_ONLY;
	ret = efi_set_variable_common(L"SecureBoot", &efi_global_variable_guid,
				      attributes, sizeof(secure_boot),
				      &secure_boot, false);
	if (ret != EFI_SUCCESS)
		goto err;

	ret = efi_set_variable_common(L"SetupMode", &efi_global_variable_guid,
				      attributes, sizeof(setup_mode),
				      &setup_mode, false);
	if (ret != EFI_SUCCESS)
		goto err;

	ret = efi_set_variable_common(L"AuditMode", &efi_global_variable_guid,
				      attributes, sizeof(audit_mode),
				      &audit_mode, false);
	if (ret != EFI_SUCCESS)
		goto err;

	ret = efi_set_variable_common(L"DeployedMode",
				      &efi_global_variable_guid, attributes,
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
 * Those variables are *read-only* for users, efi_set_variable_common()
 * is called here.
 *
 * Return:	status code
 */
static efi_status_t efi_transfer_secure_state(enum efi_secure_mode mode)
{
	efi_status_t ret;

	debug("Switching secure state from %d to %d\n", efi_secure_mode, mode);

	if (mode == EFI_MODE_DEPLOYED) {
		ret = efi_set_secure_state(1, 0, 0, 1);
		if (ret != EFI_SUCCESS)
			goto err;

		efi_secure_boot = true;
	} else if (mode == EFI_MODE_AUDIT) {
		ret = efi_set_variable_common(L"PK", &efi_global_variable_guid,
					      EFI_VARIABLE_BOOTSERVICE_ACCESS |
					      EFI_VARIABLE_RUNTIME_ACCESS,
					      0, NULL, false);
		if (ret != EFI_SUCCESS)
			goto err;

		ret = efi_set_secure_state(0, 1, 1, 0);
		if (ret != EFI_SUCCESS)
			goto err;

		efi_secure_boot = true;
	} else if (mode == EFI_MODE_USER) {
		ret = efi_set_secure_state(1, 0, 0, 0);
		if (ret != EFI_SUCCESS)
			goto err;

		efi_secure_boot = true;
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
	enum efi_secure_mode mode;
	efi_uintn_t size;
	efi_status_t ret;

	/*
	 * TODO:
	 * Since there is currently no "platform-specific" installation
	 * method of Platform Key, we can't say if VendorKeys is 0 or 1
	 * precisely.
	 */

	size = 0;
	ret = efi_get_variable_common(L"PK", &efi_global_variable_guid,
				      NULL, &size, NULL);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		if (IS_ENABLED(CONFIG_EFI_SECURE_BOOT))
			mode = EFI_MODE_USER;
		else
			mode = EFI_MODE_SETUP;

		efi_vendor_keys = 0;
	} else if (ret == EFI_NOT_FOUND) {
		mode = EFI_MODE_SETUP;
		efi_vendor_keys = 1;
	} else {
		goto err;
	}

	ret = efi_transfer_secure_state(mode);
	if (ret == EFI_SUCCESS)
		ret = efi_set_variable_common(L"VendorKeys",
					      &efi_global_variable_guid,
					      EFI_VARIABLE_BOOTSERVICE_ACCESS |
					      EFI_VARIABLE_RUNTIME_ACCESS |
					      READ_ONLY,
					      sizeof(efi_vendor_keys),
					      &efi_vendor_keys, false);

err:
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
	debug("Makeshift prefix added to authentication data\n");
	ebuflen = sizeof(pkcs7_hdr) + buflen;
	if (ebuflen <= 0x7f) {
		debug("Data is too short\n");
		return NULL;
	}

	ebuf = malloc(ebuflen);
	if (!ebuf) {
		debug("Out of memory\n");
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

	*data += sizeof(auth->time_stamp) + auth->auth_info.hdr.dwLength;
	*data_size -= (sizeof(auth->time_stamp)
				+ auth->auth_info.hdr.dwLength);

	memcpy(&timestamp, &auth->time_stamp, sizeof(timestamp));
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
		debug("Parsing variable's signature failed\n");
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
		debug("Verified\n");
	} else {
		if (truststore2 &&
		    efi_signature_verify_with_sigdb(regs, var_sig,
						    truststore2, NULL)) {
			debug("Verified\n");
		} else {
			debug("Verifying variable's signature failed\n");
			goto err;
		}
	}

	/* finished checking */
	*time = rtc_mktime(&tm);
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

static efi_status_t efi_get_variable_common(u16 *variable_name,
					    const efi_guid_t *vendor,
					    u32 *attributes,
					    efi_uintn_t *data_size, void *data)
{
	char *native_name;
	efi_status_t ret;
	unsigned long in_size;
	const char *val = NULL, *s;
	u64 time = 0;
	u32 attr;

	if (!variable_name || !vendor || !data_size)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	ret = efi_to_native(&native_name, variable_name, vendor);
	if (ret)
		return ret;

	EFI_PRINT("get '%s'\n", native_name);

	val = env_get(native_name);
	free(native_name);
	if (!val)
		return EFI_NOT_FOUND;

	val = parse_attr(val, &attr, &time);

	in_size = *data_size;

	if ((s = prefix(val, "(blob)"))) {
		size_t len = strlen(s);

		/* number of hexadecimal digits must be even */
		if (len & 1)
			return EFI_DEVICE_ERROR;

		/* two characters per byte: */
		len /= 2;
		*data_size = len;

		if (in_size < len) {
			ret = EFI_BUFFER_TOO_SMALL;
			goto out;
		}

		if (!data) {
			debug("Variable with no data shouldn't exist.\n");
			return EFI_INVALID_PARAMETER;
		}

		if (hex2bin(data, s, len))
			return EFI_DEVICE_ERROR;

		EFI_PRINT("got value: \"%s\"\n", s);
	} else if ((s = prefix(val, "(utf8)"))) {
		unsigned len = strlen(s) + 1;

		*data_size = len;

		if (in_size < len) {
			ret = EFI_BUFFER_TOO_SMALL;
			goto out;
		}

		if (!data) {
			debug("Variable with no data shouldn't exist.\n");
			return EFI_INVALID_PARAMETER;
		}

		memcpy(data, s, len);
		((char *)data)[len] = '\0';

		EFI_PRINT("got value: \"%s\"\n", (char *)data);
	} else {
		EFI_PRINT("invalid value: '%s'\n", val);
		return EFI_DEVICE_ERROR;
	}

out:
	if (attributes)
		*attributes = attr & EFI_VARIABLE_MASK;

	return ret;
}

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

	EFI_ENTRY("\"%ls\" %pUl %p %p %p", variable_name, vendor, attributes,
		  data_size, data);

	ret = efi_get_variable_common(variable_name, vendor, attributes,
				      data_size, data);
	return EFI_EXIT(ret);
}

static char *efi_variables_list;
static char *efi_cur_variable;

/**
 * parse_uboot_variable() - parse a u-boot variable and get uefi-related
 *			    information
 * @variable:		whole data of u-boot variable (ie. name=value)
 * @variable_name_size: size of variable_name buffer in byte
 * @variable_name:	name of uefi variable in u16, null-terminated
 * @vendor:		vendor's guid
 * @attributes:		attributes
 *
 * A uefi variable is encoded into a u-boot variable as described above.
 * This function parses such a u-boot variable and retrieve uefi-related
 * information into respective parameters. In return, variable_name_size
 * is the size of variable name including NULL.
 *
 * Return:		EFI_SUCCESS if parsing is OK, EFI_NOT_FOUND when
 *			the entire variable list has been returned,
 *			otherwise non-zero status code
 */
static efi_status_t parse_uboot_variable(char *variable,
					 efi_uintn_t *variable_name_size,
					 u16 *variable_name,
					 const efi_guid_t *vendor,
					 u32 *attributes)
{
	char *guid, *name, *end, c;
	size_t name_len;
	efi_uintn_t old_variable_name_size;
	u64 time;
	u16 *p;

	guid = strchr(variable, '_');
	if (!guid)
		return EFI_INVALID_PARAMETER;
	guid++;
	name = strchr(guid, '_');
	if (!name)
		return EFI_INVALID_PARAMETER;
	name++;
	end = strchr(name, '=');
	if (!end)
		return EFI_INVALID_PARAMETER;

	name_len = end - name;
	old_variable_name_size = *variable_name_size;
	*variable_name_size = sizeof(u16) * (name_len + 1);
	if (old_variable_name_size < *variable_name_size)
		return EFI_BUFFER_TOO_SMALL;

	end++; /* point to value */

	/* variable name */
	p = variable_name;
	utf8_utf16_strncpy(&p, name, name_len);
	variable_name[name_len] = 0;

	/* guid */
	c = *(name - 1);
	*(name - 1) = '\0'; /* guid need be null-terminated here */
	if (uuid_str_to_bin(guid, (unsigned char *)vendor,
			    UUID_STR_FORMAT_GUID))
		/* The only error would be EINVAL. */
		return EFI_INVALID_PARAMETER;
	*(name - 1) = c;

	/* attributes */
	parse_attr(end, attributes, &time);

	return EFI_SUCCESS;
}

/**
 * efi_get_next_variable_name() - enumerate the current variable names
 *
 * @variable_name_size:	size of variable_name buffer in byte
 * @variable_name:	name of uefi variable's name in u16
 * @vendor:		vendor's guid
 *
 * This function implements the GetNextVariableName service.
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
	char *native_name, *variable;
	ssize_t name_len, list_len;
	char regex[256];
	char * const regexlist[] = {regex};
	u32 attributes;
	int i;
	efi_status_t ret;

	EFI_ENTRY("%p \"%ls\" %pUl", variable_name_size, variable_name, vendor);

	if (!variable_name_size || !variable_name || !vendor)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	if (variable_name[0]) {
		/* check null-terminated string */
		for (i = 0; i < *variable_name_size; i++)
			if (!variable_name[i])
				break;
		if (i >= *variable_name_size)
			return EFI_EXIT(EFI_INVALID_PARAMETER);

		/* search for the last-returned variable */
		ret = efi_to_native(&native_name, variable_name, vendor);
		if (ret)
			return EFI_EXIT(ret);

		name_len = strlen(native_name);
		for (variable = efi_variables_list; variable && *variable;) {
			if (!strncmp(variable, native_name, name_len) &&
			    variable[name_len] == '=')
				break;

			variable = strchr(variable, '\n');
			if (variable)
				variable++;
		}

		free(native_name);
		if (!(variable && *variable))
			return EFI_EXIT(EFI_INVALID_PARAMETER);

		/* next variable */
		variable = strchr(variable, '\n');
		if (variable)
			variable++;
		if (!(variable && *variable))
			return EFI_EXIT(EFI_NOT_FOUND);
	} else {
		/*
		 *new search: free a list used in the previous search
		 */
		free(efi_variables_list);
		efi_variables_list = NULL;
		efi_cur_variable = NULL;

		snprintf(regex, 256, "efi_.*-.*-.*-.*-.*_.*");
		list_len = hexport_r(&env_htab, '\n',
				     H_MATCH_REGEX | H_MATCH_KEY,
				     &efi_variables_list, 0, 1, regexlist);

		if (list_len <= 1)
			return EFI_EXIT(EFI_NOT_FOUND);

		variable = efi_variables_list;
	}

	ret = parse_uboot_variable(variable, variable_name_size, variable_name,
				   vendor, &attributes);

	return EFI_EXIT(ret);
}

static efi_status_t efi_set_variable_common(u16 *variable_name,
					    const efi_guid_t *vendor,
					    u32 attributes,
					    efi_uintn_t data_size,
					    const void *data,
					    bool ro_check)
{
	char *native_name = NULL, *old_data = NULL, *val = NULL, *s;
	efi_uintn_t old_size;
	bool append, delete;
	u64 time = 0;
	u32 attr;
	efi_status_t ret = EFI_SUCCESS;

	if (!variable_name || !*variable_name || !vendor ||
	    ((attributes & EFI_VARIABLE_RUNTIME_ACCESS) &&
	     !(attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS))) {
		ret = EFI_INVALID_PARAMETER;
		goto err;
	}

	ret = efi_to_native(&native_name, variable_name, vendor);
	if (ret)
		goto err;

	/* check if a variable exists */
	old_size = 0;
	attr = 0;
	ret = efi_get_variable_common(variable_name, vendor, &attr,
				      &old_size, NULL);
	append = !!(attributes & EFI_VARIABLE_APPEND_WRITE);
	attributes &= ~(u32)EFI_VARIABLE_APPEND_WRITE;
	delete = !append && (!data_size || !attributes);

	/* check attributes */
	if (old_size) {
		if (ro_check && (attr & READ_ONLY)) {
			ret = EFI_WRITE_PROTECTED;
			goto err;
		}

		/* attributes won't be changed */
		if (!delete &&
		    ((ro_check && attr != attributes) ||
		     (!ro_check && ((attr & ~(u32)READ_ONLY)
				    != (attributes & ~(u32)READ_ONLY))))) {
			ret = EFI_INVALID_PARAMETER;
			goto err;
		}
	} else {
		if (delete || append) {
			/*
			 * Trying to delete or to update a non-existent
			 * variable.
			 */
			ret = EFI_NOT_FOUND;
			goto err;
		}
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
			debug("%ls: AUTHENTICATED_WRITE_ACCESS required\n",
			      variable_name);
			ret = EFI_INVALID_PARAMETER;
			goto err;
		}
	}

	/* authenticate a variable */
	if (IS_ENABLED(CONFIG_EFI_SECURE_BOOT)) {
		if (attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) {
			ret = EFI_INVALID_PARAMETER;
			goto err;
		}
		if (attributes &
		    EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) {
			ret = efi_variable_authenticate(variable_name, vendor,
							&data_size, &data,
							attributes, &attr,
							&time);
			if (ret != EFI_SUCCESS)
				goto err;

			/* last chance to check for delete */
			if (!data_size)
				delete = true;
		}
	} else {
		if (attributes &
		    (EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS |
		     EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)) {
			debug("Secure boot is not configured\n");
			ret = EFI_INVALID_PARAMETER;
			goto err;
		}
	}

	/* delete a variable */
	if (delete) {
		/* !old_size case has been handled before */
		val = NULL;
		ret = EFI_SUCCESS;
		goto out;
	}

	if (append) {
		old_data = malloc(old_size);
		if (!old_data) {
			ret = EFI_OUT_OF_RESOURCES;
			goto err;
		}
		ret = efi_get_variable_common(variable_name, vendor,
					      &attr, &old_size, old_data);
		if (ret != EFI_SUCCESS)
			goto err;
	} else {
		old_size = 0;
	}

	val = malloc(2 * old_size + 2 * data_size
		     + strlen("{ro,run,boot,nv,time=0123456701234567}(blob)")
		     + 1);
	if (!val) {
		ret = EFI_OUT_OF_RESOURCES;
		goto err;
	}

	s = val;

	/*
	 * store attributes
	 */
	attributes &= (READ_ONLY |
		       EFI_VARIABLE_NON_VOLATILE |
		       EFI_VARIABLE_BOOTSERVICE_ACCESS |
		       EFI_VARIABLE_RUNTIME_ACCESS |
		       EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS);
	s += sprintf(s, "{");
	while (attributes) {
		attr = 1 << (ffs(attributes) - 1);

		if (attr == READ_ONLY) {
			s += sprintf(s, "ro");
		} else if (attr == EFI_VARIABLE_NON_VOLATILE) {
			s += sprintf(s, "nv");
		} else if (attr == EFI_VARIABLE_BOOTSERVICE_ACCESS) {
			s += sprintf(s, "boot");
		} else if (attr == EFI_VARIABLE_RUNTIME_ACCESS) {
			s += sprintf(s, "run");
		} else if (attr ==
			   EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) {
			s += sprintf(s, "time=");
			s = bin2hex(s, (u8 *)&time, sizeof(time));
		}

		attributes &= ~attr;
		if (attributes)
			s += sprintf(s, ",");
	}
	s += sprintf(s, "}");
	s += sprintf(s, "(blob)");

	/* store payload: */
	if (append)
		s = bin2hex(s, old_data, old_size);
	s = bin2hex(s, data, data_size);
	*s = '\0';

	EFI_PRINT("setting: %s=%s\n", native_name, val);

out:
	if (env_set(native_name, val)) {
		ret = EFI_DEVICE_ERROR;
	} else {
		bool vendor_keys_modified = false;

		if ((u16_strcmp(variable_name, L"PK") == 0 &&
		     guidcmp(vendor, &efi_global_variable_guid) == 0)) {
			ret = efi_transfer_secure_state(
					(delete ? EFI_MODE_SETUP :
						  EFI_MODE_USER));
			if (ret != EFI_SUCCESS)
				goto err;

			if (efi_secure_mode != EFI_MODE_SETUP)
				vendor_keys_modified = true;
		} else if ((u16_strcmp(variable_name, L"KEK") == 0 &&
		     guidcmp(vendor, &efi_global_variable_guid) == 0)) {
			if (efi_secure_mode != EFI_MODE_SETUP)
				vendor_keys_modified = true;
		}

		/* update VendorKeys */
		if (vendor_keys_modified & efi_vendor_keys) {
			efi_vendor_keys = 0;
			ret = efi_set_variable_common(
						L"VendorKeys",
						&efi_global_variable_guid,
						EFI_VARIABLE_BOOTSERVICE_ACCESS
						 | EFI_VARIABLE_RUNTIME_ACCESS
						 | READ_ONLY,
						sizeof(efi_vendor_keys),
						&efi_vendor_keys,
						false);
		} else {
			ret = EFI_SUCCESS;
		}
	}

err:
	free(native_name);
	free(old_data);
	free(val);

	return ret;
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
	EFI_ENTRY("\"%ls\" %pUl %x %zu %p", variable_name, vendor, attributes,
		  data_size, data);

	/* READ_ONLY bit is not part of API */
	attributes &= ~(u32)READ_ONLY;

	return EFI_EXIT(efi_set_variable_common(variable_name, vendor,
						attributes, data_size, data,
						true));
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
efi_status_t __efi_runtime EFIAPI efi_query_variable_info(
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

	ret = efi_init_secure_state();

	return ret;
}
