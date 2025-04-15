// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * dtbdump.efi saves the device tree provided as a configuration table
 * to a file.
 */

#include <efi_api.h>
#include <efi_dt_fixup.h>
#include <part.h>
#include <linux/libfdt.h>

#define BUFFER_SIZE 64
#define ESC 0x17

#define efi_size_in_pages(size) ((size + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT)

static struct efi_simple_text_output_protocol *cerr;
static struct efi_simple_text_output_protocol *cout;
static struct efi_simple_text_input_protocol *cin;
static struct efi_boot_services *bs;
static const efi_guid_t fdt_guid = EFI_FDT_GUID;
static const efi_guid_t loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static const efi_guid_t guid_simple_file_system_protocol =
					EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
static efi_handle_t handle;
static struct efi_system_table *systable;
static const efi_guid_t efi_dt_fixup_protocol_guid = EFI_DT_FIXUP_PROTOCOL_GUID;
static const efi_guid_t efi_file_info_guid = EFI_FILE_INFO_GUID;
static const efi_guid_t efi_system_partition_guid = PARTITION_SYSTEM_GUID;

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
 * print_char() - print character
 *
 * 0x00 is replaced by '", "'.
 *
 * @c:	- character
 */
static void print_char(unsigned char c)
{
	u16 out[2] = u"?";

	if (!c) {
		print(u"\", \"");
		return;
	}

	if (c > 0x1f && c < 0x80)
		out[0] = c;

	print(out);
}

/**
 * print_hex_digit() - print hexadecimal digit
 *
 * @digit:	digit to print
 */
static void print_hex_digit(unsigned char digit)
{
	if (digit < 10)
		digit += '0';
	else
		digit += 'a' - 10;
	print_char(digit);
}

/**
 * printx() - print hexadecimal byte
 *
 * @val:	value to print
 */
static void printx(unsigned char val)
{
	print_hex_digit(val >> 4);
	print_hex_digit(val & 0xf);
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
 * efi_input_yn() - get answer to yes/no question
 *
 * Return:
 * y or Y
 *     EFI_SUCCESS
 * n or N
 *     EFI_ACCESS_DENIED
 * ESC
 *     EFI_ABORTED
 */
static efi_status_t efi_input_yn(void)
{
	struct efi_input_key key = {0};
	efi_uintn_t index;
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
			return EFI_ABORTED;
		default:
			break;
		}
		/* Convert to lower case */
		switch (key.unicode_char | 0x20) {
		case 'y':
			return EFI_SUCCESS;
		case 'n':
			return EFI_ACCESS_DENIED;
		default:
			break;
		}
	}
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

/*
 * Convert FDT value to host endianness.
 *
 * @val		FDT value
 * Return:	converted value
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
static void *get_dtb(struct efi_system_table *systable)
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
	error(u"dump       - print device-tree\r\n");
	error(u"load <dtb> - load device-tree from file\r\n");
	error(u"save <dtb> - save device-tree to file\r\n");
	error(u"exit       - exit the shell\r\n");
}

/**
 * open_file_system() - open simple file system protocol
 *
 * file_system:	interface of the simple file system protocol
 * Return:	status code
 */
static efi_status_t
open_file_system(struct efi_simple_file_system_protocol **file_system)
{
	struct efi_loaded_image *loaded_image;
	efi_status_t ret;
	efi_handle_t *handle_buffer = NULL;
	efi_uintn_t count;

	ret = bs->open_protocol(handle, &loaded_image_guid,
				(void **)&loaded_image, NULL, NULL,
				EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		error(u"Loaded image protocol not found\r\n");
		return ret;
	}

	/* Open the simple file system protocol on the same partition */
	ret = bs->open_protocol(loaded_image->device_handle,
				&guid_simple_file_system_protocol,
				(void **)file_system, NULL, NULL,
				EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret == EFI_SUCCESS)
		return ret;

	/* Open the simple file system protocol on the UEFI system partition */
	ret = bs->locate_handle_buffer(BY_PROTOCOL, &efi_system_partition_guid,
				       NULL, &count, &handle_buffer);
	if (ret == EFI_SUCCESS && handle_buffer)
		ret = bs->open_protocol(handle_buffer[0],
					&guid_simple_file_system_protocol,
					(void **)file_system, NULL, NULL,
					EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS)
		error(u"Failed to open simple file system protocol\r\n");
	if (handle)
		bs->free_pool(handle_buffer);

	return ret;
}

