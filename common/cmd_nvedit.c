/*
 * (C) Copyright 2000-2010
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

/*
 * Support for persistent environment data
 *
 * The "environment" is stored on external storage as a list of '\0'
 * terminated "name=value" strings. The end of the list is marked by
 * a double '\0'. The environment is preceeded by a 32 bit CRC over
 * the data part and, in case of redundant environment, a byte of
 * flags.
 *
 * This linearized representation will also be used before
 * relocation, i. e. as long as we don't have a full C runtime
 * environment. After that, we use a hash table.
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <search.h>
#include <errno.h>
#include <malloc.h>
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

/*
 * Maximum expected input data size for import command
 */
#define	MAX_ENV_SIZE	(1 << 20)	/* 1 MiB */

ulong load_addr = CONFIG_SYS_LOAD_ADDR;	/* Default Load Address */

/*
 * Table with supported baudrates (defined in config_xyz.h)
 */
static const unsigned long baudrate_table[] = CONFIG_SYS_BAUDRATE_TABLE;
#define	N_BAUDRATES (sizeof(baudrate_table) / sizeof(baudrate_table[0]))

/*
 * This variable is incremented on each do_env_set(), so it can
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

/*
 * Command interface: print one or all environment variables
 *
 * Returns 0 in case of error, or length of printed string
 */
static int env_print(char *name)
{
	char *res = NULL;
	size_t len;

	if (name) {		/* print a single name */
		ENTRY e, *ep;

		e.key = name;
		e.data = NULL;
		hsearch_r(e, FIND, &ep, &env_htab);
		if (ep == NULL)
			return 0;
		len = printf ("%s=%s\n", ep->key, ep->data);
		return len;
	}

	/* print whole list */
	len = hexport_r(&env_htab, '\n', &res, 0);

	if (len > 0) {
		puts(res);
		free(res);
		return len;
	}

	/* should never happen */
	return 0;
}

int do_env_print (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	int rcode = 0;

	if (argc == 1) {
		/* print all env vars */
		rcode = env_print(NULL);
		if (!rcode)
			return 1;
		printf("\nEnvironment size: %d/%ld bytes\n",
			rcode, (ulong)ENV_SIZE);
		return 0;
	}

	/* print selected env vars */
	for (i = 1; i < argc; ++i) {
		int rc = env_print(argv[i]);
		if (!rc) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			++rcode;
		}
	}

	return rcode;
}

/*
 * Set a new environment variable,
 * or replace or delete an existing one.
 */

