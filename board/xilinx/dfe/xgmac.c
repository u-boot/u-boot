/*
 */

#include <common.h>
#include <net.h>

#include "xemacpss_example.h"

/************************ Forward function declaration **********************/

int Xgmac_process_rx(XEmacPss * EmacPssInstancePtr);
int Xgmac_init_rxq(XEmacPss * EmacPssInstancePtr, void *bd_start, int num_elem);
int Xgmac_make_rxbuff_mem(XEmacPss * EmacPssInstancePtr, void *rx_buf_start,
			  u32 rx_buffsize);
int Xgmac_next_rx_buf(XEmacPss * EmacPssInstancePtr);
int Xgmac_phy_mgmt_idle(XEmacPss * EmacPssInstancePtr);

/*************************** Constant Definitions ***************************/

#define EMACPSS_DEVICE_ID   0
#define PHY_ADDR 7

#define RXBD_CNT       8	/* Number of RxBDs to use */
#define TXBD_CNT       8	/* Number of TxBDs to use */

#define phy_spinwait(e) do { while (!Xgmac_phy_mgmt_idle(e)); } while (0)

/*************************** Variable Definitions ***************************/

/*
 * Aligned memory segments to be used for buffer descriptors
 */

static XEmacPss_Bd RxBdSpace[RXBD_CNT] __attribute__ ((aligned(16)));
/* To have some space between the BDs and Rx buffer. */
static u32 dummy1;
static u32 dummy2;
static XEmacPss_Bd TxBdSpace[TXBD_CNT] __attribute__ ((aligned(16)));
static u32 dummy3;
static char RxBuffer[RXBD_CNT * XEMACPSS_RX_BUF_SIZE];
static uchar data_buffer[XEMACPSS_RX_BUF_SIZE];

static struct {
	u8 initialized;
} ethstate = {
0};

XEmacPss EmacPssInstance;

/*******************************************************************************************/
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

	while (phy_rd(e, 0) & 0x8000)
		putc('.');
	puts("\nPHY reset complete.\n");
}

/*******************************************************************************************/

#if DEBUG
void device_regs(int num)
{
	int i;
	if (num & 0x2) {
		printf("***PHY REGISTER DUMP***\n");
		for (i = 0; i < 32; i++) {
			printf("\t%2d: 0x%04x", i, phy_rd(&EmacPssInstance, i));
			if (i % 8 == 7)
				putc('\n');
		}
	}
	if (num & 0x1) {
		printf("***GEM REGISTER DUMP***\n");
		for (i = 0; i < 0x040; i += 4) {
			printf("\t0x%03x: 0x%08lx", i,
			       XEmacPss_ReadReg(EmacPssInstance.Config.
						BaseAddress, i));
			if (i % 16 == 12)
				putc('\n');
		}
	}
}
#endif

void eth_halt(void)
{
	return;
}

