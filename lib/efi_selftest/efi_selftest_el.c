// SPDX-License-Identifier: GPL-2.0-only
/*
 *  Check current exception level on ARMv8.
 */
#include <efi_loader.h>
#include <efi_selftest.h>

/**
 * current_exception_level()
 *
 * Return:	current exception level, 0 - 3
 */
static unsigned int current_exception_level(void)
{
	unsigned long el;

	asm volatile (
		"MRS %0, CurrentEL"
		: "=r" (el) : : );

	return (el >> 2) & 0x3;
}

/**
 * execute() - execute test
 *
 * Check that the exception level is not EL3.
 */
static int execute(void)
{
	unsigned int el = current_exception_level();

	efi_st_printf("Exception level EL%u\n", el);
	if (el != 1 && el != 2) {
		efi_st_error("EL1 or EL2 expected");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(el) = {
	.name = "exception level",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.execute = execute,
};
