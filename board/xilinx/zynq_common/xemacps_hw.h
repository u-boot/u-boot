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
* @file xemacps_hw.h
*
* This header file contains identifiers and low-level driver functions (or
* macros) that can be used to access the PSS Ethernet MAC (XEmacPss) device.
* High-level driver functions are defined in xemacps.h.
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a wsy  06/01/09 First release.
* </pre>
*
******************************************************************************/

#ifndef XEMACPSS_HW_H		/* prevent circular inclusions */
#define XEMACPSS_HW_H		/* by using protection macros */

/***************************** Include Files *********************************/

//#include "xbasic_types.h"
//#include "xio.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/

#define XEMACPSS_MAX_MAC_ADDR     4   /**< Maxmum number of mac address
                                           supported */
#define XEMACPSS_MAX_TYPE_ID      4   /**< Maxmum number of type id supported */
#define XEMACPSS_BD_ALIGNMENT     4   /**< Minimum buffer descriptor alignment
                                           on the local bus */
#define XEMACPSS_RX_BUF_ALIGNMENT 4   /**< Minimum buffer alignment when using
                                           options that impose alignment
                                           restrictions on the buffer data on
                                           the local bus */

/** @name Direction identifiers
 *
 *  These are used by several functions and callbacks that need
 *  to specify whether an operation specifies a send or receive channel.
 * @{
 */
#define XEMACPSS_SEND        1	      /**< send direction */
#define XEMACPSS_RECV        2	      /**< receive direction */
/*@}*/

/**  @name MDC clock division
 *  currently supporting 8, 16, 32, 48, 64, 96, 128, 224.
 * @{
 */
typedef enum { XMDC_DIV_8 = 0, XMDC_DIV_16, XMDC_DIV_32, XMDC_DIV_48,
	XMDC_DIV_64, XMDC_DIV_96, XMDC_DIV_128, XMDC_DIV_224
} XEmacPss_MdcDiv;

/*@}*/

#define XEMACPSS_RX_BUF_SIZE 1536 /**< Specify the receive buffer size in
                                       bytes, 64, 128, ... 10240 */
#define XEMACPSS_RX_BUF_UNIT   64 /**< Number of receive buffer bytes as a
                                       unit, this is HW setup */

#define XEMACPSS_MAX_RXBD     128 /**< Size of RX buffer descriptor queues */
#define XEMACPSS_MAX_TXBD     128 /**< Size of TX buffer descriptor queues */

#define XEMACPSS_MAX_HASH_BITS 64 /**< Maximum value for hash bits. 2**6 */

/* Register offset definitions. Unless otherwise noted, register access is
 * 32 bit. Names are self explained here.
 */

#define XEMACPSS_NWCTRL_OFFSET        0x00000000 /**< Network Control reg */
#define XEMACPSS_NWCFG_OFFSET         0x00000004 /**< Network Config reg */
#define XEMACPSS_NWSR_OFFSET          0x00000008 /**< Network Status reg */

#define XEMACPSS_DMACR_OFFSET         0x00000010 /**< DMA Control reg */
#define XEMACPSS_TXSR_OFFSET          0x00000014 /**< TX Status reg */
#define XEMACPSS_RXQBASE_OFFSET       0x00000018 /**< RX Q Base address reg */
#define XEMACPSS_TXQBASE_OFFSET       0x0000001C /**< TX Q Base address reg */
#define XEMACPSS_RXSR_OFFSET          0x00000020 /**< RX Status reg */

#define XEMACPSS_ISR_OFFSET           0x00000024 /**< Interrupt Status reg */
#define XEMACPSS_IER_OFFSET           0x00000028 /**< Interrupt Enable reg */
#define XEMACPSS_IDR_OFFSET           0x0000002C /**< Interrupt Disable reg */
#define XEMACPSS_IMR_OFFSET           0x00000030 /**< Interrupt Mask reg */

