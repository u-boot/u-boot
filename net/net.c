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
 */


#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <linux/compiler.h>
#include <net.h>
#include "arp.h"
#include "bootp.h"
#include "tftp.h"
#include "rarp.h"
#include "nfs.h"
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#include <miiphy.h>
#endif
#if defined(CONFIG_CMD_SNTP)
#include "sntp.h"
#endif
#include "cdp.h"
#if defined(CONFIG_CMD_DNS)
#include "dns.h"
#endif
#include "ping.h"

DECLARE_GLOBAL_DATA_PTR;

/** BOOTP EXTENTIONS **/

/* Our subnet mask (0=unknown) */
IPaddr_t	NetOurSubnetMask;
/* Our gateways IP address */
IPaddr_t	NetOurGatewayIP;
/* Our DNS IP address */
IPaddr_t	NetOurDNSIP;
#if defined(CONFIG_BOOTP_DNS2)
/* Our 2nd DNS IP address */
IPaddr_t	NetOurDNS2IP;
#endif
/* Our NIS domain */
char		NetOurNISDomain[32] = {0,};
/* Our hostname */
char		NetOurHostName[32] = {0,};
/* Our bootpath */
char		NetOurRootPath[64] = {0,};
/* Our bootfile size in blocks */
ushort		NetBootFileSize;

#ifdef CONFIG_MCAST_TFTP	/* Multicast TFTP */
IPaddr_t Mcast_addr;
#endif

/** END OF BOOTP EXTENTIONS **/

/* The actual transferred size of the bootfile (in bytes) */
ulong		NetBootFileXferSize;
/* Our ethernet address */
uchar		NetOurEther[6];
/* Boot server enet address */
uchar		NetServerEther[6];
/* Our IP addr (0 = unknown) */
IPaddr_t	NetOurIP;
/* Server IP addr (0 = unknown) */
IPaddr_t	NetServerIP;
/* Current receive packet */
uchar *NetRxPacket;
/* Current rx packet length */
int		NetRxPacketLen;
/* IP packet ID */
unsigned	NetIPID;
/* Ethernet bcast address */
uchar		NetBcastAddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
uchar		NetEtherNullAddr[6];
#ifdef CONFIG_API
void		(*push_packet)(void *, int len) = 0;
#endif
/* Network loop state */
int		NetState;
/* Tried all network devices */
int		NetRestartWrap;
/* Network loop restarted */
static int	NetRestarted;
/* At least one device configured */
static int	NetDevExists;

/* XXX in both little & big endian machines 0xFFFF == ntohs(-1) */
/* default is without VLAN */
ushort		NetOurVLAN = 0xFFFF;
/* ditto */
ushort		NetOurNativeVLAN = 0xFFFF;

/* Boot File name */
char		BootFile[128];

#if defined(CONFIG_CMD_SNTP)
/* NTP server IP address */
IPaddr_t	NetNtpServerIP;
/* offset time from UTC */
int		NetTimeOffset;
#endif

uchar PktBuf[(PKTBUFSRX+1) * PKTSIZE_ALIGN + PKTALIGN];

/* Receive packet */
uchar *NetRxPackets[PKTBUFSRX];

/* Current RX packet handler */
static rxhand_f *packetHandler;
#ifdef CONFIG_CMD_TFTPPUT
static rxhand_icmp_f *packet_icmp_handler;	/* Current ICMP rx handler */
#endif
/* Current timeout handler */
static thand_f *timeHandler;
/* Time base value */
static ulong	timeStart;
/* Current timeout value */
static ulong	timeDelta;
/* THE transmit packet */
uchar *NetTxPacket;

static int net_check_prereq(enum proto_t protocol);

static int NetTryCount;

/**********************************************************************/

/*
 * Check if autoload is enabled. If so, use either NFS or TFTP to download
 * the boot file.
 */
void net_auto_load(void)
{
	const char *s = getenv("autoload");

	if (s != NULL) {
		if (*s == 'n') {
			/*
			 * Just use BOOTP/RARP to configure system;
			 * Do not use TFTP to load the bootfile.
			 */
			NetState = NETLOOP_SUCCESS;
			return;
		}
#if defined(CONFIG_CMD_NFS)
		if (strcmp(s, "NFS") == 0) {
			/*
			 * Use NFS to load the bootfile.
			 */
			NfsStart();
			return;
		}
#endif
	}
	TftpStart(TFTPGET);
}

