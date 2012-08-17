/*
 * (C) Copyright 2000-2009
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

#include <config.h>

#ifndef NULL
#define NULL	0
#endif

/* Default to a width of 8 characters for help message command width */
#ifndef CONFIG_SYS_HELP_CMD_WIDTH
#define CONFIG_SYS_HELP_CMD_WIDTH	8
#endif

#ifndef	__ASSEMBLY__
/*
 * Monitor Command Table
 */

struct cmd_tbl_s {
	char		*name;		/* Command Name			*/
	int		maxargs;	/* maximum number of arguments	*/
	int		repeatable;	/* autorepeat allowed?		*/
					/* Implementation function	*/
	int		(*cmd)(struct cmd_tbl_s *, int, int, char * const []);
	char		*usage;		/* Usage message	(short)	*/
#ifdef	CONFIG_SYS_LONGHELP
	char		*help;		/* Help  message	(long)	*/
#endif
#ifdef CONFIG_AUTO_COMPLETE
	/* do auto completion on the arguments */
	int		(*complete)(int argc, char * const argv[], char last_char, int maxv, char *cmdv[]);
#endif
};

typedef struct cmd_tbl_s	cmd_tbl_t;

extern cmd_tbl_t  __u_boot_cmd_start;
extern cmd_tbl_t  __u_boot_cmd_end;

#if defined(CONFIG_CMD_RUN)
extern int do_run(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif

/* common/command.c */
int _do_help (cmd_tbl_t *cmd_start, int cmd_items, cmd_tbl_t * cmdtp, int
	      flag, int argc, char * const argv[]);
cmd_tbl_t *find_cmd(const char *cmd);
cmd_tbl_t *find_cmd_tbl (const char *cmd, cmd_tbl_t *table, int table_len);

extern int cmd_usage(const cmd_tbl_t *cmdtp);

#ifdef CONFIG_AUTO_COMPLETE
extern int var_complete(int argc, char * const argv[], char last_char, int maxv, char *cmdv[]);
extern int cmd_auto_complete(const char *const prompt, char *buf, int *np, int *colp);
#endif

/*
 * Monitor Command
 *
 * All commands use a common argument format:
 *
 * void function (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
 */

#if defined(CONFIG_CMD_MEMORY)		\
    || defined(CONFIG_CMD_I2C)		\
    || defined(CONFIG_CMD_ITEST)	\
    || defined(CONFIG_CMD_PCI)		\
    || defined(CONFIG_CMD_PORTIO)
#define CMD_DATA_SIZE
extern int cmd_get_data_size(char* arg, int default_size);
#endif

#ifdef CONFIG_CMD_BOOTD
extern int do_bootd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif
#ifdef CONFIG_CMD_BOOTM
extern int do_bootm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
extern int bootm_maybe_autostart(cmd_tbl_t *cmdtp, const char *cmd);
#else
static inline int bootm_maybe_autostart(cmd_tbl_t *cmdtp, const char *cmd)
{
	return 0;
}
#endif
extern int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

/*
 * Error codes that commands return to cmd_process(). We use the standard 0
 * and 1 for success and failure, but add one more case - failure with a
 * request to call cmd_usage(). But the cmd_process() function handles
 * CMD_RET_USAGE itself and after calling cmd_usage() it will return 1.
 * This is just a convenience for commands to avoid them having to call
 * cmd_usage() all over the place.
 */
enum command_ret_t {
	CMD_RET_SUCCESS,	/* 0 = Success */
	CMD_RET_FAILURE,	/* 1 = Failure */
	CMD_RET_USAGE = -1,	/* Failure, please report 'usage' error */
};

/**
 * Process a command with arguments. We look up the command and execute it
 * if valid. Otherwise we print a usage message.
 *
 * @param flag		Some flags normally 0 (see CMD_FLAG_.. above)
 * @param argc		Number of arguments (arg 0 must be the command text)
 * @param argv		Arguments
 * @param repeatable	This function sets this to 0 if the command is not
 *			repeatable. If the command is repeatable, the value
 *			is left unchanged.
 * @return 0 if the command succeeded, 1 if it failed
 */
int cmd_process(int flag, int argc, char * const argv[],
			       int *repeatable);

#endif	/* __ASSEMBLY__ */

/*
 * Command Flags:
 */
#define CMD_FLAG_REPEAT		0x0001	/* repeat last command		*/
#define CMD_FLAG_BOOTD		0x0002	/* command is from bootd	*/

#define Struct_Section  __attribute__((unused, section(".u_boot_cmd"), \
		aligned(4)))

#ifdef CONFIG_AUTO_COMPLETE
# define _CMD_COMPLETE(x) x,
#else
# define _CMD_COMPLETE(x)
#endif
#ifdef CONFIG_SYS_LONGHELP
# define _CMD_HELP(x) x,
#else
# define _CMD_HELP(x)
#endif

#define U_BOOT_CMD_MKENT_COMPLETE(name,maxargs,rep,cmd,usage,help,comp) \
	{#name, maxargs, rep, cmd, usage, _CMD_HELP(help) _CMD_COMPLETE(comp)}

#define U_BOOT_CMD_MKENT(name,maxargs,rep,cmd,usage,help) \
	U_BOOT_CMD_MKENT_COMPLETE(name,maxargs,rep,cmd,usage,help,NULL)

#define U_BOOT_CMD_COMPLETE(name,maxargs,rep,cmd,usage,help,comp) \
	cmd_tbl_t __u_boot_cmd_##name Struct_Section = \
		U_BOOT_CMD_MKENT_COMPLETE(name,maxargs,rep,cmd,usage,help,comp)

#define U_BOOT_CMD(name,maxargs,rep,cmd,usage,help) \
	U_BOOT_CMD_COMPLETE(name,maxargs,rep,cmd,usage,help,NULL)

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
void fixup_cmdtable(cmd_tbl_t *cmdtp, int size);
#endif

#endif	/* __COMMAND_H */
