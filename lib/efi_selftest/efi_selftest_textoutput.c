/*
 * efi_selftest_textoutput
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * SPDX-License-Identifier:     GPL-2.0+
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
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	size_t foreground;
	size_t background;
	size_t attrib;
	efi_status_t ret;

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
			L" !\"#$%&'()*+,-./0-9:;<=>?@A-Z[\\]^_`a-z{|}~\n");
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("TestString failed for ANSI characters\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(textoutput) = {
	.name = "text output",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.execute = execute,
};
