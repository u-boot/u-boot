// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * efi_selftest_net_device
 *
 */

#include <efi_selftest.h>
#include <net.h>

static struct efi_boot_services *boottime;

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_uintn_t num_handles;
	efi_handle_t *handles;

	boottime = systable->boottime;

	num_handles = 0;
	boottime->locate_handle_buffer(BY_PROTOCOL, &efi_net_guid,
				       NULL, &num_handles, &handles);
	efi_st_printf("Detected %u active EFI net devices\n", (unsigned int)num_handles);

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	int exit_status = EFI_ST_SUCCESS;

	return exit_status;
}

EFI_UNIT_TEST(netdevices) = {
	.name = "netdevices",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
	.on_request = true,
};
