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
#ifndef	_CMD_BOOT_H
#define	_CMD_BOOT_H

#if (CONFIG_COMMANDS & CFG_CMD_BDI)
#define	CMD_TBL_BDINFO	MK_CMD_TBL_ENTRY(					\
	"bdinfo",	2,	1,	1,	do_bdinfo,			\
	"bdinfo  - print Board Info structure\n",				\
	NULL									\
),
int do_bdinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_BDINFO
#endif


#define CMD_TBL_GO	MK_CMD_TBL_ENTRY(					\
	"go",		2,	CFG_MAXARGS,	1,	do_go,			\
	"go      - start application at address 'addr'\n",			\
	"addr [arg ...]\n    - start application at address 'addr'\n"		\
	"      passing 'arg' as arguments\n"					\
),
int do_go (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#if (CONFIG_COMMANDS & CFG_CMD_LOADS)
#ifdef	CFG_LOADS_BAUD_CHANGE
#define	CMD_TBL_LOADS	MK_CMD_TBL_ENTRY(					\
	"loads",	5,	3,	0,	do_load_serial,			\
	"loads   - load S-Record file over serial line\n",			\
	"[ off ] [ baud ]\n"							\
	"    - load S-Record file over serial line"				\
	" with offset 'off' and baudrate 'baud'\n"				\
),
#else	/* ! CFG_LOADS_BAUD_CHANGE */
#define	CMD_TBL_LOADS	MK_CMD_TBL_ENTRY(					\
	"loads",	5,	2,	0,	do_load_serial,			\
	"loads   - load S-Record file over serial line\n",			\
	"[ off ]\n"								\
	"    - load S-Record file over serial line with offset 'off'\n"		\
),
#endif	/* CFG_LOADS_BAUD_CHANGE */
int do_load_serial (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/*
 * SAVES always requires LOADS support, but not vice versa
 */
#if (CONFIG_COMMANDS & CFG_CMD_SAVES)
#ifdef	CFG_LOADS_BAUD_CHANGE
#define	CMD_TBL_SAVES	MK_CMD_TBL_ENTRY(					\
	"saves",	5,	4,	0,	do_save_serial,			\
	"saves   - save S-Record file over serial line\n",			\
	"[ off ] [size] [ baud ]\n"							\
	"    - save S-Record file over serial line"				\
	" with offset 'off', size 'size' and baudrate 'baud'\n"				\
),
#else	/* ! CFG_LOADS_BAUD_CHANGE */
#define	CMD_TBL_SAVES	MK_CMD_TBL_ENTRY(					\
	"saves",	5,	3,	0,	do_save_serial,			\
	"saves   - save S-Record file over serial line\n",			\
	"[ off ] [size]\n"								\
	"    - save S-Record file over serial line with offset 'off' and size 'size'\n"		\
),
#endif	/* CFG_LOADS_BAUD_CHANGE */

int do_save_serial (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#else	/* ! CFG_CMD_SAVES */
#define CMD_TBL_SAVES
#endif	/* CFG_CMD_SAVES */

#else	/* ! CFG_CMD_LOADS */
#define CMD_TBL_LOADS
#define CMD_TBL_SAVES
#endif	/* CFG_CMD_LOADS */


#if (CONFIG_COMMANDS & CFG_CMD_LOADB)
#define	CMD_TBL_LOADB	MK_CMD_TBL_ENTRY(					\
	"loadb",	5,	3,	0,	do_load_serial_bin,		\
	"loadb   - load binary file over serial line (kermit mode)\n",		\
	"[ off ] [ baud ]\n"							\
	"    - load binary file over serial line"				\
	" with offset 'off' and baudrate 'baud'\n"				\
),
int do_load_serial_bin (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_LOADB
#endif	/* CFG_CMD_LOADB */

#define CMD_TBL_RESET	MK_CMD_TBL_ENTRY(					\
	"reset",	5,	1,	0,	do_reset,			\
	"reset   - Perform RESET of the CPU\n",					\
	NULL									\
),

/* Implemented in $(CPU)/cpu.c */
int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#if (CONFIG_COMMANDS & CFG_CMD_HWFLOW)
#define	CMD_TBL_HWFLOW	MK_CMD_TBL_ENTRY(					\
	"hwflow [on|off]",	2,	2,	0,	do_hwflow,			\
    "hwflow  - turn the harwdare flow control on/off\n", \
	"\n - change RTS/CTS hardware flow control over serial line\n" \
),

int do_hwflow (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_HWFLOW
#endif
#endif	/* _CMD_BOOT_H */
