/*
 * U-boot - bootm.c - misc boot helper functions
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <asm/blackfin.h>

#ifdef SHARED_RESOURCES
extern void swap_to(int device_id);
#endif

static char *make_command_line(void)
{
	char *dest = (char *)CMD_LINE_ADDR;
	char *bootargs = getenv("bootargs");

	if (bootargs == NULL)
		return NULL;

	strncpy(dest, bootargs, 0x1000);
	dest[0xfff] = 0;
	return dest;
}

int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	int	(*appl) (char *cmdline);
	char	*cmdline;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

#ifdef SHARED_RESOURCES
	swap_to(FLASH);
#endif

	appl = (int (*)(char *))images->ep;

	printf("Starting Kernel at = %x\n", appl);
	cmdline = make_command_line();
	icache_disable();
	dcache_disable();
	(*appl) (cmdline);
	/* does not return */

	return 1;
}
