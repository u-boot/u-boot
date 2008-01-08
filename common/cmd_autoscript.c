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
autoscript (ulong addr)
{
	ulong len;
	image_header_t *hdr = (image_header_t *)addr;
	ulong *len_ptr;
	char *cmd;
	int rcode = 0;
	int verify;

	verify = getenv_verify ();

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
	len_ptr = (ulong *)image_get_data (hdr);

	if ((len = image_to_cpu (*len_ptr)) == 0) {
		puts ("Empty Script\n");
		return 1;
	}

	debug ("** Script length: %ld\n", len);

	if ((cmd = malloc (len + 1)) == NULL) {
		return 1;
	}

	while (*len_ptr++);

	/* make sure cmd is null terminated */
	memmove (cmd, (char *)len_ptr, len);
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

	if (argc < 2) {
		addr = CFG_LOAD_ADDR;
	} else {
		addr = simple_strtoul (argv[1],0,16);
	}

	printf ("## Executing script at %08lx\n",addr);
	rcode = autoscript (addr);
	return rcode;
}

#if defined(CONFIG_CMD_AUTOSCRIPT)
U_BOOT_CMD(
	autoscr, 2, 0,	do_autoscript,
	"autoscr - run script from memory\n",
	"[addr] - run script starting at addr"
	" - A valid autoscr header must be present\n"
);
#endif

#endif
