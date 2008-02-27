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
static void get_fdt (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		bootm_headers_t *images, char **of_flat_tree, ulong *of_size);
static ulong fdt_relocate (ulong alloc_current,
		cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		char **of_flat_tree, ulong *of_size);
#endif

#ifdef CFG_INIT_RAM_LOCK
#include <asm/cache.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
static ulong get_sp (void);
static void set_clocks_in_mhz (bd_t *kbd);

void  __attribute__((noinline))
do_bootm_linux(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		bootm_headers_t *images)
{
	ulong	sp, sp_limit, alloc_current;

	ulong	initrd_start, initrd_end;
	ulong	rd_data_start, rd_data_end, rd_len;

	ulong	cmd_start, cmd_end;
	bd_t	*kbd;
	ulong	ep = 0;
	void	(*kernel)(bd_t *, ulong, ulong, ulong, ulong);

#if defined(CONFIG_OF_LIBFDT)
	char	*of_flat_tree = NULL;
	ulong	of_size = 0;
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

	/* find kernel entry point */
	if (images->legacy_hdr_valid) {
		ep = image_get_ep (images->legacy_hdr_os);
#if defined(CONFIG_FIT)
	} else if (images->fit_uname_os) {
		fit_unsupported_reset ("PPC linux bootm");
		do_reset (cmdtp, flag, argc, argv);
#endif
	} else {
		puts ("Could not find kernel entry point!\n");
		do_reset (cmdtp, flag, argc, argv);
	}
	kernel = (void (*)(bd_t *, ulong, ulong, ulong, ulong))ep;

	/* find ramdisk */
	get_ramdisk (cmdtp, flag, argc, argv, images,
			IH_ARCH_PPC, &rd_data_start, &rd_data_end);

	rd_len = rd_data_end - rd_data_start;

	alloc_current = ramdisk_high (alloc_current, rd_data_start, rd_len,
			kbd, sp_limit, get_sp (),
			&initrd_start, &initrd_end);

#if defined(CONFIG_OF_LIBFDT)
	/* find flattened device tree */
	get_fdt (cmdtp, flag, argc, argv, images, &of_flat_tree, &of_size);

	alloc_current = fdt_relocate (alloc_current,
			cmdtp, flag, argc, argv, &of_flat_tree, &of_size);

	/*
	 * Add the chosen node if it doesn't exist, add the env and bd_t
	 * if the user wants it (the logic is in the subroutines).
	 */
	if (of_size) {
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

static image_header_t *image_get_fdt (ulong fdt_addr)
{
	image_header_t *fdt_hdr = (image_header_t *)fdt_addr;

	image_print_contents (fdt_hdr);

	puts ("   Verifying Checksum ... ");
	if (!image_check_hcrc (fdt_hdr)) {
		fdt_error ("fdt header checksum invalid");
		return NULL;
	}

	if (!image_check_dcrc (fdt_hdr)) {
		fdt_error ("fdt checksum invalid");
		return NULL;
	}
	puts ("OK\n");

	if (!image_check_type (fdt_hdr, IH_TYPE_FLATDT)) {
		fdt_error ("uImage is not a fdt");
		return NULL;
	}
	if (image_get_comp (fdt_hdr) != IH_COMP_NONE) {
		fdt_error ("uImage is compressed");
		return NULL;
	}
	if (fdt_check_header ((char *)image_get_data (fdt_hdr)) != 0) {
		fdt_error ("uImage data is not a fdt");
		return NULL;
	}
	return fdt_hdr;
}

static void get_fdt (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		bootm_headers_t *images, char **of_flat_tree, ulong *of_size)
{
	ulong		fdt_addr;
	image_header_t	*fdt_hdr;
	char		*fdt_blob = NULL;
	ulong		image_start, image_end;
	ulong		load_start, load_end;
#if defined(CONFIG_FIT)
        void            *fit_hdr;
        const char      *fit_uname_config = NULL;
        const char      *fit_uname_fdt = NULL;
	ulong		default_addr;
#endif

	if (argc > 3) {
#if defined(CONFIG_FIT)
		/*
		 * If the FDT blob comes from the FIT image and the FIT image
		 * address is omitted in the command line argument, try to use
		 * ramdisk or os FIT image address or default load address.
		 */
		if (images->fit_uname_rd)
			default_addr = (ulong)images->fit_hdr_rd;
		else if (images->fit_uname_os)
			default_addr = (ulong)images->fit_hdr_os;
		else
			default_addr = load_addr;

		if (fit_parse_conf (argv[3], default_addr,
					&fdt_addr, &fit_uname_config)) {
			debug ("*  fdt: config '%s' from image at 0x%08lx\n",
					fit_uname_config, fdt_addr);
		} else if (fit_parse_subimage (argv[3], default_addr,
					&fdt_addr, &fit_uname_fdt)) {
			debug ("*  fdt: subimage '%s' from image at 0x%08lx\n",
					fit_uname_fdt, fdt_addr);
		} else
#endif
		{
			fdt_addr = simple_strtoul(argv[3], NULL, 16);
			debug ("*  fdt: cmdline image address = 0x%08lx\n",
					fdt_addr);
		}

		debug ("## Checking for 'FDT'/'FDT image' at %08lx\n",
				fdt_addr);

		/* copy from dataflash if needed */
		fdt_addr = gen_get_image (fdt_addr);

		/*
		 * Check if there is an FDT image at the
		 * address provided in the second bootm argument
		 * check image type, for FIT images get a FIT node.
		 */
		switch (gen_image_get_format ((void *)fdt_addr)) {
		case IMAGE_FORMAT_LEGACY:
			debug ("*  fdt: legacy format image\n");

			/* verify fdt_addr points to a valid image header */
			printf ("## Flattened Device Tree Legacy Image at %08lx\n",
					fdt_addr);
			fdt_hdr = image_get_fdt (fdt_addr);
			if (!fdt_hdr)
				do_reset (cmdtp, flag, argc, argv);

			/*
			 * move image data to the load address,
			 * make sure we don't overwrite initial image
			 */
			image_start = (ulong)fdt_hdr;
			image_end = image_get_image_end (fdt_hdr);

			load_start = image_get_load (fdt_hdr);
			load_end = load_start + image_get_data_size (fdt_hdr);

			if ((load_start < image_end) && (load_end > image_start)) {
				fdt_error ("fdt overwritten");
				do_reset (cmdtp, flag, argc, argv);
			}
			memmove ((void *)image_get_load (fdt_hdr),
					(void *)image_get_data (fdt_hdr),
					image_get_data_size (fdt_hdr));

			fdt_blob = (char *)image_get_load (fdt_hdr);
			break;
#if defined(CONFIG_FIT)
		case IMAGE_FORMAT_FIT:

			/* check FDT blob vs FIT hdr */
			if (fit_uname_config || fit_uname_fdt) {
				/*
				 * FIT image
				 */
				fit_hdr = (void *)fdt_addr;
				debug ("*  fdt: FIT format image\n");
				fit_unsupported_reset ("PPC fdt");
				do_reset (cmdtp, flag, argc, argv);
			} else {
				/*
				 * FDT blob
				 */
				printf ("## Flattened Device Tree blob at %08lx\n", fdt_blob);
				fdt_blob = (char *)fdt_addr;
			}
			break;
#endif
		default:
			fdt_error ("Did not find a cmdline Flattened Device Tree");
			do_reset (cmdtp, flag, argc, argv);
		}

		printf ("   Booting using the fdt blob at 0x%x\n", fdt_blob);

	} else if (images->legacy_hdr_valid &&
			image_check_type (images->legacy_hdr_os, IH_TYPE_MULTI)) {

		ulong fdt_data, fdt_len;

		/*
		 * Now check if we have a legacy multi-component image,
		 * get second entry data start address and len.
		 */
		printf ("## Flattened Device Tree from multi "
			"component Image at %08lX\n",
			(ulong)images->legacy_hdr_os);

		image_multi_getimg (images->legacy_hdr_os, 2, &fdt_data, &fdt_len);
		if (fdt_len) {

			fdt_blob = (char *)fdt_data;
			printf ("   Booting using the fdt at 0x%x\n", fdt_blob);

			if (fdt_check_header (fdt_blob) != 0) {
				fdt_error ("image is not a fdt");
				do_reset (cmdtp, flag, argc, argv);
			}

			if (be32_to_cpu (fdt_totalsize (fdt_blob)) != fdt_len) {
				fdt_error ("fdt size != image size");
				do_reset (cmdtp, flag, argc, argv);
			}
		} else {
			fdt_error ("Did not find a Flattened Device Tree "
				"in a legacy multi-component image");
			do_reset (cmdtp, flag, argc, argv);
		}
	} else {
		debug ("## No Flattened Device Tree\n");
		*of_flat_tree = NULL;
		*of_size = 0;
		return;
	}

	*of_flat_tree = fdt_blob;
	*of_size = be32_to_cpu (fdt_totalsize (fdt_blob));
	debug ("   of_flat_tree at 0x%08lx size 0x%08lx\n",
			*of_flat_tree, *of_size);
}

static ulong fdt_relocate (ulong alloc_current,
		cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		char **of_flat_tree, ulong *of_size)
{
	char	*fdt_blob = *of_flat_tree;
	ulong	relocate = 0;
	ulong	new_alloc_current;

	/* nothing to do */
	if (*of_size == 0)
		return alloc_current;

	if (fdt_check_header (fdt_blob) != 0) {
		fdt_error ("image is not a fdt");
		do_reset (cmdtp, flag, argc, argv);
	}

#ifndef CFG_NO_FLASH
	/* move the blob if it is in flash (set relocate) */
	if (addr2info ((ulong)fdt_blob) != NULL)
		relocate = 1;
#endif

#ifdef CFG_BOOTMAPSZ
	/*
	 * The blob must be within CFG_BOOTMAPSZ,
	 * so we flag it to be copied if it is not.
	 */
	if (fdt_blob >= (char *)CFG_BOOTMAPSZ)
		relocate = 1;
#endif

	/* move flattend device tree if needed */
	if (relocate) {
		int err;
		ulong of_start, of_len;

		of_len = *of_size;

		/* position on a 4K boundary before the alloc_current */
		of_start  = alloc_current - of_len;
		of_start &= ~(4096 - 1);	/* align on page */

		debug ("## device tree at 0x%08lX ... 0x%08lX (len=%ld=0x%lX)\n",
			(ulong)fdt_blob, (ulong)fdt_blob + of_len - 1,
			of_len, of_len);

		printf ("   Loading Device Tree to %08lx, end %08lx ... ",
			of_start, of_start + of_len - 1);

		err = fdt_open_into (fdt_blob, (void *)of_start, of_len);
		if (err != 0) {
			fdt_error ("fdt move failed");
			do_reset (cmdtp, flag, argc, argv);
		}
		puts ("OK\n");

		*of_flat_tree = (char *)of_start;
		new_alloc_current = of_start;
	} else {
		*of_flat_tree = fdt_blob;
		new_alloc_current = alloc_current;
	}

	return new_alloc_current;
}
#endif
