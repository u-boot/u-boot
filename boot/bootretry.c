// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <stdio.h>
#include <bootretry.h>
#include <cli.h>
#include <env.h>
#include <errno.h>
#include <time.h>
#include <vsprintf.h>
#include <watchdog.h>

static uint64_t endtime;  /* must be set, default is instant timeout */
static int      retry_time = -1; /* -1 so can call readline before main_loop */

/***************************************************************************
 * initialize command line timeout
 */
void bootretry_init_cmd_timeout(void)
{
	char *s = env_get("bootretry");

	if (s != NULL)
		retry_time = (int)simple_strtol(s, NULL, 10);
	else
		retry_time = CONFIG_BOOT_RETRY_TIME;

	if (retry_time >= 0 && retry_time < CONFIG_BOOT_RETRY_MIN)
		retry_time = CONFIG_BOOT_RETRY_MIN;
}

/***************************************************************************
 * reset command line timeout to retry_time seconds
 */
void bootretry_reset_cmd_timeout(void)
{
	/* Parse changes to bootretry */
	bootretry_init_cmd_timeout();
	endtime = endtick(retry_time);
}

int bootretry_tstc_timeout(void)
{
	while (!tstc()) {	/* while no incoming data */
		if (retry_time >= 0 && get_ticks() > endtime)
			return -ETIMEDOUT;
		schedule();
	}

	return 0;
}

void bootretry_dont_retry(void)
{
	retry_time = -1;
}
