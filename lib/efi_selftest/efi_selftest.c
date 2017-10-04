/*
 * EFI efi_selftest
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <efi_selftest.h>
#include <vsprintf.h>

static const struct efi_system_table *systable;
static const struct efi_boot_services *boottime;
static const struct efi_runtime_services *runtime;
static efi_handle_t handle;
static u16 reset_message[] = L"Selftest completed";

/*
 * Exit the boot services.
 *
 * The size of the memory map is determined.
 * Pool memory is allocated to copy the memory map.
 * The memory amp is copied and the map key is obtained.
 * The map key is used to exit the boot services.
 */
void efi_st_exit_boot_services(void)
{
	unsigned long  map_size = 0;
	unsigned long  map_key;
	unsigned long desc_size;
	u32 desc_version;
	efi_status_t ret;
	struct efi_mem_desc *memory_map;

	ret = boottime->get_memory_map(&map_size, NULL, &map_key, &desc_size,
				       &desc_version);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error(
			"GetMemoryMap did not return EFI_BUFFER_TOO_SMALL\n");
		return;
	}
	/* Allocate extra space for newly allocated memory */
	map_size += sizeof(struct efi_mem_desc);
	ret = boottime->allocate_pool(EFI_BOOT_SERVICES_DATA, map_size,
				      (void **)&memory_map);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool did not return EFI_SUCCESS\n");
		return;
	}
	ret = boottime->get_memory_map(&map_size, memory_map, &map_key,
				       &desc_size, &desc_version);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetMemoryMap did not return EFI_SUCCESS\n");
		return;
	}
	ret = boottime->exit_boot_services(handle, map_key);
	if (ret != EFI_SUCCESS) {
		efi_st_error("ExitBootServices did not return EFI_SUCCESS\n");
		return;
	}
	efi_st_printf("\nBoot services terminated\n");
}

/*
 * Set up a test.
 *
 * @test	the test to be executed
 * @failures	counter that will be incremented if a failure occurs
 * @return	EFI_ST_SUCCESS for success
 */
static int setup(struct efi_unit_test *test, unsigned int *failures)
{
	int ret;

	if (!test->setup)
		return EFI_ST_SUCCESS;
	efi_st_printf("\nSetting up '%s'\n", test->name);
	ret = test->setup(handle, systable);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("Setting up '%s' failed\n", test->name);
		++*failures;
	} else {
		efi_st_printf("Setting up '%s' succeeded\n", test->name);
	}
	return ret;
}

/*
 * Execute a test.
 *
 * @test	the test to be executed
 * @failures	counter that will be incremented if a failure occurs
 * @return	EFI_ST_SUCCESS for success
 */
static int execute(struct efi_unit_test *test, unsigned int *failures)
{
	int ret;

	if (!test->execute)
		return EFI_ST_SUCCESS;
	efi_st_printf("\nExecuting '%s'\n", test->name);
	ret = test->execute();
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("Executing '%s' failed\n", test->name);
		++*failures;
	} else {
		efi_st_printf("Executing '%s' succeeded\n", test->name);
	}
	return ret;
}

/*
 * Tear down a test.
 *
 * @test	the test to be torn down
 * @failures	counter that will be incremented if a failure occurs
 * @return	EFI_ST_SUCCESS for success
 */
static int teardown(struct efi_unit_test *test, unsigned int *failures)
{
	int ret;

	if (!test->teardown)
		return EFI_ST_SUCCESS;
	efi_st_printf("\nTearing down '%s'\n", test->name);
	ret = test->teardown();
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("Tearing down '%s' failed\n", test->name);
		++*failures;
	} else {
		efi_st_printf("Tearing down '%s' succeeded\n", test->name);
	}
	return ret;
}

/*
 * Execute selftest of the EFI API
 *
 * This is the main entry point of the EFI selftest application.
 *
 * All tests use a driver model and are run in three phases:
 * setup, execute, teardown.
 *
 * A test may be setup and executed at boottime,
 * it may be setup at boottime and executed at runtime,
 * or it may be setup and executed at runtime.
 *
 * After executing all tests the system is reset.
 *
 * @image_handle:	handle of the loaded EFI image
 * @systab:		EFI system table
 */
efi_status_t EFIAPI efi_selftest(efi_handle_t image_handle,
				 struct efi_system_table *systab)
{
	struct efi_unit_test *test;
	unsigned int failures = 0;

	systable = systab;
	boottime = systable->boottime;
	runtime = systable->runtime;
	handle = image_handle;
	con_out = systable->con_out;
	con_in = systable->con_in;

	efi_st_printf("\nTesting EFI API implementation\n");

	efi_st_printf("\nNumber of tests to execute: %u\n",
		      ll_entry_count(struct efi_unit_test, efi_unit_test));

	/* Execute boottime tests */
	for (test = ll_entry_start(struct efi_unit_test, efi_unit_test);
	     test < ll_entry_end(struct efi_unit_test, efi_unit_test); ++test) {
		if (test->phase == EFI_EXECUTE_BEFORE_BOOTTIME_EXIT) {
			setup(test, &failures);
			execute(test, &failures);
			teardown(test, &failures);
		}
	}

	/* Execute mixed tests */
	for (test = ll_entry_start(struct efi_unit_test, efi_unit_test);
	     test < ll_entry_end(struct efi_unit_test, efi_unit_test); ++test) {
		if (test->phase == EFI_SETUP_BEFORE_BOOTTIME_EXIT)
			setup(test, &failures);
	}

	efi_st_exit_boot_services();

	for (test = ll_entry_start(struct efi_unit_test, efi_unit_test);
	     test < ll_entry_end(struct efi_unit_test, efi_unit_test); ++test) {
		if (test->phase == EFI_SETUP_BEFORE_BOOTTIME_EXIT) {
			execute(test, &failures);
			teardown(test, &failures);
		}
	}

	/* Execute runtime tests */
	for (test = ll_entry_start(struct efi_unit_test, efi_unit_test);
	     test < ll_entry_end(struct efi_unit_test, efi_unit_test); ++test) {
		if (test->phase == EFI_SETUP_AFTER_BOOTTIME_EXIT) {
			setup(test, &failures);
			execute(test, &failures);
			teardown(test, &failures);
		}
	}

	/* Give feedback */
	efi_st_printf("\nSummary: %u failures\n\n", failures);

	/* Reset system */
	efi_st_printf("Preparing for reset. Press any key.\n");
	efi_st_get_key();
	runtime->reset_system(EFI_RESET_WARM, EFI_NOT_READY,
			      sizeof(reset_message), reset_message);
	efi_st_printf("\n");
	efi_st_error("Reset failed.\n");

	return EFI_UNSUPPORTED;
}
