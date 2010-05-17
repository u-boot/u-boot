/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/**************************************************************************
 *
 * Support for persistent environment data
 *
 * The "environment" is stored as a list of '\0' terminated
 * "name=value" strings. The end of the list is marked by a double
 * '\0'. New entries are always added at the end. Deleting an entry
 * shifts the remaining entries to the front. Replacing an entry is a
 * combination of deleting the old value and adding the new one.
 *
 * The environment is preceeded by a 32 bit CRC over the data part.
 *
 **************************************************************************
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#if defined(CONFIG_CMD_EDITENV)
#include <malloc.h>
#endif
#include <watchdog.h>
#include <serial.h>
#include <linux/stddef.h>
#include <asm/byteorder.h>
#if defined(CONFIG_CMD_NET)
#include <net.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_ENV_IS_IN_EEPROM)	&& \
    !defined(CONFIG_ENV_IS_IN_FLASH)	&& \
    !defined(CONFIG_ENV_IS_IN_DATAFLASH)	&& \
    !defined(CONFIG_ENV_IS_IN_MG_DISK)	&& \
    !defined(CONFIG_ENV_IS_IN_MMC)  && \
    !defined(CONFIG_ENV_IS_IN_NAND)	&& \
    !defined(CONFIG_ENV_IS_IN_NVRAM)	&& \
    !defined(CONFIG_ENV_IS_IN_ONENAND)	&& \
    !defined(CONFIG_ENV_IS_IN_SPI_FLASH)	&& \
    !defined(CONFIG_ENV_IS_NOWHERE)
# error Define one of CONFIG_ENV_IS_IN_{EEPROM|FLASH|DATAFLASH|ONENAND|\
SPI_FLASH|MG_DISK|NVRAM|MMC|NOWHERE}
#endif

#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

/************************************************************************
************************************************************************/

/*
 * Table with supported baudrates (defined in config_xyz.h)
 */
static const unsigned long baudrate_table[] = CONFIG_SYS_BAUDRATE_TABLE;
#define	N_BAUDRATES (sizeof(baudrate_table) / sizeof(baudrate_table[0]))

/*
 * This variable is incremented on each do_setenv (), so it can
 * be used via get_env_id() as an indication, if the environment
 * has changed or not. So it is possible to reread an environment
 * variable only if the environment was changed ... done so for
 * example in NetInitLoop()
 */
static int env_id = 1;

int get_env_id (void)
{
	return env_id;
}
/************************************************************************
 * Command interface: print one or all environment variables
 */

/*
 * state 0: finish printing this string and return (matched!)
 * state 1: no matching to be done; print everything
 * state 2: continue searching for matched name
 */
static int printenv(char *name, int state)
{
	int i, j;
	char c, buf[17];

	i = 0;
	buf[16] = '\0';

	while (state && env_get_char(i) != '\0') {
		if (state == 2 && envmatch((uchar *)name, i) >= 0)
			state = 0;

		j = 0;
		do {
			buf[j++] = c = env_get_char(i++);
			if (j == sizeof(buf) - 1) {
				if (state <= 1)
					puts(buf);
				j = 0;
			}
		} while (c != '\0');

		if (state <= 1) {
			if (j)
				puts(buf);
			putc('\n');
		}

		if (ctrlc())
			return -1;
	}

	if (state == 0)
		i = 0;
	return i;
}

int do_printenv (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	int rcode = 0;

	if (argc == 1) {
		/* print all env vars */
		rcode = printenv(NULL, 1);
		if (rcode < 0)
			return 1;
		printf("\nEnvironment size: %d/%ld bytes\n",
			rcode, (ulong)ENV_SIZE);
		return 0;
	}

	/* print selected env vars */
	for (i = 1; i < argc; ++i) {
		char *name = argv[i];
		if (printenv(name, 2)) {
			printf("## Error: \"%s\" not defined\n", name);
			++rcode;
		}
	}

	return rcode;
}

/************************************************************************
 * Set a new environment variable,
 * or replace or delete an existing one.
 *
 * This function will ONLY work with a in-RAM copy of the environment
 */