static void NetInitLoop(enum proto_t protocol)
{
	static int env_changed_id;
	int env_id = get_env_id();

	/* update only when the environment has changed */
	if (env_changed_id != env_id) {
		NetOurIP = getenv_IPaddr("ipaddr");
		NetOurGatewayIP = getenv_IPaddr("gatewayip");
		NetOurSubnetMask = getenv_IPaddr("netmask");
		NetServerIP = getenv_IPaddr("serverip");
		NetOurNativeVLAN = getenv_VLAN("nvlan");
		NetOurVLAN = getenv_VLAN("vlan");
#if defined(CONFIG_CMD_DNS)
		NetOurDNSIP = getenv_IPaddr("dnsip");
#endif
		env_changed_id = env_id;
	}

	return;
}

/**********************************************************************/
/*
 *	Main network processing loop.
 */

int NetLoop(enum proto_t protocol)
{
	bd_t *bd = gd->bd;
	int ret = -1;

	NetRestarted = 0;
	NetDevExists = 0;

	NetTxPacket = NULL;
	NetTryCount = 1;

	ArpInit();

	if (!NetTxPacket) {
		int	i;
		/*
		 *	Setup packet buffers, aligned correctly.
		 */
		NetTxPacket = &PktBuf[0] + (PKTALIGN - 1);
		NetTxPacket -= (ulong)NetTxPacket % PKTALIGN;
		for (i = 0; i < PKTBUFSRX; i++)
			NetRxPackets[i] = NetTxPacket + (i+1)*PKTSIZE_ALIGN;
	}

	bootstage_mark_name(BOOTSTAGE_ID_ETH_START, "eth_start");
	eth_halt();
	eth_set_current();
	if (eth_init(bd) < 0) {
		eth_halt();
		return -1;
	}

restart:
	memcpy(NetOurEther, eth_get_dev()->enetaddr, 6);

	NetState = NETLOOP_CONTINUE;

	/*
	 *	Start the ball rolling with the given start function.  From
	 *	here on, this code is a state machine driven by received
	 *	packets and timer events.
	 */
	NetInitLoop(protocol);

	switch (net_check_prereq(protocol)) {
	case 1:
		/* network not configured */
		eth_halt();
		return -1;

	case 2:
		/* network device not configured */
		break;

	case 0:
		NetDevExists = 1;
		NetBootFileXferSize = 0;
		switch (protocol) {
		case TFTPGET:
#ifdef CONFIG_CMD_TFTPPUT
		case TFTPPUT:
#endif
			/* always use ARP to get server ethernet address */
			TftpStart(protocol);
			break;
#ifdef CONFIG_CMD_TFTPSRV
		case TFTPSRV:
			TftpStartServer();
			break;
#endif
#if defined(CONFIG_CMD_DHCP)
		case DHCP:
			BootpTry = 0;
			NetOurIP = 0;
			DhcpRequest();		/* Basically same as BOOTP */
			break;
#endif

		case BOOTP:
			BootpTry = 0;
			NetOurIP = 0;
			BootpRequest();
			break;

#if defined(CONFIG_CMD_RARP)
		case RARP:
			RarpTry = 0;
			NetOurIP = 0;
			RarpRequest();
			break;
#endif
#if defined(CONFIG_CMD_PING)
		case PING:
			ping_start();
			break;
#endif
#if defined(CONFIG_CMD_NFS)
		case NFS:
			NfsStart();
			break;
#endif
#if defined(CONFIG_CMD_CDP)
		case CDP:
			CDPStart();
			break;
#endif
#ifdef CONFIG_NETCONSOLE
		case NETCONS:
			NcStart();
			break;
#endif
#if defined(CONFIG_CMD_SNTP)
		case SNTP:
			SntpStart();
			break;
#endif
#if defined(CONFIG_CMD_DNS)
		case DNS:
			DnsStart();
			break;
#endif
		default:
			break;
		}

		break;
	}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
#if	defined(CONFIG_SYS_FAULT_ECHO_LINK_DOWN)	&& \
	defined(CONFIG_STATUS_LED)			&& \
	defined(STATUS_LED_RED)
	/*
	 * Echo the inverted link state to the fault LED.
	 */
	if (miiphy_link(eth_get_dev()->name, CONFIG_SYS_FAULT_MII_ADDR))
		status_led_set(STATUS_LED_RED, STATUS_LED_OFF);
	else
		status_led_set(STATUS_LED_RED, STATUS_LED_ON);
#endif /* CONFIG_SYS_FAULT_ECHO_LINK_DOWN, ... */
#endif /* CONFIG_MII, ... */

	/*
	 *	Main packet reception loop.  Loop receiving packets until
	 *	someone sets `NetState' to a state that terminates.
	 */
	for (;;) {
		WATCHDOG_RESET();
#ifdef CONFIG_SHOW_ACTIVITY
		show_activity(1);
#endif
		/*
		 *	Check the ethernet for a new packet.  The ethernet
		 *	receive routine will process it.
		 */
		eth_rx();

		/*
		 *	Abort if ctrl-c was pressed.
		 */
		if (ctrlc()) {
			eth_halt();
			puts("\nAbort\n");
			goto done;
		}

		ArpTimeoutCheck();

		/*
		 *	Check for a timeout, and run the timeout handler
		 *	if we have one.
		 */
		if (timeHandler && ((get_timer(0) - timeStart) > timeDelta)) {
			thand_f *x;

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
#if	defined(CONFIG_SYS_FAULT_ECHO_LINK_DOWN)	&& \
	defined(CONFIG_STATUS_LED)			&& \
	defined(STATUS_LED_RED)
			/*
			 * Echo the inverted link state to the fault LED.
			 */
			if (miiphy_link(eth_get_dev()->name,
				       CONFIG_SYS_FAULT_MII_ADDR)) {
				status_led_set(STATUS_LED_RED, STATUS_LED_OFF);
			} else {
				status_led_set(STATUS_LED_RED, STATUS_LED_ON);
			}
#endif /* CONFIG_SYS_FAULT_ECHO_LINK_DOWN, ... */
#endif /* CONFIG_MII, ... */
			x = timeHandler;
			timeHandler = (thand_f *)0;
			(*x)();
		}


		switch (NetState) {

		case NETLOOP_RESTART:
			NetRestarted = 1;
			goto restart;

		case NETLOOP_SUCCESS:
			if (NetBootFileXferSize > 0) {
				char buf[20];
				printf("Bytes transferred = %ld (%lx hex)\n",
					NetBootFileXferSize,
					NetBootFileXferSize);
				sprintf(buf, "%lX", NetBootFileXferSize);
				setenv("filesize", buf);

				sprintf(buf, "%lX", (unsigned long)load_addr);
				setenv("fileaddr", buf);
			}
			eth_halt();
			ret = NetBootFileXferSize;
			goto done;

		case NETLOOP_FAIL:
			goto done;
		}
	}

done:
#ifdef CONFIG_CMD_TFTPPUT
	/* Clear out the handlers */
	NetSetHandler(NULL);
	net_set_icmp_handler(NULL);
#endif
	return ret;
}

