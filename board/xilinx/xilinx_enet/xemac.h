/******************************************************************************
*
*     Author: Xilinx, Inc.
*
*
*     This program is free software; you can redistribute it and/or modify it
*     under the terms of the GNU General Public License as published by the
*     Free Software Foundation; either version 2 of the License, or (at your
*     option) any later version.
*
*
*     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
*     COURTESY TO YOU. BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
*     ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION OR STANDARD,
*     XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION IS FREE
*     FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE FOR OBTAINING
*     ANY THIRD PARTY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
*     XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
*     THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY
*     WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM
*     CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND
*     FITNESS FOR A PARTICULAR PURPOSE.
*
*
*     Xilinx hardware products are not intended for use in life support
*     appliances, devices, or systems. Use in such applications is
*     expressly prohibited.
*
*
*     (c) Copyright 2002-2004 Xilinx Inc.
*     All rights reserved.
*
*
*     You should have received a copy of the GNU General Public License along
*     with this program; if not, write to the Free Software Foundation, Inc.,
*     675 Mass Ave, Cambridge, MA 02139, USA.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xemac.h
*
* The Xilinx Ethernet driver component.	 This component supports the Xilinx
* Ethernet 10/100 MAC (EMAC).
*
* The Xilinx Ethernet 10/100 MAC supports the following features:
*   - Simple and scatter-gather DMA operations, as well as simple memory
*     mapped direct I/O interface (FIFOs).
*   - Media Independent Interface (MII) for connection to external
*     10/100 Mbps PHY transceivers.
*   - MII management control reads and writes with MII PHYs
*   - Independent internal transmit and receive FIFOs
*   - CSMA/CD compliant operations for half-duplex modes
*   - Programmable PHY reset signal
*   - Unicast, broadcast, and promiscuous address filtering (no multicast yet)
*   - Internal loopback
*   - Automatic source address insertion or overwrite (programmable)
*   - Automatic FCS insertion and stripping (programmable)
*   - Automatic pad insertion and stripping (programmable)
*   - Pause frame (flow control) detection in full-duplex mode
*   - Programmable interframe gap
*   - VLAN frame support.
*   - Pause frame support
*
* The device driver supports all the features listed above.
*
* <b>Driver Description</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the EMAC. The driver handles transmission and reception of
* Ethernet frames, as well as configuration of the controller. It does not
* handle protocol stack functionality such as Link Layer Control (LLC) or the
* Address Resolution Protocol (ARP). The protocol stack that makes use of the
* driver handles this functionality. This implies that the driver is simply a
* pass-through mechanism between a protocol stack and the EMAC. A single device
* driver can support multiple EMACs.
*
* The driver is designed for a zero-copy buffer scheme. That is, the driver will
* not copy buffers. This avoids potential throughput bottlenecks within the
* driver.
*
* Since the driver is a simple pass-through mechanism between a protocol stack
* and the EMAC, no assembly or disassembly of Ethernet frames is done at the
* driver-level. This assumes that the protocol stack passes a correctly
* formatted Ethernet frame to the driver for transmission, and that the driver
* does not validate the contents of an incoming frame
*
* <b>PHY Communication</b>
*
* The driver provides rudimentary read and write functions to allow the higher
* layer software to access the PHY. The EMAC provides MII registers for the
* driver to access. This management interface can be parameterized away in the
* FPGA implementation process. If this is the case, the PHY read and write
* functions of the driver return XST_NO_FEATURE.
*
* External loopback is usually supported at the PHY. It is up to the user to
* turn external loopback on or off at the PHY. The driver simply provides pass-
* through functions for configuring the PHY. The driver does not read, write,
* or reset the PHY on its own. All control of the PHY must be done by the user.
*
* <b>Asynchronous Callbacks</b>
*
* The driver services interrupts and passes Ethernet frames to the higher layer
* software through asynchronous callback functions. When using the driver
* directly (i.e., not with the RTOS protocol stack), the higher layer
* software must register its callback functions during initialization. The
* driver requires callback functions for received frames, for confirmation of
* transmitted frames, and for asynchronous errors.
*
* <b>Interrupts</b>
*
* The driver has no dependencies on the interrupt controller. The driver
* provides two interrupt handlers.  XEmac_IntrHandlerDma() handles interrupts
* when the EMAC is configured with scatter-gather DMA.	XEmac_IntrHandlerFifo()
* handles interrupts when the EMAC is configured for direct FIFO I/O or simple
* DMA.	Either of these routines can be connected to the system interrupt
* controller by the user.
*
* <b>Interrupt Frequency</b>
*
* When the EMAC is configured with scatter-gather DMA, the frequency of
* interrupts can be controlled with the interrupt coalescing features of the
* scatter-gather DMA engine. The frequency of interrupts can be adjusted using
* the driver API functions for setting the packet count threshold and the packet
* wait bound values.
*
* The scatter-gather DMA engine only interrupts when the packet count threshold
* is reached, instead of interrupting for each packet. A packet is a generic
* term used by the scatter-gather DMA engine, and is equivalent to an Ethernet
* frame in our case.
*
* The packet wait bound is a timer value used during interrupt coalescing to
* trigger an interrupt when not enough packets have been received to reach the
* packet count threshold.
*
* These values can be tuned by the user to meet their needs. If there appear to
* be interrupt latency problems or delays in packet arrival that are longer than
* might be expected, the user should verify that the packet count threshold is
* set low enough to receive interrupts before the wait bound timer goes off.
*
* <b>Device Reset</b>
*
* Some errors that can occur in the device require a device reset. These errors
* are listed in the XEmac_SetErrorHandler() function header. The user's error
* handler is responsible for resetting the device and re-configuring it based on
* its needs (the driver does not save the current configuration). When
* integrating into an RTOS, these reset and re-configure obligations are
* taken care of by the Xilinx adapter software if it exists for that RTOS.
*
* <b>Device Configuration</b>
*
* The device can be configured in various ways during the FPGA implementation
* process.  Configuration parameters are stored in the xemac_g.c files.
* A table is defined where each entry contains configuration information
* for an EMAC device.  This information includes such things as the base address
* of the memory-mapped device, the base addresses of IPIF, DMA, and FIFO modules
* within the device, and whether the device has DMA, counter registers,
* multicast support, MII support, and flow control.
*
* The driver tries to use the features built into the device. So if, for
* example, the hardware is configured with scatter-gather DMA, the driver
* expects to start the scatter-gather channels and expects that the user has set
* up the buffer descriptor lists already. If the user expects to use the driver
* in a mode different than how the hardware is configured, the user should
* modify the configuration table to reflect the mode to be used. Modifying the
* configuration table is a workaround for now until we get some experience with
* how users are intending to use the hardware in its different configurations.
* For example, if the hardware is built with scatter-gather DMA but the user is
* intending to use only simple DMA, the user either needs to modify the config
* table as a workaround or rebuild the hardware with only simple DMA. The
* recommendation at this point is to build the hardware with the features you
* intend to use. If you're inclined to modify the table, do so before the call
* to XEmac_Initialize().  Here is a snippet of code that changes a device to
* simple DMA (the hardware needs to have DMA for this to work of course):
* <pre>
*	 XEmac_Config *ConfigPtr;
*
*	 ConfigPtr = XEmac_LookupConfig(DeviceId);
*	 ConfigPtr->IpIfDmaConfig = XEM_CFG_SIMPLE_DMA;
* </pre>
*
* <b>Simple DMA</b>
*
* Simple DMA is supported through the FIFO functions, FifoSend and FifoRecv, of
* the driver (i.e., there is no separate interface for it). The driver makes use
* of the DMA engine for a simple DMA transfer if the device is configured with
* DMA, otherwise it uses the FIFOs directly. While the simple DMA interface is
* therefore transparent to the user, the caching of network buffers is not.
* If the device is configured with DMA and the FIFO interface is used, the user
* must ensure that the network buffers are not cached or are cache coherent,
* since DMA will be used to transfer to and from the Emac device. If the device
* is configured with DMA and the user really wants to use the FIFOs directly,
* the user should rebuild the hardware without DMA. If unable to do this, there
* is a workaround (described above in Device Configuration) to modify the
* configuration table of the driver to fake the driver into thinking the device
* has no DMA. A code snippet follows:
* <pre>
*	 XEmac_Config *ConfigPtr;
*
*	 ConfigPtr = XEmac_LookupConfig(DeviceId);
*	 ConfigPtr->IpIfDmaConfig = XEM_CFG_NO_DMA;
* </pre>
*
* <b>Asserts</b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b>Building the driver</b>
*
* The XEmac driver is composed of several source files. Why so many?  This
* allows the user to build and link only those parts of the driver that are
* necessary. Since the EMAC hardware can be configured in various ways (e.g.,
* with or without DMA), the driver too can be built with varying features.
* For the most part, this means that besides always linking in xemac.c, you
* link in only the driver functionality you want. Some of the choices you have
* are polled vs. interrupt, interrupt with FIFOs only vs. interrupt with DMA,
* self-test diagnostics, and driver statistics. Note that currently the DMA code
* must be linked in, even if you don't have DMA in the device.
*
* @note
*
* Xilinx drivers are typically composed of two components, one is the driver
* and the other is the adapter.	 The driver is independent of OS and processor
* and is intended to be highly portable.  The adapter is OS-specific and
* facilitates communication between the driver and an OS.
* <br><br>
* This driver is intended to be RTOS and processor independent.	 It works
* with physical addresses only.	 Any needs for dynamic memory management,
* threads or thread mutual exclusion, virtual memory, or cache control must
* be satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rpm  07/31/01 First release
* 1.00b rpm  02/20/02 Repartitioned files and functions
* 1.00b rpm  10/08/02 Replaced HasSgDma boolean with IpifDmaConfig enumerated
*		      configuration parameter
* 1.00c rpm  12/05/02 New version includes support for simple DMA and the delay
*		      argument to SgSend
* 1.00c rpm  02/03/03 The XST_DMA_SG_COUNT_EXCEEDED return code was removed
*		      from SetPktThreshold in the internal DMA driver. Also
*		      avoided compiler warnings by initializing Result in the
*		      DMA interrupt service routines.
* </pre>
*
******************************************************************************/

