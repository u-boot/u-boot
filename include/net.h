/*
 *	LiMon Monitor (LiMon) - Network.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *
 *
 * History
 *	9/16/00	  bor  adapted to TQM823L/STK8xxL board, RARP/TFTP boot added
 */

#ifndef __NET_H__
#define __NET_H__

#if defined(CONFIG_8xx)
#include <commproc.h>
#endif	/* CONFIG_8xx */

#include <asm/cache.h>
#include <asm/byteorder.h>	/* for nton* / ntoh* stuff */

#define DEBUG_LL_STATE 0	/* Link local state machine changes */
#define DEBUG_DEV_PKT 0		/* Packets or info directed to the device */
#define DEBUG_NET_PKT 0		/* Packets on info on the network at large */
#define DEBUG_INT_STATE 0	/* Internal network state changes */

/*
 *	The number of receive packet buffers, and the required packet buffer
 *	alignment in memory.
 *
 */

#ifdef CONFIG_SYS_RX_ETH_BUFFER
# define PKTBUFSRX	CONFIG_SYS_RX_ETH_BUFFER
#else
# define PKTBUFSRX	4
#endif

#define PKTALIGN	ARCH_DMA_MINALIGN

/* IPv4 addresses are always 32 bits in size */
typedef u32		IPaddr_t;


/**
 * An incoming packet handler.
 * @param pkt    pointer to the application packet
 * @param dport  destination UDP port
 * @param sip    source IP address
 * @param sport  source UDP port
 * @param len    packet length
 */
typedef void rxhand_f(uchar *pkt, unsigned dport,
		      IPaddr_t sip, unsigned sport,
		      unsigned len);

/**
 * An incoming ICMP packet handler.
 * @param type	ICMP type
 * @param code	ICMP code
 * @param dport	destination UDP port
 * @param sip	source IP address
 * @param sport	source UDP port
 * @param pkt	pointer to the ICMP packet data
 * @param len	packet length
 */
typedef void rxhand_icmp_f(unsigned type, unsigned code, unsigned dport,
		IPaddr_t sip, unsigned sport, uchar *pkt, unsigned len);

/*
 *	A timeout handler.  Called after time interval has expired.
 */
typedef void	thand_f(void);

enum eth_state_t {
	ETH_STATE_INIT,
	ETH_STATE_PASSIVE,
	ETH_STATE_ACTIVE
};

struct eth_device {
	char name[16];
	unsigned char enetaddr[6];
	int iobase;
	int state;

	int  (*init) (struct eth_device *, bd_t *);
	int  (*send) (struct eth_device *, void *packet, int length);
	int  (*recv) (struct eth_device *);
	void (*halt) (struct eth_device *);
#ifdef CONFIG_MCAST_TFTP
	int (*mcast) (struct eth_device *, u32 ip, u8 set);
#endif
	int  (*write_hwaddr) (struct eth_device *);
	struct eth_device *next;
	int index;
	void *priv;
};

extern int eth_initialize(bd_t *bis);	/* Initialize network subsystem */
extern int eth_register(struct eth_device* dev);/* Register network device */
extern int eth_unregister(struct eth_device *dev);/* Remove network device */
extern void eth_try_another(int first_restart);	/* Change the device */
extern void eth_set_current(void);		/* set nterface to ethcur var */
extern struct eth_device *eth_get_dev(void);	/* get the current device MAC */
extern struct eth_device *eth_get_dev_by_name(const char *devname);
extern struct eth_device *eth_get_dev_by_index(int index); /* get dev @ index */
extern int eth_get_dev_index(void);		/* get the device index */
extern void eth_parse_enetaddr(const char *addr, uchar *enetaddr);
extern int eth_getenv_enetaddr(char *name, uchar *enetaddr);
extern int eth_setenv_enetaddr(char *name, const uchar *enetaddr);

