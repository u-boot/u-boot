/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * Diagnostics support
 */
#ifndef	_CMD_DIAG_H
#define	_CMD_DIAG_H

#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_DIAG) && defined(CONFIG_POST)
#define	CMD_TBL_DIAG	MK_CMD_TBL_ENTRY(				\
	"diag",	4,	CFG_MAXARGS,	0,	do_diag,		\
	"diag    - perform board diagnostics\n",			\
	     "    - print list of available tests\n"			\
	"diag [test1 [test2]]\n"					\
	"         - print information about specified tests\n"		\
	"diag run - run all available tests\n"				\
	"diag run [test1 [test2]]\n"					\
	"         - run specified tests\n"				\
),

int do_diag (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#else
#define CMD_TBL_DIAG
#endif	/* CFG_CMD_DIAG */

#endif	/* _CMD_DIAG_H */
