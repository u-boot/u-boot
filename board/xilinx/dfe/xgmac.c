/*
 */

#include <common.h>
#include <net.h>

#include "xemacpss.h"

/************************ Forward function declaration **********************/

int Xgmac_process_rx(XEmacPss * EmacPssInstancePtr);
int Xgmac_init_rxq(XEmacPss * EmacPssInstancePtr, void *bd_start, int num_elem);
int Xgmac_make_rxbuff_mem(XEmacPss * EmacPssInstancePtr, void *rx_buf_start,
			  u32 rx_buffsize);
int Xgmac_next_rx_buf(XEmacPss * EmacPssInstancePtr);
int Xgmac_phy_mgmt_idle(XEmacPss * EmacPssInstancePtr);

/*************************** Constant Definitions ***************************/

#define EMACPSS_DEVICE_ID   0
#define PHY_ADDR 0x7

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

static u32 phy_rd(XEmacPss * e, u32 a)
{
	u16 PhyData;

	phy_spinwait(e);
	XEmacPss_PhyRead(e, PHY_ADDR, a, &PhyData);
	phy_spinwait(e);
	return PhyData;
}

static void phy_wr(XEmacPss * e, u32 a, u32 v)
{
	phy_spinwait(e);
	XEmacPss_PhyWrite(e, PHY_ADDR, a, v);
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
	}
	puts("\nPHY reset complete.\n");
}

static void Out32(u32 OutAddress, u32 Value)
{
	*(volatile u32 *) OutAddress = Value;
	dmb();
}

/*****************************************************************************/

void eth_halt(void)
{
	return;
}

int eth_init(bd_t * bis)
{
	int tmp;
	int link_speed;
	int Status;
	XEmacPss_Config *Config;
	XEmacPss *EmacPssInstancePtr = &EmacPssInstance;
	XEmacPss_Bd BdTemplate;

	if (ethstate.initialized) {
		return 1;
	}

	ethstate.initialized = 0;

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
		return 0;
	}

	Status = XEmacPss_BdRingClone(&(XEmacPss_GetTxRing(EmacPssInstancePtr)),
				      &BdTemplate, XEMACPSS_SEND);
	if (Status != 0) {
		puts("Error setting up TxBD space, BdRingClone");
		return 0;
	}

	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_TXQBASE_OFFSET,
			  EmacPssInstancePtr->TxBdRing.BaseBdAddr);

	/* MAC Setup */
	/*
	 *  Following is the setup for Network Configuration register.
	 *  Bit 0:  Set for 100 Mbps operation.
	 *  Bit 1:  Set for Full Duplex mode.
	 *  Bit 4:  Set to allow Copy all frames.
	 *  Bit 10: Set for Gigabit.
	 *  Bit 17: Set for FCS removal.
	 *  Bits 20-18: Set with value binary 010 to divide pclk by 32
	 *              (pclk up to 80 MHz)
	 */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET, 0x000A0413);

	/*
	 * Following is the setup for DMA Configuration register.
	 * Bits 4-0: To set AHB fixed burst length for DMA data operations ->
	 *           Set with binary 00100 to use INCR4 AHB bursts.
	 * Bits 9-8: Receiver packet buffer memory size ->
	 *       Set with binary 11 to Use full configured addressable space (8 Kb)
	 * Bit 10  : Transmitter packet buffer memory size ->
	 *       Set with binary 1 to Use full configured addressable space (4 Kb)
	 * Bits 23-16 : DMA receive buffer size in AHB system memory ->
	 *   Set with binary 00011000 to use 1536 byte (1*max length frame/buffer).
	 */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_DMACR_OFFSET, 0x00180704);

	/* Disable all the MAC Interrupts */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_IDR_OFFSET, 0xFFFFFFFF);

	/*
	 * Following is the setup for Network Control register.
	 * Bit 2:  Set to enable Receive operation.
	 * Bit 3:  Set to enable Transmitt operation.
	 * Bit 4:  Set to enable MDIO operation.
	 */
	tmp =
	    XEmacPss_ReadReg(EmacPssInstancePtr->Config.BaseAddress,
			     XEMACPSS_NWCTRL_OFFSET);
	/*MDIO, Rx and Tx enable */
	tmp |=
	    XEMACPSS_NWCTRL_MDEN_MASK | XEMACPSS_NWCTRL_RXEN_MASK |
	    XEMACPSS_NWCTRL_TXEN_MASK;
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCTRL_OFFSET, tmp);

	/* PHY Setup */

