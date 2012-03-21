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
#include <net.h>
#include "bootp.h"
#include "tftp.h"
#ifdef CONFIG_CMD_RARP
#include "rarp.h"
#endif
#include "nfs.h"
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#include <miiphy.h>
#endif
#if defined(CONFIG_CMD_SNTP)
#include "sntp.h"
#endif
#if defined(CONFIG_CDP_VERSION)
#include <timestamp.h>
#endif
#if defined(CONFIG_CMD_DNS)
#include "dns.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifndef	CONFIG_ARP_TIMEOUT
# define ARP_TIMEOUT		5000UL	/* Milliseconds before trying ARP again */
#else
# define ARP_TIMEOUT		CONFIG_ARP_TIMEOUT
#endif


#ifndef	CONFIG_NET_RETRY_COUNT
# define ARP_TIMEOUT_COUNT	5	/* # of timeouts before giving up  */
#else
# define ARP_TIMEOUT_COUNT	CONFIG_NET_RETRY_COUNT
#endif

/** BOOTP EXTENTIONS **/

IPaddr_t	NetOurSubnetMask=0;		/* Our subnet mask (0=unknown)	*/
IPaddr_t	NetOurGatewayIP=0;		/* Our gateways IP address	*/
IPaddr_t	NetOurDNSIP=0;			/* Our DNS IP address		*/
#if defined(CONFIG_BOOTP_DNS2)
IPaddr_t	NetOurDNS2IP=0;			/* Our 2nd DNS IP address	*/
#endif
char		NetOurNISDomain[32]={0,};	/* Our NIS domain		*/
char		NetOurHostName[32]={0,};	/* Our hostname			*/
char		NetOurRootPath[64]={0,};	/* Our bootpath			*/
ushort		NetBootFileSize=0;		/* Our bootfile size in blocks	*/

#ifdef CONFIG_MCAST_TFTP	/* Multicast TFTP */
IPaddr_t Mcast_addr;
#endif

/** END OF BOOTP EXTENTIONS **/

ulong		NetBootFileXferSize;	/* The actual transferred size of the bootfile (in bytes) */
uchar		NetOurEther[6];		/* Our ethernet address			*/
uchar		NetServerEther[6] =	/* Boot server enet address		*/
			{ 0, 0, 0, 0, 0, 0 };
