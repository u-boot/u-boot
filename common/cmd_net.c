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
#include <common.h>
#include <command.h>
#include <cmd_net.h>
#include <net.h>

#if (CONFIG_COMMANDS & CFG_CMD_NET)

# if (CONFIG_COMMANDS & CFG_CMD_AUTOSCRIPT)
# include <cmd_autoscript.h>
# endif

extern int do_bootm (cmd_tbl_t *, int, int, char *[]);

static int netboot_common (int, cmd_tbl_t *, int , char *[]);

int do_bootp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common (BOOTP, cmdtp, argc, argv);
}

int do_tftpb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common (TFTP, cmdtp, argc, argv);
}

int do_rarpb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common (RARP, cmdtp, argc, argv);
}

#if (CONFIG_COMMANDS & CFG_CMD_DHCP)
int do_dhcp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common(DHCP, cmdtp, argc, argv);
}
#endif	/* CFG_CMD_DHCP */

static void netboot_update_env(void)
{
    char tmp[16] ;

    if (NetOurGatewayIP) {
	ip_to_string (NetOurGatewayIP, tmp);
	setenv("gatewayip", tmp);
    }

    if (NetOurSubnetMask) {
	ip_to_string (NetOurSubnetMask, tmp);
	setenv("netmask", tmp);
    }

    if (NetOurHostName[0])
	setenv("hostname", NetOurHostName);

    if (NetOurRootPath[0])
	setenv("rootpath", NetOurRootPath);

    if (NetOurIP) {
	ip_to_string (NetOurIP, tmp);
	setenv("ipaddr", tmp);
    }

    if (NetServerIP) {
	ip_to_string (NetServerIP, tmp);
	setenv("serverip", tmp);
    }

    if (NetOurDNSIP) {
	ip_to_string (NetOurDNSIP, tmp);
	setenv("dnsip", tmp);
    }
}
static int
netboot_common (int proto, cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	char *s;
	int   rcode = 0;
	int   size;

	/* pre-set load_addr */
	if ((s = getenv("loadaddr")) != NULL) {
		load_addr = simple_strtoul(s, NULL, 16);
	}

	switch (argc) {
	case 1:
		break;

	case 2:	/* only one arg - accept two forms:
		 * just load address, or just boot file name.
		 * The latter form must be written "filename" here.
		 */
		if (argv[1][0] == '"') {	/* just boot filename */
			copy_filename (BootFile, argv[1], sizeof(BootFile));
		} else {			/* load address	*/
			load_addr = simple_strtoul(argv[1], NULL, 16);
		}
		break;

	case 3:	load_addr = simple_strtoul(argv[1], NULL, 16);
		copy_filename (BootFile, argv[2], sizeof(BootFile));

		break;

	default: printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if ((size = NetLoop(proto)) < 0)
		return 1;

	/* NetLoop ok, update environment */
	netboot_update_env();

	/* done if no file was loaded (no errors though) */
	if (size == 0)
		return 0;

	/* flush cache */
	flush_cache(load_addr, size);

	/* Loading ok, check if we should attempt an auto-start */
	if (((s = getenv("autostart")) != NULL) && (strcmp(s,"yes") == 0)) {
		char *local_args[2];
		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf ("Automatic boot of image at addr 0x%08lX ...\n",
			load_addr);
		rcode = do_bootm (cmdtp, 0, 1, local_args);
	}

#ifdef CONFIG_AUTOSCRIPT
	if (((s = getenv("autoscript")) != NULL) && (strcmp(s,"yes") == 0)) {
		printf("Running autoscript at addr 0x%08lX ...\n", load_addr);
		rcode = autoscript (load_addr);
	}
#endif
	return rcode;
}

#endif	/* CFG_CMD_NET */
