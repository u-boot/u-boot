/*
 * (C) Copyright 2002
 * Richard Jones, rjones@nexus-tech.net
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
 * FAT support
 */
#ifndef	_CMD_FAT_H
#define	_CMD_FAT_H

#if (CONFIG_COMMANDS & CFG_CMD_FAT)

int do_fat_fsload (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_fat_fsinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_fat_ls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_fat_dump (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#define	CMD_TBL_FAT	MK_CMD_TBL_ENTRY(				\
	"fatload",	5,	4,	0,	do_fat_fsload,		\
	"fatload - load binary file from a dos filesystem\n",		\
	"[ off ] [ filename ]\n"					\
	"    - load binary file from dos filesystem\n"			\
	"      with offset 'off'\n"					\
),									\
    	MK_CMD_TBL_ENTRY(						\
	"fatinfo",	5,	1,	1,	do_fat_fsinfo,		\
	"fatinfo - print information about filesystem\n",		\
	"\n"								\
	"    - print information about filesystem\n"			\
),									\
	MK_CMD_TBL_ENTRY(						\
	"fatls",	2,	2,	1,	do_fat_ls,		\
	"fatls   - list files in a directory (default /)\n",		\
	"[ directory ]\n"						\
	"    - list files in a directory.\n"				\
),

#else
#define CMD_TBL_FAT
#endif	/* CFG_CMD_FAT */

#endif	/* _CMD_FAT_H */
