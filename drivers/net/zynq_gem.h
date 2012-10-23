/*
 * (C) Copyright 2012 Xilinx
 *
 * The Xilinx Embedded Processor Block Ethernet driver.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef XEMACPSS_H		/* prevent circular inclusions */
#define XEMACPSS_H		/* by using protection macros */

/**
 * For a full description of XEMACPSS features, please see the hardware spec.
 * This driver supports the following features:
 *   - Memory mapped access to host interface registers
 *   - Statistics counter registers for RMON/MIB
 *   - API for interrupt driven frame transfers for hardware configured DMA 
 *   - Virtual memory support
 *   - Unicast, broadcast, and multicast receive address filtering
 *   - Full and half duplex operation
 *   - Automatic PAD & FCS insertion and stripping
 *   - Flow control
 *   - Support up to four 48bit addresses
 *   - Address checking for four specific 48bit addresses
 *   - VLAN frame support
 *   - Pause frame support
 *   - Large frame support up to 1536 bytes
 *   - Checksum offload
 *
 * Driver Description:
 *
 * The device driver enables higher layer software (e.g., an application) to
 * communicate to the XEmacPss. The driver handles transmission and reception
 * of Ethernet frames, as well as configuration and control. No pre or post
 * processing of frame data is performed. The driver does not validate the
 * contents of an incoming frame in addition to what has already occurred in
 * hardware.
 * A single device driver can support multiple devices even when those devices
 * have significantly different configurations.
 *
 * Initialization & Configuration:
 *
 * The XEmacPss_Config structure is used by the driver to configure itself.
 * This configuration structure is typically created by the tool-chain based
 * on hardware build properties.
 *
 * The driver instance can be initialized in 
 *
 *   - XEmacPss_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddress):  Uses a 
 *     configuration structure provided by the caller. If running in a system
 *     with address translation, the provided virtual memory base address 
 *     replaces the physical address present in the configuration structure.
 *
 * The device supports DMA only as current development plan. No FIFO mode is
 * supported. The driver expects to start the DMA channels and expects that
 * the user has set up the buffer descriptor lists.
 *
 * Interrupts and Asynchronous Callbacks:
 *
 * The driver has no dependencies on the interrupt controller. When an
 * interrupt occurs, the handler will perform a small amount of
 * housekeeping work, determine the source of the interrupt, and call the
 * appropriate callback function. All callbacks are registered by the user
 * level application.
 *
 * Virtual Memory:
 *
 * All virtual to physical memory mappings must occur prior to accessing the
 * driver API.
 *
 * For DMA transactions, user buffers supplied to the driver must be in terms
 * of their physical address.
 *
 * DMA:
 *
 * The DMA engine uses buffer descriptors (BDs) to describe Ethernet frames.
 * These BDs are typically chained together into a list the hardware follows
 * when transferring data in and out of the packet buffers. Each BD describes
 * a memory region containing either a full or partial Ethernet packet.
 *
 * Interrupt coalescing is not suppoted from this built-in DMA engine.
 *
 * This API requires the user to understand how the DMA operates. The
 * following paragraphs provide some explanation, but the user is encouraged
 * to read documentation in zynq_gem_bdring.h as well as study example code
 * that accompanies this driver.
 *
 * The API is designed to get BDs to and from the DMA engine in the most
 * efficient means possible. The first step is to establish a  memory region
 * to contain all BDs for a specific channel. This is done with
 * XEmacPss_BdRingCreate(). This function sets up a BD ring that hardware will
 * follow as BDs are processed. The ring will consist of a user defined number
 * of BDs which will all be partially initialized. For example on the transmit
 * channel, the driver will initialize all BDs' so that they are configured
 * for transmit. The more fields that can be permanently setup at
 * initialization, then the fewer accesses will be needed to each BD while
 * the DMA engine is in operation resulting in better throughput and CPU
 * utilization. The best case initialization would require the user to set
 * only a frame buffer address and length prior to submitting the BD to the
 * engine.
 *
 * BDs move through the engine with the help of functions
 * XEmacPss_BdRingAlloc(), XEmacPss_BdRingToHw(), XEmacPss_BdRingFromHw(),
 * and XEmacPss_BdRingFree().
 * All these functions handle BDs that are in place. That is, there are no
 * copies of BDs kept anywhere and any BD the user interacts with is an actual
 * BD from the same ring hardware accesses.
 *
 * BDs in the ring go through a series of states as follows:
 *   1. Idle. The driver controls BDs in this state.
 *   2. The user has data to transfer. XEmacPss_BdRingAlloc() is called to
 *      reserve BD(s). Once allocated, the user may setup the BD(s) with
 *      frame buffer address, length, and other attributes. The user controls
 *      BDs in this state.
 *   3. The user submits BDs to the DMA engine with XEmacPss_BdRingToHw. BDs
 *      in this state are either waiting to be processed by hardware, are in
 *      process, or have been processed. The DMA engine controls BDs in this
 *      state.
 *   4. Processed BDs are retrieved with XEmacEpv_BdRingFromHw() by the
 *      user. Once retrieved, the user can examine each BD for the outcome of
 *      the DMA transfer. The user controls BDs in this state. After examining
 *      the BDs the user calls XEmacPss_BdRingFree() which places the BDs back
 *      into state 1.
 *
 * Each of the four BD accessor functions operate on a set of BDs. A set is
 * defined as a segment of the BD ring consisting of one or more BDs. The user
 * views the set as a pointer to the first BD along with the number of BDs for
 * that set. The set can be navigated by using macros XEmacPss_BdNext(). The
 * user must exercise extreme caution when changing BDs in a set as there is
 * nothing to prevent doing a mBdNext past the end of the set and modifying a
 * BD out of bounds.
 *
 * XEmacPss_BdRingAlloc() + XEmacPss_BdRingToHw(), as well as
 * XEmacPss_BdRingFromHw() + XEmacPss_BdRingFree() are designed to be used in
 * tandem. The same BD set retrieved with BdRingAlloc should be the same one
 * provided to hardware with BdRingToHw. Same goes with BdRingFromHw and
 * BdRIngFree.
 *
 * Alignment & Data Cache Restrictions:
 *
 * Due to the design of the hardware, all RX buffers, BDs need to be 4-byte
 * aligned. Please reference zynq_gem_bd.h for cache related macros.
 *
 * DMA Tx:
 *
 *   - If frame buffers exist in cached memory, then they must be flushed
 *     prior to committing them to hardware.
 *
 * DMA Rx:
 *
 *   - If frame buffers exist in cached memory, then the cache must be
 *     invalidated for the memory region containing the frame prior to data
 *     access
 *
 * Both cache invalidate/flush are taken care of in driver code.
 *
 * Buffer Copying:
 *
 * The driver is designed for a zero-copy buffer scheme. That is, the driver
 * will not copy buffers. This avoids potential throughput bottlenecks within
 * the driver. If byte copying is required, then the transfer will take longer
 * to complete.
 *
 * Checksum Offloading:
 *
 * The Embedded Processor Block Ethernet can be configured to perform IP, TCP
 * and UDP checksum offloading in both receive and transmit directions.
 *
 * IP packets contain a 16-bit checksum field, which is the 16-bit 1s
 * complement of the 1s complement sum of all 16-bit words in the header.
 * TCP and UDP packets contain a 16-bit checksum field, which is the 16-bit
 * 1s complement of the 1s complement sum of all 16-bit words in the header,
 * the data and a conceptual pseudo header.
 *
 * To calculate these checksums in software requires each byte of the packet
 * to be read. For TCP and UDP this can use a large amount of processing power.
 * Offloading the checksum calculation to hardware can result in significant
 * performance improvements.
 *
 * The transmit checksum offload is only available to use DMA in packet buffer
 * mode. This is because the complete frame to be transmitted must be read
 * into the packet buffer memory before the checksum can be calculated and
 * written to the header at the beginning of the frame.
 *
 * For IP, TCP or UDP receive checksum offload to be useful, the operating
 * system containing the protocol stack must be aware that this offload is
 * available so that it can make use of the fact that the hardware has verified
 * the checksum. 
 * 
 * When receive checksum offloading is enabled in the hardware, the IP header
 * checksum is checked, where the packet meets the following criteria: 
 *
 * 1. If present, the VLAN header must be four octets long and the CFI bit
 *    must not be set.
 * 2. Encapsulation must be RFC 894 Ethernet Type Encoding or RFC 1042 SNAP
 *    encoding.
 * 3. IP v4 packet.
 * 4. IP header is of a valid length.
 * 5. Good IP header checksum.
 * 6. No IP fragmentation.
 * 7. TCP or UDP packet.
 *
 * When an IP, TCP or UDP frame is received, the receive buffer descriptor
 * gives an indication if the hardware was able to verify the checksums.
 * There is also an indication if the frame had SNAP encapsulation. These
 * indication bits will replace the type ID match indication bits when the
 * receive checksum offload is enabled.
 *
 * If any of the checksums are verified incorrect by the hardware, the packet
 * is discarded and the appropriate statistics counter incremented.
 *
 * PHY Interfaces:
 *
 * RGMII 1.3 is the only interface supported.
 *
 * Asserts:
 *
 * Asserts are used within all Xilinx drivers to enforce constraints on
 * parameters. Asserts can be turned off on a system-wide basis by defining,
 * at compile time, the NDEBUG identifier. By default, asserts are turned on
 * and it is recommended that users leave asserts on during development. For
 * deployment use -DNDEBUG compiler switch to remove assert code.
 *
 * @note
 *
 * Xilinx drivers are typically composed of two parts, one is the driver
 * and the other is the adapter.  The driver is independent of OS and processor
 * and is intended to be highly portable.  The adapter is OS-specific and
 * facilitates communication between the driver and an OS.
 * This driver is intended to be RTOS and processor independent. Any needs for
 * dynamic memory management, threads or thread mutual exclusion, or cache
 * control must be satisfied bythe layer above this driver.
 */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xbasic_types.h"
