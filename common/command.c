/*
 * (C) Copyright 2000-2003
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
 *  Command Processor Table
 */

#include <common.h>
#include <command.h>

int
do_version (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	extern char version_string[];
	printf ("\n%s\n", version_string);
	return 0;
}

U_BOOT_CMD(
	version,	1,		1,	do_version,
 	"version - print monitor version\n",
	NULL
);

int
do_echo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i, putnl = 1;

	for (i = 1; i < argc; i++) {
		char *p = argv[i], c;

		if (i > 1)
			putc(' ');
		while ((c = *p++) != '\0') {
			if (c == '\\' && *p == 'c') {
				putnl = 0;
				p++;
			} else {
				putc(c);
			}
		}
	}

	if (putnl)
		putc('\n');
	return 0;
}

U_BOOT_CMD(
	echo,	CFG_MAXARGS,	1,	do_echo,
 	"echo    - echo args to console\n",
 	"[args..]\n"
	"    - echo args to console; \\c suppresses newline\n"
);

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */
int do_help (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i;
	int rcode = 0;

	if (argc == 1) {	/*show list of commands */

		int cmd_items = &__u_boot_cmd_end -
				&__u_boot_cmd_start;	/* pointer arith! */
		cmd_tbl_t *cmd_array[cmd_items];
		int i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = &__u_boot_cmd_start;
		for (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		for (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			for (j = 0; j < i; ++j) {
				if (strcmp (cmd_array[j]->name,
					    cmd_array[j + 1]->name) > 0) {
					cmd_tbl_t *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			if (!swaps)
				break;
		}

		/* print short help (usage) */
		for (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			/* allow user abort */
			if (ctrlc ())
				return 1;
			if (usage == NULL)
				continue;
			puts (usage);
		}
		return 0;
	}
	/*
	 * command help (long version)
	 */
	for (i = 1; i < argc; ++i) {
		if ((cmdtp = find_cmd (argv[i])) != NULL) {
#ifdef	CFG_LONGHELP
			/* found - print (long) help info */
			puts (cmdtp->name);
			putc (' ');
			if (cmdtp->help) {
				puts (cmdtp->help);
			} else {
				puts ("- No help available.\n");
				rcode = 1;
			}
			putc ('\n');
#else	/* no long help available */
			if (cmdtp->usage)
				puts (cmdtp->usage);
#endif	/* CFG_LONGHELP */
		} else {
			printf ("Unknown command '%s' - try 'help'"
				" without arguments for list of all"
				" known commands\n\n", argv[i]
					);
			rcode = 1;
		}
	}
	return rcode;
}


U_BOOT_CMD(
	help,	CFG_MAXARGS,	1,	do_help,
 	"help    - print online help\n",
 	"[command ...]\n"
 	"    - show help information (for 'command')\n"
 	"'help' prints online help for the monitor commands.\n\n"
 	"Without arguments, it prints a short usage message for all commands.\n\n"
 	"To get detailed help information for specific commands you can type\n"
  "'help' with one or more command names as arguments.\n"
);

/* This do not ust the U_BOOT_CMD macro as ? can't be used in symbol names */
#ifdef  CFG_LONGHELP
cmd_tbl_t __u_boot_cmd_question_mark Struct_Section = {
	"?",	CFG_MAXARGS,	1,	do_help,
 	"?       - alias for 'help'\n",
	NULL
};
#else
cmd_tbl_t __u_boot_cmd_question_mark Struct_Section = {
	"?",	CFG_MAXARGS,	1,	do_help,
 	"?       - alias for 'help'\n"
};
#endif /* CFG_LONGHELP */

/***************************************************************************
 * find command table entry for a command
 */
cmd_tbl_t *find_cmd (const char *cmd)
{
	cmd_tbl_t *cmdtp;
	cmd_tbl_t *cmdtp_temp = &__u_boot_cmd_start;	/*Init value */
	const char *p;
	int len;
	int n_found = 0;

	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	for (cmdtp = &__u_boot_cmd_start;
	     cmdtp != &__u_boot_cmd_end;
	     cmdtp++) {
		if (strncmp (cmd, cmdtp->name, len) == 0) {
			if (len == strlen (cmdtp->name))
				return cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	if (n_found == 1) {			/* exactly one match */
		return cmdtp_temp;
	}

	return NULL;	/* not found or ambiguous command */
}
