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
#include <net.h>

static int netboot_common(enum proto_t, cmd_tbl_t *, int, char * const []);

static int do_bootp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return netboot_common(BOOTP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	bootp,	3,	1,	do_bootp,
	"boot image via network using BOOTP/TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);

int do_tftpb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;

	bootstage_mark_name(BOOTSTAGE_KERNELREAD_START, "tftp_start");
	ret = netboot_common(TFTPGET, cmdtp, argc, argv);
	bootstage_mark_name(BOOTSTAGE_KERNELREAD_STOP, "tftp_done");
	return ret;
}

U_BOOT_CMD(
	tftpboot,	3,	1,	do_tftpb,
	"boot image via network using TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);

#ifdef CONFIG_CMD_TFTPPUT
int do_tftpput(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;

	ret = netboot_common(TFTPPUT, cmdtp, argc, argv);
	return ret;
}

U_BOOT_CMD(
	tftpput,	4,	1,	do_tftpput,
	"TFTP put command, for uploading files to a server",
	"Address Size [[hostIPaddr:]filename]"
);
#endif

#ifdef CONFIG_CMD_TFTPSRV
static int do_tftpsrv(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	return netboot_common(TFTPSRV, cmdtp, argc, argv);
}

U_BOOT_CMD(
	tftpsrv,	2,	1,	do_tftpsrv,
	"act as a TFTP server and boot the first received file",
	"[loadAddress]\n"
	"Listen for an incoming TFTP transfer, receive a file and boot it.\n"
	"The transfer is aborted if a transfer has not been started after\n"
	"about 50 seconds or if Ctrl-C is pressed."
);
#endif


#ifdef CONFIG_CMD_RARP
int do_rarpb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return netboot_common(RARP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	rarpboot,	3,	1,	do_rarpb,
	"boot image via network using RARP/TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

#if defined(CONFIG_CMD_DHCP)
static int do_dhcp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return netboot_common(DHCP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	dhcp,	3,	1,	do_dhcp,
	"boot image via network using DHCP/TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

#if defined(CONFIG_CMD_NFS)
static int do_nfs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return netboot_common(NFS, cmdtp, argc, argv);
}

U_BOOT_CMD(
	nfs,	3,	1,	do_nfs,
	"boot image via network using NFS protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

static void netboot_update_env(void)
{
	char tmp[22];

	if (NetOurGatewayIP) {
		ip_to_string(NetOurGatewayIP, tmp);
		setenv("gatewayip", tmp);
	}

	if (NetOurSubnetMask) {
		ip_to_string(NetOurSubnetMask, tmp);
		setenv("netmask", tmp);
	}

	if (NetOurHostName[0])
		setenv("hostname", NetOurHostName);

	if (NetOurRootPath[0])
		setenv("rootpath", NetOurRootPath);

	if (NetOurIP) {
		ip_to_string(NetOurIP, tmp);
		setenv("ipaddr", tmp);
	}
#if !defined(CONFIG_BOOTP_SERVERIP)
	/*
	 * Only attempt to change serverip if net/bootp.c:BootpCopyNetParams()
	 * could have set it
	 */
	if (NetServerIP) {
		ip_to_string(NetServerIP, tmp);
		setenv("serverip", tmp);
	}
#endif
	if (NetOurDNSIP) {
		ip_to_string(NetOurDNSIP, tmp);
		setenv("dnsip", tmp);
	}
#if defined(CONFIG_BOOTP_DNS2)
	if (NetOurDNS2IP) {
		ip_to_string(NetOurDNS2IP, tmp);
		setenv("dnsip2", tmp);
	}
#endif
	if (NetOurNISDomain[0])
		setenv("domain", NetOurNISDomain);

#if defined(CONFIG_CMD_SNTP) \
    && defined(CONFIG_BOOTP_TIMEOFFSET)
	if (NetTimeOffset) {
		sprintf(tmp, "%d", NetTimeOffset);
		setenv("timeoffset", tmp);
	}
#endif
#if defined(CONFIG_CMD_SNTP) \
    && defined(CONFIG_BOOTP_NTPSERVER)
	if (NetNtpServerIP) {
		ip_to_string(NetNtpServerIP, tmp);
		setenv("ntpserverip", tmp);
	}
#endif
}

static int netboot_common(enum proto_t proto, cmd_tbl_t *cmdtp, int argc,
		char * const argv[])
{
	char *s;
	char *end;
	int   rcode = 0;
	int   size;
	ulong addr;

	/* pre-set load_addr */
	if ((s = getenv("loadaddr")) != NULL) {
		load_addr = simple_strtoul(s, NULL, 16);
	}

	switch (argc) {
	case 1:
		break;

	case 2:	/*
		 * Only one arg - accept two forms:
		 * Just load address, or just boot file name. The latter
		 * form must be written in a format which can not be
		 * mis-interpreted as a valid number.
		 */
		addr = simple_strtoul(argv[1], &end, 16);
		if (end == (argv[1] + strlen(argv[1])))
			load_addr = addr;
		else
			copy_filename(BootFile, argv[1], sizeof(BootFile));
		break;

	case 3:	load_addr = simple_strtoul(argv[1], NULL, 16);
		copy_filename(BootFile, argv[2], sizeof(BootFile));

		break;

#ifdef CONFIG_CMD_TFTPPUT
	case 4:
		if (strict_strtoul(argv[1], 16, &save_addr) < 0 ||
			strict_strtoul(argv[2], 16, &save_size) < 0) {
			printf("Invalid address/size\n");
			return cmd_usage(cmdtp);
		}
		copy_filename(BootFile, argv[3], sizeof(BootFile));
		break;
#endif
	default:
		bootstage_error(BOOTSTAGE_ID_NET_START);
		return CMD_RET_USAGE;
	}
	bootstage_mark(BOOTSTAGE_ID_NET_START);

	if ((size = NetLoop(proto)) < 0) {
		bootstage_error(BOOTSTAGE_ID_NET_NETLOOP_OK);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_NET_NETLOOP_OK);

	/* NetLoop ok, update environment */
	netboot_update_env();

	/* done if no file was loaded (no errors though) */
	if (size == 0) {
		bootstage_error(BOOTSTAGE_ID_NET_LOADED);
		return 0;
	}

	/* flush cache */
	flush_cache(load_addr, size);

	bootstage_mark(BOOTSTAGE_ID_NET_LOADED);

	rcode = bootm_maybe_autostart(cmdtp, argv[0]);

	if (rcode < 0)
		bootstage_error(BOOTSTAGE_ID_NET_DONE_ERR);
	else
		bootstage_mark(BOOTSTAGE_ID_NET_DONE);
	return rcode;
}

#if defined(CONFIG_CMD_PING)
static int do_ping(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return -1;

	NetPingIP = string_to_ip(argv[1]);
	if (NetPingIP == 0)
		return CMD_RET_USAGE;

	if (NetLoop(PING) < 0) {
		printf("ping failed; host %s is not alive\n", argv[1]);
		return 1;
	}

	printf("host %s is alive\n", argv[1]);

	return 0;
}

U_BOOT_CMD(
	ping,	2,	1,	do_ping,
	"send ICMP ECHO_REQUEST to network host",
	"pingAddress"
);
#endif

#if defined(CONFIG_CMD_CDP)

static void cdp_update_env(void)
{
	char tmp[16];

	if (CDPApplianceVLAN != htons(-1)) {
		printf("CDP offered appliance VLAN %d\n", ntohs(CDPApplianceVLAN));
		VLAN_to_string(CDPApplianceVLAN, tmp);
		setenv("vlan", tmp);
		NetOurVLAN = CDPApplianceVLAN;
	}

	if (CDPNativeVLAN != htons(-1)) {
		printf("CDP offered native VLAN %d\n", ntohs(CDPNativeVLAN));
		VLAN_to_string(CDPNativeVLAN, tmp);
		setenv("nvlan", tmp);
		NetOurNativeVLAN = CDPNativeVLAN;
	}

}

int do_cdp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int r;

	r = NetLoop(CDP);
	if (r < 0) {
		printf("cdp failed; perhaps not a CISCO switch?\n");
		return 1;
	}

	cdp_update_env();

	return 0;
}

U_BOOT_CMD(
	cdp,	1,	1,	do_cdp,
	"Perform CDP network configuration",
	"\n"
);
#endif

#if defined(CONFIG_CMD_SNTP)
int do_sntp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *toff;

	if (argc < 2) {
		NetNtpServerIP = getenv_IPaddr("ntpserverip");
		if (NetNtpServerIP == 0) {
			printf("ntpserverip not set\n");
			return (1);
		}
	} else {
		NetNtpServerIP = string_to_ip(argv[1]);
		if (NetNtpServerIP == 0) {
			printf("Bad NTP server IP address\n");
			return (1);
		}
	}

	toff = getenv("timeoffset");
	if (toff == NULL)
		NetTimeOffset = 0;
	else
		NetTimeOffset = simple_strtol(toff, NULL, 10);

	if (NetLoop(SNTP) < 0) {
		printf("SNTP failed: host %pI4 not responding\n",
			&NetNtpServerIP);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	sntp,	2,	1,	do_sntp,
	"synchronize RTC via network",
	"[NTP server IP]\n"
);
#endif

#if defined(CONFIG_CMD_DNS)
int do_dns(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc == 1)
		return CMD_RET_USAGE;

	/*
	 * We should check for a valid hostname:
	 * - Each label must be between 1 and 63 characters long
	 * - the entire hostname has a maximum of 255 characters
	 * - only the ASCII letters 'a' through 'z' (case-insensitive),
	 *   the digits '0' through '9', and the hyphen
	 * - cannot begin or end with a hyphen
	 * - no other symbols, punctuation characters, or blank spaces are
	 *   permitted
	 * but hey - this is a minimalist implmentation, so only check length
	 * and let the name server deal with things.
	 */
	if (strlen(argv[1]) >= 255) {
		printf("dns error: hostname too long\n");
		return 1;
	}

	NetDNSResolve = argv[1];

	if (argc == 3)
		NetDNSenvvar = argv[2];
	else
		NetDNSenvvar = NULL;

	if (NetLoop(DNS) < 0) {
		printf("dns lookup of %s failed, check setup\n", argv[1]);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	dns,	3,	1,	do_dns,
	"lookup the IP of a hostname",
	"hostname [envvar]"
);

#endif	/* CONFIG_CMD_DNS */

#if defined(CONFIG_CMD_LINK_LOCAL)
static int do_link_local(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	char tmp[22];

	if (NetLoop(LINKLOCAL) < 0)
		return 1;

	NetOurGatewayIP = 0;
	ip_to_string(NetOurGatewayIP, tmp);
	setenv("gatewayip", tmp);

	ip_to_string(NetOurSubnetMask, tmp);
	setenv("netmask", tmp);

	ip_to_string(NetOurIP, tmp);
	setenv("ipaddr", tmp);
	setenv("llipaddr", tmp); /* store this for next time */

	return 0;
}

U_BOOT_CMD(
	linklocal,	1,	1,	do_link_local,
	"acquire a network IP address using the link-local protocol",
	""
);

#endif  /* CONFIG_CMD_LINK_LOCAL */