#include "xstatus.h"
#include "zynq_gem_hw.h"
#include "zynq_gem_bd.h"
#include "zynq_gem_bdring.h"

/************************** Constant Definitions ****************************/

/*
 * Device information
 */
#define XEMACPSS_DEVICE_NAME     "xemacps"
#define XEMACPSS_DEVICE_DESC     "Xilinx PSS 10/100/1000 MAC"

/** @name Configuration options
 *
 * Device configuration options. See the XEmacPss_SetOptions(),
 * XEmacPss_ClearOptions() and XEmacPss_GetOptions() for information on how to
 * use options.
 *
 * The default state of the options are noted and are what the device and
 * driver will be set to after calling XEmacPss_Reset() or
 * XEmacPss_Initialize().
 *
 * @{
 */

#define XEMACPSS_PROMISC_OPTION               0x00000001
/**< Accept all incoming packets.
 *   This option defaults to disabled (cleared) */

#define XEMACPSS_FRAME1536_OPTION             0x00000002
/**< Frame larger than 1516 support for Tx & Rx.
 *   This option defaults to disabled (cleared) */

#define XEMACPSS_VLAN_OPTION                  0x00000004
/**< VLAN Rx & Tx frame support.
 *   This option defaults to disabled (cleared) */

#define XEMACPSS_FLOW_CONTROL_OPTION          0x00000010
/**< Enable recognition of flow control frames on Rx
 *   This option defaults to enabled (set) */