int _do_setenv (int flag, int argc, char * const argv[])
{
	int   i, len, oldval;
	int   console = -1;
	uchar *env, *nxt = NULL;
	char *name;
	bd_t *bd = gd->bd;

	uchar *env_data = env_get_addr(0);

	if (!env_data)	/* need copy in RAM */
		return 1;

	name = argv[1];

	if (strchr(name, '=')) {
		printf ("## Error: illegal character '=' in variable name \"%s\"\n", name);
		return 1;
	}

	env_id++;
	/*
	 * search if variable with this name already exists
	 */
	oldval = -1;
	for (env=env_data; *env; env=nxt+1) {
		for (nxt=env; *nxt; ++nxt)
			;
		if ((oldval = envmatch((uchar *)name, env-env_data)) >= 0)
			break;
	}

	/* Check for console redirection */
	if (strcmp(name,"stdin") == 0) {
		console = stdin;
	} else if (strcmp(name,"stdout") == 0) {
		console = stdout;
	} else if (strcmp(name,"stderr") == 0) {
		console = stderr;
	}

	if (console != -1) {
		if (argc < 3) {		/* Cannot delete it! */
			printf("Can't delete \"%s\"\n", name);
			return 1;
		}

#ifdef CONFIG_CONSOLE_MUX
		i = iomux_doenv(console, argv[2]);
		if (i)
			return i;
#else
		/* Try assigning specified device */
		if (console_assign (console, argv[2]) < 0)
			return 1;

#ifdef CONFIG_SERIAL_MULTI
		if (serial_assign (argv[2]) < 0)
			return 1;
#endif
#endif /* CONFIG_CONSOLE_MUX */
	}

	/*
	 * Delete any existing definition
	 */
	if (oldval >= 0) {
#ifndef CONFIG_ENV_OVERWRITE

		/*
		 * Ethernet Address and serial# can be set only once,
		 * ver is readonly.
		 */
		if (
#ifdef CONFIG_HAS_UID
		/* Allow serial# forced overwrite with 0xdeaf4add flag */
		    ((strcmp (name, "serial#") == 0) && (flag != 0xdeaf4add)) ||
#else
		    (strcmp (name, "serial#") == 0) ||
#endif
		    ((strcmp (name, "ethaddr") == 0)
#if defined(CONFIG_OVERWRITE_ETHADDR_ONCE) && defined(CONFIG_ETHADDR)
		     && (strcmp ((char *)env_get_addr(oldval),MK_STR(CONFIG_ETHADDR)) != 0)
#endif	/* CONFIG_OVERWRITE_ETHADDR_ONCE && CONFIG_ETHADDR */
		    ) ) {
			printf ("Can't overwrite \"%s\"\n", name);
			return 1;
		}
#endif

		/*
		 * Switch to new baudrate if new baudrate is supported
		 */
		if (strcmp(argv[1],"baudrate") == 0) {
			int baudrate = simple_strtoul(argv[2], NULL, 10);
			int i;
			for (i=0; i<N_BAUDRATES; ++i) {
				if (baudrate == baudrate_table[i])
					break;
			}
			if (i == N_BAUDRATES) {
				printf ("## Baudrate %d bps not supported\n",
					baudrate);
				return 1;
			}
			printf ("## Switch baudrate to %d bps and press ENTER ...\n",
				baudrate);
			udelay(50000);
			gd->baudrate = baudrate;
#if defined(CONFIG_PPC) || defined(CONFIG_MCF52x2)
			gd->bd->bi_baudrate = baudrate;
#endif

			serial_setbrg ();
			udelay(50000);
			for (;;) {
				if (getc() == '\r')
				      break;
			}
		}

		if (*++nxt == '\0') {
			if (env > env_data) {
				env--;
			} else {
				*env = '\0';
			}
		} else {
			for (;;) {
				*env = *nxt++;
				if ((*env == '\0') && (*nxt == '\0'))
					break;
				++env;
			}
		}
		*++env = '\0';
	}

	/* Delete only ? */
	if ((argc < 3) || argv[2] == NULL) {
		env_crc_update ();
		return 0;
	}

	/*
	 * Append new definition at the end
	 */
	for (env=env_data; *env || *(env+1); ++env)
		;
	if (env > env_data)
		++env;
	/*
	 * Overflow when:
	 * "name" + "=" + "val" +"\0\0"  > ENV_SIZE - (env-env_data)
	 */
	len = strlen(name) + 2;
	/* add '=' for first arg, ' ' for all others */
	for (i=2; i<argc; ++i) {
		len += strlen(argv[i]) + 1;
	}
	if (len > (&env_data[ENV_SIZE]-env)) {
		printf ("## Error: environment overflow, \"%s\" deleted\n", name);
		return 1;
	}
	while ((*env = *name++) != '\0')
		env++;
	for (i=2; i<argc; ++i) {
		char *val = argv[i];

		*env = (i==2) ? '=' : ' ';
		while ((*++env = *val++) != '\0')
			;
	}

	/* end is marked with double '\0' */
	*++env = '\0';

	/* Update CRC */
	env_crc_update ();

	/*
	 * Some variables should be updated when the corresponding
	 * entry in the enviornment is changed
	 */

	if (strcmp(argv[1],"ethaddr") == 0)
		return 0;

	if (strcmp(argv[1],"ipaddr") == 0) {
		char *s = argv[2];	/* always use only one arg */
		char *e;
		unsigned long addr;
		bd->bi_ip_addr = 0;
		for (addr=0, i=0; i<4; ++i) {
			ulong val = s ? simple_strtoul(s, &e, 10) : 0;
			addr <<= 8;
			addr  |= (val & 0xFF);
			if (s) s = (*e) ? e+1 : e;
		}
		bd->bi_ip_addr = htonl(addr);
		return 0;
	}
	if (strcmp(argv[1],"loadaddr") == 0) {
		load_addr = simple_strtoul(argv[2], NULL, 16);
		return 0;
	}
#if defined(CONFIG_CMD_NET)
	if (strcmp(argv[1],"bootfile") == 0) {
		copy_filename (BootFile, argv[2], sizeof(BootFile));
		return 0;
	}
#endif
	return 0;
}

