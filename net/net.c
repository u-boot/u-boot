// SPDX-License-Identifier: GPL-2.0
/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 */

/*
 * General Desription:
 *
 * The user interface supports commands for BOOTP, RARP, and TFTP.
 * Also, we support ARP internally. Depending on available data,
 * these interact as follows:
 *
 * BOOTP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *			- name of bootfile
 *	Next step:	ARP
 *
 * LINK_LOCAL:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *	Next step:	ARP
 *
 * RARP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *	Next step:	ARP
 *
 * ARP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *	We want:	- TFTP server ethernet address
 *	Next step:	TFTP
 *
 * DHCP:
 *
 *     Prerequisites:	- own ethernet address
 *     We want:		- IP, Netmask, ServerIP, Gateway IP
 *			- bootfilename, lease time
 *     Next step:	- TFTP
 *
 * TFTP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *			- TFTP server ethernet address
 *			- name of bootfile (if unknown, we use a default name
 *			  derived from our own IP address)
 *	We want:	- load the boot file
 *	Next step:	none
 *
 * NFS:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- name of bootfile (if unknown, we use a default name
 *			  derived from our own IP address)
 *	We want:	- load the boot file
 *	Next step:	none
 *
 * SNTP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *	We want:	- network time
 *	Next step:	none
 *
 * WOL:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- magic packet or timeout
 *	Next step:	none
 */


#include <common.h>
#include <command.h>
#include <console.h>
#include <env.h>
#include <env_internal.h>
#include <errno.h>
#include <net.h>
#include <net/fastboot.h>
#include <net/tftp.h>
#if defined(CONFIG_LED_STATUS)
#include <miiphy.h>
#include <status_led.h>
#endif
#include <watchdog.h>
#include <linux/compiler.h>
#include "arp.h"
#include "bootp.h"
#include "cdp.h"
#if defined(CONFIG_CMD_DNS)
#include "dns.h"
#endif
#include "link_local.h"
#include "nfs.h"
#include "ping.h"
#include "rarp.h"
#if defined(CONFIG_CMD_SNTP)
#include "sntp.h"
#endif
#if defined(CONFIG_CMD_WOL)
#include "wol.h"
#endif

/** BOOTP EXTENTIONS **/

/* Our subnet mask (0=unknown) */
struct in_addr net_netmask;
/* Our gateways IP address */
struct in_addr net_gateway;
/* Our DNS IP address */
struct in_addr net_dns_server;
#if defined(CONFIG_BOOTP_DNS2)
/* Our 2nd DNS IP address */
struct in_addr net_dns_server2;
#endif

/** END OF BOOTP EXTENTIONS **/

/* Our ethernet address */
u8 net_ethaddr[6];
/* Boot server enet address */
u8 net_server_ethaddr[6];
/* Our IP addr (0 = unknown) */
struct in_addr	net_ip;
/* Server IP addr (0 = unknown) */
struct in_addr	net_server_ip;
/* Current receive packet */
uchar *net_rx_packet;
/* Current rx packet length */
int		net_rx_packet_len;
/* IP packet ID */
static unsigned	net_ip_id;
/* Ethernet bcast address */
const u8 net_bcast_ethaddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
const u8 net_null_ethaddr[6];
#if defined(CONFIG_API) || defined(CONFIG_EFI_LOADER)
void (*push_packet)(void *, int len) = 0;
#endif
/* Network loop state */
enum net_loop_state net_state;
/* Tried all network devices */
int		net_restart_wrap;
/* Network loop restarted */
static int	net_restarted;
/* At least one device configured */
static int	net_dev_exists;

/* XXX in both little & big endian machines 0xFFFF == ntohs(-1) */
/* default is without VLAN */
ushort		net_our_vlan = 0xFFFF;
/* ditto */
ushort		net_native_vlan = 0xFFFF;

/* Boot File name */
char net_boot_file_name[1024];
/* Indicates whether the file name was specified on the command line */
bool net_boot_file_name_explicit;
/* The actual transferred size of the bootfile (in bytes) */
u32 net_boot_file_size;
/* Boot file size in blocks as reported by the DHCP server */
u32 net_boot_file_expected_size_in_blocks;

#if defined(CONFIG_CMD_SNTP)
/* NTP server IP address */
struct in_addr	net_ntp_server;
/* offset time from UTC */
int		net_ntp_time_offset;
#endif

static uchar net_pkt_buf[(PKTBUFSRX+1) * PKTSIZE_ALIGN + PKTALIGN];
/* Receive packets */
uchar *net_rx_packets[PKTBUFSRX];
/* Current UDP RX packet handler */
static rxhand_f *udp_packet_handler;
/* Current ARP RX packet handler */
static rxhand_f *arp_packet_handler;
#ifdef CONFIG_CMD_TFTPPUT
/* Current ICMP rx handler */
static rxhand_icmp_f *packet_icmp_handler;
#endif
/* Current timeout handler */
static thand_f *time_handler;
/* Time base value */
static ulong	time_start;
/* Current timeout value */
static ulong	time_delta;
/* THE transmit packet */
uchar *net_tx_packet;

static int net_check_prereq(enum proto_t protocol);

static int net_try_count;

int __maybe_unused net_busy_flag;

/**********************************************************************/

static int on_ipaddr(const char *name, const char *value, enum env_op op,
	int flags)
{
	if (flags & H_PROGRAMMATIC)
		return 0;

	net_ip = string_to_ip(value);

	return 0;
}
U_BOOT_ENV_CALLBACK(ipaddr, on_ipaddr);

static int on_gatewayip(const char *name, const char *value, enum env_op op,
	int flags)
{
	if (flags & H_PROGRAMMATIC)
		return 0;

	net_gateway = string_to_ip(value);

	return 0;
}
U_BOOT_ENV_CALLBACK(gatewayip, on_gatewayip);

static int on_netmask(const char *name, const char *value, enum env_op op,
	int flags)
{
	if (flags & H_PROGRAMMATIC)
		return 0;

	net_netmask = string_to_ip(value);

	return 0;
}
U_BOOT_ENV_CALLBACK(netmask, on_netmask);