IPaddr_t	NetOurIP;		/* Our IP addr (0 = unknown)		*/
IPaddr_t	NetServerIP;		/* Server IP addr (0 = unknown)		*/
volatile uchar *NetRxPacket;		/* Current receive packet		*/
int		NetRxPacketLen;		/* Current rx packet length		*/
unsigned	NetIPID;		/* IP packet ID				*/
uchar		NetBcastAddr[6] =	/* Ethernet bcast address		*/
			{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
uchar		NetEtherNullAddr[6] =
			{ 0, 0, 0, 0, 0, 0 };
#ifdef CONFIG_API
void		(*push_packet)(volatile void *, int len) = 0;
#endif
#if defined(CONFIG_CMD_CDP)
uchar		NetCDPAddr[6] =		/* Ethernet bcast address		*/
			{ 0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc };
#endif
int		NetState;		/* Network loop state			*/
#ifdef CONFIG_NET_MULTI
int		NetRestartWrap = 0;	/* Tried all network devices		*/
static int	NetRestarted = 0;	/* Network loop restarted		*/
static int	NetDevExists = 0;	/* At least one device configured	*/
#endif

/* XXX in both little & big endian machines 0xFFFF == ntohs(-1) */
ushort		NetOurVLAN = 0xFFFF;		/* default is without VLAN	*/
ushort		NetOurNativeVLAN = 0xFFFF;	/* ditto			*/

char		BootFile[128];		/* Boot File name			*/

#if defined(CONFIG_CMD_PING)
IPaddr_t	NetPingIP;		/* the ip address to ping		*/

static void PingStart(void);
#endif

#if defined(CONFIG_CMD_CDP)
static void CDPStart(void);
#endif

#if defined(CONFIG_CMD_SNTP)
IPaddr_t	NetNtpServerIP;		/* NTP server IP address		*/
int		NetTimeOffset=0;	/* offset time from UTC			*/
#endif

#ifdef CONFIG_NETCONSOLE
void NcStart(void);
int nc_input_packet(uchar *pkt, unsigned dest, unsigned src, unsigned len);
#endif

volatile uchar	PktBuf[(PKTBUFSRX+1) * PKTSIZE_ALIGN + PKTALIGN];

volatile uchar *NetRxPackets[PKTBUFSRX]; /* Receive packets			*/

static rxhand_f *packetHandler;		/* Current RX packet handler		*/
static thand_f *timeHandler;		/* Current timeout handler		*/
static ulong	timeStart;		/* Time base value			*/
static ulong	timeDelta;		/* Current timeout value		*/
volatile uchar *NetTxPacket = 0;	/* THE transmit packet			*/

static int net_check_prereq (proto_t protocol);

static int NetTryCount;

/**********************************************************************/

IPaddr_t	NetArpWaitPacketIP;
IPaddr_t	NetArpWaitReplyIP;
uchar	       *NetArpWaitPacketMAC;	/* MAC address of waiting packet's destination	*/
uchar	       *NetArpWaitTxPacket;	/* THE transmit packet			*/
int		NetArpWaitTxPacketSize;
uchar		NetArpWaitPacketBuf[PKTSIZE_ALIGN + PKTALIGN];
ulong		NetArpWaitTimerStart;
int		NetArpWaitTry;

void ArpRequest (void)
{
	int i;
	volatile uchar *pkt;
	ARP_t *arp;

	debug("ARP broadcast %d\n", NetArpWaitTry);

	pkt = NetTxPacket;

	pkt += NetSetEther (pkt, NetBcastAddr, PROT_ARP);

	arp = (ARP_t *) pkt;

	arp->ar_hrd = htons (ARP_ETHER);
	arp->ar_pro = htons (PROT_IP);
	arp->ar_hln = 6;
	arp->ar_pln = 4;
	arp->ar_op = htons (ARPOP_REQUEST);

	memcpy (&arp->ar_data[0], NetOurEther, 6);		/* source ET addr	*/
	NetWriteIP ((uchar *) & arp->ar_data[6], NetOurIP);	/* source IP addr	*/
	for (i = 10; i < 16; ++i) {
		arp->ar_data[i] = 0;				/* dest ET addr = 0     */
	}

	if ((NetArpWaitPacketIP & NetOurSubnetMask) !=
	    (NetOurIP & NetOurSubnetMask)) {
		if (NetOurGatewayIP == 0) {
			puts ("## Warning: gatewayip needed but not set\n");
			NetArpWaitReplyIP = NetArpWaitPacketIP;
		} else {
			NetArpWaitReplyIP = NetOurGatewayIP;
		}
	} else {
		NetArpWaitReplyIP = NetArpWaitPacketIP;
	}

	NetWriteIP ((uchar *) & arp->ar_data[16], NetArpWaitReplyIP);
	(void) eth_send (NetTxPacket, (pkt - NetTxPacket) + ARP_HDR_SIZE);
}

void ArpTimeoutCheck(void)
{
	ulong t;

	if (!NetArpWaitPacketIP)
		return;

	t = get_timer(0);

	/* check for arp timeout */
	if ((t - NetArpWaitTimerStart) > ARP_TIMEOUT) {
		NetArpWaitTry++;

		if (NetArpWaitTry >= ARP_TIMEOUT_COUNT) {
			puts ("\nARP Retry count exceeded; starting again\n");
			NetArpWaitTry = 0;
			NetStartAgain();
		} else {
			NetArpWaitTimerStart = t;
			ArpRequest();
		}
	}
}

static void
NetInitLoop(proto_t protocol)
{
	static int env_changed_id = 0;
	bd_t *bd = gd->bd;
	int env_id = get_env_id ();

	/* update only when the environment has changed */
	if (env_changed_id != env_id) {
		NetCopyIP(&NetOurIP, &bd->bi_ip_addr);
		NetOurGatewayIP = getenv_IPaddr ("gatewayip");
		NetOurSubnetMask= getenv_IPaddr ("netmask");
		NetServerIP = getenv_IPaddr ("serverip");
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

int
NetLoop(proto_t protocol)
{
	bd_t *bd = gd->bd;

#ifdef CONFIG_NET_MULTI
	NetRestarted = 0;
	NetDevExists = 0;
#endif

	/* XXX problem with bss workaround */
	NetArpWaitPacketMAC = NULL;
	NetArpWaitTxPacket = NULL;
	NetArpWaitPacketIP = 0;
	NetArpWaitReplyIP = 0;
	NetArpWaitTxPacket = NULL;
	NetTxPacket = NULL;
	NetTryCount = 1;

	if (!NetTxPacket) {
		int	i;
		/*
		 *	Setup packet buffers, aligned correctly.
		 */
		NetTxPacket = &PktBuf[0] + (PKTALIGN - 1);
		NetTxPacket -= (ulong)NetTxPacket % PKTALIGN;
		for (i = 0; i < PKTBUFSRX; i++) {
			NetRxPackets[i] = NetTxPacket + (i+1)*PKTSIZE_ALIGN;
		}
	}

	if (!NetArpWaitTxPacket) {
		NetArpWaitTxPacket = &NetArpWaitPacketBuf[0] + (PKTALIGN - 1);
		NetArpWaitTxPacket -= (ulong)NetArpWaitTxPacket % PKTALIGN;
		NetArpWaitTxPacketSize = 0;
	}

	eth_halt();
#ifdef CONFIG_NET_MULTI
	eth_set_current();
#endif
	if (eth_init(bd) < 0) {
		eth_halt();
		return(-1);
	}

restart:
#ifdef CONFIG_NET_MULTI
	memcpy (NetOurEther, eth_get_dev()->enetaddr, 6);
#else
	eth_getenv_enetaddr("ethaddr", NetOurEther);
#endif

	NetState = NETLOOP_CONTINUE;

	/*
	 *	Start the ball rolling with the given start function.  From
	 *	here on, this code is a state machine driven by received
	 *	packets and timer events.
	 */
	NetInitLoop(protocol);

	switch (net_check_prereq (protocol)) {
	case 1:
		/* network not configured */
		eth_halt();
		return (-1);

#ifdef CONFIG_NET_MULTI
	case 2:
		/* network device not configured */
		break;
#endif /* CONFIG_NET_MULTI */

	case 0:
#ifdef CONFIG_NET_MULTI
		NetDevExists = 1;
#endif
		switch (protocol) {
		case TFTP:
			/* always use ARP to get server ethernet address */
			TftpStart();
			break;

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
			BootpRequest ();
			break;

#if defined(CONFIG_CMD_RARP)
		case RARP:
			RarpTry = 0;
			NetOurIP = 0;
			RarpRequest ();
			break;
#endif
#if defined(CONFIG_CMD_PING)
		case PING:
			PingStart();
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

		NetBootFileXferSize = 0;
		break;
	}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
#if defined(CONFIG_SYS_FAULT_ECHO_LINK_DOWN) && defined(CONFIG_STATUS_LED) && defined(STATUS_LED_RED)
	/*
	 * Echo the inverted link state to the fault LED.
	 */
	if(miiphy_link(eth_get_dev()->name, CONFIG_SYS_FAULT_MII_ADDR)) {
		status_led_set (STATUS_LED_RED, STATUS_LED_OFF);
	} else {
		status_led_set (STATUS_LED_RED, STATUS_LED_ON);
	}
#endif /* CONFIG_SYS_FAULT_ECHO_LINK_DOWN, ... */
#endif /* CONFIG_MII, ... */

	/*
	 *	Main packet reception loop.  Loop receiving packets until
	 *	someone sets `NetState' to a state that terminates.
	 */
	for (;;) {
		WATCHDOG_RESET();
#ifdef CONFIG_SHOW_ACTIVITY
		{
			extern void show_activity(int arg);
			show_activity(1);
		}
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
			puts ("\nAbort\n");
			return (-1);
		}

		ArpTimeoutCheck();

		/*
		 *	Check for a timeout, and run the timeout handler
		 *	if we have one.
		 */
		if (timeHandler && ((get_timer(0) - timeStart) > timeDelta)) {
			thand_f *x;

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
#  if defined(CONFIG_SYS_FAULT_ECHO_LINK_DOWN) && \
      defined(CONFIG_STATUS_LED) &&	   \
      defined(STATUS_LED_RED)
			/*
			 * Echo the inverted link state to the fault LED.
			 */
			if(miiphy_link(eth_get_dev()->name, CONFIG_SYS_FAULT_MII_ADDR)) {
				status_led_set (STATUS_LED_RED, STATUS_LED_OFF);
			} else {
				status_led_set (STATUS_LED_RED, STATUS_LED_ON);
			}
#  endif /* CONFIG_SYS_FAULT_ECHO_LINK_DOWN, ... */
#endif /* CONFIG_MII, ... */
			x = timeHandler;
			timeHandler = (thand_f *)0;
			(*x)();
		}


		switch (NetState) {

		case NETLOOP_RESTART:
#ifdef CONFIG_NET_MULTI
			NetRestarted = 1;
#endif
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
			return NetBootFileXferSize;

		case NETLOOP_FAIL:
			return (-1);
		}
	}
}

/**********************************************************************/

static void
startAgainTimeout(void)
{
	NetState = NETLOOP_RESTART;
}

static void
startAgainHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	/* Totally ignore the packet */
}

void NetStartAgain (void)
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

#ifndef CONFIG_NET_MULTI
	NetSetTimeout (10000UL, startAgainTimeout);
	NetSetHandler (startAgainHandler);
#else	/* !CONFIG_NET_MULTI*/
	eth_halt ();
#if !defined(CONFIG_NET_DO_NOT_TRY_ANOTHER)
	eth_try_another (!NetRestarted);
#endif
	eth_init (gd->bd);
	if (NetRestartWrap) {
		NetRestartWrap = 0;
		if (NetDevExists) {
			NetSetTimeout (10000UL, startAgainTimeout);
			NetSetHandler (startAgainHandler);
		} else {
			NetState = NETLOOP_FAIL;
		}
	} else {
		NetState = NETLOOP_RESTART;
	}
#endif	/* CONFIG_NET_MULTI */
}

/**********************************************************************/
/*
 *	Miscelaneous bits.
 */

void
NetSetHandler(rxhand_f * f)
{
	packetHandler = f;
}


void
NetSetTimeout(ulong iv, thand_f * f)
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
NetSendPacket(volatile uchar * pkt, int len)
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

	/* if MAC address was not discovered yet, save the packet and do an ARP request */
	if (memcmp(ether, NetEtherNullAddr, 6) == 0) {

		debug("sending ARP for %08lx\n", dest);

		NetArpWaitPacketIP = dest;
		NetArpWaitPacketMAC = ether;

		pkt = NetArpWaitTxPacket;
		pkt += NetSetEther (pkt, NetArpWaitPacketMAC, PROT_IP);

		NetSetIP (pkt, dest, dport, sport, len);
		memcpy(pkt + IP_HDR_SIZE, (uchar *)NetTxPacket + (pkt - (uchar *)NetArpWaitTxPacket) + IP_HDR_SIZE, len);

		/* size of the waiting packet */
		NetArpWaitTxPacketSize = (pkt - NetArpWaitTxPacket) + IP_HDR_SIZE + len;

		/* and do the ARP request */
		NetArpWaitTry = 1;
		NetArpWaitTimerStart = get_timer(0);
		ArpRequest();
		return 1;	/* waiting */
	}

	debug("sending UDP to %08lx/%pM\n", dest, ether);

	pkt = (uchar *)NetTxPacket;
	pkt += NetSetEther (pkt, ether, PROT_IP);
	NetSetIP (pkt, dest, dport, sport, len);
	(void) eth_send(NetTxPacket, (pkt - NetTxPacket) + IP_HDR_SIZE + len);

	return 0;	/* transmitted */
}