/**
 * do_load() - load and install device-tree
 *
 * @filename:	file name
 * Return:	status code
 */
static efi_status_t do_load(u16 *filename)
{
	struct efi_dt_fixup_protocol *dt_fixup_prot;
	struct efi_simple_file_system_protocol *file_system;
	struct efi_file_handle *root = NULL, *file = NULL;
	u64 addr = 0;
	struct efi_file_info *info;
	struct fdt_header *dtb;
	efi_uintn_t buffer_size;
	efi_uintn_t pages;
	efi_status_t ret, ret2;

	ret = bs->locate_protocol(&efi_dt_fixup_protocol_guid, NULL,
				  (void **)&dt_fixup_prot);
	if (ret != EFI_SUCCESS) {
		error(u"Device-tree fix-up protocol not found\r\n");
		return ret;
	}

	filename = skip_whitespace(filename);

	ret = open_file_system(&file_system);
	if (ret != EFI_SUCCESS)
		goto out;

	/* Open volume */
	ret = file_system->open_volume(file_system, &root);
	if (ret != EFI_SUCCESS) {
		error(u"Failed to open volume\r\n");
		goto out;
	}

	/* Open file */
	ret = root->open(root, &file, filename, EFI_FILE_MODE_READ, 0);
	if (ret != EFI_SUCCESS) {
		error(u"File not found\r\n");
		goto out;
	}
	/* Get file size */
	buffer_size = 0;
	ret = file->getinfo(file, &efi_file_info_guid, &buffer_size, NULL);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		error(u"Can't get file info size\r\n");
		goto out;
	}
	ret = bs->allocate_pool(EFI_LOADER_DATA, buffer_size, (void **)&info);
	if (ret != EFI_SUCCESS) {
		error(u"Out of memory\r\n");
		goto out;
	}
	ret = file->getinfo(file, &efi_file_info_guid, &buffer_size, info);
	if (ret != EFI_SUCCESS) {
		error(u"Can't get file info\r\n");
		goto out;
	}
	buffer_size = info->file_size;
	pages = efi_size_in_pages(buffer_size);
	ret = bs->free_pool(info);
	if (ret != EFI_SUCCESS)
		error(u"Can't free memory pool\r\n");
	/* Read file */
	ret = bs->allocate_pages(EFI_ALLOCATE_ANY_PAGES,
				 EFI_ACPI_RECLAIM_MEMORY,
				 pages, &addr);
	if (ret != EFI_SUCCESS) {
		error(u"Out of memory\r\n");
		goto out;
	}
	dtb = (struct fdt_header *)(uintptr_t)addr;
	ret = file->read(file, &buffer_size, dtb);
	if (ret != EFI_SUCCESS) {
		error(u"Can't read file\r\n");
		goto out;
	}
	/* Fixup file, expecting EFI_BUFFER_TOO_SMALL */
	ret = dt_fixup_prot->fixup(dt_fixup_prot, dtb, &buffer_size,
				   EFI_DT_APPLY_FIXUPS | EFI_DT_RESERVE_MEMORY |
				   EFI_DT_INSTALL_TABLE);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		/* Read file into larger buffer */
		ret = bs->free_pages(addr, pages);
		if (ret != EFI_SUCCESS)
			error(u"Can't free memory pages\r\n");
		pages = efi_size_in_pages(buffer_size);
		ret = bs->allocate_pages(EFI_ALLOCATE_ANY_PAGES,
					 EFI_ACPI_RECLAIM_MEMORY,
					 pages, &addr);
		if (ret != EFI_SUCCESS) {
			error(u"Out of memory\r\n");
			goto out;
		}
		dtb = (struct fdt_header *)(uintptr_t)addr;
		ret = file->setpos(file, 0);
		if (ret != EFI_SUCCESS) {
			error(u"Can't position file\r\n");
			goto out;
		}
		ret = file->read(file, &buffer_size, dtb);
		if (ret != EFI_SUCCESS) {
			error(u"Can't read file\r\n");
			goto out;
		}
		buffer_size = pages << EFI_PAGE_SHIFT;
		ret = dt_fixup_prot->fixup(
				dt_fixup_prot, dtb, &buffer_size,
				EFI_DT_APPLY_FIXUPS | EFI_DT_RESERVE_MEMORY |
				EFI_DT_INSTALL_TABLE);
	}
	if (ret == EFI_SUCCESS)
		print(u"device-tree installed\r\n");
	else
		error(u"Device-tree fix-up failed\r\n");