#define XEMACPSS_PHYMNTNC_OFFSET      0x00000034 /**< Phy Maintaince reg */
#define XEMACPSS_RXPAUSE_OFFSET       0x00000038 /**< RX Pause Time reg */
#define XEMACPSS_TXPAUSE_OFFSET       0x0000003C /**< TX Pause Time reg */

#define XEMACPSS_HASHL_OFFSET         0x00000080 /**< Hash Low address reg */
#define XEMACPSS_HASHH_OFFSET         0x00000084 /**< Hash High address reg */

#define XEMACPSS_LADDR1L_OFFSET       0x00000088 /**< Specific1 addr low reg */
#define XEMACPSS_LADDR1H_OFFSET       0x0000008C /**< Specific1 addr high reg */
#define XEMACPSS_LADDR2L_OFFSET       0x00000090 /**< Specific2 addr low reg */
#define XEMACPSS_LADDR2H_OFFSET       0x00000094 /**< Specific2 addr high reg */
#define XEMACPSS_LADDR3L_OFFSET       0x00000098 /**< Specific3 addr low reg */
#define XEMACPSS_LADDR3H_OFFSET       0x0000009C /**< Specific3 addr high reg */
#define XEMACPSS_LADDR4L_OFFSET       0x000000A0 /**< Specific4 addr low reg */
#define XEMACPSS_LADDR4H_OFFSET       0x000000A4 /**< Specific4 addr high reg */

#define XEMACPSS_MATCH1_OFFSET        0x000000A8 /**< Type ID1 Match reg */
#define XEMACPSS_MATCH2_OFFSET        0x000000AC /**< Type ID2 Match reg */
#define XEMACPSS_MATCH3_OFFSET        0x000000B0 /**< Type ID3 Match reg */
#define XEMACPSS_MATCH4_OFFSET        0x000000B4 /**< Type ID4 Match reg */

#define XEMACPSS_STRETCH_OFFSET       0x000000BC /**< IPG Stretch reg */

#define XEMACPSS_OCTTXL_OFFSET        0x00000100 /**< Octects transmitted Low
                                                      reg */
#define XEMACPSS_OCTTXH_OFFSET        0x00000104 /**< Octects transmitted High
                                                      reg */

#define XEMACPSS_TXCNT_OFFSET         0x00000108 /**< Error-free Frmaes
                                                      transmitted counter */
#define XEMACPSS_TXBCCNT_OFFSET       0x0000010C /**< Error-free Broadcast
                                                      Frames counter*/
#define XEMACPSS_TXMCCNT_OFFSET       0x00000110 /**< Error-free Multicast
                                                      Frame counter */
#define XEMACPSS_TXPAUSECNT_OFFSET    0x00000114 /**< Pause Frames Transmitted
                                                      Counter */
#define XEMACPSS_TX64CNT_OFFSET       0x00000118 /**< Error-free 64 byte Frames
                                                      Transmitted counter */
#define XEMACPSS_TX65CNT_OFFSET       0x0000011C /**< Error-free 65-127 byte
                                                      Frames Transmitted
                                                      counter */
#define XEMACPSS_TX128CNT_OFFSET      0x00000120 /**< Error-free 128-255 byte
                                                      Frames Transmitted
                                                      counter*/
#define XEMACPSS_TX256CNT_OFFSET      0x00000124 /**< Error-free 256-511 byte
                                                      Frames transmitted
                                                      counter */
#define XEMACPSS_TX512CNT_OFFSET      0x00000128 /**< Error-free 512-1023 byte
                                                      Frames transmitted
                                                      counter */
#define XEMACPSS_TX1024CNT_OFFSET     0x0000012C /**< Error-free 1024-1518 byte
                                                      Frames transmitted
                                                      counter */
#define XEMACPSS_TX1519CNT_OFFSET     0x00000130 /**< Error-free larger than
                                                      1519 byte Frames
                                                      transmitted counter */
#define XEMACPSS_TXURUNCNT_OFFSET     0x00000134 /**< TX under run error
                                                      counter */

#define XEMACPSS_SNGLCOLLCNT_OFFSET   0x00000138 /**< Single Collision Frame
                                                      Counter */
