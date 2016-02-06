/*
 * U-Boot - boot.c - misc boot helper functions
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

#ifdef CONFIG_VIDEO
extern void video_stop(void);
#endif

static char *make_command_line(void)
{
	char *dest = (char *)CONFIG_LINUX_CMDLINE_ADDR;
	char *bootargs = getenv("bootargs");

	if (bootargs == NULL)
		return NULL;

	strncpy(dest, bootargs, CONFIG_LINUX_CMDLINE_SIZE);
	dest[CONFIG_LINUX_CMDLINE_SIZE - 1] = 0;
	return dest;
}

extern ulong bfin_poweron_retx;

int do_bootm_linux(int flag, int argc, char * const argv[], bootm_headers_t *images)
{
	int	(*appl) (char *cmdline);
	char	*cmdline;

	if (flag & BOOTM_STATE_OS_PREP)
		return 0;
	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

#ifdef SHARED_RESOURCES
	swap_to(FLASH);
#endif

#ifdef CONFIG_VIDEO
	/* maybe this should be standardized and moved to bootm ... */
	video_stop();
#endif

	appl = (int (*)(char *))images->ep;

	printf("Starting Kernel at = %p\n", appl);
	cmdline = make_command_line();
	icache_disable();
	dcache_disable();
	asm __volatile__(
		"RETX = %[retx];"
		"CALL (%0);"
		:
		: "p"(appl), "q0"(cmdline), [retx] "d"(bfin_poweron_retx)
	);
	/* does not return */

	return 1;
}
