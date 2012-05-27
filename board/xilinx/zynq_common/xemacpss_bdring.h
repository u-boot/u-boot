/* $Id: xemacpss_bdring.h,v 1.1.2.1 2009/06/17 16:10:26 wyang Exp $ */
/******************************************************************************
*
* (c) Copyright 2009-2010 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xemacpss_bdring.h
*
* The Xiline EmacPss Buffer Descriptor ring driver. This is part of EmacPss
* DMA functionalities.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a wsy  06/01/09 First release
* </pre>
*
******************************************************************************/

#ifndef XEMACPSS_BDRING_H	/* prevent curcular inclusions */
#define XEMACPSS_BDRING_H	/* by using protection macros */


/**************************** Type Definitions *******************************/

/** This is an internal structure used to maintain the DMA list */
typedef struct {
	u32 PhysBaseAddr;/**< Physical address of 1st BD in list */
	u32 BaseBdAddr;	 /**< Virtual address of 1st BD in list */
	u32 HighBdAddr;	 /**< Virtual address of last BD in the list */
	u32 Length;	 /**< Total size of ring in bytes */
	u32 RunState;	 /**< Flag to indicate DMA is started */
	u32 Separation;	 /**< Number of bytes between the starting address
                                  of adjacent BDs */
	XEmacPss_Bd *RxBD_start;	/**< First BD in the Rx queue*/
	int RxBD_current;		/**< Index to the current BD*/
	int RxBD_end;			/**< Index to the last BD*/
	int Rx_first_buf;		/**< Index to the first BD*/
	XEmacPss_Bd *FreeHead;
			     /**< First BD in the free group */
	XEmacPss_Bd *PreHead;/**< First BD in the pre-work group */
	XEmacPss_Bd *HwHead; /**< First BD in the work group */
	XEmacPss_Bd *HwTail; /**< Last BD in the work group */
	XEmacPss_Bd *PostHead;
			     /**< First BD in the post-work group */
	XEmacPss_Bd *BdaRestart;
			     /**< BDA to load when channel is started */
	unsigned HwCnt;	     /**< Number of BDs in work group */
	unsigned PreCnt;     /**< Number of BDs in pre-work group */
	unsigned FreeCnt;    /**< Number of allocatable BDs in the free group */
	unsigned PostCnt;    /**< Number of BDs in post-work group */
	unsigned AllCnt;     /**< Total Number of BDs for channel */
} XEmacPss_BdRing;


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* Use this macro at initialization time to determine how many BDs will fit
* in a BD list within the given memory constraints.
*
* The results of this macro can be provided to XEmacPss_BdRingCreate().
*
* @param Alignment specifies what byte alignment the BDs must fall on and
*        must be a power of 2 to get an accurate calculation (32, 64, 128,...)
* @param Bytes is the number of bytes to be used to store BDs.
*
* @return Number of BDs that can fit in the given memory area
*
* @note
* C-style signature:
*    u32 XEmacPss_BdRingCntCalc(u32 Alignment, u32 Bytes)
*
******************************************************************************/
#define XEmacPss_BdRingCntCalc(Alignment, Bytes)                    \
    (u32)((Bytes) / ((sizeof(XEmacPss_Bd) + ((Alignment)-1)) &   \
    ~((Alignment)-1)))

/*****************************************************************************/
/**
* Use this macro at initialization time to determine how many bytes of memory
* is required to contain a given number of BDs at a given alignment.
*
* @param Alignment specifies what byte alignment the BDs must fall on. This
*        parameter must be a power of 2 to get an accurate calculation (32, 64,
*        128,...)
* @param NumBd is the number of BDs to calculate memory size requirements for
*
* @return The number of bytes of memory required to create a BD list with the
*         given memory constraints.
*
* @note
* C-style signature:
*    u32 XEmacPss_BdRingMemCalc(u32 Alignment, u32 NumBd)
*
******************************************************************************/
#define XEmacPss_BdRingMemCalc(Alignment, NumBd)                    \
    (u32)((sizeof(XEmacPss_Bd) + ((Alignment)-1)) &              \
    ~((Alignment)-1)) * (NumBd)