int _do_env_set (int flag, int argc, char * const argv[])
{
	bd_t  *bd = gd->bd;
	int   i, len;
	int   console = -1;
	char  *name, *value, *s;
	ENTRY e, *ep;

	name = argv[1];

	if (strchr(name, '=')) {
		printf ("## Error: illegal character '=' in variable name \"%s\"\n", name);
		return 1;
	}

	env_id++;
	/*
	 * search if variable with this name already exists
	 */
	e.key = name;
	e.data = NULL;
	hsearch_r(e, FIND, &ep, &env_htab);

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
	 * Some variables like "ethaddr" and "serial#" can be set only
	 * once and cannot be deleted; also, "ver" is readonly.
	 */
	if (ep) {		/* variable exists */
#ifndef CONFIG_ENV_OVERWRITE
		if ((strcmp (name, "serial#") == 0) ||
		    ((strcmp (name, "ethaddr") == 0)
#if defined(CONFIG_OVERWRITE_ETHADDR_ONCE) && defined(CONFIG_ETHADDR)
		     && (strcmp (ep->data,MK_STR(CONFIG_ETHADDR)) != 0)
#endif	/* CONFIG_OVERWRITE_ETHADDR_ONCE && CONFIG_ETHADDR */
		    ) ) {
			printf ("Can't overwrite \"%s\"\n", name);
			return 1;
		}
#endif
		/*
		 * Switch to new baudrate if new baudrate is supported
		 */
		if (strcmp(name,"baudrate") == 0) {
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
	}

	/* Delete only ? */
	if ((argc < 3) || argv[2] == NULL) {
		int rc = hdelete_r(name, &env_htab);
		return !rc;
	}

	/*
	 * Insert / replace new value
	 */
	for (i=2,len=0; i<argc; ++i) {
		len += strlen(argv[i]) + 1;
	}
	if ((value = malloc(len)) == NULL) {
		printf("## Can't malloc %d bytes\n", len);
		return 1;
	}
	for (i=2,s=value; i<argc; ++i) {
		char *v = argv[i];

		while ((*s++ = *v++) != '\0')
			;
		*(s-1) = ' ';
	}
	if (s != value)
		*--s = '\0';

	e.key  = name;
	e.data = value;
	hsearch_r(e, ENTER, &ep, &env_htab);
	free(value);
	if (!ep) {
		printf("## Error inserting \"%s\" variable, errno=%d\n",
			name, errno);
		return 1;
	}

	/*
	 * Some variables should be updated when the corresponding
	 * entry in the environment is changed
	 */

	if (strcmp(name,"ipaddr") == 0) {
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
	} else if (strcmp(argv[1],"loadaddr") == 0) {
		load_addr = simple_strtoul(argv[2], NULL, 16);
		return 0;
	}
#if defined(CONFIG_CMD_NET)
	else if (strcmp(argv[1],"bootfile") == 0) {
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
		return _do_env_set(0, 2, argv);
	else
		return _do_env_set(0, 3, argv);
}

int do_env_set (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return cmd_usage(cmdtp);

	return _do_env_set(flag, argc, argv);
}

/*
 * Prompt for environment variable
 */
#if defined(CONFIG_CMD_ASKENV)
int do_env_ask ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	extern char console_buffer[CONFIG_SYS_CBSIZE];
	char message[CONFIG_SYS_CBSIZE];
	int size = CONFIG_SYS_CBSIZE - 1;
	int i, len, pos;
	char *local_args[4];

	local_args[0] = argv[0];
	local_args[1] = argv[1];
	local_args[2] = NULL;
	local_args[3] = NULL;

	/* Check the syntax */
	switch (argc) {
	case 1:
		return cmd_usage(cmdtp);

	case 2:		/* env_ask envname */
		sprintf(message, "Please enter '%s':", argv[1]);
		break;

	case 3:		/* env_ask envname size */
		sprintf(message, "Please enter '%s':", argv[1]);
		size = simple_strtoul(argv[2], NULL, 10);
		break;

	default:	/* env_ask envname message1 ... messagen size */
		for (i=2,pos=0; i < argc - 1; i++) {
			if (pos) {
				message[pos++] = ' ';
			}
			strcpy(message+pos, argv[i]);
			pos += strlen(argv[i]);
		}
		message[pos] = '\0';
		size = simple_strtoul(argv[argc - 1], NULL, 10);
		break;
	}

	if (size >= CONFIG_SYS_CBSIZE)
		size = CONFIG_SYS_CBSIZE - 1;

	if (size <= 0)
		return 1;

	/* prompt for input */
	len = readline(message);

	if (size < len)
		console_buffer[size] = '\0';

	len = 2;
	if (console_buffer[0] != '\0') {
		local_args[2] = console_buffer;
		len = 3;
	}

	/* Continue calling setenv code */
	return _do_env_set(flag, len, local_args);
}
#endif

/*
 * Interactively edit an environment variable
 */
#if defined(CONFIG_CMD_EDITENV)
int do_env_edit(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
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

/*
 * Look up variable from environment,
 * return address of storage for that variable,
 * or NULL if not found
 */
char *getenv (char *name)
{
	if (gd->flags & GD_FLG_ENV_READY) {	/* after import into hashtable */
		ENTRY e, *ep;

		WATCHDOG_RESET();

		e.key  = name;
		e.data = NULL;
		hsearch_r(e, FIND, &ep, &env_htab);

		return (ep ? ep->data : NULL);
	}

	/* restricted capabilities before import */

	if (getenv_f(name, (char *)(gd->env_buf), sizeof(gd->env_buf)) > 0)
		return (char *)(gd->env_buf);

	return NULL;
}

/*
 * Look up variable from environment for restricted C runtime env.
 */
int getenv_f (char *name, char *buf, unsigned len)
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

int do_env_save (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	extern char * env_name_spec;

	printf ("Saving Environment to %s...\n", env_name_spec);

	return (saveenv() ? 1 : 0);
}

U_BOOT_CMD(
	saveenv, 1, 0,	do_env_save,
	"save environment variables to persistent storage",
	""
);

#endif


/*
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

static int do_env_default(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	if ((argc != 2) || (strcmp(argv[1], "-f") != 0)) {
		return cmd_usage(cmdtp);
	}
	set_default_env("## Resetting to default environment\n");
	return 0;
}

static int do_env_delete(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	printf("Not implemented yet\n");
	return 0;
}

/*
 * env export [-t | -b | -c] addr [size]
 *	-t:	export as text format; if size is given, data will be
 *		padded with '\0' bytes; if not, one terminating '\0'
 *		will be added (which is included in the "filesize"
 *		setting so you can for exmple copy this to flash and
 *		keep the termination).
 *	-b:	export as binary format (name=value pairs separated by
 *		'\0', list end marked by double "\0\0")
 *	-c:	export as checksum protected environment format as
 *		used for example by "saveenv" command
 *	addr:	memory address where environment gets stored
 *	size:	size of output buffer
 *
 * With "-c" and size is NOT given, then the export command will
 * format the data as currently used for the persistent storage,
 * i. e. it will use CONFIG_ENV_SECT_SIZE as output block size and
 * prepend a valid CRC32 checksum and, in case of resundant
 * environment, a "current" redundancy flag. If size is given, this
 * value will be used instead of CONFIG_ENV_SECT_SIZE; again, CRC32
 * checksum and redundancy flag will be inserted.
 *
 * With "-b" and "-t", always only the real data (including a
 * terminating '\0' byte) will be written; here the optional size
 * argument will be used to make sure not to overflow the user
 * provided buffer; the command will abort if the size is not
 * sufficient. Any remainign space will be '\0' padded.
 *
 * On successful return, the variable "filesize" will be set.
 * Note that filesize includes the trailing/terminating '\0' byte(s).
 *
 * Usage szenario:  create a text snapshot/backup of the current settings:
 *
 *	=> env export -t 100000
 *	=> era ${backup_addr} +${filesize}
 *	=> cp.b 100000 ${backup_addr} ${filesize}
 *
 * Re-import this snapshot, deleting all other settings:
 *
 *	=> env import -d -t ${backup_addr}
 */
static int do_env_export(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char	buf[32];
	char	*addr, *cmd, *res;
	size_t	size;
	ssize_t	len;
	env_t	*envp;
	char	sep = '\n';
	int	chk = 0;
	int	fmt = 0;

	cmd = *argv;

	while (--argc > 0 && **++argv == '-') {
		char *arg = *argv;
		while (*++arg) {
			switch (*arg) {
			case 'b':		/* raw binary format */
				if (fmt++)
					goto sep_err;
				sep = '\0';
				break;
			case 'c':		/* external checksum format */
				if (fmt++)
					goto sep_err;
				sep = '\0';
				chk = 1;
				break;
			case 't':		/* text format */
				if (fmt++)
					goto sep_err;
				sep = '\n';
				break;
			default:
				return cmd_usage(cmdtp);
			}
		}
	}

	if (argc < 1) {
		return cmd_usage(cmdtp);
	}

	addr = (char *)simple_strtoul(argv[0], NULL, 16);

	if (argc == 2) {
		size = simple_strtoul(argv[1], NULL, 16);
		memset(addr, '\0', size);
	} else {
		size = 0;
	}

	if (sep) {		/* export as text file */
		len = hexport_r(&env_htab, sep, &addr, size);
		if (len < 0) {
			error("Cannot export environment: errno = %d\n",
				errno);
			return 1;
		}
		sprintf(buf, "%zX", len);
		setenv("filesize", buf);

		return 0;
	}

	envp = (env_t *)addr;

	if (chk)		/* export as checksum protected block */
		res = (char *)envp->data;
	else			/* export as raw binary data */
		res = addr;

	len = hexport_r(&env_htab, '\0', &res, ENV_SIZE);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n",
			errno);
		return 1;
	}

	if (chk) {
		envp->crc   = crc32(0, envp->data, ENV_SIZE);
#ifdef CONFIG_ENV_ADDR_REDUND
		envp->flags = ACTIVE_FLAG;
#endif
	}
	sprintf(buf, "%zX", len + offsetof(env_t,data));
	setenv("filesize", buf);

	return 0;

sep_err:
	printf("## %s: only one of \"-b\", \"-c\" or \"-t\" allowed\n",
		cmd);
	return 1;
}

