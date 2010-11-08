/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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

#if defined (CONFIG_SYS_NIOS_SYSID_BASE)

#include <command.h>
#include <asm/io.h>
#include <nios2-io.h>
#include <linux/time.h>

void display_sysid (void)
{
	struct nios_sysid_t *sysid = (struct nios_sysid_t *)CONFIG_SYS_NIOS_SYSID_BASE;
	struct tm t;
	char asc[32];
	time_t stamp;

	stamp = readl (&sysid->timestamp);
	localtime_r (&stamp, &t);
	asctime_r (&t, asc);
	printf ("SYSID : %08lx, %s", readl (&sysid->id), asc);

}

int do_sysid (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	display_sysid ();
	return (0);
}

U_BOOT_CMD(
	sysid,	1,	1,	do_sysid,
	"display Nios-II system id",
	""
);
#endif /* CONFIG_SYS_NIOS_SYSID_BASE */
