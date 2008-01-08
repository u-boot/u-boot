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
#elif defined(CONFIG_OF_FLAT_TREE)
#include <ft_build.h>
#endif

#ifdef CONFIG_LOGBUFFER
#include <logbuff.h>
#endif

#ifdef CFG_INIT_RAM_LOCK
#include <asm/cache.h>
#endif

DECLARE_GLOBAL_DATA_PTR;
extern image_header_t header;

/*cmd_boot.c*/
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#if defined(CONFIG_CMD_BDI)
extern int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif

void  __attribute__((noinline))
do_bootm_linux (cmd_tbl_t *cmdtp, int flag,
		int	argc, char *argv[],
		ulong	addr,
		ulong	*len_ptr,
		int	verify)
{
	ulong	sp;
	ulong	len;
	ulong	initrd_start, initrd_end;
	ulong	cmd_start, cmd_end;
	ulong	initrd_high;
	ulong	data;
	int	initrd_copy_to_ram = 1;
	char    *cmdline;
	char	*s;
	bd_t	*kbd;
	void	(*kernel)(bd_t *, ulong, ulong, ulong, ulong);
	image_header_t *hdr = &header;
#if defined(CONFIG_OF_FLAT_TREE) || defined(CONFIG_OF_LIBFDT)
	char	*of_flat_tree = NULL;
	ulong	of_data = 0;
#endif

	if ((s = getenv ("initrd_high")) != NULL) {
		/* a value of "no" or a similar string will act like 0,
		 * turning the "load high" feature off. This is intentional.
		 */
		initrd_high = simple_strtoul(s, NULL, 16);
		if (initrd_high == ~0)
			initrd_copy_to_ram = 0;
	} else {	/* not set, no restrictions to load high */
		initrd_high = ~0;
	}

#ifdef CONFIG_LOGBUFFER
	kbd=gd->bd;
	/* Prevent initrd from overwriting logbuffer */
	if (initrd_high < (kbd->bi_memsize-LOGBUFF_LEN-LOGBUFF_OVERHEAD))
		initrd_high = kbd->bi_memsize-LOGBUFF_LEN-LOGBUFF_OVERHEAD;
	debug ("## Logbuffer at 0x%08lX ", kbd->bi_memsize-LOGBUFF_LEN);
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

	asm( "mr %0,1": "=r"(sp) : );

	debug ("## Current stack ends at 0x%08lX ", sp);

	sp -= 2048;		/* just to be sure */
	if (sp > CFG_BOOTMAPSZ)
		sp = CFG_BOOTMAPSZ;
	sp &= ~0xF;

	debug ("=> set upper limit to 0x%08lX\n", sp);

	cmdline = (char *)((sp - CFG_BARGSIZE) & ~0xF);
	kbd = (bd_t *)(((ulong)cmdline - sizeof(bd_t)) & ~0xF);

	if ((s = getenv("bootargs")) == NULL)
		s = "";

	strcpy (cmdline, s);

	cmd_start    = (ulong)&cmdline[0];
	cmd_end      = cmd_start + strlen(cmdline);

	*kbd = *(gd->bd);

#ifdef	DEBUG
	printf ("## cmdline at 0x%08lX ... 0x%08lX\n", cmd_start, cmd_end);

#if defined(CONFIG_CMD_BDI)
	do_bdinfo (NULL, 0, 0, NULL);
#endif
#endif

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

	kernel = (void (*)(bd_t *, ulong, ulong, ulong, ulong))image_get_ep (hdr);

	/*
	 * Check if there is an initrd image
	 */

#if defined(CONFIG_OF_FLAT_TREE) || defined(CONFIG_OF_LIBFDT)
	/* Look for a '-' which indicates to ignore the ramdisk argument */
	if (argc >= 3 && strcmp(argv[2], "-") ==  0) {
			debug ("Skipping initrd\n");
			len = data = 0;
		}
	else
#endif
	if (argc >= 3) {
		debug ("Not skipping initrd\n");
		show_boot_progress (9);

		addr = simple_strtoul(argv[2], NULL, 16);

		printf ("## Loading RAMDisk Image at %08lx ...\n", addr);
		hdr = (image_header_t *)addr;

		if (!image_check_magic (hdr)) {
			puts ("Bad Magic Number\n");
			show_boot_progress (-10);
			do_reset (cmdtp, flag, argc, argv);
		}

		if (!image_check_hcrc (hdr)) {
			puts ("Bad Header Checksum\n");
			show_boot_progress (-11);
			do_reset (cmdtp, flag, argc, argv);
		}
		show_boot_progress (10);

		print_image_hdr (hdr);

		if (verify) {
			puts ("   Verifying Checksum ... ");

			if (!image_check_dcrc_wd (hdr, CHUNKSZ)) {
				puts ("Bad Data CRC\n");
				show_boot_progress (-12);
				do_reset (cmdtp, flag, argc, argv);
			}
			puts ("OK\n");
		}

		show_boot_progress (11);

		if (!image_check_os (hdr, IH_OS_LINUX) ||
		    !image_check_arch (hdr, IH_ARCH_PPC) ||
		    !image_check_type (hdr, IH_TYPE_RAMDISK)) {
			puts ("No Linux PPC Ramdisk Image\n");
			show_boot_progress (-13);
			do_reset (cmdtp, flag, argc, argv);
		}

		data = image_get_data (hdr);
		len = image_get_data_size (hdr);

		/*
		 * Now check if we have a multifile image
		 */
	} else if (image_check_type (hdr, IH_TYPE_MULTI) && (len_ptr[1])) {
		u_long tail    = image_to_cpu (len_ptr[0]) % 4;
		int i;

		show_boot_progress (13);

		/* skip kernel length and terminator */
		data = (ulong)(&len_ptr[2]);
		/* skip any additional image length fields */
		for (i=1; len_ptr[i]; ++i)
			data += 4;
		/* add kernel length, and align */
		data += image_to_cpu (len_ptr[0]);
		if (tail) {
			data += 4 - tail;
		}

		len   = image_to_cpu (len_ptr[1]);

	} else {
		/*
		 * no initrd image
		 */
		show_boot_progress (14);

		len = data = 0;
	}

#if defined(CONFIG_OF_FLAT_TREE) || defined(CONFIG_OF_LIBFDT)
	if(argc > 3) {
		of_flat_tree = (char *) simple_strtoul(argv[3], NULL, 16);
		hdr = (image_header_t *)of_flat_tree;
#if defined(CONFIG_OF_FLAT_TREE)
		if (*((ulong *)(of_flat_tree + image_get_header_size ())) != OF_DT_HEADER) {
#elif defined(CONFIG_OF_LIBFDT)
		if (fdt_check_header (of_flat_tree + image_get_header_size ()) != 0) {
#endif
#ifndef CFG_NO_FLASH
			if (addr2info((ulong)of_flat_tree) != NULL)
				of_data = (ulong)of_flat_tree;
#endif
		} else if (image_check_magic (hdr)) {
			printf("## Flat Device Tree at %08lX\n", hdr);
			print_image_hdr (hdr);

			if ((image_get_load (hdr) <  ((unsigned long)hdr + image_get_image_size (hdr))) &&
			   ((image_get_load (hdr) + image_get_data_size (hdr)) > (unsigned long)hdr)) {
				puts ("ERROR: fdt overwritten - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}

			puts ("   Verifying Checksum ... ");
			if (!image_check_hcrc (hdr)) {
				puts ("ERROR: fdt header checksum invalid - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}

			if (!image_check_dcrc (hdr)) {
				puts ("ERROR: fdt checksum invalid - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}
			puts ("OK\n");

			if (!image_check_type (hdr, IH_TYPE_FLATDT)) {
				puts ("ERROR: uImage is not a fdt - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}
			if (image_get_comp (hdr) != IH_COMP_NONE) {
				puts ("ERROR: uImage is compressed - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}
#if defined(CONFIG_OF_FLAT_TREE)
			if (*((ulong *)(of_flat_tree + image_get_header_size ())) != OF_DT_HEADER) {
#elif defined(CONFIG_OF_LIBFDT)
			if (fdt_check_header (of_flat_tree + image_get_header_size ()) != 0) {
#endif
				puts ("ERROR: uImage data is not a fdt - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}

			memmove ((void *)image_get_load (hdr),
				(void *)(of_flat_tree + image_get_header_size ()),
				image_get_data_size (hdr));

			of_flat_tree = (char *)image_get_load (hdr);
		} else {
			puts ("Did not find a flat Flat Device Tree.\n"
				"Must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
		printf ("   Booting using the fdt at 0x%x\n",
				of_flat_tree);
	} else if (image_check_type (hdr, IH_TYPE_MULTI) && (len_ptr[1]) && (len_ptr[2])) {
		u_long tail    = image_to_cpu (len_ptr[0]) % 4;
		int i;

		/* skip kernel length, initrd length, and terminator */
		of_flat_tree = (char *)(&len_ptr[3]);
		/* skip any additional image length fields */
		for (i=2; len_ptr[i]; ++i)
			of_flat_tree += 4;
		/* add kernel length, and align */
		of_flat_tree += image_to_cpu (len_ptr[0]);
		if (tail) {
			of_flat_tree += 4 - tail;
		}

		/* add initrd length, and align */
		tail = image_to_cpu (len_ptr[1]) % 4;
		of_flat_tree += image_to_cpu (len_ptr[1]);
		if (tail) {
			of_flat_tree += 4 - tail;
		}

#ifndef CFG_NO_FLASH
		/* move the blob if it is in flash (set of_data to !null) */
		if (addr2info ((ulong)of_flat_tree) != NULL)
			of_data = (ulong)of_flat_tree;
#endif


#if defined(CONFIG_OF_FLAT_TREE)
		if (*((ulong *)(of_flat_tree)) != OF_DT_HEADER) {
#elif defined(CONFIG_OF_LIBFDT)
		if (fdt_check_header (of_flat_tree) != 0) {
#endif
			puts ("ERROR: image is not a fdt - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}

#if defined(CONFIG_OF_FLAT_TREE)
		if (((struct boot_param_header *)of_flat_tree)->totalsize !=
			image_to_cpu (len_ptr[2])) {
#elif defined(CONFIG_OF_LIBFDT)
		if (be32_to_cpu (fdt_totalsize (of_flat_tree)) !=
			image_to_cpu (len_ptr[2])) {
#endif
			puts ("ERROR: fdt size != image size - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
	}
#endif
	if (!data) {
		debug ("No initrd\n");
	}

	if (data) {
	    if (!initrd_copy_to_ram) {	/* zero-copy ramdisk support */
		initrd_start = data;
		initrd_end = initrd_start + len;
	    } else {
		initrd_start  = (ulong)kbd - len;
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
			asm( "mr %0,1": "=r"(nsp) : );
			nsp -= 2048;		/* just to be sure */
			nsp &= ~0xF;
			if (nsp > initrd_high)	/* limit as specified */
				nsp = initrd_high;
			nsp -= len;
			nsp &= ~(4096 - 1);	/* align on page */
			if (nsp >= sp)
				initrd_start = nsp;
		}

		show_boot_progress (12);

		debug ("## initrd at 0x%08lX ... 0x%08lX (len=%ld=0x%lX)\n",
			data, data + len - 1, len, len);

		initrd_end    = initrd_start + len;
		printf ("   Loading Ramdisk to %08lx, end %08lx ... ",
			initrd_start, initrd_end);
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
		{
			size_t l = len;
			void *to = (void *)initrd_start;
			void *from = (void *)data;

			while (l > 0) {
				size_t tail = (l > CHUNKSZ) ? CHUNKSZ : l;
				WATCHDOG_RESET();
				memmove (to, from, tail);
				to += tail;
				from += tail;
				l -= tail;
			}
		}
#else	/* !(CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG) */
		memmove ((void *)initrd_start, (void *)data, len);
#endif	/* CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG */
		puts ("OK\n");
	    }
	} else {
		initrd_start = 0;
		initrd_end = 0;
	}

#if defined(CONFIG_OF_LIBFDT)

#ifdef CFG_BOOTMAPSZ
	/*
	 * The blob must be within CFG_BOOTMAPSZ,
	 * so we flag it to be copied if it is not.
	 */
	if (of_flat_tree >= (char *)CFG_BOOTMAPSZ)
		of_data = (ulong)of_flat_tree;
#endif

	/* move of_flat_tree if needed */
	if (of_data) {
		int err;
		ulong of_start, of_len;

		of_len = be32_to_cpu(fdt_totalsize(of_data));

		/* position on a 4K boundary before the kbd */
		of_start  = (ulong)kbd - of_len;
		of_start &= ~(4096 - 1);	/* align on page */
		debug ("## device tree at 0x%08lX ... 0x%08lX (len=%ld=0x%lX)\n",
			of_data, of_data + of_len - 1, of_len, of_len);

		of_flat_tree = (char *)of_start;
		printf ("   Loading Device Tree to %08lx, end %08lx ... ",
			of_start, of_start + of_len - 1);
		err = fdt_open_into((void *)of_data, (void *)of_start, of_len);
		if (err != 0) {
			puts ("ERROR: fdt move failed - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
		puts ("OK\n");
	}
	/*
	 * Add the chosen node if it doesn't exist, add the env and bd_t
	 * if the user wants it (the logic is in the subroutines).
	 */
	if (of_flat_tree) {
		if (fdt_chosen(of_flat_tree, initrd_start, initrd_end, 0) < 0) {
			puts ("ERROR: /chosen node create failed - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
#ifdef CONFIG_OF_HAS_UBOOT_ENV
		if (fdt_env(of_flat_tree) < 0) {
			puts ("ERROR: /u-boot-env node create failed - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
#endif
#ifdef CONFIG_OF_HAS_BD_T
		if (fdt_bd_t(of_flat_tree) < 0) {
			puts ("ERROR: /bd_t node create failed - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
#endif
#ifdef CONFIG_OF_BOARD_SETUP
		/* Call the board-specific fixup routine */
		ft_board_setup(of_flat_tree, gd->bd);
#endif
	}

#elif defined(CONFIG_OF_FLAT_TREE)

#ifdef CFG_BOOTMAPSZ
	/*
	 * The blob must be within CFG_BOOTMAPSZ,
	 * so we flag it to be copied if it is not.
	 */
	if (of_flat_tree >= (char *)CFG_BOOTMAPSZ)
		of_data = (ulong)of_flat_tree;
#endif

	/* move of_flat_tree if needed */
	if (of_data) {
		ulong of_start, of_len;
		of_len = ((struct boot_param_header *)of_data)->totalsize;

		/* provide extra 8k pad */
		of_start  = (ulong)kbd - of_len - 8192;
		of_start &= ~(4096 - 1);	/* align on page */
		debug ("## device tree at 0x%08lX ... 0x%08lX (len=%ld=0x%lX)\n",
			of_data, of_data + of_len - 1, of_len, of_len);

		of_flat_tree = (char *)of_start;
		printf ("   Loading Device Tree to %08lx, end %08lx ... ",
			of_start, of_start + of_len - 1);
		memmove ((void *)of_start, (void *)of_data, of_len);
		puts ("OK\n");
	}
	/*
	 * Create the /chosen node and modify the blob with board specific
	 * values as needed.
	 */
	ft_setup(of_flat_tree, kbd, initrd_start, initrd_end);
	/* ft_dump_blob(of_flat_tree); */

#endif	/* #if defined(CONFIG_OF_LIBFDT) #elif defined(CONFIG_OF_FLAT_TREE) */

	debug ("## Transferring control to Linux (at address %08lx) ...\n",
		(ulong)kernel);

	show_boot_progress (15);

#if defined(CFG_INIT_RAM_LOCK) && !defined(CONFIG_E500)
	unlock_ram_in_cache();
#endif

#if defined(CONFIG_OF_FLAT_TREE) || defined(CONFIG_OF_LIBFDT)
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