#if defined(CONFIG_CMD_PING)
static ushort PingSeqNo;

int PingSend(void)
{
	static uchar mac[6];
	volatile IP_t *ip;
	volatile ushort *s;
	uchar *pkt;

	/* XXX always send arp request */

	memcpy(mac, NetEtherNullAddr, 6);

	debug("sending ARP for %08lx\n", NetPingIP);

	NetArpWaitPacketIP = NetPingIP;
	NetArpWaitPacketMAC = mac;

	pkt = NetArpWaitTxPacket;
	pkt += NetSetEther(pkt, mac, PROT_IP);

	ip = (volatile IP_t *)pkt;

	/*
	 *	Construct an IP and ICMP header.  (need to set no fragment bit - XXX)
	 */
	ip->ip_hl_v  = 0x45;		/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_tos   = 0;
	ip->ip_len   = htons(IP_HDR_SIZE_NO_UDP + 8);
	ip->ip_id    = htons(NetIPID++);
	ip->ip_off   = htons(IP_FLAGS_DFRAG);	/* Don't fragment */
	ip->ip_ttl   = 255;
	ip->ip_p     = 0x01;		/* ICMP */
	ip->ip_sum   = 0;
	NetCopyIP((void*)&ip->ip_src, &NetOurIP); /* already in network byte order */
	NetCopyIP((void*)&ip->ip_dst, &NetPingIP);	   /* - "" - */
	ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2);

	s = &ip->udp_src;		/* XXX ICMP starts here */
	s[0] = htons(0x0800);		/* echo-request, code */
	s[1] = 0;			/* checksum */
	s[2] = 0;			/* identifier */
	s[3] = htons(PingSeqNo++);	/* sequence number */
	s[1] = ~NetCksum((uchar *)s, 8/2);

	/* size of the waiting packet */
	NetArpWaitTxPacketSize = (pkt - NetArpWaitTxPacket) + IP_HDR_SIZE_NO_UDP + 8;

	/* and do the ARP request */
	NetArpWaitTry = 1;
	NetArpWaitTimerStart = get_timer(0);
	ArpRequest();
	return 1;	/* waiting */
}