static int on_serverip(const char *name, const char *value, enum env_op op,
	int flags)
{
	if (flags & H_PROGRAMMATIC)
		return 0;

	net_server_ip = string_to_ip(value);

	return 0;
}
U_BOOT_ENV_CALLBACK(serverip, on_serverip);

static int on_nvlan(const char *name, const char *value, enum env_op op,
	int flags)
{
	if (flags & H_PROGRAMMATIC)
		return 0;

	net_native_vlan = string_to_vlan(value);

	return 0;
}
U_BOOT_ENV_CALLBACK(nvlan, on_nvlan);

static int on_vlan(const char *name, const char *value, enum env_op op,
	int flags)
{
	if (flags & H_PROGRAMMATIC)
		return 0;

	net_our_vlan = string_to_vlan(value);

	return 0;
}
U_BOOT_ENV_CALLBACK(vlan, on_vlan);

#if defined(CONFIG_CMD_DNS)
static int on_dnsip(const char *name, const char *value, enum env_op op,
	int flags)
{
	if (flags & H_PROGRAMMATIC)
		return 0;

	net_dns_server = string_to_ip(value);

	return 0;
}
U_BOOT_ENV_CALLBACK(dnsip, on_dnsip);
#endif

/*
 * Check if autoload is enabled. If so, use either NFS or TFTP to download
 * the boot file.
 */
void net_auto_load(void)
{
#if defined(CONFIG_CMD_NFS)
	const char *s = env_get("autoload");

	if (s != NULL && strcmp(s, "NFS") == 0) {
		if (net_check_prereq(NFS)) {
/* We aren't expecting to get a serverip, so just accept the assigned IP */
#ifdef CONFIG_BOOTP_SERVERIP
			net_set_state(NETLOOP_SUCCESS);
#else
			printf("Cannot autoload with NFS\n");
			net_set_state(NETLOOP_FAIL);
#endif
			return;
		}
		/*
		 * Use NFS to load the bootfile.
		 */
		nfs_start();
		return;
	}
#endif
	if (env_get_yesno("autoload") == 0) {
		/*
		 * Just use BOOTP/RARP to configure system;
		 * Do not use TFTP to load the bootfile.
		 */
		net_set_state(NETLOOP_SUCCESS);
		return;
	}
	if (net_check_prereq(TFTPGET)) {
/* We aren't expecting to get a serverip, so just accept the assigned IP */
#ifdef CONFIG_BOOTP_SERVERIP
		net_set_state(NETLOOP_SUCCESS);
#else
		printf("Cannot autoload with TFTPGET\n");
		net_set_state(NETLOOP_FAIL);
#endif
		return;
	}
	tftp_start(TFTPGET);
}

static void net_init_loop(void)
{
	if (eth_get_dev())
		memcpy(net_ethaddr, eth_get_ethaddr(), 6);

	return;
}

static void net_clear_handlers(void)
{
	net_set_udp_handler(NULL);
	net_set_arp_handler(NULL);
	net_set_timeout_handler(0, NULL);
}

static void net_cleanup_loop(void)
{
	net_clear_handlers();
}

void net_init(void)
{
	static int first_call = 1;

	if (first_call) {
		/*
		 *	Setup packet buffers, aligned correctly.
		 */
		int i;

		net_tx_packet = &net_pkt_buf[0] + (PKTALIGN - 1);
		net_tx_packet -= (ulong)net_tx_packet % PKTALIGN;
		for (i = 0; i < PKTBUFSRX; i++) {
			net_rx_packets[i] = net_tx_packet +
				(i + 1) * PKTSIZE_ALIGN;
		}
		arp_init();
		net_clear_handlers();

		/* Only need to setup buffer pointers once. */
		first_call = 0;
	}

	net_init_loop();
}

/**********************************************************************/
/*
 *	Main network processing loop.
 */

