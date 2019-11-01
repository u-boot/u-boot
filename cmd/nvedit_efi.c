// SPDX-License-Identifier: GPL-2.0+
/*
 *  Integrate UEFI variables to u-boot env interface
 *
 *  Copyright (c) 2018 AKASHI Takahiro, Linaro Limited
 */

#include <charset.h>
#include <common.h>
#include <command.h>
#include <efi_loader.h>
#include <env.h>
#include <exports.h>
#include <hexdump.h>
#include <malloc.h>
#include <mapmem.h>
#include <linux/kernel.h>

/*
 * From efi_variable.c,
 *
 * Mapping between UEFI variables and u-boot variables:
 *
 *   efi_$guid_$varname = {attributes}(type)value
 */

static const struct {
	u32 mask;
	char *text;
} efi_var_attrs[] = {
	{EFI_VARIABLE_NON_VOLATILE, "NV"},
	{EFI_VARIABLE_BOOTSERVICE_ACCESS, "BS"},
	{EFI_VARIABLE_RUNTIME_ACCESS, "RT"},
	{EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS, "AW"},
	{EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS, "AT"},
};

static const struct {
	efi_guid_t guid;
	char *text;
} efi_guid_text[] = {
	/* signature database */
	{EFI_GLOBAL_VARIABLE_GUID, "EFI_GLOBAL_VARIABLE_GUID"},
};

/* "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" */
static char unknown_guid[37];

/**
 * efi_guid_to_str() - convert guid to readable name
 *
 * @guid:	GUID
 * Return:	string for GUID
 *
 * convert guid to readable name
 */
static const char *efi_guid_to_str(const efi_guid_t *guid)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(efi_guid_text); i++)
		if (!guidcmp(guid, &efi_guid_text[i].guid))
			return efi_guid_text[i].text;

	uuid_bin_to_str((unsigned char *)guid->b, unknown_guid,
			UUID_STR_FORMAT_GUID);

	return unknown_guid;
}

/**
 * efi_dump_single_var() - show information about a UEFI variable
 *
 * @name:	Name of the variable
 * @guid:	Vendor GUID
 * @verbose:	if true, dump data
 *
 * Show information encoded in one UEFI variable
 */
static void efi_dump_single_var(u16 *name, const efi_guid_t *guid, bool verbose)
{
	u32 attributes;
	u8 *data;
	efi_uintn_t size;
	int count, i;
	efi_status_t ret;

	data = NULL;
	size = 0;
	ret = EFI_CALL(efi_get_variable(name, guid, &attributes, &size, data));
	if (ret == EFI_BUFFER_TOO_SMALL) {
		data = malloc(size);
		if (!data)
			goto out;

		ret = EFI_CALL(efi_get_variable(name, guid, &attributes, &size,
						data));
	}
	if (ret == EFI_NOT_FOUND) {
		printf("Error: \"%ls\" not defined\n", name);
		goto out;
	}
	if (ret != EFI_SUCCESS)
		goto out;

	printf("%ls:\n    %s:", name, efi_guid_to_str(guid));
	for (count = 0, i = 0; i < ARRAY_SIZE(efi_var_attrs); i++)
		if (attributes & efi_var_attrs[i].mask) {
			if (count)
				putc('|');
			else
				putc(' ');
			count++;
			puts(efi_var_attrs[i].text);
		}
	printf(", DataSize = 0x%zx\n", size);
	if (verbose)
		print_hex_dump("    ", DUMP_PREFIX_OFFSET, 16, 1,
			       data, size, true);

out:
	free(data);
}

/**
 * efi_dump_vars() - show information about named UEFI variables
 *
 * @argc:	Number of arguments (variables)
 * @argv:	Argument (variable name) array
 * @verbose:	if true, dump data
 * Return:	CMD_RET_SUCCESS on success, or CMD_RET_RET_FAILURE
 *
 * Show information encoded in named UEFI variables
 */
static int efi_dump_vars(int argc,  char * const argv[],
			 const efi_guid_t *guid, bool verbose)
{
	u16 *var_name16, *p;
	efi_uintn_t buf_size, size;

	buf_size = 128;
	var_name16 = malloc(buf_size);
	if (!var_name16)
		return CMD_RET_FAILURE;

	for (; argc > 0; argc--, argv++) {
		size = (utf8_utf16_strlen(argv[0]) + 1) * sizeof(u16);
		if (buf_size < size) {
			buf_size = size;
			p = realloc(var_name16, buf_size);
			if (!p) {
				free(var_name16);
				return CMD_RET_FAILURE;
			}
			var_name16 = p;
		}

		p = var_name16;
		utf8_utf16_strcpy(&p, argv[0]);

		efi_dump_single_var(var_name16, guid, verbose);
	}

	free(var_name16);

	return CMD_RET_SUCCESS;
}