out:
	if (addr) {
		ret2 = bs->free_pages(addr, pages);
		if (ret2 != EFI_SUCCESS)
			error(u"Can't free memory pages\r\n");
	}
	if (file) {
		ret2 = file->close(file);
		if (ret2 != EFI_SUCCESS)
			error(u"Can't close file\r\n");
	}
	if (root) {
		ret2 = root->close(root);
		if (ret2 != EFI_SUCCESS)
			error(u"Can't close volume\r\n");
	}
	return ret;
}

/**
 * do_save() - save current device-tree
 *
 * @filename:	file name
 * Return:	status code
 */
static efi_status_t do_save(u16 *filename)
{
	struct efi_simple_file_system_protocol *file_system;
	efi_uintn_t dtb_size;
	struct efi_file_handle *root, *file;
	struct fdt_header *dtb;
	efi_uintn_t ret;

	dtb = get_dtb(systable);
	if (!dtb) {
		error(u"DTB not found\r\n");
		return EFI_NOT_FOUND;
	}
	if (f2h(dtb->magic) != FDT_MAGIC) {
		error(u"Wrong device tree magic\r\n");
		return EFI_NOT_FOUND;
	}
	dtb_size = f2h(dtb->totalsize);

	filename = skip_whitespace(filename);

	ret = open_file_system(&file_system);
	if (ret != EFI_SUCCESS)
		return ret;

	/* Open volume */
	ret = file_system->open_volume(file_system, &root);
	if (ret != EFI_SUCCESS) {
		error(u"Failed to open volume\r\n");
		return ret;
	}
	/* Check if file already exists */
	ret = root->open(root, &file, filename, EFI_FILE_MODE_READ, 0);
	if (ret == EFI_SUCCESS) {
		file->close(file);
		print(u"Overwrite existing file (y/n)? ");
		ret = efi_input_yn();
		print(u"\r\n");
		if (ret != EFI_SUCCESS) {
			root->close(root);
			error(u"Aborted by user\r\n");
			return ret;
		}
	}

	/* Create file */
	ret = root->open(root, &file, filename,
			 EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE |
			 EFI_FILE_MODE_CREATE, EFI_FILE_ARCHIVE);
	if (ret == EFI_SUCCESS) {
		/* Write file */
		ret = file->write(file, &dtb_size, dtb);
		if (ret != EFI_SUCCESS)
			error(u"Failed to write file\r\n");
		file->close(file);
	} else {
		error(u"Failed to open file\r\n");
	}
	root->close(root);

	if (ret == EFI_SUCCESS) {
		print(filename);
		print(u" written\r\n");
	}

	return ret;
}

/**
 * indent() - print a number of tabstops
 *
 * @level:	indentation level
 */
static void indent(u32 level)
{
	for (; level; --level)
		print(u"\t");
}

/**
 * is_string_value() - determine if property is a string
 *
 * If a property is a string, an x-string, or a u32 cannot be deducted
 * from the device-tree. Therefore a heuristic is used.
 *
 * @str:	pointer to device-tree property
 * @len:	length of the device-tree property
 * Return:	1 for string, 0 otherwise
 */
static int is_string_value(const unsigned char *str, u32 len)
{
	int nonzero_flag = 0;

	/* Zero length or not ending with 0x00 */
	if (!len || str[len - 1])
		return 0;

	for (u32 i = 0; i < len; ++i) {
		if (!str[i]) {
			/* Zero length string or two consecutive 0x00 */
			if (!nonzero_flag)
				return 0;

			nonzero_flag = 0;

			continue;
		}
		/* Non-printable */
		if (str[i] < 0x20 || str[i] >= 0x80)
			return 0;

		nonzero_flag = 1;
	}

	return 1;
}

/**
 * print_property() - print device-tree property
 *
 * If a property is a string, an x-string, or a u32 cannot be deducted
 * from the device-tree. Therefore a heuristic is used.
 *
 * @str:	property value
 * @len:	length of property value
 */
static void print_property(const unsigned char *val, u32 len)
{
	if (is_string_value(val, len)) {
		/* string */
		print(u"\"");
		for (int i = 0; i < len - 1; ++i)
			print_char(val[i]);
		print(u"\"");
	} else if (len & 0x3) {
		/* byte string */
		print(u"[");
		for (int i = 0; i < len; ++i) {
			if (i)
				print(u" ");
			printx(val[i]);
		}
		print(u"]\"");
	} else {
		/* cell list */
		print(u"<");
		for (u32 i = 0; i < len; ++i) {
			if ((i & 0x3) == 0) {
				if (i > 0)
					print(u" ");
				print(u"0x");
			}
			printx(val[i]);
		}
		print(u">");
	}
}

