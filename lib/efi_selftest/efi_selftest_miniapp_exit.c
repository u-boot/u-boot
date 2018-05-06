// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_miniapp_exit
 *
 * Copyright (c) 2018 Heinrich Schuchardt
 *
 * This EFI application is run by the StartImage selftest.
 * It uses the Exit boot service to return.
 */

#include <common.h>
#include <efi_api.h>

/*
 * Entry point of the EFI application.
 *
 * @handle	handle of the loaded image
 * @systable	system table
 * @return	status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t handle,
			     struct efi_system_table *systable)
{
	struct efi_simple_text_output_protocol *con_out = systable->con_out;

	con_out->output_string(con_out, L"EFI application calling Exit\n");

	/* The return value is checked by the calling test */
	systable->boottime->exit(handle, EFI_UNSUPPORTED, 0, NULL);

	/*
	 * This statement should not be reached.
	 * To enable testing use a different return value.
	 */
	return EFI_SUCCESS;
}
