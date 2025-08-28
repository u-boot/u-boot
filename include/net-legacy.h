/* SPDX-License-Identifier: GPL-2.0-only */
/*
 *	LiMon Monitor (LiMon) - Network.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *
 * History
 *	9/16/00	  bor  adapted to TQM823L/STK8xxL board, RARP/TFTP boot added
 */

#ifndef __NET_LEGACY_H__
#define __NET_LEGACY_H__

#include <linux/types.h>
#include <asm/byteorder.h>	/* for nton* / ntoh* stuff */
#include <log.h>
#include <time.h>
#include <linux/if_ether.h>
#include <linux/string.h>

struct bd_info;
struct cmd_tbl;
struct udevice;

#define DEBUG_LL_STATE 0	/* Link local state machine changes */
#define DEBUG_DEV_PKT 0		/* Packets or info directed to the device */
#define DEBUG_NET_PKT 0		/* Packets on info on the network at large */
#define DEBUG_INT_STATE 0	/* Internal network state changes */

/* ARP hardware address length */
#define ARP_HLEN 6
/*
 * The size of a MAC address in string form, each digit requires two chars
 * and five separator characters to form '00:00:00:00:00:00'.
 */
#define ARP_HLEN_ASCII (ARP_HLEN * 2) + (ARP_HLEN - 1)

/**
 * An incoming packet handler.
 * @param pkt    pointer to the application packet
 * @param dport  destination UDP port
 * @param sip    source IP address
 * @param sport  source UDP port
 * @param len    packet length
 */
typedef void rxhand_f(uchar *pkt, unsigned int dport,
		      struct in_addr sip, unsigned int sport,
		      unsigned int len);

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
typedef void rxhand_icmp_f(unsigned type, unsigned int code, unsigned int dport,
			   struct in_addr sip, unsigned int sport, uchar *pkt,
			   unsigned int len);

/*
 *	A timeout handler.  Called after time interval has expired.
 */
typedef void	thand_f(void);

/*
 * The devname can be either an exact name given by the driver or device tree
 * or it can be an alias of the form "eth%d"
 */
struct udevice *eth_get_dev_by_name(const char *devname);
int eth_init_state_only(void); /* Set active state */
void eth_halt_state_only(void); /* Set passive state */

/**
 * eth_env_set_enetaddr_by_index() - set the MAC address environment variable
 *
 * This sets up an environment variable with the given MAC address (@enetaddr).
 * The environment variable to be set is defined by <@base_name><@index>addr.
 * If @index is 0 it is omitted. For common Ethernet this means ethaddr,
 * eth1addr, etc.
 *
 * @base_name:  Base name for variable, typically "eth"
 * @index:      Index of interface being updated (>=0)
 * @enetaddr:   Pointer to MAC address to put into the variable
 * Return: 0 if OK, other value on error
 */
int eth_env_set_enetaddr_by_index(const char *base_name, int index,
				  uchar *enetaddr);

/*
 * Get the hardware address for an ethernet interface .
 * Args:
 *	base_name - base name for device (normally "eth")
 *	index - device index number (0 for first)
 *	enetaddr - returns 6 byte hardware address
 * Returns:
 *	Return true if the address is valid.
 */
int eth_env_get_enetaddr_by_index(const char *base_name, int index,
				  uchar *enetaddr);

int eth_send(void *packet, int length);	   /* Send a packet */

#if defined(CONFIG_API) || defined(CONFIG_EFI_LOADER)
int eth_receive(void *packet, int length); /* Receive a packet*/
extern void (*push_packet)(void *packet, int length);
#endif
int eth_mcast_join(struct in_addr mcast_addr, int join);

/**********************************************************************/
/*
 *	Protocol headers.
 */

#define ETH_FCS_LEN	4		/* Octets in the FCS		*/

struct e802_hdr {
	u8		et_dest[ARP_HLEN];	/* Destination node	*/
	u8		et_src[ARP_HLEN];	/* Source node		*/
	u16		et_protlen;		/* Protocol or length	*/
	u8		et_dsap;		/* 802 DSAP		*/
	u8		et_ssap;		/* 802 SSAP		*/
	u8		et_ctl;			/* 802 control		*/
	u8		et_snap1;		/* SNAP			*/
	u8		et_snap2;
	u8		et_snap3;
	u16		et_prot;		/* 802 protocol		*/
} __packed;

/* 802 + SNAP + ethernet header size */
#define E802_HDR_SIZE	(sizeof(struct e802_hdr))

/*
 *	Virtual LAN Ethernet header
 */