#ifndef XEMAC_H			/* prevent circular inclusions */
#define XEMAC_H			/* by using protection macros */

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#include "xstatus.h"
#include "xparameters.h"
#include "xpacket_fifo_v1_00_b.h"	/* Uses v1.00b of Packet Fifo */
#include "xdma_channel.h"

/************************** Constant Definitions *****************************/

/*
 * Device information
 */
#define XEM_DEVICE_NAME	    "xemac"
#define XEM_DEVICE_DESC	    "Xilinx Ethernet 10/100 MAC"

/** @name Configuration options
 *
 * Device configuration options (see the XEmac_SetOptions() and
 * XEmac_GetOptions() for information on how to use these options)
 * @{
 */
/**
 * <pre>
 *   XEM_BROADCAST_OPTION	 Broadcast addressing on or off (default is on)
 *   XEM_UNICAST_OPTION		 Unicast addressing on or off (default is on)
 *   XEM_PROMISC_OPTION		 Promiscuous addressing on or off (default is off)
 *   XEM_FDUPLEX_OPTION		 Full duplex on or off (default is off)
 *   XEM_POLLED_OPTION		 Polled mode on or off (default is off)
 *   XEM_LOOPBACK_OPTION	 Internal loopback on or off (default is off)
 *   XEM_FLOW_CONTROL_OPTION	 Interpret pause frames in full duplex mode
 *				 (default is off)
 *   XEM_INSERT_PAD_OPTION	 Pad short frames on transmit (default is on)
 *   XEM_INSERT_FCS_OPTION	 Insert FCS (CRC) on transmit (default is on)
 *   XEM_INSERT_ADDR_OPTION	 Insert source address on transmit (default is on)
 *   XEM_OVWRT_ADDR_OPTION	 Overwrite source address on transmit. This is
 *				 only used if source address insertion is on.
 *				 (default is on)
 *   XEM_STRIP_PAD_FCS_OPTION	 Strip FCS and padding from received frames
 *				 (default is off)
  * </pre>
 */