/*
 * Get the hardware address for an ethernet interface .
 * Args:
 *	base_name - base name for device (normally "eth")
 *	index - device index number (0 for first)
 *	enetaddr - returns 6 byte hardware address
 * Returns:
 *	Return true if the address is valid.
 */
extern int eth_getenv_enetaddr_by_index(const char *base_name, int index,
					uchar *enetaddr);

#ifdef CONFIG_RANDOM_MACADDR
/*
 * The u-boot policy does not allow hardcoded ethernet addresses. Under the
 * following circumstances a random generated address is allowed:
 *  - in emergency cases, where you need a working network connection to set
 *    the ethernet address.
 *    Eg. you want a rescue boot and don't have a serial port to access the
 *    CLI to set environment variables.
 *
 * In these cases, we generate a random locally administered ethernet address.
 *
 * Args:
 *  enetaddr - returns 6 byte hardware address
 */
extern void eth_random_enetaddr(uchar *enetaddr);
#endif

extern int usb_eth_initialize(bd_t *bi);
extern int eth_init(bd_t *bis);			/* Initialize the device */
extern int eth_send(void *packet, int length);	   /* Send a packet */

#ifdef CONFIG_API
extern int eth_receive(void *packet, int length); /* Receive a packet*/
extern void (*push_packet)(void *packet, int length);
#endif
extern int eth_rx(void);			/* Check for received packets */
extern void eth_halt(void);			/* stop SCC */
extern char *eth_get_name(void);		/* get name of current device */

/*
 * Set the hardware address for an ethernet interface based on 'eth%daddr'
 * environment variable (or just 'ethaddr' if eth_number is 0).
 * Args:
 *	base_name - base name for device (normally "eth")
 *	eth_number - value of %d (0 for first device of this type)
 * Returns:
 *	0 is success, non-zero is error status from driver.
 */
int eth_write_hwaddr(struct eth_device *dev, const char *base_name,
		     int eth_number);

#ifdef CONFIG_MCAST_TFTP
int eth_mcast_join(IPaddr_t mcast_addr, u8 join);
u32 ether_crc(size_t len, unsigned char const *p);
#endif


/**********************************************************************/
/*
 *	Protocol headers.
 */

/*
 *	Ethernet header
 */

struct ethernet_hdr {
	uchar		et_dest[6];	/* Destination node		*/
	uchar		et_src[6];	/* Source node			*/
	ushort		et_protlen;	/* Protocol or length		*/
};

/* Ethernet header size */
#define ETHER_HDR_SIZE	(sizeof(struct ethernet_hdr))

struct e802_hdr {
	uchar		et_dest[6];	/* Destination node		*/
	uchar		et_src[6];	/* Source node			*/
	ushort		et_protlen;	/* Protocol or length		*/
	uchar		et_dsap;	/* 802 DSAP			*/
	uchar		et_ssap;	/* 802 SSAP			*/
	uchar		et_ctl;		/* 802 control			*/
	uchar		et_snap1;	/* SNAP				*/
	uchar		et_snap2;
	uchar		et_snap3;
	ushort		et_prot;	/* 802 protocol			*/
};

/* 802 + SNAP + ethernet header size */
#define E802_HDR_SIZE	(sizeof(struct e802_hdr))

/*
 *	Virtual LAN Ethernet header
 */
struct vlan_ethernet_hdr {
	uchar		vet_dest[6];	/* Destination node		*/
	uchar		vet_src[6];	/* Source node			*/
	ushort		vet_vlan_type;	/* PROT_VLAN			*/
	ushort		vet_tag;	/* TAG of VLAN			*/
	ushort		vet_type;	/* protocol type		*/
};

/* VLAN Ethernet header size */
#define VLAN_ETHER_HDR_SIZE	(sizeof(struct vlan_ethernet_hdr))

#define PROT_IP		0x0800		/* IP protocol			*/
#define PROT_ARP	0x0806		/* IP ARP protocol		*/
#define PROT_RARP	0x8035		/* IP ARP protocol		*/
#define PROT_VLAN	0x8100		/* IEEE 802.1q protocol		*/