#ifdef CONFIG_EP107
	/* "add delay to RGMII rx interface" */
	phy_wr(EmacPssInstancePtr, 20, 0xc93);
#else
	phy_wr(EmacPssInstancePtr, 22, 2);	/* page 2 */

	/* rx clock transition when data stable */
	phy_wr(EmacPssInstancePtr, 21, 0x3030);

	phy_wr(EmacPssInstancePtr, 22, 0);	/* page 0 */
#endif

	/* link speed advertisement for autonegotiation */
	tmp = phy_rd(EmacPssInstancePtr, 4);
	tmp |= 0xd80;		/* enable 100Mbps */
	tmp |= 0x60;		/* enable 10 Mbps */
	phy_wr(EmacPssInstancePtr, 4, tmp);

	/* enable gigabit advertisement */
	tmp = phy_rd(EmacPssInstancePtr, 9);
	tmp |= 0x0300;
	phy_wr(EmacPssInstancePtr, 9, tmp);

	/* Copper specific control register 1 */
	phy_wr(EmacPssInstancePtr, 22, 0);
	tmp = phy_rd(EmacPssInstancePtr, 16);
	tmp |= (7 << 12);	/* max number of gigabit attempts */
	tmp |= (1 << 11);	/* enable downshift */
	phy_wr(EmacPssInstancePtr, 16, tmp);

	/* enable autonegotiation, set 100Mbps, full-duplex, restart aneg */
	tmp = phy_rd(EmacPssInstancePtr, 0);
	phy_wr(EmacPssInstancePtr, 0, 0x3300 | (tmp & 0x1F));

	phy_rst(EmacPssInstancePtr);

	puts("\nWaiting for PHY to complete autonegotiation.");
	tmp = 0; /* delay counter */
	while (!(phy_rd(EmacPssInstancePtr, 1) & (1 << 5))) {
		udelay(10000);
		tmp++;
		if (tmp > 1000) { /* stalled if no link after 10 seconds */
			printf("***Error: Auto-negotiation stalled...\n");
			return -1;
		}
	}

	puts("\nPHY claims autonegotiation complete...\n");

	/* Check if the link is up */
	phy_wr(EmacPssInstancePtr, 22, 0);
	tmp = phy_rd(EmacPssInstancePtr, 17);
	if (  ((tmp >> 10) & 1) ) {
		/* Check for an auto-negotiation error */
		phy_wr(EmacPssInstancePtr, 22, 0);
		tmp = phy_rd(EmacPssInstancePtr, 19);
		if ( (tmp >> 15) & 1 ) {
			puts("***Error: Auto-negotiation error is present.\n");
			return -1;
		}
	} else {
		puts("***Error: Link is not up.\n");
		return -1;
	}

	/* Determine link speed */
	phy_wr(EmacPssInstancePtr, 22, 0);
	tmp = phy_rd(EmacPssInstancePtr, 17);
	if ( ((tmp >> 14) & 3) == 2)		/* 1000Mbps */
		link_speed = 1000;
	else if ( ((tmp >> 14) & 3) == 1)	/* 100Mbps */
		link_speed = 100;
	else					/* 10Mbps */
		link_speed = 10;

	/* MAC Setup */
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

	/* GEM0_CLK Setup */
	/* SLCR unlock */
	Out32(0xF8000008, 0xDF0D);

	/* Configure GEM0_RCLK_CTRL */
	Out32(0xF8000138, ((0 << 4) | (1 << 0)));

	/* Set divisors for appropriate frequency in GEM0_CLK_CTRL */
	if (link_speed == 1000)		/* 125MHz */
		Out32(0xF8000140, ((1 << 20) | (8 << 8) | (0 << 4) | (1 << 0)));
	else if (link_speed == 100)	/* 25 MHz */
		Out32(0xF8000140, ((1 << 20) | (40 << 8) | (0 << 4) | (1 << 0)));
	else				/* 2.5 MHz */
		Out32(0xF8000140, ((10 << 20) | (40 << 8) | (0 << 4) | (1 << 0)));

	/* SLCR lock */
	Out32(0xF8000008, 0x767B);

	printf("GEM link speed is %dMbps!\n", link_speed);

	ethstate.initialized = 1;
	return 0;
}

int eth_send(volatile void *ptr, int len)
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
	XEmacPss_BdSetAddressTx(BdPtr, (u32) ptr);
	XEmacPss_BdSetLength(BdPtr, len);
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

int eth_rx(void)
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