#define XEM_UNICAST_OPTION	  0x00000001UL
#define XEM_BROADCAST_OPTION	  0x00000002UL
#define XEM_PROMISC_OPTION	  0x00000004UL
#define XEM_FDUPLEX_OPTION	  0x00000008UL
#define XEM_POLLED_OPTION	  0x00000010UL
#define XEM_LOOPBACK_OPTION	  0x00000020UL
#define XEM_FLOW_CONTROL_OPTION	  0x00000080UL
#define XEM_INSERT_PAD_OPTION	  0x00000100UL
#define XEM_INSERT_FCS_OPTION	  0x00000200UL
#define XEM_INSERT_ADDR_OPTION	  0x00000400UL
#define XEM_OVWRT_ADDR_OPTION	  0x00000800UL
#define XEM_STRIP_PAD_FCS_OPTION  0x00002000UL
/*@}*/
/*
 * Not supported yet:
 *   XEM_MULTICAST_OPTION	 Multicast addressing on or off (default is off)
 */
/* NOT SUPPORTED YET... */
#define XEM_MULTICAST_OPTION	  0x00000040UL

/*
 * Some default values for interrupt coalescing within the scatter-gather
 * DMA engine.
 */
#define XEM_SGDMA_DFT_THRESHOLD	    1	/* Default pkt threshold */
#define XEM_SGDMA_MAX_THRESHOLD	    255 /* Maximum pkt theshold */
#define XEM_SGDMA_DFT_WAITBOUND	    5	/* Default pkt wait bound (msec) */
#define XEM_SGDMA_MAX_WAITBOUND	    1023	/* Maximum pkt wait bound (msec) */