#define XEMACPSS_MULTICOLLCNT_OFFSET  0x0000013C /**< Multiple Collision Frame
                                                      Counter */
#define XEMACPSS_EXCESSCOLLCNT_OFFSET 0x00000140 /**< Excessive Collision Frame
                                                      Counter */
#define XEMACPSS_LATECOLLCNT_OFFSET   0x00000144 /**< Late Collision Frame
                                                      Counter */
#define XEMACPSS_TXDEFERCNT_OFFSET    0x00000148 /**< Deferred Transmission
                                                      Frame Counter */
#define XEMACPSS_TXCSENSECNT_OFFSET   0x0000014C /**< Transmit Carrier Sense
                                                      Error Counter */

#define XEMACPSS_OCTRXL_OFFSET        0x00000150 /**< Octects Received register
                                                      Low */
#define XEMACPSS_OCTRXH_OFFSET        0x00000154 /**< Octects Received register
                                                      High */

#define XEMACPSS_RXCNT_OFFSET         0x00000158 /**< Error-free Frames
                                                      Received Counter */
#define XEMACPSS_RXBROADCNT_OFFSET    0x0000015C /**< Error-free Broadcast
                                                      Frames Received Counter */
#define XEMACPSS_RXMULTICNT_OFFSET    0x00000160 /**< Error-free Multicast
                                                      Frames Received Counter */
#define XEMACPSS_RXPAUSECNT_OFFSET    0x00000164 /**< Pause Frames
                                                      Received Counter */
#define XEMACPSS_RX64CNT_OFFSET       0x00000168 /**< Error-free 64 byte Frames
                                                      Received Counter */
#define XEMACPSS_RX65CNT_OFFSET       0x0000016C /**< Error-free 65-127 byte
                                                      Frames Received Counter */
#define XEMACPSS_RX128CNT_OFFSET      0x00000170 /**< Error-free 128-255 byte
                                                      Frames Received Counter */
#define XEMACPSS_RX256CNT_OFFSET      0x00000174 /**< Error-free 256-512 byte
                                                      Frames Received Counter */
#define XEMACPSS_RX512CNT_OFFSET      0x00000178 /**< Error-free 512-1023 byte
                                                      Frames Received Counter */
#define XEMACPSS_RX1024CNT_OFFSET     0x0000017C /**< Error-free 1024-1518 byte
                                                      Frames Received Counter */
#define XEMACPSS_RX1519CNT_OFFSET     0x00000180 /**< Error-free 1519-max byte
                                                      Frames Received Counter */
#define XEMACPSS_RXUNDRCNT_OFFSET     0x00000184 /**< Undersize Frames Received
                                                      Counter */
#define XEMACPSS_RXOVRCNT_OFFSET      0x00000188 /**< Oversize Frames Received
                                                      Counter */
#define XEMACPSS_RXJABCNT_OFFSET      0x0000018C /**< Jabbers Received
                                                      Counter */
#define XEMACPSS_RXFCSCNT_OFFSET      0x00000190 /**< Frame Check Sequence
                                                      Error Counter */
#define XEMACPSS_RXLENGTHCNT_OFFSET   0x00000194 /**< Length Field Error
                                                      Counter */
#define XEMACPSS_RXSYMBCNT_OFFSET     0x00000198 /**< Symbol Error Counter */
#define XEMACPSS_RXALIGNCNT_OFFSET    0x0000019C /**< Alignment Error Counter */
#define XEMACPSS_RXRESERRCNT_OFFSET   0x000001A0 /**< Receive Resource Error
                                                      Counter */
#define XEMACPSS_RXORCNT_OFFSET       0x000001A4 /**< Receive Overrun Counter */
#define XEMACPSS_RXIPCCNT_OFFSET      0x000001A8 /**< IP header Checksum Error 
                                                      Counter */
#define XEMACPSS_RXTCPCCNT_OFFSET     0x000001AC /**< TCP Checksum Error
                                                      Counter */
#define XEMACPSS_RXUDPCCNT_OFFSET     0x000001B0 /**< UDP Checksum Error
                                                      Counter */
