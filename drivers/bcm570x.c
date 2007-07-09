/*
 * Broadcom BCM570x Ethernet Driver for U-Boot.
 * Support 5701, 5702, 5703, and 5704. Single instance driver.
 * Copyright (C) 2002 James F. Dougherty (jfd@broadcom.com)
 */

#include <common.h>

#if defined(CONFIG_CMD_NET) \
	&& (!defined(CONFIG_NET_MULTI)) && defined(CONFIG_BCM570x)

#ifdef CONFIG_BMW
#include <mpc824x.h>
#endif
#include <net.h>
#include "bcm570x_mm.h"
#include "bcm570x_autoneg.h"
#include <pci.h>
#include <malloc.h>


/*
 * PCI Registers and definitions.
 */
#define PCI_CMD_MASK	0xffff0000	/* mask to save status bits */
#define PCI_ANY_ID (~0)

/*
 * PCI memory base for Ethernet device as well as device Interrupt.
 */
#define BCM570X_MBAR 	0x80100000
#define BCM570X_ILINE   1


#define SECOND_USEC	1000000
#define MAX_PACKET_SIZE 1600
#define MAX_UNITS       4

/* Globals to this module */
int initialized = 0;
unsigned int ioBase = 0;
volatile PLM_DEVICE_BLOCK    pDevice = NULL;        /* 570x softc */
volatile PUM_DEVICE_BLOCK    pUmDevice = NULL;

/* Used to pass the full-duplex flag, etc. */
int line_speed[MAX_UNITS] = {0,0,0,0};
static int full_duplex[MAX_UNITS] = {1,1,1,1};
static int rx_flow_control[MAX_UNITS] = {0,0,0,0};
static int tx_flow_control[MAX_UNITS] = {0,0,0,0};
static int auto_flow_control[MAX_UNITS] = {0,0,0,0};
static int tx_checksum[MAX_UNITS] = {1,1,1,1};
static int rx_checksum[MAX_UNITS] = {1,1,1,1};
static int auto_speed[MAX_UNITS] = {1,1,1,1};

#if JUMBO_FRAMES
/* Jumbo MTU for interfaces. */
static int mtu[MAX_UNITS] = {0,0,0,0};
#endif

/* Turn on Wake-on lan for a device unit */
static int enable_wol[MAX_UNITS] = {0,0,0,0};

#define TX_DESC_CNT DEFAULT_TX_PACKET_DESC_COUNT
static unsigned int tx_pkt_desc_cnt[MAX_UNITS] =
	{TX_DESC_CNT,TX_DESC_CNT,TX_DESC_CNT, TX_DESC_CNT};

#define RX_DESC_CNT DEFAULT_STD_RCV_DESC_COUNT
static unsigned int rx_std_desc_cnt[MAX_UNITS] =
	{RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT};

static unsigned int rx_adaptive_coalesce[MAX_UNITS] = {1,1,1,1};

#if T3_JUMBO_RCV_RCB_ENTRY_COUNT
#define JBO_DESC_CNT DEFAULT_JUMBO_RCV_DESC_COUNT
static unsigned int rx_jumbo_desc_cnt[MAX_UNITS] =
	{JBO_DESC_CNT, JBO_DESC_CNT, JBO_DESC_CNT, JBO_DESC_CNT};
#endif
#define RX_COAL_TK DEFAULT_RX_COALESCING_TICKS
static unsigned int rx_coalesce_ticks[MAX_UNITS] =
	{RX_COAL_TK, RX_COAL_TK, RX_COAL_TK, RX_COAL_TK};

#define RX_COAL_FM DEFAULT_RX_MAX_COALESCED_FRAMES
static unsigned int rx_max_coalesce_frames[MAX_UNITS] =
	{RX_COAL_FM, RX_COAL_FM, RX_COAL_FM, RX_COAL_FM};

#define TX_COAL_TK DEFAULT_TX_COALESCING_TICKS
static unsigned int tx_coalesce_ticks[MAX_UNITS] =
	{TX_COAL_TK, TX_COAL_TK, TX_COAL_TK, TX_COAL_TK};

#define TX_COAL_FM DEFAULT_TX_MAX_COALESCED_FRAMES
static unsigned int tx_max_coalesce_frames[MAX_UNITS] =
	{TX_COAL_FM, TX_COAL_FM, TX_COAL_FM, TX_COAL_FM};

#define ST_COAL_TK DEFAULT_STATS_COALESCING_TICKS
static unsigned int stats_coalesce_ticks[MAX_UNITS] =
	{ST_COAL_TK, ST_COAL_TK, ST_COAL_TK, ST_COAL_TK};


/*
 * Legitimate values for BCM570x device types
 */
typedef enum {
	BCM5700VIGIL = 0,
	BCM5700A6,
	BCM5700T6,
	BCM5700A9,
	BCM5700T9,
	BCM5700,
	BCM5701A5,
	BCM5701T1,
	BCM5701T8,
	BCM5701A7,
	BCM5701A10,
	BCM5701A12,
	BCM5701,
	BCM5702,
	BCM5703,
	BCM5703A31,
	TC996T,
	TC996ST,
	TC996SSX,
	TC996SX,
	TC996BT,
	TC997T,
	TC997SX,
	TC1000T,
	TC940BR01,
	TC942BR01,
	NC6770,
	NC7760,
	NC7770,
	NC7780
} board_t;

/* Chip-Rev names for each device-type */
static struct {
    char* name;
} chip_rev[] = {
       {"BCM5700VIGIL"},
       {"BCM5700A6"},
       {"BCM5700T6"},
       {"BCM5700A9"},
       {"BCM5700T9"},
       {"BCM5700"},
       {"BCM5701A5"},
       {"BCM5701T1"},
       {"BCM5701T8"},
       {"BCM5701A7"},
       {"BCM5701A10"},
       {"BCM5701A12"},
       {"BCM5701"},
       {"BCM5702"},
       {"BCM5703"},
       {"BCM5703A31"},
       {"TC996T"},
       {"TC996ST"},
       {"TC996SSX"},
       {"TC996SX"},
       {"TC996BT"},
       {"TC997T"},
       {"TC997SX"},
       {"TC1000T"},
       {"TC940BR01"},
       {"TC942BR01"},
       {"NC6770"},
       {"NC7760"},
       {"NC7770"},
       {"NC7780"},
       {0}
};


/* indexed by board_t, above */
static struct {
    char *name;
} board_info[] = {
	{ "Broadcom Vigil B5700 1000Base-T" },
	{ "Broadcom BCM5700 1000Base-T" },
	{ "Broadcom BCM5700 1000Base-SX" },
	{ "Broadcom BCM5700 1000Base-SX" },
	{ "Broadcom BCM5700 1000Base-T" },
	{ "Broadcom BCM5700" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-SX" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701" },
	{ "Broadcom BCM5702 1000Base-T" },
	{ "Broadcom BCM5703 1000Base-T" },
	{ "Broadcom BCM5703 1000Base-SX" },
	{ "3Com 3C996 10/100/1000 Server NIC" },
	{ "3Com 3C996 10/100/1000 Server NIC" },
	{ "3Com 3C996 Gigabit Fiber-SX Server NIC" },
	{ "3Com 3C996 Gigabit Fiber-SX Server NIC" },
	{ "3Com 3C996B Gigabit Server NIC" },
	{ "3Com 3C997 Gigabit Server NIC" },
	{ "3Com 3C997 Gigabit Fiber-SX Server NIC" },
	{ "3Com 3C1000 Gigabit NIC" },
	{ "3Com 3C940 Gigabit LOM (21X21)" },
	{ "3Com 3C942 Gigabit LOM (31X31)" },
	{ "Compaq NC6770 Gigabit Server Adapter" },
	{ "Compaq NC7760 Gigabit Server Adapter" },
	{ "Compaq NC7770 Gigabit Server Adapter" },
	{ "Compaq NC7780 Gigabit Server Adapter" },
	{ 0 },
};

