/*
 * functions for handling OS log buffer
 *
 * Copyright (c) 2009 Analog Devices Inc.
 *
 * Licensed under the 2-clause BSD.
 */

#include <common.h>

#define OS_LOG_MAGIC       0xDEADBEEF
#define OS_LOG_MAGIC_ADDR  ((unsigned long *)0x4f0)
#define OS_LOG_PTR_ADDR    ((char **)0x4f4)

bool bfin_os_log_check(void)
{
	if (*OS_LOG_MAGIC_ADDR != OS_LOG_MAGIC)
		return false;
	*OS_LOG_MAGIC_ADDR = 0;
	return true;
}

void bfin_os_log_dump(void)
{
	char *log = *OS_LOG_PTR_ADDR;
	while (*log) {
		puts(log);
		log += strlen(log) + 1;
	}
}
