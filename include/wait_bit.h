/*
 * Wait for bit with timeout and ctrlc
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __WAIT_BIT_H
#define __WAIT_BIT_H

#include <common.h>
#include <console.h>
#include <asm/errno.h>
#include <asm/io.h>

/**
 * wait_for_bit()	waits for bit set/cleared in register
 *
 * Function polls register waiting for specific bit(s) change
 * (either 0->1 or 1->0). It can fail under two conditions:
 * - Timeout
 * - User interaction (CTRL-C)
 * Function succeeds only if all bits of masked register are set/cleared
 * (depending on set option).
 *
 * @param prefix	Prefix added to timeout messagge (message visible only
 *			with debug enabled)
 * @param reg		Register that will be read (using readl())
 * @param mask		Bit(s) of register that must be active
 * @param set		Selects wait condition (bit set or clear)
 * @param timeout_ms	Timeout (in miliseconds)
 * @param breakable	Enables CTRL-C interruption
 * @return		0 on success, -ETIMEDOUT or -EINTR on failure
 */
static inline int wait_for_bit(const char *prefix, const u32 *reg,
			       const u32 mask, const bool set,
			       const unsigned int timeout_ms,
			       const bool breakable)
{
	u32 val;
	unsigned long start = get_timer(0);

	while (1) {
		val = readl(reg);

		if (!set)
			val = ~val;

		if ((val & mask) == mask)
			return 0;

		if (get_timer(start) > timeout_ms)
			break;

		if (breakable && ctrlc()) {
			puts("Abort\n");
			return -EINTR;
		}

		udelay(1);
	}

	debug("%s: Timeout (reg=%p mask=%08x wait_set=%i)\n", prefix, reg, mask,
	      set);

	return -ETIMEDOUT;
}


#endif