#define XEMACPSS_FCS_STRIP_OPTION             0x00000020
/**< Strip FCS and PAD from incoming frames. Note: PAD from VLAN frames is not
 *   stripped.
 *   This option defaults to enabled (set) */

#define XEMACPSS_FCS_INSERT_OPTION            0x00000040
/**< Generate FCS field and add PAD automatically for outgoing frames.
 *   This option defaults to disabled (cleared) */

#define XEMACPSS_LENTYPE_ERR_OPTION           0x00000080
/**< Enable Length/Type error checking for incoming frames. When this option is
 *   set, the MAC will filter frames that have a mismatched type/length field
 *   and if XEMACPSS_REPORT_RXERR_OPTION is set, the user is notified when these
 *   types of frames are encountered. When this option is cleared, the MAC will
 *   allow these types of frames to be received.
 *
 *   This option defaults to disabled (cleared) */

#define XEMACPSS_TRANSMITTER_ENABLE_OPTION    0x00000100
/**< Enable the transmitter.
 *   This option defaults to enabled (set) */

#define XEMACPSS_RECEIVER_ENABLE_OPTION       0x00000200
/**< Enable the receiver
 *   This option defaults to enabled (set) */

#define XEMACPSS_BROADCAST_OPTION             0x00000400
/**< Allow reception of the broadcast address
 *   This option defaults to enabled (set) */

#define XEMACPSS_MULTICAST_OPTION             0x00000800
/**< Allows reception of multicast addresses programmed into hash
 *   This option defaults to disabled (clear) */

#define XEMACPSS_RX_CHKSUM_ENABLE_OPTION      0x00001000
/**< Enable the RX checksum offload
 *   This option defaults to enabled (set) */