/* PCI Devices which use the 570x chipset */
struct pci_device_table {
    unsigned short vendor_id, device_id; /* Vendor/DeviceID */
    unsigned short subvendor, subdevice; /* Subsystem ID's or PCI_ANY_ID */
    unsigned int class, class_mask; /* (class,subclass,prog-if) triplet */
    unsigned long board_id;	    /* Data private to the driver */
    int io_size, min_latency;
} bcm570xDevices[] = {
	{0x14e4, 0x1644, 0x1014, 0x0277, 0, 0, BCM5700VIGIL ,128,32},
	{0x14e4, 0x1644, 0x14e4, 0x1644, 0, 0, BCM5700A6 ,128,32},
	{0x14e4, 0x1644, 0x14e4, 0x2, 0, 0, BCM5700T6 ,128,32},
	{0x14e4, 0x1644, 0x14e4, 0x3, 0, 0, BCM5700A9 ,128,32},
	{0x14e4, 0x1644, 0x14e4, 0x4, 0, 0, BCM5700T9 ,128,32},
	{0x14e4, 0x1644, 0x1028, 0xd1, 0, 0, BCM5700 ,128,32},
	{0x14e4, 0x1644, 0x1028, 0x0106, 0, 0, BCM5700 ,128,32},
	{0x14e4, 0x1644, 0x1028, 0x0109, 0, 0, BCM5700 ,128,32},
	{0x14e4, 0x1644, 0x1028, 0x010a, 0, 0, BCM5700 ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1000, 0, 0, TC996T ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1001, 0, 0, TC996ST ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1002, 0, 0, TC996SSX ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1003, 0, 0, TC997T ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1005, 0, 0, TC997SX ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1008, 0, 0, TC942BR01 ,128,32},
	{0x14e4, 0x1644, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5700 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 1, 0, 0, BCM5701A5 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 5, 0, 0, BCM5701T1 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 6, 0, 0, BCM5701T8 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 7, 0, 0, BCM5701A7 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 8, 0, 0, BCM5701A10 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 0x8008, 0, 0, BCM5701A12 ,128,32},
	{0x14e4, 0x1645, 0x0e11, 0xc1, 0, 0, NC6770 ,128,32},
	{0x14e4, 0x1645, 0x0e11, 0x7c, 0, 0, NC7770 ,128,32},
	{0x14e4, 0x1645, 0x0e11, 0x85, 0, 0, NC7780 ,128,32},
	{0x14e4, 0x1645, 0x1028, 0x0121, 0, 0, BCM5701 ,128,32},
	{0x14e4, 0x1645, 0x10b7, 0x1004, 0, 0, TC996SX ,128,32},
	{0x14e4, 0x1645, 0x10b7, 0x1006, 0, 0, TC996BT ,128,32},
	{0x14e4, 0x1645, 0x10b7, 0x1007, 0, 0, TC1000T ,128,32},
	{0x14e4, 0x1645, 0x10b7, 0x1008, 0, 0, TC940BR01 ,128,32},
	{0x14e4, 0x1645, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5701 ,128,32},
	{0x14e4, 0x1646, 0x14e4, 0x8009, 0, 0, BCM5702 ,128,32},
	{0x14e4, 0x1646, 0x0e11, 0xbb, 0, 0, NC7760 ,128,32},
	{0x14e4, 0x1646, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5702 ,128,32},
	{0x14e4, 0x16a6, 0x14e4, 0x8009, 0, 0, BCM5702 ,128,32},
	{0x14e4, 0x16a6, 0x0e11, 0xbb, 0, 0, NC7760 ,128,32},
	{0x14e4, 0x16a6, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5702 ,128,32},
	{0x14e4, 0x1647, 0x14e4, 0x0009, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x1647, 0x14e4, 0x000a, 0, 0, BCM5703A31 ,128,32},
	{0x14e4, 0x1647, 0x14e4, 0x000b, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x1647, 0x14e4, 0x800a, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x1647, 0x0e11, 0x9a, 0, 0, NC7770 ,128,32},
	{0x14e4, 0x1647, 0x0e11, 0x99, 0, 0, NC7780 ,128,32},
	{0x14e4, 0x1647, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x16a7, 0x14e4, 0x0009, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x16a7, 0x14e4, 0x000a, 0, 0, BCM5703A31 ,128,32},
	{0x14e4, 0x16a7, 0x14e4, 0x000b, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x16a7, 0x14e4, 0x800a, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x16a7, 0x0e11, 0x9a, 0, 0, NC7770 ,128,32},
	{0x14e4, 0x16a7, 0x0e11, 0x99, 0, 0, NC7780 ,128,32},
	{0x14e4, 0x16a7, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5703 ,128,32}
};

#define n570xDevices   (sizeof(bcm570xDevices)/sizeof(bcm570xDevices[0]))


/*
 * Allocate a packet buffer from the bcm570x packet pool.
 */
void *
bcm570xPktAlloc(int u, int pksize)
{
    return malloc(pksize);
}

/*
 * Free a packet previously allocated from the bcm570x packet
 * buffer pool.
 */
void
bcm570xPktFree(int u, void *p)
{
    free(p);
}

int
bcm570xReplenishRxBuffers(PUM_DEVICE_BLOCK pUmDevice)
{
    PLM_PACKET pPacket;
    PUM_PACKET pUmPacket;
    void *skb;
    int queue_rx = 0;
    int ret = 0;

    while ((pUmPacket = (PUM_PACKET)
	    QQ_PopHead(&pUmDevice->rx_out_of_buf_q.Container)) != 0) {

	pPacket = (PLM_PACKET) pUmPacket;

	/* reuse an old skb */
	if (pUmPacket->skbuff) {
	    QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
	    queue_rx = 1;
	    continue;
	}
	if ( ( skb = bcm570xPktAlloc(pUmDevice->index,
				     pPacket->u.Rx.RxBufferSize + 2)) == 0) {
	    QQ_PushHead(&pUmDevice->rx_out_of_buf_q.Container,pPacket);
	    printf("NOTICE: Out of RX memory.\n");
	    ret = 1;
	    break;
	}

	pUmPacket->skbuff = skb;
	QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
	queue_rx = 1;
    }

    if (queue_rx) {
	LM_QueueRxPackets(pDevice);
    }

    return ret;
}

/*
 * Probe, Map, and Init 570x device.
 */