int setenv (char *varname, char *varvalue)
{
	char * const argv[4] = { "setenv", varname, varvalue, NULL };
	if ((varvalue == NULL) || (varvalue[0] == '\0'))
		return _do_setenv (0, 2, argv);
	else
		return _do_setenv (0, 3, argv);
}

#ifdef CONFIG_HAS_UID
void forceenv (char *varname, char *varvalue)
{
	char * const argv[4] = { "forceenv", varname, varvalue, NULL };
	_do_setenv (0xdeaf4add, 3, argv);
}
#endif

int do_setenv (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return cmd_usage(cmdtp);

	return _do_setenv (flag, argc, argv);
}

/************************************************************************
 * Prompt for environment variable
 */

#if defined(CONFIG_CMD_ASKENV)
int do_askenv ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	extern char console_buffer[CONFIG_SYS_CBSIZE];
	char message[CONFIG_SYS_CBSIZE];
	int size = CONFIG_SYS_CBSIZE - 1;
	int len;
	char *local_args[4];

	local_args[0] = argv[0];
	local_args[1] = argv[1];
	local_args[2] = NULL;
	local_args[3] = NULL;

	if (argc < 2)
		return cmd_usage(cmdtp);

	/* Check the syntax */
	switch (argc) {
	case 1:
		return cmd_usage(cmdtp);

	case 2:		/* askenv envname */
		sprintf (message, "Please enter '%s':", argv[1]);
		break;

	case 3:		/* askenv envname size */
		sprintf (message, "Please enter '%s':", argv[1]);
		size = simple_strtoul (argv[2], NULL, 10);
		break;

	default:	/* askenv envname message1 ... messagen size */
	    {
		int i;
		int pos = 0;

		for (i = 2; i < argc - 1; i++) {
			if (pos) {
				message[pos++] = ' ';
			}
			strcpy (message+pos, argv[i]);
			pos += strlen(argv[i]);
		}
		message[pos] = '\0';
		size = simple_strtoul (argv[argc - 1], NULL, 10);
	    }
		break;
	}

	if (size >= CONFIG_SYS_CBSIZE)
		size = CONFIG_SYS_CBSIZE - 1;

	if (size <= 0)
		return 1;

	/* prompt for input */
	len = readline (message);

	if (size < len)
		console_buffer[size] = '\0';

	len = 2;
	if (console_buffer[0] != '\0') {
		local_args[2] = console_buffer;
		len = 3;
	}

	/* Continue calling setenv code */
	return _do_setenv (flag, len, local_args);
}
#endif

/************************************************************************
 * Interactively edit an environment variable
 */