#define IPPROTO_ICMP	 1	/* Internet Control Message Protocol	*/
#define IPPROTO_UDP	17	/* User Datagram Protocol		*/

/*
 *	Internet Protocol (IP) header.
 */
struct ip_hdr {
	uchar		ip_hl_v;	/* header length and version	*/
	uchar		ip_tos;		/* type of service		*/
	ushort		ip_len;		/* total length			*/
	ushort		ip_id;		/* identification		*/
	ushort		ip_off;		/* fragment offset field	*/
	uchar		ip_ttl;		/* time to live			*/
	uchar		ip_p;		/* protocol			*/
	ushort		ip_sum;		/* checksum			*/
	IPaddr_t	ip_src;		/* Source IP address		*/
	IPaddr_t	ip_dst;		/* Destination IP address	*/
};

#define IP_OFFS		0x1fff /* ip offset *= 8 */
#define IP_FLAGS	0xe000 /* first 3 bits */
#define IP_FLAGS_RES	0x8000 /* reserved */
#define IP_FLAGS_DFRAG	0x4000 /* don't fragments */
#define IP_FLAGS_MFRAG	0x2000 /* more fragments */

#define IP_HDR_SIZE		(sizeof(struct ip_hdr))

/*
 *	Internet Protocol (IP) + UDP header.
 */
struct ip_udp_hdr {
	uchar		ip_hl_v;	/* header length and version	*/
	uchar		ip_tos;		/* type of service		*/
	ushort		ip_len;		/* total length			*/
	ushort		ip_id;		/* identification		*/
	ushort		ip_off;		/* fragment offset field	*/
	uchar		ip_ttl;		/* time to live			*/
	uchar		ip_p;		/* protocol			*/
	ushort		ip_sum;		/* checksum			*/
	IPaddr_t	ip_src;		/* Source IP address		*/
	IPaddr_t	ip_dst;		/* Destination IP address	*/
	ushort		udp_src;	/* UDP source port		*/
	ushort		udp_dst;	/* UDP destination port		*/
	ushort		udp_len;	/* Length of UDP packet		*/
	ushort		udp_xsum;	/* Checksum			*/
};

#define IP_UDP_HDR_SIZE		(sizeof(struct ip_udp_hdr))
#define UDP_HDR_SIZE		(IP_UDP_HDR_SIZE - IP_HDR_SIZE)

/*
 *	Address Resolution Protocol (ARP) header.
 */
struct arp_hdr {
	ushort		ar_hrd;		/* Format of hardware address	*/
#   define ARP_ETHER	    1		/* Ethernet  hardware address	*/
	ushort		ar_pro;		/* Format of protocol address	*/
	uchar		ar_hln;		/* Length of hardware address	*/
#   define ARP_HLEN	6
	uchar		ar_pln;		/* Length of protocol address	*/
#   define ARP_PLEN	4
	ushort		ar_op;		/* Operation			*/
#   define ARPOP_REQUEST    1		/* Request  to resolve  address	*/
#   define ARPOP_REPLY	    2		/* Response to previous request	*/

#   define RARPOP_REQUEST   3		/* Request  to resolve  address	*/
#   define RARPOP_REPLY	    4		/* Response to previous request */

	/*
	 * The remaining fields are variable in size, according to
	 * the sizes above, and are defined as appropriate for
	 * specific hardware/protocol combinations.
	 */
	uchar		ar_data[0];
#define ar_sha		ar_data[0]
#define ar_spa		ar_data[ARP_HLEN]
#define ar_tha		ar_data[ARP_HLEN + ARP_PLEN]
#define ar_tpa		ar_data[ARP_HLEN + ARP_PLEN + ARP_HLEN]
#if 0
	uchar		ar_sha[];	/* Sender hardware address	*/
	uchar		ar_spa[];	/* Sender protocol address	*/
	uchar		ar_tha[];	/* Target hardware address	*/
	uchar		ar_tpa[];	/* Target protocol address	*/
#endif /* 0 */
};

