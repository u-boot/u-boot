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
 * FLASH support
 */
#ifndef	_CMD_FLASH_H
#define	_CMD_FLASH_H

#if (CONFIG_COMMANDS & CFG_CMD_FLASH)
#define	CMD_TBL_FLINFO	MK_CMD_TBL_ENTRY(					\
	"flinfo",	3,	2,	1,	do_flinfo,			\
	"flinfo  - print FLASH memory information\n",				\
	"\n    - print information for all FLASH memory banks\n"		\
	"flinfo N\n    - print information for FLASH memory bank # N\n"		\
),

#define	CMD_TBL_FLERASE	MK_CMD_TBL_ENTRY(					\
	"erase",	3,	3,	1,	do_flerase,			\
	"erase   - erase FLASH memory\n",					\
	"start end\n"								\
	"    - erase FLASH from addr 'start' to addr 'end'\n"			\
	"erase N:SF[-SL]\n    - erase sectors SF-SL in FLASH bank # N\n"	\
	"erase bank N\n    - erase FLASH bank # N\n"				\
	"erase all\n    - erase all FLASH banks\n"				\
),

#define	CMD_TBL_PROTECT	MK_CMD_TBL_ENTRY(					\
	"protect",	4,	4,	1,	do_protect,			\
	"protect - enable or disable FLASH write protection\n",			\
	"on  start end\n"							\
	"    - protect FLASH from addr 'start' to addr 'end'\n"			\
	"protect on  N:SF[-SL]\n"						\
	"    - protect sectors SF-SL in FLASH bank # N\n"			\
	"protect on  bank N\n    - protect FLASH bank # N\n"			\
	"protect on  all\n    - protect all FLASH banks\n"			\
	"protect off start end\n"						\
	"    - make FLASH from addr 'start' to addr 'end' writable\n"		\
	"protect off N:SF[-SL]\n"						\
	"    - make sectors SF-SL writable in FLASH bank # N\n"			\
	"protect off bank N\n    - make FLASH bank # N writable\n"		\
	"protect off all\n    - make all FLASH banks writable\n"		\
),
int do_flinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_flerase(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_protect(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#else
#define CMD_TBL_FLINFO
#define CMD_TBL_FLERASE
#define CMD_TBL_PROTECT
#endif	/* CFG_CMD_FLASH */

#endif	/* _CMD_FLASH_H */