int net_loop(enum proto_t protocol)
{
	int ret = -EINVAL;
	enum net_loop_state prev_net_state = net_state;

	net_restarted = 0;
	net_dev_exists = 0;
	net_try_count = 1;
	debug_cond(DEBUG_INT_STATE, "--- net_loop Entry\n");

	bootstage_mark_name(BOOTSTAGE_ID_ETH_START, "eth_start");
	net_init();
	if (eth_is_on_demand_init() || protocol != NETCONS) {
		eth_halt();
		eth_set_current();
		ret = eth_init();
		if (ret < 0) {
			eth_halt();
			return ret;
		}
	} else {
		eth_init_state_only();
	}
restart:
#ifdef CONFIG_USB_KEYBOARD
	net_busy_flag = 0;
#endif
	net_set_state(NETLOOP_CONTINUE);

	/*
	 *	Start the ball rolling with the given start function.  From
	 *	here on, this code is a state machine driven by received
	 *	packets and timer events.
	 */
	debug_cond(DEBUG_INT_STATE, "--- net_loop Init\n");
	net_init_loop();

	switch (net_check_prereq(protocol)) {
	case 1:
		/* network not configured */
		eth_halt();
		net_set_state(prev_net_state);
		return -ENODEV;

	case 2:
		/* network device not configured */
		break;

	case 0:
		net_dev_exists = 1;
		net_boot_file_size = 0;
		switch (protocol) {
		case TFTPGET:
#ifdef CONFIG_CMD_TFTPPUT
		case TFTPPUT:
#endif
			/* always use ARP to get server ethernet address */
			tftp_start(protocol);
			break;
#ifdef CONFIG_CMD_TFTPSRV
		case TFTPSRV:
			tftp_start_server();
			break;
#endif
#ifdef CONFIG_UDP_FUNCTION_FASTBOOT
		case FASTBOOT:
			fastboot_start_server();
			break;
#endif
#if defined(CONFIG_CMD_DHCP)
		case DHCP:
			bootp_reset();
			net_ip.s_addr = 0;
			dhcp_request();		/* Basically same as BOOTP */
			break;
#endif

		case BOOTP:
			bootp_reset();
			net_ip.s_addr = 0;
			bootp_request();
			break;

#if defined(CONFIG_CMD_RARP)
		case RARP:
			rarp_try = 0;
			net_ip.s_addr = 0;
			rarp_request();
			break;
#endif
#if defined(CONFIG_CMD_PING)
		case PING:
			ping_start();
			break;
#endif
#if defined(CONFIG_CMD_NFS)
		case NFS:
			nfs_start();
			break;
#endif
#if defined(CONFIG_CMD_CDP)
		case CDP:
			cdp_start();
			break;
#endif
#if defined(CONFIG_NETCONSOLE) && !defined(CONFIG_SPL_BUILD)
		case NETCONS:
			nc_start();
			break;
#endif
#if defined(CONFIG_CMD_SNTP)
		case SNTP:
			sntp_start();
			break;
#endif
#if defined(CONFIG_CMD_DNS)
		case DNS:
			dns_start();
			break;
#endif
#if defined(CONFIG_CMD_LINK_LOCAL)
		case LINKLOCAL:
			link_local_start();
			break;
#endif
#if defined(CONFIG_CMD_WOL)
		case WOL:
			wol_start();
			break;
#endif
		default:
			break;
		}

		break;
	}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
#if	defined(CONFIG_SYS_FAULT_ECHO_LINK_DOWN)	&& \
	defined(CONFIG_LED_STATUS)			&& \
	defined(CONFIG_LED_STATUS_RED)
	/*
	 * Echo the inverted link state to the fault LED.
	 */
	if (miiphy_link(eth_get_dev()->name, CONFIG_SYS_FAULT_MII_ADDR))
		status_led_set(CONFIG_LED_STATUS_RED, CONFIG_LED_STATUS_OFF);
	else
		status_led_set(CONFIG_LED_STATUS_RED, CONFIG_LED_STATUS_ON);
#endif /* CONFIG_SYS_FAULT_ECHO_LINK_DOWN, ... */
#endif /* CONFIG_MII, ... */
#ifdef CONFIG_USB_KEYBOARD
	net_busy_flag = 1;
#endif

	/*
	 *	Main packet reception loop.  Loop receiving packets until
	 *	someone sets `net_state' to a state that terminates.
	 */
	for (;;) {
		WATCHDOG_RESET();
#ifdef CONFIG_SHOW_ACTIVITY
		show_activity(1);
#endif
		if (arp_timeout_check() > 0)
			time_start = get_timer(0);

		/*
		 *	Check the ethernet for a new packet.  The ethernet
		 *	receive routine will process it.
		 *	Most drivers return the most recent packet size, but not
		 *	errors that may have happened.
		 */
		eth_rx();

		/*
		 *	Abort if ctrl-c was pressed.
		 */
		if (ctrlc()) {
			/* cancel any ARP that may not have completed */
			net_arp_wait_packet_ip.s_addr = 0;

			net_cleanup_loop();
			eth_halt();
			/* Invalidate the last protocol */
			eth_set_last_protocol(BOOTP);

			puts("\nAbort\n");
			/* include a debug print as well incase the debug
			   messages are directed to stderr */
			debug_cond(DEBUG_INT_STATE, "--- net_loop Abort!\n");
			ret = -EINTR;
			goto done;
		}

		/*
		 *	Check for a timeout, and run the timeout handler
		 *	if we have one.
		 */
		if (time_handler &&
		    ((get_timer(0) - time_start) > time_delta)) {
			thand_f *x;

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
#if	defined(CONFIG_SYS_FAULT_ECHO_LINK_DOWN)	&& \
	defined(CONFIG_LED_STATUS)			&& \
	defined(CONFIG_LED_STATUS_RED)
			/*
			 * Echo the inverted link state to the fault LED.
			 */
			if (miiphy_link(eth_get_dev()->name,
					CONFIG_SYS_FAULT_MII_ADDR))
				status_led_set(CONFIG_LED_STATUS_RED,
					       CONFIG_LED_STATUS_OFF);
			else
				status_led_set(CONFIG_LED_STATUS_RED,
					       CONFIG_LED_STATUS_ON);
#endif /* CONFIG_SYS_FAULT_ECHO_LINK_DOWN, ... */
#endif /* CONFIG_MII, ... */
			debug_cond(DEBUG_INT_STATE, "--- net_loop timeout\n");
			x = time_handler;
			time_handler = (thand_f *)0;
			(*x)();
		}

		if (net_state == NETLOOP_FAIL)
			ret = net_start_again();

		switch (net_state) {
		case NETLOOP_RESTART:
			net_restarted = 1;
			goto restart;

		case NETLOOP_SUCCESS:
			net_cleanup_loop();
			if (net_boot_file_size > 0) {
				printf("Bytes transferred = %d (%x hex)\n",
				       net_boot_file_size, net_boot_file_size);
				env_set_hex("filesize", net_boot_file_size);
				env_set_hex("fileaddr", load_addr);
			}
			if (protocol != NETCONS)
				eth_halt();
			else
				eth_halt_state_only();

			eth_set_last_protocol(protocol);

			ret = net_boot_file_size;
			debug_cond(DEBUG_INT_STATE, "--- net_loop Success!\n");
			goto done;

		case NETLOOP_FAIL:
			net_cleanup_loop();
			/* Invalidate the last protocol */
			eth_set_last_protocol(BOOTP);
			debug_cond(DEBUG_INT_STATE, "--- net_loop Fail!\n");
			ret = -ENONET;
			goto done;

		case NETLOOP_CONTINUE:
			continue;
		}
	}

done:
#ifdef CONFIG_USB_KEYBOARD
	net_busy_flag = 0;
#endif
#ifdef CONFIG_CMD_TFTPPUT
	/* Clear out the handlers */
	net_set_udp_handler(NULL);
	net_set_icmp_handler(NULL);
#endif
	net_set_state(prev_net_state);
	return ret;
}

/**********************************************************************/

static void start_again_timeout_handler(void)
{
	net_set_state(NETLOOP_RESTART);
}

