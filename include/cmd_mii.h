/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
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
 * MII Functions
 */
#ifndef	_CMD_MII_H
#define _CMD_MII_H

#if (CONFIG_COMMANDS & CFG_CMD_MII)
#define CMD_TBL_MII	MK_CMD_TBL_ENTRY(				\
	"mii",		3,	5,	1,	do_mii,			\
	"mii     - MII utility commands\n",				\
	"\
info  <addr>              - display MII PHY info\n\
mii read  <addr> <reg>        - read  MII PHY <addr> register <reg>\n\
mii write <addr> <reg> <data> - write MII PHY <addr> register <reg>\n"	\
),

int do_mii       (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_MII
#endif	/* CFG_CMD_MII */

#endif	/* _CMD_MII_H */
