/*
 * (C) Copyright 2000
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
 * IDE support
 */
#ifndef	_CMD_IDE_H
#define	_CMD_IDE_H

#include <common.h>
#include <command.h>


#if (CONFIG_COMMANDS & CFG_CMD_IDE)
#define	CMD_TBL_IDE	MK_CMD_TBL_ENTRY(					\
	"ide",	3,	5,	1,	do_ide,					\
	"ide     - IDE sub-system\n",						\
	"reset - reset IDE controller\n"					\
	"ide info  - show available IDE devices\n"				\
	"ide device [dev] - show or set current device\n"			\
	"ide part [dev] - print partition table of one or all IDE devices\n"	\
	"ide read  addr blk# cnt\n"						\
	"ide write addr blk# cnt - read/write `cnt'"				\
	" blocks starting at block `blk#'\n"					\
	"    to/from memory address `addr'\n"					\
),

#define CMD_TBL_DISK	MK_CMD_TBL_ENTRY(					\
	"diskboot", 4,	3,	1,	do_diskboot,				\
	"diskboot- boot from IDE device\n",					\
	"loadAddr dev:part\n"							\
),

int do_ide (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_diskboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_IDE
#define CMD_TBL_DISK
#endif

#endif	/* _CMD_IDE_H */