int net_start_again(void)
{
	char *nretry;
	int retry_forever = 0;
	unsigned long retrycnt = 0;
	int ret;

	nretry = env_get("netretry");
	if (nretry) {
		if (!strcmp(nretry, "yes"))
			retry_forever = 1;
		else if (!strcmp(nretry, "no"))
			retrycnt = 0;
		else if (!strcmp(nretry, "once"))
			retrycnt = 1;
		else
			retrycnt = simple_strtoul(nretry, NULL, 0);
	} else {
		retrycnt = 0;
		retry_forever = 0;
	}

	if ((!retry_forever) && (net_try_count > retrycnt)) {
		eth_halt();
		net_set_state(NETLOOP_FAIL);
		/*
		 * We don't provide a way for the protocol to return an error,
		 * but this is almost always the reason.
		 */
		return -ETIMEDOUT;
	}

	net_try_count++;

	eth_halt();
#if !defined(CONFIG_NET_DO_NOT_TRY_ANOTHER)
	eth_try_another(!net_restarted);
#endif
	ret = eth_init();
	if (net_restart_wrap) {
		net_restart_wrap = 0;
		if (net_dev_exists) {
			net_set_timeout_handler(10000UL,
						start_again_timeout_handler);
			net_set_udp_handler(NULL);
		} else {
			net_set_state(NETLOOP_FAIL);
		}
	} else {
		net_set_state(NETLOOP_RESTART);
	}
	return ret;
}

/**********************************************************************/
/*
 *	Miscelaneous bits.
 */

static void dummy_handler(uchar *pkt, unsigned dport,
			struct in_addr sip, unsigned sport,
			unsigned len)
{
}

rxhand_f *net_get_udp_handler(void)
{
	return udp_packet_handler;
}

void net_set_udp_handler(rxhand_f *f)
{
	debug_cond(DEBUG_INT_STATE, "--- net_loop UDP handler set (%p)\n", f);
	if (f == NULL)
		udp_packet_handler = dummy_handler;
	else
		udp_packet_handler = f;
}

rxhand_f *net_get_arp_handler(void)
{
	return arp_packet_handler;
}

void net_set_arp_handler(rxhand_f *f)
{
	debug_cond(DEBUG_INT_STATE, "--- net_loop ARP handler set (%p)\n", f);
	if (f == NULL)
		arp_packet_handler = dummy_handler;
	else
		arp_packet_handler = f;
}

#ifdef CONFIG_CMD_TFTPPUT
void net_set_icmp_handler(rxhand_icmp_f *f)
{
	packet_icmp_handler = f;
}
#endif

void net_set_timeout_handler(ulong iv, thand_f *f)
{
	if (iv == 0) {
		debug_cond(DEBUG_INT_STATE,
			   "--- net_loop timeout handler cancelled\n");
		time_handler = (thand_f *)0;
	} else {
		debug_cond(DEBUG_INT_STATE,
			   "--- net_loop timeout handler set (%p)\n", f);
		time_handler = f;
		time_start = get_timer(0);
		time_delta = iv * CONFIG_SYS_HZ / 1000;
	}
}

uchar *net_get_async_tx_pkt_buf(void)
{
	if (arp_is_waiting())
		return arp_tx_packet; /* If we are waiting, we already sent */
	else
		return net_tx_packet;
}

int net_send_udp_packet(uchar *ether, struct in_addr dest, int dport, int sport,
		int payload_len)
{
	return net_send_ip_packet(ether, dest, dport, sport, payload_len,
				  IPPROTO_UDP, 0, 0, 0);
}

int net_send_ip_packet(uchar *ether, struct in_addr dest, int dport, int sport,
		       int payload_len, int proto, u8 action, u32 tcp_seq_num,
		       u32 tcp_ack_num)
{
	uchar *pkt;
	int eth_hdr_size;
	int pkt_hdr_size;

	/* make sure the net_tx_packet is initialized (net_init() was called) */
	assert(net_tx_packet != NULL);
	if (net_tx_packet == NULL)
		return -1;

	/* convert to new style broadcast */
	if (dest.s_addr == 0)
		dest.s_addr = 0xFFFFFFFF;

	/* if broadcast, make the ether address a broadcast and don't do ARP */
	if (dest.s_addr == 0xFFFFFFFF)
		ether = (uchar *)net_bcast_ethaddr;

	pkt = (uchar *)net_tx_packet;

	eth_hdr_size = net_set_ether(pkt, ether, PROT_IP);

	switch (proto) {
	case IPPROTO_UDP:
		net_set_udp_header(pkt + eth_hdr_size, dest, dport, sport,
				   payload_len);
		pkt_hdr_size = eth_hdr_size + IP_UDP_HDR_SIZE;
		break;
	default:
		return -EINVAL;
	}

	/* if MAC address was not discovered yet, do an ARP request */
	if (memcmp(ether, net_null_ethaddr, 6) == 0) {
		debug_cond(DEBUG_DEV_PKT, "sending ARP for %pI4\n", &dest);

		/* save the ip and eth addr for the packet to send after arp */
		net_arp_wait_packet_ip = dest;
		arp_wait_packet_ethaddr = ether;

		/* size of the waiting packet */
		arp_wait_tx_packet_size = pkt_hdr_size + payload_len;

		/* and do the ARP request */
		arp_wait_try = 1;
		arp_wait_timer_start = get_timer(0);
		arp_request();
		return 1;	/* waiting */
	} else {
		debug_cond(DEBUG_DEV_PKT, "sending UDP to %pI4/%pM\n",
			   &dest, ether);
		net_send_packet(net_tx_packet, pkt_hdr_size + payload_len);
		return 0;	/* transmitted */
	}
}

#ifdef CONFIG_IP_DEFRAG
/*
 * This function collects fragments in a single packet, according
 * to the algorithm in RFC815. It returns NULL or the pointer to
 * a complete packet, in static storage
 */
#ifndef CONFIG_NET_MAXDEFRAG
#define CONFIG_NET_MAXDEFRAG 16384
#endif
#define IP_PKTSIZE (CONFIG_NET_MAXDEFRAG)

#define IP_MAXUDP (IP_PKTSIZE - IP_HDR_SIZE)

