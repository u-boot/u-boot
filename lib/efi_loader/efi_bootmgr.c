/*
 *  EFI utils
 *
 *  Copyright (c) 2017 Rob Clark
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <charset.h>
#include <malloc.h>
#include <efi_loader.h>

static const struct efi_boot_services *bs;
static const struct efi_runtime_services *rs;

#define LOAD_OPTION_ACTIVE		0x00000001
#define LOAD_OPTION_FORCE_RECONNECT	0x00000002
#define LOAD_OPTION_HIDDEN		0x00000008

/*
 * bootmgr implements the logic of trying to find a payload to boot
 * based on the BootOrder + BootXXXX variables, and then loading it.
 *
 * TODO detecting a special key held (f9?) and displaying a boot menu
 * like you would get on a PC would be clever.
 *
 * TODO if we had a way to write and persist variables after the OS
 * has started, we'd also want to check OsIndications to see if we
 * should do normal or recovery boot.
 */


/*
 * See section 3.1.3 in the v2.7 UEFI spec for more details on
 * the layout of EFI_LOAD_OPTION.  In short it is:
 *
 *    typedef struct _EFI_LOAD_OPTION {
 *        UINT32 Attributes;
 *        UINT16 FilePathListLength;
 *        // CHAR16 Description[];   <-- variable length, NULL terminated
 *        // EFI_DEVICE_PATH_PROTOCOL FilePathList[];  <-- FilePathListLength bytes
 *        // UINT8 OptionalData[];
 *    } EFI_LOAD_OPTION;
 */
struct load_option {
	u32 attributes;
	u16 file_path_length;
	u16 *label;
	struct efi_device_path *file_path;
	u8 *optional_data;
};

/* parse an EFI_LOAD_OPTION, as described above */
static void parse_load_option(struct load_option *lo, void *ptr)
{
	lo->attributes = *(u32 *)ptr;
	ptr += sizeof(u32);

	lo->file_path_length = *(u16 *)ptr;
	ptr += sizeof(u16);

	lo->label = ptr;
	ptr += (utf16_strlen(lo->label) + 1) * 2;

	lo->file_path = ptr;
	ptr += lo->file_path_length;

	lo->optional_data = ptr;
}

/* free() the result */
static void *get_var(u16 *name, const efi_guid_t *vendor,
		     unsigned long *size)
{
	efi_guid_t *v = (efi_guid_t *)vendor;
	efi_status_t ret;
	void *buf = NULL;

	*size = 0;
	EFI_CALL(ret = rs->get_variable((s16 *)name, v, NULL, size, buf));
	if (ret == EFI_BUFFER_TOO_SMALL) {
		buf = malloc(*size);
		EFI_CALL(ret = rs->get_variable((s16 *)name, v, NULL, size, buf));
	}

	if (ret != EFI_SUCCESS) {
		free(buf);
		*size = 0;
		return NULL;
	}

	return buf;
}

/*
 * Attempt to load load-option number 'n', returning device_path and file_path
 * if successful.  This checks that the EFI_LOAD_OPTION is active (enabled)
 * and that the specified file to boot exists.
 */
static void *try_load_entry(uint16_t n, struct efi_device_path **device_path,
			    struct efi_device_path **file_path)
{
	struct load_option lo;
	u16 varname[] = L"Boot0000";
	u16 hexmap[] = L"0123456789ABCDEF";
	void *load_option, *image = NULL;
	unsigned long size;

	varname[4] = hexmap[(n & 0xf000) >> 12];
	varname[5] = hexmap[(n & 0x0f00) >> 8];
	varname[6] = hexmap[(n & 0x00f0) >> 4];
	varname[7] = hexmap[(n & 0x000f) >> 0];

	load_option = get_var(varname, &efi_global_variable_guid, &size);
	if (!load_option)
		return NULL;

	parse_load_option(&lo, load_option);

	if (lo.attributes & LOAD_OPTION_ACTIVE) {
		efi_status_t ret;

		debug("%s: trying to load \"%ls\" from %pD\n",
		      __func__, lo.label, lo.file_path);

		ret = efi_load_image_from_path(lo.file_path, &image);

		if (ret != EFI_SUCCESS)
			goto error;

		printf("Booting: %ls\n", lo.label);
		efi_dp_split_file_path(lo.file_path, device_path, file_path);
	}

error:
	free(load_option);

	return image;
}

/*
 * Attempt to load, in the order specified by BootOrder EFI variable, the
 * available load-options, finding and returning the first one that can
 * be loaded successfully.
 */
void *efi_bootmgr_load(struct efi_device_path **device_path,
		       struct efi_device_path **file_path)
{
	uint16_t *bootorder;
	unsigned long size;
	void *image = NULL;
	int i, num;

	__efi_entry_check();

	bs = systab.boottime;
	rs = systab.runtime;

	bootorder = get_var(L"BootOrder", &efi_global_variable_guid, &size);
	if (!bootorder)
		goto error;

	num = size / sizeof(uint16_t);
	for (i = 0; i < num; i++) {
		debug("%s: trying to load Boot%04X\n", __func__, bootorder[i]);
		image = try_load_entry(bootorder[i], device_path, file_path);
		if (image)
			break;
	}

	free(bootorder);

error:
	__efi_exit_check();

	return image;
}
