/*
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define DEBUG

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <image.h>
#include <malloc.h>
#include <zlib.h>
#include <bzlib.h>
#include <environment.h>
#include <asm/byteorder.h>

#if defined(CONFIG_OF_LIBFDT)
#include <fdt.h>
#include <libfdt.h>
#include <fdt_support.h>

static void fdt_error (const char *msg);
static ulong get_fdt (ulong alloc_current, cmd_tbl_t *cmdtp, int flag,
		int argc, char *argv[],
		image_header_t *hdr, char **of_flat_tree);
#endif

#ifdef CFG_INIT_RAM_LOCK
#include <asm/cache.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
static ulong get_sp (void);
static void set_clocks_in_mhz (bd_t *kbd);

void  __attribute__((noinline))
do_bootm_linux(cmd_tbl_t *cmdtp, int flag,
		int	argc, char *argv[],
		image_header_t *hdr,
		int	verify)
{
	ulong	sp, sp_limit, alloc_current;

	ulong	initrd_start, initrd_end;
	ulong	rd_data_start, rd_data_end, rd_len;

	ulong	cmd_start, cmd_end;
	bd_t	*kbd;
	void	(*kernel)(bd_t *, ulong, ulong, ulong, ulong);

#if defined(CONFIG_OF_LIBFDT)
	char	*of_flat_tree;
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
	sp = get_sp();
	debug ("## Current stack ends at 0x%08lx ", sp);

	alloc_current = sp_limit = get_boot_sp_limit(sp);
	debug ("=> set upper limit to 0x%08lx\n", sp_limit);

	/* allocate space and init command line */
	alloc_current = get_boot_cmdline (alloc_current, &cmd_start, &cmd_end);

	/* allocate space for kernel copy of board info */
	alloc_current = get_boot_kbd (alloc_current, &kbd);
	set_clocks_in_mhz(kbd);

	/* find kernel */
	kernel = (void (*)(bd_t *, ulong, ulong, ulong, ulong))image_get_ep (hdr);

	/* find ramdisk */
	get_ramdisk (cmdtp, flag, argc, argv, hdr, verify,
			IH_ARCH_PPC, &rd_data_start, &rd_data_end);

	rd_len = rd_data_end - rd_data_start;

	alloc_current = ramdisk_high (alloc_current, rd_data_start, rd_len,
			kbd, sp_limit, get_sp (),
			&initrd_start, &initrd_end);

#if defined(CONFIG_OF_LIBFDT)
	/* find flattened device tree */
	alloc_current = get_fdt (alloc_current,
			cmdtp, flag, argc, argv, hdr, &of_flat_tree);

	/*
	 * Add the chosen node if it doesn't exist, add the env and bd_t
	 * if the user wants it (the logic is in the subroutines).
	 */
	if (of_flat_tree) {
		if (fdt_chosen(of_flat_tree, initrd_start, initrd_end, 0) < 0) {
			fdt_error ("/chosen node create failed");
			do_reset (cmdtp, flag, argc, argv);
		}
#ifdef CONFIG_OF_HAS_UBOOT_ENV
		if (fdt_env(of_flat_tree) < 0) {
			fdt_error ("/u-boot-env node create failed");
			do_reset (cmdtp, flag, argc, argv);
		}
#endif
#ifdef CONFIG_OF_HAS_BD_T
		if (fdt_bd_t(of_flat_tree) < 0) {
			fdt_error ("/bd_t node create failed");
			do_reset (cmdtp, flag, argc, argv);
		}
#endif
#ifdef CONFIG_OF_BOARD_SETUP
		/* Call the board-specific fixup routine */
		ft_board_setup(of_flat_tree, gd->bd);
#endif
	}
#endif	/* CONFIG_OF_LIBFDT */

	debug ("## Transferring control to Linux (at address %08lx) ...\n",
		(ulong)kernel);

	show_boot_progress (15);

#if defined(CFG_INIT_RAM_LOCK) && !defined(CONFIG_E500)
	unlock_ram_in_cache();
#endif

#if defined(CONFIG_OF_LIBFDT)
	if (of_flat_tree) {	/* device tree; boot new style */
		/*
		 * Linux Kernel Parameters (passing device tree):
		 *   r3: pointer to the fdt, followed by the board info data
		 *   r4: physical pointer to the kernel itself
		 *   r5: NULL
		 *   r6: NULL
		 *   r7: NULL
		 */
		(*kernel) ((bd_t *)of_flat_tree, (ulong)kernel, 0, 0, 0);
		/* does not return */
	}
#endif
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

static ulong get_sp (void)
{
	ulong sp;

	asm( "mr %0,1": "=r"(sp) : );
	return sp;
}

static void set_clocks_in_mhz (bd_t *kbd)
{
	char	*s;

	if ((s = getenv ("clocks_in_mhz")) != NULL) {
		/* convert all clock information to MHz */
		kbd->bi_intfreq /= 1000000L;
		kbd->bi_busfreq /= 1000000L;
#if defined(CONFIG_MPC8220)
		kbd->bi_inpfreq /= 1000000L;
		kbd->bi_pcifreq /= 1000000L;
		kbd->bi_pevfreq /= 1000000L;
		kbd->bi_flbfreq /= 1000000L;
		kbd->bi_vcofreq /= 1000000L;
#endif
#if defined(CONFIG_CPM2)
		kbd->bi_cpmfreq /= 1000000L;
		kbd->bi_brgfreq /= 1000000L;
		kbd->bi_sccfreq /= 1000000L;
		kbd->bi_vco     /= 1000000L;
#endif
#if defined(CONFIG_MPC5xxx)
		kbd->bi_ipbfreq /= 1000000L;
		kbd->bi_pcifreq /= 1000000L;
#endif /* CONFIG_MPC5xxx */
	}
}