/*
 * this is the packet being assembled, either data or frag control.
 * Fragments go by 8 bytes, so this union must be 8 bytes long
 */
struct hole {
	/* first_byte is address of this structure */
	u16 last_byte;	/* last byte in this hole + 1 (begin of next hole) */
	u16 next_hole;	/* index of next (in 8-b blocks), 0 == none */
	u16 prev_hole;	/* index of prev, 0 == none */
	u16 unused;
};

static struct ip_udp_hdr *__net_defragment(struct ip_udp_hdr *ip, int *lenp)
{
	static uchar pkt_buff[IP_PKTSIZE] __aligned(PKTALIGN);
	static u16 first_hole, total_len;
	struct hole *payload, *thisfrag, *h, *newh;
	struct ip_udp_hdr *localip = (struct ip_udp_hdr *)pkt_buff;
	uchar *indata = (uchar *)ip;
	int offset8, start, len, done = 0;
	u16 ip_off = ntohs(ip->ip_off);

	/* payload starts after IP header, this fragment is in there */
	payload = (struct hole *)(pkt_buff + IP_HDR_SIZE);
	offset8 =  (ip_off & IP_OFFS);
	thisfrag = payload + offset8;
	start = offset8 * 8;
	len = ntohs(ip->ip_len) - IP_HDR_SIZE;

	if (start + len > IP_MAXUDP) /* fragment extends too far */
		return NULL;

	if (!total_len || localip->ip_id != ip->ip_id) {
		/* new (or different) packet, reset structs */
		total_len = 0xffff;
		payload[0].last_byte = ~0;
		payload[0].next_hole = 0;
		payload[0].prev_hole = 0;
		first_hole = 0;
		/* any IP header will work, copy the first we received */
		memcpy(localip, ip, IP_HDR_SIZE);
	}

	/*
	 * What follows is the reassembly algorithm. We use the payload
	 * array as a linked list of hole descriptors, as each hole starts
	 * at a multiple of 8 bytes. However, last byte can be whatever value,
	 * so it is represented as byte count, not as 8-byte blocks.
	 */

	h = payload + first_hole;
	while (h->last_byte < start) {
		if (!h->next_hole) {
			/* no hole that far away */
			return NULL;
		}
		h = payload + h->next_hole;
	}

	/* last fragment may be 1..7 bytes, the "+7" forces acceptance */
	if (offset8 + ((len + 7) / 8) <= h - payload) {
		/* no overlap with holes (dup fragment?) */
		return NULL;
	}

	if (!(ip_off & IP_FLAGS_MFRAG)) {
		/* no more fragmentss: truncate this (last) hole */
		total_len = start + len;
		h->last_byte = start + len;
	}

	/*
	 * There is some overlap: fix the hole list. This code doesn't
	 * deal with a fragment that overlaps with two different holes
	 * (thus being a superset of a previously-received fragment).
	 */

	if ((h >= thisfrag) && (h->last_byte <= start + len)) {
		/* complete overlap with hole: remove hole */
		if (!h->prev_hole && !h->next_hole) {
			/* last remaining hole */
			done = 1;
		} else if (!h->prev_hole) {
			/* first hole */
			first_hole = h->next_hole;
			payload[h->next_hole].prev_hole = 0;
		} else if (!h->next_hole) {
			/* last hole */
			payload[h->prev_hole].next_hole = 0;
		} else {
			/* in the middle of the list */
			payload[h->next_hole].prev_hole = h->prev_hole;
			payload[h->prev_hole].next_hole = h->next_hole;
		}

	} else if (h->last_byte <= start + len) {
		/* overlaps with final part of the hole: shorten this hole */
		h->last_byte = start;

	} else if (h >= thisfrag) {
		/* overlaps with initial part of the hole: move this hole */
		newh = thisfrag + (len / 8);
		*newh = *h;
		h = newh;
		if (h->next_hole)
			payload[h->next_hole].prev_hole = (h - payload);
		if (h->prev_hole)
			payload[h->prev_hole].next_hole = (h - payload);
		else
			first_hole = (h - payload);

	} else {
		/* fragment sits in the middle: split the hole */
		newh = thisfrag + (len / 8);
		*newh = *h;
		h->last_byte = start;
		h->next_hole = (newh - payload);
		newh->prev_hole = (h - payload);
		if (newh->next_hole)
			payload[newh->next_hole].prev_hole = (newh - payload);
	}

	/* finally copy this fragment and possibly return whole packet */
	memcpy((uchar *)thisfrag, indata + IP_HDR_SIZE, len);
	if (!done)
		return NULL;

	localip->ip_len = htons(total_len);
	*lenp = total_len + IP_HDR_SIZE;
	return localip;
}

static inline struct ip_udp_hdr *net_defragment(struct ip_udp_hdr *ip,
	int *lenp)
{
	u16 ip_off = ntohs(ip->ip_off);
	if (!(ip_off & (IP_OFFS | IP_FLAGS_MFRAG)))
		return ip; /* not a fragment */
	return __net_defragment(ip, lenp);
}

#else /* !CONFIG_IP_DEFRAG */

static inline struct ip_udp_hdr *net_defragment(struct ip_udp_hdr *ip,
	int *lenp)
{
	u16 ip_off = ntohs(ip->ip_off);
	if (!(ip_off & (IP_OFFS | IP_FLAGS_MFRAG)))
		return ip; /* not a fragment */
	return NULL;
}
#endif

/**
 * Receive an ICMP packet. We deal with REDIRECT and PING here, and silently
 * drop others.
 *
 * @parma ip	IP packet containing the ICMP
 */
static void receive_icmp(struct ip_udp_hdr *ip, int len,
			struct in_addr src_ip, struct ethernet_hdr *et)
{
	struct icmp_hdr *icmph = (struct icmp_hdr *)&ip->udp_src;

