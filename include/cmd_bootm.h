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
 * Boot support
 */
#ifndef	_CMD_BOOTM_H
#define	_CMD_BOOTM_H
int do_bootm (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#define	CMD_TBL_BOOTM	MK_CMD_TBL_ENTRY(					\
	"bootm",	5,	CFG_MAXARGS,	1,	do_bootm,		\
	"bootm   - boot application image from memory\n",			\
	"[addr [arg ...]]\n    - boot application image stored in memory\n"	\
	"        passing arguments 'arg ...'; when booting a Linux kernel,\n"	\
	"        'arg' can be the address of an initrd image\n"			\
),

#if (CONFIG_COMMANDS & CFG_CMD_BOOTD)
int do_bootd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#define CMD_TBL_BOOTD	MK_CMD_TBL_ENTRY(					\
	"bootd",	4,	1,	1,	do_bootd,			\
	"bootd   - boot default, i.e., run 'bootcmd'\n",			\
	NULL									\
),
#else
#define CMD_TBL_BOOTD
#endif

#if (CONFIG_COMMANDS & CFG_CMD_IMI)
int do_iminfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#define	CMD_TBL_IMINFO	MK_CMD_TBL_ENTRY(					\
	"iminfo",	3,	CFG_MAXARGS,	1,	do_iminfo,		\
	"iminfo  - print header information for application image\n",		\
	"addr [addr ...]\n"							\
	"    - print header information for application image starting at\n"	\
	"      address 'addr' in memory; this includes verification of the\n"	\
	"      image contents (magic number, header and payload checksums)\n"	\
),
#else
#define CMD_TBL_IMINFO
#endif

#endif	/* _CMD_BOOTM_H */