/**********************************************************************/

static void
startAgainTimeout(void)
{
	NetState = NETLOOP_RESTART;
}

static void
startAgainHandler(uchar *pkt, unsigned dest, IPaddr_t sip,
		  unsigned src, unsigned len)
{
	/* Totally ignore the packet */
}

void NetStartAgain(void)
{
	char *nretry;
	int retry_forever = 0;
	unsigned long retrycnt = 0;

	nretry = getenv("netretry");
	if (nretry) {
		if (!strcmp(nretry, "yes"))
			retry_forever = 1;
		else if (!strcmp(nretry, "no"))
			retrycnt = 0;
		else if (!strcmp(nretry, "once"))
			retrycnt = 1;
		else
			retrycnt = simple_strtoul(nretry, NULL, 0);
	} else
		retry_forever = 1;

	if ((!retry_forever) && (NetTryCount >= retrycnt)) {
		eth_halt();
		NetState = NETLOOP_FAIL;
		return;
	}

	NetTryCount++;

	eth_halt();
#if !defined(CONFIG_NET_DO_NOT_TRY_ANOTHER)
	eth_try_another(!NetRestarted);
#endif
	eth_init(gd->bd);
	if (NetRestartWrap) {
		NetRestartWrap = 0;
		if (NetDevExists) {
			NetSetTimeout(10000UL, startAgainTimeout);
			NetSetHandler(startAgainHandler);
		} else {
			NetState = NETLOOP_FAIL;
		}
	} else {
		NetState = NETLOOP_RESTART;
	}
}

