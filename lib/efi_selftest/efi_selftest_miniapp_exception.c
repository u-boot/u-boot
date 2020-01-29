// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_miniapp_return
 *
 * Copyright (c) 2019 Heinrich Schuchardt
 *
 * This EFI application triggers an exception.
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

	con_out->output_string(con_out,
			       L"EFI application triggers exception.\n");

#if defined(CONFIG_ARM)
	/*
	 * 0xe7f...f.	is undefined in ARM mode
	 * 0xde..	is undefined in Thumb mode
	 */
	asm volatile (".word 0xe7f7defb\n");
#elif defined(CONFIG_RISCV)
	asm volatile (".word 0xffffffff\n");
#elif defined(CONFIG_X86)
	asm volatile (".word 0xffff\n");
#endif
	con_out->output_string(con_out, L"Exception not triggered.\n");
	return EFI_ABORTED;
}