struct vlan_ethernet_hdr {
	u8		vet_dest[ARP_HLEN];	/* Destination node	*/
	u8		vet_src[ARP_HLEN];	/* Source node		*/
	u16		vet_vlan_type;		/* PROT_VLAN		*/
	u16		vet_tag;		/* TAG of VLAN		*/
	u16		vet_type;		/* protocol type	*/
} __packed;

/* VLAN Ethernet header size */
#define VLAN_ETHER_HDR_SIZE	(sizeof(struct vlan_ethernet_hdr))

/*
 *	Internet Protocol (IP) header.
 */
struct ip_hdr {
	u8		ip_hl_v;	/* header length and version	*/
	u8		ip_tos;		/* type of service		*/
	u16		ip_len;		/* total length			*/
	u16		ip_id;		/* identification		*/
	u16		ip_off;		/* fragment offset field	*/
	u8		ip_ttl;		/* time to live			*/
	u8		ip_p;		/* protocol			*/
	u16		ip_sum;		/* checksum			*/
	struct in_addr	ip_src;		/* Source IP address		*/
	struct in_addr	ip_dst;		/* Destination IP address	*/
} __packed;

#define IP_OFFS		0x1fff /* ip offset *= 8 */
#define IP_FLAGS	0xe000 /* first 3 bits */
#define IP_FLAGS_RES	0x8000 /* reserved */
#define IP_FLAGS_DFRAG	0x4000 /* don't fragments */
#define IP_FLAGS_MFRAG	0x2000 /* more fragments */

#define IP_HDR_SIZE		(sizeof(struct ip_hdr))

#define IP_MIN_FRAG_DATAGRAM_SIZE	(IP_HDR_SIZE + 8)

/*
 *	Address Resolution Protocol (ARP) header.
 */
struct arp_hdr {
	u16		ar_hrd;		/* Format of hardware address	*/
#   define ARP_ETHER	    1		/* Ethernet  hardware address	*/
	u16		ar_pro;		/* Format of protocol address	*/
	u8		ar_hln;		/* Length of hardware address	*/
	u8		ar_pln;		/* Length of protocol address	*/
#   define ARP_PLEN	4
	u16		ar_op;		/* Operation			*/
#   define ARPOP_REQUEST    1		/* Request  to resolve  address	*/
#   define ARPOP_REPLY	    2		/* Response to previous request	*/

#   define RARPOP_REQUEST   3		/* Request  to resolve  address	*/
#   define RARPOP_REPLY	    4		/* Response to previous request */

	/*
	 * The remaining fields are variable in size, according to
	 * the sizes above, and are defined as appropriate for
	 * specific hardware/protocol combinations.
	 */
	u8		ar_data[0];
#define ar_sha		ar_data[0]
#define ar_spa		ar_data[ARP_HLEN]
#define ar_tha		ar_data[ARP_HLEN + ARP_PLEN]
#define ar_tpa		ar_data[ARP_HLEN + ARP_PLEN + ARP_HLEN]
#if 0
	u8		ar_sha[];	/* Sender hardware address	*/
	u8		ar_spa[];	/* Sender protocol address	*/
	u8		ar_tha[];	/* Target hardware address	*/
	u8		ar_tpa[];	/* Target protocol address	*/
#endif /* 0 */
} __packed;

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
	u8		type;
	u8		code;
	u16		checksum;
	union {
		struct {
			u16	id;
			u16	sequence;
		} echo;
		u32	gateway;
		struct {
			u16	unused;
			u16	mtu;
		} frag;
		u8 data[0];
	} un;
} __packed;

#define ICMP_HDR_SIZE		(sizeof(struct icmp_hdr))
#define IP_ICMP_HDR_SIZE	(IP_HDR_SIZE + ICMP_HDR_SIZE)

/*
 * Maximum packet size; used to allocate packet storage. Use
 * the maximum Ethernet frame size as specified by the Ethernet
 * standard including the 802.1Q tag (VLAN tagging).
 * maximum packet size =  1522
 * maximum packet size and multiple of 32 bytes =  1536
 */
#define PKTSIZE			1522
#ifndef CONFIG_DM_DSA
#define PKTSIZE_ALIGN		1536
#else
/* Maximum DSA tagging overhead (headroom and/or tailroom) */
#define DSA_MAX_OVR		256
#define PKTSIZE_ALIGN		(1536 + DSA_MAX_OVR)
#endif

/**********************************************************************/
/*
 *	Globals.
 *
 * Note:
 *
 * All variables of type struct in_addr are stored in NETWORK byte order
 * (big endian).
 */