/*
 * Direction identifiers. These are used for setting values like packet
 * thresholds and wait bound for specific channels
 */
#define XEM_SEND    1
#define XEM_RECV    2

/*
 * Arguments to SgSend function to indicate whether to hold off starting
 * the scatter-gather engine.
 */
#define XEM_SGDMA_NODELAY     0 /* start SG DMA immediately */
#define XEM_SGDMA_DELAY	      1 /* do not start SG DMA */

/*
 * Constants to determine the configuration of the hardware device. They are
 * used to allow the driver to verify it can operate with the hardware.
 */
#define XEM_CFG_NO_IPIF		    0	/* Not supported by the driver */
#define XEM_CFG_NO_DMA		    1	/* No DMA */
#define XEM_CFG_SIMPLE_DMA	    2	/* Simple DMA */
#define XEM_CFG_DMA_SG		    3	/* DMA scatter gather */

/*
 * The next few constants help upper layers determine the size of memory
 * pools used for Ethernet buffers and descriptor lists.
 */
#define XEM_MAC_ADDR_SIZE   6	/* six-byte MAC address */
#define XEM_MTU		    1500	/* max size of Ethernet frame */
#define XEM_HDR_SIZE	    14	/* size of Ethernet header */
#define XEM_HDR_VLAN_SIZE   18	/* size of Ethernet header with VLAN */
#define XEM_TRL_SIZE	    4	/* size of Ethernet trailer (FCS) */
#define XEM_MAX_FRAME_SIZE  (XEM_MTU + XEM_HDR_SIZE + XEM_TRL_SIZE)
#define XEM_MAX_VLAN_FRAME_SIZE	 (XEM_MTU + XEM_HDR_VLAN_SIZE + XEM_TRL_SIZE)

/*
 * Define a default number of send and receive buffers
 */
#define XEM_MIN_RECV_BUFS   32	/* minimum # of recv buffers */
#define XEM_DFT_RECV_BUFS   64	/* default # of recv buffers */

#define XEM_MIN_SEND_BUFS   16	/* minimum # of send buffers */
#define XEM_DFT_SEND_BUFS   32	/* default # of send buffers */

#define XEM_MIN_BUFFERS	    (XEM_MIN_RECV_BUFS + XEM_MIN_SEND_BUFS)
#define XEM_DFT_BUFFERS	    (XEM_DFT_RECV_BUFS + XEM_DFT_SEND_BUFS)

/*
 * Define the number of send and receive buffer descriptors, used for
 * scatter-gather DMA
 */
