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
 *     Prerequisites:   - own ethernet address
 *     We want:         - IP, Netmask, ServerIP, Gateway IP
 *                      - bootfilename, lease time
 *     Next step:       - TFTP
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
 */


#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <net.h>
#include "bootp.h"
#include "tftp.h"
#include "rarp.h"

#if (CONFIG_COMMANDS & CFG_CMD_NET)

#define ARP_TIMEOUT		5		/* Seconds before trying ARP again */
#ifndef	CONFIG_NET_RETRY_COUNT
# define ARP_TIMEOUT_COUNT	5		/* # of timeouts before giving up  */
#else
# define ARP_TIMEOUT_COUNT  (CONFIG_NET_RETRY_COUNT)
#endif

#if 0
#define ET_DEBUG
#endif

/** BOOTP EXTENTIONS **/

IPaddr_t	NetOurSubnetMask=0;		/* Our subnet mask (0=unknown)	*/
IPaddr_t	NetOurGatewayIP=0;		/* Our gateways IP address	*/
IPaddr_t	NetOurDNSIP=0;			/* Our DNS IP address		*/
char		NetOurNISDomain[32]={0,};	/* Our NIS domain		*/
char		NetOurHostName[32]={0,};	/* Our hostname			*/
char		NetOurRootPath[64]={0,};	/* Our bootpath			*/
ushort		NetBootFileSize=0;		/* Our bootfile size in blocks	*/

/** END OF BOOTP EXTENTIONS **/

ulong		NetBootFileXferSize;	/* The actual transferred size of the bootfile (in bytes) */
uchar		NetOurEther[6];		/* Our ethernet address			*/
uchar		NetServerEther[6] =	/* Boot server enet address		*/
			{ 0, 0, 0, 0, 0, 0 };
