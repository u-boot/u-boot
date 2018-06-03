// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application console interface
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#include <common.h>
#include <charset.h>
#include <dm/device.h>
#include <efi_loader.h>
#include <stdio_dev.h>
#include <video_console.h>

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

const efi_guid_t efi_guid_text_output_protocol =
			EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;
const efi_guid_t efi_guid_text_input_protocol =
			EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID;

#define cESC '\x1b'
#define ESC "\x1b"

/* Default to mode 0 */
static struct simple_text_output_mode efi_con_mode = {
	.max_mode = 1,
	.mode = 0,
	.attribute = 0,
	.cursor_column = 0,
	.cursor_row = 0,
	.cursor_visible = 1,
};

/*
 * Receive and parse a reply from the terminal.
 *
 * @n:		array of return values
 * @num:	number of return values expected
 * @end_char:	character indicating end of terminal message
 * @return:	non-zero indicates error
 */
static int term_read_reply(int *n, int num, char end_char)
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
			if (i >= num)
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
	if (i != num - 1)
		return -1;

	return 0;
}

static efi_status_t EFIAPI efi_cout_reset(
			struct efi_simple_text_output_protocol *this,
			char extended_verification)
{
	EFI_ENTRY("%p, %d", this, extended_verification);
	return EFI_EXIT(EFI_UNSUPPORTED);
}

