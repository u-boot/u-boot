/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <command.h>
#include <asm/byteorder.h>

/* The SH kernel reads arguments from the empty zero page at location
 * 0 at the start of SDRAM. The following are copied from
 * arch/sh/kernel/setup.c and may require tweaking if the kernel sources
 * change.
 */
#define PARAM   ((unsigned char *)CFG_SDRAM_BASE + 0x1000)

#define MOUNT_ROOT_RDONLY (*(unsigned long *) (PARAM+0x000))
#define RAMDISK_FLAGS (*(unsigned long *) (PARAM+0x004))
#define ORIG_ROOT_DEV (*(unsigned long *) (PARAM+0x008))
#define LOADER_TYPE (*(unsigned long *) (PARAM+0x00c))
#define INITRD_START (*(unsigned long *) (PARAM+0x010))
#define INITRD_SIZE (*(unsigned long *) (PARAM+0x014))
/* ... */
#define COMMAND_LINE ((char *) (PARAM+0x100))

#define RAMDISK_IMAGE_START_MASK        0x07FF

extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#ifdef CFG_DEBUG
static void hexdump (unsigned char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
			printf ("%s%08x: ", i ? "\n" : "", (unsigned int) &buf[i]);
		printf ("%02x ", buf[i]);
	}
	printf ("\n");
}
#endif

void do_bootm_linux (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
                     bootm_headers_t *images)
{
	ulong	ep = 0;
	char	*bootargs = getenv("bootargs");

	/* find kernel entry point */
	if (images->legacy_hdr_valid) {
		ep = image_get_ep (images->legacy_hdr_os);
#if defined(CONFIG_FIT)
	} else if (images->fit_uname_os) {
		fit_unsupported_reset ("SH linux bootm");
		do_reset (cmdtp, flag, argc, argv);
#endif
	} else {
		puts ("Could not find kernel entry point!\n");
		do_reset (cmdtp, flag, argc, argv);
	}
	void (*kernel) (void) = (void (*)(void))ep;

	/* Setup parameters */
	memset(PARAM, 0, 0x1000);	/* Clear zero page */
	strcpy(COMMAND_LINE, bootargs);

	kernel();
}
