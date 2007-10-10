/*
 * (C) Copyright 2002 Wolfgang Grandegger, wg@denx.de.
 *
 * This driver for AMD PCnet network controllers is derived from the
 * Linux driver pcnet32.c written 1996-1999 by Thomas Bogendoerfer.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <pci.h>

#if 0
#define	PCNET_DEBUG_LEVEL	0 /* 0=off, 1=init, 2=rx/tx */
#endif

#if PCNET_DEBUG_LEVEL > 0
#define	PCNET_DEBUG1(fmt,args...)	printf (fmt ,##args)
#if PCNET_DEBUG_LEVEL > 1
#define	PCNET_DEBUG2(fmt,args...)	printf (fmt ,##args)
#else
#define PCNET_DEBUG2(fmt,args...)
#endif
#else
#define PCNET_DEBUG1(fmt,args...)
#define PCNET_DEBUG2(fmt,args...)
#endif

#if defined(CONFIG_CMD_NET) \
	&& defined(CONFIG_NET_MULTI) && defined(CONFIG_PCNET)

#if !defined(CONF_PCNET_79C973) && defined(CONF_PCNET_79C975)
#error "Macro for PCnet chip version is not defined!"
#endif

/*
 * Set the number of Tx and Rx buffers, using Log_2(# buffers).
 * Reasonable default values are 4 Tx buffers, and 16 Rx buffers.
 * That translates to 2 (4 == 2^^2) and 4 (16 == 2^^4).
 */
#define PCNET_LOG_TX_BUFFERS	0
#define PCNET_LOG_RX_BUFFERS	2

#define TX_RING_SIZE		(1 << (PCNET_LOG_TX_BUFFERS))
#define TX_RING_LEN_BITS	((PCNET_LOG_TX_BUFFERS) << 12)

#define RX_RING_SIZE		(1 << (PCNET_LOG_RX_BUFFERS))
#define RX_RING_LEN_BITS	((PCNET_LOG_RX_BUFFERS) << 4)

#define PKT_BUF_SZ		1544

/* The PCNET Rx and Tx ring descriptors. */
struct pcnet_rx_head {
    u32 base;
    s16 buf_length;
    s16 status;
    u32 msg_length;
    u32 reserved;
};

struct pcnet_tx_head {
    u32 base;
    s16 length;
    s16 status;
    u32 misc;
    u32 reserved;
};

/* The PCNET 32-Bit initialization block, described in databook. */
struct pcnet_init_block {
    u16 mode;
    u16 tlen_rlen;
    u8	phys_addr[6];
    u16 reserved;
    u32 filter[2];
    /* Receive and transmit ring base, along with extra bits. */
    u32 rx_ring;
    u32 tx_ring;
    u32 reserved2;
};

typedef struct pcnet_priv {
    struct pcnet_rx_head    rx_ring[RX_RING_SIZE];
    struct pcnet_tx_head    tx_ring[TX_RING_SIZE];
    struct pcnet_init_block init_block;
    /* Receive Buffer space */
    unsigned char rx_buf[RX_RING_SIZE][PKT_BUF_SZ + 4];
    int cur_rx;
    int cur_tx;
} pcnet_priv_t;

static pcnet_priv_t *lp;

/* Offsets from base I/O address for WIO mode */
#define PCNET_RDP		0x10
#define PCNET_RAP		0x12
#define PCNET_RESET		0x14
#define PCNET_BDP		0x16

static u16 pcnet_read_csr (struct eth_device *dev, int index)
{
    outw (index, dev->iobase+PCNET_RAP);
    return inw (dev->iobase+PCNET_RDP);
}

static void pcnet_write_csr (struct eth_device *dev, int index, u16 val)
{
    outw (index, dev->iobase+PCNET_RAP);
    outw (val, dev->iobase+PCNET_RDP);
}

static u16 pcnet_read_bcr (struct eth_device *dev, int index)
{
    outw (index, dev->iobase+PCNET_RAP);
    return inw (dev->iobase+PCNET_BDP);
}

static void pcnet_write_bcr (struct eth_device *dev, int index, u16 val)
{
    outw (index, dev->iobase+PCNET_RAP);
    outw (val, dev->iobase+PCNET_BDP);
}

static void pcnet_reset (struct eth_device *dev)
{
    inw (dev->iobase+PCNET_RESET);
}

static int pcnet_check (struct eth_device *dev)
{
    outw (88, dev->iobase+PCNET_RAP);
    return (inw (dev->iobase+PCNET_RAP) == 88);
}