#if defined(CONFIG_CMD_EDITENV)
int do_editenv(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char buffer[CONFIG_SYS_CBSIZE];
	char *init_val;
	int len;

	if (argc < 2)
		return cmd_usage(cmdtp);

	/* Set read buffer to initial value or empty sting */
	init_val = getenv(argv[1]);
	if (init_val)
		len = sprintf(buffer, "%s", init_val);
	else
		buffer[0] = '\0';

	readline_into_buffer("edit: ", buffer);

	return setenv(argv[1], buffer);
}
#endif /* CONFIG_CMD_EDITENV */

/************************************************************************
 * Look up variable from environment,
 * return address of storage for that variable,
 * or NULL if not found
 */

char *getenv (char *name)
{
	int i, nxt;

	WATCHDOG_RESET();

	for (i=0; env_get_char(i) != '\0'; i=nxt+1) {
		int val;

		for (nxt=i; env_get_char(nxt) != '\0'; ++nxt) {
			if (nxt >= CONFIG_ENV_SIZE) {
				return (NULL);
			}
		}
		if ((val=envmatch((uchar *)name, i)) < 0)
			continue;
		return ((char *)env_get_addr(val));
	}

	return (NULL);
}

int getenv_f(char *name, char *buf, unsigned len)
{
	int i, nxt;

	for (i=0; env_get_char(i) != '\0'; i=nxt+1) {
		int val, n;

		for (nxt=i; env_get_char(nxt) != '\0'; ++nxt) {
			if (nxt >= CONFIG_ENV_SIZE) {
				return (-1);
			}
		}
		if ((val=envmatch((uchar *)name, i)) < 0)
			continue;

		/* found; copy out */
		for (n=0; n<len; ++n, ++buf) {
			if ((*buf = env_get_char(val++)) == '\0')
				return n;
		}

		if (n)
			*--buf = '\0';

		printf("env_buf too small [%d]\n", len);

		return n;
	}
	return (-1);
}

#if defined(CONFIG_CMD_SAVEENV) && !defined(CONFIG_ENV_IS_NOWHERE)

int do_saveenv (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	extern char * env_name_spec;

	printf ("Saving Environment to %s...\n", env_name_spec);

	return (saveenv() ? 1 : 0);
}

U_BOOT_CMD(
	saveenv, 1, 0,	do_saveenv,
	"save environment variables to persistent storage",
	""
);

#endif


/************************************************************************
 * Match a name / name=value pair
 *
 * s1 is either a simple 'name', or a 'name=value' pair.
 * i2 is the environment index for a 'name2=value2' pair.
 * If the names match, return the index for the value2, else NULL.
 */

int envmatch (uchar *s1, int i2)
{

	while (*s1 == env_get_char(i2++))
		if (*s1++ == '=')
			return(i2);
	if (*s1 == '\0' && env_get_char(i2-1) == '=')
		return(i2);
	return(-1);
}


/**************************************************/

#if defined(CONFIG_CMD_EDITENV)
U_BOOT_CMD(
	editenv, 2, 0,	do_editenv,
	"edit environment variable",
	"name\n"
	"    - edit environment variable 'name'"
);
#endif

U_BOOT_CMD(
	printenv, CONFIG_SYS_MAXARGS, 1,	do_printenv,
	"print environment variables",
	"\n    - print values of all environment variables\n"
	"printenv name ...\n"
	"    - print value of environment variable 'name'"
);

U_BOOT_CMD(
	setenv, CONFIG_SYS_MAXARGS, 0,	do_setenv,
	"set environment variables",
	"name value ...\n"
	"    - set environment variable 'name' to 'value ...'\n"
	"setenv name\n"
	"    - delete environment variable 'name'"
);

#if defined(CONFIG_CMD_ASKENV)

U_BOOT_CMD(
	askenv,	CONFIG_SYS_MAXARGS,	1,	do_askenv,
	"get environment variables from stdin",
	"name [message] [size]\n"
	"    - get environment variable 'name' from stdin (max 'size' chars)\n"
	"askenv name\n"
	"    - get environment variable 'name' from stdin\n"
	"askenv name size\n"
	"    - get environment variable 'name' from stdin (max 'size' chars)\n"
	"askenv name [message] size\n"
	"    - display 'message' string and get environment variable 'name'"
	"from stdin (max 'size' chars)"
);
#endif

#if defined(CONFIG_CMD_RUN)
int do_run (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
U_BOOT_CMD(
	run,	CONFIG_SYS_MAXARGS,	1,	do_run,
	"run commands in an environment variable",
	"var [...]\n"
	"    - run the commands in the environment variable(s) 'var'"
);
#endif
