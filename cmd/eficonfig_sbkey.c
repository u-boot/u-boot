// SPDX-License-Identifier: GPL-2.0+
/*
 *  Menu-driven UEFI Secure Boot Key Maintenance
 *
 *  Copyright (c) 2022 Masahisa Kojima, Linaro Limited
 */

#include <ansi.h>
#include <common.h>
#include <charset.h>
#include <hexdump.h>
#include <log.h>
#include <malloc.h>
#include <menu.h>
#include <efi_loader.h>
#include <efi_config.h>
#include <efi_variable.h>
#include <crypto/pkcs7_parser.h>

enum efi_sbkey_signature_type {
	SIG_TYPE_X509 = 0,
	SIG_TYPE_HASH,
	SIG_TYPE_CRL,
	SIG_TYPE_RSA2048,
};

struct eficonfig_sigtype_to_str {
	efi_guid_t sig_type;
	char *str;
	enum efi_sbkey_signature_type type;
};

static const struct eficonfig_sigtype_to_str sigtype_to_str[] = {
	{EFI_CERT_X509_GUID,		"X509",			SIG_TYPE_X509},
	{EFI_CERT_SHA256_GUID,		"SHA256",		SIG_TYPE_HASH},
	{EFI_CERT_X509_SHA256_GUID,	"X509_SHA256 CRL",	SIG_TYPE_CRL},
	{EFI_CERT_X509_SHA384_GUID,	"X509_SHA384 CRL",	SIG_TYPE_CRL},
	{EFI_CERT_X509_SHA512_GUID,	"X509_SHA512 CRL",	SIG_TYPE_CRL},
	/* U-Boot does not support the following signature types */
/*	{EFI_CERT_RSA2048_GUID,		"RSA2048",		SIG_TYPE_RSA2048}, */
/*	{EFI_CERT_RSA2048_SHA256_GUID,	"RSA2048_SHA256",	SIG_TYPE_RSA2048}, */
/*	{EFI_CERT_SHA1_GUID,		"SHA1",			SIG_TYPE_HASH}, */
/*	{EFI_CERT_RSA2048_SHA_GUID,	"RSA2048_SHA",		SIG_TYPE_RSA2048 }, */
/*	{EFI_CERT_SHA224_GUID,		"SHA224",		SIG_TYPE_HASH}, */
/*	{EFI_CERT_SHA384_GUID,		"SHA384",		SIG_TYPE_HASH}, */
/*	{EFI_CERT_SHA512_GUID,		"SHA512",		SIG_TYPE_HASH}, */
};

/**
 * file_have_auth_header() - check file has EFI_VARIABLE_AUTHENTICATION_2 header
 * @buf:	pointer to file
 * @size:	file size
 * Return:	true if file has auth header, false otherwise
 */
static bool file_have_auth_header(void *buf, efi_uintn_t size)
{
	struct efi_variable_authentication_2 *auth = buf;

	if (auth->auth_info.hdr.wCertificateType != WIN_CERT_TYPE_EFI_GUID)
		return false;

	if (guidcmp(&auth->auth_info.cert_type, &efi_guid_cert_type_pkcs7))
		return false;

	return true;
}

/**
 * eficonfig_process_enroll_key() - enroll key into signature database
 *
 * @data:	pointer to the data for each entry
 * Return:	status code
 */
