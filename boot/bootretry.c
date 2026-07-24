// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <stdio.h>
#include <bootretry.h>
#include <cli.h>
#include <env.h>
#include <env_callback.h>
#include <errno.h>
#include <time.h>
#include <vsprintf.h>
#include <watchdog.h>

static uint64_t endtime;  /* must be set, default is instant timeout */
static int      retry_time = -1; /* -1 so can call readline before main_loop */

/***************************************************************************
 * initialize command line timeout
 */

static void bootretry_parse(const char *s)
{
	if (s != NULL)
		retry_time = (int)simple_strtol(s, NULL, 10);
	else
		retry_time = CONFIG_BOOT_RETRY_TIME;

	if (retry_time >= 0 && retry_time < CONFIG_BOOT_RETRY_MIN)
		retry_time = CONFIG_BOOT_RETRY_MIN;
}

void bootretry_init_cmd_timeout(void)
{
	bootretry_parse(env_get("bootretry"));
}

/* Parse changes to bootretry */
static int on_bootretry(const char *name, const char *value, enum env_op op,
			int flags)
{
	switch (op) {
	case env_op_create:
	case env_op_overwrite:
	case env_op_delete:
		bootretry_parse(value);
		break;
	}
	return 0;
}
U_BOOT_ENV_CALLBACK(bootretry, on_bootretry);

/***************************************************************************
 * reset command line timeout to retry_time seconds
 */
void bootretry_reset_cmd_timeout(void)
{
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
