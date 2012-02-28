/*
 */

#include <common.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>

#include "xemacpss.h"

/************************ Forward function declaration **********************/

static int Xgmac_process_rx(XEmacPss * EmacPssInstancePtr);
static int Xgmac_init_rxq(XEmacPss * EmacPssInstancePtr, void *bd_start, int num_elem);
static int Xgmac_make_rxbuff_mem(XEmacPss * EmacPssInstancePtr, void *rx_buf_start,
			  u32 rx_buffsize);
static int Xgmac_next_rx_buf(XEmacPss * EmacPssInstancePtr);
static int Xgmac_phy_mgmt_idle(XEmacPss * EmacPssInstancePtr);

static void Xgmac_set_eth_advertise(XEmacPss * EmacPssInstancePtr, int link_speed);

/*************************** Constant Definitions ***************************/

#define EMACPSS_DEVICE_ID   0
#define RXBD_CNT       8	/* Number of RxBDs to use */
#define TXBD_CNT       8	/* Number of TxBDs to use */

#define phy_spinwait(e) do { while (!Xgmac_phy_mgmt_idle(e)); } while (0)

#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")

/*************************** Variable Definitions ***************************/

/*
 * Aligned memory segments to be used for buffer descriptors
 */
//#define BRAM_BUFFERS
#ifdef BRAM_BUFFERS
static XEmacPss_Bd RxBdSpace[RXBD_CNT] __attribute__ ((section (".bram_buffers")));
static XEmacPss_Bd TxBdSpace[TXBD_CNT] __attribute__ ((section (".bram_buffers")));
static char RxBuffers[RXBD_CNT * XEMACPSS_RX_BUF_SIZE] __attribute__ ((section (".bram_buffers")));
static uchar data_buffer[XEMACPSS_RX_BUF_SIZE] __attribute__ ((section (".bram_buffers")));
#else
static XEmacPss_Bd RxBdSpace[RXBD_CNT];
static XEmacPss_Bd TxBdSpace[TXBD_CNT];
static char RxBuffers[RXBD_CNT * XEMACPSS_RX_BUF_SIZE];
static uchar data_buffer[XEMACPSS_RX_BUF_SIZE];
#endif

static struct {
	u8 initialized;
} ethstate = {0};

XEmacPss EmacPssInstance;

/*****************************************************************************/
/*
*	Following are the supporting functions to read and write GEM PHY registers.
*/
int Xgmac_phy_mgmt_idle(XEmacPss * EmacPssInstancePtr)
{
	return ((XEmacPss_ReadReg
		 (EmacPssInstancePtr->Config.BaseAddress, XEMACPSS_NWSR_OFFSET)
		 & XEMACPSS_NWSR_MDIOIDLE_MASK) == XEMACPSS_NWSR_MDIOIDLE_MASK);
}

#if defined(CONFIG_CMD_MII) && !defined(CONFIG_BITBANGMII)
static int Xgmac_mii_read(const char *devname, unsigned char addr,
		unsigned char reg, unsigned short *value)
{
	phy_spinwait(&EmacPssInstance);
	XEmacPss_PhyRead(&EmacPssInstance, addr, reg, value);
	phy_spinwait(&EmacPssInstance);
	return 0;
}

static int Xgmac_mii_write(const char *devname, unsigned char addr,
		unsigned char reg, unsigned short value)
{
	phy_spinwait(&EmacPssInstance);
	XEmacPss_PhyWrite(&EmacPssInstance, addr, reg, value);
	phy_spinwait(&EmacPssInstance);
	return 0;
}
#endif

static u32 phy_rd(XEmacPss * e, u32 a)
{
	u16 PhyData;

	phy_spinwait(e);
	XEmacPss_PhyRead(e, CONFIG_XGMAC_PHY_ADDR, a, &PhyData);
	phy_spinwait(e);
	return PhyData;
}

static void phy_wr(XEmacPss * e, u32 a, u32 v)
{
	phy_spinwait(e);
	XEmacPss_PhyWrite(e, CONFIG_XGMAC_PHY_ADDR, a, v);
	phy_spinwait(e);
}

