/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
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
 * Dos floppy support
 */
#ifndef	_CMD_FDOS_H
#define	_CMD_FDOS_H

#include <common.h>
#include <command.h>


#if (CONFIG_COMMANDS & CFG_CMD_FDOS)

#define CMD_TBL_FDOS_BOOT	MK_CMD_TBL_ENTRY(       \
	"fdosboot", 5,	3,	0,	do_fdosboot,    \
	"fdosboot- boot from a dos floppy file\n",     \
	"[loadAddr] [filename]\n"                       \
),
#define CMD_TBL_FDOS_LS	        MK_CMD_TBL_ENTRY(       \
	"fdosls", 5,	2,	0,	do_fdosls,      \
	"fdosls  - list files in a directory\n",       \
	"[directory]\n"                                 \
),
int do_fdosboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_fdosls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_FDOS_BOOT
#define CMD_TBL_FDOS_LS
#endif

#endif	/* _CMD_FDOS_H */