int eth_init(bd_t *bis)
{
    int i, rv, devFound = FALSE;
    pci_dev_t  devbusfn;
    unsigned short status;

    /* Find PCI device, if it exists, configure ...  */
    for( i = 0; i < n570xDevices; i++){
	devbusfn = pci_find_device(bcm570xDevices[i].vendor_id,
				   bcm570xDevices[i].device_id, 0);
	if(devbusfn == -1) {
	    continue; /* No device of that vendor/device ID */
	} else {

	    /* Set ILINE */
	    pci_write_config_byte(devbusfn,
				  PCI_INTERRUPT_LINE, BCM570X_ILINE);

	    /*
	     * 0x10 - 0x14 define one 64-bit MBAR.
	     * 0x14 is the higher-order address bits of the BAR.
	     */
	    pci_write_config_dword(devbusfn,
				   PCI_BASE_ADDRESS_1, 0);

	    ioBase = BCM570X_MBAR;

	    pci_write_config_dword(devbusfn,
				   PCI_BASE_ADDRESS_0, ioBase);

	    /*
	     * Enable PCI memory, IO, and Master -- don't
	     * reset any status bits in doing so.
	     */
	    pci_read_config_word(devbusfn,
				 PCI_COMMAND, &status);

	    status |= PCI_COMMAND_MEMORY|PCI_COMMAND_MASTER;

	    pci_write_config_word(devbusfn,
				  PCI_COMMAND, status);

	    printf("\n%s: bus %d, device %d, function %d: MBAR=0x%x\n",
		   board_info[bcm570xDevices[i].board_id].name,
		   PCI_BUS(devbusfn),
		   PCI_DEV(devbusfn),
		   PCI_FUNC(devbusfn),
		   ioBase);

	    /* Allocate once, but always clear on init */
	    if (!pDevice) {
		pDevice = malloc(sizeof(UM_DEVICE_BLOCK));
		pUmDevice = (PUM_DEVICE_BLOCK)pDevice;
		memset(pDevice, 0x0, sizeof(UM_DEVICE_BLOCK));
	    }

	    /* Configure pci dev structure */
	    pUmDevice->pdev = devbusfn;
	    pUmDevice->index = 0;
	    pUmDevice->tx_pkt = 0;
	    pUmDevice->rx_pkt = 0;
	    devFound = TRUE;
	    break;
	}
    }

    if(!devFound){
	printf("eth_init: FAILURE: no BCM570x Ethernet devices found.\n");
	return -1;
    }

    /* Setup defaults for chip */
    pDevice->TaskToOffload = LM_TASK_OFFLOAD_NONE;

    if (pDevice->ChipRevId == T3_CHIP_ID_5700_B0) {
	pDevice->TaskToOffload = LM_TASK_OFFLOAD_NONE;
    } else {

	if (rx_checksum[i]) {
	    pDevice->TaskToOffload |=
		LM_TASK_OFFLOAD_RX_TCP_CHECKSUM |
		LM_TASK_OFFLOAD_RX_UDP_CHECKSUM;
	}

	if (tx_checksum[i]) {
	    pDevice->TaskToOffload |=
		LM_TASK_OFFLOAD_TX_TCP_CHECKSUM |
		LM_TASK_OFFLOAD_TX_UDP_CHECKSUM;
	    pDevice->NoTxPseudoHdrChksum = TRUE;
	}
    }

    /* Set Device PCI Memory base address */
    pDevice->pMappedMemBase = (PLM_UINT8) ioBase;

    /* Pull down adapter info */
    if ((rv = LM_GetAdapterInfo(pDevice)) != LM_STATUS_SUCCESS) {
	printf("bcm570xEnd: LM_GetAdapterInfo failed: rv=%d!\n", rv );
	return -2;
    }

    /* Lock not needed */
    pUmDevice->do_global_lock = 0;

    if (T3_ASIC_REV(pUmDevice->lm_dev.ChipRevId) == T3_ASIC_REV_5700) {
	/* The 5700 chip works best without interleaved register */
	/* accesses on certain machines. */
	pUmDevice->do_global_lock = 1;
    }

    /* Setup timer delays */
    if (T3_ASIC_REV(pDevice->ChipRevId) == T3_ASIC_REV_5701) {
	pDevice->UseTaggedStatus = TRUE;
	pUmDevice->timer_interval = CFG_HZ;
    }
    else {
	pUmDevice->timer_interval = CFG_HZ / 50;
    }

    /* Grab name .... */
    pUmDevice->name =
	(char*)malloc(strlen(board_info[bcm570xDevices[i].board_id].name)+1);
    strcpy(pUmDevice->name,board_info[bcm570xDevices[i].board_id].name);

    memcpy(pDevice->NodeAddress, bis->bi_enetaddr, 6);
    LM_SetMacAddress(pDevice, bis->bi_enetaddr);
    /* Init queues  .. */
    QQ_InitQueue(&pUmDevice->rx_out_of_buf_q.Container,
		 MAX_RX_PACKET_DESC_COUNT);
    pUmDevice->rx_last_cnt = pUmDevice->tx_last_cnt = 0;

    /* delay for 4 seconds */
    pUmDevice->delayed_link_ind =
	(4 * CFG_HZ) / pUmDevice->timer_interval;

    pUmDevice->adaptive_expiry =
	CFG_HZ / pUmDevice->timer_interval;

    /* Sometimes we get spurious ints. after reset when link is down. */
    /* This field tells the isr to service the int. even if there is */
    /* no status block update. */
    pUmDevice->adapter_just_inited =
	(3 * CFG_HZ) / pUmDevice->timer_interval;

    /* Initialize 570x */
    if (LM_InitializeAdapter(pDevice) != LM_STATUS_SUCCESS) {
	printf("ERROR: Adapter initialization failed.\n");
	return ERROR;
    }

    /* Enable chip ISR */
    LM_EnableInterrupt(pDevice);

    /* Clear MC table */
    LM_MulticastClear(pDevice);

    /* Enable Multicast */
    LM_SetReceiveMask(pDevice,
		      pDevice->ReceiveMask | LM_ACCEPT_ALL_MULTICAST);

    pUmDevice->opened = 1;
    pUmDevice->tx_full = 0;
    pUmDevice->tx_pkt = 0;
    pUmDevice->rx_pkt = 0;
    printf("eth%d: %s @0x%lx,",
	   pDevice->index, pUmDevice->name, (unsigned long)ioBase);
    printf(	"node addr ");
    for (i = 0; i < 6; i++) {
	printf("%2.2x", pDevice->NodeAddress[i]);
    }
    printf("\n");

    printf("eth%d: ", pDevice->index);
    printf("%s with ",
	   chip_rev[bcm570xDevices[i].board_id].name);

    if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5400_PHY_ID)
	printf("Broadcom BCM5400 Copper ");
    else if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5401_PHY_ID)
	printf("Broadcom BCM5401 Copper ");
    else if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5411_PHY_ID)
	printf("Broadcom BCM5411 Copper ");
    else if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5701_PHY_ID)
	printf("Broadcom BCM5701 Integrated Copper ");
    else if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5703_PHY_ID)
	printf("Broadcom BCM5703 Integrated Copper ");
    else if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM8002_PHY_ID)
	printf("Broadcom BCM8002 SerDes ");
    else if (pDevice->EnableTbi)
	printf("Agilent HDMP-1636 SerDes ");
    else
	printf("Unknown ");
    printf("transceiver found\n");

    printf("eth%d: %s, MTU: %d,",
	   pDevice->index, pDevice->BusSpeedStr, 1500);

    if ((pDevice->ChipRevId != T3_CHIP_ID_5700_B0) &&
	rx_checksum[i])
	printf("Rx Checksum ON\n");
    else
	printf("Rx Checksum OFF\n");
    initialized++;

    return 0;
}