static void phy_rst(XEmacPss * e)
{
	int tmp;

	puts("Resetting PHY...\n");
	tmp = phy_rd(e, 0);
	tmp |= 0x8000;
	phy_wr(e, 0, tmp);

	while (phy_rd(e, 0) & 0x8000) {
		udelay(10000);
		tmp++;
		if (tmp > 1000) { /* stalled if reset unfinished after 10 seconds */
			puts("***Error: Reset stalled...\n");
			return;
		}
	}
	puts("\nPHY reset complete.\n");
}

static void Out32(u32 OutAddress, u32 Value)
{
	*(volatile u32 *) OutAddress = Value;
	dmb();
}

/*****************************************************************************/

int Xgmac_one_time_init(void)
{
	int tmp;
	int Status;
	XEmacPss_Config *Config;
	XEmacPss *EmacPssInstancePtr = &EmacPssInstance;
	XEmacPss_Bd BdTemplate;

	Config = XEmacPss_LookupConfig(EMACPSS_DEVICE_ID);

	Status =
	    XEmacPss_CfgInitialize(EmacPssInstancePtr, Config,
				   Config->BaseAddress);
	if (Status != 0) {
		puts("Error in initialize");
		return 0;
	}

	/*
	 * Setup RxBD space.
	 */

	if (Xgmac_init_rxq(EmacPssInstancePtr, &RxBdSpace, RXBD_CNT)) {
		puts("Xgmac_init_rxq failed!\n");
		return -1;
	}

	/*
	 * Create the RxBD ring
	 */
	tmp =
	    Xgmac_make_rxbuff_mem(EmacPssInstancePtr, &RxBuffers,
				  sizeof(RxBuffers));
	if (tmp == 0 || tmp == -1) {
		printf("Xgmac_make_rxbuff_mem failed! (%i)\n", tmp);
		return -1;
	}

	/*
	 * Setup TxBD space.
	 */

	XEmacPss_BdClear(&BdTemplate);
	XEmacPss_BdSetStatus(&BdTemplate, XEMACPSS_TXBUF_USED_MASK);

	/*
	 * Create the TxBD ring
	 */
	Status =
	    XEmacPss_BdRingCreate(&(XEmacPss_GetTxRing(EmacPssInstancePtr)),
				  (u32) & TxBdSpace, (u32) & TxBdSpace,
				  XEMACPSS_BD_ALIGNMENT, TXBD_CNT);
	if (Status != 0) {
		puts("Error setting up TxBD space, BdRingCreate");
		return -1;
	}

	Status = XEmacPss_BdRingClone(&(XEmacPss_GetTxRing(EmacPssInstancePtr)),
				      &BdTemplate, XEMACPSS_SEND);
	if (Status != 0) {
		puts("Error setting up TxBD space, BdRingClone");
		return -1;
	}

	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_TXQBASE_OFFSET,
			  EmacPssInstancePtr->TxBdRing.BaseBdAddr);

	return 0;
}

int Xgmac_init(struct eth_device *dev, bd_t * bis)
{
	int tmp;
	int link_speed;
	XEmacPss *EmacPssInstancePtr = &EmacPssInstance;

	if (ethstate.initialized)
		return 1;

	/*
	 * Setup the ethernet.
	 */
	printf("Trying to set up GEM link...\n");

	/*************************** MAC Setup ***************************/
	tmp = (2 << 18);	/* MDC clock division (32 for up to 80MHz) */
	tmp |= (1 << 17);	/* set for FCS removal */
	tmp |= (1 << 10);	/* enable gigabit */
	tmp |= (1 << 4);	/* copy all frames */
	tmp |= (1 << 1);	/* enable full duplex */

	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET, tmp);

	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_DMACR_OFFSET, 0x00180704);

	/* Disable all the MAC Interrupts */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_IDR_OFFSET, 0xFFFFFFFF);

	/* MDIO, Rx and Tx enable */
	tmp =
	    XEmacPss_ReadReg(EmacPssInstancePtr->Config.BaseAddress,
			     XEMACPSS_NWCTRL_OFFSET);
	tmp |=
	    XEMACPSS_NWCTRL_MDEN_MASK | XEMACPSS_NWCTRL_RXEN_MASK |
	    XEMACPSS_NWCTRL_TXEN_MASK;
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCTRL_OFFSET, tmp);

	/*************************** PHY Setup ***************************/

	phy_wr(EmacPssInstancePtr, 22, 0);	/* page 0 */

	/* Auto-negotiation advertisement register */
	tmp = phy_rd(EmacPssInstancePtr, 4);
	tmp |= (1 << 11);	/* asymmetric pause */
	tmp |= (1 << 10);	/* MAC pause implemented */
	phy_wr(EmacPssInstancePtr, 4, tmp);