	switch (icmph->type) {
	case ICMP_REDIRECT:
		if (icmph->code != ICMP_REDIR_HOST)
			return;
		printf(" ICMP Host Redirect to %pI4 ",
		       &icmph->un.gateway);
		break;
	default:
#if defined(CONFIG_CMD_PING)
		ping_receive(et, ip, len);
#endif
#ifdef CONFIG_CMD_TFTPPUT
		if (packet_icmp_handler)
			packet_icmp_handler(icmph->type, icmph->code,
					    ntohs(ip->udp_dst), src_ip,
					    ntohs(ip->udp_src), icmph->un.data,
					    ntohs(ip->udp_len));
#endif
		break;
	}
}

void net_process_received_packet(uchar *in_packet, int len)
{
	struct ethernet_hdr *et;
	struct ip_udp_hdr *ip;
	struct in_addr dst_ip;
	struct in_addr src_ip;
	int eth_proto;
#if defined(CONFIG_CMD_CDP)
	int iscdp;
#endif
	ushort cti = 0, vlanid = VLAN_NONE, myvlanid, mynvlanid;

	debug_cond(DEBUG_NET_PKT, "packet received\n");

	net_rx_packet = in_packet;
	net_rx_packet_len = len;
	et = (struct ethernet_hdr *)in_packet;

	/* too small packet? */
	if (len < ETHER_HDR_SIZE)
		return;

#if defined(CONFIG_API) || defined(CONFIG_EFI_LOADER)
	if (push_packet) {
		(*push_packet)(in_packet, len);
		return;
	}
#endif

#if defined(CONFIG_CMD_CDP)
	/* keep track if packet is CDP */
	iscdp = is_cdp_packet(et->et_dest);
#endif

	myvlanid = ntohs(net_our_vlan);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;
	mynvlanid = ntohs(net_native_vlan);
	if (mynvlanid == (ushort)-1)
		mynvlanid = VLAN_NONE;

	eth_proto = ntohs(et->et_protlen);

	if (eth_proto < 1514) {
		struct e802_hdr *et802 = (struct e802_hdr *)et;
		/*
		 *	Got a 802.2 packet.  Check the other protocol field.
		 *	XXX VLAN over 802.2+SNAP not implemented!
		 */
		eth_proto = ntohs(et802->et_prot);

		ip = (struct ip_udp_hdr *)(in_packet + E802_HDR_SIZE);
		len -= E802_HDR_SIZE;

	} else if (eth_proto != PROT_VLAN) {	/* normal packet */
		ip = (struct ip_udp_hdr *)(in_packet + ETHER_HDR_SIZE);
		len -= ETHER_HDR_SIZE;

	} else {			/* VLAN packet */
		struct vlan_ethernet_hdr *vet =
			(struct vlan_ethernet_hdr *)et;

		debug_cond(DEBUG_NET_PKT, "VLAN packet received\n");

		/* too small packet? */
		if (len < VLAN_ETHER_HDR_SIZE)
			return;

		/* if no VLAN active */
		if ((ntohs(net_our_vlan) & VLAN_IDMASK) == VLAN_NONE
#if defined(CONFIG_CMD_CDP)
				&& iscdp == 0
#endif
				)
			return;

		cti = ntohs(vet->vet_tag);
		vlanid = cti & VLAN_IDMASK;
		eth_proto = ntohs(vet->vet_type);

		ip = (struct ip_udp_hdr *)(in_packet + VLAN_ETHER_HDR_SIZE);
		len -= VLAN_ETHER_HDR_SIZE;
	}

	debug_cond(DEBUG_NET_PKT, "Receive from protocol 0x%x\n", eth_proto);

#if defined(CONFIG_CMD_CDP)
	if (iscdp) {
		cdp_receive((uchar *)ip, len);
		return;
	}
#endif

	if ((myvlanid & VLAN_IDMASK) != VLAN_NONE) {
		if (vlanid == VLAN_NONE)
			vlanid = (mynvlanid & VLAN_IDMASK);
		/* not matched? */
		if (vlanid != (myvlanid & VLAN_IDMASK))
			return;
	}

	switch (eth_proto) {
	case PROT_ARP:
		arp_receive(et, ip, len);
		break;

#ifdef CONFIG_CMD_RARP
	case PROT_RARP:
		rarp_receive(ip, len);
		break;
#endif
	case PROT_IP:
		debug_cond(DEBUG_NET_PKT, "Got IP\n");
		/* Before we start poking the header, make sure it is there */
		if (len < IP_UDP_HDR_SIZE) {
			debug("len bad %d < %lu\n", len,
			      (ulong)IP_UDP_HDR_SIZE);
			return;
		}
		/* Check the packet length */
		if (len < ntohs(ip->ip_len)) {
			debug("len bad %d < %d\n", len, ntohs(ip->ip_len));
			return;
		}
		len = ntohs(ip->ip_len);
		debug_cond(DEBUG_NET_PKT, "len=%d, v=%02x\n",
			   len, ip->ip_hl_v & 0xff);

		/* Can't deal with anything except IPv4 */
		if ((ip->ip_hl_v & 0xf0) != 0x40)
			return;
		/* Can't deal with IP options (headers != 20 bytes) */
		if ((ip->ip_hl_v & 0x0f) > 0x05)
			return;
		/* Check the Checksum of the header */
		if (!ip_checksum_ok((uchar *)ip, IP_HDR_SIZE)) {
			debug("checksum bad\n");
			return;
		}
		/* If it is not for us, ignore it */
		dst_ip = net_read_ip(&ip->ip_dst);
		if (net_ip.s_addr && dst_ip.s_addr != net_ip.s_addr &&
		    dst_ip.s_addr != 0xFFFFFFFF) {
				return;
		}
		/* Read source IP address for later use */
		src_ip = net_read_ip(&ip->ip_src);
		/*
		 * The function returns the unchanged packet if it's not
		 * a fragment, and either the complete packet or NULL if
		 * it is a fragment (if !CONFIG_IP_DEFRAG, it returns NULL)
		 */
		ip = net_defragment(ip, &len);
		if (!ip)
			return;
		/*
		 * watch for ICMP host redirects
		 *
		 * There is no real handler code (yet). We just watch
		 * for ICMP host redirect messages. In case anybody
		 * sees these messages: please contact me
		 * (wd@denx.de), or - even better - send me the
		 * necessary fixes :-)
		 *
		 * Note: in all cases where I have seen this so far
		 * it was a problem with the router configuration,
		 * for instance when a router was configured in the
		 * BOOTP reply, but the TFTP server was on the same
		 * subnet. So this is probably a warning that your
		 * configuration might be wrong. But I'm not really
		 * sure if there aren't any other situations.
		 *
		 * Simon Glass <sjg@chromium.org>: We get an ICMP when
		 * we send a tftp packet to a dead connection, or when
		 * there is no server at the other end.
		 */
		if (ip->ip_p == IPPROTO_ICMP) {
			receive_icmp(ip, len, src_ip, et);
			return;
		} else if (ip->ip_p != IPPROTO_UDP) {	/* Only UDP packets */
			return;
		}

		debug_cond(DEBUG_DEV_PKT,
			   "received UDP (to=%pI4, from=%pI4, len=%d)\n",
			   &dst_ip, &src_ip, len);

#ifdef CONFIG_UDP_CHECKSUM
		if (ip->udp_xsum != 0) {
			ulong   xsum;
			ushort *sumptr;
			ushort  sumlen;

			xsum  = ip->ip_p;
			xsum += (ntohs(ip->udp_len));
			xsum += (ntohl(ip->ip_src.s_addr) >> 16) & 0x0000ffff;
			xsum += (ntohl(ip->ip_src.s_addr) >>  0) & 0x0000ffff;
			xsum += (ntohl(ip->ip_dst.s_addr) >> 16) & 0x0000ffff;
			xsum += (ntohl(ip->ip_dst.s_addr) >>  0) & 0x0000ffff;

			sumlen = ntohs(ip->udp_len);
			sumptr = (ushort *)&(ip->udp_src);

			while (sumlen > 1) {
				ushort sumdata;

				sumdata = *sumptr++;
				xsum += ntohs(sumdata);
				sumlen -= 2;
			}
			if (sumlen > 0) {
				ushort sumdata;

				sumdata = *(unsigned char *)sumptr;
				sumdata = (sumdata << 8) & 0xff00;
				xsum += sumdata;
			}
			while ((xsum >> 16) != 0) {
				xsum = (xsum & 0x0000ffff) +
				       ((xsum >> 16) & 0x0000ffff);
			}
			if ((xsum != 0x00000000) && (xsum != 0x0000ffff)) {
				printf(" UDP wrong checksum %08lx %08x\n",
				       xsum, ntohs(ip->udp_xsum));
				return;
			}
		}
#endif

#if defined(CONFIG_NETCONSOLE) && !defined(CONFIG_SPL_BUILD)
		nc_input_packet((uchar *)ip + IP_UDP_HDR_SIZE,
				src_ip,
				ntohs(ip->udp_dst),
				ntohs(ip->udp_src),
				ntohs(ip->udp_len) - UDP_HDR_SIZE);
#endif
		/*
		 * IP header OK.  Pass the packet to the current handler.
		 */
		(*udp_packet_handler)((uchar *)ip + IP_UDP_HDR_SIZE,
				      ntohs(ip->udp_dst),
				      src_ip,
				      ntohs(ip->udp_src),
				      ntohs(ip->udp_len) - UDP_HDR_SIZE);
		break;
#ifdef CONFIG_CMD_WOL
	case PROT_WOL:
		wol_receive(ip, len);
		break;
#endif
	}
}