#define XEMACPSS_TX_CHKSUM_ENABLE_OPTION      0x00002000
/**< Enable the TX checksum offload
 *   This option defaults to enabled (set) */


#define XEMACPSS_DEFAULT_OPTIONS                     \
    (XEMACPSS_FLOW_CONTROL_OPTION |                  \
     XEMACPSS_FCS_INSERT_OPTION |                    \
     XEMACPSS_FCS_STRIP_OPTION |                     \
     XEMACPSS_BROADCAST_OPTION |                     \
     XEMACPSS_LENTYPE_ERR_OPTION |                   \
     XEMACPSS_TRANSMITTER_ENABLE_OPTION |            \
     XEMACPSS_RECEIVER_ENABLE_OPTION |               \
     XEMACPSS_RX_CHKSUM_ENABLE_OPTION |              \
     XEMACPSS_TX_CHKSUM_ENABLE_OPTION)

/**< Default options set when device is initialized or reset */
/*@}*/

/** @name Callback identifiers
 *
 * These constants are used as parameters to XEmacPss_SetHandler()
 * @{
 */
#define XEMACPSS_HANDLER_DMASEND 1
#define XEMACPSS_HANDLER_DMARECV 2
#define XEMACPSS_HANDLER_ERROR   3
/*@}*/

/* Constants to determine the configuration of the hardware device. They are
 * used to allow the driver to verify it can operate with the hardware.
 */
#define XEMACPSS_MDIO_DIV_DFT    MDC_DIV_32 /**< Default MDIO clock divisor */

/* The next few constants help upper layers determine the size of memory
 * pools used for Ethernet buffers and descriptor lists.
 */
#define XEMACPSS_MAC_ADDR_SIZE   6	/* size of Ethernet header */

#define XEMACPSS_MTU             1500	/* max MTU size of Ethernet frame */
#define XEMACPSS_HDR_SIZE        14	/* size of Ethernet header */
#define XEMACPSS_HDR_VLAN_SIZE   18	/* size of Ethernet header with VLAN */
#define XEMACPSS_TRL_SIZE        4	/* size of Ethernet trailer (FCS) */
#define XEMACPSS_MAX_FRAME_SIZE       (XEMACPSS_MTU + XEMACPSS_HDR_SIZE + \
        XEMACPSS_TRL_SIZE)
#define XEMACPSS_MAX_VLAN_FRAME_SIZE  (XEMACPSS_MTU + XEMACPSS_HDR_SIZE + \
        XEMACPSS_HDR_VLAN_SIZE + XEMACPSS_TRL_SIZE)

/* Use MII register 1 (MII status register) to detect PHY */
#define PHY_DETECT_REG  1

/* Mask used to verify certain PHY features (or register contents)
 * in the register above:
 *  0x1000: 10Mbps full duplex support
 *  0x0800: 10Mbps half duplex support
 *  0x0008: Auto-negotiation support
 */
#define PHY_DETECT_MASK 0x1808

/**************************** Type Definitions ******************************/
/** @name Typedefs for callback functions
 *
 * These callbacks are invoked in interrupt context.
 * @{
 */
/**
 * Callback invoked when frame(s) have been sent or received in interrupt
 * driven DMA mode. To set the send callback, invoke XEmacPss_SetHandler().
 *
 * @param CallBackRef is user data assigned when the callback was set.
 *
 * @note
 * See zynq_gem_hw.h for bitmasks definitions and the device hardware spec for
 * further information on their meaning.
 *
 */
typedef void (*XEmacPss_Handler) (void *CallBackRef);

/**
 * Callback when an asynchronous error occurs. To set this callback, invoke
 * XEmacPss_SetHandler() with XEMACPSS_HANDLER_ERROR in the HandlerType
 * paramter.
 *
 * @param CallBackRef is user data assigned when the callback was set.
 * @param Direction defines either receive or transmit error(s) has occurred.
 * @param ErrorWord definition varies with Direction
 *
 */
typedef void (*XEmacPss_ErrHandler) (void *CallBackRef, u8 Direction,
				     u32 ErrorWord);

/*@}*/

/**
 * This typedef contains configuration information for a device.
 */
typedef struct {
	u16 DeviceId;	/**< Unique ID  of device */
	u32 BaseAddress;/**< Physical base address of IPIF registers */
} XEmacPss_Config;