#ifdef CONFIG_EP107
	/* Extended PHY specific control register */
	tmp = phy_rd(EmacPssInstancePtr, 20);
	tmp |= (7 << 9);	/* max number of gigabit attempts */
	tmp |= (1 << 8);	/* enable downshift */
	tmp |= (1 << 7);	/* RGMII receive timing internally delayed */
	tmp |= (1 << 1);	/* RGMII transmit clock internally delayed */
	phy_wr(EmacPssInstancePtr, 20, tmp);
#else
	/* Copper specific control register 1 */
	tmp = phy_rd(EmacPssInstancePtr, 16);
	tmp |= (7 << 12);	/* max number of gigabit attempts */
	tmp |= (1 << 11);	/* enable downshift */
	phy_wr(EmacPssInstancePtr, 16, tmp);

	/* Control register - MAC */
	phy_wr(EmacPssInstancePtr, 22, 2);	/* page 2 */
	tmp = phy_rd(EmacPssInstancePtr, 21);
	tmp |= (1 << 5);	/* RGMII receive timing transition when data stable */
	tmp |= (1 << 4);	/* RGMII transmit clock internally delayed */
	phy_wr(EmacPssInstancePtr, 21, tmp);
	phy_wr(EmacPssInstancePtr, 22, 0);	/* page 0 */
#endif

	/* Control register */
	tmp = phy_rd(EmacPssInstancePtr, 0);
	tmp |= (1 << 12);	/* auto-negotiation enable */
	tmp |= (1 << 8);	/* enable full duplex */
	phy_wr(EmacPssInstancePtr, 0, tmp);

	/***** Try to establish a link at the highest speed possible  *****/
#ifdef CONFIG_EP107
	Xgmac_set_eth_advertise(EmacPssInstancePtr, 100);
#else
	/* Could be 1000 if an unknown bug is fixed */
	Xgmac_set_eth_advertise(EmacPssInstancePtr, 1000);
#endif
	phy_rst(EmacPssInstancePtr);

	/* Attempt auto-negotiation */
	puts("Waiting for PHY to complete auto-negotiation...\n");
	tmp = 0; /* delay counter */
	while (!(phy_rd(EmacPssInstancePtr, 1) & (1 << 5))) {
		udelay(10000);
		tmp++;
		if (tmp > 1000) { /* stalled if no link after 10 seconds */
			puts("***Error: Auto-negotiation stalled...\n");
			return -1;
		}
	}

	/* Check if the link is up */
	tmp = phy_rd(EmacPssInstancePtr, 17);
	if (  ((tmp >> 10) & 1) ) {
		/* Check for an auto-negotiation error */
		tmp = phy_rd(EmacPssInstancePtr, 19);
		if ( (tmp >> 15) & 1 ) {
			puts("***Error: Auto-negotiation error is present.\n");
			return -1;
		}
	} else {
		puts("***Error: Link is not up.\n");
		return -1;
	}

	/********************** Determine link speed **********************/
	tmp = phy_rd(EmacPssInstancePtr, 17);
	if ( ((tmp >> 14) & 3) == 2)		/* 1000Mbps */
		link_speed = 1000;
	else if ( ((tmp >> 14) & 3) == 1)	/* 100Mbps */
		link_speed = 100;
	else					/* 10Mbps */
		link_speed = 10;

	/*************************** MAC Setup ***************************/
	tmp = XEmacPss_ReadReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET);
	if (link_speed == 10)
		tmp &= ~(0x1);		/* enable 10Mbps */
	else
		tmp |= 0x1;		/* enable 100Mbps */
	if (link_speed == 1000)
		tmp |= 0x400;		/* enable 1000Mbps */
	else
		tmp &= ~(0x400);	/* disable gigabit */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET, tmp);

	/************************* GEM0_CLK Setup *************************/
	/* SLCR unlock */
	Out32(0xF8000008, 0xDF0D);

	/* Configure GEM0_RCLK_CTRL */
	Out32(0xF8000138, ((0 << 4) | (1 << 0)));

	/* Set divisors for appropriate frequency in GEM0_CLK_CTRL */
