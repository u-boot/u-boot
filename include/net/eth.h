/* SPDX-License-Identifier: GPL-2.0+ */

#include <net/arp.h>

struct udevice;

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
/*
 * The devname can be either an exact name given by the driver or device tree
 * or it can be an alias of the form "eth%d"
 */
struct udevice *eth_get_dev_by_name(const char *devname);
unsigned char *eth_get_ethaddr(void); /* get the current device MAC */

/* Used only when NetConsole is enabled */
int eth_is_active(struct udevice *dev); /* Test device for active state */
int eth_init_state_only(void); /* Set active state */
void eth_halt_state_only(void); /* Set passive state */

int eth_initialize(void);		/* Initialize network subsystem */
void eth_try_another(int first_restart);	/* Change the device */
void eth_set_current(void);		/* set nterface to ethcur var */

int eth_get_dev_index(void);		/* get the device index */

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

void eth_init_rings(void);	/* Initialize rings */
int eth_init(void);			/* Initialize the device */
int eth_send(void *packet, int length);	   /* Send a packet */

#if defined(CONFIG_API) || defined(CONFIG_EFI_LOADER)
int eth_receive(void *packet, int length); /* Receive a packet*/
extern void (*push_packet)(void *packet, int length);
#endif
int eth_rx(void);			/* Check for received packets */
void eth_halt(void);			/* stop SCC */
const char *eth_get_name(void);		/* get name of current device */

/*
 *	The number of receive packet buffers, and the required packet buffer
 *	alignment in memory.
 *
 */
#define PKTBUFSRX	CONFIG_SYS_RX_ETH_BUFFER
#define PKTALIGN	ARCH_DMA_MINALIGN

/* Number of packets processed together */
#define ETH_PACKETS_BATCH_RECV	32

extern u8		net_ethaddr[ARP_HLEN];		/* Our ethernet address */
extern u8		net_server_ethaddr[ARP_HLEN];	/* Boot server enet address */
extern struct in_addr	net_ip;		/* Our    IP addr (0 = unknown) */
extern struct in_addr	net_server_ip;	/* Server IP addr (0 = unknown) */
extern uchar		*net_tx_packet;		/* THE transmit packet */
extern uchar		*net_rx_packets[PKTBUFSRX]; /* Receive packets */
extern uchar		*net_rx_packet;		/* Current receive packet */
extern int		net_rx_packet_len;	/* Current rx packet length */
extern const u8		net_bcast_ethaddr[ARP_HLEN];	/* Ethernet broadcast address */
extern const u8		net_null_ethaddr[ARP_HLEN];

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

typedef struct ulwip {
	bool loop; /* spin in lwip loop */
	bool active; /* active lwip app processing*/
	int err;
	bool init_done;

} ulwip;

struct ulwip *eth_lwip_priv(struct udevice *current);
