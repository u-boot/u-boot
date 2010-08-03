/*
 * Copyright 2010 Freescale Semiconductor, Inc.
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
#include <linux/compiler.h>
#include <asm/processor.h>

static int do_errata(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	__maybe_unused u32 svr = get_svr();

#if defined(CONFIG_FSL_SATA_V2) && defined(CONFIG_FSL_SATA_ERRATUM_A001)
	if (IS_SVR_REV(svr, 1, 0)) {
		switch (SVR_SOC_VER(svr)) {
		case SVR_P1013:
		case SVR_P1013_E:
		case SVR_P1022:
		case SVR_P1022_E:
			puts("Work-around for Erratum SATA A001 enabled\n");
		}
	}
#endif

#if defined(CONFIG_SYS_P4080_ERRATUM_SERDES8)
	puts("Work-around for Erratum SERDES8 enabled\n");
#endif
#if defined(CONFIG_SYS_P4080_ERRATUM_CPU22)
	puts("Work-around for Erratum CPU22 enabled\n");
#endif
	return 0;
}

U_BOOT_CMD(
	errata, 1, 0,	do_errata,
	"Report errata workarounds",
	""
);
