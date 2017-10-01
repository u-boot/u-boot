/*
 * efi_selftest_events
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * This unit test checks that the notification function of an
 * EVT_SIGNAL_EXIT_BOOT_SERVICES event is called exactly once.
 */

#include <efi_selftest.h>

static struct efi_boot_services *boottime;
static struct efi_event *event_notify;
static unsigned int counter;

/*
 * Notification function, increments a counter.
 *
 * @event	notified event
 * @context	pointer to the counter
 */
static void EFIAPI notify(struct efi_event *event, void *context)
{
	if (!context)
		return;
	++*(unsigned int *)context;
}

/*
 * Setup unit test.
 *
 * Create an EVT_SIGNAL_EXIT_BOOT_SERVICES event.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	boottime = systable->boottime;

	counter = 0;
	ret = boottime->create_event(EVT_SIGNAL_EXIT_BOOT_SERVICES,
				     TPL_CALLBACK, notify, (void *)&counter,
				     &event_notify);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return 1;
	}
	return 0;
}

/*
 * Tear down unit test.
 *
 * Close the event created in setup.
 */
static int teardown(void)
{
	efi_status_t ret;

	if (event_notify) {
		ret = boottime->close_event(event_notify);
		event_notify = NULL;
		if (ret != EFI_SUCCESS) {
			efi_st_error("could not close event\n");
			return 1;
		}
	}
	return 0;
}

/*
 * Execute unit test.
 *
 * Check that the notification function of the EVT_SIGNAL_EXIT_BOOT_SERVICES
 * event has been called.
 *
 * Call ExitBootServices again and check that the notification function is
 * not called again.
 */
static int execute(void)
{
	if (counter != 1) {
		efi_st_error("ExitBootServices was not notified");
		return 1;
	}
	efi_st_exit_boot_services();
	if (counter != 1) {
		efi_st_error("ExitBootServices was notified twice");
		return 1;
	}
	return 0;
}

EFI_UNIT_TEST(exitbootservices) = {
	.name = "ExitBootServices",
	.phase = EFI_SETUP_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
