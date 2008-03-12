/*
 * (C) Copyright 2001
 * Kyle Harris, kharris@nexus-tech.net
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

/*
 * autoscript allows a remote host to download a command file and,
 * optionally, binary data for automatically updating the target. For
 * example, you create a new kernel image and want the user to be
 * able to simply download the image and the machine does the rest.
 * The kernel image is postprocessed with mkimage, which creates an
 * image with a script file prepended. If enabled, autoscript will
 * verify the script and contents of the download and execute the
 * script portion. This would be responsible for erasing flash,
 * copying the new image, and rebooting the machine.
 */

/* #define DEBUG */

#include <common.h>
#include <command.h>
#include <image.h>
#include <malloc.h>
#include <asm/byteorder.h>
#if defined(CONFIG_8xx)
#include <mpc8xx.h>
#endif
#ifdef CFG_HUSH_PARSER
#include <hush.h>
#endif

#if defined(CONFIG_AUTOSCRIPT) || defined(CONFIG_CMD_AUTOSCRIPT)

int
autoscript (ulong addr, const char *fit_uname)
{
	ulong 		len;
	image_header_t	*hdr;
	ulong		*data;
	char		*cmd;
	int		rcode = 0;
	int		verify;
#if defined(CONFIG_FIT)
	const void*	fit_hdr;
	int		noffset;
	const void	*fit_data;
	size_t		fit_len;
#endif

	verify = getenv_verify ();

	switch (genimg_get_format ((void *)addr)) {
	case IMAGE_FORMAT_LEGACY:
		hdr = (image_header_t *)addr;

		if (!image_check_magic (hdr)) {
			puts ("Bad magic number\n");
			return 1;
		}

		if (!image_check_hcrc (hdr)) {
			puts ("Bad header crc\n");
			return 1;
		}

		if (verify) {
			if (!image_check_dcrc (hdr)) {
				puts ("Bad data crc\n");
				return 1;
			}
		}

		if (!image_check_type (hdr, IH_TYPE_SCRIPT)) {
			puts ("Bad image type\n");
			return 1;
		}

		/* get length of script */
		data = (ulong *)image_get_data (hdr);

		if ((len = uimage_to_cpu (*data)) == 0) {
			puts ("Empty Script\n");
			return 1;
		}
		break;
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		if (fit_uname == NULL) {
			puts ("No FIT subimage unit name\n");
			return 1;
		}

		fit_hdr = (const void *)addr;
		if (!fit_check_format (fit_hdr)) {
			puts ("Bad FIT image format\n");
			return 1;
		}

		/* get script component image node offset */
		noffset = fit_image_get_node (fit_hdr, fit_uname);
		if (noffset < 0) {
			printf ("Can't find '%s' FIT subimage\n", fit_uname);
			return 1;
		}

		if (!fit_image_check_type (fit_hdr, noffset, IH_TYPE_SCRIPT)) {
			puts ("Not a image image\n");
			return 1;
		}

		/* verify integrity */
		if (verify) {
			if (!fit_image_check_hashes (fit_hdr, noffset)) {
				puts ("Bad Data Hash\n");
				return 1;
			}
		}

		/* get script subimage data address and length */
		if (fit_image_get_data (fit_hdr, noffset, &fit_data, &fit_len)) {
			puts ("Could not find script subimage data\n");
			return 1;
		}

		data = (ulong *)fit_data;
		len = (ulong)fit_len;
		break;
#endif
	default:
		puts ("Wrong image format for autoscript\n");
		return 1;
	}

	debug ("** Script length: %ld\n", len);

	if ((cmd = malloc (len + 1)) == NULL) {
		return 1;
	}

	while (*data++);

	/* make sure cmd is null terminated */
	memmove (cmd, (char *)data, len);
	*(cmd + len) = 0;

#ifdef CFG_HUSH_PARSER /*?? */
	rcode = parse_string_outer (cmd, FLAG_PARSE_SEMICOLON);
#else
	{
		char *line = cmd;
		char *next = cmd;

		/*
		 * break into individual lines,
		 * and execute each line;
		 * terminate on error.
		 */
		while (*next) {
			if (*next == '\n') {
				*next = '\0';
				/* run only non-empty commands */
				if ((next - line) > 1) {
					debug ("** exec: \"%s\"\n",
						line);
					if (run_command (line, 0) < 0) {
						rcode = 1;
						break;
					}
				}
				line = next + 1;
			}
			++next;
		}
	}
#endif
	free (cmd);
	return rcode;
}

#endif

/**************************************************/
#if defined(CONFIG_CMD_AUTOSCRIPT)
int
do_autoscript (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong addr;
	int rcode;
	const char *fit_uname = NULL;

	/* Find script image */
	if (argc < 2) {
		addr = CFG_LOAD_ADDR;
		debug ("*  autoscr: default load address = 0x%08lx\n", addr);
#if defined(CONFIG_FIT)
	} else if (fit_parse_subimage (argv[1], load_addr, &addr, &fit_uname)) {
		debug ("*  autoscr: subimage '%s' from FIT image at 0x%08lx\n",
				fit_uname, addr);
#endif
	} else {
		addr = simple_strtoul(argv[1], NULL, 16);
		debug ("*  autoscr: cmdline image address = 0x%08lx\n", addr);
	}

	printf ("## Executing script at %08lx\n", addr);
	rcode = autoscript (addr, fit_uname);
	return rcode;
}

U_BOOT_CMD(
	autoscr, 2, 0,	do_autoscript,
	"autoscr - run script from memory\n",
	"[addr] - run script starting at addr"
	" - A valid autoscr header must be present\n"
#if defined(CONFIG_FIT)
	"For FIT format uImage addr must include subimage\n"
	"unit name in the form of addr:<subimg_uname>\n"
#endif
);
#endif