#define XEMACPSS_LAST_OFFSET          0x000001B4 /**< Last statistic counter
						      offset, for clearing */

#define XEMACPSS_1588_SEC_OFFSET      0x000001D0 /**< 1588 second counter */
#define XEMACPSS_1588_NANOSEC_OFFSET  0x000001D4 /**< 1588 nanosecond counter */
#define XEMACPSS_1588_ADJ_OFFSET      0x000001D8 /**< 1588 nanosecond
						      adjustment counter */
#define XEMACPSS_1588_INC_OFFSET      0x000001DC /**< 1588 nanosecond
						      increment counter */
#define XEMACPSS_PTP_TXSEC_OFFSET     0x000001E0 /**< 1588 PTP transmit second
						      counter */
#define XEMACPSS_PTP_TXNANOSEC_OFFSET 0x000001E4 /**< 1588 PTP transmit 
						      nanosecond counter */
#define XEMACPSS_PTP_RXSEC_OFFSET     0x000001E8 /**< 1588 PTP receive second
						      counter */
#define XEMACPSS_PTP_RXNANOSEC_OFFSET 0x000001EC /**< 1588 PTP receive 
						      nanosecond counter */
#define XEMACPSS_PTPP_TXSEC_OFFSET    0x000001F0 /**< 1588 PTP peer transmit
						      second counter */
#define XEMACPSS_PTPP_TXNANOSEC_OFFSET 0x000001F4 /**< 1588 PTP peer transmit 
						      nanosecond counter */
#define XEMACPSS_PTPP_RXSEC_OFFSET    0x000001F8 /**< 1588 PTP peer receive
						      second counter */
#define XEMACPSS_PTPP_RXNANOSEC_OFFSET 0x000001FC /**< 1588 PTP peer receive 
						      nanosecond counter */

/* Define some bit positions for registers. */

/** @name network control register bit definitions
 * @{
 */
#define XEMACPSS_NWCTRL_ZEROPAUSETX_MASK 0x00000800 /**< Transmit zero quantum
                                                         pause frame */
#define XEMACPSS_NWCTRL_PAUSETX_MASK     0x00000800 /**< Transmit pause frame */
#define XEMACPSS_NWCTRL_HALTTX_MASK      0x00000400 /**< Halt transmission
                                                         after current frame */
#define XEMACPSS_NWCTRL_STARTTX_MASK     0x00000200 /**< Start tx (tx_go) */

#define XEMACPSS_NWCTRL_STATWEN_MASK     0x00000080 /**< Enable writing to
                                                         stat counters */
#define XEMACPSS_NWCTRL_STATINC_MASK     0x00000040 /**< Increment statistic
                                                         registers */
#define XEMACPSS_NWCTRL_STATCLR_MASK     0x00000020 /**< Clear statistic
                                                         registers */
#define XEMACPSS_NWCTRL_MDEN_MASK        0x00000010 /**< Enable MDIO port */
#define XEMACPSS_NWCTRL_TXEN_MASK        0x00000008 /**< Enable transmit */
#define XEMACPSS_NWCTRL_RXEN_MASK        0x00000004 /**< Enable receive */
#define XEMACPSS_NWCTRL_LOOPEN_MASK      0x00000002 /**< local loopback */
/*@}*/

/** @name network configuration register bit definitions
 * @{
 */
#define XEMACPSS_NWCFG_BADPREAMBEN_MASK 0x20000000 /**< disable rejection of
                                                        non-standard preamble */
#define XEMACPSS_NWCFG_IPDSTRETCH_MASK  0x10000000 /**< enable transmit IPG */
#define XEMACPSS_NWCFG_FCSIGNORE_MASK   0x04000000 /**< disable rejection of
                                                        FCS error */
#define XEMACPSS_NWCFG_HDRXEN_MASK      0x02000000 /**< RX half duplex */
#define XEMACPSS_NWCFG_RXCHKSUMEN_MASK  0x01000000 /**< enable RX checksum
                                                        offload */
