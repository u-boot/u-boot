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
 * Network boot support
 */
#ifndef	_CMD_CONSOLE_H
#define	_CMD_CONSOLE_H

#if (CONFIG_COMMANDS & CFG_CMD_CONSOLE)
#define	CMD_TBL_CONINFO	MK_CMD_TBL_ENTRY(			\
	"coninfo",	5,	3,	1,	do_coninfo,	\
	"coninfo - print console devices and information\n",	\
	""		 		 			\
),
int do_coninfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define	CMD_TBL_CONINFO
#endif	/* CFG_CMD_CONSOLE */

#endif
