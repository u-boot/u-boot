/*
 * Copyright (C) 2012 Boundary Devices Inc.
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
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/imx-common/boot_mode.h>
#include <malloc.h>

static const struct boot_mode *modes[2];

static const struct boot_mode *search_modes(char *arg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		const struct boot_mode *p = modes[i];
		if (p) {
			while (p->name) {
				if (!strcmp(p->name, arg))
					return p;
				p++;
			}
		}
	}
	return NULL;
}

static int create_usage(char *dest)
{
	int i;
	int size = 0;

	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		const struct boot_mode *p = modes[i];
		if (p) {
			while (p->name) {
				int len = strlen(p->name);
				if (dest) {
					memcpy(dest, p->name, len);
					dest += len;
					*dest++ = '|';
				}
				size += len + 1;
				p++;
			}
		}
	}
	if (dest)
		memcpy(dest - 1, " [noreset]", 11);	/* include trailing 0 */
	size += 10;
	return size;
}

static int do_boot_mode(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	const struct boot_mode *p;
	int reset_requested = 1;

	if (argc < 2)
		return CMD_RET_USAGE;
	p = search_modes(argv[1]);
	if (!p)
		return CMD_RET_USAGE;
	if (argc == 3) {
		if (strcmp(argv[2], "noreset"))
			return CMD_RET_USAGE;
		reset_requested = 0;
	}

	boot_mode_apply(p->cfg_val);
	if (reset_requested && p->cfg_val)
		do_reset(NULL, 0, 0, NULL);
	return 0;
}

U_BOOT_CMD(
	bmode, 3, 0, do_boot_mode,
	NULL,
	"");

void add_board_boot_modes(const struct boot_mode *p)
{
	int size;
	char *dest;

	if (__u_boot_cmd_bmode.usage) {
		free(__u_boot_cmd_bmode.usage);
		__u_boot_cmd_bmode.usage = NULL;
	}

	modes[0] = p;
	modes[1] = soc_boot_modes;
	size = create_usage(NULL);
	dest = malloc(size);
	if (dest) {
		create_usage(dest);
		__u_boot_cmd_bmode.usage = dest;
	}
}
