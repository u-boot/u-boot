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

extern image_header_t header;           /* from cmd_bootm.c */


image_header_t *fake_header(image_header_t *hdr, void *ptr, int size)
{
	/* try each supported image type in order */
	if (NULL != fake_zimage_header(hdr, ptr, size)) {
		return hdr;
	}

	return NULL;
}


void do_bootm_linux(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		ulong addr, ulong *len_ptr, int   verify)
{
	void *base_ptr;

	ulong len = 0, checksum;
	ulong initrd_start, initrd_end;
	ulong data;
	image_header_t *hdr = &header;

	/*
	 * Check if there is an initrd image
	 */
	if (argc >= 3) {
		addr = simple_strtoul(argv[2], NULL, 16);

		printf ("## Loading Ramdisk Image at %08lx ...\n", addr);

		/* Copy header so we can blank CRC field for re-calculation */
		memcpy (&header, (char *)addr, sizeof(image_header_t));

		if (ntohl(hdr->ih_magic) != IH_MAGIC) {
			printf ("Bad Magic Number\n");
			do_reset (cmdtp, flag, argc, argv);
		}

		data = (ulong)&header;
		len  = sizeof(image_header_t);

		checksum = ntohl(hdr->ih_hcrc);
		hdr->ih_hcrc = 0;

		if (crc32 (0, (char *)data, len) != checksum) {
			printf ("Bad Header Checksum\n");
			do_reset (cmdtp, flag, argc, argv);
		}

		print_image_hdr (hdr);

		data = addr + sizeof(image_header_t);
		len  = ntohl(hdr->ih_size);

		if (verify) {
			ulong csum = 0;

			printf ("   Verifying Checksum ... ");
			csum = crc32 (0, (char *)data, len);
			if (csum != ntohl(hdr->ih_dcrc)) {
				printf ("Bad Data CRC\n");
				do_reset (cmdtp, flag, argc, argv);
			}
			printf ("OK\n");
		}

		if ((hdr->ih_os   != IH_OS_LINUX)	||
		    (hdr->ih_arch != IH_CPU_I386)	||
		    (hdr->ih_type != IH_TYPE_RAMDISK)	) {
			printf ("No Linux i386 Ramdisk Image\n");
			do_reset (cmdtp, flag, argc, argv);
		}

		/*
		 * Now check if we have a multifile image
		 */
	} else if ((hdr->ih_type==IH_TYPE_MULTI) && (len_ptr[1])) {
		ulong tail    = ntohl(len_ptr[0]) % 4;
		int i;

		/* skip kernel length and terminator */
		data = (ulong)(&len_ptr[2]);
		/* skip any additional image length fields */
		for (i=1; len_ptr[i]; ++i)
			data += 4;
		/* add kernel length, and align */
		data += ntohl(len_ptr[0]);
		if (tail) {
			data += 4 - tail;
		}

		len   = ntohl(len_ptr[1]);

	} else {
		/*
		 * no initrd image
		 */
		data = 0;
	}

#ifdef	DEBUG
	if (!data) {
		printf ("No initrd\n");
	}
#endif

	if (data) {
		initrd_start = data;
		initrd_end   = initrd_start + len;
		printf ("   Loading Ramdisk to %08lx, end %08lx ... ",
			initrd_start, initrd_end);
		memmove ((void *)initrd_start, (void *)data, len);
		printf ("OK\n");
	} else {
		initrd_start = 0;
		initrd_end = 0;
	}

	base_ptr = load_zimage((void*)addr + sizeof(image_header_t), ntohl(hdr->ih_size),
			       initrd_start, initrd_end-initrd_start, 0);

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
