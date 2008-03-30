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

extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

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

void do_bootm_linux(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		    bootm_headers_t *images)
{
	int	(*appl) (char *cmdline);
	char	*cmdline;
	ulong	ep = 0;

	if (!images->autostart)
		return;

#ifdef SHARED_RESOURCES
	swap_to(FLASH);
#endif

	/* find kernel entry point */
	if (images->legacy_hdr_valid) {
		ep = image_get_ep (images->legacy_hdr_os);
#if defined(CONFIG_FIT)
	} else if (images->fit_uname_os) {
		int ret = fit_image_get_entry (images->fit_hdr_os,
				images->fit_noffset_os, &ep);
		if (ret) {
			puts ("Can't get entry point property!\n");
			goto error;
		}
#endif
	} else {
		puts ("Could not find kernel entry point!\n");
		goto error;
	}
	appl = (int (*)(char *))ep;

	printf("Starting Kernel at = %x\n", appl);
	cmdline = make_command_line();
	icache_disable();
	dcache_disable();
	(*appl) (cmdline);
	/* does not return */
	return;

 error:
	if (images->autostart)
		do_reset (cmdtp, flag, argc, argv);
}
