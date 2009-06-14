/*
 * (C) Copyright 2009 mGine co.
 * unsik Kim <donari75@gmail.com>
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
#include <command.h>

#if defined (CONFIG_CMD_MG_DISK)

#include <mg_disk.h>

int do_mg_disk_cmd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 from, to, size;

	switch (argc) {
	case 2:
		if (!strcmp(argv[1], "init"))
			mg_disk_init();
		else
			return 1;
		break;
	case 5:
		from = simple_strtoul(argv[2], NULL, 0);
		to = simple_strtoul(argv[3], NULL, 0);
		size = simple_strtoul(argv[4], NULL, 0);

		if (!strcmp(argv[1], "read"))
			mg_disk_read(from, (u8 *)to, size);
		else if (!strcmp(argv[1], "write"))
			mg_disk_write(to, (u8 *)from, size);
		else if (!strcmp(argv[1], "readsec"))
			mg_disk_read_sects((void *)to, from, size);
		else if (!strcmp(argv[1], "writesec"))
			mg_disk_write_sects((void *)from, to, size);
		else
			return 1;
		break;
	default:
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	return 0;
}

U_BOOT_CMD(
	mgd,	5,	0,	do_mg_disk_cmd,
	"mgd     - mgine m[g]flash command\n",
	": mgine mflash IO mode (disk) command\n"
	"    - initialize : mgd init\n"
	"    - random read : mgd read [from] [to] [size]\n"
	"    - random write : mgd write [from] [to] [size]\n"
	"    - sector read : mgd readsec [sector] [to] [counts]\n"
	"    - sector write : mgd writesec [from] [sector] [counts]"
);

#endif
