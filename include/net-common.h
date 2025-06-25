/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __NET_COMMON_H__
#define __NET_COMMON_H__

#include <asm/cache.h>
#include <command.h>
#include <hexdump.h>
#include <linux/if_ether.h>
#include <linux/sizes.h>
#include <linux/types.h>
#include <rand.h>
#include <time.h>

#define DEBUG_NET_PKT_TRACE 0	/* Trace all packet data */

/*
 *	The number of receive packet buffers, and the required packet buffer
 *	alignment in memory.
 *
 */
#define PKTBUFSRX	CONFIG_SYS_RX_ETH_BUFFER
#define PKTALIGN	ARCH_DMA_MINALIGN

/* IPv4 addresses are always 32 bits in size */
struct in_addr {
	__be32 s_addr;
};

#define PROT_IP		0x0800		/* IP protocol			*/
#define PROT_ARP	0x0806		/* IP ARP protocol		*/
#define PROT_WOL	0x0842		/* ether-wake WoL protocol	*/
#define PROT_RARP	0x8035		/* IP ARP protocol		*/
#define PROT_VLAN	0x8100		/* IEEE 802.1q protocol		*/
#define PROT_IPV6	0x86dd		/* IPv6 over bluebook		*/
#define PROT_PPP_SES	0x8864		/* PPPoE session messages	*/
#define PROT_NCSI	0x88f8		/* NC-SI control packets        */

#define IPPROTO_ICMP	 1	/* Internet Control Message Protocol	*/
#define IPPROTO_TCP	6	/* Transmission Control Protocol	*/
#define IPPROTO_UDP	17	/* User Datagram Protocol		*/

#define IP_OFFS		0x1fff /* ip offset *= 8 */
#define IP_FLAGS	0xe000 /* first 3 bits */
#define IP_FLAGS_RES	0x8000 /* reserved */
#define IP_FLAGS_DFRAG	0x4000 /* don't fragments */
#define IP_FLAGS_MFRAG	0x2000 /* more fragments */

#define IP_HDR_SIZE		(sizeof(struct ip_hdr))

#define IP_MIN_FRAG_DATAGRAM_SIZE	(IP_HDR_SIZE + 8)

/*
 *	Internet Protocol (IP) + UDP header.
 */
struct ip_udp_hdr {
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
	u16		udp_src;	/* UDP source port		*/
	u16		udp_dst;	/* UDP destination port		*/
	u16		udp_len;	/* Length of UDP packet		*/
	u16		udp_xsum;	/* Checksum			*/
} __packed;

#define IP_UDP_HDR_SIZE		(sizeof(struct ip_udp_hdr))
#define UDP_HDR_SIZE		(IP_UDP_HDR_SIZE - IP_HDR_SIZE)

/* Number of packets processed together */
#define ETH_PACKETS_BATCH_RECV	32

/* ARP hardware address length */
#define ARP_HLEN 6
/*
 * The size of a MAC address in string form, each digit requires two chars
 * and five separator characters to form '00:00:00:00:00:00'.
 */
#define ARP_HLEN_ASCII (ARP_HLEN * 2) + (ARP_HLEN - 1)

#define ARP_HDR_SIZE	(8 + 20)	/* Size assuming ethernet	*/

#   define ARP_ETHER	    1		/* Ethernet  hardware address	*/

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

/*
 * Maximum receive ring size; that is, the number of packets
 * we can buffer before overflow happens. Basically, this just
 * needs to be enough to prevent a packet being discarded while
 * we are processing the previous one.
 * Used only in drivers/net/mvgbe.c.
 */
#define RINGSZ		4
#define RINGSZ_LOG2	2

extern int		net_restart_wrap;	/* Tried all network devices */
extern uchar               *net_rx_packets[PKTBUFSRX]; /* Receive packets */
extern const u8		net_bcast_ethaddr[ARP_HLEN];	/* Ethernet broadcast address */
extern char	net_boot_file_name[1024];/* Boot File name */
extern struct in_addr	net_ip;		/* Our    IP addr (0 = unknown) */
/* Indicates whether the pxe path prefix / config file was specified in dhcp option */
extern char *pxelinux_configfile;

/**
 * compute_ip_checksum() - Compute IP checksum
 *
 * @addr:	Address to check (must be 16-bit aligned)
 * @nbytes:	Number of bytes to check (normally a multiple of 2)
 * Return: 16-bit IP checksum
 */
unsigned compute_ip_checksum(const void *addr, unsigned int nbytes);

/**
 * ip_checksum_ok() - check if a checksum is correct
 *
 * This works by making sure the checksum sums to 0
 *
 * @addr:	Address to check (must be 16-bit aligned)
 * @nbytes:	Number of bytes to check (normally a multiple of 2)
 * Return: true if the checksum matches, false if not
 */
