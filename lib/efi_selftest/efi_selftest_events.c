/*
 * efi_selftest_events
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * This unit test uses timer events to check the implementation
 * of the following boottime services:
 * CreateEvent, CloseEvent, WaitForEvent, CheckEvent, SetTimer.
 */

#include <efi_selftest.h>

static struct efi_event *event_notify;
static struct efi_event *event_wait;
static unsigned int counter;
static struct efi_boot_services *boottime;

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
 * Create two timer events.
 * One with EVT_NOTIFY_SIGNAL, the other with EVT_NOTIFY_WAIT.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	boottime = systable->boottime;

	ret = boottime->create_event(EVT_TIMER | EVT_NOTIFY_SIGNAL,
				     TPL_CALLBACK, notify, (void *)&counter,
				     &event_notify);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return 1;
	}
	ret = boottime->create_event(EVT_TIMER | EVT_NOTIFY_WAIT,
				     TPL_CALLBACK, notify, NULL, &event_wait);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return 1;
	}
	return 0;
}

/*
 * Tear down unit test.
 *
 * Close the events created in setup.
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
	if (event_wait) {
		ret = boottime->close_event(event_wait);
		event_wait = NULL;
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
 * Run a 10 ms periodic timer and check that it is called 10 times
 * while waiting for 100 ms single shot timer.
 *
 * Run a 100 ms single shot timer and check that it is called once
 * while waiting for 100 ms periodic timer for two periods.
 */
static int execute(void)
{
	unsigned long index;
	efi_status_t ret;

	/* Set 10 ms timer */
	counter = 0;
	ret = boottime->set_timer(event_notify, EFI_TIMER_PERIODIC, 100000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return 1;
	}
	/* Set 100 ms timer */
	ret = boottime->set_timer(event_wait, EFI_TIMER_RELATIVE, 1000000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return 1;
	}

	index = 5;
	ret = boottime->wait_for_event(1, &event_wait, &index);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not wait for event\n");
		return 1;
	}
	ret = boottime->check_event(event_wait);
	if (ret != EFI_NOT_READY) {
		efi_st_error("Signaled state was not cleared.\n");
		efi_st_printf("ret = %u\n", (unsigned int)ret);
		return 1;
	}
	if (index != 0) {
		efi_st_error("WaitForEvent returned wrong index\n");
		return 1;
	}
	efi_st_printf("Counter periodic: %u\n", counter);
	if (counter < 8 || counter > 12) {
		efi_st_error("Incorrect timing of events\n");
		return 1;
	}
	ret = boottime->set_timer(event_notify, EFI_TIMER_STOP, 0);
	if (index != 0) {
		efi_st_error("Could not cancel timer\n");
		return 1;
	}
	/* Set 10 ms timer */
	counter = 0;
	ret = boottime->set_timer(event_notify, EFI_TIMER_RELATIVE, 100000);
	if (index != 0) {
		efi_st_error("Could not set timer\n");
		return 1;
	}
	/* Set 100 ms timer */
	ret = boottime->set_timer(event_wait, EFI_TIMER_PERIODIC, 1000000);
	if (index != 0) {
		efi_st_error("Could not set timer\n");
		return 1;
	}
	ret = boottime->wait_for_event(1, &event_wait, &index);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not wait for event\n");
		return 1;
	}
	efi_st_printf("Counter single shot: %u\n", counter);
	if (counter != 1) {
		efi_st_error("Single shot timer failed\n");
		return 1;
	}
	ret = boottime->wait_for_event(1, &event_wait, &index);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not wait for event\n");
		return 1;
	}
	efi_st_printf("Stopped counter: %u\n", counter);
	if (counter != 1) {
		efi_st_error("Stopped timer fired\n");
		return 1;
	}
	ret = boottime->set_timer(event_wait, EFI_TIMER_STOP, 0);
	if (index != 0) {
		efi_st_error("Could not cancel timer\n");
		return 1;
	}

	return 0;
}

EFI_UNIT_TEST(events) = {
	.name = "event services",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