/**********************************************************************/

static int net_check_prereq(enum proto_t protocol)
{
	switch (protocol) {
		/* Fall through */
#if defined(CONFIG_CMD_PING)
	case PING:
		if (net_ping_ip.s_addr == 0) {
			puts("*** ERROR: ping address not given\n");
			return 1;
		}
		goto common;
#endif
#if defined(CONFIG_CMD_SNTP)
	case SNTP:
		if (net_ntp_server.s_addr == 0) {
			puts("*** ERROR: NTP server address not given\n");
			return 1;
		}
		goto common;
#endif
#if defined(CONFIG_CMD_DNS)
	case DNS:
		if (net_dns_server.s_addr == 0) {
			puts("*** ERROR: DNS server address not given\n");
			return 1;
		}
		goto common;
#endif
#if defined(CONFIG_CMD_NFS)
	case NFS:
#endif
		/* Fall through */
	case TFTPGET:
	case TFTPPUT:
		if (net_server_ip.s_addr == 0 && !is_serverip_in_cmd()) {
			puts("*** ERROR: `serverip' not set\n");
			return 1;
		}
#if	defined(CONFIG_CMD_PING) || defined(CONFIG_CMD_SNTP) || \
	defined(CONFIG_CMD_DNS)
common:
#endif
		/* Fall through */

	case NETCONS:
	case FASTBOOT:
	case TFTPSRV:
		if (net_ip.s_addr == 0) {
			puts("*** ERROR: `ipaddr' not set\n");
			return 1;
		}
		/* Fall through */

#ifdef CONFIG_CMD_RARP
	case RARP:
#endif
	case BOOTP:
	case CDP:
	case DHCP:
	case LINKLOCAL:
		if (memcmp(net_ethaddr, "\0\0\0\0\0\0", 6) == 0) {
			int num = eth_get_dev_index();

			switch (num) {
			case -1:
				puts("*** ERROR: No ethernet found.\n");
				return 1;
			case 0:
				puts("*** ERROR: `ethaddr' not set\n");
				break;
			default:
				printf("*** ERROR: `eth%daddr' not set\n",
				       num);
				break;
			}

			net_start_again();
			return 2;
		}
		/* Fall through */
	default:
		return 0;
	}
	return 0;		/* OK */
}
/**********************************************************************/

int
net_eth_hdr_size(void)
{
	ushort myvlanid;

	myvlanid = ntohs(net_our_vlan);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;

	return ((myvlanid & VLAN_IDMASK) == VLAN_NONE) ? ETHER_HDR_SIZE :
		VLAN_ETHER_HDR_SIZE;
}