#define ARP_HDR_SIZE	(8+20)		/* Size assuming ethernet	*/

/*
 * ICMP stuff (just enough to handle (host) redirect messages)
 */
#define ICMP_ECHO_REPLY		0	/* Echo reply			*/
#define ICMP_NOT_REACH		3	/* Detination unreachable	*/
#define ICMP_REDIRECT		5	/* Redirect (change route)	*/
#define ICMP_ECHO_REQUEST	8	/* Echo request			*/

/* Codes for REDIRECT. */
#define ICMP_REDIR_NET		0	/* Redirect Net			*/
#define ICMP_REDIR_HOST		1	/* Redirect Host		*/

/* Codes for NOT_REACH */
#define ICMP_NOT_REACH_PORT	3	/* Port unreachable		*/

struct icmp_hdr {
	uchar		type;
	uchar		code;
	ushort		checksum;
	union {
		struct {
			ushort	id;
			ushort	sequence;
		} echo;
		ulong	gateway;
		struct {
			ushort	__unused;
			ushort	mtu;
		} frag;
		uchar data[0];
	} un;
};

#define ICMP_HDR_SIZE		(sizeof(struct icmp_hdr))
#define IP_ICMP_HDR_SIZE	(IP_HDR_SIZE + ICMP_HDR_SIZE)

/*
 * Maximum packet size; used to allocate packet storage.
 * TFTP packets can be 524 bytes + IP header + ethernet header.
 * Lets be conservative, and go for 38 * 16.  (Must also be
 * a multiple of 32 bytes).
 */
/*
 * AS.HARNOIS : Better to set PKTSIZE to maximum size because
 * traffic type is not always controlled
 * maximum packet size =  1518
 * maximum packet size and multiple of 32 bytes =  1536
 */
#define PKTSIZE			1518
#define PKTSIZE_ALIGN		1536
/*#define PKTSIZE		608*/

/*
 * Maximum receive ring size; that is, the number of packets
 * we can buffer before overflow happens. Basically, this just
 * needs to be enough to prevent a packet being discarded while
 * we are processing the previous one.
 */
#define RINGSZ		4
#define RINGSZ_LOG2	2

/**********************************************************************/
/*
 *	Globals.
 *
 * Note:
 *
 * All variables of type IPaddr_t are stored in NETWORK byte order
 * (big endian).
 */

/* net.c */
/** BOOTP EXTENTIONS **/
extern IPaddr_t NetOurGatewayIP;	/* Our gateway IP address */
extern IPaddr_t NetOurSubnetMask;	/* Our subnet mask (0 = unknown) */
extern IPaddr_t NetOurDNSIP;	/* Our Domain Name Server (0 = unknown) */
#if defined(CONFIG_BOOTP_DNS2)
extern IPaddr_t NetOurDNS2IP;	/* Our 2nd Domain Name Server (0 = unknown) */
#endif
extern char	NetOurNISDomain[32];	/* Our NIS domain */
extern char	NetOurHostName[32];	/* Our hostname */
extern char	NetOurRootPath[64];	/* Our root path */
extern ushort	NetBootFileSize;	/* Our boot file size in blocks */
/** END OF BOOTP EXTENTIONS **/
extern ulong		NetBootFileXferSize;	/* size of bootfile in bytes */
extern uchar		NetOurEther[6];		/* Our ethernet address */
extern uchar		NetServerEther[6];	/* Boot server enet address */
extern IPaddr_t		NetOurIP;	/* Our    IP addr (0 = unknown) */
extern IPaddr_t		NetServerIP;	/* Server IP addr (0 = unknown) */
extern uchar		*NetTxPacket;		/* THE transmit packet */
extern uchar		*NetRxPackets[PKTBUFSRX]; /* Receive packets */
extern uchar		*NetRxPacket;		/* Current receive packet */
extern int		NetRxPacketLen;		/* Current rx packet length */
extern unsigned		NetIPID;		/* IP ID (counting) */
extern uchar		NetBcastAddr[6];	/* Ethernet boardcast address */
extern uchar		NetEtherNullAddr[6];