int ip_checksum_ok(const void *addr, unsigned int nbytes);

/**
 * add_ip_checksums() - add two IP checksums
 *
 * @offset:	Offset of first sum (if odd we do a byte-swap)
 * @sum:	First checksum
 * @new_sum:	New checksum to add
 * Return: updated 16-bit IP checksum
 */
unsigned add_ip_checksums(unsigned offset, unsigned sum, unsigned int new_sum);

/*
 * The devname can be either an exact name given by the driver or device tree
 * or it can be an alias of the form "eth%d"
 */
struct udevice *eth_get_dev_by_name(const char *devname);
int eth_is_active(struct udevice *dev); /* Test device for active state */

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
 * Initialize USB ethernet device with CONFIG_DM_ETH
 * Returns:
 *	0 is success, non-zero is error status.
 */
int usb_ether_init(void);

int eth_init(void);			/* Initialize the device */
int eth_start_udev(struct udevice *dev); /* ->start() if not already running */
int eth_send(void *packet, int length);	   /* Send a packet */
#if defined(CONFIG_API) || defined(CONFIG_EFI_LOADER)
int eth_receive(void *packet, int length); /* Receive a packet*/
extern void (*push_packet)(void *packet, int length);
#endif
int eth_rx(void);			/* Check for received packets */

/**
 * reset_phy() - Reset the Ethernet PHY
 *
 * This should be implemented by boards if CONFIG_RESET_PHY_R is enabled
 */
void reset_phy(void);

#if CONFIG_IS_ENABLED(NET) || CONFIG_IS_ENABLED(NET_LWIP)
/**
 * eth_set_enable_bootdevs() - Enable or disable binding of Ethernet bootdevs
 *
 * These get in the way of bootstd testing, so are normally disabled by tests.
 * This provide control of this setting. It only affects binding of Ethernet
 * devices, so if that has already happened, this flag does nothing.
 *
 * @enable: true to enable binding of bootdevs when binding new Ethernet
 * devices, false to disable it
 */
void eth_set_enable_bootdevs(bool enable);
#else
static inline void eth_set_enable_bootdevs(bool enable) {}
#endif

static inline void net_send_packet(uchar *pkt, int len)
{
	if (DEBUG_NET_PKT_TRACE)
		print_hex_dump_bytes("tx: ", DUMP_PREFIX_OFFSET, pkt, len);
	/* Currently no way to return errors from eth_send() */
	(void)eth_send(pkt, len);
}

enum eth_recv_flags {
	/*
	 * Check hardware device for new packets (otherwise only return those
	 * which are already in the memory buffer ready to process)
	 */
	ETH_RECV_CHECK_DEVICE		= 1 << 0,
};

/**
 * struct eth_ops - functions of Ethernet MAC controllers
 *
 * start: Prepare the hardware to send and receive packets
 * send: Send the bytes passed in "packet" as a packet on the wire
 * recv: Check if the hardware received a packet. If so, set the pointer to the
 *	 packet buffer in the packetp parameter. If not, return an error or 0 to
 *	 indicate that the hardware receive FIFO is empty. If 0 is returned, the
 *	 network stack will not process the empty packet, but free_pkt() will be
 *	 called if supplied
 * free_pkt: Give the driver an opportunity to manage its packet buffer memory
 *	     when the network stack is finished processing it. This will only be
 *	     called when no error was returned from recv - optional
 * stop: Stop the hardware from looking for packets - may be called even if
 *	 state == PASSIVE
 * mcast: Join or leave a multicast group (for TFTP) - optional
 * write_hwaddr: Write a MAC address to the hardware (used to pass it to Linux
 *		 on some platforms like ARM). This function expects the
 *		 eth_pdata::enetaddr field to be populated. The method can
 *		 return -ENOSYS to indicate that this is not implemented for
		 this hardware - optional.
 * read_rom_hwaddr: Some devices have a backup of the MAC address stored in a
 *		    ROM on the board. This is how the driver should expose it
 *		    to the network stack. This function should fill in the
 *		    eth_pdata::enetaddr field - optional
 * set_promisc: Enable or Disable promiscuous mode
 * get_sset_count: Number of statistics counters
 * get_string: Names of the statistic counters
 * get_stats: The values of the statistic counters
 */
struct eth_ops {
	int (*start)(struct udevice *dev);
	int (*send)(struct udevice *dev, void *packet, int length);
	int (*recv)(struct udevice *dev, int flags, uchar **packetp);
	int (*free_pkt)(struct udevice *dev, uchar *packet, int length);
	void (*stop)(struct udevice *dev);
	int (*mcast)(struct udevice *dev, const u8 *enetaddr, int join);
	int (*write_hwaddr)(struct udevice *dev);
	int (*read_rom_hwaddr)(struct udevice *dev);
	int (*set_promisc)(struct udevice *dev, bool enable);
	int (*get_sset_count)(struct udevice *dev);
	void (*get_strings)(struct udevice *dev, u8 *data);
	void (*get_stats)(struct udevice *dev, u64 *data);
};