#ifdef CONFIG_EP107
	if (link_speed == 1000)		/* 125MHz */
		Out32(0xF8000140, ((1 << 20) | (48 << 8) | (1 << 4) | (1 << 0)));
	else if (link_speed == 100)	/* 25 MHz */
		Out32(0xF8000140, ((1 << 20) | (48 << 8) | (0 << 4) | (1 << 0)));
	else				/* 2.5 MHz */
		Out32(0xF8000140, ((1 << 20) | (48 << 8) | (3 << 4) | (1 << 0)));
#else
	if (link_speed == 1000)		/* 125MHz */
		Out32(0xF8000140, ((1 << 20) | (8 << 8) | (0 << 4) | (1 << 0)));
	else if (link_speed == 100)	/* 25 MHz */
		Out32(0xF8000140, ((1 << 20) | (40 << 8) | (0 << 4) | (1 << 0)));
	else				/* 2.5 MHz */
		Out32(0xF8000140, ((10 << 20) | (40 << 8) | (0 << 4) | (1 << 0)));
#endif

	/* SLCR lock */
	Out32(0xF8000004, 0x767B);

	printf("Link is now at %dMbps!\n", link_speed);

	ethstate.initialized = 1;
	return 0;
}

void Xgmac_halt(struct eth_device *dev)
{
	return;
}

int Xgmac_send(struct eth_device *dev, volatile void *packet, int length)
{
	volatile int Status;
	XEmacPss_Bd *BdPtr;
	XEmacPss *EmacPssInstancePtr = &EmacPssInstance;

	if (!ethstate.initialized) {
		puts("Error GMAC not initialized");
		return 0;
	}

	Status =
	    XEmacPss_BdRingAlloc(&(XEmacPss_GetTxRing(&EmacPssInstance)), 1,
				 &BdPtr);
	if (Status != 0) {
		puts("Error allocating TxBD");
		return 0;
	}

	/*
	 * Setup TxBD
	 */
	XEmacPss_BdSetAddressTx(BdPtr, (u32)packet);
	XEmacPss_BdSetLength(BdPtr, length);
	XEmacPss_BdClearTxUsed(BdPtr);
	XEmacPss_BdSetLast(BdPtr);

	/*
	 * Enqueue to HW
	 */
	Status =
	    XEmacPss_BdRingToHw(&(XEmacPss_GetTxRing(&EmacPssInstance)), 1,
				BdPtr);
	if (Status != 0) {
		puts("Error committing TxBD to HW");
		return 0;
	}

	/* Start transmit */
	XEmacPss_Transmit(EmacPssInstancePtr);

	/* Read the status register to know if the packet has been Transmitted. */
	Status =
	    XEmacPss_ReadReg(EmacPssInstance.Config.BaseAddress,
			     XEMACPSS_TXSR_OFFSET);
	if (Status &
	    (XEMACPSS_TXSR_HRESPNOK_MASK | XEMACPSS_TXSR_URUN_MASK |
	     XEMACPSS_TXSR_BUFEXH_MASK)) {
		printf("Something has gone wrong here!? Status is 0x%x.\n",
		       Status);
	}

	if (Status & XEMACPSS_TXSR_TXCOMPL_MASK) {

//		printf("tx packet sent\n");

		/*
		 * Now that the frame has been sent, post process our TxBDs.
		 */
		if (XEmacPss_BdRingFromHwTx
		    (&(XEmacPss_GetTxRing(&EmacPssInstance)), 1, &BdPtr) == 0) {
			puts("TxBDs were not ready for post processing");
			return 0;
		}

		/*
		 * Free the TxBD.
		 */
		Status =
		    XEmacPss_BdRingFree(&(XEmacPss_GetTxRing(&EmacPssInstance)),
					1, BdPtr);
		if (Status != 0) {
			puts("Error freeing up TxBDs");
			return 0;
		}
	}
	/* Clear Tx status register before leaving . */
	XEmacPss_WriteReg(EmacPssInstance.Config.BaseAddress,
			  XEMACPSS_TXSR_OFFSET, Status);
	return 1;

}