#define VLAN_NONE	4095			/* untagged */
#define VLAN_IDMASK	0x0fff			/* mask of valid vlan id */
extern ushort		NetOurVLAN;		/* Our VLAN */
extern ushort		NetOurNativeVLAN;	/* Our Native VLAN */

extern int		NetRestartWrap;		/* Tried all network devices */

enum proto_t {
	BOOTP, RARP, ARP, TFTPGET, DHCP, PING, DNS, NFS, CDP, NETCONS, SNTP,
	TFTPSRV, TFTPPUT, LINKLOCAL
};

/* from net/net.c */
extern char	BootFile[128];			/* Boot File name */

#if defined(CONFIG_CMD_DNS)
extern char *NetDNSResolve;		/* The host to resolve  */
extern char *NetDNSenvvar;		/* the env var to put the ip into */
#endif

#if defined(CONFIG_CMD_PING)
extern IPaddr_t	NetPingIP;			/* the ip address to ping */
#endif

#if defined(CONFIG_CMD_CDP)
/* when CDP completes these hold the return values */
extern ushort CDPNativeVLAN;		/* CDP returned native VLAN */
extern ushort CDPApplianceVLAN;		/* CDP returned appliance VLAN */

/*
 * Check for a CDP packet by examining the received MAC address field
 */
static inline int is_cdp_packet(const uchar *et_addr)
{
	extern const uchar NetCDPAddr[6];

	return memcmp(et_addr, NetCDPAddr, 6) == 0;
}
#endif

#if defined(CONFIG_CMD_SNTP)
extern IPaddr_t	NetNtpServerIP;			/* the ip address to NTP */
extern int NetTimeOffset;			/* offset time from UTC */
#endif

#if defined(CONFIG_MCAST_TFTP)
extern IPaddr_t Mcast_addr;
#endif

/* Initialize the network adapter */
extern void net_init(void);
extern int NetLoop(enum proto_t);

/* Shutdown adapters and cleanup */
extern void	NetStop(void);

/* Load failed.	 Start again. */
extern void	NetStartAgain(void);

/* Get size of the ethernet header when we send */
extern int	NetEthHdrSize(void);

/* Set ethernet header; returns the size of the header */
extern int NetSetEther(uchar *, uchar *, uint);
extern int net_update_ether(struct ethernet_hdr *et, uchar *addr, uint prot);

/* Set IP header */
extern void net_set_ip_header(uchar *pkt, IPaddr_t dest, IPaddr_t source);
extern void net_set_udp_header(uchar *pkt, IPaddr_t dest, int dport,
				int sport, int len);

/* Checksum */
extern int	NetCksumOk(uchar *, int);	/* Return true if cksum OK */
extern uint	NetCksum(uchar *, int);		/* Calculate the checksum */

/* Callbacks */
extern rxhand_f *net_get_udp_handler(void);	/* Get UDP RX packet handler */
extern void net_set_udp_handler(rxhand_f *);	/* Set UDP RX packet handler */
extern rxhand_f *net_get_arp_handler(void);	/* Get ARP RX packet handler */
extern void net_set_arp_handler(rxhand_f *);	/* Set ARP RX packet handler */
extern void net_set_icmp_handler(rxhand_icmp_f *f); /* Set ICMP RX handler */
extern void	NetSetTimeout(ulong, thand_f *);/* Set timeout handler */

/* Network loop state */
enum net_loop_state {
	NETLOOP_CONTINUE,
	NETLOOP_RESTART,
	NETLOOP_SUCCESS,
	NETLOOP_FAIL
};
static inline void net_set_state(enum net_loop_state state)
{
	extern enum net_loop_state net_state;

	debug_cond(DEBUG_INT_STATE, "--- NetState set to %d\n", state);
	net_state = state;
}