int eth_init(bd_t * bis)
{
	int tmp;
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
	    Xgmac_make_rxbuff_mem(EmacPssInstancePtr, &RxBuffer,
				  sizeof(RxBuffer));
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
	 *      Following is the setup for Network Configuration register.
	 *      Bit 0:  Set for 100 Mbps operation.
	 *      Bit 1:  Set for Full Duplex mode.
	 *      Bit 4:  Set to allow Copy all frames.
	 *      Bit 17: Set for FCS removal.
	 *      Bits 20-18: Set with value binary 010 to divide pclk by 32 (pclk up to 80 MHz)
	 */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET, 0x000A0013);

	/*
	 *      Following is the setup for DMA Configuration register.
	 *      Bits 4-0: To set AHB fixed burst length for DMA data operations -> Set with binary 00100 to use INCR4 AHB bursts.
	 *      Bits 9-8: Receiver packet buffer memory size -> Set with binary 11 to Use full configured addressable space (8 Kb).
	 *      Bit 10  : Transmitter packet buffer memory size -> Set with binary 1 to Use full configured addressable space (4 Kb). 
	 *      Bits 23-16  : DMA receive buffer size in AHB system memory -> Set with binary 00011000 to use 1536 byte (1*max length frame/buffer).
	 */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_DMACR_OFFSET, 0x00180704);

	/* Disable all the MAC Interrupts */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_IDR_OFFSET, 0xFFFFFFFF);

	/*
	 *      Following is the setup for Network Control register.
	 *      Bit 2:  Set to enable Receive operation.
	 *      Bit 3:  Set to enable Transmitt operation.
	 *      Bit 4:  Set to enable MDIO operation.
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
	/* enable autonegotiation, set 100Mbps, full-duplex, restart aneg */
	tmp = phy_rd(EmacPssInstancePtr, 0);
	phy_wr(EmacPssInstancePtr, 0, 0x3300 | (tmp & 0x1F));

	/* "add delay to RGMII rx interface" */
	phy_wr(EmacPssInstancePtr, 20, 0xc93);

	/* link speed advertisement for autonegotiation */
	tmp = phy_rd(EmacPssInstancePtr, 4);
	tmp |= 0xd80;		/* enable 100Mbps */
	tmp &= ~0x60;		/* disable 10 Mbps */
	phy_wr(EmacPssInstancePtr, 4, tmp);

	/* *disable* gigabit advertisement */
	tmp = phy_rd(EmacPssInstancePtr, 9);
	tmp &= ~0x0300;
	phy_wr(EmacPssInstancePtr, 9, tmp);

	phy_rst(EmacPssInstancePtr);
	puts("\nWaiting for PHY to complete autonegotiation.");
	while (!(phy_rd(EmacPssInstancePtr, 1) & (1 << 5))) ;
	puts("\nPHY claims autonegotiation complete...\n");

	puts("GEM link speed is 100Mbps\n");

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
	volatile u32 status;
	XEmacPss *EmacPssInstancePtr = &EmacPssInstance;
	status =
	    XEmacPss_ReadReg(EmacPssInstancePtr->Config.BaseAddress,
			     XEMACPSS_RXSR_OFFSET);
	if (status & XEMACPSS_RXSR_FRAMERX_MASK) {
		if ((Xgmac_process_rx(EmacPssInstancePtr)) == (-1)) {
			return 0;
		}
	}

	/* Disabled clearing the status because NetLoop() calls eth_rx() in a tight loop 
	 *  and if Rx status is cleared it will not receive any pasket after the first pasket.
	 *  XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,XEMACPSS_RXSR_OFFSET,status);
	 */
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
	u32 rx_status, mem_addr;
	int frame_len;
	u32 *addr =
	    (u32 *) & EmacPssInstancePtr->RxBdRing.
	    RxBD_start[EmacPssInstancePtr->RxBdRing.RxBD_current];

	rx_status = XEmacPss_BdIsRxSOF(addr);

	/* if not a start of frame, something wrong. So recover and return */
	if (!rx_status) {
		return (-1);
	}
	rx_status = XEmacPss_BdIsRxEOF(addr);

	if (rx_status) {
		frame_len = XEmacPss_BdGetLength(addr);
		if (frame_len == 0) {
			printf("Hardware reported 0 length frame!\n");
			return (-1);
		}

		mem_addr = (u32) (*addr & XEMACPSS_RXBUF_ADD_MASK);
		if (mem_addr == (u32) NULL) {
			printf("Error swapping out buffer!\n");
			return (-1);
		}
		memcpy(buffer, (void *)mem_addr, frame_len);
		Xgmac_next_rx_buf(EmacPssInstancePtr);
		NetReceive(buffer, frame_len);
		return (0);
	} else {
		/* this is wrong ! should never be here is things are OK */
		printf
		    ("Something is wrong we should not be here for last buffer received!\n");
		return (-1);
	}
	return 0;
}