static efi_status_t EFIAPI efi_cout_output_string(
			struct efi_simple_text_output_protocol *this,
			const efi_string_t string)
{
	struct simple_text_output_mode *con = &efi_con_mode;
	struct cout_mode *mode = &efi_cout_modes[con->mode];

	EFI_ENTRY("%p, %p", this, string);

	unsigned int n16 = utf16_strlen(string);
	char buf[MAX_UTF8_PER_UTF16 * n16 + 1];
	u16 *p;

	*utf16_to_utf8((u8 *)buf, string, n16) = '\0';

	fputs(stdout, buf);

	/*
	 * Update the cursor position.
	 *
	 * The UEFI spec provides advance rules for U+0000, U+0008, U+000A,
	 * and U000D. All other characters, including control characters
	 * U+0007 (bel) and U+0009 (tab), have to increase the column by one.
	 */
	for (p = string; *p; ++p) {
		switch (*p) {
		case '\b':	/* U+0008, backspace */
			con->cursor_column = max(0, con->cursor_column - 1);
			break;
		case '\n':	/* U+000A, newline */
			con->cursor_column = 0;
			con->cursor_row++;
			break;
		case '\r':	/* U+000D, carriage-return */
			con->cursor_column = 0;
			break;
		case 0xd800 ... 0xdbff:
			/*
			 * Ignore high surrogates, we do not want to count a
			 * Unicode character twice.
			 */
			break;
		default:
			con->cursor_column++;
			break;
		}
		if (con->cursor_column >= mode->columns) {
			con->cursor_column = 0;
			con->cursor_row++;
		}
		con->cursor_row = min(con->cursor_row, (s32)mode->rows - 1);
	}

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_cout_test_string(
			struct efi_simple_text_output_protocol *this,
			const efi_string_t string)
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

static int query_console_serial(int *rows, int *cols)
{
	/* Ask the terminal about its size */
	int n[3];
	u64 timeout;

	/* Empty input buffer */
	while (tstc())
		getc();

	printf(ESC"[18t");

	/* Check if we have a terminal that understands */
	timeout = timer_get_us() + 1000000;
	while (!tstc())
		if (timer_get_us() > timeout)
			return -1;

	/* Read {depth,rows,cols} */
	if (term_read_reply(n, 3, 't'))
		return -1;

	*cols = n[2];
	*rows = n[1];

	return 0;
}

/*
 * Update the mode table.
 *
 * By default the only mode available is 80x25. If the console has at least 50
 * lines, enable mode 80x50. If we can query the console size and it is neither
 * 80x25 nor 80x50, set it as an additional mode.
 */
static void query_console_size(void)
{
	const char *stdout_name = env_get("stdout");
	int rows = 25, cols = 80;

	if (stdout_name && !strcmp(stdout_name, "vidconsole") &&
	    IS_ENABLED(CONFIG_DM_VIDEO)) {
		struct stdio_dev *stdout_dev =
			stdio_get_by_name("vidconsole");
		struct udevice *dev = stdout_dev->priv;
		struct vidconsole_priv *priv =
			dev_get_uclass_priv(dev);
		rows = priv->rows;
		cols = priv->cols;
	} else if (query_console_serial(&rows, &cols)) {
		return;
	}

	/* Test if we can have Mode 1 */
	if (cols >= 80 && rows >= 50) {
		efi_cout_modes[1].present = 1;
		efi_con_mode.max_mode = 2;
	}

	/*
	 * Install our mode as mode 2 if it is different
	 * than mode 0 or 1 and set it as the currently selected mode
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

static efi_status_t EFIAPI efi_cout_query_mode(
			struct efi_simple_text_output_protocol *this,
			unsigned long mode_number, unsigned long *columns,
			unsigned long *rows)
{
	EFI_ENTRY("%p, %ld, %p, %p", this, mode_number, columns, rows);

	if (mode_number >= efi_con_mode.max_mode)
		return EFI_EXIT(EFI_UNSUPPORTED);

	if (efi_cout_modes[mode_number].present != 1)
		return EFI_EXIT(EFI_UNSUPPORTED);

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

static const struct {
	unsigned int fg;
	unsigned int bg;
} color[] = {
	{ 30, 40 },     /* 0: black */
	{ 34, 44 },     /* 1: blue */
	{ 32, 42 },     /* 2: green */
	{ 36, 46 },     /* 3: cyan */
	{ 31, 41 },     /* 4: red */
	{ 35, 45 },     /* 5: magenta */
	{ 33, 43 },     /* 6: brown, map to yellow as edk2 does*/
	{ 37, 47 },     /* 7: light grey, map to white */
};

/* See EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.SetAttribute(). */
static efi_status_t EFIAPI efi_cout_set_attribute(
			struct efi_simple_text_output_protocol *this,
			unsigned long attribute)
{
	unsigned int bold = EFI_ATTR_BOLD(attribute);
	unsigned int fg = EFI_ATTR_FG(attribute);
	unsigned int bg = EFI_ATTR_BG(attribute);

	EFI_ENTRY("%p, %lx", this, attribute);

	if (attribute)
		printf(ESC"[%u;%u;%um", bold, color[fg].fg, color[bg].bg);
	else
		printf(ESC"[0;37;40m");

	return EFI_EXIT(EFI_SUCCESS);
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

struct efi_simple_text_output_protocol efi_con_out = {
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

/*
 * Analyze modifiers (shift, alt, ctrl) for function keys.
 * This gets called when we have already parsed CSI.
 *
 * @modifiers:  bitmask (shift, alt, ctrl)
 * @return:	the unmodified code
 */
static char skip_modifiers(int *modifiers)
{
	char c, mod = 0, ret = 0;

	c = getc();

	if (c != ';') {
		ret = c;
		if (c == '~')
			goto out;
		c = getc();
	}
	for (;;) {
		switch (c) {
		case '0'...'9':
			mod *= 10;
			mod += c - '0';
		/* fall through */
		case ';':
			c = getc();
			break;
		default:
			goto out;
		}
	}
out:
	if (mod)
		--mod;
	if (modifiers)
		*modifiers = mod;
	if (!ret)
		ret = c;
	return ret;
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
		/*
		 * Xterm Control Sequences
		 * https://www.xfree86.org/4.8.0/ctlseqs.html
		 */
		ch = getc();
		switch (ch) {
		case cESC: /* ESC */
			pressed_key.scan_code = 23;
			break;
		case 'O': /* F1 - F4 */
			ch = getc();
			/* skip modifiers */
			if (ch <= '9')
				ch = getc();
			pressed_key.scan_code = ch - 'P' + 11;
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
			case '1':
				ch = skip_modifiers(NULL);
				switch (ch) {
				case '1'...'5': /* F1 - F5 */
					pressed_key.scan_code = ch - '1' + 11;
					break;
				case '7'...'9': /* F6 - F8 */
					pressed_key.scan_code = ch - '7' + 16;
					break;
				case 'A'...'D': /* up, down right, left */
					pressed_key.scan_code = ch - 'A' + 1;
					break;
				case 'F':
					pressed_key.scan_code = 6; /* End */
					break;
				case 'H':
					pressed_key.scan_code = 5; /* Home */
					break;
				}
				break;
			case '2':
				ch = skip_modifiers(NULL);
				switch (ch) {
				case '0'...'1': /* F9 - F10 */
					pressed_key.scan_code = ch - '0' + 19;
					break;
				case '3'...'4': /* F11 - F12 */
					pressed_key.scan_code = ch - '3' + 21;
					break;
				case '~': /* INS */
					pressed_key.scan_code = 7;
					break;
				}
				break;
			case '3': /* DEL */
				pressed_key.scan_code = 8;
				skip_modifiers(NULL);
				break;
			case '5': /* PG UP */
				pressed_key.scan_code = 9;
				skip_modifiers(NULL);
				break;
			case '6': /* PG DOWN */
				pressed_key.scan_code = 10;
				skip_modifiers(NULL);
				break;
			}
			break;
		}
	} else if (ch == 0x7f) {
		/* Backspace */
		ch = 0x08;
	}
	if (!pressed_key.scan_code)
		pressed_key.unicode_char = ch;
	*key = pressed_key;

	return EFI_EXIT(EFI_SUCCESS);
}

struct efi_simple_input_interface efi_con_in = {
	.reset = efi_cin_reset,
	.read_key_stroke = efi_cin_read_key_stroke,
	.wait_for_key = NULL,
};

static struct efi_event *console_timer_event;

static void EFIAPI efi_key_notify(struct efi_event *event, void *context)
{
}

/*
 * Notification function of the console timer event.
 *
 * event:	console timer event
 * context:	not used
 */
static void EFIAPI efi_console_timer_notify(struct efi_event *event,
					    void *context)
{
	EFI_ENTRY("%p, %p", event, context);

	/* Check if input is available */
	if (tstc()) {
		/* Queue the wait for key event */
		efi_con_in.wait_for_key->is_signaled = true;
		efi_signal_event(efi_con_in.wait_for_key, true);
	}
	EFI_EXIT(EFI_SUCCESS);
}

/* This gets called from do_bootefi_exec(). */
int efi_console_register(void)
{
	efi_status_t r;
	struct efi_object *efi_console_output_obj;
	struct efi_object *efi_console_input_obj;

	/* Set up mode information */
	query_console_size();

	/* Create handles */
	r = efi_create_handle((efi_handle_t *)&efi_console_output_obj);
	if (r != EFI_SUCCESS)
		goto out_of_memory;
	r = efi_add_protocol(efi_console_output_obj->handle,
			     &efi_guid_text_output_protocol, &efi_con_out);
	if (r != EFI_SUCCESS)
		goto out_of_memory;
	r = efi_create_handle((efi_handle_t *)&efi_console_input_obj);
	if (r != EFI_SUCCESS)
		goto out_of_memory;
	r = efi_add_protocol(efi_console_input_obj->handle,
			     &efi_guid_text_input_protocol, &efi_con_in);
	if (r != EFI_SUCCESS)
		goto out_of_memory;

	/* Create console events */
	r = efi_create_event(EVT_NOTIFY_WAIT, TPL_CALLBACK, efi_key_notify,
			     NULL, NULL, &efi_con_in.wait_for_key);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to register WaitForKey event\n");
		return r;
	}
	r = efi_create_event(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
			     efi_console_timer_notify, NULL, NULL,
			     &console_timer_event);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to register console event\n");
		return r;
	}
	/* 5000 ns cycle is sufficient for 2 MBaud */
	r = efi_set_timer(console_timer_event, EFI_TIMER_PERIODIC, 50);
	if (r != EFI_SUCCESS)
		printf("ERROR: Failed to set console timer\n");
	return r;
out_of_memory:
	printf("ERROR: Out of meemory\n");
	return r;
}
