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
 * Memory Functions
 */
#ifndef	_CMD_MEM_H
#define _CMD_MEM_H

#if (CONFIG_COMMANDS & CFG_CMD_MEMORY)
#define CMD_TBL_MD	MK_CMD_TBL_ENTRY(					\
	"md",		2,	3,	1,	do_mem_md,			\
	"md      - memory display\n",						\
	"[.b, .w, .l] address [# of objects]\n    - memory display\n"		\
),
#define CMD_TBL_MM	MK_CMD_TBL_ENTRY(					\
 	"mm",		2,	2,	1,	do_mem_mm,			\
	"mm      - memory modify (auto-incrementing)\n",			\
	"[.b, .w, .l] address\n"						\
	"    - memory modify, auto increment address\n"				\
),
#define CMD_TBL_NM	MK_CMD_TBL_ENTRY(					\
	"nm",		2,	2,	1,	do_mem_nm,			\
	"nm      - memory modify (constant address)\n",				\
	"[.b, .w, .l] address\n    - memory modify, read and keep address\n"	\
),
#define CMD_TBL_MW	MK_CMD_TBL_ENTRY(					\
	"mw",		2,	4,	1,	do_mem_mw,			\
	"mw      - memory write (fill)\n",					\
	"[.b, .w, .l] address value [count]\n    - write memory\n"		\
),
#define	CMD_TBL_CP	MK_CMD_TBL_ENTRY(					\
	"cp",		2,	4,	1,	do_mem_cp,			\
	"cp      - memory copy\n",						\
	"[.b, .w, .l] source target count\n    - copy memory\n"			\
),
#define	CMD_TBL_CMP	MK_CMD_TBL_ENTRY(					\
	"cmp",		3,	4,	1,	do_mem_cmp,			\
	"cmp     - memory compare\n",						\
	"[.b, .w, .l] addr1 addr2 count\n    - compare memory\n"		\
),
#define	CMD_TBL_CRC	MK_CMD_TBL_ENTRY(					\
	"crc32",	3,	4,	1,	do_mem_crc,			\
	"crc32   - checksum calculation\n",					\
	"address count [addr]\n    - compute CRC32 checksum [save at addr]\n"	\
),
#define CMD_TBL_BASE	MK_CMD_TBL_ENTRY(					\
	"base",		2,	2,	1,	do_mem_base,			\
	"base    - print or set address offset\n",				\
	"\n    - print address offset for memory commands\n"			\
	"base off\n    - set address offset for memory commands to 'off'\n"	\
),
/*
 * Require full name for "loop" and "mtest" because these are infinite loops!
 */
#define CMD_TBL_LOOP	MK_CMD_TBL_ENTRY(					\
	"loop",		4,	3,	1,	do_mem_loop,			\
	"loop    - infinite loop on address range\n",				\
	"[.b, .w, .l] address number_of_objects\n"				\
	"    - loop on a set of addresses\n"					\
),
#define CMD_TBL_MTEST	MK_CMD_TBL_ENTRY(					\
	"mtest",	5,	4,	1,	do_mem_mtest,			\
	"mtest   - simple RAM test\n",						\
	"[start [end [pattern]]]\n"						\
	"    - simple RAM read/write test\n"					\
),
int do_mem_md    (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_mem_mm    (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_mem_nm    (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_mem_mw    (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_mem_cp    (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_mem_cmp   (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_mem_crc   (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_mem_base  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_mem_loop  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_mem_mtest (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_MD
#define CMD_TBL_MM
#define CMD_TBL_NM
#define CMD_TBL_MW
#define CMD_TBL_CP
#define CMD_TBL_CMP
#define CMD_TBL_CRC
#define CMD_TBL_BASE
#define CMD_TBL_LOOP
#define CMD_TBL_MTEST
#endif	/* CFG_CMD_MEMORY */

#endif	/* _CMD_MEM_H */