static bool match_name(int argc, char * const argv[], u16 *var_name16)
{
	char *buf, *p;
	size_t buflen;
	int i;
	bool result = false;

	buflen = utf16_utf8_strlen(var_name16) + 1;
	buf = calloc(1, buflen);
	if (!buf)
		return result;

	p = buf;
	utf16_utf8_strcpy(&p, var_name16);

	for (i = 0; i < argc; argc--, argv++) {
		if (!strcmp(buf, argv[i])) {
			result = true;
			goto out;
		}
	}

out:
	free(buf);

	return result;
}

/**
 * efi_dump_var_all() - show information about all the UEFI variables
 *
 * @argc:	Number of arguments (variables)
 * @argv:	Argument (variable name) array
 * @verbose:	if true, dump data
 * Return:	CMD_RET_SUCCESS on success, or CMD_RET_RET_FAILURE
 *
 * Show information encoded in all the UEFI variables
 */
static int efi_dump_var_all(int argc,  char * const argv[],
			    const efi_guid_t *guid_p, bool verbose)
{
	u16 *var_name16, *p;
	efi_uintn_t buf_size, size;
	efi_guid_t guid;
	efi_status_t ret;

	if (argc && guid_p)
		/* simplified case */
		return efi_dump_vars(argc, argv, guid_p, verbose);

	buf_size = 128;
	var_name16 = malloc(buf_size);
	if (!var_name16)
		return CMD_RET_FAILURE;

	var_name16[0] = 0;
	for (;;) {
		size = buf_size;
		ret = EFI_CALL(efi_get_next_variable_name(&size, var_name16,
							  &guid));
		if (ret == EFI_NOT_FOUND)
			break;
		if (ret == EFI_BUFFER_TOO_SMALL) {
			buf_size = size;
			p = realloc(var_name16, buf_size);
			if (!p) {
				free(var_name16);
				return CMD_RET_FAILURE;
			}
			var_name16 = p;
			ret = EFI_CALL(efi_get_next_variable_name(&size,
								  var_name16,
								  &guid));
		}
		if (ret != EFI_SUCCESS) {
			free(var_name16);
			return CMD_RET_FAILURE;
		}

		if ((!guid_p || !guidcmp(guid_p, &guid)) &&
		    (!argc || match_name(argc, argv, var_name16)))
			efi_dump_single_var(var_name16, &guid, verbose);
	}

	free(var_name16);

	return CMD_RET_SUCCESS;
}

/**
 * do_env_print_efi() - show information about UEFI variables
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, or CMD_RET_RET_FAILURE
 *
 * This function is for "env print -e" or "printenv -e" command:
 *   => env print -e [-n] [-guid <guid> | -all] [var [...]]
 * If one or more variable names are specified, show information
 * named UEFI variables, otherwise show all the UEFI variables.
 */
int do_env_print_efi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	efi_guid_t guid;
	const efi_guid_t *guid_p;
	bool default_guid, guid_any, verbose;
	efi_status_t ret;

	/* Initialize EFI drivers */
	ret = efi_init_obj_list();
	if (ret != EFI_SUCCESS) {
		printf("Error: Cannot initialize UEFI sub-system, r = %lu\n",
		       ret & ~EFI_ERROR_MASK);
		return CMD_RET_FAILURE;
	}

	default_guid = true;
	guid_any = false;
	verbose = true;
	for (argc--, argv++; argc > 0 && argv[0][0] == '-'; argc--, argv++) {
		if (!strcmp(argv[0], "-guid")) {
			if (argc == 1)
				return CMD_RET_USAGE;

			/* -a already specified */
			if (!default_guid & guid_any)
				return CMD_RET_USAGE;

			argc--;
			argv++;
			if (uuid_str_to_bin(argv[0], guid.b,
					    UUID_STR_FORMAT_GUID))
				return CMD_RET_USAGE;
			default_guid = false;
		} else if (!strcmp(argv[0], "-all")) {
			/* -guid already specified */
			if (!default_guid && !guid_any)
				return CMD_RET_USAGE;

			guid_any = true;
			default_guid = false;
		} else if (!strcmp(argv[0], "-n")) {
			verbose = false;
		} else {
			return CMD_RET_USAGE;
		}
	}

	if (guid_any)
		guid_p = NULL;
	else if (default_guid)
		guid_p = &efi_global_variable_guid;
	else
		guid_p = (const efi_guid_t *)guid.b;

	/* enumerate and show all UEFI variables */
	return efi_dump_var_all(argc, argv, guid_p, verbose);
}

