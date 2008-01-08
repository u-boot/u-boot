/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
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
#include <image.h>
#include <zlib.h>
#include <asm/byteorder.h>
#include <asm/zimage.h>

/*cmd_boot.c*/
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

void do_bootm_linux(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		image_header_t *hdr, int verify)
{
	void *base_ptr;

	ulong os_data, os_len;
	ulong rd_data, rd_len;
	ulong initrd_start, initrd_end;
	image_header_t *rd_hdr;

	/*
	 * Check if there is an initrd image
	 */
	if (argc >= 3) {
		rd_hdr = (image_header_t *)simple_strtoul (argv[2], NULL, 16);
		printf ("## Loading Ramdisk Image at %08lx ...\n", rd_hdr);

		if (!image_check_magic (rd_hdr)) {
			printf ("Bad Magic Number\n");
			do_reset (cmdtp, flag, argc, argv);
		}

		if (!image_check_hcrc (rd_hdr)) {
			printf ("Bad Header Checksum\n");
			do_reset (cmdtp, flag, argc, argv);
		}

		print_image_hdr (rd_hdr);

		rd_data = image_get_data (rd_hdr);
		rd_len = image_get_data_size (rd_hdr);

		if (verify) {
			printf ("   Verifying Checksum ... ");
			if (!image_check_dcrc (rd_hdr)) {
				printf ("Bad Data CRC\n");
				do_reset (cmdtp, flag, argc, argv);
			}
			printf ("OK\n");
		}

		if (!image_check_os (rd_hdr, IH_OS_LINUX) ||
		    !image_check_arch (rd_hdr, IH_ARCH_I386) ||
		    !image_check_type (rd_hdr, IH_TYPE_RAMDISK)) {
			printf ("No Linux i386 Ramdisk Image\n");
			do_reset (cmdtp, flag, argc, argv);
		}

		/*
		 * Now check if we have a multifile image
		 */
	} else if (image_check_type (hdr, IH_TYPE_MULTI)) {
		/*
		 * Get second entry data start address and len
		 */
		image_multi_getimg (hdr, 1, &rd_data, &rd_len);
	} else {
		/*
		 * no initrd image
		 */
		rd_data = rd_len = 0;
	}

#ifdef	DEBUG
	if (!rd_data) {
		printf ("No initrd\n");
	}
#endif

	if (rd_data) {
		initrd_start = rd_data;
		initrd_end = initrd_start + rd_len;
		printf ("   Loading Ramdisk to %08lx, end %08lx ... ",
			initrd_start, initrd_end);
		memmove ((void *)initrd_start, (void *)rd_data, rd_len);
		printf ("OK\n");
	} else {
		initrd_start = 0;
		initrd_end = 0;
	}

	/* if multi-part image, we need to advance base ptr */
	if (image_check_type (hdr, IH_TYPE_MULTI)) {
		image_multi_getimg (hdr, 0, &os_data, &os_len);
	} else {
		os_data = image_get_data (hdr);
		os_len = image_get_data_size (hdr);
	}

	base_ptr = load_zimage ((void*)os_data, os_len,
			initrd_start, rd_len, 0);

	if (NULL == base_ptr) {
		printf ("## Kernel loading failed ...\n");
		do_reset(cmdtp, flag, argc, argv);

	}

#ifdef DEBUG
	printf ("## Transferring control to Linux (at address %08x) ...\n",
		(u32)base_ptr);
#endif

	/* we assume that the kernel is in place */
	printf("\nStarting kernel ...\n\n");

	boot_zimage(base_ptr);

}
