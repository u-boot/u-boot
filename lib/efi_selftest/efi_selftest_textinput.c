// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_textinput
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Provides a unit test for the EFI_SIMPLE_TEXT_INPUT_PROTOCOL.
 * The unicode character and the scan code are printed for text
 * input. To run the test:
 *
 *	setenv efi_selftest text input
 *	bootefi selftest
 */

#include <efi_selftest.h>

struct translate {
	u16 code;
	u16 *text;
};

static struct efi_boot_services *boottime;

static struct translate control_characters[] = {
	{0, L"Null"},
	{8, L"BS"},
	{9, L"TAB"},
	{10, L"LF"},
	{13, L"CR"},
	{0, NULL},
};

static u16 ch[] = L"' '";
static u16 unknown[] = L"unknown";

static struct translate scan_codes[] = {
	{0x00, L"Null"},
	{0x01, L"Up"},
	{0x02, L"Down"},
	{0x03, L"Right"},
	{0x04, L"Left"},
	{0x05, L"Home"},
	{0x06, L"End"},
	{0x07, L"Insert"},
	{0x08, L"Delete"},
	{0x09, L"Page Up"},
	{0x0a, L"Page Down"},
	{0x0b, L"FN 1"},
	{0x0c, L"FN 2"},
	{0x0d, L"FN 3"},
	{0x0e, L"FN 4"},
	{0x0f, L"FN 5"},
	{0x10, L"FN 6"},
	{0x11, L"FN 7"},
	{0x12, L"FN 8"},
	{0x13, L"FN 9"},
	{0x14, L"FN 10"},
	{0x15, L"FN 11"},
	{0x16, L"FN 12"},
	{0x17, L"Escape"},
	{0x68, L"FN 13"},
	{0x69, L"FN 14"},
	{0x6a, L"FN 15"},
	{0x6b, L"FN 16"},
	{0x6c, L"FN 17"},
	{0x6d, L"FN 18"},
	{0x6e, L"FN 19"},
	{0x6f, L"FN 20"},
	{0x70, L"FN 21"},
	{0x71, L"FN 22"},
	{0x72, L"FN 23"},
	{0x73, L"FN 24"},
	{0x7f, L"Mute"},
	{0x80, L"Volume Up"},
	{0x81, L"Volume Down"},
	{0x100, L"Brightness Up"},
	{0x101, L"Brightness Down"},
	{0x102, L"Suspend"},
	{0x103, L"Hibernate"},
	{0x104, L"Toggle Display"},
	{0x105, L"Recovery"},
	{0x106, L"Reject"},
	{0x0, NULL},
};

/*
 * Translate a unicode character to a string.
 *
 * @code	unicode character
 * @return	string
 */
static u16 *translate_char(u16 code)
{
	struct translate *tr;

	if (code >= ' ') {
		ch[1] = code;
		return ch;
	}
	for (tr = control_characters; tr->text; ++tr) {
		if (tr->code == code)
			return tr->text;
	}
	return unknown;
}

/*
 * Translate a scan code to a human readable string.
 *
 * @code	unicode character
 * @return	string
 */
static u16 *translate_code(u16 code)
{
	struct translate *tr;

	for (tr = scan_codes; tr->text; ++tr) {
		if (tr->code == code)
			return tr->text;
	}
	return unknown;
}

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	boottime = systable->boottime;

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	struct efi_input_key input_key = {0};
	efi_status_t ret;

	efi_st_printf("Waiting for your input\n");
	efi_st_printf("To terminate type 'x'\n");

	for (;;) {
		/* Wait for next key */
		do {
			ret = con_in->read_key_stroke(con_in, &input_key);
		} while (ret == EFI_NOT_READY);

		/* Allow 5 minutes until time out */
		boottime->set_watchdog_timer(300, 0, 0, NULL);

		efi_st_printf("Unicode char %u (%ps), scan code %u (%ps)\n",
			      (unsigned int)input_key.unicode_char,
			      translate_char(input_key.unicode_char),
			      (unsigned int)input_key.scan_code,
			      translate_code(input_key.scan_code));

		switch (input_key.unicode_char) {
		case 'x':
		case 'X':
			return EFI_ST_SUCCESS;
		}
	}
}

EFI_UNIT_TEST(textinput) = {
	.name = "text input",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.on_request = true,
};
