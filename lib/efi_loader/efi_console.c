/*
 *  EFI application console interface
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>

static bool console_size_queried;

#define EFI_COUT_MODE_2 2
#define EFI_MAX_COUT_MODE 3

struct cout_mode {
	unsigned long columns;
	unsigned long rows;
	int present;
};

static struct cout_mode efi_cout_modes[] = {
	/* EFI Mode 0 is 80x25 and always present */
	{
		.columns = 80,
		.rows = 25,
		.present = 1,
	},
	/* EFI Mode 1 is always 80x50 */
	{
		.columns = 80,
		.rows = 50,
		.present = 0,
	},
	/* Value are unknown until we query the console */
	{
		.columns = 0,
		.rows = 0,
		.present = 0,
	},
};

const efi_guid_t efi_guid_console_control = CONSOLE_CONTROL_GUID;

#define cESC '\x1b'
#define ESC "\x1b"

static efi_status_t EFIAPI efi_cin_get_mode(
			struct efi_console_control_protocol *this,
			int *mode, char *uga_exists, char *std_in_locked)
{
	EFI_ENTRY("%p, %p, %p, %p", this, mode, uga_exists, std_in_locked);

	if (mode)
		*mode = EFI_CONSOLE_MODE_TEXT;
	if (uga_exists)
		*uga_exists = 0;
	if (std_in_locked)
		*std_in_locked = 0;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_cin_set_mode(
			struct efi_console_control_protocol *this, int mode)
{
	EFI_ENTRY("%p, %d", this, mode);
	return EFI_EXIT(EFI_UNSUPPORTED);
}

static efi_status_t EFIAPI efi_cin_lock_std_in(
			struct efi_console_control_protocol *this,
			uint16_t *password)
{
	EFI_ENTRY("%p, %p", this, password);
	return EFI_EXIT(EFI_UNSUPPORTED);
}

const struct efi_console_control_protocol efi_console_control = {
	.get_mode = efi_cin_get_mode,
	.set_mode = efi_cin_set_mode,
	.lock_std_in = efi_cin_lock_std_in,
};

/* Default to mode 0 */
static struct simple_text_output_mode efi_con_mode = {
	.max_mode = 1,
	.mode = 0,
	.attribute = 0,
	.cursor_column = 0,
	.cursor_row = 0,
	.cursor_visible = 1,
};

static int term_read_reply(int *n, int maxnum, char end_char)
{
	char c;
	int i = 0;

	c = getc();
	if (c != cESC)
		return -1;
	c = getc();
	if (c != '[')
		return -1;

	n[0] = 0;
	while (1) {
		c = getc();
		if (c == ';') {
			i++;
			if (i >= maxnum)
				return -1;
			n[i] = 0;
			continue;
		} else if (c == end_char) {
			break;
		} else if (c > '9' || c < '0') {
			return -1;
		}

		/* Read one more decimal position */
		n[i] *= 10;
		n[i] += c - '0';
	}

	return 0;
}

static efi_status_t EFIAPI efi_cout_reset(
			struct efi_simple_text_output_protocol *this,
			char extended_verification)
{
	EFI_ENTRY("%p, %d", this, extended_verification);
	return EFI_EXIT(EFI_UNSUPPORTED);
}

static void print_unicode_in_utf8(u16 c)
{
	char utf8[4] = { 0 };
	char *b = utf8;

	if (c < 0x80) {
		*(b++) = c;
	} else if (c < 0x800) {
		*(b++) = 192 + c / 64;
		*(b++) = 128 + c % 64;
	} else {
		*(b++) = 224 + c / 4096;
		*(b++) = 128 + c / 64 % 64;
		*(b++) = 128 + c % 64;
	}

	puts(utf8);
}

static efi_status_t EFIAPI efi_cout_output_string(
			struct efi_simple_text_output_protocol *this,
			const unsigned short *string)
{
	struct cout_mode *mode;
	u16 ch;

	mode = &efi_cout_modes[efi_con_mode.mode];
	EFI_ENTRY("%p, %p", this, string);
	for (;(ch = *string); string++) {
		print_unicode_in_utf8(ch);
		efi_con_mode.cursor_column++;
		if (ch == '\n') {
			efi_con_mode.cursor_column = 1;
			efi_con_mode.cursor_row++;
		} else if (efi_con_mode.cursor_column > mode->columns) {
			efi_con_mode.cursor_column = 1;
			efi_con_mode.cursor_row++;
		}
		if (efi_con_mode.cursor_row > mode->rows)
			efi_con_mode.cursor_row = mode->rows;
	}

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_cout_test_string(
			struct efi_simple_text_output_protocol *this,
			const unsigned short *string)
{
	EFI_ENTRY("%p, %p", this, string);
	return EFI_EXIT(EFI_SUCCESS);
}

static bool cout_mode_matches(struct cout_mode *mode, int rows, int cols)
{
	if (!mode->present)
		return false;

	return (mode->rows == rows) && (mode->columns == cols);
}

static efi_status_t EFIAPI efi_cout_query_mode(
			struct efi_simple_text_output_protocol *this,
			unsigned long mode_number, unsigned long *columns,
			unsigned long *rows)
{
	EFI_ENTRY("%p, %ld, %p, %p", this, mode_number, columns, rows);

	if (!console_size_queried) {
		/* Ask the terminal about its size */
		int n[3];
		int cols;
		int rows;
		u64 timeout;

		console_size_queried = true;

		/* Empty input buffer */
		while (tstc())
			getc();

		printf(ESC"[18t");

		/* Check if we have a terminal that understands */
		timeout = timer_get_us() + 1000000;
		while (!tstc())
			if (timer_get_us() > timeout)
				goto out;

		/* Read {depth,rows,cols} */
		if (term_read_reply(n, 3, 't')) {
			goto out;
		}

		cols = n[2];
		rows = n[1];

		/* Test if we can have Mode 1 */
		if (cols >= 80 && rows >= 50) {
			efi_cout_modes[1].present = 1;
			efi_con_mode.max_mode = 2;
		}

		/*
		 * Install our mode as mode 2 if it is different
		 * than mode 0 or 1 and set it  as the currently selected mode
		 */
		if (!cout_mode_matches(&efi_cout_modes[0], rows, cols) &&
		    !cout_mode_matches(&efi_cout_modes[1], rows, cols)) {
			efi_cout_modes[EFI_COUT_MODE_2].columns = cols;
			efi_cout_modes[EFI_COUT_MODE_2].rows = rows;
			efi_cout_modes[EFI_COUT_MODE_2].present = 1;
			efi_con_mode.max_mode = EFI_MAX_COUT_MODE;
			efi_con_mode.mode = EFI_COUT_MODE_2;
		}
	}

	if (mode_number >= efi_con_mode.max_mode)
		return EFI_EXIT(EFI_UNSUPPORTED);

	if (efi_cout_modes[mode_number].present != 1)
		return EFI_EXIT(EFI_UNSUPPORTED);

out:
	if (columns)
		*columns = efi_cout_modes[mode_number].columns;
	if (rows)
		*rows = efi_cout_modes[mode_number].rows;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_cout_set_mode(
			struct efi_simple_text_output_protocol *this,
			unsigned long mode_number)
{
	EFI_ENTRY("%p, %ld", this, mode_number);


	if (mode_number > efi_con_mode.max_mode)
		return EFI_EXIT(EFI_UNSUPPORTED);

	efi_con_mode.mode = mode_number;
	efi_con_mode.cursor_column = 0;
	efi_con_mode.cursor_row = 0;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_cout_set_attribute(
			struct efi_simple_text_output_protocol *this,
			unsigned long attribute)
{
	EFI_ENTRY("%p, %lx", this, attribute);

	/* Just ignore attributes (colors) for now */
	return EFI_EXIT(EFI_UNSUPPORTED);
}

static efi_status_t EFIAPI efi_cout_clear_screen(
			struct efi_simple_text_output_protocol *this)
{
	EFI_ENTRY("%p", this);

	printf(ESC"[2J");

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_cout_set_cursor_position(
			struct efi_simple_text_output_protocol *this,
			unsigned long column, unsigned long row)
{
	EFI_ENTRY("%p, %ld, %ld", this, column, row);

	printf(ESC"[%d;%df", (int)row, (int)column);
	efi_con_mode.cursor_column = column;
	efi_con_mode.cursor_row = row;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_cout_enable_cursor(
			struct efi_simple_text_output_protocol *this,
			bool enable)
{
	EFI_ENTRY("%p, %d", this, enable);

	printf(ESC"[?25%c", enable ? 'h' : 'l');

	return EFI_EXIT(EFI_SUCCESS);
}

const struct efi_simple_text_output_protocol efi_con_out = {
	.reset = efi_cout_reset,
	.output_string = efi_cout_output_string,
	.test_string = efi_cout_test_string,
	.query_mode = efi_cout_query_mode,
	.set_mode = efi_cout_set_mode,
	.set_attribute = efi_cout_set_attribute,
	.clear_screen = efi_cout_clear_screen,
	.set_cursor_position = efi_cout_set_cursor_position,
	.enable_cursor = efi_cout_enable_cursor,
	.mode = (void*)&efi_con_mode,
};

static efi_status_t EFIAPI efi_cin_reset(
			struct efi_simple_input_interface *this,
			bool extended_verification)
{
	EFI_ENTRY("%p, %d", this, extended_verification);
	return EFI_EXIT(EFI_UNSUPPORTED);
}

static efi_status_t EFIAPI efi_cin_read_key_stroke(
			struct efi_simple_input_interface *this,
			struct efi_input_key *key)
{
	struct efi_input_key pressed_key = {
		.scan_code = 0,
		.unicode_char = 0,
	};
	char ch;

	EFI_ENTRY("%p, %p", this, key);

	/* We don't do interrupts, so check for timers cooperatively */
	efi_timer_check();

	if (!tstc()) {
		/* No key pressed */
		return EFI_EXIT(EFI_NOT_READY);
	}

	ch = getc();
	if (ch == cESC) {
		/* Escape Sequence */
		ch = getc();
		switch (ch) {
		case cESC: /* ESC */
			pressed_key.scan_code = 23;
			break;
		case 'O': /* F1 - F4 */
			pressed_key.scan_code = getc() - 'P' + 11;
			break;
		case 'a'...'z':
			ch = ch - 'a';
			break;
		case '[':
			ch = getc();
			switch (ch) {
			case 'A'...'D': /* up, down right, left */
				pressed_key.scan_code = ch - 'A' + 1;
				break;
			case 'F': /* End */
				pressed_key.scan_code = 6;
				break;
			case 'H': /* Home */
				pressed_key.scan_code = 5;
				break;
			case '1': /* F5 - F8 */
				pressed_key.scan_code = getc() - '0' + 11;
				getc();
				break;
			case '2': /* F9 - F12 */
				pressed_key.scan_code = getc() - '0' + 19;
				getc();
				break;
			case '3': /* DEL */
				pressed_key.scan_code = 8;
				getc();
				break;
			}
			break;
		}
	} else if (ch == 0x7f) {
		/* Backspace */
		ch = 0x08;
	}
	pressed_key.unicode_char = ch;
	*key = pressed_key;

	return EFI_EXIT(EFI_SUCCESS);
}

const struct efi_simple_input_interface efi_con_in = {
	.reset = efi_cin_reset,
	.read_key_stroke = efi_cin_read_key_stroke,
	.wait_for_key = NULL,
};