static void
PingTimeout (void)
{
	eth_halt();
	NetState = NETLOOP_FAIL;	/* we did not get the reply */
}

static void
PingHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	IPaddr_t tmp;
	volatile IP_t *ip = (volatile IP_t *)pkt;

	tmp = NetReadIP((void *)&ip->ip_src);
	if (tmp != NetPingIP)
		return;

	NetState = NETLOOP_SUCCESS;
}

static void PingStart(void)
{
#if defined(CONFIG_NET_MULTI)
	printf ("Using %s device\n", eth_get_name());
#endif	/* CONFIG_NET_MULTI */
	NetSetTimeout (10000UL, PingTimeout);
	NetSetHandler (PingHandler);

	PingSend();
}
#endif

#if defined(CONFIG_CMD_CDP)

#define CDP_DEVICE_ID_TLV		0x0001
#define CDP_ADDRESS_TLV			0x0002
#define CDP_PORT_ID_TLV			0x0003
#define CDP_CAPABILITIES_TLV		0x0004
#define CDP_VERSION_TLV			0x0005
#define CDP_PLATFORM_TLV		0x0006
#define CDP_NATIVE_VLAN_TLV		0x000a
#define CDP_APPLIANCE_VLAN_TLV		0x000e
#define CDP_TRIGGER_TLV			0x000f
#define CDP_POWER_CONSUMPTION_TLV	0x0010
#define CDP_SYSNAME_TLV			0x0014
#define CDP_SYSOBJECT_TLV		0x0015
#define CDP_MANAGEMENT_ADDRESS_TLV	0x0016

#define CDP_TIMEOUT			250UL	/* one packet every 250ms */

static int CDPSeq;
static int CDPOK;

ushort CDPNativeVLAN;
ushort CDPApplianceVLAN;

static const uchar CDP_SNAP_hdr[8] = { 0xAA, 0xAA, 0x03, 0x00, 0x00, 0x0C, 0x20, 0x00 };

static ushort CDP_compute_csum(const uchar *buff, ushort len)
{
	ushort csum;
	int     odd;
	ulong   result = 0;
	ushort  leftover;
	ushort *p;

	if (len > 0) {
		odd = 1 & (ulong)buff;
		if (odd) {
			result = *buff << 8;
			len--;
			buff++;
		}
		while (len > 1) {
			p = (ushort *)buff;
			result += *p++;
			buff = (uchar *)p;
			if (result & 0x80000000)
				result = (result & 0xFFFF) + (result >> 16);
			len -= 2;
		}
		if (len) {
			leftover = (signed short)(*(const signed char *)buff);
			/* CISCO SUCKS big time! (and blows too):
			 * CDP uses the IP checksum algorithm with a twist;
			 * for the last byte it *sign* extends and sums.
			 */
			result = (result & 0xffff0000) | ((result + leftover) & 0x0000ffff);
		}
		while (result >> 16)
			result = (result & 0xFFFF) + (result >> 16);

		if (odd)
			result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
	}

	/* add up 16-bit and 17-bit words for 17+c bits */
	result = (result & 0xffff) + (result >> 16);
	/* add up 16-bit and 2-bit for 16+c bit */
	result = (result & 0xffff) + (result >> 16);
	/* add up carry.. */
	result = (result & 0xffff) + (result >> 16);

	/* negate */
	csum = ~(ushort)result;

	/* run time endian detection */
	if (csum != htons(csum))	/* little endian */
		csum = htons(csum);

	return csum;
}