/* Ethernet Interrupt service routine */
void
eth_isr(void)
{
    LM_UINT32 oldtag, newtag;
    int i;

    pUmDevice->interrupt = 1;

    if (pDevice->UseTaggedStatus) {
	if ((pDevice->pStatusBlkVirt->Status & STATUS_BLOCK_UPDATED) ||
	    pUmDevice->adapter_just_inited) {
	    MB_REG_WR(pDevice, Mailbox.Interrupt[0].Low, 1);
	    oldtag = pDevice->pStatusBlkVirt->StatusTag;

	    for (i = 0; ; i++) {
		pDevice->pStatusBlkVirt->Status &= ~STATUS_BLOCK_UPDATED;
		LM_ServiceInterrupts(pDevice);
		newtag = pDevice->pStatusBlkVirt->StatusTag;
		if ((newtag == oldtag) || (i > 50)) {
		    MB_REG_WR(pDevice, Mailbox.Interrupt[0].Low, newtag << 24);
		    if (pDevice->UndiFix) {
			REG_WR(pDevice, Grc.LocalCtrl,
			       pDevice->GrcLocalCtrl | 0x2);
		    }
		    break;
		 }
		oldtag = newtag;
	    }
	}
    }
    else {
	while (pDevice->pStatusBlkVirt->Status & STATUS_BLOCK_UPDATED) {
	    unsigned int dummy;

	    pDevice->pMemView->Mailbox.Interrupt[0].Low = 1;
	    pDevice->pStatusBlkVirt->Status &= ~STATUS_BLOCK_UPDATED;
	    LM_ServiceInterrupts(pDevice);
	    pDevice->pMemView->Mailbox.Interrupt[0].Low = 0;
	    dummy = pDevice->pMemView->Mailbox.Interrupt[0].Low;
	}
    }

    /* Allocate new RX buffers */
    if (QQ_GetEntryCnt(&pUmDevice->rx_out_of_buf_q.Container)) {
	bcm570xReplenishRxBuffers(pUmDevice);
    }

    /* Queue packets */
    if (QQ_GetEntryCnt(&pDevice->RxPacketFreeQ.Container)) {
	LM_QueueRxPackets(pDevice);
    }

    if (pUmDevice->tx_queued) {
	pUmDevice->tx_queued = 0;
    }

    if(pUmDevice->tx_full){
	if(pDevice->LinkStatus != LM_STATUS_LINK_DOWN){
	    printf("NOTICE: tx was previously blocked, restarting MUX\n");
	    pUmDevice->tx_full = 0;
	}
    }

    pUmDevice->interrupt = 0;

}

int
eth_send(volatile void *packet, int length)
{
    int status = 0;
#if ET_DEBUG
    unsigned char* ptr = (unsigned char*)packet;
#endif
    PLM_PACKET pPacket;
    PUM_PACKET pUmPacket;

    /* Link down, return */
    while(pDevice->LinkStatus == LM_STATUS_LINK_DOWN) {
#if 0
	printf("eth%d: link down - check cable or link partner.\n",
	       pUmDevice->index);
#endif
	eth_isr();

	/* Wait to see link for one-half a second before sending ... */
	udelay(1500000);

    }

    /* Clear sent flag */
    pUmDevice->tx_pkt = 0;

    /* Previously blocked */
    if(pUmDevice->tx_full){
	printf("eth%d: tx blocked.\n", pUmDevice->index);
	return 0;
    }

    pPacket = (PLM_PACKET)
	QQ_PopHead(&pDevice->TxPacketFreeQ.Container);

    if (pPacket == 0) {
	pUmDevice->tx_full = 1;
	printf("bcm570xEndSend: TX full!\n");
	return 0;
    }

    if (pDevice->SendBdLeft.counter == 0) {
	pUmDevice->tx_full = 1;
	printf("bcm570xEndSend: no more TX descriptors!\n");
	QQ_PushHead(&pDevice->TxPacketFreeQ.Container, pPacket);
	return 0;
    }

    if (length <= 0){
	printf("eth: bad packet size: %d\n", length);
	goto out;
    }

    /* Get packet buffers and fragment list */
    pUmPacket = (PUM_PACKET) pPacket;
    /* Single DMA Descriptor transmit.
     * Fragments may be provided, but one DMA descriptor max is
     * used to send the packet.
     */
    if (MM_CoalesceTxBuffer (pDevice, pPacket) != LM_STATUS_SUCCESS) {
	if (pUmPacket->skbuff == NULL){
	    /* Packet was discarded */
	    printf("TX: failed (1)\n");
	    status = 1;
	} else{
	    printf("TX: failed (2)\n");
	    status = 2;
	}
	QQ_PushHead (&pDevice->TxPacketFreeQ.Container, pPacket);
	return status;
    }

    /* Copy packet to DMA buffer */
    memset(pUmPacket->skbuff, 0x0, MAX_PACKET_SIZE);
    memcpy((void*)pUmPacket->skbuff, (void*)packet, length);
    pPacket->PacketSize = length;
    pPacket->Flags |= SND_BD_FLAG_END|SND_BD_FLAG_COAL_NOW;
    pPacket->u.Tx.FragCount = 1;
    /* We've already provided a frame ready for transmission */
    pPacket->Flags &= ~SND_BD_FLAG_TCP_UDP_CKSUM;

    if ( LM_SendPacket(pDevice, pPacket) == LM_STATUS_FAILURE){
	/*
	 *  A lower level send failure will push the packet descriptor back
	 *  in the free queue, so just deal with the VxWorks clusters.
	 */
	if (pUmPacket->skbuff == NULL){
	    printf("TX failed (1)!\n");
	    /* Packet was discarded */
	    status = 3;
	} else {
	    /* A resource problem ... */
	    printf("TX failed (2)!\n");
	    status = 4;
	}

	if (QQ_GetEntryCnt(&pDevice->TxPacketFreeQ.Container) == 0) {
	    printf("TX: emptyQ!\n");
	    pUmDevice->tx_full = 1;
	}
    }

    while(pUmDevice->tx_pkt == 0){
	/* Service TX */
	eth_isr();
    }
#if ET_DEBUG
    printf("eth_send: 0x%x, %d bytes\n"
	   "[%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x] ...\n",
	   (int)pPacket, length,
	   ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],
	   ptr[6],ptr[7],ptr[8],ptr[9],ptr[10],ptr[11],ptr[12],
	   ptr[13],ptr[14],ptr[15]);
#endif
    pUmDevice->tx_pkt = 0;
    QQ_PushHead(&pDevice->TxPacketFreeQ.Container, pPacket);

    /* Done with send */
 out:
    return status;
}