/****************************************************************************/
/**
* Return the total number of BDs allocated by this channel with
* XEmacPss_BdRingCreate().
*
* @param  InstancePtr is the DMA channel to operate on.
*
* @return The total number of BDs allocated for this channel.
*
* @note
* C-style signature:
*    u32 XEmacPss_BdRingGetCnt(XEmacPss_BdRing* RingPtr)
*
*****************************************************************************/
#define XEmacPss_BdRingGetCnt(RingPtr) ((RingPtr)->AllCnt)

/****************************************************************************/
/**
* Return the number of BDs allocatable with XEmacPss_BdRingAlloc() for pre-
* processing.
*
* @param  InstancePtr is the DMA channel to operate on.
*
* @return The number of BDs currently allocatable.
*
* @note
* C-style signature:
*    u32 XEmacPss_BdRingGetFreeCnt(XEmacPss_BdRing* RingPtr)
*
*****************************************************************************/
#define XEmacPss_BdRingGetFreeCnt(RingPtr)   ((RingPtr)->FreeCnt)

/****************************************************************************/
/**
* Return the next BD from BdPtr in a list.
*
* @param  InstancePtr is the DMA channel to operate on.
* @param  BdPtr is the BD to operate on.
*
* @return The next BD in the list relative to the BdPtr parameter.
*
* @note
* C-style signature:
*    XEmacPss_Bd *XEmacPss_BdRingNext(XEmacPss_BdRing* RingPtr,
*                                      XEmacPss_Bd *BdPtr)
*
*****************************************************************************/
#define XEmacPss_BdRingNext(RingPtr, BdPtr)                           \
    (((u32)(BdPtr) >= (RingPtr)->HighBdAddr) ?                     \
    (XEmacPss_Bd*)(RingPtr)->BaseBdAddr :                              \
    (XEmacPss_Bd*)((u32)(BdPtr) + (RingPtr)->Separation))

/****************************************************************************/
/**
* Return the previous BD from BdPtr in the list.
*
* @param  InstancePtr is the DMA channel to operate on.
* @param  BdPtr is the BD to operate on
*
* @return The previous BD in the list relative to the BdPtr parameter.
*
* @note
* C-style signature:
*    XEmacPss_Bd *XEmacPss_BdRingPrev(XEmacPss_BdRing* RingPtr,
*                                      XEmacPss_Bd *BdPtr)
*
*****************************************************************************/
#define XEmacPss_BdRingPrev(RingPtr, BdPtr)                           \
    (((u32)(BdPtr) <= (RingPtr)->BaseBdAddr) ?                     \
    (XEmacPss_Bd*)(RingPtr)->HighBdAddr :                              \
    (XEmacPss_Bd*)((u32)(BdPtr) - (RingPtr)->Separation))

/************************** Function Prototypes ******************************/

/*
 * Scatter gather DMA related functions in xemacpss_bdring.c
 */
int XEmacPss_BdRingCreate(XEmacPss_BdRing * RingPtr, u32 PhysAddr,
			  u32 VirtAddr, u32 Alignment, unsigned BdCount);
int XEmacPss_BdRingClone(XEmacPss_BdRing * RingPtr, XEmacPss_Bd * SrcBdPtr,
			 u8 Direction);
int XEmacPss_BdRingAlloc(XEmacPss_BdRing * RingPtr, unsigned NumBd,
			 XEmacPss_Bd ** BdSetPtr);
int XEmacPss_BdRingUnAlloc(XEmacPss_BdRing * RingPtr, unsigned NumBd,
			   XEmacPss_Bd * BdSetPtr);
int XEmacPss_BdRingToHw(XEmacPss_BdRing * RingPtr, unsigned NumBd,
			XEmacPss_Bd * BdSetPtr);
int XEmacPss_BdRingFree(XEmacPss_BdRing * RingPtr, unsigned NumBd,
			XEmacPss_Bd * BdSetPtr);
unsigned XEmacPss_BdRingFromHwTx(XEmacPss_BdRing * RingPtr, unsigned BdLimit,
				 XEmacPss_Bd ** BdSetPtr);
unsigned XEmacPss_BdRingFromHwRx(XEmacPss_BdRing * RingPtr, unsigned BdLimit,
				 XEmacPss_Bd ** BdSetPtr);
int XEmacPss_BdRingCheck(XEmacPss_BdRing * RingPtr, u8 Direction);


#endif /* end of protection macros */