int CDPSendTrigger(void)
{
	volatile uchar *pkt;
	volatile ushort *s;
	volatile ushort *cp;
	Ethernet_t *et;
	int len;
	ushort chksum;
#if defined(CONFIG_CDP_DEVICE_ID) || defined(CONFIG_CDP_PORT_ID)   || \
    defined(CONFIG_CDP_VERSION)   || defined(CONFIG_CDP_PLATFORM)
	char buf[32];
#endif

	pkt = NetTxPacket;
	et = (Ethernet_t *)pkt;

	/* NOTE: trigger sent not on any VLAN */

	/* form ethernet header */
	memcpy(et->et_dest, NetCDPAddr, 6);
	memcpy(et->et_src, NetOurEther, 6);

	pkt += ETHER_HDR_SIZE;

	/* SNAP header */
	memcpy((uchar *)pkt, CDP_SNAP_hdr, sizeof(CDP_SNAP_hdr));
	pkt += sizeof(CDP_SNAP_hdr);

	/* CDP header */
	*pkt++ = 0x02;				/* CDP version 2 */
	*pkt++ = 180;				/* TTL */
	s = (volatile ushort *)pkt;
	cp = s;
	*s++ = htons(0);			/* checksum (0 for later calculation) */

	/* CDP fields */
#ifdef CONFIG_CDP_DEVICE_ID
	*s++ = htons(CDP_DEVICE_ID_TLV);
	*s++ = htons(CONFIG_CDP_DEVICE_ID);
	sprintf(buf, CONFIG_CDP_DEVICE_ID_PREFIX "%pm", NetOurEther);
	memcpy((uchar *)s, buf, 16);
	s += 16 / 2;
#endif

#ifdef CONFIG_CDP_PORT_ID
	*s++ = htons(CDP_PORT_ID_TLV);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, CONFIG_CDP_PORT_ID, eth_get_dev_index());
	len = strlen(buf);
	if (len & 1)	/* make it even */
		len++;
	*s++ = htons(len + 4);
	memcpy((uchar *)s, buf, len);
	s += len / 2;
#endif

#ifdef CONFIG_CDP_CAPABILITIES
	*s++ = htons(CDP_CAPABILITIES_TLV);
	*s++ = htons(8);
	*(ulong *)s = htonl(CONFIG_CDP_CAPABILITIES);
	s += 2;
#endif

#ifdef CONFIG_CDP_VERSION
	*s++ = htons(CDP_VERSION_TLV);
	memset(buf, 0, sizeof(buf));
	strcpy(buf, CONFIG_CDP_VERSION);
	len = strlen(buf);
	if (len & 1)	/* make it even */
		len++;
	*s++ = htons(len + 4);
	memcpy((uchar *)s, buf, len);
	s += len / 2;
#endif

#ifdef CONFIG_CDP_PLATFORM
	*s++ = htons(CDP_PLATFORM_TLV);
	memset(buf, 0, sizeof(buf));
	strcpy(buf, CONFIG_CDP_PLATFORM);
	len = strlen(buf);
	if (len & 1)	/* make it even */
		len++;
	*s++ = htons(len + 4);
	memcpy((uchar *)s, buf, len);
	s += len / 2;
#endif

#ifdef CONFIG_CDP_TRIGGER
	*s++ = htons(CDP_TRIGGER_TLV);
	*s++ = htons(8);
	*(ulong *)s = htonl(CONFIG_CDP_TRIGGER);
	s += 2;
#endif

#ifdef CONFIG_CDP_POWER_CONSUMPTION
	*s++ = htons(CDP_POWER_CONSUMPTION_TLV);
	*s++ = htons(6);
	*s++ = htons(CONFIG_CDP_POWER_CONSUMPTION);
#endif

	/* length of ethernet packet */
	len = (uchar *)s - ((uchar *)NetTxPacket + ETHER_HDR_SIZE);
	et->et_protlen = htons(len);

	len = ETHER_HDR_SIZE + sizeof(CDP_SNAP_hdr);
	chksum = CDP_compute_csum((uchar *)NetTxPacket + len, (uchar *)s - (NetTxPacket + len));
	if (chksum == 0)
		chksum = 0xFFFF;
	*cp = htons(chksum);

	(void) eth_send(NetTxPacket, (uchar *)s - NetTxPacket);
	return 0;
}

static void
CDPTimeout (void)
{
	CDPSeq++;

	if (CDPSeq < 3) {
		NetSetTimeout (CDP_TIMEOUT, CDPTimeout);
		CDPSendTrigger();
		return;
	}

	/* if not OK try again */
	if (!CDPOK)
		NetStartAgain();
	else
		NetState = NETLOOP_SUCCESS;
}

static void
CDPDummyHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	/* nothing */
}