#define XEM_MIN_RECV_DESC   16	/* minimum # of recv descriptors */
#define XEM_DFT_RECV_DESC   32	/* default # of recv descriptors */

#define XEM_MIN_SEND_DESC   8	/* minimum # of send descriptors */
#define XEM_DFT_SEND_DESC   16	/* default # of send descriptors */

/**************************** Type Definitions *******************************/

/**
 * Ethernet statistics (see XEmac_GetStats() and XEmac_ClearStats())
 */
typedef struct {
	u32 XmitFrames;		 /**< Number of frames transmitted */
	u32 XmitBytes;		 /**< Number of bytes transmitted */
	u32 XmitLateCollisionErrors;
				 /**< Number of transmission failures
					  due to late collisions */
	u32 XmitExcessDeferral;	 /**< Number of transmission failures
					  due o excess collision deferrals */
	u32 XmitOverrunErrors;	 /**< Number of transmit overrun errors */
	u32 XmitUnderrunErrors;	 /**< Number of transmit underrun errors */
	u32 RecvFrames;		 /**< Number of frames received */
	u32 RecvBytes;		 /**< Number of bytes received */
	u32 RecvFcsErrors;	 /**< Number of frames discarded due
					  to FCS errors */
	u32 RecvAlignmentErrors; /**< Number of frames received with
					  alignment errors */
	u32 RecvOverrunErrors;	 /**< Number of frames discarded due
					  to overrun errors */
	u32 RecvUnderrunErrors;	 /**< Number of recv underrun errors */
	u32 RecvMissedFrameErrors;
				 /**< Number of frames missed by MAC */
	u32 RecvCollisionErrors; /**< Number of frames discarded due
					  to collisions */
	u32 RecvLengthFieldErrors;
				 /**< Number of frames discarded with
					  invalid length field */
	u32 RecvShortErrors;	 /**< Number of short frames discarded */
	u32 RecvLongErrors;	 /**< Number of long frames discarded */
	u32 DmaErrors;		 /**< Number of DMA errors since init */
	u32 FifoErrors;		 /**< Number of FIFO errors since init */
	u32 RecvInterrupts;	 /**< Number of receive interrupts */
	u32 XmitInterrupts;	 /**< Number of transmit interrupts */
	u32 EmacInterrupts;	 /**< Number of MAC (device) interrupts */
	u32 TotalIntrs;		 /**< Total interrupts */
} XEmac_Stats;

/**
 * This typedef contains configuration information for a device.
 */
typedef struct {
	u16 DeviceId;	    /**< Unique ID  of device */
	u32 BaseAddress;    /**< Register base address */
	u32 HasCounters;   /**< Does device have counters? */
	u8 IpIfDmaConfig;   /**< IPIF/DMA hardware configuration */
	u32 HasMii;	   /**< Does device support MII? */

} XEmac_Config;

/** @name Typedefs for callbacks
 * Callback functions.
 * @{
 */
/**
 * Callback when data is sent or received with scatter-gather DMA.
 *
 * @param CallBackRef is a callback reference passed in by the upper layer
 *	  when setting the callback functions, and passed back to the upper
 *	  layer when the callback is invoked.
 * @param BdPtr is a pointer to the first buffer descriptor in a list of
 *	  buffer descriptors.
 * @param NumBds is the number of buffer descriptors in the list pointed
 *	  to by BdPtr.
 */
typedef void (*XEmac_SgHandler) (void *CallBackRef, XBufDescriptor * BdPtr,
				 u32 NumBds);

/**
 * Callback when data is sent or received with direct FIFO communication or
 * simple DMA. The user typically defines two callacks, one for send and one
 * for receive.
 *
 * @param CallBackRef is a callback reference passed in by the upper layer
 *	  when setting the callback functions, and passed back to the upper
 *	  layer when the callback is invoked.
 */
typedef void (*XEmac_FifoHandler) (void *CallBackRef);

/**
 * Callback when an asynchronous error occurs.
 *
 * @param CallBackRef is a callback reference passed in by the upper layer
 *	  when setting the callback functions, and passed back to the upper
 *	  layer when the callback is invoked.
 * @param ErrorCode is a Xilinx error code defined in xstatus.h.  Also see
 *	  XEmac_SetErrorHandler() for a description of possible errors.
 */
