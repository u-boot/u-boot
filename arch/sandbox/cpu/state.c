/*
 * Copyright (c) 2011-2012 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/state.h>

/* Main state record for the sandbox */
static struct sandbox_state main_state;
static struct sandbox_state *state;	/* Pointer to current state record */

void state_record_exit(enum exit_type_id exit_type)
{
	state->exit_type = exit_type;
}

struct sandbox_state *state_get_current(void)
{
	assert(state);
	return state;
}

int state_init(void)
{
	state = &main_state;

	/*
	 * Example of how to use GPIOs:
	 *
	 * sandbox_gpio_set_direction(170, 0);
	 * sandbox_gpio_set_value(170, 0);
	 */
	return 0;
}