static efi_status_t eficonfig_process_enroll_key(void *data)
{
	u32 attr;
	char *buf = NULL;
	efi_uintn_t size;
	efi_status_t ret;
	struct efi_file_handle *f = NULL;
	struct efi_device_path *full_dp = NULL;
	struct eficonfig_select_file_info file_info;

	file_info.current_path = calloc(1, EFICONFIG_FILE_PATH_BUF_SIZE);
	if (!file_info.current_path) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

	ret = eficonfig_process_select_file(&file_info);
	if (ret != EFI_SUCCESS)
		goto out;

	full_dp = eficonfig_create_device_path(file_info.dp_volume, file_info.current_path);
	if (!full_dp) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	f = efi_file_from_path(full_dp);
	if (!f) {
		ret = EFI_NOT_FOUND;
		goto out;
	}

	size = 0;
	ret = EFI_CALL(f->getinfo(f, &efi_file_info_guid, &size, NULL));
	if (ret != EFI_BUFFER_TOO_SMALL)
		goto out;

	buf = malloc(size);
	if (!buf) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	ret = EFI_CALL(f->getinfo(f, &efi_file_info_guid, &size, buf));
	if (ret != EFI_SUCCESS)
		goto out;

	size = ((struct efi_file_info *)buf)->file_size;
	free(buf);

	if (!size) {
		eficonfig_print_msg("ERROR! File is empty.");
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	buf = malloc(size);
	if (!buf) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

	ret = EFI_CALL(f->read(f, &size, buf));
	if (ret != EFI_SUCCESS) {
		eficonfig_print_msg("ERROR! Failed to read file.");
		goto out;
	}
	if (!file_have_auth_header(buf, size)) {
		eficonfig_print_msg("ERROR! Invalid file format. Only .auth variables is allowed.");
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	attr = EFI_VARIABLE_NON_VOLATILE |
	       EFI_VARIABLE_BOOTSERVICE_ACCESS |
	       EFI_VARIABLE_RUNTIME_ACCESS |
	       EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;

	/* PK can enroll only one certificate */
	if (u16_strcmp(data, u"PK")) {
		efi_uintn_t db_size = 0;

		/* check the variable exists. If exists, add APPEND_WRITE attribute */
		ret = efi_get_variable_int(data, efi_auth_var_get_guid(data), NULL,
					   &db_size, NULL,  NULL);
		if (ret == EFI_BUFFER_TOO_SMALL)
			attr |= EFI_VARIABLE_APPEND_WRITE;
	}

	ret = efi_set_variable_int((u16 *)data, efi_auth_var_get_guid((u16 *)data),
				   attr, size, buf, false);
	if (ret != EFI_SUCCESS)
		eficonfig_print_msg("ERROR! Failed to update signature database");

out:
	free(file_info.current_path);
	free(buf);
	efi_free_pool(full_dp);
	if (f)
		EFI_CALL(f->close(f));

	/* return to the parent menu */
	ret = (ret == EFI_ABORTED) ? EFI_NOT_READY : ret;

	return ret;
}

static struct eficonfig_item key_config_menu_items[] = {
	{"Enroll New Key", eficonfig_process_enroll_key},
	{"Quit", eficonfig_process_quit},
};

/**
 * eficonfig_process_set_secure_boot_key() - display the key configuration menu
 *
 * @data:	pointer to the data for each entry
 * Return:	status code
 */
static efi_status_t eficonfig_process_set_secure_boot_key(void *data)
{
	u32 i;
	efi_status_t ret;
	char header_str[32];
	struct efimenu *efi_menu;

	for (i = 0; i < ARRAY_SIZE(key_config_menu_items); i++)
		key_config_menu_items[i].data = data;

	snprintf(header_str, sizeof(header_str), "  ** Configure %ls **", (u16 *)data);

	while (1) {
		efi_menu = eficonfig_create_fixed_menu(key_config_menu_items,
						       ARRAY_SIZE(key_config_menu_items));

		ret = eficonfig_process_common(efi_menu, header_str);
		eficonfig_destroy(efi_menu);

		if (ret == EFI_ABORTED)
			break;
	}

	/* return to the parent menu */
	ret = (ret == EFI_ABORTED) ? EFI_NOT_READY : ret;

	return ret;
}

static const struct eficonfig_item secure_boot_menu_items[] = {
	{"PK", eficonfig_process_set_secure_boot_key, u"PK"},
	{"KEK", eficonfig_process_set_secure_boot_key, u"KEK"},
	{"db", eficonfig_process_set_secure_boot_key, u"db"},
	{"dbx", eficonfig_process_set_secure_boot_key, u"dbx"},
	{"Quit", eficonfig_process_quit},
};

/**
 * eficonfig_process_secure_boot_config() - display the key list menu
 *
 * @data:	pointer to the data for each entry
 * Return:	status code
 */
efi_status_t eficonfig_process_secure_boot_config(void *data)
{
	efi_status_t ret;
	struct efimenu *efi_menu;

	while (1) {
		char header_str[64];

		snprintf(header_str, sizeof(header_str),
			 "  ** UEFI Secure Boot Key Configuration (SecureBoot : %s) **",
			 (efi_secure_boot_enabled() ? "ON" : "OFF"));

		efi_menu = eficonfig_create_fixed_menu(secure_boot_menu_items,
						       ARRAY_SIZE(secure_boot_menu_items));
		if (!efi_menu) {
			ret = EFI_OUT_OF_RESOURCES;
			break;
		}

		ret = eficonfig_process_common(efi_menu, header_str);
		eficonfig_destroy(efi_menu);

		if (ret == EFI_ABORTED)
			break;
	}

	/* return to the parent menu */
	ret = (ret == EFI_ABORTED) ? EFI_NOT_READY : ret;

	return ret;
}