/* net.c */
/** BOOTP EXTENTIONS **/
extern struct in_addr net_gateway;	/* Our gateway IP address */
extern struct in_addr net_netmask;	/* Our subnet mask (0 = unknown) */
/* Our Domain Name Server (0 = unknown) */
extern struct in_addr net_dns_server;
#if defined(CONFIG_BOOTP_DNS2)
/* Our 2nd Domain Name Server (0 = unknown) */
extern struct in_addr net_dns_server2;
#endif
extern char	net_nis_domain[32];	/* Our IS domain */
extern char	net_hostname[32];	/* Our hostname */
#ifdef CONFIG_NET
extern char	net_root_path[CONFIG_BOOTP_MAX_ROOT_PATH_LEN];	/* Our root path */
#endif
/** END OF BOOTP EXTENTIONS **/
extern u8		net_ethaddr[ARP_HLEN];		/* Our ethernet address */
extern u8		net_server_ethaddr[ARP_HLEN];	/* Boot server enet address */
extern struct in_addr	net_server_ip;	/* Server IP addr (0 = unknown) */
extern uchar		*net_tx_packet;		/* THE transmit packet */
extern uchar		*net_rx_packets[PKTBUFSRX]; /* Receive packets */
extern uchar		*net_rx_packet;		/* Current receive packet */
extern int		net_rx_packet_len;	/* Current rx packet length */
extern const u8		net_null_ethaddr[ARP_HLEN];

#define VLAN_NONE	4095			/* untagged */
#define VLAN_IDMASK	0x0fff			/* mask of valid vlan id */
extern ushort		net_our_vlan;		/* Our VLAN */
extern ushort		net_native_vlan;	/* Our Native VLAN */

extern int		net_restart_wrap;	/* Tried all network devices */

enum proto_t {
	BOOTP, RARP, ARP, TFTPGET, DHCP, DHCP6, PING, PING6, DNS, NFS, CDP,
	NETCONS, SNTP, TFTPSRV, TFTPPUT, LINKLOCAL, FASTBOOT_UDP, FASTBOOT_TCP,
	WOL, UDP, NCSI, WGET, RS
};

/* Indicates whether the file name was specified on the command line */
extern bool	net_boot_file_name_explicit;
/* The actual transferred size of the bootfile (in bytes) */
extern u32	net_boot_file_size;
/* Boot file size in blocks as reported by the DHCP server */
extern u32	net_boot_file_expected_size_in_blocks;

#if defined(CONFIG_DNS)
extern char *net_dns_resolve;		/* The host to resolve  */
extern char *net_dns_env_var;		/* the env var to put the ip into */
#endif

#if defined(CONFIG_CMD_PING)
extern struct in_addr net_ping_ip;	/* the ip address to ping */
#endif

#if defined(CONFIG_CMD_CDP)
/* when CDP completes these hold the return values */
extern ushort cdp_native_vlan;		/* CDP returned native VLAN */
extern ushort cdp_appliance_vlan;	/* CDP returned appliance VLAN */

/*
 * Check for a CDP packet by examining the received MAC address field
 */
static inline int is_cdp_packet(const uchar *ethaddr)
{
	extern const u8 net_cdp_ethaddr[ARP_HLEN];

	return memcmp(ethaddr, net_cdp_ethaddr, ARP_HLEN) == 0;
}
#endif

#if defined(CONFIG_CMD_SNTP)
extern struct in_addr	net_ntp_server;		/* the ip address to NTP */
extern int net_ntp_time_offset;			/* offset time from UTC */
#endif

int net_loop(enum proto_t);

/* Get size of the ethernet header when we send */
int net_eth_hdr_size(void);

/* Set ethernet header; returns the size of the header */
int net_set_ether(uchar *xet, const uchar *dest_ethaddr, uint prot);
int net_update_ether(struct ethernet_hdr *et, uchar *addr, uint prot);

/* Set IP header */
void net_set_ip_header(uchar *pkt, struct in_addr dest, struct in_addr source,
		       u16 pkt_len, u8 proto);
void net_set_udp_header(uchar *pkt, struct in_addr dest, int dport,
			int sport, int len);

/* Callbacks */
rxhand_f *net_get_udp_handler(void);	/* Get UDP RX packet handler */
void net_set_udp_handler(rxhand_f *f);	/* Set UDP RX packet handler */
rxhand_f *net_get_arp_handler(void);	/* Get ARP RX packet handler */
void net_set_arp_handler(rxhand_f *f);	/* Set ARP RX packet handler */
bool arp_is_waiting(void);		/* Waiting for ARP reply? */
void net_set_icmp_handler(rxhand_icmp_f *f); /* Set ICMP RX handler */
void net_set_timeout_handler(ulong t, thand_f *f);/* Set timeout handler */

/* Network loop state */
enum net_loop_state {
	NETLOOP_CONTINUE,
	NETLOOP_RESTART,
	NETLOOP_SUCCESS,
	NETLOOP_FAIL
};

extern enum net_loop_state net_state;

