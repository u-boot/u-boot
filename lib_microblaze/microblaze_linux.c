/*
 * (C) Copyright 2007 Michal Simek
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Michal  SIMEK <monstr@monstr.eu>
 * Yasushi SHOJI <yashi@atmark-techno.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <zlib.h>
#include <asm/byteorder.h>

DECLARE_GLOBAL_DATA_PTR;

extern image_header_t header;	/* from cmd_bootm.c */
/*cmd_boot.c*/
extern int do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);

void do_bootm_linux (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[],
		     ulong addr, ulong * len_ptr, int verify)
{
	ulong len = 0, checksum;
	ulong initrd_start, initrd_end;
	ulong data;
	/* First parameter is mapped to $r5 for kernel boot args */
	void (*theKernel) (char *);
	image_header_t *hdr = &header;
	char *commandline = getenv ("bootargs");
	int i;

	theKernel = (void (*)(char *))ntohl (hdr->ih_ep);

	/* Check if there is an initrd image */
	if (argc >= 3) {
		show_boot_progress (9);

		addr = simple_strtoul (argv[2], NULL, 16);

		printf ("## Loading Ramdisk Image at %08lx ...\n", addr);

		/* Copy header so we can blank CRC field for re-calculation */
		memcpy (&header, (char *)addr, sizeof (image_header_t));

		if (ntohl (hdr->ih_magic) != IH_MAGIC) {
			printf ("Bad Magic Number\n");
			show_boot_progress (-10);
			do_reset (cmdtp, flag, argc, argv);
		}

		data = (ulong) & header;
		len = sizeof (image_header_t);

		checksum = ntohl (hdr->ih_hcrc);
		hdr->ih_hcrc = 0;

		if (crc32 (0, (char *)data, len) != checksum) {
			printf ("Bad Header Checksum\n");
			show_boot_progress (-11);
			do_reset (cmdtp, flag, argc, argv);
		}

		show_boot_progress (10);

		print_image_hdr (hdr);

		data = addr + sizeof (image_header_t);
		len = ntohl (hdr->ih_size);

		if (verify) {
			ulong csum = 0;

			printf ("   Verifying Checksum ... ");
			csum = crc32 (0, (char *)data, len);
			if (csum != ntohl (hdr->ih_dcrc)) {
				printf ("Bad Data CRC\n");
				show_boot_progress (-12);
				do_reset (cmdtp, flag, argc, argv);
			}
			printf ("OK\n");
		}

		show_boot_progress (11);

		if ((hdr->ih_os != IH_OS_LINUX) ||
		    (hdr->ih_arch != IH_CPU_MICROBLAZE) ||
		    (hdr->ih_type != IH_TYPE_RAMDISK)) {
			printf ("No Linux Microblaze Ramdisk Image\n");
			show_boot_progress (-13);
			do_reset (cmdtp, flag, argc, argv);
		}

		/*
		 * Now check if we have a multifile image
		 */
	} else if ((hdr->ih_type == IH_TYPE_MULTI) && (len_ptr[1])) {
		ulong tail = ntohl (len_ptr[0]) % 4;

		show_boot_progress (13);

		/* skip kernel length and terminator */
		data = (ulong) (&len_ptr[2]);
		/* skip any additional image length fields */
		for (i = 1; len_ptr[i]; ++i)
			data += 4;
		/* add kernel length, and align */
		data += ntohl (len_ptr[0]);
		if (tail) {
			data += 4 - tail;
		}

		len = ntohl (len_ptr[1]);

	} else {
		/*
		 * no initrd image
		 */
		show_boot_progress (14);

		data = 0;
	}

#ifdef  DEBUG
	if (!data) {
		printf ("No initrd\n");
	}
#endif

	if (data) {
		initrd_start = data;
		initrd_end = initrd_start + len;
	} else {
		initrd_start = 0;
		initrd_end = 0;
	}

	show_boot_progress (15);

#ifdef DEBUG
	printf ("## Transferring control to Linux (at address %08lx) ...\n",
		(ulong) theKernel);
#endif

	theKernel (commandline);
}