/*
 * env import [-d] [-t | -b | -c] addr [size]
 *	-d:	delete existing environment before importing;
 *		otherwise overwrite / append to existion definitions
 *	-t:	assume text format; either "size" must be given or the
 *		text data must be '\0' terminated
 *	-b:	assume binary format ('\0' separated, "\0\0" terminated)
 *	-c:	assume checksum protected environment format
 *	addr:	memory address to read from
 *	size:	length of input data; if missing, proper '\0'
 *		termination is mandatory
 */
static int do_env_import(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	char	*cmd, *addr;
	char	sep = '\n';
	int	chk = 0;
	int	fmt = 0;
	int	del = 0;
	size_t	size;

	cmd = *argv;

	while (--argc > 0 && **++argv == '-') {
		char *arg = *argv;
		while (*++arg) {
			switch (*arg) {
			case 'b':		/* raw binary format */
				if (fmt++)
					goto sep_err;
				sep = '\0';
				break;
			case 'c':		/* external checksum format */
				if (fmt++)
					goto sep_err;
				sep = '\0';
				chk = 1;
				break;
			case 't':		/* text format */
				if (fmt++)
					goto sep_err;
				sep = '\n';
				break;
			case 'd':
				del = 1;
				break;
			default:
				return cmd_usage(cmdtp);
			}
		}
	}

	if (argc < 1) {
		return cmd_usage(cmdtp);
	}

	if (!fmt)
		printf("## Warning: defaulting to text format\n");

	addr = (char *)simple_strtoul(argv[0], NULL, 16);

	if (argc == 2) {
		size = simple_strtoul(argv[1], NULL, 16);
	} else {
		char *s = addr;

		size = 0;

		while (size < MAX_ENV_SIZE) {
			if ((*s == sep) && (*(s+1) == '\0'))
				break;
			++s;
			++size;
		}
		if (size == MAX_ENV_SIZE) {
			printf("## Warning: Input data exceeds %d bytes"
				" - truncated\n", MAX_ENV_SIZE);
		}
		++size;
		printf("## Info: input data size = %zd = 0x%zX\n", size, size);
	}

	if (chk) {
		uint32_t crc;
		env_t *ep = (env_t *)addr;

		size -= offsetof(env_t, data);
		memcpy(&crc, &ep->crc, sizeof(crc));

		if (crc32(0, ep->data, size) != crc) {
			puts("## Error: bad CRC, import failed\n");
			return 1;
		}
		addr = (char *)ep->data;
	}

	if (himport_r(&env_htab, addr, size, sep, del ? 0 : H_NOCLEAR) == 0) {
		error("Environment import failed: errno = %d\n", errno);
		return 1;
	}
	gd->flags |= GD_FLG_ENV_READY;

	return 0;

sep_err:
	printf("## %s: only one of \"-b\", \"-c\" or \"-t\" allowed\n",
		cmd);
	return 1;
}