/**
 * The XEmacPss driver instance data. The user is required to allocate a
 * structure of this type for every XEmacPss device in the system. A pointer
 * to a structure of this type is then passed to the driver API functions.
 */
typedef struct XEmacPss {
	XEmacPss_Config Config;	/* Hardware configuration */
	u32 IsStarted;		/* Device is currently started */
	u32 IsReady;		/* Device is initialized and ready */
	u32 Options;		/* Current options word */
	int phyaddr;		/* Phy address */

	XEmacPss_BdRing TxBdRing;	/* Transmit BD ring */
	XEmacPss_BdRing RxBdRing;	/* Receive BD ring */

	XEmacPss_Handler SendHandler;
	XEmacPss_Handler RecvHandler;
	void *SendRef;
	void *RecvRef;

	XEmacPss_ErrHandler ErrorHandler;
	void *ErrorRef;

} XEmacPss;


/***************** Macros (Inline Functions) Definitions ********************/

/****************************************************************************/
/**
* Retrieve the Tx ring object. This object can be used in the various Ring
* API functions.
*
* @param  InstancePtr is the DMA channel to operate on.
*
* @return TxBdRing attribute
*
* @note
* C-style signature:
*    XEmacPss_BdRing XEmacPss_GetTxRing(XEmacPss *InstancePtr)
*
*****************************************************************************/
#define XEmacPss_GetTxRing(InstancePtr) ((InstancePtr)->TxBdRing)

/****************************************************************************/
/**
* Retrieve the Rx ring object. This object can be used in the various Ring
* API functions.
*
* @param  InstancePtr is the DMA channel to operate on.
*
* @return RxBdRing attribute
*
* @note
* C-style signature:
*    XEmacPss_BdRing XEmacPss_GetRxRing(XEmacPss *InstancePtr)
*
*****************************************************************************/
#define XEmacPss_GetRxRing(InstancePtr) ((InstancePtr)->RxBdRing)

/****************************************************************************/
/**
*
* Enable interrupts specified in <i>Mask</i>. The corresponding interrupt for
* each bit set to 1 in <i>Mask</i>, will be enabled.
*
* @param InstancePtr is a pointer to the instance to be worked on.
* @param Mask contains a bit mask of interrupts to enable. The mask can
*        be formed using a set of bitwise or'd values.
*
* @note
* The state of the transmitter and receiver are not modified by this function.
* C-style signature
*     void XEmacPss_IntEnable(XEmacPss *InstancePtr, u32 Mask)
*
*****************************************************************************/
#define XEmacPss_IntEnable(InstancePtr, Mask)                            \
	XEmacPss_WriteReg((InstancePtr)->Config.BaseAddress,             \
		XEMACPSS_IER_OFFSET,                                     \
		XEmacPss_ReadReg((InstancePtr)->Config.BaseAddress,      \
			XEMACPSS_IER_OFFSET) | ((Mask) & XEMACPSS_IXR_ALL_MASK));

/****************************************************************************/
/**
*
* Disable interrupts specified in <i>Mask</i>. The corresponding interrupt for
* each bit set to 1 in <i>Mask</i>, will be enabled.
*
* @param InstancePtr is a pointer to the instance to be worked on.
* @param Mask contains a bit mask of interrupts to disable. The mask can
*        be formed using a set of bitwise or'd values.
*
* @note
* The state of the transmitter and receiver are not modified by this function.
* C-style signature
*     void XEmacPss_IntDisable(XEmacPss *InstancePtr, u32 Mask)
*
*****************************************************************************/
#define XEmacPss_IntDisable(InstancePtr, Mask)                           \
	XEmacPss_WriteReg((InstancePtr)->Config.BaseAddress,             \
		XEMACPSS_IER_OFFSET,                                     \
		XEmacPss_ReadReg((InstancePtr)->Config.BaseAddress,      \
			XEMACPSS_IER_OFFSET) & ~((Mask) & XEMACPSS_IXR_ALL_MASK));

/****************************************************************************/
/**
*
* This macro triggers trasmit circuit to send data currently in TX buffer(s).
*
* @param InstancePtr is a pointer to the XEmacPss instance to be worked on.
*
* @return
*
* @note
*
* Signature: void XEmacPss_Transmit(XEmacPss *InstancePtr)
*
*****************************************************************************/
#define XEmacPss_Transmit(InstancePtr)                              \
        XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,          \
        XEMACPSS_NWCTRL_OFFSET,                                     \
        (XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,          \
        XEMACPSS_NWCTRL_OFFSET) | XEMACPSS_NWCTRL_STARTTX_MASK))