static int  pcnet_init( struct eth_device* dev, bd_t *bis);
static int  pcnet_send (struct eth_device* dev, volatile void *packet,
			int length);
static int  pcnet_recv (struct eth_device* dev);
static void pcnet_halt (struct eth_device* dev);
static int  pcnet_probe(struct eth_device* dev, bd_t *bis, int dev_num);

#define PCI_TO_MEM(d,a) pci_phys_to_mem((pci_dev_t)d->priv, (u_long)(a))
#define PCI_TO_MEM_LE(d,a) (u32)(cpu_to_le32(PCI_TO_MEM(d,a)))

static struct pci_device_id supported[] = {
	{ PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_LANCE },
	{ }
};


int pcnet_initialize(bd_t *bis)
{
    pci_dev_t devbusfn;
    struct eth_device* dev;
    u16 command, status;
    int dev_nr = 0;

    PCNET_DEBUG1("\npcnet_initialize...\n");

    for (dev_nr = 0; ; dev_nr++) {

	/*
	 * Find the PCnet PCI device(s).
	 */
	if ((devbusfn = pci_find_devices(supported, dev_nr)) < 0) {
	    break;
	}

	/*
	 * Allocate and pre-fill the device structure.
	 */
	dev = (struct eth_device*) malloc(sizeof *dev);
	dev->priv = (void *)devbusfn;
	sprintf(dev->name, "pcnet#%d", dev_nr);

	/*
	 * Setup the PCI device.
	 */
	pci_read_config_dword(devbusfn, PCI_BASE_ADDRESS_0, (unsigned int *)&dev->iobase);
	dev->iobase &= ~0xf;

	PCNET_DEBUG1("%s: devbusfn=0x%x iobase=0x%x: ",
	       dev->name, devbusfn, dev->iobase);

	command = PCI_COMMAND_IO | PCI_COMMAND_MASTER;
	pci_write_config_word(devbusfn, PCI_COMMAND, command);
	pci_read_config_word(devbusfn, PCI_COMMAND, &status);
	if ((status & command) != command) {
	    printf("%s: Couldn't enable IO access or Bus Mastering\n",
		   dev->name);
	    free(dev);
	    continue;
	}

	pci_write_config_byte(devbusfn, PCI_LATENCY_TIMER, 0x40);

	/*
	 * Probe the PCnet chip.
	 */
	if (pcnet_probe(dev, bis, dev_nr) < 0) {
	    free(dev);
	    continue;
	}

	/*
	 * Setup device structure and register the driver.
	 */
	dev->init   = pcnet_init;
	dev->halt   = pcnet_halt;
	dev->send   = pcnet_send;
	dev->recv   = pcnet_recv;

	eth_register(dev);
    }

    udelay(10 * 1000);

    return dev_nr;
}

static int pcnet_probe(struct eth_device* dev, bd_t *bis, int dev_nr)
{
    int chip_version;
    char *chipname;
#ifdef PCNET_HAS_PROM
    int i;
#endif

    /* Reset the PCnet controller */
    pcnet_reset(dev);

    /* Check if register access is working */
    if (pcnet_read_csr(dev, 0) != 4 || !pcnet_check(dev)) {
	printf("%s: CSR register access check failed\n", dev->name);
	return -1;
    }

    /* Identify the chip */
    chip_version = pcnet_read_csr(dev, 88) | (pcnet_read_csr(dev,89) << 16);
    if ((chip_version & 0xfff) != 0x003)
       return -1;
    chip_version = (chip_version >> 12) & 0xffff;
    switch (chip_version) {
    case 0x2621:
	chipname = "PCnet/PCI II 79C970A"; /* PCI */
	break;
#ifdef CONFIG_PCNET_79C973
    case 0x2625:
	chipname = "PCnet/FAST III 79C973"; /* PCI */
	break;
#endif
#ifdef CONFIG_PCNET_79C975
    case 0x2627:
	chipname = "PCnet/FAST III 79C975"; /* PCI */
	break;
#endif
    default:
	printf("%s: PCnet version %#x not supported\n",
	       dev->name, chip_version);
	return -1;
    }

    PCNET_DEBUG1("AMD %s\n", chipname);

#ifdef PCNET_HAS_PROM
    /*
     * In most chips, after a chip reset, the ethernet address is read from
     * the station address PROM at the base address and programmed into the
     * "Physical Address Registers" CSR12-14.
     */
    for (i = 0; i < 3; i++) {
	unsigned int val;
	val = pcnet_read_csr(dev, i+12) & 0x0ffff;
	/* There may be endianness issues here. */
	dev->enetaddr[2*i  ] =  val       & 0x0ff;
	dev->enetaddr[2*i+1] = (val >> 8) & 0x0ff;
    }
#endif /* PCNET_HAS_PROM */

    return 0;
}