#define XEMACPSS_NWCFG_PAUSECOPYDI_MASK 0x00800000 /**< Do not copy pause
                                                        Frames to memory */
#define XEMACPSS_NWCFG_MDC_SHIFT_MASK   18	   /**< shift bits for MDC */
#define XEMACPSS_NWCFG_MDCCLKDIV_MASK   0x001C0000 /**< MDC Mask PCLK divisor */
#define XEMACPSS_NWCFG_FCSREM_MASK      0x00020000 /**< Discard FCS from
                                                        received frames */
#define XEMACPSS_NWCFG_LENGTHERRDSCRD_MASK 0x00010000
/**< RX length error discard */
#define XEMACPSS_NWCFG_RXOFFS_MASK      0x0000C000 /**< RX buffer offset */
#define XEMACPSS_NWCFG_PAUSEEN_MASK     0x00002000 /**< Enable pause RX */
#define XEMACPSS_NWCFG_RETRYTESTEN_MASK 0x00001000 /**< Retry test */
#define XEMACPSS_NWCFG_EXTADDRMATCHEN_MASK 0x00000200
/**< External address match enable */
#define XEMACPSS_NWCFG_1000_MASK        0x00000400 /**< 1000 Mbps */
#define XEMACPSS_NWCFG_1536RXEN_MASK    0x00000100 /**< Enable 1536 byte
                                                        frames reception */
#define XEMACPSS_NWCFG_UCASTHASHEN_MASK 0x00000080 /**< Receive unicast hash
                                                        frames */
#define XEMACPSS_NWCFG_MCASTHASHEN_MASK 0x00000040 /**< Receive multicast hash
                                                        frames */
#define XEMACPSS_NWCFG_BCASTDI_MASK     0x00000020 /**< Do not receive
                                                        broadcast frames */
#define XEMACPSS_NWCFG_COPYALLEN_MASK   0x00000010 /**< Copy all frames */
#define XEMACPSS_NWCFG_JUMBO_MASK       0x00000008 /**< Jumbo frames */
#define XEMACPSS_NWCFG_NVLANDISC_MASK   0x00000004 /**< Receive only VLAN
                                                        frames */
#define XEMACPSS_NWCFG_FDEN_MASK        0x00000002 /**< full duplex */
#define XEMACPSS_NWCFG_100_MASK         0x00000001 /**< 100 Mbps */
/*@}*/

/** @name network status register bit definitaions
 * @{
 */
#define XEMACPSS_NWSR_MDIOIDLE_MASK     0x00000004 /**< PHY management idle */
#define XEMACPSS_NWSR_MDIO_MASK         0x00000002 /**< Status of mdio_in */
/*@}*/


/** @name MAC address register word 1 mask
 * @{
 */
#define XEMACPSS_LADDR_MACH_MASK        0x0000FFFF /**< Address bits[47:32]
                                                      bit[31:0] are in BOTTOM */
/*@}*/


/** @name DMA control register bit definitions
 * @{
 */
#define XEMACPSS_DMACR_RXBUF_MASK      0x00FF0000 /**< Mask bit for RX buffer
                                                       size */
#define XEMACPSS_DMACR_RXBUF_SHIFT     16	  /**< Shift bit for RX buffer
                                                       size */
#define XEMACPSS_DMACR_TCPCKSUM_MASK   0x00000800 /**< enable/disable TX
                                                       checksum offload */
#define XEMACPSS_DMACR_TXSIZE_MASK     0x00000400 /**< TX buffer memory size */
#define XEMACPSS_DMACR_RXSIZE_MASK     0x00000300 /**< RX buffer memory size */
#define XEMACPSS_DMACR_ENDIAN_MASK     0x00000080 /**< endian configuration */
#define XEMACPSS_DMACR_BLENGTH_MASK    0x0000001F /**< buffer burst length */
/*@}*/

/** @name transmit status register bit definitions
 * @{
 */