static void
CDPHandler(const uchar * pkt, unsigned len)
{
	const uchar *t;
	const ushort *ss;
	ushort type, tlen;
	uchar applid;
	ushort vlan, nvlan;

	/* minimum size? */
	if (len < sizeof(CDP_SNAP_hdr) + 4)
		goto pkt_short;

	/* check for valid CDP SNAP header */
	if (memcmp(pkt, CDP_SNAP_hdr, sizeof(CDP_SNAP_hdr)) != 0)
		return;

	pkt += sizeof(CDP_SNAP_hdr);
	len -= sizeof(CDP_SNAP_hdr);

	/* Version of CDP protocol must be >= 2 and TTL != 0 */
	if (pkt[0] < 0x02 || pkt[1] == 0)
		return;

	/* if version is greater than 0x02 maybe we'll have a problem; output a warning */
	if (pkt[0] != 0x02)
		printf("** WARNING: CDP packet received with a protocol version %d > 2\n",
				pkt[0] & 0xff);

	if (CDP_compute_csum(pkt, len) != 0)
		return;

	pkt += 4;
	len -= 4;

	vlan = htons(-1);
	nvlan = htons(-1);
	while (len > 0) {
		if (len < 4)
			goto pkt_short;

		ss = (const ushort *)pkt;
		type = ntohs(ss[0]);
		tlen = ntohs(ss[1]);
		if (tlen > len) {
			goto pkt_short;
		}

		pkt += tlen;
		len -= tlen;

		ss += 2;	/* point ss to the data of the TLV */
		tlen -= 4;

		switch (type) {
			case CDP_DEVICE_ID_TLV:
				break;
			case CDP_ADDRESS_TLV:
				break;
			case CDP_PORT_ID_TLV:
				break;
			case CDP_CAPABILITIES_TLV:
				break;
			case CDP_VERSION_TLV:
				break;
			case CDP_PLATFORM_TLV:
				break;
			case CDP_NATIVE_VLAN_TLV:
				nvlan = *ss;
				break;
			case CDP_APPLIANCE_VLAN_TLV:
				t = (const uchar *)ss;
				while (tlen > 0) {
					if (tlen < 3)
						goto pkt_short;

					applid = t[0];
					ss = (const ushort *)(t + 1);

#ifdef CONFIG_CDP_APPLIANCE_VLAN_TYPE
					if (applid == CONFIG_CDP_APPLIANCE_VLAN_TYPE)
						vlan = *ss;
#else
					vlan = ntohs(*ss);	/* XXX will this work; dunno */
#endif
					t += 3; tlen -= 3;
				}
				break;
			case CDP_TRIGGER_TLV:
				break;
			case CDP_POWER_CONSUMPTION_TLV:
				break;
			case CDP_SYSNAME_TLV:
				break;
			case CDP_SYSOBJECT_TLV:
				break;
			case CDP_MANAGEMENT_ADDRESS_TLV:
				break;
		}
	}

	CDPApplianceVLAN = vlan;
	CDPNativeVLAN = nvlan;

	CDPOK = 1;
	return;

 pkt_short:
	printf("** CDP packet is too short\n");
	return;
}

static void CDPStart(void)
{
#if defined(CONFIG_NET_MULTI)
	printf ("Using %s device\n", eth_get_name());
#endif
	CDPSeq = 0;
	CDPOK = 0;

	CDPNativeVLAN = htons(-1);
	CDPApplianceVLAN = htons(-1);

	NetSetTimeout (CDP_TIMEOUT, CDPTimeout);
	NetSetHandler (CDPDummyHandler);

	CDPSendTrigger();
}
#endif

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

