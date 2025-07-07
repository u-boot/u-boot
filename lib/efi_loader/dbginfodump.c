// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2025, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * dbginfodump.efi prints out the content of the EFI_DEBUG_IMAGE_INFO_TABLE.
 */

#include <efi_api.h>

/**
 * BUFFER_SIZE - size of the command line input buffer
 */
#define BUFFER_SIZE 64

static struct efi_simple_text_output_protocol *cerr;
static struct efi_simple_text_output_protocol *cout;
static struct efi_simple_text_input_protocol *cin;
static efi_handle_t handle;
static struct efi_system_table *systable;
static struct efi_boot_services *bs;

static efi_guid_t guid_device_path_to_text_protocol =
	EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;

static struct efi_device_path_to_text_protocol *device_path_to_text;

/* EFI_DEBUG_IMAGE_INFO_TABLE_GUID */
static const efi_guid_t dbg_info_guid =
	EFI_GUID(0x49152E77, 0x1ADA, 0x4764, 0xB7, 0xA2,
		 0x7A, 0xFE, 0xFE, 0xD9, 0x5E, 0x8B);

/* EFI_DEBUG_IMAGE_INFO_NORMAL */
struct dbg_info {
	u32 type;
	struct efi_loaded_image *info;
	efi_handle_t handle;
};

/* FI_DEBUG_IMAGE_INFO_TABLE_HEADER */
struct dbg_info_header {
	u32 status;
	u32 size;
	struct dbg_info **info;
};

/**
 * print() - print string
 *
 * @string:	text
 */
static void print(u16 *string)
{
	cout->output_string(cout, string);
}

/**
 * error() - print error string
 *
 * @string:	error text
 */
static void error(u16 *string)
{
	cout->set_attribute(cout, EFI_LIGHTRED | EFI_BACKGROUND_BLACK);
	print(string);
	cout->set_attribute(cout, EFI_LIGHTBLUE | EFI_BACKGROUND_BLACK);
}

/**
 * printu() - print unsigned
 *
 * @val:	value to print
 */
static void printu(u32 val)
{
	u16 str[19] = u"0x";
	u16 *ptr = &str[2];
	u16 ch;

	for (ssize_t i = 8 * sizeof(u32) - 4; i >= 0; i -= 4) {
		ch = (val >> i & 0xf) + '0';
		if (ch > '9')
			ch += 'a' - '9' - 1;
		*ptr++ = ch;
	}
	*ptr = 0;
	print(str);
}

/**
 * printp() - print pointer
 *
 * @p:	pointer
 */
static void printp(void *p)
{
	u16 str[19] = u"0x";
	u16 *ptr = &str[2];
	u16 ch;

	for (ssize_t i = 8 * sizeof(void *) - 4; i >= 0; i -= 4) {
		ch = ((uintptr_t)p >> i & 0xf) + '0';
		if (ch > '9')
			ch += 'a' - '9' - 1;
		*ptr++ = ch;
	}
	*ptr = 0;
	print(str);
}

/**
 * efi_input() - read string from console
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
	u16 outbuf[2] = u" ";
	efi_status_t ret;

	/* Drain the console input */
	ret = cin->reset(cin, true);
	*buffer = 0;
	for (;;) {
		ret = bs->wait_for_event(1, &cin->wait_for_key, &index);
		if (ret != EFI_SUCCESS)
			continue;
		ret = cin->read_key_stroke(cin, &key);
		if (ret != EFI_SUCCESS)
			continue;
		switch (key.scan_code) {
		case 0x17: /* Escape */
			print(u"\r\nAborted\r\n");
			return EFI_ABORTED;
		default:
			break;
		}
		switch (key.unicode_char) {
		case 0x08: /* Backspace */
			if (pos) {
				buffer[pos--] = 0;
				print(u"\b \b");
			}
			break;
		case 0x0a: /* Linefeed */
		case 0x0d: /* Carriage return */
			print(u"\r\n");
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
			buffer[pos] = 0;
			print(outbuf);
		}
	}
}

/**
 * skip_whitespace() - skip over leading whitespace
 *
 * @pos:	UTF-16 string
 * Return:	pointer to first non-whitespace
 */
static u16 *skip_whitespace(u16 *pos)
{
	for (; *pos && *pos <= 0x20; ++pos)
		;
	return pos;
}

/**
 * starts_with() - check if @string starts with @keyword
 *
 * @string:	string to search for keyword
 * @keyword:	keyword to be searched
 * Return:	true fi @string starts with the keyword
 */