#define XEMACPSS_TXSR_HRESPNOK_MASK    0x00000100 /**< Transmit hresp not OK */
#define XEMACPSS_TXSR_URUN_MASK        0x00000040 /**< Transmit underrun */
#define XEMACPSS_TXSR_TXCOMPL_MASK     0x00000020 /**< Transmit completed OK */
#define XEMACPSS_TXSR_BUFEXH_MASK      0x00000010 /**< Transmit buffs exhausted
                                                       mid frame */
#define XEMACPSS_TXSR_TXGO_MASK        0x00000008 /**< Status of go flag */
#define XEMACPSS_TXSR_RXOVR_MASK       0x00000004 /**< Retry limit exceeded */
#define XEMACPSS_TXSR_FRAMERX_MASK     0x00000002 /**< Collision tx frame */
#define XEMACPSS_TXSR_USEDREAD_MASK    0x00000001 /**< TX buffer used bit set */

#define XEMACPSS_TXSR_ERROR_MASK      (XEMACPSS_TXSR_HRESPNOK_MASK | \
                                       XEMACPSS_TXSR_URUN_MASK | \
                                       XEMACPSS_TXSR_BUFEXH_MASK | \
                                       XEMACPSS_TXSR_RXOVR_MASK | \
                                       XEMACPSS_TXSR_FRAMERX_MASK | \
                                       XEMACPSS_TXSR_USEDREAD_MASK)
/*@}*/

/** @name receive status register bit definitions
 * @{
 */
#define XEMACPSS_RXSR_HRESPNOK_MASK    0x00000008 /**< Receive hresp not OK */
#define XEMACPSS_RXSR_RXOVR_MASK       0x00000004 /**< Receive overrun */
#define XEMACPSS_RXSR_FRAMERX_MASK     0x00000002 /**< Frame received OK */
#define XEMACPSS_RXSR_BUFFNA_MASK      0x00000001 /**< RX buffer used bit set */

#define XEMACPSS_RXSR_ERROR_MASK      (XEMACPSS_RXSR_HRESPNOK_MASK | \
                                       XEMACPSS_RXSR_RXOVR_MASK | \
                                       XEMACPSS_RXSR_BUFFNA_MASK)
/*@}*/

/** @name interrupts bit definitions
 * Bits definitions are same in XEMACPSS_ISR_OFFSET,
 * XEMACPSS_IER_OFFSET, XEMACPSS_IDR_OFFSET, and XEMACPSS_IMR_OFFSET
 * @{
 */
#define XEMACPSS_IXR_PTPPSTX_MASK    0x02000000 /**< PTP Psync transmitted */
#define XEMACPSS_IXR_PTPPDRTX_MASK   0x01000000 /**< PTP Pdelay_req
						     transmitted */
#define XEMACPSS_IXR_PTPSTX_MASK     0x00800000 /**< PTP Sync transmitted */
#define XEMACPSS_IXR_PTPDRTX_MASK    0x00400000 /**< PTP Delay_req transmitted
						*/
#define XEMACPSS_IXR_PTPPSRX_MASK    0x00200000 /**< PTP Psync received */
#define XEMACPSS_IXR_PTPPDRRX_MASK   0x00100000 /**< PTP Pdelay_req received */
#define XEMACPSS_IXR_PTPSRX_MASK     0x00080000 /**< PTP Sync received */
#define XEMACPSS_IXR_PTPDRRX_MASK    0x00040000 /**< PTP Delay_req received */
#define XEMACPSS_IXR_PAUSETX_MASK    0x00004000	/**< Pause frame transmitted */
#define XEMACPSS_IXR_PAUSEZERO_MASK  0x00002000	/**< Pause time has reached
                                                     zero */
#define XEMACPSS_IXR_PAUSENZERO_MASK 0x00001000	/**< Pause frame received */
#define XEMACPSS_IXR_HRESPNOK_MASK   0x00000800	/**< hresp not ok */
#define XEMACPSS_IXR_RXOVR_MASK      0x00000400	/**< Receive overrun occurred */
#define XEMACPSS_IXR_TXCOMPL_MASK    0x00000080	/**< Frame transmitted ok */
#define XEMACPSS_IXR_TXEXH_MASK      0x00000040	/**< Transmit err occurred or
                                                     no buffers*/