int Xgmac_init_rxq(XEmacPss * EmacPssInstancePtr, void *bd_start, int num_elem)
{
	int loop = 0;

	if ((num_elem <= 0) || (num_elem > RXBD_CNT)) {
		return (-1);
	} else {
		for (; loop < 2 * (num_elem);) {
			*(((u32 *) bd_start) + loop) = 0x00000000;
			*(((u32 *) bd_start) + loop + 1) = 0xF0000000;
			loop += 2;
		}
		EmacPssInstancePtr->RxBdRing.RxBD_start =
		    (XEmacPss_Bd *) bd_start;
		EmacPssInstancePtr->RxBdRing.Length = num_elem;
		EmacPssInstancePtr->RxBdRing.RxBD_current = 0;
		EmacPssInstancePtr->RxBdRing.RxBD_end = 0;
		EmacPssInstancePtr->RxBdRing.Rx_first_buf = 0;

		XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
				  XEMACPSS_RXQBASE_OFFSET, (u32) bd_start);

		return 0;
	}
}

int Xgmac_make_rxbuff_mem(XEmacPss * EmacPssInstancePtr, void *rx_buf_start,
			  u32 rx_buffsize)
{
	int num_bufs;
	int assigned_bufs;
	int i;
	u32 *addr;
	if ((EmacPssInstancePtr == NULL) || (rx_buf_start == NULL)) {
		return (-1);
	} else {
		assigned_bufs = 0;

		if ((num_bufs = rx_buffsize / XEMACPSS_RX_BUF_SIZE) == 0) {
			return 0;
		}
		for (i = 0; i < num_bufs; i++) {
			if (EmacPssInstancePtr->RxBdRing.RxBD_end <
			    EmacPssInstancePtr->RxBdRing.Length) {
				memset((char *)(rx_buf_start +
						(i * XEMACPSS_RX_BUF_SIZE)), 0,
				       XEMACPSS_RX_BUF_SIZE);

				addr =
				    (u32 *) & EmacPssInstancePtr->RxBdRing.
				    RxBD_start[EmacPssInstancePtr->RxBdRing.
					       RxBD_end];

				XEmacPss_BdSetAddressRx(addr,
							(u32) (((char *)
								rx_buf_start) +
							       (i *
								XEMACPSS_RX_BUF_SIZE)));

				EmacPssInstancePtr->RxBdRing.RxBD_end++;
				assigned_bufs++;
			} else {
				return assigned_bufs;
			}
		}
		addr =
		    (u32 *) & EmacPssInstancePtr->RxBdRing.
		    RxBD_start[EmacPssInstancePtr->RxBdRing.RxBD_end - 1];
		XEmacPss_BdSetRxWrap(addr);
		return assigned_bufs;
	}
}

int Xgmac_next_rx_buf(XEmacPss * EmacPssInstancePtr)
{
	u32 prev_stat = 0;
	u32 *addr = NULL;
	if (EmacPssInstancePtr != NULL) {
		addr =
		    (u32 *) & EmacPssInstancePtr->RxBdRing.
		    RxBD_start[EmacPssInstancePtr->RxBdRing.RxBD_current];
		prev_stat = XEmacPss_BdIsRxSOF(addr);

		if (prev_stat) {
			EmacPssInstancePtr->RxBdRing.Rx_first_buf =
			    EmacPssInstancePtr->RxBdRing.RxBD_current;
		} else {
			XEmacPss_BdClearRxNew(addr);
			XIo_Out32((u32) (addr + 1), 0xF0000000);
		}

		if (XEmacPss_BdIsRxEOF(addr)) {
			addr =
			    (u32 *) & EmacPssInstancePtr->RxBdRing.
			    RxBD_start[EmacPssInstancePtr->RxBdRing.
				       Rx_first_buf];
			XEmacPss_BdClearRxNew(addr);
			XIo_Out32((u32) (addr + 1), 0xF0000000);
		}

		if ((++EmacPssInstancePtr->RxBdRing.RxBD_current) >
		    EmacPssInstancePtr->RxBdRing.Length - 1) {
			EmacPssInstancePtr->RxBdRing.RxBD_current = 0;
		}

		return 0;
	} else {
		printf
		    ("\ngem_clr_rx_buf with EmacPssInstancePtr as !!NULL!! \n");
		return -1;
	}
}
