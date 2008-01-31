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
	ulong initrd_start, initrd_end;

	get_ramdisk (cmdtp, flag, argc, argv, hdr, verify,
			IH_ARCH_I386, &initrd_start, &initrd_end);

	/* if multi-part image, we need to advance base ptr */
	if (image_check_type (hdr, IH_TYPE_MULTI)) {
		image_multi_getimg (hdr, 0, &os_data, &os_len);
	} else {
		os_data = image_get_data (hdr);
		os_len = image_get_data_size (hdr);
	}

	base_ptr = load_zimage ((void*)os_data, os_len,
			initrd_start, initrd_end - initrd_start, 0);

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