/* Ethernet receive */
int
eth_rx(void)
{
    PLM_PACKET          pPacket = NULL;
    PUM_PACKET          pUmPacket = NULL;
    void *skb;
    int size=0;

    while(TRUE) {

    bcm570x_service_isr:
	/* Pull down packet if it is there */
	eth_isr();

	/* Indicate RX packets called */
	if(pUmDevice->rx_pkt){
	    /* printf("eth_rx: got a packet...\n"); */
	    pUmDevice->rx_pkt = 0;
	} else {
	    /* printf("eth_rx: waiting for packet...\n"); */
	    goto bcm570x_service_isr;
	}

	pPacket = (PLM_PACKET)
	    QQ_PopHead(&pDevice->RxPacketReceivedQ.Container);

	if (pPacket == 0){
	    printf("eth_rx: empty packet!\n");
	    goto bcm570x_service_isr;
	}

	pUmPacket = (PUM_PACKET) pPacket;
#if ET_DEBUG
	printf("eth_rx: packet @0x%x\n",
	       (int)pPacket);
#endif
	/* If the packet generated an error, reuse buffer */
	if ((pPacket->PacketStatus != LM_STATUS_SUCCESS) ||
	    ((size = pPacket->PacketSize) > pDevice->RxMtu)) {

	    /* reuse skb */
	    QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
	    printf("eth_rx: error in packet dma!\n");
	    goto bcm570x_service_isr;
	}

	/* Set size and address */
	skb = pUmPacket->skbuff;
	size = pPacket->PacketSize;

	/* Pass the packet up to the protocol
	 * layers.
	 */
	NetReceive(skb, size);

	/* Free packet buffer */
	bcm570xPktFree (pUmDevice->index, skb);
	pUmPacket->skbuff = NULL;

	/* Reuse SKB */
	QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);

	return 0; /* Got a packet, bail ... */
    }
    return size;
}


/* Shut down device */
void
eth_halt(void)
{
    int i;
    if ( initialized)
    if (pDevice && pUmDevice && pUmDevice->opened){
	printf("\neth%d:%s,", pUmDevice->index, pUmDevice->name);
	printf("HALT,");
	/* stop device */
	LM_Halt(pDevice);
	printf("POWER DOWN,");
	LM_SetPowerState(pDevice, LM_POWER_STATE_D3);

	/* Free the memory allocated by the device in tigon3 */
	for (i = 0; i < pUmDevice->mem_list_num; i++)  {
	    if (pUmDevice->mem_list[i])  {
		/* sanity check */
		if (pUmDevice->dma_list[i]) {  /* cache-safe memory */
		    free(pUmDevice->mem_list[i]);
		} else {
		    free(pUmDevice->mem_list[i]);  /* normal memory   */
		}
	    }
	}
	pUmDevice->opened = 0;
	free(pDevice);
	pDevice = NULL;
	pUmDevice = NULL;
	initialized = 0;
	printf("done - offline.\n");
    }
}


/*
 *
 * Middle Module: Interface between the HW driver (tigon3 modules) and
 * the native (SENS) driver.  These routines implement the system
 * interface for tigon3 on VxWorks.
 */

/* Middle module dependency - size of a packet descriptor */
int MM_Packet_Desc_Size = sizeof(UM_PACKET);


LM_STATUS
MM_ReadConfig32(PLM_DEVICE_BLOCK pDevice,
		LM_UINT32 Offset,
		LM_UINT32 *pValue32)
{
    UM_DEVICE_BLOCK *pUmDevice;
    pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
    pci_read_config_dword(pUmDevice->pdev,
			  Offset, (u32 *) pValue32);
    return LM_STATUS_SUCCESS;
}


LM_STATUS
MM_WriteConfig32(PLM_DEVICE_BLOCK pDevice,
		 LM_UINT32 Offset,
		 LM_UINT32 Value32)
{
    UM_DEVICE_BLOCK *pUmDevice;
    pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
    pci_write_config_dword(pUmDevice->pdev,
			   Offset, Value32);
    return LM_STATUS_SUCCESS;
}


