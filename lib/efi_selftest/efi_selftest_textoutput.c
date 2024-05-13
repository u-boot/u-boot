// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_textoutput
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test the EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.
 *
 * The following services are tested:
 * OutputString, TestString, SetAttribute.
 */

#include <efi_selftest.h>

/*
 * Execute unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	size_t foreground;
	size_t background;
	size_t attrib;
	efi_status_t ret;
	s16 col;
	u16 cr[] = { 0x0d, 0x00 };
	u16 lf[] = { 0x0a, 0x00 };
	u16 brahmi[] = { /* 2 Brahmi letters */
		0xD804, 0xDC05,
		0xD804, 0xDC22,
		0};

	const u16 text[] =
		u"This should render international characters as described\n"
		u"U+00D6 \u00D6 - Latin capital letter O with diaresis\n"
		u"U+00DF \u00DF - Latin small letter sharp s\n"
		u"U+00E5 \u00E5 - Latin small letter a with ring above\n"
		u"U+00E9 \u00E9 - Latin small letter e with acute\n"
		u"U+00F1 \u00F1 - Latin small letter n with tilde\n"
		u"U+00F6 \u00F6 - Latin small letter o with diaresis\n"
		u"The following characters will render as '?' with bitmap fonts\n"
		u"U+00F8 \u00F8 - Latin small letter o with stroke\n"
		u"U+03AC \u03AC - Greek small letter alpha with tonus\n"
		u"U+03BB \u03BB - Greek small letter lambda\n"
		u"U+03C2 \u03C2 - Greek small letter final sigma\n"
		u"U+1F19 \u1F19 - Greek capital letter epsilon with dasia\n";

	const u16 boxes[] =
		u"This should render as four boxes with text\n"
		u"\u250c\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
		u"\u2500\u2500\u2500\u252c\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
		u"\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2510\n\u2502"
		u" left top    \u2502 right top     \u2502\n\u251c\u2500"
		u"\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
		u"\u2500\u253c\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
		u"\u2500\u2500\u2500\u2500\u2500\u2500\u2524\n\u2502 "
		u"left bottom \u2502 right bottom  \u2502\n\u2514\u2500\u2500\u2500"
		u"\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2534"
		u"\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
		u"\u2500\u2500\u2500\u2500\u2518\n";

	const u16 shapes[] =
		u"Geometric shapes as described\n"
		u"U+25B2 \u25B2 - Black up-pointing triangle\n"
		u"U+25BA \u25BA - Black right-pointing pointer\n"
		u"U+25BC \u25BC - Black down-pointing triangle\n"
		u"U+25C4 \u25C4 - Black left-pointing pointer\n";

	/* SetAttribute */
	efi_st_printf("\nColor palette\n");
	for (foreground = 0; foreground < 0x10; ++foreground) {
		for (background = 0; background < 0x80; background += 0x10) {
			attrib = foreground | background;
			con_out->set_attribute(con_out, attrib);
			efi_st_printf("%p", (void *)attrib);
		}
		con_out->set_attribute(con_out, 0);
		efi_st_printf("\n");
	}
	/* TestString */
	ret = con_out->test_string(con_out,
			u" !\"#$%&'()*+,-./0-9:;<=>?@A-Z[\\]^_`a-z{|}~\n");
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("TestString failed for ANSI characters\n");
		return EFI_ST_FAILURE;
	}
	/* OutputString */
	ret = con_out->output_string(con_out,
				     u"Testing cursor column update\n");
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("OutputString failed for ANSI characters");
		return EFI_ST_FAILURE;
	}
	col = con_out->mode->cursor_column;
	ret = con_out->output_string(con_out, lf);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("OutputString failed for line feed\n");
		return EFI_ST_FAILURE;
	}
	if (con_out->mode->cursor_column != col) {
		efi_st_error("Cursor column changed by line feed\n");
		return EFI_ST_FAILURE;
	}
	ret = con_out->output_string(con_out, cr);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("OutputString failed for carriage return\n");
		return EFI_ST_FAILURE;
	}
	if (con_out->mode->cursor_column) {
		efi_st_error("Cursor column not 0 at beginning of line\n");
		return EFI_ST_FAILURE;
	}
	ret = con_out->output_string(con_out, u"123");
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("OutputString failed for ANSI characters\n");
		return EFI_ST_FAILURE;
	}
	if (con_out->mode->cursor_column != 3) {
		efi_st_error("Cursor column not incremented properly\n");
		return EFI_ST_FAILURE;
	}
	ret = con_out->output_string(con_out, u"\b");
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("OutputString failed for backspace\n");
		return EFI_ST_FAILURE;
	}
	if (con_out->mode->cursor_column != 2) {
		efi_st_error("Cursor column not decremented properly\n");
		return EFI_ST_FAILURE;
	}
	ret = con_out->output_string(con_out, u"\b\b");
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("OutputString failed for backspace\n");
		return EFI_ST_FAILURE;
	}
	if (con_out->mode->cursor_column) {
		efi_st_error("Cursor column not decremented properly\n");
		return EFI_ST_FAILURE;
	}
	ret = con_out->output_string(con_out, u"\b\b");
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("OutputString failed for backspace\n");
		return EFI_ST_FAILURE;
	}
	if (con_out->mode->cursor_column) {
		efi_st_error("Cursor column decremented past zero\n");
		return EFI_ST_FAILURE;
	}
	ret = con_out->output_string(con_out, brahmi);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_todo("Unicode output not fully supported\n");
	} else if (con_out->mode->cursor_column != 2) {
		efi_st_printf("Unicode not handled properly\n");
		return EFI_ST_FAILURE;
	}
	efi_st_printf("\n");
	ret = con_out->output_string(con_out, text);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("OutputString failed for international chars\n");
		return EFI_ST_FAILURE;
	}
	efi_st_printf("\n");
	ret = con_out->output_string(con_out, boxes);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("OutputString failed for box drawing chars\n");
		return EFI_ST_FAILURE;
	}
	efi_st_printf("\n");
	ret = con_out->output_string(con_out, shapes);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("OutputString failed for geometric shapes\n");
		return EFI_ST_FAILURE;
	}
	efi_st_printf("\n");

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(textoutput) = {
	.name = "text output",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.execute = execute,
};