/**********************************************************************/
/*
 *	Miscelaneous bits.
 */

rxhand_f *
NetGetHandler(void)
{
	return packetHandler;
}


void
NetSetHandler(rxhand_f *f)
{
	packetHandler = f;
}

#ifdef CONFIG_CMD_TFTPPUT
void net_set_icmp_handler(rxhand_icmp_f *f)
{
	packet_icmp_handler = f;
}
#endif

void
NetSetTimeout(ulong iv, thand_f *f)
{
	if (iv == 0) {
		timeHandler = (thand_f *)0;
	} else {
		timeHandler = f;
		timeStart = get_timer(0);
		timeDelta = iv;
	}
}


void
NetSendPacket(uchar *pkt, int len)
{
	(void) eth_send(pkt, len);
}

int
NetSendUDPPacket(uchar *ether, IPaddr_t dest, int dport, int sport, int len)
{
	uchar *pkt;

	/* convert to new style broadcast */
	if (dest == 0)
		dest = 0xFFFFFFFF;

	/* if broadcast, make the ether address a broadcast and don't do ARP */
	if (dest == 0xFFFFFFFF)
		ether = NetBcastAddr;

	/*
	 * if MAC address was not discovered yet, save the packet and do
	 * an ARP request
	 */
	if (memcmp(ether, NetEtherNullAddr, 6) == 0) {

		debug("sending ARP for %08x\n", dest);

		NetArpWaitPacketIP = dest;
		NetArpWaitPacketMAC = ether;

		pkt = NetArpWaitTxPacket;
		pkt += NetSetEther(pkt, NetArpWaitPacketMAC, PROT_IP);

		NetSetIP(pkt, dest, dport, sport, len);
		memcpy(pkt + IP_UDP_HDR_SIZE, (uchar *)NetTxPacket +
		       (pkt - (uchar *)NetArpWaitTxPacket) +
		       IP_UDP_HDR_SIZE, len);

		/* size of the waiting packet */
		NetArpWaitTxPacketSize = (pkt - NetArpWaitTxPacket) +
			IP_UDP_HDR_SIZE + len;

		/* and do the ARP request */
		NetArpWaitTry = 1;
		NetArpWaitTimerStart = get_timer(0);
		ArpRequest();
		return 1;	/* waiting */
	}

	debug("sending UDP to %08x/%pM\n", dest, ether);

	pkt = (uchar *)NetTxPacket;
	pkt += NetSetEther(pkt, ether, PROT_IP);
	NetSetIP(pkt, dest, dport, sport, len);
	eth_send(NetTxPacket, (pkt - NetTxPacket) + IP_UDP_HDR_SIZE + len);

	return 0;	/* transmitted */
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
/*
 * MAXDEFRAG, above, is chosen in the config file and  is real data
 * so we need to add the NFS overhead, which is more than TFTP.
 * To use sizeof in the internal unnamed structures, we need a real
 * instance (can't do "sizeof(struct rpc_t.u.reply))", unfortunately).
 * The compiler doesn't complain nor allocates the actual structure
 */
static struct rpc_t rpc_specimen;
#define IP_PKTSIZE (CONFIG_NET_MAXDEFRAG + sizeof(rpc_specimen.u.reply))

#define IP_MAXUDP (IP_PKTSIZE - IP_HDR_SIZE_NO_UDP)

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

static struct ip_udp_hdr *__NetDefragment(struct ip_udp_hdr *ip, int *lenp)
{
	static uchar pkt_buff[IP_PKTSIZE] __aligned(PKTALIGN);
	static u16 first_hole, total_len;
	struct hole *payload, *thisfrag, *h, *newh;
	struct ip_udp_hdr *localip = (struct ip_udp_hdr *)pkt_buff;
	uchar *indata = (uchar *)ip;
	int offset8, start, len, done = 0;
	u16 ip_off = ntohs(ip->ip_off);

	/* payload starts after IP header, this fragment is in there */
	payload = (struct hole *)(pkt_buff + IP_HDR_SIZE_NO_UDP);
	offset8 =  (ip_off & IP_OFFS);
	thisfrag = payload + offset8;
	start = offset8 * 8;
	len = ntohs(ip->ip_len) - IP_HDR_SIZE_NO_UDP;

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
		memcpy(localip, ip, IP_HDR_SIZE_NO_UDP);
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
	memcpy((uchar *)thisfrag, indata + IP_HDR_SIZE_NO_UDP, len);
	if (!done)
		return NULL;

	localip->ip_len = htons(total_len);
	*lenp = total_len + IP_HDR_SIZE_NO_UDP;
	return localip;
}

static inline struct ip_udp_hdr *NetDefragment(struct ip_udp_hdr *ip, int *lenp)
{
	u16 ip_off = ntohs(ip->ip_off);
	if (!(ip_off & (IP_OFFS | IP_FLAGS_MFRAG)))
		return ip; /* not a fragment */
	return __NetDefragment(ip, lenp);
}

#else /* !CONFIG_IP_DEFRAG */

static inline struct ip_udp_hdr *NetDefragment(struct ip_udp_hdr *ip, int *lenp)
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
			IPaddr_t src_ip, Ethernet_t *et)
{
	ICMP_t *icmph = (ICMP_t *)&ip->udp_src;

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
				ntohs(ip->udp_dst), src_ip, ntohs(ip->udp_src),
				icmph->un.data, ntohs(ip->udp_len));