#define XEMACPSS_IXR_RETRY_MASK      0x00000020	/**< Retry limit exceeded */
#define XEMACPSS_IXR_URUN_MASK       0x00000010	/**< Transmit underrun */
#define XEMACPSS_IXR_TXUSED_MASK     0x00000008	/**< Tx buffer used bit read */
#define XEMACPSS_IXR_RXUSED_MASK     0x00000004	/**< Rx buffer used bit read */
#define XEMACPSS_IXR_FRAMERX_MASK    0x00000002	/**< Frame received ok */
#define XEMACPSS_IXR_MGMNT_MASK      0x00000001	/**< PHY management complete */
#define XEMACPSS_IXR_ALL_MASK        0x00007FFF	/**< Everything! */

#define XEMACPSS_IXR_TX_ERR_MASK    (XEMACPSS_IXR_TXEXH_MASK |         \
                                     XEMACPSS_IXR_RETRY_MASK |         \
                                     XEMACPSS_IXR_URUN_MASK  |         \
                                     XEMACPSS_IXR_TXUSED_MASK)


#define XEMACPSS_IXR_RX_ERR_MASK    (XEMACPSS_IXR_HRESPNOK_MASK |      \
                                     XEMACPSS_IXR_RXUSED_MASK |        \
                                     XEMACPSS_IXR_RXOVR_MASK)

/*@}*/

/** @name PHY Maintenance bit definitions
 * @{
 */
#define XEMACPSS_PHYMNTNC_OP_MASK    0x40020000	/**< operation mask bits */
#define XEMACPSS_PHYMNTNC_OP_R_MASK  0x20000000	/**< read operation */
#define XEMACPSS_PHYMNTNC_OP_W_MASK  0x10000000	/**< write operation */
#define XEMACPSS_PHYMNTNC_ADDR_MASK  0x0F800000	/**< Address bits */
#define XEMACPSS_PHYMNTNC_REG_MASK   0x007C0000	/**< register bits */
#define XEMACPSS_PHYMNTNC_DATA_MASK  0x00000FFF	/**< data bits */
#define XEMACPSS_PHYMNTNC_PHYAD_SHIFT_MASK   23	/**< Shift bits for PHYAD */
#define XEMACPSS_PHYMNTNC_PHREG_SHIFT_MASK   18	/**< Shift bits for PHREG */
/*@}*/

/* Transmit buffer descriptor status words offset
 * @{
 */
#define XEMACPSS_BD_ADDR_OFFSET  0x00000000 /**< word 0/addr of BDs */
#define XEMACPSS_BD_STAT_OFFSET  0x00000004 /**< word 1/status of BDs */
/*@}*/

/* Transmit buffer descriptor status words bit positions.
 * Transmit buffer descriptor consists of two 32-bit registers,
 * the first - word0 contains a 32-bit address pointing to the location of
 * the transmit data.
 * The following register - word1, consists of various information to control
 * the XEmacPss transmit process.  After transmit, this is updated with status
 * information, whether the frame was transmitted OK or why it had failed.
 * @{
 */
#define XEMACPSS_TXBUF_USED_MASK  0x80000000 /**< Used bit. */
#define XEMACPSS_TXBUF_WRAP_MASK  0x40000000 /**< Wrap bit, last descriptor */
#define XEMACPSS_TXBUF_RETRY_MASK 0x20000000 /**< Retry limit exceeded */
#define XEMACPSS_TXBUF_URUN_MASK  0x10000000 /**< Transmit underrun occurred */
#define XEMACPSS_TXBUF_EXH_MASK   0x08000000 /**< Buffers exhausted */
#define XEMACPSS_TXBUF_TCP_MASK   0x04000000 /**< Late collision. */
#define XEMACPSS_TXBUF_NOCRC_MASK 0x00010000 /**< No CRC */
#define XEMACPSS_TXBUF_LAST_MASK  0x00008000 /**< Last buffer */
#define XEMACPSS_TXBUF_LEN_MASK   0x00003FFF /**< Mask for length field */
/*@}*/

