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
int do_ping(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_tftpb(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

#endif /* __NET_LWIP_H__ */