static int pcnet_init(struct eth_device* dev, bd_t *bis)
{
    int i, val;
    u32 addr;

    PCNET_DEBUG1("%s: pcnet_init...\n", dev->name);

    /* Switch pcnet to 32bit mode */
    pcnet_write_bcr (dev, 20, 2);

#ifdef CONFIG_PN62
    /* Setup LED registers */
    val = pcnet_read_bcr (dev, 2) | 0x1000;
    pcnet_write_bcr (dev, 2, val);    /* enable LEDPE */
    pcnet_write_bcr (dev, 4, 0x5080); /* 100MBit */
    pcnet_write_bcr (dev, 5, 0x40c0); /* LNKSE */
    pcnet_write_bcr (dev, 6, 0x4090); /* TX Activity */
    pcnet_write_bcr (dev, 7, 0x4084); /* RX Activity */
#endif

    /* Set/reset autoselect bit */
    val = pcnet_read_bcr (dev, 2) & ~2;
    val |= 2;
    pcnet_write_bcr (dev, 2, val);

    /* Enable auto negotiate, setup, disable fd */
    val = pcnet_read_bcr(dev, 32) & ~0x98;
    val |= 0x20;
    pcnet_write_bcr(dev, 32, val);

    /*
     * We only maintain one structure because the drivers will never
     * be used concurrently. In 32bit mode the RX and TX ring entries
     * must be aligned on 16-byte boundaries.
     */
    if (lp == NULL) {
	addr = (u32)malloc(sizeof(pcnet_priv_t) + 0x10);
	addr = (addr + 0xf) & ~0xf;
	lp = (pcnet_priv_t *)addr;
    }

    lp->init_block.mode = cpu_to_le16(0x0000);
    lp->init_block.filter[0] = 0x00000000;
    lp->init_block.filter[1] = 0x00000000;

    /*
     * Initialize the Rx ring.
     */
    lp->cur_rx = 0;
    for (i = 0; i < RX_RING_SIZE; i++) {
	lp->rx_ring[i].base = PCI_TO_MEM_LE(dev, lp->rx_buf[i]);
	lp->rx_ring[i].buf_length = cpu_to_le16(-PKT_BUF_SZ);
	lp->rx_ring[i].status = cpu_to_le16(0x8000);
	PCNET_DEBUG1("Rx%d: base=0x%x buf_length=0x%hx status=0x%hx\n",
	       i, lp->rx_ring[i].base, lp->rx_ring[i].buf_length,
	       lp->rx_ring[i].status);
    }

    /*
     * Initialize the Tx ring. The Tx buffer address is filled in as
     * needed, but we do need to clear the upper ownership bit.
     */
    lp->cur_tx = 0;
    for (i = 0; i < TX_RING_SIZE; i++) {
	lp->tx_ring[i].base = 0;
	lp->tx_ring[i].status = 0;
    }

    /*
     * Setup Init Block.
     */
    PCNET_DEBUG1("Init block at 0x%p: MAC", &lp->init_block);

    for (i = 0; i < 6; i++) {
	lp->init_block.phys_addr[i] = dev->enetaddr[i];
	PCNET_DEBUG1(" %02x", lp->init_block.phys_addr[i]);
    }

    lp->init_block.tlen_rlen = cpu_to_le16(TX_RING_LEN_BITS |
					   RX_RING_LEN_BITS);
    lp->init_block.rx_ring = PCI_TO_MEM_LE(dev, lp->rx_ring);
    lp->init_block.tx_ring = PCI_TO_MEM_LE(dev, lp->tx_ring);

    PCNET_DEBUG1("\ntlen_rlen=0x%x rx_ring=0x%x tx_ring=0x%x\n",
	   lp->init_block.tlen_rlen,
	   lp->init_block.rx_ring, lp->init_block.tx_ring);

    /*
     * Tell the controller where the Init Block is located.
     */
    addr = PCI_TO_MEM(dev, &lp->init_block);
    pcnet_write_csr(dev, 1, addr & 0xffff);
    pcnet_write_csr(dev, 2, (addr >> 16) & 0xffff);

    pcnet_write_csr (dev, 4, 0x0915);
    pcnet_write_csr (dev, 0, 0x0001); /* start */

    /* Wait for Init Done bit */
    for (i = 10000; i > 0; i--) {
	if (pcnet_read_csr (dev, 0) & 0x0100)
	    break;
	udelay(10);
    }
    if (i <= 0) {
	printf("%s: TIMEOUT: controller init failed\n", dev->name);
	pcnet_reset (dev);
	return -1;
    }

    /*
     * Finally start network controller operation.
     */
    pcnet_write_csr (dev, 0, 0x0002);

    return 0;
}