#define eth_get_ops(dev) ((struct eth_ops *)(dev)->driver->ops)

struct udevice *eth_get_dev(void); /* get the current device */
void eth_set_dev(struct udevice *dev); /* set a device */
unsigned char *eth_get_ethaddr(void); /* get the current device MAC */
int eth_rx(void);                      /* Check for received packets */
void eth_halt(void);			/* stop SCC */
const char *eth_get_name(void);		/* get name of current device */
int eth_get_dev_index(void);

int eth_initialize(void);		/* Initialize network subsystem */
void eth_try_another(int first_restart);	/* Change the device */
void eth_set_current(void);		/* set nterface to ethcur var */

enum eth_state_t {
	ETH_STATE_INIT,
	ETH_STATE_PASSIVE,
	ETH_STATE_ACTIVE
};

/**
 * struct eth_pdata - Platform data for Ethernet MAC controllers
 *
 * @iobase: The base address of the hardware registers
 * @enetaddr: The Ethernet MAC address that is loaded from EEPROM or env
 * @phy_interface: PHY interface to use - see PHY_INTERFACE_MODE_...
 * @max_speed: Maximum speed of Ethernet connection supported by MAC
 * @priv_pdata: device specific plat
 */
struct eth_pdata {
	phys_addr_t iobase;
	unsigned char enetaddr[ARP_HLEN];
	int phy_interface;
	int max_speed;
	void *priv_pdata;
};

struct ethernet_hdr {
	u8		et_dest[ARP_HLEN];	/* Destination node	*/
	u8		et_src[ARP_HLEN];	/* Source node		*/
	u16		et_protlen;		/* Protocol or length	*/
} __packed;

/* Ethernet header size */
#define ETHER_HDR_SIZE	(sizeof(struct ethernet_hdr))

/**
 * net_random_ethaddr - Generate software assigned random Ethernet address
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Generate a random Ethernet address (MAC) that is not multicast
 * and has the local assigned bit set.
 */
static inline void net_random_ethaddr(uchar *addr)
{
	int i;
	unsigned int seed = get_ticks();

	for (i = 0; i < 6; i++)
		addr[i] = rand_r(&seed);

	addr[0] &= 0xfe;	/* clear multicast bit */
	addr[0] |= 0x02;	/* set local assignment bit (IEEE802) */
}

/**
 * is_zero_ethaddr - Determine if give Ethernet address is all zeros.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is all zeroes.
 */
static inline int is_zero_ethaddr(const u8 *addr)
{
	return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

/**
 * is_multicast_ethaddr - Determine if the Ethernet address is a multicast.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is a multicast address.
 * By definition the broadcast address is also a multicast address.
 */
static inline int is_multicast_ethaddr(const u8 *addr)
{
	return 0x01 & addr[0];
}

/*
 * is_broadcast_ethaddr - Determine if the Ethernet address is broadcast
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is the broadcast address.
 */
static inline int is_broadcast_ethaddr(const u8 *addr)
{
	return (addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5]) ==
		0xff;
}

/*
 * is_valid_ethaddr - Determine if the given Ethernet address is valid
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Check that the Ethernet address (MAC) is not 00:00:00:00:00:00, is not
 * a multicast address, and is not FF:FF:FF:FF:FF:FF.
 *
 * Return true if the address is valid.
 */
static inline int is_valid_ethaddr(const u8 *addr)
{
	/* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
	 * explicitly check for it here. */
	return !is_multicast_ethaddr(addr) && !is_zero_ethaddr(addr);
}

/**
 * string_to_enetaddr() - Parse a MAC address
 *
 * Convert a string MAC address
 *
 * Implemented in lib/net_utils.c (built unconditionally)
 *
 * @addr: MAC address in aa:bb:cc:dd:ee:ff format, where each part is a 2-digit
 *	hex value
 * @enetaddr: Place to put MAC address (6 bytes)
 */
void string_to_enetaddr(const char *addr, uint8_t *enetaddr);

/**
 * string_to_ip() - Convert a string to ip address
 *
 * Implemented in lib/net_utils.c (built unconditionally)
 *
 * @s: Input string to parse
 * @return: in_addr struct containing the parsed IP address
 */
struct in_addr string_to_ip(const char *s);

/**
 * ip_to_string() - Convert an IPv4 address to a string
 *
 * Implemented in lib/net_utils.c (built unconditionally)
 *
 * @x: Input ip to parse
 * @s: string containing the parsed ip address
 */
void ip_to_string(struct in_addr x, char *s);