/* Transmit a packet */
static inline void NetSendPacket(uchar *pkt, int len)
{
	(void) eth_send(pkt, len);
}

/*
 * Transmit "NetTxPacket" as UDP packet, performing ARP request if needed
 *  (ether will be populated)
 *
 * @param ether Raw packet buffer
 * @param dest IP address to send the datagram to
 * @param dport Destination UDP port
 * @param sport Source UDP port
 * @param payload_len Length of data after the UDP header
 */
extern int NetSendUDPPacket(uchar *ether, IPaddr_t dest, int dport,
			int sport, int payload_len);

/* Processes a received packet */
extern void NetReceive(uchar *, int);

#ifdef CONFIG_NETCONSOLE
void NcStart(void);
int nc_input_packet(uchar *pkt, unsigned dest, unsigned src, unsigned len);
#endif

/*
 * Check if autoload is enabled. If so, use either NFS or TFTP to download
 * the boot file.
 */
void net_auto_load(void);

/*
 * The following functions are a bit ugly, but necessary to deal with
 * alignment restrictions on ARM.
 *
 * We're using inline functions, which had the smallest memory
 * footprint in our tests.
 */
/* return IP *in network byteorder* */
static inline IPaddr_t NetReadIP(void *from)
{
	IPaddr_t ip;

	memcpy((void *)&ip, (void *)from, sizeof(ip));
	return ip;
}

/* return ulong *in network byteorder* */
static inline ulong NetReadLong(ulong *from)
{
	ulong l;

	memcpy((void *)&l, (void *)from, sizeof(l));
	return l;
}

/* write IP *in network byteorder* */
static inline void NetWriteIP(void *to, IPaddr_t ip)
{
	memcpy(to, (void *)&ip, sizeof(ip));
}

/* copy IP */
static inline void NetCopyIP(void *to, void *from)
{
	memcpy((void *)to, from, sizeof(IPaddr_t));
}

/* copy ulong */
static inline void NetCopyLong(ulong *to, ulong *from)
{
	memcpy((void *)to, (void *)from, sizeof(ulong));
}

/**
 * is_zero_ether_addr - Determine if give Ethernet address is all zeros.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is all zeroes.
 */
static inline int is_zero_ether_addr(const u8 *addr)
{
	return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

/**
 * is_multicast_ether_addr - Determine if the Ethernet address is a multicast.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is a multicast address.
 * By definition the broadcast address is also a multicast address.
 */
static inline int is_multicast_ether_addr(const u8 *addr)
{
	return 0x01 & addr[0];
}

/*
 * is_broadcast_ether_addr - Determine if the Ethernet address is broadcast
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is the broadcast address.
 */
static inline int is_broadcast_ether_addr(const u8 *addr)
{
	return (addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5]) ==
		0xff;
}

/*
 * is_valid_ether_addr - Determine if the given Ethernet address is valid
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Check that the Ethernet address (MAC) is not 00:00:00:00:00:00, is not
 * a multicast address, and is not FF:FF:FF:FF:FF:FF.
 *
 * Return true if the address is valid.
 */
static inline int is_valid_ether_addr(const u8 *addr)
{
	/* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
	 * explicitly check for it here. */
	return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
}

/* Convert an IP address to a string */
extern void ip_to_string(IPaddr_t x, char *s);

/* Convert a string to ip address */
extern IPaddr_t string_to_ip(const char *s);

/* Convert a VLAN id to a string */
extern void VLAN_to_string(ushort x, char *s);

/* Convert a string to a vlan id */
extern ushort string_to_VLAN(const char *s);

/* read a VLAN id from an environment variable */
extern ushort getenv_VLAN(char *);

/* copy a filename (allow for "..." notation, limit length) */
extern void copy_filename(char *dst, const char *src, int size);

/* get a random source port */
extern unsigned int random_port(void);

/**********************************************************************/

#endif /* __NET_H__ */