/**
 * append_value() - encode UEFI variable's value
 * @bufp:	Buffer of encoded UEFI variable's value
 * @sizep:	Size of buffer
 * @data:	data to be encoded into the value
 * Return:	0 on success, -1 otherwise
 *
 * Interpret a given data string and append it to buffer.
 * Buffer will be realloc'ed if necessary.
 *
 * Currently supported formats are:
 *   =0x0123...:		Hexadecimal number
 *   =H0123...:			Hexadecimal-byte array
 *   ="...", =S"..." or <string>:
 *				String
 */
static int append_value(char **bufp, size_t *sizep, char *data)
{
	char *tmp_buf = NULL, *new_buf = NULL, *value;
	unsigned long len = 0;

	if (!strncmp(data, "=0x", 2)) { /* hexadecimal number */
		union {
			u8 u8;
			u16 u16;
			u32 u32;
			u64 u64;
		} tmp_data;
		unsigned long hex_value;
		void *hex_ptr;

		data += 3;
		len = strlen(data);
		if ((len & 0x1)) /* not multiple of two */
			return -1;

		len /= 2;
		if (len > 8)
			return -1;
		else if (len > 4)
			len = 8;
		else if (len > 2)
			len = 4;

		/* convert hex hexadecimal number */
		if (strict_strtoul(data, 16, &hex_value) < 0)
			return -1;

		tmp_buf = malloc(len);
		if (!tmp_buf)
			return -1;

		if (len == 1) {
			tmp_data.u8 = hex_value;
			hex_ptr = &tmp_data.u8;
		} else if (len == 2) {
			tmp_data.u16 = hex_value;
			hex_ptr = &tmp_data.u16;
		} else if (len == 4) {
			tmp_data.u32 = hex_value;
			hex_ptr = &tmp_data.u32;
		} else {
			tmp_data.u64 = hex_value;
			hex_ptr = &tmp_data.u64;
		}
		memcpy(tmp_buf, hex_ptr, len);
		value = tmp_buf;

	} else if (!strncmp(data, "=H", 2)) { /* hexadecimal-byte array */
		data += 2;
		len = strlen(data);
		if (len & 0x1) /* not multiple of two */
			return -1;

		len /= 2;
		tmp_buf = malloc(len);
		if (!tmp_buf)
			return -1;

		if (hex2bin((u8 *)tmp_buf, data, len) < 0) {
			printf("Error: illegal hexadecimal string\n");
			free(tmp_buf);
			return -1;
		}

		value = tmp_buf;
	} else { /* string */
		if (!strncmp(data, "=\"", 2) || !strncmp(data, "=S\"", 3)) {
			if (data[1] == '"')
				data += 2;
			else
				data += 3;
			value = data;
			len = strlen(data) - 1;
			if (data[len] != '"')
				return -1;
		} else {
			value = data;
			len = strlen(data);
		}
	}

	new_buf = realloc(*bufp, *sizep + len);
	if (!new_buf)
		goto out;

	memcpy(new_buf + *sizep, value, len);
	*bufp = new_buf;
	*sizep += len;

out:
	free(tmp_buf);

	return 0;
}

/**
 * do_env_set_efi() - set UEFI variable
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, or CMD_RET_RET_FAILURE
 *
 * This function is for "env set -e" or "setenv -e" command:
 *   => env set -e [-guid guid][-nv][-bs][-rt][-a][-v]
 *		   [-i address,size] var, or
 *                 var [value ...]
 * Encode values specified and set given UEFI variable.
 * If no value is specified, delete the variable.
 */