int Xgmac_rx(struct eth_device *dev)
{
	u32 status, retval;
	XEmacPss *EmacPssInstancePtr = &EmacPssInstance;

	status =
	    XEmacPss_ReadReg(EmacPssInstancePtr->Config.BaseAddress,
			     XEMACPSS_RXSR_OFFSET);
	if (status & XEMACPSS_RXSR_FRAMERX_MASK) {

//		printf("rx packet received\n");
	
		do {
			retval = Xgmac_process_rx(EmacPssInstancePtr);
		} while (retval == 0) ;
	}

	/* Clear interrupt status.
	 */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
	                  XEMACPSS_RXSR_OFFSET, status);
	
	return 1;
}

int Xgmac_register(bd_t * bis)
{
	struct eth_device *dev;
	dev = malloc(sizeof(*dev));
	if (dev == NULL) {
		return 1;
	}
	memset(dev, 0, sizeof(*dev));
	sprintf(dev->name, "xgmac");

	if (Xgmac_one_time_init() < 0) {
		printf("xgmac init failed!");
		return -1;
	}
	dev->iobase = EmacPssInstance.Config.BaseAddress;
	dev->priv = &EmacPssInstance;
	dev->init = Xgmac_init;
	dev->halt = Xgmac_halt;
	dev->send = Xgmac_send;
	dev->recv = Xgmac_rx;

	eth_register(dev);

#if defined(CONFIG_CMD_MII) && !defined(CONFIG_BITBANGMII)
	miiphy_register(dev->name, Xgmac_mii_read, Xgmac_mii_write);
#endif
	return 0;
}

/*=============================================================================
 *
 * Xgmac_process_rx- process the next incoming packet
 *
 * return's 0 if OK, -1 on error
 */
int Xgmac_process_rx(XEmacPss * EmacPssInstancePtr)
{
	uchar *buffer = data_buffer;
	u32 rx_status, hwbuf;
	int frame_len;
	u32 *bd_addr;

    bd_addr = (u32 *) & EmacPssInstancePtr->RxBdRing.
	    RxBD_start[EmacPssInstancePtr->RxBdRing.RxBD_current];

	rx_status = XEmacPss_BdRead((bd_addr), XEMACPSS_BD_ADDR_OFFSET);
	if (! (rx_status & XEMACPSS_RXBUF_NEW_MASK)) {
		return (-1);
	}

	rx_status = XEmacPss_BdIsRxSOF(bd_addr);
	if (!rx_status) {
		printf("GEM: SOF not set for last buffer received!\n");
		return (-1);
	}
	rx_status = XEmacPss_BdIsRxEOF(bd_addr);
	if (!rx_status) {
		printf("GEM: EOF not set for last buffer received!\n");
		return (-1);
	}

	frame_len = XEmacPss_BdGetLength(bd_addr);
	if (frame_len == 0) {
		printf("GEM: Hardware reported 0 length frame!\n");
		return (-1);
	}

	hwbuf = (u32) (*bd_addr & XEMACPSS_RXBUF_ADD_MASK);
	if (hwbuf == (u32) NULL) {
		printf("GEM: Error swapping out buffer!\n");
		return (-1);
	}
	memcpy(buffer, (void *)hwbuf, frame_len);
	Xgmac_next_rx_buf(EmacPssInstancePtr);
	NetReceive(buffer, frame_len);

	return (0);
}

int Xgmac_init_rxq(XEmacPss * EmacPssInstancePtr, void *bd_start, int num_elem)
{
	XEmacPss_BdRing *r;
	int loop = 0;

	if ((num_elem <= 0) || (num_elem > RXBD_CNT)) {
		return (-1);
	}

	for (; loop < 2 * (num_elem);) {
		*(((u32 *) bd_start) + loop) = 0x00000000;
		*(((u32 *) bd_start) + loop + 1) = 0xF0000000;
		loop += 2;
	}

	r = & EmacPssInstancePtr->RxBdRing;
	r->RxBD_start = (XEmacPss_Bd *) bd_start;
	r->Length = num_elem;
	r->RxBD_current = 0;
	r->RxBD_end = 0;
	r->Rx_first_buf = 0;

	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_RXQBASE_OFFSET, (u32) bd_start);

	return 0;
}

