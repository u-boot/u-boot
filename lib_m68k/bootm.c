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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 *
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <zlib.h>
#include <bzlib.h>
#include <watchdog.h>
#include <environment.h>
#include <asm/byteorder.h>
#ifdef CONFIG_SHOW_BOOT_PROGRESS
# include <status_led.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#define PHYSADDR(x) x

#define LINUX_MAX_ENVS		256
#define LINUX_MAX_ARGS		256

static ulong get_sp (void);
static void set_clocks_in_mhz (bd_t *kbd);
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

void do_bootm_linux(cmd_tbl_t * cmdtp, int flag,
		    int argc, char *argv[],
		    bootm_headers_t *images)
{
	ulong sp, sp_limit, alloc_current;

	ulong rd_data_start, rd_data_end, rd_len;
	ulong initrd_start, initrd_end;
	int ret;

	ulong cmd_start, cmd_end;
	bd_t  *kbd;
	ulong ep = 0;
	void  (*kernel) (bd_t *, ulong, ulong, ulong, ulong);

	/*
	 * Booting a (Linux) kernel image
	 *
	 * Allocate space for command line and board info - the
	 * address should be as high as possible within the reach of
	 * the kernel (see CFG_BOOTMAPSZ settings), but in unused
	 * memory, which means far enough below the current stack
	 * pointer.
	 */
	sp = get_sp();
	debug ("## Current stack ends at 0x%08lx ", sp);

	alloc_current = sp_limit = get_boot_sp_limit(sp);
	debug ("=> set upper limit to 0x%08lx\n", sp_limit);

	/* allocate space and init command line */
	alloc_current = get_boot_cmdline (alloc_current, &cmd_start, &cmd_end);

	/* allocate space for kernel copy of board info */
	alloc_current = get_boot_kbd (alloc_current, &kbd);
	set_clocks_in_mhz(kbd);

	/* find kernel entry point */
	if (images->legacy_hdr_valid) {
		ep = image_get_ep (images->legacy_hdr_os);
#if defined(CONFIG_FIT)
	} else if (images->fit_uname_os) {
		fit_unsupported_reset ("M68K linux bootm");
		do_reset (cmdtp, flag, argc, argv);
#endif
	} else {
		puts ("Could not find kernel entry point!\n");
		do_reset (cmdtp, flag, argc, argv);
	}
	kernel = (void (*)(bd_t *, ulong, ulong, ulong, ulong))ep;

	/* find ramdisk */
	ret = get_ramdisk (cmdtp, flag, argc, argv, images,
			IH_ARCH_M68K, &rd_data_start, &rd_data_end);

	if (ret)
		goto error;

	rd_len = rd_data_end - rd_data_start;
	alloc_current = ramdisk_high (alloc_current, rd_data_start, rd_len,
			sp_limit, get_sp (), &initrd_start, &initrd_end);

	debug("## Transferring control to Linux (at address %08lx) ...\n",
	      (ulong) kernel);

	show_boot_progress (15);

	/*
	 * Linux Kernel Parameters (passing board info data):
	 *   r3: ptr to board info data
	 *   r4: initrd_start or 0 if no initrd
	 *   r5: initrd_end - unused if r4 is 0
	 *   r6: Start of command line string
	 *   r7: End   of command line string
	 */
	(*kernel) (kbd, initrd_start, initrd_end, cmd_start, cmd_end);
	/* does not return */
	return ;

error:
	do_reset (cmdtp, flag, argc, argv);
	return ;
}

static ulong get_sp (void)
{
	ulong sp;

	asm("movel %%a7, %%d0\n"
	    "movel %%d0, %0\n": "=d"(sp): :"%d0");

	return sp;
}

static void set_clocks_in_mhz (bd_t *kbd)
{
	char *s;

	if ((s = getenv("clocks_in_mhz")) != NULL) {
		/* convert all clock information to MHz */
		kbd->bi_intfreq /= 1000000L;
		kbd->bi_busfreq /= 1000000L;
	}
}