#endif
		break;
	}
}

void
NetReceive(uchar *inpkt, int len)
{
	Ethernet_t *et;
	struct ip_udp_hdr *ip;
	IPaddr_t tmp;
	IPaddr_t src_ip;
	int	x;
#if defined(CONFIG_CMD_CDP)
	int iscdp;
#endif
	ushort cti = 0, vlanid = VLAN_NONE, myvlanid, mynvlanid;

	debug("packet received\n");

	NetRxPacket = inpkt;
	NetRxPacketLen = len;
	et = (Ethernet_t *)inpkt;

	/* too small packet? */
	if (len < ETHER_HDR_SIZE)
		return;

#ifdef CONFIG_API
	if (push_packet) {
		(*push_packet)(inpkt, len);
		return;
	}
#endif

#if defined(CONFIG_CMD_CDP)
	/* keep track if packet is CDP */
	iscdp = is_cdp_packet(et->et_dest);
#endif

	myvlanid = ntohs(NetOurVLAN);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;
	mynvlanid = ntohs(NetOurNativeVLAN);
	if (mynvlanid == (ushort)-1)
		mynvlanid = VLAN_NONE;

	x = ntohs(et->et_protlen);

	debug("packet received\n");

	if (x < 1514) {
		/*
		 *	Got a 802 packet.  Check the other protocol field.
		 */
		x = ntohs(et->et_prot);

		ip = (struct ip_udp_hdr *)(inpkt + E802_HDR_SIZE);
		len -= E802_HDR_SIZE;

	} else if (x != PROT_VLAN) {	/* normal packet */
		ip = (struct ip_udp_hdr *)(inpkt + ETHER_HDR_SIZE);
		len -= ETHER_HDR_SIZE;

	} else {			/* VLAN packet */
		VLAN_Ethernet_t *vet = (VLAN_Ethernet_t *)et;

		debug("VLAN packet received\n");

		/* too small packet? */
		if (len < VLAN_ETHER_HDR_SIZE)
			return;

		/* if no VLAN active */
		if ((ntohs(NetOurVLAN) & VLAN_IDMASK) == VLAN_NONE
#if defined(CONFIG_CMD_CDP)
				&& iscdp == 0
#endif
				)
			return;

		cti = ntohs(vet->vet_tag);
		vlanid = cti & VLAN_IDMASK;
		x = ntohs(vet->vet_type);

		ip = (struct ip_udp_hdr *)(inpkt + VLAN_ETHER_HDR_SIZE);
		len -= VLAN_ETHER_HDR_SIZE;
	}

	debug("Receive from protocol 0x%x\n", x);

#if defined(CONFIG_CMD_CDP)
	if (iscdp) {
		CDPHandler((uchar *)ip, len);
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

	switch (x) {

	case PROT_ARP:
		ArpReceive(et, ip, len);
		break;

#ifdef CONFIG_CMD_RARP
	case PROT_RARP:
		rarp_receive(ip, len);
		break;
#endif
	case PROT_IP:
		debug("Got IP\n");
		/* Before we start poking the header, make sure it is there */
		if (len < IP_UDP_HDR_SIZE) {
			debug("len bad %d < %lu\n", len,
				(ulong)IP_UDP_HDR_SIZE);
			return;
		}
		/* Check the packet length */
		if (len < ntohs(ip->ip_len)) {
			printf("len bad %d < %d\n", len, ntohs(ip->ip_len));
			return;
		}
		len = ntohs(ip->ip_len);
		debug("len=%d, v=%02x\n", len, ip->ip_hl_v & 0xff);

		/* Can't deal with anything except IPv4 */
		if ((ip->ip_hl_v & 0xf0) != 0x40)
			return;
		/* Can't deal with IP options (headers != 20 bytes) */
		if ((ip->ip_hl_v & 0x0f) > 0x05)
			return;
		/* Check the Checksum of the header */
		if (!NetCksumOk((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2)) {
			puts("checksum bad\n");
			return;
		}
		/* If it is not for us, ignore it */
		tmp = NetReadIP(&ip->ip_dst);
		if (NetOurIP && tmp != NetOurIP && tmp != 0xFFFFFFFF) {
#ifdef CONFIG_MCAST_TFTP
			if (Mcast_addr != tmp)
#endif
				return;
		}
		/* Read source IP address for later use */
		src_ip = NetReadIP(&ip->ip_src);
		/*
		 * The function returns the unchanged packet if it's not
		 * a fragment, and either the complete packet or NULL if
		 * it is a fragment (if !CONFIG_IP_DEFRAG, it returns NULL)
		 */
		ip = NetDefragment(ip, &len);
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

#ifdef CONFIG_UDP_CHECKSUM
		if (ip->udp_xsum != 0) {
			ulong   xsum;
			ushort *sumptr;
			ushort  sumlen;

			xsum  = ip->ip_p;
			xsum += (ntohs(ip->udp_len));
			xsum += (ntohl(ip->ip_src) >> 16) & 0x0000ffff;
			xsum += (ntohl(ip->ip_src) >>  0) & 0x0000ffff;
			xsum += (ntohl(ip->ip_dst) >> 16) & 0x0000ffff;
			xsum += (ntohl(ip->ip_dst) >>  0) & 0x0000ffff;

			sumlen = ntohs(ip->udp_len);
			sumptr = (ushort *) &(ip->udp_src);

			while (sumlen > 1) {
				ushort sumdata;

				sumdata = *sumptr++;
				xsum += ntohs(sumdata);
				sumlen -= 2;
			}
			if (sumlen > 0) {
				ushort sumdata;

				sumdata = *(unsigned char *) sumptr;
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


#ifdef CONFIG_NETCONSOLE
		nc_input_packet((uchar *)ip + IP_UDP_HDR_SIZE,
					ntohs(ip->udp_dst),
					ntohs(ip->udp_src),
					ntohs(ip->udp_len) - UDP_HDR_SIZE);
#endif
		/*
		 *	IP header OK.  Pass the packet to the current handler.
		 */
		(*packetHandler)((uchar *)ip + IP_UDP_HDR_SIZE,
					ntohs(ip->udp_dst),
					src_ip,
					ntohs(ip->udp_src),
					ntohs(ip->udp_len) - UDP_HDR_SIZE);
		break;
	}
}


/**********************************************************************/

static int net_check_prereq(enum proto_t protocol)
{
	switch (protocol) {
		/* Fall through */
#if defined(CONFIG_CMD_PING)
	case PING:
		if (NetPingIP == 0) {
			puts("*** ERROR: ping address not given\n");
			return 1;
		}
		goto common;
#endif
#if defined(CONFIG_CMD_SNTP)
	case SNTP:
		if (NetNtpServerIP == 0) {
			puts("*** ERROR: NTP server address not given\n");
			return 1;
		}
		goto common;
#endif
#if defined(CONFIG_CMD_DNS)
	case DNS:
		if (NetOurDNSIP == 0) {
			puts("*** ERROR: DNS server address not given\n");
			return 1;
		}
		goto common;
#endif
#if defined(CONFIG_CMD_NFS)
	case NFS:
#endif
	case TFTPGET:
	case TFTPPUT:
		if (NetServerIP == 0) {
			puts("*** ERROR: `serverip' not set\n");
			return 1;
		}
#if	defined(CONFIG_CMD_PING) || defined(CONFIG_CMD_SNTP) || \
	defined(CONFIG_CMD_DNS)
common:
#endif
		/* Fall through */

	case NETCONS:
	case TFTPSRV:
		if (NetOurIP == 0) {
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
		if (memcmp(NetOurEther, "\0\0\0\0\0\0", 6) == 0) {
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

			NetStartAgain();
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
NetCksumOk(uchar *ptr, int len)
{
	return !((NetCksum(ptr, len) + 1) & 0xfffe);
}


unsigned
NetCksum(uchar *ptr, int len)
{
	ulong	xsum;
	ushort *p = (ushort *)ptr;

	xsum = 0;
	while (len-- > 0)
		xsum += *p++;
	xsum = (xsum & 0xffff) + (xsum >> 16);
	xsum = (xsum & 0xffff) + (xsum >> 16);
	return xsum & 0xffff;
}

int
NetEthHdrSize(void)
{
	ushort myvlanid;

	myvlanid = ntohs(NetOurVLAN);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;

	return ((myvlanid & VLAN_IDMASK) == VLAN_NONE) ? ETHER_HDR_SIZE :
		VLAN_ETHER_HDR_SIZE;
}

int
NetSetEther(uchar *xet, uchar * addr, uint prot)
{
	Ethernet_t *et = (Ethernet_t *)xet;
	ushort myvlanid;

	myvlanid = ntohs(NetOurVLAN);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;

	memcpy(et->et_dest, addr, 6);
	memcpy(et->et_src, NetOurEther, 6);
	if ((myvlanid & VLAN_IDMASK) == VLAN_NONE) {
		et->et_protlen = htons(prot);
		return ETHER_HDR_SIZE;
	} else {
		VLAN_Ethernet_t *vet = (VLAN_Ethernet_t *)xet;

		vet->vet_vlan_type = htons(PROT_VLAN);
		vet->vet_tag = htons((0 << 5) | (myvlanid & VLAN_IDMASK));
		vet->vet_type = htons(prot);
		return VLAN_ETHER_HDR_SIZE;
	}
}

void NetSetIP(uchar *xip, IPaddr_t dest, int dport, int sport, int len)
{
	struct ip_udp_hdr *ip = (struct ip_udp_hdr *)xip;

	/*
	 *	If the data is an odd number of bytes, zero the
	 *	byte after the last byte so that the checksum
	 *	will work.
	 */
	if (len & 1)
		xip[IP_UDP_HDR_SIZE + len] = 0;

	/*
	 *	Construct an IP and UDP header.
	 *	(need to set no fragment bit - XXX)
	 */
	/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_hl_v  = 0x45;
	ip->ip_tos   = 0;
	ip->ip_len   = htons(IP_UDP_HDR_SIZE + len);
	ip->ip_id    = htons(NetIPID++);
	ip->ip_off   = htons(IP_FLAGS_DFRAG);	/* Don't fragment */
	ip->ip_ttl   = 255;
	ip->ip_p     = 17;		/* UDP */
	ip->ip_sum   = 0;
	/* already in network byte order */
	NetCopyIP((void *)&ip->ip_src, &NetOurIP);
	/* - "" - */
	NetCopyIP((void *)&ip->ip_dst, &dest);
	ip->udp_src  = htons(sport);
	ip->udp_dst  = htons(dport);
	ip->udp_len  = htons(UDP_HDR_SIZE + len);
	ip->udp_xsum = 0;
	ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2);
}

void copy_filename(char *dst, const char *src, int size)
{
	if (*src && (*src == '"')) {
		++src;
		--size;
	}

	while ((--size > 0) && *src && (*src != '"'))
		*dst++ = *src++;
	*dst = '\0';
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

void ip_to_string(IPaddr_t x, char *s)
{
	x = ntohl(x);
	sprintf(s, "%d.%d.%d.%d",
		(int) ((x >> 24) & 0xff),
		(int) ((x >> 16) & 0xff),
		(int) ((x >> 8) & 0xff), (int) ((x >> 0) & 0xff)
	);
}

void VLAN_to_string(ushort x, char *s)
{
	x = ntohs(x);

	if (x == (ushort)-1)
		x = VLAN_NONE;

	if (x == VLAN_NONE)
		strcpy(s, "none");
	else
		sprintf(s, "%d", x & VLAN_IDMASK);
}

ushort string_to_VLAN(const char *s)
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

ushort getenv_VLAN(char *var)
{
	return string_to_VLAN(getenv(var));
}