static inline void net_set_state(enum net_loop_state state)
{
	debug_cond(DEBUG_INT_STATE, "--- NetState set to %d\n", state);
	net_state = state;
}

/*
 * net_get_async_tx_pkt_buf - Get a packet buffer that is not in use for
 *			      sending an asynchronous reply
 *
 * returns - ptr to packet buffer
 */
uchar * net_get_async_tx_pkt_buf(void);

/**
 * net_send_ip_packet() - Transmit "net_tx_packet" as UDP or TCP packet,
 *                        send ARP request if needed (ether will be populated)
 * @ether: Raw packet buffer
 * @dest: IP address to send the datagram to
 * @dport: Destination UDP port
 * @sport: Source UDP port
 * @payload_len: Length of data after the UDP header
 * @action: TCP action to be performed
 * @tcp_seq_num: TCP sequence number of this transmission
 * @tcp_ack_num: TCP stream acknolegement number
 *
 * Return: 0 on success, other value on failure
 */
int net_send_ip_packet(uchar *ether, struct in_addr dest, int dport, int sport,
		       int payload_len, int proto, u8 action, u32 tcp_seq_num,
		       u32 tcp_ack_num);
/**
 * net_send_tcp_packet() - Transmit TCP packet.
 * @payload_len: length of payload
 * @dhost: Destination host
 * @dport: Destination TCP port
 * @sport: Source TCP port
 * @action: TCP action to be performed
 * @tcp_seq_num: TCP sequence number of this transmission
 * @tcp_ack_num: TCP stream acknolegement number
 *
 * Return: 0 on success, other value on failure
 */
int net_send_tcp_packet(int payload_len, struct in_addr dhost, int dport,
			int sport, u8 action, u32 tcp_seq_num, u32 tcp_ack_num);
int net_send_udp_packet(uchar *ether, struct in_addr dest, int dport,
			int sport, int payload_len);

#if defined(CONFIG_NETCONSOLE) && !defined(CONFIG_XPL_BUILD)
void nc_start(void);
int nc_input_packet(uchar *pkt, struct in_addr src_ip, unsigned int dest_port,
		    unsigned int src_port, unsigned int len);
#endif

static __always_inline int eth_is_on_demand_init(void)
{
#if defined(CONFIG_NETCONSOLE) && !defined(CONFIG_XPL_BUILD)
	extern enum proto_t net_loop_last_protocol;

	return net_loop_last_protocol != NETCONS;
#else
	return 1;
#endif
}

static inline void eth_set_last_protocol(int protocol)
{
#if defined(CONFIG_NETCONSOLE) && !defined(CONFIG_XPL_BUILD)
	extern enum proto_t net_loop_last_protocol;

	net_loop_last_protocol = protocol;
#endif
}

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
static inline struct in_addr net_read_ip(void *from)
{
	struct in_addr ip;

	memcpy((void *)&ip, (void *)from, sizeof(ip));
	return ip;
}

/* return ulong *in network byteorder* */
static inline u32 net_read_u32(void *from)
{
	u32 l;

	memcpy((void *)&l, (void *)from, sizeof(l));
	return l;
}

/* write IP *in network byteorder* */
static inline void net_write_ip(void *to, struct in_addr ip)
{
	memcpy(to, (void *)&ip, sizeof(ip));
}

/* copy IP */
static inline void net_copy_ip(void *to, void *from)
{
	memcpy((void *)to, from, sizeof(struct in_addr));
}

/* copy ulong */
static inline void net_copy_u32(void *to, void *from)
{
	memcpy((void *)to, (void *)from, sizeof(u32));
}

/* Convert an IP address to a string */
void ip_to_string(struct in_addr x, char *s);

/**
 * string_to_ip() - Convert a string to ip address
 *
 * Implemented in lib/net_utils.c (built unconditionally)
 *
 * @s: Input string to parse
 * @return: in_addr struct containing the parsed IP address
 */
struct in_addr string_to_ip(const char *s);

/* Convert a VLAN id to a string */
void vlan_to_string(ushort x, char *s);

/* Convert a string to a vlan id */
ushort string_to_vlan(const char *s);

/* read a VLAN id from an environment variable */
ushort env_get_vlan(char *var);

/* check if serverip is specified in filename from the command line */
int is_serverip_in_cmd(void);

/**
 * net_parse_bootfile - Parse the bootfile env var / cmd line param
 *
 * @param ipaddr - a pointer to the ipaddr to populate if included in bootfile
 * @param filename - a pointer to the string to save the filename part
 * @param max_len - The longest - 1 that the filename part can be
 *
 * return 1 if parsed, 0 if bootfile is empty
 */
int net_parse_bootfile(struct in_addr *ipaddr, char *filename, int max_len);

#endif /* __NET_LEGACY_H__ */
