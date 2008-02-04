/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2003
 * Kai-Uwe Bloem, Auerswald GmbH & Co KG, <linux-development@auerswald.de>
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

#if defined(CONFIG_CMD_XIMG)

/*
 * Multi Image extract
 */
#include <common.h>
#include <command.h>
#include <image.h>
#include <asm/byteorder.h>

int
do_imgextract(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong addr = load_addr, dest = 0;
	ulong data, len;
	ulong *len_ptr;
	int i, verify, part = 0;
	char pbuf[10], *s;
	image_header_t *hdr;

	verify = getenv_verify ();

	if (argc > 1) {
		addr = simple_strtoul(argv[1], NULL, 16);
	}
	if (argc > 2) {
		part = simple_strtoul(argv[2], NULL, 16);
	}
	if (argc > 3) {
		dest = simple_strtoul(argv[3], NULL, 16);
	}


	switch (gen_image_get_format ((void *)addr)) {
	case IMAGE_FORMAT_LEGACY:

		printf("## Copying from legacy image at %08lx ...\n", addr);
		hdr = (image_header_t *)addr;
		if (!image_check_magic (hdr)) {
			printf("Bad Magic Number\n");
			return 1;
		}

		if (!image_check_hcrc (hdr)) {
			printf("Bad Header Checksum\n");
			return 1;
		}
	#ifdef DEBUG
		image_print_contents (hdr);
	#endif

		if (!image_check_type (hdr, IH_TYPE_MULTI)) {
			printf("Wrong Image Type for %s command\n",
					cmdtp->name);
			return 1;
		}

		if (image_get_comp (hdr) != IH_COMP_NONE) {
			printf("Wrong Compression Type for %s command\n",
					cmdtp->name);
			return 1;
		}

		if (verify) {
			printf("   Verifying Checksum ... ");
			if (!image_check_dcrc (hdr)) {
				printf("Bad Data CRC\n");
				return 1;
			}
			printf("OK\n");
		}

		data = image_get_data (hdr);
		len_ptr = (ulong *) data;

		data += 4;		/* terminator */
		for (i = 0; len_ptr[i]; ++i) {
			data += 4;
			if (argc > 2 && part > i) {
				u_long tail;
				len = image_to_cpu (len_ptr[i]);
				tail = len % 4;
				data += len;
				if (tail) {
					data += 4 - tail;
				}
			}
		}
		if (argc > 2 && part >= i) {
			printf("Bad Image Part\n");
			return 1;
		}
		len = image_to_cpu (len_ptr[part]);
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		fit_unsupported ("imxtract");
		return 1;
#endif
	default:
		puts ("Invalid image type for imxtract\n");
		return 1;
	}

	if (argc > 3) {
		memcpy((char *) dest, (char *) data, len);
	}

	sprintf(pbuf, "%8lx", data);
	setenv("fileaddr", pbuf);
	sprintf(pbuf, "%8lx", len);
	setenv("filesize", pbuf);

	return 0;
}

U_BOOT_CMD(imxtract, 4, 1, do_imgextract,
	   "imxtract- extract a part of a multi-image\n",
	   "addr part [dest]\n"
	   "    - extract <part> from image at <addr> and copy to <dest>\n");

#endif
