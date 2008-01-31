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

DECLARE_GLOBAL_DATA_PTR;

#define PHYSADDR(x) x

#define LINUX_MAX_ENVS		256
#define LINUX_MAX_ARGS		256

#ifdef CONFIG_SHOW_BOOT_PROGRESS
# include <status_led.h>
# define SHOW_BOOT_PROGRESS(arg)	show_boot_progress(arg)
#else
# define SHOW_BOOT_PROGRESS(arg)
#endif

void do_bootm_linux(cmd_tbl_t * cmdtp, int flag,
		    int argc, char *argv[],
		    image_header_t *hdr, int verify)
{
	ulong sp;

	ulong rd_data_start, rd_data_end, rd_len;
	ulong initrd_high;
	ulong initrd_start, initrd_end;
	int initrd_copy_to_ram = 1;

	ulong cmd_start, cmd_end;
	char *cmdline;
	char *s;
	bd_t *kbd;
	void (*kernel) (bd_t *, ulong, ulong, ulong, ulong);

	if ((s = getenv("initrd_high")) != NULL) {
		/* a value of "no" or a similar string will act like 0,
		 * turning the "load high" feature off. This is intentional.
		 */
		initrd_high = simple_strtoul(s, NULL, 16);
		if (initrd_high == ~0)
			initrd_copy_to_ram = 0;
	} else {		/* not set, no restrictions to load high */
		initrd_high = ~0;
	}

#ifdef CONFIG_LOGBUFFER
	kbd = gd->bd;
	/* Prevent initrd from overwriting logbuffer */
	if (initrd_high < (kbd->bi_memsize - LOGBUFF_LEN - LOGBUFF_OVERHEAD))
		initrd_high = kbd->bi_memsize - LOGBUFF_LEN - LOGBUFF_OVERHEAD;
	debug("## Logbuffer at 0x%08lX ", kbd->bi_memsize - LOGBUFF_LEN);
#endif

	/*
	 * Booting a (Linux) kernel image
	 *
	 * Allocate space for command line and board info - the
	 * address should be as high as possible within the reach of
	 * the kernel (see CFG_BOOTMAPSZ settings), but in unused
	 * memory, which means far enough below the current stack
	 * pointer.
	 */
	asm("movel %%a7, %%d0\n"
	    "movel %%d0, %0\n": "=d"(sp): :"%d0");

	debug("## Current stack ends at 0x%08lX ", sp);

	sp -= 2048;		/* just to be sure */
	if (sp > CFG_BOOTMAPSZ)
		sp = CFG_BOOTMAPSZ;
	sp &= ~0xF;

	debug("=> set upper limit to 0x%08lX\n", sp);

	cmdline = (char *)((sp - CFG_BARGSIZE) & ~0xF);
	kbd = (bd_t *) (((ulong) cmdline - sizeof(bd_t)) & ~0xF);

	if ((s = getenv("bootargs")) == NULL)
		s = "";

	strcpy(cmdline, s);

	cmd_start = (ulong) & cmdline[0];
	cmd_end = cmd_start + strlen(cmdline);

	*kbd = *(gd->bd);

#ifdef	DEBUG
	printf("## cmdline at 0x%08lX ... 0x%08lX\n", cmd_start, cmd_end);

	do_bdinfo(NULL, 0, 0, NULL);
#endif

	if ((s = getenv("clocks_in_mhz")) != NULL) {
		/* convert all clock information to MHz */
		kbd->bi_intfreq /= 1000000L;
		kbd->bi_busfreq /= 1000000L;
	}

	kernel =
	    (void (*)(bd_t *, ulong, ulong, ulong, ulong))image_get_ep (hdr);

	get_ramdisk (cmdtp, flag, argc, argv, hdr, verify,
			IH_ARCH_M68K, &rd_data_start, &rd_data_end);
	rd_len = rd_data_end - rd_data_start;

	if (rd_data_start) {
		if (!initrd_copy_to_ram) {	/* zero-copy ramdisk support */
			initrd_start = rd_data_start;
			initrd_end = rd_data_end;
		} else {
			initrd_start = (ulong) kbd - rd_len;
			initrd_start &= ~(4096 - 1);	/* align on page */

			if (initrd_high) {
				ulong nsp;

				/*
				 * the inital ramdisk does not need to be within
				 * CFG_BOOTMAPSZ as it is not accessed until after
				 * the mm system is initialised.
				 *
				 * do the stack bottom calculation again and see if
				 * the initrd will fit just below the monitor stack
				 * bottom without overwriting the area allocated
				 * above for command line args and board info.
				 */
				asm("movel %%a7, %%d0\n"
				    "movel %%d0, %0\n": "=d"(nsp): :"%d0");

				nsp -= 2048;	/* just to be sure */
				nsp &= ~0xF;

				if (nsp > initrd_high)	/* limit as specified */
					nsp = initrd_high;

					nsp -= rd_len;
				nsp &= ~(4096 - 1);	/* align on page */

				if (nsp >= sp)
					initrd_start = nsp;
			}

			SHOW_BOOT_PROGRESS(12);

			debug
			    ("## initrd at 0x%08lX ... 0x%08lX (len=%ld=0x%lX)\n",
			     rd_data_start, rd_data_end - 1, rd_len, rd_len);

			initrd_end = initrd_start + rd_len;
			printf("   Loading Ramdisk to %08lx, end %08lx ... ",
			       initrd_start, initrd_end);

			memmove_wd((void *)initrd_start,
				   (void *)rd_data_start, rd_len, CHUNKSZ);

			puts("OK\n");
		}
	} else {
		initrd_start = 0;
		initrd_end = 0;
	}

	debug("## Transferring control to Linux (at address %08lx) ...\n",
	      (ulong) kernel);

	SHOW_BOOT_PROGRESS(15);

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
}