int do_env_set_efi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *var_name, *value, *ep;
	ulong addr;
	efi_uintn_t size;
	efi_guid_t guid;
	u32 attributes;
	bool default_guid, verbose, value_on_memory;
	u16 *var_name16 = NULL, *p;
	size_t len;
	efi_status_t ret;

	if (argc == 1)
		return CMD_RET_USAGE;

	/* Initialize EFI drivers */
	ret = efi_init_obj_list();
	if (ret != EFI_SUCCESS) {
		printf("Error: Cannot initialize UEFI sub-system, r = %lu\n",
		       ret & ~EFI_ERROR_MASK);
		return CMD_RET_FAILURE;
	}

	/*
	 * attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS |
	 *	     EFI_VARIABLE_RUNTIME_ACCESS;
	 */
	value = NULL;
	size = 0;
	attributes = 0;
	guid = efi_global_variable_guid;
	default_guid = true;
	verbose = false;
	value_on_memory = false;
	for (argc--, argv++; argc > 0 && argv[0][0] == '-'; argc--, argv++) {
		if (!strcmp(argv[0], "-guid")) {
			if (argc == 1)
				return CMD_RET_USAGE;

			argc--;
			argv++;
			if (uuid_str_to_bin(argv[0], guid.b,
					    UUID_STR_FORMAT_GUID)) {
				printf("## Guid not specified or in XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX format\n");
				return CMD_RET_FAILURE;
			}
			default_guid = false;
		} else if (!strcmp(argv[0], "-bs")) {
			attributes |= EFI_VARIABLE_BOOTSERVICE_ACCESS;
		} else if (!strcmp(argv[0], "-rt")) {
			attributes |= EFI_VARIABLE_RUNTIME_ACCESS;
		} else if (!strcmp(argv[0], "-nv")) {
			attributes |= EFI_VARIABLE_NON_VOLATILE;
		} else if (!strcmp(argv[0], "-a")) {
			attributes |= EFI_VARIABLE_APPEND_WRITE;
		} else if (!strcmp(argv[0], "-i")) {
			/* data comes from memory */
			if (argc == 1)
				return CMD_RET_USAGE;

			argc--;
			argv++;
			addr = simple_strtoul(argv[0], &ep, 16);
			if (*ep != ',')
				return CMD_RET_USAGE;

			size = simple_strtoul(++ep, NULL, 16);
			if (!size)
				return CMD_RET_FAILURE;
			value_on_memory = true;
		} else if (!strcmp(argv[0], "-v")) {
			verbose = true;
		} else {
			return CMD_RET_USAGE;
		}
	}
	if (!argc)
		return CMD_RET_USAGE;

	var_name = argv[0];
	if (default_guid)
		guid = efi_global_variable_guid;

	if (verbose) {
		printf("GUID: %s\n", efi_guid_to_str((const efi_guid_t *)
						     &guid));
		printf("Attributes: 0x%x\n", attributes);
	}

	/* for value */
	if (value_on_memory)
		value = map_sysmem(addr, 0);
	else if (argc > 1)
		for (argc--, argv++; argc > 0; argc--, argv++)
			if (append_value(&value, &size, argv[0]) < 0) {
				printf("## Failed to process an argument, %s\n",
				       argv[0]);
				ret = CMD_RET_FAILURE;
				goto out;
			}

	if (size && verbose) {
		printf("Value:\n");
		print_hex_dump("    ", DUMP_PREFIX_OFFSET,
			       16, 1, value, size, true);
	}

	len = utf8_utf16_strnlen(var_name, strlen(var_name));
	var_name16 = malloc((len + 1) * 2);
	if (!var_name16) {
		printf("## Out of memory\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}
	p = var_name16;
	utf8_utf16_strncpy(&p, var_name, len + 1);

	ret = EFI_CALL(efi_set_variable(var_name16, &guid, attributes,
					size, value));
	unmap_sysmem(value);
	if (ret == EFI_SUCCESS) {
		ret = CMD_RET_SUCCESS;
	} else {
		const char *msg;

		switch (ret) {
		case EFI_NOT_FOUND:
			msg = " (not found)";
			break;
		case EFI_WRITE_PROTECTED:
			msg = " (read only)";
			break;
		case EFI_INVALID_PARAMETER:
			msg = " (invalid parameter)";
			break;
		case EFI_SECURITY_VIOLATION:
			msg = " (validation failed)";
			break;
		case EFI_OUT_OF_RESOURCES:
			msg = " (out of memory)";
			break;
		default:
			msg = "";
			break;
		}
		printf("## Failed to set EFI variable%s\n", msg);
		ret = CMD_RET_FAILURE;
	}
out:
	if (value_on_memory)
		unmap_sysmem(value);
	else
		free(value);
	free(var_name16);

	return ret;
}