/* copy a filename (allow for "..." notation, limit length) */
void copy_filename(char *dst, const char *src, int size);

/* Processes a received packet */
void net_process_received_packet(uchar *in_packet, int len);

/**
 * update_tftp - Update firmware over TFTP (via DFU)
 *
 * This function updates board's firmware via TFTP
 *
 * @param addr - memory address where data is stored
 * @param interface - the DFU medium name - e.g. "mmc"
 * @param devstring - the DFU medium number - e.g. "1"
 *
 * Return: - 0 on success, other value on failure
 */
int update_tftp(ulong addr, char *interface, char *devstring);

int net_init(void);

/* Called when a network operation fails to know if it should be re-tried */
int net_start_again(void);

/* NET compatibility */
enum proto_t;
int net_loop(enum proto_t protocol);

/**
 * dhcp_run() - Run DHCP on the current ethernet device
 *
 * This sets the autoload variable, then puts it back to similar to its original
 * state (y, n or unset).
 *
 * @addr: Address to load the file into (0 if @autoload is false)
 * @fname: Filename of file to load (NULL if @autoload is false or to use the
 * default filename)
 * @autoload: true to load the file, false to just get the network IP
 * @return 0 if OK, -EINVAL if the environment failed, -ENOENT if ant file was
 * not found
 */
int dhcp_run(ulong addr, const char *fname, bool autoload);


/**
 * do_ping - Run the ping command
 *
 * @cmdtp: Unused
 * @flag: Command flags (CMD_FLAG_...)
 * @argc: Number of arguments
 * @argv: List of arguments
 * Return: result (see enum command_ret_t)
 */
int do_ping(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

/**
 * do_sntp - Run the sntp command
 *
 * @cmdtp: Unused
 * @flag: Command flags (CMD_FLAG_...)
 * @argc: Number of arguments
 * @argv: List of arguments
 * Return: result (see enum command_ret_t)
 */
int do_sntp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

/**
 * do_tftpb - Run the tftpboot command
 *
 * @cmdtp: Command information for tftpboot
 * @flag: Command flags (CMD_FLAG_...)
 * @argc: Number of arguments
 * @argv: List of arguments
 * Return: result (see enum command_ret_t)
 */
int do_tftpb(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

/**
 * wget_do_request() - sends a wget request
 *
 * Sends a wget request, if DNS resolution is enabled it resolves the
 * given uri.
 *
 * @dst_addr:	destination address to download the file
 * @uri:	uri string of target file of wget
 * Return:	zero on success, negative if failed
 */
int wget_do_request(ulong dst_addr, char *uri);
/**
 * wget_validate_uri() - varidate the uri
 *
 * @uri:	uri string of target file of wget
 * Return:	true if uri is valid, false if uri is invalid
 */
bool wget_validate_uri(char *uri);

/**
 * enum wget_http_method - http method
 */
enum wget_http_method {
	WGET_HTTP_METHOD_GET,
	WGET_HTTP_METHOD_POST,
	WGET_HTTP_METHOD_PATCH,
	WGET_HTTP_METHOD_OPTIONS,
	WGET_HTTP_METHOD_CONNECT,
	WGET_HTTP_METHOD_HEAD,
	WGET_HTTP_METHOD_PUT,
	WGET_HTTP_METHOD_DELETE,
	WGET_HTTP_METHOD_TRACE,
	WGET_HTTP_METHOD_MAX
};

/**
 * define MAX_HTTP_HEADERS_SIZE - maximum headers buffer size
 *
 * When receiving http headers, wget fills a buffer with up
 * to MAX_HTTP_HEADERS_SIZE bytes of header information.
 */
#define MAX_HTTP_HEADERS_SIZE SZ_64K

/**
 * struct wget_http_info - wget parameters
 * @method:		HTTP Method. Filled by client.
 * @status_code:	HTTP status code. Filled by wget.
 * @file_size:		download size. Filled by wget.
 * @buffer_size:	size of client-provided buffer. Filled by client.
 * @set_bootdev:	set boot device with download. Filled by client.
 * @check_buffer_size:	check download does not exceed buffer size.
 *			Filled by client.
 * @hdr_cont_len:	content length according to headers. Filled by wget
 * @headers:		buffer for headers. Filled by wget.
 * @silent:		do not print anything to the console. Filled by client.
 */
struct wget_http_info {
	enum wget_http_method method;
	u32 status_code;
	ulong file_size;
	ulong buffer_size;
	bool set_bootdev;
	bool check_buffer_size;
	u32 hdr_cont_len;
	char *headers;
	bool silent;
};

extern struct wget_http_info default_wget_info;
extern struct wget_http_info *wget_info;
int wget_request(ulong dst_addr, char *uri, struct wget_http_info *info);

void net_sntp_set_rtc(u32 seconds);

#endif /* __NET_COMMON_H__ */
