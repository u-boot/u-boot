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
#ifndef	_CMD_NVEDIT_H
#define	_CMD_NVEDIT_H

#define	CMD_TBL_PRINTENV	MK_CMD_TBL_ENTRY(				\
	"printenv",	4,	CFG_MAXARGS,	1,	do_printenv,		\
	"printenv- print environment variables\n",				\
	"\n    - print values of all environment variables\n"			\
	"printenv name ...\n"							\
	"    - print value of environment variable 'name'\n"			\
),
int do_printenv (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#define CMD_TBL_SETENV		MK_CMD_TBL_ENTRY(				\
	"setenv",	6,	CFG_MAXARGS,	0,	do_setenv,		\
	"setenv  - set environment variables\n",				\
	"name value ...\n"							\
	"    - set environment variable 'name' to 'value ...'\n"		\
	"setenv name\n"								\
	"    - delete environment variable 'name'\n"				\
),
int do_setenv   (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#if ((CONFIG_COMMANDS & (CFG_CMD_ENV|CFG_CMD_FLASH)) == (CFG_CMD_ENV|CFG_CMD_FLASH))
#define	CMD_TBL_SAVEENV		MK_CMD_TBL_ENTRY(				\
	"saveenv",	4,	1,		0,	do_saveenv,		\
	"saveenv - save environment variables to persistent storage\n",		\
	NULL									\
),
int do_saveenv  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#else
#define CMD_TBL_SAVEENV
#endif	/* CFG_CMD_ENV */

#if (CONFIG_COMMANDS & CFG_CMD_ASKENV)
#define CMD_TBL_ASKENV		MK_CMD_TBL_ENTRY(				\
	"askenv",	8,	CFG_MAXARGS,	1,	do_askenv,		\
	"askenv  - get environment variables from stdin\n",			\
	"name [message] [size]\n"						\
	"    - get environment variable 'name' from stdin (max 'size' chars)\n"	\
	"askenv name\n"								\
	"    - get environment variable 'name' from stdin\n"			\
	"askenv name size\n"							\
	"    - get environment variable 'name' from stdin (max 'size' chars)\n"	\
	"askenv name [message] size\n"						\
	"    - display 'message' string and get environment variable 'name'"	\
	"from stdin (max 'size' chars)\n"					\
),
int do_askenv   (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#else
#define CMD_TBL_ASKENV
#endif	/* CFG_CMD_ASKENV */

#if (CONFIG_COMMANDS & CFG_CMD_RUN)
#define	CMD_TBL_RUN	MK_CMD_TBL_ENTRY(					\
	"run",	3,	CFG_MAXARGS,	1,	do_run,				\
	"run     - run commands in an environment variable\n",			\
	"var [...]\n"								\
	"    - run the commands in the environment variable(s) 'var'\n"		\
),
int do_run (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#else
#define CMD_TBL_RUN
#endif  /* CFG_CMD_RUN */

#endif	/* _CMD_NVEDIT_H */