static bool starts_with(u16 *string, u16 *keyword)
{
	for (; *keyword; ++string, ++keyword) {
		if (*string != *keyword)
			return false;
	}
	return true;
}

/**
 * do_help() - print help
 */
static void do_help(void)
{
	error(u"dump       - print debug info table\r\n");
	error(u"exit       - exit the shell\r\n");
}

/**
 * get_dbg_info_table() - get debug info table
 *
 * Return:	debug info table or NULL
 */
static void *get_dbg_info(void)
{
	void *dbg = NULL;
	efi_uintn_t i;

	for (i = 0; i < systable->nr_tables; ++i) {
		if (!memcmp(&systable->tables[i].guid, &dbg_info_guid,
			    sizeof(efi_guid_t))) {
			dbg = systable->tables[i].table;
			break;
		}
	}
	return dbg;
}

/**
 * print_info() - print loaded image protocol
 */
static void print_info(struct efi_loaded_image *info)
{
	print(u"  Address: [");
	printp(info->image_base);
	print(u", ");
	printp(info->image_base + info->image_size - 1);
	print(u"]\r\n");
	if (device_path_to_text && info->file_path) {
		u16 *string;

		string = device_path_to_text->convert_device_path_to_text(
					info->file_path, true, false);
		if (!string) {
			error(u"ConvertDevicePathToText failed");
		} else {
			print(u"  File: ");
			print(string);
		}
		print(u"\r\n");
	}
}

/**
 * do_dump() - print debug info table
 */
static efi_status_t do_dump(void)
{
	struct dbg_info_header *dbg;
	u32 count;

	dbg = get_dbg_info();
	if (!dbg) {
		error(u"Debug info table not found\r\n");
		return EFI_NOT_FOUND;
	}
	if (dbg->status & 0x01) {
		error(u"Update in progress\r\n");
		return EFI_LOAD_ERROR;
	}
	if (dbg->status & 0x02)
		print(u"Modified\r\n");
	print(u"Number of entries: ");
	printu(dbg->size);
	print(u"\r\n");

	count = dbg->size;
	for (u32 i = 0; count; ++i) {
		struct dbg_info *info = dbg->info[i];

		/*
		 * The EDK II implementation decreases the size field and
		 * writes a NULL value when deleting an entry which is not
		 * backed by the UEFI specification.
		 */
		if (!info) {
			print(u"Deleted entry\r\n");
			continue;
		}
		--count;
		print(u"Info type ");
		printu(info->type);
		print(u"\r\n");
		if (info->type != 1)
			continue;
		print_info(info->info);
		print(u"  Handle: ");
		printp(info->handle);
		print(u"\r\n");
	}

	return EFI_SUCCESS;
}

/**
 * efi_main() - entry point of the EFI application.
 *
 * @handle:	handle of the loaded image
 * @systab:	system table
 * Return:	status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t image_handle,
			     struct efi_system_table *systab)
{
	efi_status_t ret;

	handle = image_handle;
	systable = systab;
	cerr = systable->std_err;
	cout = systable->con_out;
	cin = systable->con_in;
	bs = systable->boottime;

	cout->set_attribute(cout, EFI_LIGHTBLUE | EFI_BACKGROUND_BLACK);
	cout->clear_screen(cout);
	cout->set_attribute(cout, EFI_WHITE | EFI_BACKGROUND_BLACK);
	print(u"Debug Info Table Dump\r\n=====================\r\n\r\n");
	cout->set_attribute(cout, EFI_LIGHTBLUE | EFI_BACKGROUND_BLACK);

	ret = bs->locate_protocol(&guid_device_path_to_text_protocol,
				  NULL, (void **)&device_path_to_text);
	if (ret != EFI_SUCCESS)	{
		error(u"No device path to text protocol\r\n");
		device_path_to_text = NULL;
	}

	for (;;) {
		u16 command[BUFFER_SIZE];
		u16 *pos;
		efi_uintn_t ret;

		print(u"=> ");
		ret = efi_input(command, sizeof(command));
		if (ret == EFI_ABORTED)
			break;
		pos = skip_whitespace(command);
		if (starts_with(pos, u"exit"))
			break;
		else if (starts_with(pos, u"dump"))
			do_dump();
		else
			do_help();
	}

	cout->set_attribute(cout, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
	cout->clear_screen(cout);
	return EFI_SUCCESS;
}