typedef void (*XEmac_ErrorHandler) (void *CallBackRef, XStatus ErrorCode);
/*@}*/

/**
 * The XEmac driver instance data. The user is required to allocate a
 * variable of this type for every EMAC device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	u32 BaseAddress;	/* Base address (of IPIF) */
	u32 IsStarted;		/* Device is currently started */
	u32 IsReady;		/* Device is initialized and ready */
	u32 IsPolled;		/* Device is in polled mode */
	u8 IpIfDmaConfig;	/* IPIF/DMA hardware configuration */
	u32 HasMii;		/* Does device support MII? */
	u32 HasMulticastHash;	/* Does device support multicast hash table? */

	XEmac_Stats Stats;
	XPacketFifoV100b RecvFifo;	/* FIFO used to receive frames */
	XPacketFifoV100b SendFifo;	/* FIFO used to send frames */

	/*
	 * Callbacks
	 */
	XEmac_FifoHandler FifoRecvHandler;	/* for non-DMA/simple DMA interrupts */
	void *FifoRecvRef;
	XEmac_FifoHandler FifoSendHandler;	/* for non-DMA/simple DMA interrupts */
	void *FifoSendRef;
	XEmac_ErrorHandler ErrorHandler;	/* for asynchronous errors */
	void *ErrorRef;

	XDmaChannel RecvChannel;	/* DMA receive channel driver */
	XDmaChannel SendChannel;	/* DMA send channel driver */

	XEmac_SgHandler SgRecvHandler;	/* callback for scatter-gather DMA */
	void *SgRecvRef;
	XEmac_SgHandler SgSendHandler;	/* callback for scatter-gather DMA */
	void *SgSendRef;
} XEmac;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro determines if the device is currently configured for
* scatter-gather DMA.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
*
* @return
*
* Boolean TRUE if the device is configured for scatter-gather DMA, or FALSE
* if it is not.
*
* @note
*
* Signature: u32 XEmac_mIsSgDma(XEmac *InstancePtr)
*
******************************************************************************/
#define XEmac_mIsSgDma(InstancePtr) \
	    ((InstancePtr)->IpIfDmaConfig == XEM_CFG_DMA_SG)

/*****************************************************************************/
/**
*
* This macro determines if the device is currently configured for simple DMA.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
*
* @return
*
* Boolean TRUE if the device is configured for simple DMA, or FALSE otherwise
*
* @note
*
* Signature: u32 XEmac_mIsSimpleDma(XEmac *InstancePtr)
*
******************************************************************************/
#define XEmac_mIsSimpleDma(InstancePtr) \
	    ((InstancePtr)->IpIfDmaConfig == XEM_CFG_SIMPLE_DMA)

/*****************************************************************************/
/**
*
* This macro determines if the device is currently configured with DMA (either
* simple DMA or scatter-gather DMA)
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
*
* @return
*
* Boolean TRUE if the device is configured with DMA, or FALSE otherwise
*
* @note
*
* Signature: u32 XEmac_mIsDma(XEmac *InstancePtr)
*
******************************************************************************/
#define XEmac_mIsDma(InstancePtr) \
	    (XEmac_mIsSimpleDma(InstancePtr) || XEmac_mIsSgDma(InstancePtr))

/************************** Function Prototypes ******************************/

/*
 * Initialization functions in xemac.c
 */
XStatus XEmac_Initialize(XEmac * InstancePtr, u16 DeviceId);
XStatus XEmac_Start(XEmac * InstancePtr);
XStatus XEmac_Stop(XEmac * InstancePtr);
void XEmac_Reset(XEmac * InstancePtr);
XEmac_Config *XEmac_LookupConfig(u16 DeviceId);

/*
 * Diagnostic functions in xemac_selftest.c
 */
XStatus XEmac_SelfTest(XEmac * InstancePtr);

/*
 * Polled functions in xemac_polled.c
 */
XStatus XEmac_PollSend(XEmac * InstancePtr, u8 * BufPtr, u32 ByteCount);
XStatus XEmac_PollRecv(XEmac * InstancePtr, u8 * BufPtr, u32 * ByteCountPtr);

