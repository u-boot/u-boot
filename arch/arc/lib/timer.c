/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arcregs.h>

#define NH_MODE	(1 << 1)	/* Disable timer if CPU is halted */

int timer_init(void)
{
	write_aux_reg(ARC_AUX_TIMER0_CTRL, NH_MODE);
	/* Set max value for counter/timer */
	write_aux_reg(ARC_AUX_TIMER0_LIMIT, 0xffffffff);
	/* Set initial count value and restart counter/timer */
	write_aux_reg(ARC_AUX_TIMER0_CNT, 0);
	return 0;
}

unsigned long timer_read_counter(void)
{
	return read_aux_reg(ARC_AUX_TIMER0_CNT);
}