#if defined(CONFIG_CMD_RUN)
extern int do_run (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif

/*
 * New command line interface: "env" command with subcommands
 */
static cmd_tbl_t cmd_env_sub[] = {
#if defined(CONFIG_CMD_ASKENV)
	U_BOOT_CMD_MKENT(ask, CONFIG_SYS_MAXARGS, 1, do_env_ask, "", ""),
#endif
	U_BOOT_CMD_MKENT(default, 1, 0, do_env_default, "", ""),
	U_BOOT_CMD_MKENT(delete, 2, 0, do_env_delete, "", ""),
#if defined(CONFIG_CMD_EDITENV)
	U_BOOT_CMD_MKENT(edit, 2, 0, do_env_edit, "", ""),
#endif
	U_BOOT_CMD_MKENT(export, 4, 0, do_env_export, "", ""),
	U_BOOT_CMD_MKENT(import, 5, 0, do_env_import, "", ""),
	U_BOOT_CMD_MKENT(print, CONFIG_SYS_MAXARGS, 1, do_env_print, "", ""),
#if defined(CONFIG_CMD_RUN)
	U_BOOT_CMD_MKENT(run, CONFIG_SYS_MAXARGS, 1, do_run, "", ""),
#endif
#if defined(CONFIG_CMD_SAVEENV) && !defined(CONFIG_ENV_IS_NOWHERE)
	U_BOOT_CMD_MKENT(save, 1, 0, do_env_save, "", ""),
#endif
	U_BOOT_CMD_MKENT(set, CONFIG_SYS_MAXARGS, 0, do_env_set, "", ""),
};

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
void env_reloc(void)
{
	fixup_cmdtable(cmd_env_sub, ARRAY_SIZE(cmd_env_sub));
}
#endif

static int do_env (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *cp;

	if (argc < 2)
		return cmd_usage(cmdtp);

	/* drop initial "env" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_env_sub, ARRAY_SIZE(cmd_env_sub));

	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	env, CONFIG_SYS_MAXARGS, 1, do_env,
	"environment handling commands",
#if defined(CONFIG_CMD_ASKENV)
	"ask name [message] [size] - ask for environment variable\nenv "
#endif
	"default -f - reset default environment\n"
#if defined(CONFIG_CMD_EDITENV)
	"env edit name - edit environment variable\n"
#endif
	"env export [-t | -b | -c] addr [size] - export environmnt\n"
	"env import [-d] [-t | -b | -c] addr [size] - import environmnt\n"
	"env print [name ...] - print environment\n"
#if defined(CONFIG_CMD_RUN)
	"env run var [...] - run commands in an environment variable\n"
#endif
	"env save - save environment\n"
	"env set [-f] name [arg ...]\n"
);

/*
 * Old command line interface, kept for compatibility
 */

#if defined(CONFIG_CMD_EDITENV)
U_BOOT_CMD_COMPLETE(
	editenv, 2, 0,	do_env_edit,
	"edit environment variable",
	"name\n"
	"    - edit environment variable 'name'",
	var_complete
);
#endif

U_BOOT_CMD_COMPLETE(
	printenv, CONFIG_SYS_MAXARGS, 1,	do_env_print,
	"print environment variables",
	"\n    - print values of all environment variables\n"
	"printenv name ...\n"
	"    - print value of environment variable 'name'",
	var_complete
);

U_BOOT_CMD_COMPLETE(
	setenv, CONFIG_SYS_MAXARGS, 0,	do_env_set,
	"set environment variables",
	"name value ...\n"
	"    - set environment variable 'name' to 'value ...'\n"
	"setenv name\n"
	"    - delete environment variable 'name'",
	var_complete
);

#if defined(CONFIG_CMD_ASKENV)

U_BOOT_CMD(
	askenv,	CONFIG_SYS_MAXARGS,	1,	do_env_ask,
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
U_BOOT_CMD_COMPLETE(
	run,	CONFIG_SYS_MAXARGS,	1,	do_run,
	"run commands in an environment variable",
	"var [...]\n"
	"    - run the commands in the environment variable(s) 'var'",
	var_complete
);
#endif
