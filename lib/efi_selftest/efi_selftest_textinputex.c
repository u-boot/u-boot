// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_textinput
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Provides a unit test for the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL.
 * The unicode character and the scan code are printed for text
 * input. To run the test:
 *
 *	setenv efi_selftest extended text input
 *	bootefi selftest
 */

#include <efi_selftest.h>

static const efi_guid_t text_input_ex_protocol_guid =
		EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID;

static struct efi_simple_text_input_ex_protocol *con_in_ex;

static struct efi_boot_services *boottime;

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
	efi_status_t ret;

	boottime = systable->boottime;

	ret = boottime->locate_protocol(&text_input_ex_protocol_guid, NULL,
					(void **)&con_in_ex);
	if (ret != EFI_SUCCESS) {
		con_in_ex = NULL;
		efi_st_error
			("Extended text input protocol is not available.\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	struct efi_key_data input_key = {0,};
	efi_status_t ret;
	efi_uintn_t index;

	if (!con_in_ex) {
		efi_st_printf("Setup failed\n");
		return EFI_ST_FAILURE;
	}

	/* Drain the console input */
	ret = con_in_ex->reset(con_in_ex, true);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Reset failed\n");
		return EFI_ST_FAILURE;
	}
	ret = con_in_ex->read_key_stroke_ex(con_in_ex, &input_key);
	if (ret != EFI_NOT_READY) {
		efi_st_error("Empty buffer not reported\n");
		return EFI_ST_FAILURE;
	}

	efi_st_printf("Waiting for your input\n");
	efi_st_printf("To terminate type 'x'\n");

	for (;;) {
		/* Wait for next key */
		ret = boottime->wait_for_event(1, &con_in_ex->wait_for_key_ex,
					       &index);
		if (ret != EFI_ST_SUCCESS) {
			efi_st_error("WaitForEvent failed\n");
			return EFI_ST_FAILURE;
		}
		ret = con_in_ex->read_key_stroke_ex(con_in_ex, &input_key);
		if (ret != EFI_SUCCESS) {
			efi_st_error("ReadKeyStroke failed\n");
			return EFI_ST_FAILURE;
		}

		/* Allow 5 minutes until time out */
		boottime->set_watchdog_timer(300, 0, 0, NULL);

		efi_st_printf("Unicode char %u (%ps), scan code %u (",
			      (unsigned int)input_key.key.unicode_char,
			      efi_st_translate_char(input_key.key.unicode_char),
			      (unsigned int)input_key.key.scan_code);
		if (input_key.key_state.key_shift_state &
		    EFI_SHIFT_STATE_VALID) {
			if (input_key.key_state.key_shift_state &
			    (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED))
				efi_st_printf("SHIFT+");
			if (input_key.key_state.key_shift_state &
			    (EFI_LEFT_ALT_PRESSED | EFI_RIGHT_ALT_PRESSED))
				efi_st_printf("ALT+");
			if (input_key.key_state.key_shift_state &
			    (EFI_LEFT_CONTROL_PRESSED |
			     EFI_RIGHT_CONTROL_PRESSED))
				efi_st_printf("CTRL+");
			if (input_key.key_state.key_shift_state &
			    (EFI_LEFT_LOGO_PRESSED | EFI_RIGHT_LOGO_PRESSED))
				efi_st_printf("META+");
			if (input_key.key_state.key_shift_state ==
			    EFI_SHIFT_STATE_VALID)
				efi_st_printf("+");
		}

		efi_st_printf("%ps)\n",
			      efi_st_translate_code(input_key.key.scan_code));

		switch (input_key.key.unicode_char) {
		case 'x':
		case 'X':
			return EFI_ST_SUCCESS;
		}
	}
}

EFI_UNIT_TEST(textinputex) = {
	.name = "extended text input",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.on_request = true,
};