IPaddr_t	NetOurIP;		/* Our IP addr (0 = unknown)		*/
IPaddr_t	NetServerIP;		/* Our IP addr (0 = unknown)		*/
volatile uchar *NetRxPkt;		/* Current receive packet		*/
int		NetRxPktLen;		/* Current rx packet length		*/
unsigned	NetIPID;		/* IP packet ID				*/
uchar		NetBcastAddr[6] =	/* Ethernet bcast address		*/
			{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
uchar		NetEtherNullAddr[6] =
			{ 0, 0, 0, 0, 0, 0 };
int		NetState;		/* Network loop state			*/
#ifdef CONFIG_NET_MULTI
int		NetRestartWrap = 0;	/* Tried all network devices		*/
static int	NetRestarted = 0;	/* Network loop restarted		*/
static int	NetDevExists = 0;	/* At least one device configured	*/
#endif

char		BootFile[128];		/* Boot File name			*/

#if (CONFIG_COMMANDS & CFG_CMD_PING)
IPaddr_t	NetPingIP;		/* the ip address to ping 		*/

static void PingStart(void);
#endif

volatile uchar	PktBuf[(PKTBUFSRX+1) * PKTSIZE_ALIGN + PKTALIGN];

volatile uchar *NetRxPackets[PKTBUFSRX]; /* Receive packets			*/

static rxhand_f *packetHandler;		/* Current RX packet handler		*/
static thand_f *timeHandler;		/* Current timeout handler		*/
static ulong	timeValue;		/* Current timeout value		*/
volatile uchar *NetTxPacket = 0;	/* THE transmit packet			*/

static int net_check_prereq (proto_t protocol);

/**********************************************************************/

IPaddr_t	NetArpWaitPacketIP;
IPaddr_t	NetArpWaitReplyIP;
uchar	       *NetArpWaitPacketMAC;	/* MAC address of waiting packet's destination	*/
uchar          *NetArpWaitTxPacket;	/* THE transmit packet			*/
int		NetArpWaitTxPacketSize;
uchar 		NetArpWaitPacketBuf[PKTSIZE_ALIGN + PKTALIGN];
ulong		NetArpWaitTimerStart;
int		NetArpWaitTry;

void ArpRequest(void)
{
	int i;
	volatile uchar *pkt;
	ARP_t *	arp;

#ifdef ET_DEBUG
	printf("ARP broadcast %d\n", NetArpWaitTry);
#endif
	pkt = NetTxPacket;

	NetSetEther(pkt, NetBcastAddr, PROT_ARP);
	pkt += ETHER_HDR_SIZE;

	arp = (ARP_t *)pkt;

	arp->ar_hrd = htons(ARP_ETHER);
	arp->ar_pro = htons(PROT_IP);
	arp->ar_hln = 6;
	arp->ar_pln = 4;
	arp->ar_op  = htons(ARPOP_REQUEST);

	memcpy (&arp->ar_data[0], NetOurEther, 6);	/* source ET addr	*/
	NetWriteIP((uchar*)&arp->ar_data[6], NetOurIP);	/* source IP addr	*/
	for (i=10; i<16; ++i) {
		arp->ar_data[i] = 0;			/* dest ET addr = 0	*/
	}

	if((NetArpWaitPacketIP & NetOurSubnetMask) != (NetOurIP & NetOurSubnetMask)) {
	    if (NetOurGatewayIP == 0) {
		puts ("## Warning: gatewayip needed but not set\n");
	    }
	    NetArpWaitReplyIP = NetOurGatewayIP;
	} else
            NetArpWaitReplyIP = NetArpWaitPacketIP;

        NetWriteIP((uchar*)&arp->ar_data[16], NetArpWaitReplyIP);
	(void) eth_send(NetTxPacket, ETHER_HDR_SIZE + ARP_HDR_SIZE);
}

void ArpTimeoutCheck(void)
{
	ulong t;

	if (!NetArpWaitPacketIP)
		return;

	t = get_timer(0);

	/* check for arp timeout */
	if ((t - NetArpWaitTimerStart) > ARP_TIMEOUT * CFG_HZ) {
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

/**********************************************************************/
/*
 *	Main network processing loop.
 */

int
NetLoop(proto_t protocol)
{
	DECLARE_GLOBAL_DATA_PTR;

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
	if(eth_init(bd) < 0)
	    return(-1);

restart:
#ifdef CONFIG_NET_MULTI
	memcpy (NetOurEther, eth_get_dev()->enetaddr, 6);
#else
	memcpy (NetOurEther, bd->bi_enetaddr, 6);
#endif

	NetState = NETLOOP_CONTINUE;

	/*
	 *	Start the ball rolling with the given start function.  From
	 *	here on, this code is a state machine driven by received
	 *	packets and timer events.
	 */

	switch (protocol) {
#if (CONFIG_COMMANDS & CFG_CMD_PING)
	case PING:
#endif
	case TFTP:
		NetCopyIP(&NetOurIP, &bd->bi_ip_addr);
		NetOurGatewayIP = getenv_IPaddr ("gatewayip");
		NetOurSubnetMask= getenv_IPaddr ("netmask");

		switch (protocol) {
		case TFTP:
			NetServerIP = getenv_IPaddr ("serverip");
			break;
#if (CONFIG_COMMANDS & CFG_CMD_PING)
		case PING:
			/* nothing */
			break;
#endif
		default:
			break;
		}

		break;
	case BOOTP:
	case RARP:
		/*
                 * initialize our IP addr to 0 in order to accept ANY
                 * IP addr assigned to us by the BOOTP / RARP server
		 */
		NetOurIP = 0;
		NetServerIP = 0;
		break;
	default:
		break;
	}

	switch (net_check_prereq (protocol)) {
	case 1:
		/* network not configured */
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

#if (CONFIG_COMMANDS & CFG_CMD_DHCP)
		case DHCP:
			/* Start with a clean slate... */
			NetOurIP = 0;
			NetServerIP = 0;
			DhcpRequest();		/* Basically same as BOOTP */
			break;
#endif /* CFG_CMD_DHCP */

		case BOOTP:
			BootpTry = 0;
			BootpRequest ();
			break;

		case RARP:
			RarpTry = 0;
			RarpRequest ();
			break;
#if (CONFIG_COMMANDS & CFG_CMD_PING)
		case PING:
			PingStart();
			break;
#endif
		default:
			break;
		}

		NetBootFileXferSize = 0;
		break;
	}


	/*
	 *	Main packet reception loop.  Loop receiving packets until
	 *	someone sets `NetQuit'.
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
			printf("\nAbort\n");
			return (-1);
		}

		ArpTimeoutCheck();

		/*
		 *	Check for a timeout, and run the timeout handler
		 *	if we have one.
		 */
		if (timeHandler && (get_timer(0) > timeValue)) {
			thand_f *x;

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
				char buf[10];
				printf("Bytes transferred = %ld (%lx hex)\n",
					NetBootFileXferSize,
					NetBootFileXferSize);
				sprintf(buf, "%lx", NetBootFileXferSize);
				setenv("filesize", buf);
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

void
NetStartAgain(void)
{
#ifndef CONFIG_NET_MULTI
	NetSetTimeout(10 * CFG_HZ, startAgainTimeout);
	NetSetHandler(startAgainHandler);
#else
	DECLARE_GLOBAL_DATA_PTR;

	eth_halt();
	eth_try_another(!NetRestarted);
	eth_init(gd->bd);
	if (NetRestartWrap)
	{
		NetRestartWrap = 0;
		if (NetDevExists)
		{
			NetSetTimeout(10 * CFG_HZ, startAgainTimeout);
			NetSetHandler(startAgainHandler);
		}
		else
		{
			NetState = NETLOOP_FAIL;
		}
	}
	else
	{
		NetState = NETLOOP_RESTART;
	}
#endif
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
NetSetTimeout(int iv, thand_f * f)
{
	if (iv == 0) {
		timeHandler = (thand_f *)0;
	} else {
		timeHandler = f;
		timeValue = get_timer(0) + iv;
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
	/* convert to new style broadcast */
	if (dest == 0)
		dest = 0xFFFFFFFF;

	/* if broadcast, make the ether address a broadcast and don't do ARP */
	if (dest == 0xFFFFFFFF)
		ether = NetBcastAddr;

	/* if MAC address was not discovered yet, save the packet and do an ARP request */
	if (memcmp(ether, NetEtherNullAddr, 6) == 0) {

#ifdef ET_DEBUG
		printf("sending ARP for %08lx\n", dest);
#endif

		NetArpWaitPacketIP = dest;
		NetArpWaitPacketMAC = ether;
		NetSetEther (NetArpWaitTxPacket, NetArpWaitPacketMAC, PROT_IP);
		NetSetIP (NetArpWaitTxPacket + ETHER_HDR_SIZE, dest, dport, sport, len);
		memcpy(NetArpWaitTxPacket + ETHER_HDR_SIZE + IP_HDR_SIZE,
			(uchar *)NetTxPacket + ETHER_HDR_SIZE + IP_HDR_SIZE, len);

		/* size of the waiting packet */
		NetArpWaitTxPacketSize = ETHER_HDR_SIZE + IP_HDR_SIZE + len;

		/* and do the ARP request */
		NetArpWaitTry = 1;
		NetArpWaitTimerStart = get_timer(0);
		ArpRequest();
		return 1;	/* waiting */
	}

#ifdef ET_DEBUG
	printf("sending UDP to %08lx/%02x:%02x:%02x:%02x:%02x:%02x\n",
			dest, ether[0], ether[1], ether[2], ether[3], ether[4], ether[5]);
#endif

	NetSetEther (NetTxPacket, ether, PROT_IP);
	NetSetIP (NetTxPacket + ETHER_HDR_SIZE, dest, dport, sport, len);
	(void) eth_send(NetTxPacket, ETHER_HDR_SIZE + IP_HDR_SIZE + len);

	return 0;	/* transmited */
}

#if (CONFIG_COMMANDS & CFG_CMD_PING)
static ushort PingSeqNo;

int PingSend(void)
{
	static uchar mac[6];
	volatile IP_t *ip;
	volatile ushort *s;

	/* XXX always send arp request */

	memcpy(mac, NetEtherNullAddr, 6);

#ifdef ET_DEBUG
	printf("sending ARP for %08lx\n", NetPingIP);
#endif

	NetArpWaitPacketIP = NetPingIP;
	NetArpWaitPacketMAC = mac;

	NetSetEther(NetArpWaitTxPacket, mac, PROT_IP);

	ip = (volatile IP_t *)(NetArpWaitTxPacket + ETHER_HDR_SIZE);

	/*
	 *	Construct an IP and ICMP header.  (need to set no fragment bit - XXX)
	 */
	ip->ip_hl_v  = 0x45;		/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_tos   = 0;
	ip->ip_len   = htons(IP_HDR_SIZE_NO_UDP + 8);
	ip->ip_id    = htons(NetIPID++);
	ip->ip_off   = htons(0x4000);	/* No fragmentation */
	ip->ip_ttl   = 255;
	ip->ip_p     = 0x01;		/* ICMP */
	ip->ip_sum   = 0;
	NetCopyIP((void*)&ip->ip_src, &NetOurIP); /* already in network byte order */
	NetCopyIP((void*)&ip->ip_dst, &NetPingIP);	   /* - "" - */
	ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2);

	s = &ip->udp_src;		/* XXX ICMP starts here */
	s[0] = htons(0x0800);		/* echo-request, code */
	s[1] = 0;			/* checksum */
	s[2] = 0; 			/* identifier */
	s[3] = htons(PingSeqNo++);	/* sequence number */
	s[1] = ~NetCksum((uchar *)s, 8/2);

	/* size of the waiting packet */
	NetArpWaitTxPacketSize = ETHER_HDR_SIZE + IP_HDR_SIZE_NO_UDP + 8;

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
	NetSetTimeout (10 * CFG_HZ, PingTimeout);
	NetSetHandler (PingHandler);

	PingSend();
}

#endif

void
NetReceive(volatile uchar * pkt, int len)
{
	Ethernet_t *et;
	IP_t	*ip;
	ARP_t	*arp;
	IPaddr_t tmp;
	int	x;

	NetRxPkt = pkt;
	NetRxPktLen = len;
	et = (Ethernet_t *)pkt;

	x = ntohs(et->et_protlen);

	if (x < 1514) {
		/*
		 *	Got a 802 packet.  Check the other protocol field.
		 */
		x = ntohs(et->et_prot);
		ip = (IP_t *)(pkt + E802_HDR_SIZE);
		len -= E802_HDR_SIZE;
	} else {
		ip = (IP_t *)(pkt + ETHER_HDR_SIZE);
		len -= ETHER_HDR_SIZE;
	}

#ifdef ET_DEBUG
	printf("Receive from protocol 0x%x\n", x);
#endif

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
#ifdef ET_DEBUG
		printf("Got ARP\n");
#endif
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
#ifdef ET_DEBUG
			printf("Got ARP REQUEST, return our IP\n");
#endif
			NetSetEther((uchar *)et, et->et_src, PROT_ARP);
			arp->ar_op = htons(ARPOP_REPLY);
			memcpy   (&arp->ar_data[10], &arp->ar_data[0], 6);
			NetCopyIP(&arp->ar_data[16], &arp->ar_data[6]);
			memcpy   (&arp->ar_data[ 0], NetOurEther, 6);
			NetCopyIP(&arp->ar_data[ 6], &NetOurIP);
			(void) eth_send((uchar *)et, ((uchar *)arp-pkt) + ARP_HDR_SIZE);
			return;

		case ARPOP_REPLY:		/* arp reply */
			/* are we waiting for a reply */
			if (!NetArpWaitPacketIP || !NetArpWaitPacketMAC)
				break;
#ifdef ET_DEBUG
			printf("Got ARP REPLY, set server/gtwy eth addr (%02x:%02x:%02x:%02x:%02x:%02x)\n",
				arp->ar_data[0], arp->ar_data[1],
				arp->ar_data[2], arp->ar_data[3],
				arp->ar_data[4], arp->ar_data[5]);
#endif

			tmp = NetReadIP(&arp->ar_data[6]);

			/* matched waiting packet's address */
			if (tmp == NetArpWaitReplyIP) {
#ifdef ET_DEBUG
				printf("Got it\n");
#endif
				/* save address for later use */
				memcpy(NetArpWaitPacketMAC, &arp->ar_data[0], 6);

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
#ifdef ET_DEBUG
			printf("Unexpected ARP opcode 0x%x\n", ntohs(arp->ar_op));
#endif
			return;
		}

	case PROT_RARP:
#ifdef ET_DEBUG
		printf("Got RARP\n");
#endif
		arp = (ARP_t *)ip;
		if (len < ARP_HDR_SIZE) {
			printf("bad length %d < %d\n", len, ARP_HDR_SIZE);
			return;
		}

		if ((ntohs(arp->ar_op) != RARPOP_REPLY) ||
			(ntohs(arp->ar_hrd) != ARP_ETHER)   ||
			(ntohs(arp->ar_pro) != PROT_IP)     ||
			(arp->ar_hln != 6) || (arp->ar_pln != 4)) {

			printf("invalid RARP header\n");
		} else {
			NetCopyIP(&NetOurIP,    &arp->ar_data[16]);
			NetCopyIP(&NetServerIP, &arp->ar_data[ 6]);
			memcpy (NetServerEther, &arp->ar_data[ 0], 6);

			(*packetHandler)(0,0,0,0);
		}
		break;

	case PROT_IP:
#ifdef ET_DEBUG
		printf("Got IP\n");
#endif
		if (len < IP_HDR_SIZE) {
			debug ("len bad %d < %d\n", len, IP_HDR_SIZE);
			return;
		}
		if (len < ntohs(ip->ip_len)) {
			printf("len bad %d < %d\n", len, ntohs(ip->ip_len));
			return;
		}
		len = ntohs(ip->ip_len);
#ifdef ET_DEBUG
		printf("len=%d, v=%02x\n", len, ip->ip_hl_v & 0xff);
#endif
		if ((ip->ip_hl_v & 0xf0) != 0x40) {
			return;
		}
		if (ip->ip_off & htons(0x1fff)) { /* Can't deal w/ fragments */
			return;
		}
		if (!NetCksumOk((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2)) {
			printf("checksum bad\n");
			return;
		}
		tmp = NetReadIP(&ip->ip_dst);
		if (NetOurIP && tmp != NetOurIP && tmp != 0xFFFFFFFF) {
			return;
		}
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
			puts (" ICMP Host Redirect to ");
			print_IPaddr(icmph->un.gateway);
			putc(' ');
				break;
#if (CONFIG_COMMANDS & CFG_CMD_PING)
			case ICMP_ECHO_REPLY:
				/*
				 *	IP header OK.  Pass the packet to the current handler.
				 */
				/* XXX point to ip packet */
				(*packetHandler)((uchar *)ip, 0, 0, 0);
				break;
#endif
			default:
				return;
			}
		} else if (ip->ip_p != IPPROTO_UDP) {	/* Only UDP packets */
			return;
		}

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
#if (CONFIG_COMMANDS & CFG_CMD_PING)
	case PING:
			if (NetPingIP == 0) {
				puts ("*** ERROR: ping address not given\n");
				return (1);
			}
			goto common;
#endif
	case TFTP:
			if (NetServerIP == 0) {
				puts ("*** ERROR: `serverip' not set\n");
				return (1);
			}

#if (CONFIG_COMMANDS & CFG_CMD_PING)
		common:
#endif

			if (NetOurIP == 0) {
				puts ("*** ERROR: `ipaddr' not set\n");
				return (1);
			}
			/* Fall through */

	case DHCP:
	case RARP:
	case BOOTP:
			if (memcmp(NetOurEther, "\0\0\0\0\0\0", 6) == 0) {
#ifdef CONFIG_NET_MULTI
			    extern int eth_get_dev_index (void);
			    int num = eth_get_dev_index();

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
			return(0);
	}
	return (0);	/* OK */
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

	xsum = 0;
	while (len-- > 0)
		xsum += *((ushort *)ptr)++;
	xsum = (xsum & 0xffff) + (xsum >> 16);
	xsum = (xsum & 0xffff) + (xsum >> 16);
	return (xsum & 0xffff);
}


void
NetSetEther(volatile uchar * xet, uchar * addr, uint prot)
{
	Ethernet_t *et = (Ethernet_t *)xet;

	memcpy (et->et_dest, addr, 6);
	memcpy (et->et_src, NetOurEther, 6);
	et->et_protlen = htons(prot);
}


void
NetSetIP(volatile uchar * xip, IPaddr_t dest, int dport, int sport, int len)
{
	volatile IP_t *ip = (IP_t *)xip;

	/*
	 *	If the data is an odd number of bytes, zero the
	 *	byte after the last byte so that the checksum
	 *	will work.
	 */
	if (len & 1)
		xip[IP_HDR_SIZE + len] = 0;

	/*
	 *	Construct an IP and UDP header.
			(need to set no fragment bit - XXX)
	 */
	ip->ip_hl_v  = 0x45;		/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_tos   = 0;
	ip->ip_len   = htons(IP_HDR_SIZE + len);
	ip->ip_id    = htons(NetIPID++);
	ip->ip_off   = htons(0x4000);	/* No fragmentation */
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

void copy_filename (uchar *dst, uchar *src, int size)
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

#endif /* CFG_CMD_NET */

void ip_to_string (IPaddr_t x, char *s)
{
    x = ntohl(x);
    sprintf (s,"%d.%d.%d.%d",
    	(int)((x >> 24) & 0xff),
	(int)((x >> 16) & 0xff),
	(int)((x >>  8) & 0xff),
	(int)((x >>  0) & 0xff)
    );
}

IPaddr_t string_to_ip(char *s)
{
	IPaddr_t addr;
	char *e;
	int i;

	if (s == NULL)
		return(0);

	for (addr=0, i=0; i<4; ++i) {
		ulong val = s ? simple_strtoul(s, &e, 10) : 0;
		addr <<= 8;
		addr |= (val & 0xFF);
		if (s) {
			s = (*e) ? e+1 : e;
		}
	}

	return (htonl(addr));
}

void print_IPaddr (IPaddr_t x)
{
    char tmp[16];

    ip_to_string(x, tmp);

    puts(tmp);
}

IPaddr_t getenv_IPaddr (char *var)
{
	return (string_to_ip(getenv(var)));
}
