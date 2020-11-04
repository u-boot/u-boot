// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * dtbdump.efi saves the device tree provided as a configuration table
 * to a file.
 */

#include <common.h>
#include <efi_api.h>

#define BUFFER_SIZE 64
#define ESC 0x17
#define DEFAULT_FILENAME L"dtb.dtb"

static struct efi_simple_text_output_protocol *cerr;
static struct efi_simple_text_output_protocol *cout;
static struct efi_simple_text_input_protocol *cin;
static struct efi_boot_services *bs;
static const efi_guid_t fdt_guid = EFI_FDT_GUID;
static const efi_guid_t loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static const efi_guid_t guid_simple_file_system_protocol =
					EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

/**
 * input() - read string from console
 *
 * @buffer:		input buffer
 * @buffer_size:	buffer size
 * Return:		status code
 */
static efi_status_t efi_input(u16 *buffer, efi_uintn_t buffer_size)
{
	struct efi_input_key key = {0};
	efi_uintn_t index;
	efi_uintn_t pos = 0;
	u16 outbuf[2] = L" ";
	efi_status_t ret;

	/* Drain the console input */
	ret = cin->reset(cin, true);
	for (;;) {
		ret = bs->wait_for_event(1, &cin->wait_for_key, &index);
		if (ret != EFI_SUCCESS)
			continue;
		ret = cin->read_key_stroke(cin, &key);
		if (ret != EFI_SUCCESS)
			continue;
		switch (key.scan_code) {
		case 0x17: /* Escape */
			cout->output_string(cout, L"\nAborted\n");
			return EFI_ABORTED;
		default:
			break;
		}
		switch (key.unicode_char) {
		case 0x08: /* Backspace */
			if (pos) {
				buffer[pos--] = 0;
				cout->output_string(cout, L"\b \b");
			}
			break;
		case 0x0a: /* Linefeed */
		case 0x0d: /* Carriage return */
			return EFI_SUCCESS;
		default:
			break;
		}
		/* Ignore surrogate codes */
		if (key.unicode_char >= 0xD800 && key.unicode_char <= 0xDBFF)
			continue;
		if (key.unicode_char >= 0x20 &&
		    pos < buffer_size - 1) {
			*outbuf = key.unicode_char;
			buffer[pos++] = key.unicode_char;
			cout->output_string(cout, outbuf);
		}
	}
}

/*
 * Convert FDT value to host endianness.
 *
 * @val		FDT value
 * @return	converted value
 */
static u32 f2h(fdt32_t val)
{
	char *buf = (char *)&val;
	char i;

	/* Swap the bytes */
	i = buf[0]; buf[0] = buf[3]; buf[3] = i;
	i = buf[1]; buf[1] = buf[2]; buf[2] = i;
	return *(u32 *)buf;
}

/**
 * get_dtb() - get device tree
 *
 * @systable:	system table
 * Return:	device tree or NULL
 */
void *get_dtb(struct efi_system_table *systable)
{
	void *dtb = NULL;
	efi_uintn_t i;

	for (i = 0; i < systable->nr_tables; ++i) {
		if (!memcmp(&systable->tables[i].guid, &fdt_guid,
			    sizeof(efi_guid_t))) {
			dtb = systable->tables[i].table;
			break;
		}
	}
	return dtb;
}

/**
 * efi_main() - entry point of the EFI application.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t handle,
			     struct efi_system_table *systable)
{
	efi_uintn_t ret;
	u16 filename[BUFFER_SIZE] = {0};
	efi_uintn_t dtb_size;
	struct efi_loaded_image *loaded_image;
	struct efi_simple_file_system_protocol *file_system;
	struct efi_file_handle *root, *file;
	struct fdt_header *dtb;

	cerr = systable->std_err;
	cout = systable->con_out;
	cin = systable->con_in;
	bs = systable->boottime;

	cout->set_attribute(cout, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
	cout->clear_screen(cout);
	cout->set_attribute(cout, EFI_YELLOW | EFI_BACKGROUND_BLACK);
	cout->output_string(cout, L"DTB Dump\n\n");
	cout->set_attribute(cout, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);

	dtb = get_dtb(systable);
	if (!dtb) {
		cerr->output_string(cout, L"DTB not found\n");
		return EFI_NOT_FOUND;
	}
	if (f2h(dtb->magic) != FDT_MAGIC) {
		cerr->output_string(cout, L"Wrong device tree magic\n");
		return EFI_NOT_FOUND;
	}
	dtb_size = f2h(dtb->totalsize);

	cout->output_string(cout, L"Filename (" DEFAULT_FILENAME ")?\n");
	ret = efi_input(filename, sizeof(filename));
	if (ret != EFI_SUCCESS)
		return ret;
	if (!*filename)
		memcpy(filename, DEFAULT_FILENAME, sizeof(DEFAULT_FILENAME));

	cout->output_string(cout, L"\n");

	ret = bs->open_protocol(handle, &loaded_image_guid,
				(void **)&loaded_image, NULL, NULL,
				EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		cerr->output_string(cout,
				    L"Loaded image protocol not found\n");
		return ret;
	}

	/* Open the simple file system protocol */
	ret = bs->open_protocol(loaded_image->device_handle,
				&guid_simple_file_system_protocol,
				(void **)&file_system, NULL, NULL,
				EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		cerr->output_string(
			cout, L"Failed to open simple file system protocol\n");
		return ret;
	}

	/* Open volume */
	ret = file_system->open_volume(file_system, &root);
	if (ret != EFI_SUCCESS) {
		cerr->output_string(cerr, L"Failed to open volume\n");
		return ret;
	}
	/* Create file */
	ret = root->open(root, &file, filename,
			 EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE |
			 EFI_FILE_MODE_CREATE, EFI_FILE_ARCHIVE);
	if (ret == EFI_SUCCESS) {
		/* Write file */
		ret = file->write(file, &dtb_size, dtb);
		if (ret != EFI_SUCCESS)
			cerr->output_string(cerr, L"Failed to write file\n");
		file->close(file);
	} else {
		cerr->output_string(cerr, L"Failed to open file\n");
	}
	root->close(root);

	if (ret == EFI_SUCCESS) {
		cout->output_string(cout, filename);
		cout->output_string(cout, L" written\n");
	}

	return ret;
}
