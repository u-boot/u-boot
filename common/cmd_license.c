/*
 * (C) Copyright 2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
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

#if defined(CONFIG_CMD_LICENSE)

/* COPYING is currently 15951 bytes in size */
#define LICENSE_MAX	20480

#include <command.h>
#include <malloc.h>
#include <license.h>
int gunzip(void *, int, unsigned char *, unsigned long *);

int do_license(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *tok, *dst = malloc(LICENSE_MAX);
	unsigned long len = LICENSE_MAX;

	if (!dst)
		return -1;

	if (gunzip(dst, LICENSE_MAX, license_gz, &len) != 0) {
		printf("Error uncompressing license text\n");
		free(dst);
		return -1;
	}
	puts(dst);
	free(dst);

	return 0;
}

U_BOOT_CMD(license, 1, 1, do_license,
	"print GPL license text",
	""
);

#endif /* CONFIG_CMD_LICENSE */
