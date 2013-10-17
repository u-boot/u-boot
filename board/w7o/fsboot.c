/*
 * (C) Copyright 2001
 * Wave 7 Optics, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <command.h>

/*
 * FIXME: Add code to test image and it's header.
 */
extern int valid_elf_image (unsigned long addr);

static int
image_check(ulong addr)
{
    return valid_elf_image(addr);
}

void
init_fsboot(void)
{
    char  *envp;
    ulong loadaddr;
    ulong testaddr;
    ulong alt_loadaddr;
    char buf[9];

    /*
     * Get test image address
     */
    if ((envp = getenv("testaddr")) != NULL)
	testaddr = simple_strtoul(envp, NULL, 16);
    else
	testaddr = -1;

    /*
     * Are we going to test boot and image?
     */
    if ((testaddr != -1) && image_check(testaddr)) {

	/* Set alt_loadaddr */
	alt_loadaddr = testaddr;
	sprintf(buf, "%lX", alt_loadaddr);
	setenv("alt_loadaddr", buf);

	/* Clear test_addr */
	setenv("testaddr", NULL);

	/*
	 * Save current environment with alt_loadaddr,
	 * and cleared testaddr.
	 */
	saveenv();

	/*
	 * Setup temporary loadaddr to alt_loadaddr
	 * XXX - DO NOT SAVE ENVIRONMENT!
	 */
	loadaddr = alt_loadaddr;
	sprintf(buf, "%lX", loadaddr);
	setenv("loadaddr", buf);

    } else { /* Normal boot */
	setenv("alt_loadaddr", NULL);		/* Clear alt_loadaddr */
	setenv("testaddr", NULL);		/* Clear testaddr */
	saveenv();
    }

    return;
}
