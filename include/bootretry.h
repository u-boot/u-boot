/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __bootretry_h
#define __bootretry_h

#ifdef CONFIG_BOOT_RETRY_TIME
/**
 * bootretry_tstc_timeout() - ensure we get a keypress before timeout
 *
 * Check for a keypress repeatedly, resetting the watchdog each time. If a
 * keypress is not received within the command timeout, return an error.
 *
 * @return 0 if a key is received in time, -ETIMEDOUT if not
 */
int bootretry_tstc_timeout(void);
#else
static inline int bootretry_tstc_timeout(void)
{
	return 0;
}
#endif

void init_cmd_timeout(void);
void reset_cmd_timeout(void);

#endif