static IP_t *__NetDefragment(IP_t *ip, int *lenp)
{
	static uchar pkt_buff[IP_PKTSIZE] __attribute__((aligned(PKTALIGN)));
	static u16 first_hole, total_len;
	struct hole *payload, *thisfrag, *h, *newh;
	IP_t *localip = (IP_t *)pkt_buff;
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

	if ( (h >= thisfrag) && (h->last_byte <= start + len) ) {
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

static inline IP_t *NetDefragment(IP_t *ip, int *lenp)
{
	u16 ip_off = ntohs(ip->ip_off);
	if (!(ip_off & (IP_OFFS | IP_FLAGS_MFRAG)))
		return ip; /* not a fragment */
	return __NetDefragment(ip, lenp);
}

#else /* !CONFIG_IP_DEFRAG */

static inline IP_t *NetDefragment(IP_t *ip, int *lenp)
{
	u16 ip_off = ntohs(ip->ip_off);
	if (!(ip_off & (IP_OFFS | IP_FLAGS_MFRAG)))
		return ip; /* not a fragment */
	return NULL;
}
#endif

void
NetReceive(volatile uchar * inpkt, int len)
{
	Ethernet_t *et;
	IP_t	*ip;
	ARP_t	*arp;
	IPaddr_t tmp;
	int	x;
	uchar *pkt;
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
	iscdp = memcmp(et->et_dest, NetCDPAddr, 6) == 0;
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

		ip = (IP_t *)(inpkt + E802_HDR_SIZE);
		len -= E802_HDR_SIZE;

	} else if (x != PROT_VLAN) {	/* normal packet */
		ip = (IP_t *)(inpkt + ETHER_HDR_SIZE);
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

		ip = (IP_t *)(inpkt + VLAN_ETHER_HDR_SIZE);
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
		/*
		 * We have to deal with two types of ARP packets:
		 * - REQUEST packets will be answered by sending  our
		 *   IP address - if we know it.
		 * - REPLY packates are expected only after we asked
		 *   for the TFTP server's or the gateway's ethernet
		 *   address; so if we receive such a packet, we set
		 *   the server ethernet address
		 */
		debug("Got ARP\n");

		arp = (ARP_t *)ip;
		if (len < ARP_HDR_SIZE) {
			printf("bad length %d < %d\n", len, ARP_HDR_SIZE);
			return;
		}
		if (ntohs(arp->ar_hrd) != ARP_ETHER) {
			return;
		}
		if (ntohs(arp->ar_pro) != PROT_IP) {
			return;
		}
		if (arp->ar_hln != 6) {
			return;
		}
		if (arp->ar_pln != 4) {
			return;
		}

		if (NetOurIP == 0) {
			return;
		}

		if (NetReadIP(&arp->ar_data[16]) != NetOurIP) {
			return;
		}

		switch (ntohs(arp->ar_op)) {
		case ARPOP_REQUEST:		/* reply with our IP address	*/
			debug("Got ARP REQUEST, return our IP\n");
			pkt = (uchar *)et;
			pkt += NetSetEther(pkt, et->et_src, PROT_ARP);
			arp->ar_op = htons(ARPOP_REPLY);
			memcpy   (&arp->ar_data[10], &arp->ar_data[0], 6);
			NetCopyIP(&arp->ar_data[16], &arp->ar_data[6]);
			memcpy   (&arp->ar_data[ 0], NetOurEther, 6);
			NetCopyIP(&arp->ar_data[ 6], &NetOurIP);
			(void) eth_send((uchar *)et, (pkt - (uchar *)et) + ARP_HDR_SIZE);
			return;

		case ARPOP_REPLY:		/* arp reply */
			/* are we waiting for a reply */
			if (!NetArpWaitPacketIP || !NetArpWaitPacketMAC)
				break;

#ifdef CONFIG_KEEP_SERVERADDR
			if (NetServerIP == NetArpWaitPacketIP) {
				char buf[20];
				sprintf(buf, "%pM", arp->ar_data);
				setenv("serveraddr", buf);
			}
#endif

			debug("Got ARP REPLY, set server/gtwy eth addr (%pM)\n",
				arp->ar_data);

			tmp = NetReadIP(&arp->ar_data[6]);

			/* matched waiting packet's address */
			if (tmp == NetArpWaitReplyIP) {
				debug("Got it\n");
				/* save address for later use */
				memcpy(NetArpWaitPacketMAC, &arp->ar_data[0], 6);

#ifdef CONFIG_NETCONSOLE
				(*packetHandler)(0,0,0,0);
#endif
				/* modify header, and transmit it */
				memcpy(((Ethernet_t *)NetArpWaitTxPacket)->et_dest, NetArpWaitPacketMAC, 6);
				(void) eth_send(NetArpWaitTxPacket, NetArpWaitTxPacketSize);

				/* no arp request pending now */
				NetArpWaitPacketIP = 0;
				NetArpWaitTxPacketSize = 0;
				NetArpWaitPacketMAC = NULL;

			}
			return;
		default:
			debug("Unexpected ARP opcode 0x%x\n", ntohs(arp->ar_op));
			return;
		}
		break;

#ifdef CONFIG_CMD_RARP
	case PROT_RARP:
		debug("Got RARP\n");
		arp = (ARP_t *)ip;
		if (len < ARP_HDR_SIZE) {
			printf("bad length %d < %d\n", len, ARP_HDR_SIZE);
			return;
		}

		if ((ntohs(arp->ar_op) != RARPOP_REPLY) ||
			(ntohs(arp->ar_hrd) != ARP_ETHER)   ||
			(ntohs(arp->ar_pro) != PROT_IP)     ||
			(arp->ar_hln != 6) || (arp->ar_pln != 4)) {

			puts ("invalid RARP header\n");
		} else {
			NetCopyIP(&NetOurIP,    &arp->ar_data[16]);
			if (NetServerIP == 0)
				NetCopyIP(&NetServerIP, &arp->ar_data[ 6]);
			memcpy (NetServerEther, &arp->ar_data[ 0], 6);

			(*packetHandler)(0,0,0,0);
		}
		break;
#endif
	case PROT_IP:
		debug("Got IP\n");
		/* Before we start poking the header, make sure it is there */
		if (len < IP_HDR_SIZE) {
			debug("len bad %d < %lu\n", len, (ulong)IP_HDR_SIZE);
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
		if ((ip->ip_hl_v & 0xf0) != 0x40) {
			return;
		}
		/* Can't deal with IP options (headers != 20 bytes) */
		if ((ip->ip_hl_v & 0x0f) > 0x05) {
			return;
		}
		/* Check the Checksum of the header */
		if (!NetCksumOk((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2)) {
			puts ("checksum bad\n");
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
		/*
		 * The function returns the unchanged packet if it's not
		 * a fragment, and either the complete packet or NULL if
		 * it is a fragment (if !CONFIG_IP_DEFRAG, it returns NULL)
		 */
		if (!(ip = NetDefragment(ip, &len)))
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
		 */
		if (ip->ip_p == IPPROTO_ICMP) {
			ICMP_t *icmph = (ICMP_t *)&(ip->udp_src);

			switch (icmph->type) {
			case ICMP_REDIRECT:
				if (icmph->code != ICMP_REDIR_HOST)
					return;
				printf (" ICMP Host Redirect to %pI4 ", &icmph->un.gateway);
				return;
#if defined(CONFIG_CMD_PING)
			case ICMP_ECHO_REPLY:
				/*
				 *	IP header OK.  Pass the packet to the current handler.
				 */
				/* XXX point to ip packet */
				(*packetHandler)((uchar *)ip, 0, 0, 0);
				return;
			case ICMP_ECHO_REQUEST:
				debug("Got ICMP ECHO REQUEST, return %d bytes \n",
					ETHER_HDR_SIZE + len);

				memcpy (&et->et_dest[0], &et->et_src[0], 6);
				memcpy (&et->et_src[ 0], NetOurEther, 6);

				ip->ip_sum = 0;
				ip->ip_off = 0;
				NetCopyIP((void*)&ip->ip_dst, &ip->ip_src);
				NetCopyIP((void*)&ip->ip_src, &NetOurIP);
				ip->ip_sum = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP >> 1);

				icmph->type = ICMP_ECHO_REPLY;
				icmph->checksum = 0;
				icmph->checksum = ~NetCksum((uchar *)icmph,
						(len - IP_HDR_SIZE_NO_UDP) >> 1);
				(void) eth_send((uchar *)et, ETHER_HDR_SIZE + len);
				return;
#endif
			default:
				return;
			}
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
				xsum = (xsum & 0x0000ffff) + ((xsum >> 16) & 0x0000ffff);
			}
			if ((xsum != 0x00000000) && (xsum != 0x0000ffff)) {
				printf(" UDP wrong checksum %08lx %08x\n",
					xsum, ntohs(ip->udp_xsum));
				return;
			}
		}
#endif


#ifdef CONFIG_NETCONSOLE
		nc_input_packet((uchar *)ip +IP_HDR_SIZE,
						ntohs(ip->udp_dst),
						ntohs(ip->udp_src),
						ntohs(ip->udp_len) - 8);
#endif
		/*
		 *	IP header OK.  Pass the packet to the current handler.
		 */
		(*packetHandler)((uchar *)ip +IP_HDR_SIZE,
						ntohs(ip->udp_dst),
						ntohs(ip->udp_src),
						ntohs(ip->udp_len) - 8);
		break;
	}
}


/**********************************************************************/

static int net_check_prereq (proto_t protocol)
{
	switch (protocol) {
		/* Fall through */
#if defined(CONFIG_CMD_PING)
	case PING:
		if (NetPingIP == 0) {
			puts ("*** ERROR: ping address not given\n");
			return (1);
		}
		goto common;
#endif
#if defined(CONFIG_CMD_SNTP)
	case SNTP:
		if (NetNtpServerIP == 0) {
			puts ("*** ERROR: NTP server address not given\n");
			return (1);
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
	case NETCONS:
	case TFTP:
		if (NetServerIP == 0) {
			puts ("*** ERROR: `serverip' not set\n");
			return (1);
		}
#if defined(CONFIG_CMD_PING) || defined(CONFIG_CMD_SNTP)
    common:
#endif

		if (NetOurIP == 0) {
			puts ("*** ERROR: `ipaddr' not set\n");
			return (1);
		}
		/* Fall through */

#ifdef CONFIG_CMD_RARP
	case RARP:
#endif
	case BOOTP:
	case CDP:
	case DHCP:
		if (memcmp (NetOurEther, "\0\0\0\0\0\0", 6) == 0) {
#ifdef CONFIG_NET_MULTI
			extern int eth_get_dev_index (void);
			int num = eth_get_dev_index ();

			switch (num) {
			case -1:
				puts ("*** ERROR: No ethernet found.\n");
				return (1);
			case 0:
				puts ("*** ERROR: `ethaddr' not set\n");
				break;
			default:
				printf ("*** ERROR: `eth%daddr' not set\n",
					num);
				break;
			}

			NetStartAgain ();
			return (2);
#else
			puts ("*** ERROR: `ethaddr' not set\n");
			return (1);
#endif
		}
		/* Fall through */
	default:
		return (0);
	}
	return (0);		/* OK */
}
/**********************************************************************/

int
NetCksumOk(uchar * ptr, int len)
{
	return !((NetCksum(ptr, len) + 1) & 0xfffe);
}


unsigned
NetCksum(uchar * ptr, int len)
{
	ulong	xsum;
	ushort *p = (ushort *)ptr;

	xsum = 0;
	while (len-- > 0)
		xsum += *p++;
	xsum = (xsum & 0xffff) + (xsum >> 16);
	xsum = (xsum & 0xffff) + (xsum >> 16);
	return (xsum & 0xffff);
}

int
NetEthHdrSize(void)
{
	ushort myvlanid;

	myvlanid = ntohs(NetOurVLAN);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;

	return ((myvlanid & VLAN_IDMASK) == VLAN_NONE) ? ETHER_HDR_SIZE : VLAN_ETHER_HDR_SIZE;
}

int
NetSetEther(volatile uchar * xet, uchar * addr, uint prot)
{
	Ethernet_t *et = (Ethernet_t *)xet;
	ushort myvlanid;

	myvlanid = ntohs(NetOurVLAN);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;

	memcpy (et->et_dest, addr, 6);
	memcpy (et->et_src, NetOurEther, 6);
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

void
NetSetIP(volatile uchar * xip, IPaddr_t dest, int dport, int sport, int len)
{
	IP_t *ip = (IP_t *)xip;

	/*
	 *	If the data is an odd number of bytes, zero the
	 *	byte after the last byte so that the checksum
	 *	will work.
	 */
	if (len & 1)
		xip[IP_HDR_SIZE + len] = 0;

	/*
	 *	Construct an IP and UDP header.
	 *	(need to set no fragment bit - XXX)
	 */
	ip->ip_hl_v  = 0x45;		/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_tos   = 0;
	ip->ip_len   = htons(IP_HDR_SIZE + len);
	ip->ip_id    = htons(NetIPID++);
	ip->ip_off   = htons(IP_FLAGS_DFRAG);	/* Don't fragment */
	ip->ip_ttl   = 255;
	ip->ip_p     = 17;		/* UDP */
	ip->ip_sum   = 0;
	NetCopyIP((void*)&ip->ip_src, &NetOurIP); /* already in network byte order */
	NetCopyIP((void*)&ip->ip_dst, &dest);	   /* - "" - */
	ip->udp_src  = htons(sport);
	ip->udp_dst  = htons(dport);
	ip->udp_len  = htons(8 + len);
	ip->udp_xsum = 0;
	ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2);
}

void copy_filename (char *dst, const char *src, int size)
{
	if (*src && (*src == '"')) {
		++src;
		--size;
	}

	while ((--size > 0) && *src && (*src != '"')) {
		*dst++ = *src++;
	}
	*dst = '\0';
}

#if defined(CONFIG_CMD_NFS) || defined(CONFIG_CMD_SNTP) || defined(CONFIG_CMD_DNS)
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

void ip_to_string (IPaddr_t x, char *s)
{
	x = ntohl (x);
	sprintf (s, "%d.%d.%d.%d",
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
	return (string_to_VLAN(getenv(var)));
}