LM_STATUS
MM_ReadConfig16(PLM_DEVICE_BLOCK pDevice,
		LM_UINT32 Offset,
		LM_UINT16 *pValue16)
{
    UM_DEVICE_BLOCK *pUmDevice;
    pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
    pci_read_config_word(pUmDevice->pdev,
			 Offset, (u16*) pValue16);
    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_WriteConfig16(PLM_DEVICE_BLOCK pDevice,
		 LM_UINT32 Offset,
		 LM_UINT16 Value16)
{
    UM_DEVICE_BLOCK *pUmDevice;
    pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
    pci_write_config_word(pUmDevice->pdev,
			  Offset, Value16);
    return LM_STATUS_SUCCESS;
}


LM_STATUS
MM_AllocateSharedMemory(PLM_DEVICE_BLOCK pDevice, LM_UINT32 BlockSize,
			PLM_VOID *pMemoryBlockVirt,
			PLM_PHYSICAL_ADDRESS pMemoryBlockPhy,
			LM_BOOL Cached)
{
    PLM_VOID pvirt;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    dma_addr_t mapping;

    pvirt = malloc(BlockSize);
    mapping = (dma_addr_t)(pvirt);
    if (!pvirt)
	return LM_STATUS_FAILURE;

    pUmDevice->mem_list[pUmDevice->mem_list_num] = pvirt;
    pUmDevice->dma_list[pUmDevice->mem_list_num] = mapping;
    pUmDevice->mem_size_list[pUmDevice->mem_list_num++] = BlockSize;
    memset(pvirt, 0, BlockSize);

    *pMemoryBlockVirt = (PLM_VOID) pvirt;
    MM_SetAddr (pMemoryBlockPhy, (dma_addr_t) mapping);

    return LM_STATUS_SUCCESS;
}


LM_STATUS
MM_AllocateMemory(PLM_DEVICE_BLOCK pDevice, LM_UINT32 BlockSize,
	PLM_VOID *pMemoryBlockVirt)
{
    PLM_VOID pvirt;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;

    pvirt = malloc(BlockSize);

    if (!pvirt)
	return LM_STATUS_FAILURE;

    pUmDevice->mem_list[pUmDevice->mem_list_num] = pvirt;
    pUmDevice->dma_list[pUmDevice->mem_list_num] = 0;
    pUmDevice->mem_size_list[pUmDevice->mem_list_num++] = BlockSize;
    memset(pvirt, 0, BlockSize);
    *pMemoryBlockVirt = pvirt;

    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_MapMemBase(PLM_DEVICE_BLOCK pDevice)
{
    printf("BCM570x PCI Memory base address @0x%x\n",
	   (unsigned int)pDevice->pMappedMemBase);
    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_InitializeUmPackets(PLM_DEVICE_BLOCK pDevice)
{
    int i;
    void* skb;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    PUM_PACKET pUmPacket = NULL;
    PLM_PACKET pPacket = NULL;

    for (i = 0; i < pDevice->RxPacketDescCnt; i++) {
	pPacket = QQ_PopHead(&pDevice->RxPacketFreeQ.Container);
	pUmPacket = (PUM_PACKET) pPacket;

	if (pPacket == 0) {
	    printf("MM_InitializeUmPackets: Bad RxPacketFreeQ\n");
	}

	skb = bcm570xPktAlloc(pUmDevice->index,
			      pPacket->u.Rx.RxBufferSize + 2);

	if (skb == 0) {
	    pUmPacket->skbuff = 0;
	    QQ_PushTail(&pUmDevice->rx_out_of_buf_q.Container, pPacket);
	    printf("MM_InitializeUmPackets: out of buffer.\n");
	    continue;
	}

	pUmPacket->skbuff = skb;
	QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
    }

    pUmDevice->rx_low_buf_thresh = pDevice->RxPacketDescCnt / 8;

    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_GetConfig(PLM_DEVICE_BLOCK pDevice)
{
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    int index = pDevice->index;

    if (auto_speed[index] == 0)
	pDevice->DisableAutoNeg = TRUE;
    else
	pDevice->DisableAutoNeg = FALSE;

    if (line_speed[index] == 0) {
	pDevice->RequestedMediaType =
	    LM_REQUESTED_MEDIA_TYPE_AUTO;
	pDevice->DisableAutoNeg = FALSE;
    }
    else {
	if (line_speed[index] == 1000) {
	    if (pDevice->EnableTbi) {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_FIBER_1000MBPS_FULL_DUPLEX;
	    }
	    else if (full_duplex[index]) {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_1000MBPS_FULL_DUPLEX;
	    }
	    else {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_1000MBPS;
	    }
	    if (!pDevice->EnableTbi)
		pDevice->DisableAutoNeg = FALSE;
	}
	else if (line_speed[index] == 100) {
	    if (full_duplex[index]) {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_100MBPS_FULL_DUPLEX;
	    }
	    else {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_100MBPS;
	    }
	}
	else if (line_speed[index] == 10) {
	    if (full_duplex[index]) {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_10MBPS_FULL_DUPLEX;
	    }
	    else {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_10MBPS;
	    }
	}
	else {
	    pDevice->RequestedMediaType =
		LM_REQUESTED_MEDIA_TYPE_AUTO;
	    pDevice->DisableAutoNeg = FALSE;
	}

    }
    pDevice->FlowControlCap = 0;
    if (rx_flow_control[index] != 0) {
	pDevice->FlowControlCap |= LM_FLOW_CONTROL_RECEIVE_PAUSE;
    }
    if (tx_flow_control[index] != 0) {
	pDevice->FlowControlCap |= LM_FLOW_CONTROL_TRANSMIT_PAUSE;
    }
    if ((auto_flow_control[index] != 0) &&
	(pDevice->DisableAutoNeg == FALSE)) {

	pDevice->FlowControlCap |= LM_FLOW_CONTROL_AUTO_PAUSE;
	if ((tx_flow_control[index] == 0) &&
	    (rx_flow_control[index] == 0)) {
	    pDevice->FlowControlCap |=
		LM_FLOW_CONTROL_TRANSMIT_PAUSE |
		LM_FLOW_CONTROL_RECEIVE_PAUSE;
	}
    }

    /* Default MTU for now */
    pUmDevice->mtu = 1500;

#if T3_JUMBO_RCV_RCB_ENTRY_COUNT
    if (pUmDevice->mtu > 1500) {
	pDevice->RxMtu = pUmDevice->mtu;
	pDevice->RxJumboDescCnt = DEFAULT_JUMBO_RCV_DESC_COUNT;
    }
    else {
	pDevice->RxJumboDescCnt = 0;
    }
    pDevice->RxJumboDescCnt = rx_jumbo_desc_cnt[index];
#else
    pDevice->RxMtu = pUmDevice->mtu;
#endif

    if (T3_ASIC_REV(pDevice->ChipRevId) == T3_ASIC_REV_5701) {
	pDevice->UseTaggedStatus = TRUE;
	pUmDevice->timer_interval = CFG_HZ;
    }
    else {
	pUmDevice->timer_interval = CFG_HZ/50;
    }

    pDevice->TxPacketDescCnt = tx_pkt_desc_cnt[index];
    pDevice->RxStdDescCnt = rx_std_desc_cnt[index];
    /* Note:  adaptive coalescence really isn't adaptive in this driver */
    pUmDevice->rx_adaptive_coalesce = rx_adaptive_coalesce[index];
    if (!pUmDevice->rx_adaptive_coalesce) {
	pDevice->RxCoalescingTicks = rx_coalesce_ticks[index];
	if (pDevice->RxCoalescingTicks > MAX_RX_COALESCING_TICKS)
	    pDevice->RxCoalescingTicks = MAX_RX_COALESCING_TICKS;
	pUmDevice->rx_curr_coalesce_ticks =pDevice->RxCoalescingTicks;

	pDevice->RxMaxCoalescedFrames = rx_max_coalesce_frames[index];
	if (pDevice->RxMaxCoalescedFrames>MAX_RX_MAX_COALESCED_FRAMES)
	    pDevice->RxMaxCoalescedFrames =
				MAX_RX_MAX_COALESCED_FRAMES;
	pUmDevice->rx_curr_coalesce_frames =
	    pDevice->RxMaxCoalescedFrames;
	pDevice->StatsCoalescingTicks = stats_coalesce_ticks[index];
	if (pDevice->StatsCoalescingTicks>MAX_STATS_COALESCING_TICKS)
	    pDevice->StatsCoalescingTicks=
		MAX_STATS_COALESCING_TICKS;
	}
	else {
	    pUmDevice->rx_curr_coalesce_frames =
		DEFAULT_RX_MAX_COALESCED_FRAMES;
	    pUmDevice->rx_curr_coalesce_ticks =
		DEFAULT_RX_COALESCING_TICKS;
	}
    pDevice->TxCoalescingTicks = tx_coalesce_ticks[index];
    if (pDevice->TxCoalescingTicks > MAX_TX_COALESCING_TICKS)
	pDevice->TxCoalescingTicks = MAX_TX_COALESCING_TICKS;
    pDevice->TxMaxCoalescedFrames = tx_max_coalesce_frames[index];
    if (pDevice->TxMaxCoalescedFrames > MAX_TX_MAX_COALESCED_FRAMES)
	pDevice->TxMaxCoalescedFrames = MAX_TX_MAX_COALESCED_FRAMES;

    if (enable_wol[index]) {
	pDevice->WakeUpModeCap = LM_WAKE_UP_MODE_MAGIC_PACKET;
	pDevice->WakeUpMode = LM_WAKE_UP_MODE_MAGIC_PACKET;
    }
    pDevice->NicSendBd = TRUE;

    /* Don't update status blocks during interrupt */
    pDevice->RxCoalescingTicksDuringInt = 0;
    pDevice->TxCoalescingTicksDuringInt = 0;

    return LM_STATUS_SUCCESS;

}


LM_STATUS
MM_StartTxDma(PLM_DEVICE_BLOCK pDevice, PLM_PACKET pPacket)
{
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    printf("Start TX DMA: dev=%d packet @0x%x\n",
	   (int)pUmDevice->index, (unsigned int)pPacket);

    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_CompleteTxDma(PLM_DEVICE_BLOCK pDevice, PLM_PACKET pPacket)
{
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    printf("Complete TX DMA: dev=%d packet @0x%x\n",
	   (int)pUmDevice->index, (unsigned int)pPacket);
    return LM_STATUS_SUCCESS;
}


LM_STATUS
MM_IndicateStatus(PLM_DEVICE_BLOCK pDevice, LM_STATUS Status)
{
    char buf[128];
    char lcd[4];
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    LM_FLOW_CONTROL flow_control;

    pUmDevice->delayed_link_ind = 0;
    memset(lcd, 0x0, 4);

    if (Status == LM_STATUS_LINK_DOWN) {
	sprintf(buf,"eth%d: %s: NIC Link is down\n",
		pUmDevice->index,pUmDevice->name);
	lcd[0] = 'L';lcd[1]='N';lcd[2]='K';lcd[3] = '?';
    } else if (Status == LM_STATUS_LINK_ACTIVE) {
	sprintf(buf,"eth%d:%s: ", pUmDevice->index, pUmDevice->name);

	if (pDevice->LineSpeed == LM_LINE_SPEED_1000MBPS){
	    strcat(buf,"1000 Mbps ");
	    lcd[0] = '1';lcd[1]='G';lcd[2]='B';
	} else if (pDevice->LineSpeed == LM_LINE_SPEED_100MBPS){
	    strcat(buf,"100 Mbps ");
	    lcd[0] = '1';lcd[1]='0';lcd[2]='0';
	} else if (pDevice->LineSpeed == LM_LINE_SPEED_10MBPS){
	    strcat(buf,"10 Mbps ");
	    lcd[0] = '1';lcd[1]='0';lcd[2]=' ';
	}
	if (pDevice->DuplexMode == LM_DUPLEX_MODE_FULL){
	    strcat(buf, "full duplex");
	    lcd[3] = 'F';
	} else {
	    strcat(buf, "half duplex");
	    lcd[3] = 'H';
	}
	strcat(buf, " link up");

	flow_control = pDevice->FlowControl &
	    (LM_FLOW_CONTROL_RECEIVE_PAUSE |
	     LM_FLOW_CONTROL_TRANSMIT_PAUSE);

	if (flow_control) {
	    if (flow_control & LM_FLOW_CONTROL_RECEIVE_PAUSE) {
		strcat(buf,", receive ");
		if (flow_control & LM_FLOW_CONTROL_TRANSMIT_PAUSE)
		    strcat(buf," & transmit ");
	    }
	    else {
		strcat(buf,", transmit ");
	    }
	    strcat(buf,"flow control ON");
	} else {
	    strcat(buf, ", flow control OFF");
	}
	strcat(buf,"\n");
	printf("%s",buf);
    }
#if 0
    sysLedDsply(lcd[0],lcd[1],lcd[2],lcd[3]);
#endif
    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_FreeRxBuffer(PLM_DEVICE_BLOCK pDevice, PLM_PACKET pPacket)
{

    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    PUM_PACKET pUmPacket;
    void *skb;

    pUmPacket = (PUM_PACKET) pPacket;

    if ((skb = pUmPacket->skbuff))
	bcm570xPktFree(pUmDevice->index, skb);

    pUmPacket->skbuff = 0;

    return LM_STATUS_SUCCESS;
}

unsigned long
MM_AnGetCurrentTime_us(PAN_STATE_INFO pAnInfo)
{
    return get_timer(0);
}

/*
 *   Transform an MBUF chain into a single MBUF.
 *   This routine will fail if the amount of data in the
 *   chain overflows a transmit buffer.  In that case,
 *   the incoming MBUF chain will be freed.  This routine can
 *   also fail by not being able to allocate a new MBUF (including
 *   cluster and mbuf headers).  In that case the failure is
 *   non-fatal.  The incoming cluster chain is not freed, giving
 *   the caller the choice of whether to try a retransmit later.
 */
LM_STATUS
MM_CoalesceTxBuffer(PLM_DEVICE_BLOCK pDevice, PLM_PACKET pPacket)
{
    PUM_PACKET pUmPacket = (PUM_PACKET) pPacket;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    void *skbnew;
    int len = 0;

    if (len == 0)
	return (LM_STATUS_SUCCESS);

    if (len > MAX_PACKET_SIZE){
	printf ("eth%d: xmit frame discarded, too big!, size = %d\n",
		pUmDevice->index, len);
	return (LM_STATUS_FAILURE);
    }

    skbnew = bcm570xPktAlloc(pUmDevice->index, MAX_PACKET_SIZE);

    if (skbnew == NULL) {
	pUmDevice->tx_full = 1;
	printf ("eth%d: out of transmit buffers", pUmDevice->index);
	return (LM_STATUS_FAILURE);
    }

    /* New packet values */
    pUmPacket->skbuff = skbnew;
    pUmPacket->lm_packet.u.Tx.FragCount = 1;

    return (LM_STATUS_SUCCESS);
}


LM_STATUS
MM_IndicateRxPackets(PLM_DEVICE_BLOCK pDevice)
{
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    pUmDevice->rx_pkt = 1;
    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_IndicateTxPackets(PLM_DEVICE_BLOCK pDevice)
{
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    PLM_PACKET pPacket;
    PUM_PACKET pUmPacket;
    void *skb;
    while ( TRUE ) {

	pPacket = (PLM_PACKET)
	    QQ_PopHead(&pDevice->TxPacketXmittedQ.Container);

	if (pPacket == 0)
	    break;

	pUmPacket = (PUM_PACKET) pPacket;
	skb = (void*)pUmPacket->skbuff;

	/*
	* Free MBLK if we transmitted a fragmented packet or a
	* non-fragmented packet straight from the VxWorks
	* buffer pool. If packet was copied to a local transmit
	* buffer, then there's no MBUF to free, just free
	* the transmit buffer back to the cluster pool.
	*/

	if (skb)
	    bcm570xPktFree (pUmDevice->index, skb);

	pUmPacket->skbuff = 0;
	QQ_PushTail(&pDevice->TxPacketFreeQ.Container, pPacket);
	pUmDevice->tx_pkt = 1;
    }
    if (pUmDevice->tx_full) {
	if (QQ_GetEntryCnt(&pDevice->TxPacketFreeQ.Container) >=
	    (QQ_GetSize(&pDevice->TxPacketFreeQ.Container) >> 1))
	    pUmDevice->tx_full = 0;
    }
    return LM_STATUS_SUCCESS;
}

/*
 *  Scan an MBUF chain until we reach fragment number "frag"
 *  Return its length and physical address.
 */
void MM_MapTxDma
    (
    PLM_DEVICE_BLOCK pDevice,
    struct _LM_PACKET *pPacket,
    T3_64BIT_HOST_ADDR *paddr,
    LM_UINT32 *len,
    int frag)
{
    PUM_PACKET pUmPacket = (PUM_PACKET) pPacket;
    *len = pPacket->PacketSize;
    MM_SetT3Addr(paddr, (dma_addr_t) pUmPacket->skbuff);
}

/*
 *  Convert an mbuf address, a CPU local virtual address,
 *  to a physical address as seen from a PCI device.  Store the
 *  result at paddr.
 */
void MM_MapRxDma(
		 PLM_DEVICE_BLOCK pDevice,
		 struct _LM_PACKET *pPacket,
		 T3_64BIT_HOST_ADDR *paddr)
{
    PUM_PACKET pUmPacket = (PUM_PACKET) pPacket;
    MM_SetT3Addr(paddr, (dma_addr_t) pUmPacket->skbuff);
}

void
MM_SetAddr (LM_PHYSICAL_ADDRESS *paddr, dma_addr_t addr)
{
#if (BITS_PER_LONG == 64)
	paddr->High = ((unsigned long) addr) >> 32;
	paddr->Low = ((unsigned long) addr) & 0xffffffff;
#else
	paddr->High = 0;
	paddr->Low = (unsigned long) addr;
#endif
}

void
MM_SetT3Addr(T3_64BIT_HOST_ADDR *paddr, dma_addr_t addr)
{
	unsigned long baddr = (unsigned long) addr;
#if (BITS_PER_LONG == 64)
	set_64bit_addr(paddr, baddr & 0xffffffff, baddr >> 32);
#else
	set_64bit_addr(paddr, baddr, 0);
#endif
}

/*
 * This combination of `inline' and `extern' has almost the effect of a
 * macro.  The way to use it is to put a function definition in a header
 * file with these keywords, and put another copy of the definition
 * (lacking `inline' and `extern') in a library file.  The definition in
 * the header file will cause most calls to the function to be inlined.
 * If any uses of the function remain, they will refer to the single copy
 * in the library.
 */
void
atomic_set(atomic_t* entry, int val)
{
    entry->counter = val;
}
int
atomic_read(atomic_t* entry)
{
    return entry->counter;
}
void
atomic_inc(atomic_t* entry)
{
    if(entry)
	entry->counter++;
}

void
atomic_dec(atomic_t* entry)
{
    if(entry)
	entry->counter--;
}

void
atomic_sub(int a, atomic_t* entry)
{
    if(entry)
	entry->counter -= a;
}

void
atomic_add(int a, atomic_t* entry)
{
    if(entry)
	entry->counter += a;
}

/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
void
QQ_InitQueue(
PQQ_CONTAINER pQueue,
unsigned int QueueSize) {
    pQueue->Head = 0;
    pQueue->Tail = 0;
    pQueue->Size = QueueSize+1;
    atomic_set(&pQueue->EntryCnt, 0);
} /* QQ_InitQueue */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
char
QQ_Full(
PQQ_CONTAINER pQueue) {
    unsigned int NewHead;

    NewHead = (pQueue->Head + 1) % pQueue->Size;

    return(NewHead == pQueue->Tail);
} /* QQ_Full */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
char
QQ_Empty(
PQQ_CONTAINER pQueue) {
    return(pQueue->Head == pQueue->Tail);
} /* QQ_Empty */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
unsigned int
QQ_GetSize(
PQQ_CONTAINER pQueue) {
    return pQueue->Size;
} /* QQ_GetSize */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
unsigned int
QQ_GetEntryCnt(
PQQ_CONTAINER pQueue) {
    return atomic_read(&pQueue->EntryCnt);
} /* QQ_GetEntryCnt */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/*    TRUE entry was added successfully.                                      */
/*    FALSE queue is full.                                                    */
/******************************************************************************/
char
QQ_PushHead(
PQQ_CONTAINER pQueue,
PQQ_ENTRY pEntry) {
    unsigned int Head;

    Head = (pQueue->Head + 1) % pQueue->Size;

#if !defined(QQ_NO_OVERFLOW_CHECK)
    if(Head == pQueue->Tail) {
	return 0;
    } /* if */
#endif /* QQ_NO_OVERFLOW_CHECK */

    pQueue->Array[pQueue->Head] = pEntry;
    wmb();
    pQueue->Head = Head;
    atomic_inc(&pQueue->EntryCnt);

    return -1;
} /* QQ_PushHead */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/*    TRUE entry was added successfully.                                      */
/*    FALSE queue is full.                                                    */
/******************************************************************************/
char
QQ_PushTail(
PQQ_CONTAINER pQueue,
PQQ_ENTRY pEntry) {
    unsigned int Tail;

    Tail = pQueue->Tail;
    if(Tail == 0) {
	Tail = pQueue->Size;
    } /* if */
    Tail--;

#if !defined(QQ_NO_OVERFLOW_CHECK)
    if(Tail == pQueue->Head) {
	return 0;
    } /* if */
#endif /* QQ_NO_OVERFLOW_CHECK */

    pQueue->Array[Tail] = pEntry;
    wmb();
    pQueue->Tail = Tail;
    atomic_inc(&pQueue->EntryCnt);

    return -1;
} /* QQ_PushTail */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
PQQ_ENTRY
QQ_PopHead(
PQQ_CONTAINER pQueue) {
    unsigned int Head;
    PQQ_ENTRY Entry;

    Head = pQueue->Head;

#if !defined(QQ_NO_UNDERFLOW_CHECK)
    if(Head == pQueue->Tail) {
	return (PQQ_ENTRY) 0;
    } /* if */
#endif /* QQ_NO_UNDERFLOW_CHECK */

    if(Head == 0) {
	Head = pQueue->Size;
    } /* if */
    Head--;

    Entry = pQueue->Array[Head];
    membar();

    pQueue->Head = Head;
    atomic_dec(&pQueue->EntryCnt);

    return Entry;
} /* QQ_PopHead */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
PQQ_ENTRY
QQ_PopTail(
PQQ_CONTAINER pQueue) {
    unsigned int Tail;
    PQQ_ENTRY Entry;

    Tail = pQueue->Tail;

#if !defined(QQ_NO_UNDERFLOW_CHECK)
    if(Tail == pQueue->Head) {
	return (PQQ_ENTRY) 0;
    } /* if */
#endif /* QQ_NO_UNDERFLOW_CHECK */

    Entry = pQueue->Array[Tail];
    membar();
    pQueue->Tail = (Tail + 1) % pQueue->Size;
    atomic_dec(&pQueue->EntryCnt);

    return Entry;
} /* QQ_PopTail */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
PQQ_ENTRY
QQ_GetHead(
    PQQ_CONTAINER pQueue,
    unsigned int Idx)
{
    if(Idx >= atomic_read(&pQueue->EntryCnt))
    {
	return (PQQ_ENTRY) 0;
    }

    if(pQueue->Head > Idx)
    {
	Idx = pQueue->Head - Idx;
    }
    else
    {
	Idx = pQueue->Size - (Idx - pQueue->Head);
    }
    Idx--;

    return pQueue->Array[Idx];
}


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
PQQ_ENTRY
QQ_GetTail(
    PQQ_CONTAINER pQueue,
    unsigned int Idx)
{
    if(Idx >= atomic_read(&pQueue->EntryCnt))
    {
	return (PQQ_ENTRY) 0;
    }

    Idx += pQueue->Tail;
    if(Idx >= pQueue->Size)
    {
	Idx = Idx - pQueue->Size;
    }

    return pQueue->Array[Idx];
}

#endif	/* CFG_CMD_NET, !CONFIG_NET_MULTI, CONFIG_BCM570x */