/**
 * print_mem_res_block() - print memory reservation block
 *
 * @rsvblk:	memory reservation block
 */
static void print_mem_res_block(const struct fdt_reserve_entry *rsvblk)
{
	for (; rsvblk->address || rsvblk->size; ++rsvblk) {
		const unsigned char *val;

		print(u"/memreserve/ 0x");
		val = (const unsigned char *)&rsvblk->address;
		for (u32 i = 0; i < sizeof(u64); ++i)
			printx(val[i]);
		print(u" 0x");
		val = (const unsigned char *)&rsvblk->size;
		for (u32 i = 0; i < sizeof(u64); ++i)
			printx(val[i]);
		print(u";\r\n");
	}
}

/**
 * do_dump() - print device-tree
 */
static efi_status_t do_dump(void)
{
	const unsigned char *fdt;
	struct fdt_header *header;
	const u32 *end;
	const u32 *pos;
	const char *strings;
	u32 level = 0;

	fdt = get_dtb(systable);
	if (!fdt) {
		error(u"DTB not found\r\n");
		return EFI_NOT_FOUND;
	}

	header = (struct fdt_header *)fdt;
	if (f2h(header->magic) != FDT_MAGIC) {
		error(u"Wrong device tree magic\r\n");
		error(u"Not a device-tree\r\n");
		return EFI_LOAD_ERROR;
	}

	pos = (u32 *)(fdt + f2h(header->off_dt_struct));
	end = &pos[f2h(header->totalsize) >> 2];
	strings = fdt + f2h(header->off_dt_strings);

	print(u"/dts-v1/;\r\n");

	print_mem_res_block((const struct fdt_reserve_entry *)
			    (fdt + f2h(header->off_mem_rsvmap)));

	print(u"/");
	for (; pos < end;) {
		switch (f2h(pos[0])) {
		case FDT_BEGIN_NODE: {
			const char *c = (char *)&pos[1];
			size_t i;

			indent(level);
			for (i = 0; c[i]; ++i)
				print_char(c[i]);
			print(u" {\n\r");

			++level;
			pos = &pos[2 + (i >> 2)];
			break;
		}
		case FDT_PROP: {
			struct fdt_property *prop = (struct fdt_property *)pos;
			const unsigned char *label = &strings[f2h(prop->nameoff)];
			u32 len = f2h(prop->len);
			const unsigned char *str = (unsigned char *)&pos[3];

			indent(level);
			for (int i = 0; label[i]; ++i)
				print_char(label[i]);

			if (len) {
				print(u" = ");
				print_property(str, len);
			}
			print(u";\r\n");

			pos = &pos[3 + ((f2h(prop->len) + 3) >> 2)];
			break;
		}
		case FDT_NOP:
			++pos;
			break;
		case FDT_END_NODE:
			if (!level) {
				error(u"Extraneous end node\r\n");
				return EFI_LOAD_ERROR;
			}

			--level;
			indent(level);
			print(u"};\n\r");
			++pos;
			break;
		case FDT_END:
			if (level) {
				error(u"Missing end node\r\n");
				return EFI_LOAD_ERROR;
			}
			return EFI_SUCCESS;
		default:
			error(u"Invalid device tree token\r\n");
			return EFI_LOAD_ERROR;
		}
	}
	error(u"Overrun\r\n");

	return EFI_LOAD_ERROR;
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
	handle = image_handle;
	systable = systab;
	cerr = systable->std_err;
	cout = systable->con_out;
	cin = systable->con_in;
	bs = systable->boottime;

	cout->set_attribute(cout, EFI_LIGHTBLUE | EFI_BACKGROUND_BLACK);
	cout->clear_screen(cout);
	cout->set_attribute(cout, EFI_WHITE | EFI_BACKGROUND_BLACK);
	print(u"DTB Dump\r\n========\r\n\r\n");
	cout->set_attribute(cout, EFI_LIGHTBLUE | EFI_BACKGROUND_BLACK);

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
		else if (starts_with(pos, u"load "))
			do_load(pos + 5);
		else if (starts_with(pos, u"save "))
			do_save(pos + 5);
		else
			do_help();
	}

	cout->set_attribute(cout, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
	cout->clear_screen(cout);
	return EFI_SUCCESS;
}