static int pcnet_send(struct eth_device* dev, volatile void *packet, int pkt_len)
{
    int i, status;
    struct pcnet_tx_head *entry = &lp->tx_ring[lp->cur_tx];

    PCNET_DEBUG2("Tx%d: %d bytes from 0x%p ", lp->cur_tx, pkt_len, packet);

    /* Wait for completion by testing the OWN bit */
    for (i = 1000; i > 0; i--) {
	status = le16_to_cpu(entry->status);
	if ((status & 0x8000) == 0)
	    break;
	udelay(100);
	PCNET_DEBUG2(".");
    }
    if (i <= 0) {
	printf("%s: TIMEOUT: Tx%d failed (status = 0x%x)\n",
	       dev->name, lp->cur_tx, status);
	pkt_len = 0;
	goto failure;
    }

    /*
     * Setup Tx ring. Caution: the write order is important here,
     * set the status with the "ownership" bits last.
     */
    status = 0x8300;
    entry->length = le16_to_cpu(-pkt_len);
    entry->misc   = 0x00000000;
    entry->base   = PCI_TO_MEM_LE(dev, packet);
    entry->status = le16_to_cpu(status);

    /* Trigger an immediate send poll. */
    pcnet_write_csr (dev, 0, 0x0008);

 failure:
    if (++lp->cur_tx >= TX_RING_SIZE)
	lp->cur_tx = 0;

    PCNET_DEBUG2("done\n");
    return pkt_len;
}

static int pcnet_recv(struct eth_device* dev)
{
    struct pcnet_rx_head *entry;
    int pkt_len = 0;
    u16 status;

    while (1) {
	entry = &lp->rx_ring[lp->cur_rx];
	/*
	 * If we own the next entry, it's a new packet. Send it up.
	 */
	if (((status = le16_to_cpu(entry->status)) & 0x8000) != 0) {
	    break;
	}
	status >>= 8;

	if (status != 0x03) {	/* There was an error. */

	    printf("%s: Rx%d", dev->name, lp->cur_rx);
	    PCNET_DEBUG1(" (status=0x%x)", status);
	    if (status & 0x20) printf(" Frame");
	    if (status & 0x10) printf(" Overflow");
	    if (status & 0x08) printf(" CRC");
	    if (status & 0x04) printf(" Fifo");
	    printf(" Error\n");
	    entry->status &= le16_to_cpu(0x03ff);

	} else {

	    pkt_len = (le32_to_cpu(entry->msg_length) & 0xfff) - 4;
	    if (pkt_len < 60) {
		printf("%s: Rx%d: invalid packet length %d\n",
		       dev->name, lp->cur_rx, pkt_len);
	    } else {
		NetReceive(lp->rx_buf[lp->cur_rx], pkt_len);
		PCNET_DEBUG2("Rx%d: %d bytes from 0x%p\n",
		       lp->cur_rx, pkt_len, lp->rx_buf[lp->cur_rx]);
	    }
	}
	entry->status |= cpu_to_le16(0x8000);

	if (++lp->cur_rx >= RX_RING_SIZE)
	    lp->cur_rx = 0;
    }
    return pkt_len;
}

static void pcnet_halt(struct eth_device* dev)
{
    int i;

    PCNET_DEBUG1("%s: pcnet_halt...\n", dev->name);

    /* Reset the PCnet controller */
    pcnet_reset (dev);

    /* Wait for Stop bit */
    for (i = 1000; i > 0; i--) {
	if (pcnet_read_csr (dev, 0) & 0x4)
	    break;
	udelay(10);
    }
    if (i <= 0) {
	printf("%s: TIMEOUT: controller reset failed\n", dev->name);
    }
}

#endif