/****************************************************************************/
/**
*
* This macro determines if the device is configured with checksum offloading
* on the receive channel
*
* @param InstancePtr is a pointer to the XEmacPss instance to be worked on.
*
* @return
*
* Boolean TRUE if the device is configured with checksum offloading, or
* FALSE otherwise.
*
* @note
*
* Signature: u32 XEmacPss_IsRxCsum(XEmacPss *InstancePtr)
*
*****************************************************************************/
#define XEmacPss_IsRxCsum(InstancePtr)                                     \
        ((XEmacPss_ReadReg((InstancePtr)->Config.BaseAddress,             \
          XEMACPSS_NWCFG_OFFSET) & XEMACPSS_NWCFG_RXCHKSUMEN_MASK)         \
          ? TRUE : FALSE)

/****************************************************************************/
/**
*
* This macro determines if the device is configured with checksum offloading
* on the transmit channel
*
* @param InstancePtr is a pointer to the XEmacPss instance to be worked on.
*
* @return
*
* Boolean TRUE if the device is configured with checksum offloading, or
* FALSE otherwise.
*
* @note
*
* Signature: u32 XEmacPss_IsTxCsum(XEmacPss *InstancePtr)
*
*****************************************************************************/
#define XEmacPss_IsTxCsum(InstancePtr)                                     \
        ((XEmacPss_ReadReg((InstancePtr)->Config.BaseAddress,              \
          XEMACPSS_DMACR_OFFSET) & XEMACPSS_DMACR_TCPCKSUM_MASK)           \
          ? TRUE : FALSE)

/************************** Function Prototypes *****************************/

/*
 * Initialization functions in zynq_gem.c
 */
int XEmacPss_CfgInitialize(XEmacPss *InstancePtr, XEmacPss_Config *CfgPtr,
			   u32 EffectiveAddress);
void XEmacPss_Start(XEmacPss *InstancePtr);
void XEmacPss_Stop(XEmacPss *InstancePtr);
void XEmacPss_Reset(XEmacPss *InstancePtr);

/*
 * Lookup configuration in zynq_gem_sinit.c
 */
XEmacPss_Config *XEmacPss_LookupConfig(u16 DeviceId);

/*
 * Interrupt-related functions in xemacps_intr.c
 * DMA only and FIFO is not supported. This DMA does not support coalescing.
 */
int XEmacPss_SetHandler(XEmacPss *InstancePtr, u32 HandlerType,
			void *FuncPtr, void *CallBackRef);
void XEmacPss_IntrHandler(void *InstancePtr);

/*
 * MAC configuration/control functions in zynq_gem_control.c
 */
int XEmacPss_SetOptions(XEmacPss *InstancePtr, u32 Options);
int XEmacPss_ClearOptions(XEmacPss *InstancePtr, u32 Options);
u32 XEmacPss_GetOptions(XEmacPss *InstancePtr);

int XEmacPss_SetMacAddress(XEmacPss *InstancePtr, void *AddressPtr, u8 Index);
void XEmacPss_GetMacAddress(XEmacPss *InstancePtr, void *AddressPtr, u8 Index);

int XEmacPss_SetHash(XEmacPss *InstancePtr, void *AddressPtr);
void XEmacPss_ClearHash(XEmacPss *InstancePtr);
void XEmacPss_GetHash(XEmacPss *InstancePtr, void *AddressPtr);

void XEmacPss_PhySetMdioDivisor(XEmacPss *InstancePtr,
				XEmacPss_MdcDiv Divisor);
void XEmacPss_SetOperatingSpeed(XEmacPss *InstancePtr, u16 Speed);
u16 XEmacPss_GetOperatingSpeed(XEmacPss *InstancePtr);
int XEmacPss_PhyRead(XEmacPss *InstancePtr, u32 PhyAddress,
		     u32 RegisterNum, u16 *PhyDataPtr);
int XEmacPss_PhyWrite(XEmacPss *InstancePtr, u32 PhyAddress,
		      u32 RegisterNum, u16 PhyData);
int XEmacPss_SetTypeIdCheck(XEmacPss *InstancePtr, u32 Id_Check, u8 Index);

int XEmacPss_SendPausePacket(XEmacPss *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