/* Receive buffer descriptor status words bit positions.
 * Receive buffer descriptor consists of two 32-bit registers,
 * the first - word0 contains a 32-bit word aligned address pointing to the
 * address of the buffer. The lower two bits make up the wrap bit indicating
 * the last descriptor and the ownership bit to indicate it has been used by
 * the XEmacPss.
 * The following register - word1, contains status information regarding why
 * the frame was received (the filter match condition) as well as other
 * useful info.
 * @{
 */
#define XEMACPSS_RXBUF_BCAST_MASK     0x80000000 /**< Broadcast frame */
#define XEMACPSS_RXBUF_MULTIHASH_MASK 0x40000000 /**< Multicast hashed frame */
#define XEMACPSS_RXBUF_UNIHASH_MASK   0x20000000 /**< Unicast hashed frame */
#define XEMACPSS_RXBUF_EXH_MASK       0x08000000 /**< buffer exhausted */
#define XEMACPSS_RXBUF_AMATCH_MASK    0x06000000 /**< Specific address
                                                      matched */
#define XEMACPSS_RXBUF_IDFOUND_MASK   0x01000000 /**< Type ID matched */
#define XEMACPSS_RXBUF_IDMATCH_MASK   0x00C00000 /**< ID matched mask */
#define XEMACPSS_RXBUF_VLAN_MASK      0x00200000 /**< VLAN tagged */
#define XEMACPSS_RXBUF_PRI_MASK       0x00100000 /**< Priority tagged */
#define XEMACPSS_RXBUF_VPRI_MASK      0x000E0000 /**< Vlan priority */
#define XEMACPSS_RXBUF_CFI_MASK       0x00010000 /**< CFI frame */
#define XEMACPSS_RXBUF_EOF_MASK       0x00008000 /**< End of frame. */
#define XEMACPSS_RXBUF_SOF_MASK       0x00004000 /**< Start of frame. */
#define XEMACPSS_RXBUF_LEN_MASK       0x00003FFF /**< Mask for length field */

#define XEMACPSS_RXBUF_WRAP_MASK      0x00000002 /**< Wrap bit, last BD */
#define XEMACPSS_RXBUF_NEW_MASK       0x00000001 /**< Used bit.. */
#define XEMACPSS_RXBUF_ADD_MASK       0xFFFFFFFC /**< Mask for address */
/*@}*/

/*
 * Define appropriate I/O access method to mempry mapped I/O or other
 * intarfce if necessary.
 */
/* Defined in xemacps_control.c*/
void XIo_Out32(u32 OutAddress, u32 Value);
u32 XIo_In32(u32 InAddress);

#define XEmacPss_In32  XIo_In32
#define XEmacPss_Out32 XIo_Out32


/****************************************************************************/
/**
*
* Read the given register.
*
* @param    BaseAddress is the base address of the device
* @param    RegOffset is the register offset to be read
*
* @return   The 32-bit value of the register
*
* @note
* C-style signature:
*    u32 XEmacPss_ReadReg(u32 BaseAddress, u32 RegOffset)
*
*****************************************************************************/
#define XEmacPss_ReadReg(BaseAddress, RegOffset) \
    XEmacPss_In32((BaseAddress) + (RegOffset))


/****************************************************************************/
/**
*
* Write the given register.
*
* @param    BaseAddress is the base address of the device
* @param    RegOffset is the register offset to be written
* @param    Data is the 32-bit value to write to the register
*
* @return   None.
*
* @note
* C-style signature:
*    void XEmacPss_WriteReg(u32 BaseAddress, u32 RegOffset,
*         u32 Data)
*
*****************************************************************************/
#define XEmacPss_WriteReg(BaseAddress, RegOffset, Data) \
    XEmacPss_Out32((BaseAddress) + (RegOffset), (Data))

#ifdef __cplusplus
  }
#endif

#endif /* end of protection macro */
