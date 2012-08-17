/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <search.h>
#include <errno.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

/************************************************************************
 * Default settings to be used when no valid environment is found
 */
#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

const uchar default_environment[] = {
#ifdef	CONFIG_BOOTARGS
	"bootargs="	CONFIG_BOOTARGS			"\0"
#endif
#ifdef	CONFIG_BOOTCOMMAND
	"bootcmd="	CONFIG_BOOTCOMMAND		"\0"
#endif
#ifdef	CONFIG_RAMBOOTCOMMAND
	"ramboot="	CONFIG_RAMBOOTCOMMAND		"\0"
#endif
#ifdef	CONFIG_NFSBOOTCOMMAND
	"nfsboot="	CONFIG_NFSBOOTCOMMAND		"\0"
#endif
#if defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
	"bootdelay="	MK_STR(CONFIG_BOOTDELAY)	"\0"
#endif
#if defined(CONFIG_BAUDRATE) && (CONFIG_BAUDRATE >= 0)
	"baudrate="	MK_STR(CONFIG_BAUDRATE)		"\0"
#endif
#ifdef	CONFIG_LOADS_ECHO
	"loads_echo="	MK_STR(CONFIG_LOADS_ECHO)	"\0"
#endif
#ifdef	CONFIG_ETHADDR
	"ethaddr="	MK_STR(CONFIG_ETHADDR)		"\0"
#endif
#ifdef	CONFIG_ETH1ADDR
	"eth1addr="	MK_STR(CONFIG_ETH1ADDR)		"\0"
#endif
#ifdef	CONFIG_ETH2ADDR
	"eth2addr="	MK_STR(CONFIG_ETH2ADDR)		"\0"
#endif
#ifdef	CONFIG_ETH3ADDR
	"eth3addr="	MK_STR(CONFIG_ETH3ADDR)		"\0"
#endif
#ifdef	CONFIG_ETH4ADDR
	"eth4addr="	MK_STR(CONFIG_ETH4ADDR)		"\0"
#endif
#ifdef	CONFIG_ETH5ADDR
	"eth5addr="	MK_STR(CONFIG_ETH5ADDR)		"\0"
#endif
#ifdef	CONFIG_IPADDR
	"ipaddr="	MK_STR(CONFIG_IPADDR)		"\0"
#endif
#ifdef	CONFIG_SERVERIP
	"serverip="	MK_STR(CONFIG_SERVERIP)		"\0"
#endif
#ifdef	CONFIG_SYS_AUTOLOAD
	"autoload="	CONFIG_SYS_AUTOLOAD		"\0"
#endif
#ifdef	CONFIG_PREBOOT
	"preboot="	CONFIG_PREBOOT			"\0"
#endif
#ifdef	CONFIG_ROOTPATH
	"rootpath="	CONFIG_ROOTPATH			"\0"
#endif
#ifdef	CONFIG_GATEWAYIP
	"gatewayip="	MK_STR(CONFIG_GATEWAYIP)	"\0"
#endif
#ifdef	CONFIG_NETMASK
	"netmask="	MK_STR(CONFIG_NETMASK)		"\0"
#endif
#ifdef	CONFIG_HOSTNAME
	"hostname="	MK_STR(CONFIG_HOSTNAME)		"\0"
#endif
#ifdef	CONFIG_BOOTFILE
	"bootfile="	CONFIG_BOOTFILE			"\0"
#endif
#ifdef	CONFIG_LOADADDR
	"loadaddr="	MK_STR(CONFIG_LOADADDR)		"\0"
#endif
#ifdef	CONFIG_CLOCKS_IN_MHZ
	"clocks_in_mhz=1\0"
#endif
#if defined(CONFIG_PCI_BOOTDELAY) && (CONFIG_PCI_BOOTDELAY > 0)
	"pcidelay="	MK_STR(CONFIG_PCI_BOOTDELAY)	"\0"
#endif
#ifdef	CONFIG_EXTRA_ENV_SETTINGS
	CONFIG_EXTRA_ENV_SETTINGS
#endif
	"\0"
};

struct hsearch_data env_htab;

static uchar __env_get_char_spec(int index)
{
	return *((uchar *)(gd->env_addr + index));
}
uchar env_get_char_spec(int)
	__attribute__((weak, alias("__env_get_char_spec")));

static uchar env_get_char_init(int index)
{
	/* if crc was bad, use the default environment */
	if (gd->env_valid)
		return env_get_char_spec(index);
	else
		return default_environment[index];
}

uchar env_get_char_memory(int index)
{
	return *env_get_addr(index);
}

uchar env_get_char(int index)
{
	/* if relocated to RAM */
	if (gd->flags & GD_FLG_RELOC)
		return env_get_char_memory(index);
	else
		return env_get_char_init(index);
}

const uchar *env_get_addr(int index)
{
	if (gd->env_valid)
		return (uchar *)(gd->env_addr + index);
	else
		return &default_environment[index];
}

void set_default_env(const char *s)
{
	if (sizeof(default_environment) > ENV_SIZE) {
		puts("*** Error - default environment is too large\n\n");
		return;
	}

	if (s) {
		if (*s == '!') {
			printf("*** Warning - %s, "
				"using default environment\n\n",
				s + 1);
		} else {
			puts(s);
		}
	} else {
		puts("Using default environment\n\n");
	}

	if (himport_r(&env_htab, (char *)default_environment,
			sizeof(default_environment), '\0', 0) == 0)
		error("Environment import failed: errno = %d\n", errno);

	gd->flags |= GD_FLG_ENV_READY;
}

/*
 * Check if CRC is valid and (if yes) import the environment.
 * Note that "buf" may or may not be aligned.
 */
int env_import(const char *buf, int check)
{
	env_t *ep = (env_t *)buf;

	if (check) {
		uint32_t crc;

		memcpy(&crc, &ep->crc, sizeof(crc));

		if (crc32(0, ep->data, ENV_SIZE) != crc) {
			set_default_env("!bad CRC");
			return 0;
		}
	}

	if (himport_r(&env_htab, (char *)ep->data, ENV_SIZE, '\0', 0)) {
		gd->flags |= GD_FLG_ENV_READY;
		return 1;
	}

	error("Cannot import environment: errno = %d\n", errno);

	set_default_env("!import failed");

	return 0;
}

void env_relocate(void)
{
#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	env_reloc();
#endif
	if (gd->env_valid == 0) {
#if defined(CONFIG_ENV_IS_NOWHERE)	/* Environment not changable */
		set_default_env(NULL);
#else
		bootstage_error(BOOTSTAGE_ID_NET_CHECKSUM);
		set_default_env("!bad CRC");
#endif
	} else {
		env_relocate_spec();
	}
}

#ifdef CONFIG_AUTO_COMPLETE
int env_complete(char *var, int maxv, char *cmdv[], int bufsz, char *buf)
{
	ENTRY *match;
	int found, idx;

	idx = 0;
	found = 0;
	cmdv[0] = NULL;

	while ((idx = hmatch_r(var, idx, &match, &env_htab))) {
		int vallen = strlen(match->key) + 1;

		if (found >= maxv - 2 || bufsz < vallen)
			break;

		cmdv[found++] = buf;
		memcpy(buf, match->key, vallen);
		buf += vallen;
		bufsz -= vallen;
	}

	qsort(cmdv, found, sizeof(cmdv[0]), strcmp_compar);

	if (idx)
		cmdv[found++] = "...";

	cmdv[found] = NULL;
	return found;
}
#endif
