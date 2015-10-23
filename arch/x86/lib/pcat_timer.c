/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/i8254.h>

#define TIMER2_VALUE	0x0a8e	/* 440Hz */

int pcat_timer_init(void)
{
	/*
	 * Initialize counter 2, used to drive the speaker.
	 * To start a beep, set both bit0 and bit1 of port 0x61.
	 * To stop it, clear both bit0 and bit1 of port 0x61.
	 */
	outb(PIT_CMD_CTR2 | PIT_CMD_BOTH | PIT_CMD_MODE3,
	     PIT_BASE + PIT_COMMAND);
	outb(TIMER2_VALUE & 0xff, PIT_BASE + PIT_T2);
	outb(TIMER2_VALUE >> 8, PIT_BASE + PIT_T2);

	return 0;
}