#if defined(CONFIG_OF_LIBFDT)
static void fdt_error (const char *msg)
{
	puts ("ERROR: ");
	puts (msg);
	puts (" - must RESET the board to recover.\n");
}

static ulong get_fdt (ulong alloc_current,
		cmd_tbl_t *cmdtp, int flag,
		int argc, char *argv[],
		image_header_t *hdr, char **of_flat_tree)
{
	image_header_t	*fdt_hdr;
	ulong		fdt_relocate = 0;
	char		*fdt = NULL;
	ulong		new_alloc_current;

	if(argc > 3) {
		fdt = (char *)simple_strtoul (argv[3], NULL, 16);
		fdt_hdr = (image_header_t *)fdt;

		if (fdt_check_header (fdt) == 0) {
			printf ("## Flattened Device Tree blob at %08lx\n", fdt);
#ifndef CFG_NO_FLASH
			if (addr2info ((ulong)fdt) != NULL)
				fdt_relocate = 1;
#endif
		} else if (image_check_magic (fdt_hdr)) {
			ulong image_start, image_end;
			ulong load_start, load_end;

			printf ("## Flattened Device Tree Image at %08lx\n",
					fdt_hdr);

			print_image_hdr (fdt_hdr);

			image_start = (ulong)fdt_hdr;
			image_end = image_get_image_end (fdt_hdr);

			load_start = image_get_load (fdt_hdr);
			load_end = load_start + image_get_data_size (fdt_hdr);

			if ((load_start < image_end) && (load_end > image_start)) {
				fdt_error ("fdt overwritten");
				do_reset (cmdtp, flag, argc, argv);
			}

			puts ("   Verifying Checksum ... ");
			if (!image_check_hcrc (fdt_hdr)) {
				fdt_error ("fdt header checksum invalid");
				do_reset (cmdtp, flag, argc, argv);
			}

			if (!image_check_dcrc (fdt_hdr)) {
				fdt_error ("fdt checksum invalid");
				do_reset (cmdtp, flag, argc, argv);
			}
			puts ("OK\n");

			if (!image_check_type (fdt_hdr, IH_TYPE_FLATDT)) {
				fdt_error ("uImage is not a fdt");
				do_reset (cmdtp, flag, argc, argv);
			}
			if (image_get_comp (fdt_hdr) != IH_COMP_NONE) {
				fdt_error ("uImage is compressed");
				do_reset (cmdtp, flag, argc, argv);
			}
			if (fdt_check_header ((char *)image_get_data (fdt_hdr)) != 0) {
				fdt_error ("uImage data is not a fdt");
				do_reset (cmdtp, flag, argc, argv);
			}

			memmove ((void *)image_get_load (fdt_hdr),
				(void *)image_get_data (fdt_hdr),
				image_get_data_size (fdt_hdr));

			fdt = (char *)image_get_load (fdt_hdr);
		} else {
			fdt_error ("Did not find a Flattened Device Tree");
			do_reset (cmdtp, flag, argc, argv);
		}
		printf ("   Booting using the fdt at 0x%x\n",
				fdt);
	} else if (image_check_type (hdr, IH_TYPE_MULTI)) {
		ulong fdt_data, fdt_len;

		printf ("## Flattened Device Tree from multi "
			"component Image at %08lX\n", (ulong)hdr);

		image_multi_getimg (hdr, 2, &fdt_data, &fdt_len);
		if (fdt_len) {

			fdt = (char *)fdt_data;
			printf ("   Booting using the fdt at 0x%x\n", fdt);

#ifndef CFG_NO_FLASH
			/* move the blob if it is in flash (set of_relocte) */
			if (addr2info ((ulong)fdt) != NULL)
				fdt_relocate = 1;
#endif

			if (fdt_check_header (fdt) != 0) {
				fdt_error ("image is not a fdt");
				do_reset (cmdtp, flag, argc, argv);
			}

			if (be32_to_cpu (fdt_totalsize (fdt)) != fdt_len) {
				fdt_error ("fdt size != image size");
				do_reset (cmdtp, flag, argc, argv);
			}
		} else {
			debug ("   Did not find a Flattened Device Tree");
		}
	}

#ifdef CFG_BOOTMAPSZ
	/*
	 * The blob must be within CFG_BOOTMAPSZ,
	 * so we flag it to be copied if it is not.
	 */
	if (fdt >= (char *)CFG_BOOTMAPSZ)
		fdt_relocate = 1;
#endif

	/* move flattend device tree if needed */
	if (fdt_relocate) {
		int err;
		ulong of_start, of_len;

		of_len = be32_to_cpu (fdt_totalsize (fdt));

		/* position on a 4K boundary before the alloc_current */
		of_start  = alloc_current - of_len;
		of_start &= ~(4096 - 1);	/* align on page */

		debug ("## device tree at 0x%08lX ... 0x%08lX (len=%ld=0x%lX)\n",
			(ulong)fdt, (ulong)fdt + of_len - 1,
			of_len, of_len);

		printf ("   Loading Device Tree to %08lx, end %08lx ... ",
			of_start, of_start + of_len - 1);

		err = fdt_open_into (fdt, (void *)of_start, of_len);
		if (err != 0) {
			fdt_error ("fdt move failed");
			do_reset (cmdtp, flag, argc, argv);
		}
		puts ("OK\n");

		*of_flat_tree = (char *)of_start;
		new_alloc_current = of_start;
	} else {
		*of_flat_tree = fdt;
		new_alloc_current = alloc_current;
	}

	return new_alloc_current;
}
#endif
