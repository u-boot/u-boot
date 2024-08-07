/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __NET_LWIP_H__
#define __NET_LWIP_H__

#include <asm/cache.h>
#include <linux/types.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <time.h>

/*
 *	The number of receive packet buffers, and the required packet buffer
 *	alignment in memory.
 *
 */
#define PKTBUFSRX	CONFIG_SYS_RX_ETH_BUFFER
#define PKTALIGN	ARCH_DMA_MINALIGN

/* ARP hardware address length */
#define ARP_HLEN 6

/*
 * Maximum packet size; used to allocate packet storage. Use
 * the maxium Ethernet frame size as specified by the Ethernet
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

struct udevice *eth_get_dev(void); /* get the current device */
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
int eth_init(void);			/* Initialize the device */
int eth_send(void *packet, int length);	   /* Send a packet */
int eth_rx(void);

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
int eth_get_dev_index(void);
const char *eth_get_name(void);
int eth_get_dev_index(void);
int eth_init_state_only(void); /* Set active state */
void eth_set_current(void);		/* set nterface to ethcur var */

struct ethernet_hdr {
	u8		et_dest[ARP_HLEN];	/* Destination node	*/
	u8		et_src[ARP_HLEN];	/* Source node		*/
	u16		et_protlen;		/* Protocol or length	*/
} __attribute__((packed));

/* Ethernet header size */
#define ETHER_HDR_SIZE	(sizeof(struct ethernet_hdr))

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

int net_lwip_init(void);
struct netif *net_lwip_get_netif(void);

int do_dhcp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_dns(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ping(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_tftpb(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_wget(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[]);

#endif /* __NET_LWIP_H__ */