int Xgmac_make_rxbuff_mem(XEmacPss * EmacPssInstancePtr, void *rx_buf_start,
			  u32 rx_buffsize)
{
	XEmacPss_BdRing *r;
	int num_bufs;
	int assigned_bufs;
	int i;
	u32 *bd_addr;

	if ((EmacPssInstancePtr == NULL) || (rx_buf_start == NULL)) {
		return (-1);
	}

	r = & EmacPssInstancePtr->RxBdRing;

	assigned_bufs = 0;

	if ((num_bufs = rx_buffsize / XEMACPSS_RX_BUF_SIZE) == 0) {
		return 0;
	}
	for (i = 0; i < num_bufs; i++) {
		if (r->RxBD_end < r->Length) {
			memset((char *)(rx_buf_start +
					(i * XEMACPSS_RX_BUF_SIZE)), 0, XEMACPSS_RX_BUF_SIZE);

			bd_addr = (u32 *) & r->RxBD_start[r->RxBD_end];

			XEmacPss_BdSetAddressRx(bd_addr,
						(u32) (((char *)
							rx_buf_start) + (i * XEMACPSS_RX_BUF_SIZE)));

			r->RxBD_end++;
			assigned_bufs++;
		} else {
			return assigned_bufs;
		}
	}
	bd_addr = (u32 *) & r->RxBD_start[r->RxBD_end - 1];
	XEmacPss_BdSetRxWrap(bd_addr);

	return assigned_bufs;
}

int Xgmac_next_rx_buf(XEmacPss * EmacPssInstancePtr)
{
	XEmacPss_BdRing *r;
	u32 prev_stat = 0;
	u32 *bd_addr = NULL;

	if (EmacPssInstancePtr == NULL) {
		printf
		    ("\ngem_clr_rx_buf with EmacPssInstancePtr as !!NULL!! \n");
		return -1;
	}

	r = & EmacPssInstancePtr->RxBdRing;

	bd_addr = (u32 *) & r->RxBD_start[r->RxBD_current];
	prev_stat = XEmacPss_BdIsRxSOF(bd_addr);
	if (prev_stat) {
		r->Rx_first_buf = r->RxBD_current;
	} else {
		XEmacPss_BdClearRxNew(bd_addr);
		XIo_Out32((u32) (bd_addr + 1), 0xF0000000);
	}

	if (XEmacPss_BdIsRxEOF(bd_addr)) {
		bd_addr = (u32 *) & r->RxBD_start[r->Rx_first_buf];
		XEmacPss_BdClearRxNew(bd_addr);
		XIo_Out32((u32) (bd_addr + 1), 0xF0000000);
	}

	if ((++r->RxBD_current) > r->Length - 1) {
		r->RxBD_current = 0;
	}

	return 0;
}

void Xgmac_set_eth_advertise(XEmacPss * EmacPssInstancePtr, int link_speed) {

	int tmp;

	/* MAC setup */
	tmp = XEmacPss_ReadReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET);
	if (link_speed == 10)
		tmp &= ~(1 << 0);	/* enable 10Mbps */
	else if (link_speed == 100)
		tmp |= (1 << 0);	/* enable 100Mbps */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET, tmp);

	phy_wr(EmacPssInstancePtr, 22, 0);	/* page 0 */

	/* Auto-negotiation advertisement register */
	tmp = phy_rd(EmacPssInstancePtr, 4);
	if (link_speed >= 100) {
		tmp |= (1 << 8);	/* advertise 100Mbps F */
		tmp |= (1 << 7);	/* advertise 100Mbps H */
	} else {
		tmp &= ~(1 << 8);	/* advertise 100Mbps F */
		tmp &= ~(1 << 7);	/* advertise 100Mbps H */
	}
	if (link_speed >= 10) {
		tmp |= (1 << 6);	/* advertise 10Mbps F */
		tmp |= (1 << 5);	/* advertise 10Mbps H */
	} else {
		tmp &= ~(1 << 6);	/* advertise 10Mbps F */
		tmp &= ~(1 << 5);	/* advertise 10Mbps H */
	}
	phy_wr(EmacPssInstancePtr, 4, tmp);

	/* 1000BASE-T control register */
	tmp = phy_rd(EmacPssInstancePtr, 9);
	if (link_speed == 1000) {
		tmp |= (1 << 9);	/* advertise 1000Mbps F */
		tmp |= (1 << 8);	/* advertise 1000Mbps H */
	} else {
		tmp &= ~(1 << 9);	/* advertise 1000Mbps F */
		tmp &= ~(1 << 8);	/* advertise 1000Mbps H */
	}
	phy_wr(EmacPssInstancePtr, 9, tmp);

}