int net_set_ether(uchar *xet, const uchar *dest_ethaddr, uint prot)
{
	struct ethernet_hdr *et = (struct ethernet_hdr *)xet;
	ushort myvlanid;

	myvlanid = ntohs(net_our_vlan);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;

	memcpy(et->et_dest, dest_ethaddr, 6);
	memcpy(et->et_src, net_ethaddr, 6);
	if ((myvlanid & VLAN_IDMASK) == VLAN_NONE) {
		et->et_protlen = htons(prot);
		return ETHER_HDR_SIZE;
	} else {
		struct vlan_ethernet_hdr *vet =
			(struct vlan_ethernet_hdr *)xet;

		vet->vet_vlan_type = htons(PROT_VLAN);
		vet->vet_tag = htons((0 << 5) | (myvlanid & VLAN_IDMASK));
		vet->vet_type = htons(prot);
		return VLAN_ETHER_HDR_SIZE;
	}
}

int net_update_ether(struct ethernet_hdr *et, uchar *addr, uint prot)
{
	ushort protlen;

	memcpy(et->et_dest, addr, 6);
	memcpy(et->et_src, net_ethaddr, 6);
	protlen = ntohs(et->et_protlen);
	if (protlen == PROT_VLAN) {
		struct vlan_ethernet_hdr *vet =
			(struct vlan_ethernet_hdr *)et;
		vet->vet_type = htons(prot);
		return VLAN_ETHER_HDR_SIZE;
	} else if (protlen > 1514) {
		et->et_protlen = htons(prot);
		return ETHER_HDR_SIZE;
	} else {
		/* 802.2 + SNAP */
		struct e802_hdr *et802 = (struct e802_hdr *)et;
		et802->et_prot = htons(prot);
		return E802_HDR_SIZE;
	}
}

void net_set_ip_header(uchar *pkt, struct in_addr dest, struct in_addr source,
		       u16 pkt_len, u8 proto)
{
	struct ip_udp_hdr *ip = (struct ip_udp_hdr *)pkt;

	/*
	 *	Construct an IP header.
	 */
	/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_hl_v  = 0x45;
	ip->ip_tos   = 0;
	ip->ip_len   = htons(pkt_len);
	ip->ip_p     = proto;
	ip->ip_id    = htons(net_ip_id++);
	ip->ip_off   = htons(IP_FLAGS_DFRAG);	/* Don't fragment */
	ip->ip_ttl   = 255;
	ip->ip_sum   = 0;
	/* already in network byte order */
	net_copy_ip((void *)&ip->ip_src, &source);
	/* already in network byte order */
	net_copy_ip((void *)&ip->ip_dst, &dest);

	ip->ip_sum   = compute_ip_checksum(ip, IP_HDR_SIZE);
}

void net_set_udp_header(uchar *pkt, struct in_addr dest, int dport, int sport,
			int len)
{
	struct ip_udp_hdr *ip = (struct ip_udp_hdr *)pkt;

	/*
	 *	If the data is an odd number of bytes, zero the
	 *	byte after the last byte so that the checksum
	 *	will work.
	 */
	if (len & 1)
		pkt[IP_UDP_HDR_SIZE + len] = 0;

	net_set_ip_header(pkt, dest, net_ip, IP_UDP_HDR_SIZE + len,
			  IPPROTO_UDP);

	ip->udp_src  = htons(sport);
	ip->udp_dst  = htons(dport);
	ip->udp_len  = htons(UDP_HDR_SIZE + len);
	ip->udp_xsum = 0;
}

void copy_filename(char *dst, const char *src, int size)
{
	if (src && *src && (*src == '"')) {
		++src;
		--size;
	}

	while ((--size > 0) && src && *src && (*src != '"'))
		*dst++ = *src++;
	*dst = '\0';
}

int is_serverip_in_cmd(void)
{
	return !!strchr(net_boot_file_name, ':');
}

int net_parse_bootfile(struct in_addr *ipaddr, char *filename, int max_len)
{
	char *colon;

	if (net_boot_file_name[0] == '\0')
		return 0;

	colon = strchr(net_boot_file_name, ':');
	if (colon) {
		if (ipaddr)
			*ipaddr = string_to_ip(net_boot_file_name);
		strncpy(filename, colon + 1, max_len);
	} else {
		strncpy(filename, net_boot_file_name, max_len);
	}
	filename[max_len - 1] = '\0';

	return 1;
}

#if	defined(CONFIG_CMD_NFS)		|| \
	defined(CONFIG_CMD_SNTP)	|| \
	defined(CONFIG_CMD_DNS)
/*
 * make port a little random (1024-17407)
 * This keeps the math somewhat trivial to compute, and seems to work with
 * all supported protocols/clients/servers
 */
unsigned int random_port(void)
{
	return 1024 + (get_timer(0) % 0x4000);
}
#endif

void ip_to_string(struct in_addr x, char *s)
{
	x.s_addr = ntohl(x.s_addr);
	sprintf(s, "%d.%d.%d.%d",
		(int) ((x.s_addr >> 24) & 0xff),
		(int) ((x.s_addr >> 16) & 0xff),
		(int) ((x.s_addr >> 8) & 0xff),
		(int) ((x.s_addr >> 0) & 0xff)
	);
}

void vlan_to_string(ushort x, char *s)
{
	x = ntohs(x);

	if (x == (ushort)-1)
		x = VLAN_NONE;

	if (x == VLAN_NONE)
		strcpy(s, "none");
	else
		sprintf(s, "%d", x & VLAN_IDMASK);
}

ushort string_to_vlan(const char *s)
{
	ushort id;

	if (s == NULL)
		return htons(VLAN_NONE);

	if (*s < '0' || *s > '9')
		id = VLAN_NONE;
	else
		id = (ushort)simple_strtoul(s, NULL, 10);

	return htons(id);
}

ushort env_get_vlan(char *var)
{
	return string_to_vlan(env_get(var));
}

void eth_parse_enetaddr(const char *addr, uint8_t *enetaddr)
{
	char *end;
	int i;

	for (i = 0; i < 6; ++i) {
		enetaddr[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
		if (addr)
			addr = (*end) ? end + 1 : end;
	}
}
