// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI boot manager
 *
 *  Copyright (c) 2017 Rob Clark
 */

#include <common.h>
#include <charset.h>
#include <malloc.h>
#include <efi_loader.h>
#include <asm/unaligned.h>

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


/* Parse serialized data and transform it into efi_load_option structure */
void efi_deserialize_load_option(struct efi_load_option *lo, u8 *data)
{
	lo->attributes = get_unaligned_le32(data);
	data += sizeof(u32);

	lo->file_path_length = get_unaligned_le16(data);
	data += sizeof(u16);

	/* FIXME */
	lo->label = (u16 *)data;
	data += (u16_strlen(lo->label) + 1) * sizeof(u16);

	/* FIXME */
	lo->file_path = (struct efi_device_path *)data;
	data += lo->file_path_length;

	lo->optional_data = data;
}

/*
 * Serialize efi_load_option structure into byte stream for BootXXXX.
 * Return a size of allocated data.
 */
unsigned long efi_serialize_load_option(struct efi_load_option *lo, u8 **data)
{
	unsigned long label_len, option_len;
	unsigned long size;
	u8 *p;

	label_len = (u16_strlen(lo->label) + 1) * sizeof(u16);
	option_len = strlen((char *)lo->optional_data);

	/* total size */
	size = sizeof(lo->attributes);
	size += sizeof(lo->file_path_length);
	size += label_len;
	size += lo->file_path_length;
	size += option_len + 1;
	p = malloc(size);
	if (!p)
		return 0;

	/* copy data */
	*data = p;
	memcpy(p, &lo->attributes, sizeof(lo->attributes));
	p += sizeof(lo->attributes);

	memcpy(p, &lo->file_path_length, sizeof(lo->file_path_length));
	p += sizeof(lo->file_path_length);

	memcpy(p, lo->label, label_len);
	p += label_len;

	memcpy(p, lo->file_path, lo->file_path_length);
	p += lo->file_path_length;

	memcpy(p, lo->optional_data, option_len);
	p += option_len;
	*(char *)p = '\0';

	return size;
}

/* free() the result */
static void *get_var(u16 *name, const efi_guid_t *vendor,
		     efi_uintn_t *size)
{
	efi_guid_t *v = (efi_guid_t *)vendor;
	efi_status_t ret;
	void *buf = NULL;

	*size = 0;
	EFI_CALL(ret = rs->get_variable(name, v, NULL, size, buf));
	if (ret == EFI_BUFFER_TOO_SMALL) {
		buf = malloc(*size);
		EFI_CALL(ret = rs->get_variable(name, v, NULL, size, buf));
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
	struct efi_load_option lo;
	u16 varname[] = L"Boot0000";
	u16 hexmap[] = L"0123456789ABCDEF";
	void *load_option, *image = NULL;
	efi_uintn_t size;

	varname[4] = hexmap[(n & 0xf000) >> 12];
	varname[5] = hexmap[(n & 0x0f00) >> 8];
	varname[6] = hexmap[(n & 0x00f0) >> 4];
	varname[7] = hexmap[(n & 0x000f) >> 0];

	load_option = get_var(varname, &efi_global_variable_guid, &size);
	if (!load_option)
		return NULL;

	efi_deserialize_load_option(&lo, load_option);

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
	efi_uintn_t size;
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
