/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