/*
 * Interrupts with scatter-gather DMA functions in xemac_intr_dma.c
 */
XStatus XEmac_SgSend(XEmac * InstancePtr, XBufDescriptor * BdPtr, int Delay);
XStatus XEmac_SgRecv(XEmac * InstancePtr, XBufDescriptor * BdPtr);
XStatus XEmac_SetPktThreshold(XEmac * InstancePtr, u32 Direction, u8 Threshold);
XStatus XEmac_GetPktThreshold(XEmac * InstancePtr, u32 Direction,
			      u8 * ThreshPtr);
XStatus XEmac_SetPktWaitBound(XEmac * InstancePtr, u32 Direction,
			      u32 TimerValue);
XStatus XEmac_GetPktWaitBound(XEmac * InstancePtr, u32 Direction,
			      u32 * WaitPtr);
XStatus XEmac_SetSgRecvSpace(XEmac * InstancePtr, u32 * MemoryPtr,
			     u32 ByteCount);
XStatus XEmac_SetSgSendSpace(XEmac * InstancePtr, u32 * MemoryPtr,
			     u32 ByteCount);
void XEmac_SetSgRecvHandler(XEmac * InstancePtr, void *CallBackRef,
			    XEmac_SgHandler FuncPtr);
void XEmac_SetSgSendHandler(XEmac * InstancePtr, void *CallBackRef,
			    XEmac_SgHandler FuncPtr);

void XEmac_IntrHandlerDma(void *InstancePtr);	/* interrupt handler */

/*
 * Interrupts with direct FIFO functions in xemac_intr_fifo.c. Also used
 * for simple DMA.
 */
XStatus XEmac_FifoSend(XEmac * InstancePtr, u8 * BufPtr, u32 ByteCount);
XStatus XEmac_FifoRecv(XEmac * InstancePtr, u8 * BufPtr, u32 * ByteCountPtr);
void XEmac_SetFifoRecvHandler(XEmac * InstancePtr, void *CallBackRef,
			      XEmac_FifoHandler FuncPtr);
void XEmac_SetFifoSendHandler(XEmac * InstancePtr, void *CallBackRef,
			      XEmac_FifoHandler FuncPtr);

void XEmac_IntrHandlerFifo(void *InstancePtr);	/* interrupt handler */

/*
 * General interrupt-related functions in xemac_intr.c
 */
void XEmac_SetErrorHandler(XEmac * InstancePtr, void *CallBackRef,
			   XEmac_ErrorHandler FuncPtr);

/*
 * MAC configuration in xemac_options.c
 */
XStatus XEmac_SetOptions(XEmac * InstancePtr, u32 OptionFlag);
u32 XEmac_GetOptions(XEmac * InstancePtr);
XStatus XEmac_SetMacAddress(XEmac * InstancePtr, u8 * AddressPtr);
void XEmac_GetMacAddress(XEmac * InstancePtr, u8 * BufferPtr);
XStatus XEmac_SetInterframeGap(XEmac * InstancePtr, u8 Part1, u8 Part2);
void XEmac_GetInterframeGap(XEmac * InstancePtr, u8 * Part1Ptr, u8 * Part2Ptr);

/*
 * Multicast functions in xemac_multicast.c (not supported by EMAC yet)
 */
XStatus XEmac_MulticastAdd(XEmac * InstancePtr, u8 * AddressPtr);
XStatus XEmac_MulticastClear(XEmac * InstancePtr);

/*
 * PHY configuration in xemac_phy.c
 */
XStatus XEmac_PhyRead(XEmac * InstancePtr, u32 PhyAddress,
		      u32 RegisterNum, u16 * PhyDataPtr);
XStatus XEmac_PhyWrite(XEmac * InstancePtr, u32 PhyAddress,
		       u32 RegisterNum, u16 PhyData);

/*
 * Statistics in xemac_stats.c
 */
void XEmac_GetStats(XEmac * InstancePtr, XEmac_Stats * StatsPtr);
void XEmac_ClearStats(XEmac * InstancePtr);

#endif				/* end of protection macro */
