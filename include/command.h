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
 *  Definitions for Command Processor
 */
#ifndef __COMMAND_H
#define __COMMAND_H

#ifndef NULL
#define NULL	0
#endif

#ifndef	__ASSEMBLY__
/*
 * Monitor Command Table
 */

struct cmd_tbl_s {
	char		*name;		/* Command Name			*/
	int		lmin;		/* minimum abbreviated length	*/
	int		maxargs;	/* maximum number of arguments	*/
	int		repeatable;	/* autorepeat allowed?		*/
					/* Implementation function	*/
	int		(*cmd)(struct cmd_tbl_s *, int, int, char *[]);
	char		*usage;		/* Usage message	(short)	*/
#ifdef	CFG_LONGHELP
	char		*help;		/* Help  message	(long)	*/
#endif
};

typedef struct cmd_tbl_s	cmd_tbl_t;

extern	cmd_tbl_t cmd_tbl[];

#ifdef	CFG_LONGHELP
#define	MK_CMD_TBL_ENTRY(name,lmin,maxargs,rep,cmd,usage,help)	\
				{ name, lmin, maxargs, rep, cmd, usage, help }
#else	/* no help info */
#define	MK_CMD_TBL_ENTRY(name,lmin,maxargs,rep,cmd,usage,help)	\
				{ name, lmin, maxargs, rep, cmd, usage }
#endif

/* common/command.c */
cmd_tbl_t *find_cmd(const char *cmd);

/*
 * Monitor Command
 *
 * All commands use a common argument format:
 *
 * void function (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
 */

typedef	void 	command_t (cmd_tbl_t *, int, int, char *[]);

#endif	/* __ASSEMBLY__ */

/*
 * Command Flags:
 */
#define CMD_FLAG_REPEAT		0x0001	/* repeat last command		*/
#define CMD_FLAG_BOOTD		0x0002	/* command is from bootd	*/

/*
 * Configurable monitor commands definitions have been moved
 * to include/cmd_confdefs.h
 */

#endif	/* __COMMAND_H */
