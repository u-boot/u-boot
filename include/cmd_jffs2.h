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
 * JFFS2 support
 */
#ifndef	_CMD_JFFS2_H
#define	_CMD_JFFS2_H

#if (CONFIG_COMMANDS & CFG_CMD_JFFS2)

#define	CMD_TBL_JFFS2_FSLOAD	MK_CMD_TBL_ENTRY(			\
	"fsload",	5,	3,	0,	do_jffs2_fsload,	\
	"fsload  - load binary file from a filesystem image\n",		\
	"[ off ] [ filename ]\n"					\
	"    - load binary file from flash bank\n"			\
	"      with offset 'off'\n"					\
),

#define CMD_TBL_JFFS2_FSINFO   	MK_CMD_TBL_ENTRY(			\
	"fsinfo",	5,	1,	1,	do_jffs2_fsinfo,	\
	"fsinfo  - print information about filesystems\n",		\
	"    - print information about filesystems\n"			\
),

#define CMD_TBL_JFFS2_LS	MK_CMD_TBL_ENTRY(			\
	"ls",		2,	2,	1,	do_jffs2_ls,		\
	"ls      - list files in a directory (default /)\n",		\
	"[ directory ]\n"						\
	"    - list files in a directory.\n"				\
),

#define CMD_TBL_JFFS2_CHPART   	MK_CMD_TBL_ENTRY(			\
	"chpart",	6,	2,	0,	do_jffs2_chpart,	\
	"chpart  - change active partition\n",				\
	"    - change active partition\n"				\
),

int do_jffs2_fsload (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_jffs2_fsinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_jffs2_ls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_jffs2_chpart (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#else
#define	CMD_TBL_JFFS2_FSLOAD
#define CMD_TBL_JFFS2_FSINFO
#define CMD_TBL_JFFS2_LS
#define CMD_TBL_JFFS2_CHPART
#endif	/* CFG_CMD_JFFS2 */

#endif	/* _CMD_JFFS2_H */
