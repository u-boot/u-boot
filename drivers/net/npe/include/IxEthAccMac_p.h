/*
 * 
 * @par
 * IXP400 SW Release version 2.0
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * @par
 * -- End of Copyright Notice --
*/


#ifndef IxEthAccMac_p_H
#define IxEthAccMac_p_H

#include "IxOsal.h"

#define IX_ETH_ACC_MAX_MULTICAST_ADDRESSES 256
#define IX_ETH_ACC_NUM_PORTS 3
#define IX_ETH_ACC_MAX_FRAME_SIZE_DEFAULT 1536
#define IX_ETH_ACC_MAX_FRAME_SIZE_UPPER_RANGE (65536-64)
#define IX_ETH_ACC_MAX_FRAME_SIZE_LOWER_RANGE 64

/*
 * 
 * MAC register definitions
 *
 */
#define IX_ETH_ACC_MAC_0_BASE IX_OSAL_IXP400_ETHA_PHYS_BASE
#define IX_ETH_ACC_MAC_1_BASE IX_OSAL_IXP400_ETHB_PHYS_BASE
#define IX_ETH_ACC_MAC_2_BASE IX_OSAL_IXP400_ETH_NPEA_PHYS_BASE

#define IX_ETH_ACC_MAC_TX_CNTRL1       0x000
#define IX_ETH_ACC_MAC_TX_CNTRL2       0x004
#define IX_ETH_ACC_MAC_RX_CNTRL1       0x010
#define IX_ETH_ACC_MAC_RX_CNTRL2       0x014
#define IX_ETH_ACC_MAC_RANDOM_SEED     0x020
#define IX_ETH_ACC_MAC_THRESH_P_EMPTY  0x030
#define IX_ETH_ACC_MAC_THRESH_P_FULL   0x038
#define IX_ETH_ACC_MAC_BUF_SIZE_TX     0x040
#define IX_ETH_ACC_MAC_TX_DEFER        0x050
#define IX_ETH_ACC_MAC_RX_DEFER        0x054
#define IX_ETH_ACC_MAC_TX_TWO_DEFER_1  0x060
#define IX_ETH_ACC_MAC_TX_TWO_DEFER_2  0x064
#define IX_ETH_ACC_MAC_SLOT_TIME       0x070
#define IX_ETH_ACC_MAC_MDIO_CMD_1      0x080
#define IX_ETH_ACC_MAC_MDIO_CMD_2      0x084
#define IX_ETH_ACC_MAC_MDIO_CMD_3      0x088
#define IX_ETH_ACC_MAC_MDIO_CMD_4      0x08c
#define IX_ETH_ACC_MAC_MDIO_STS_1      0x090
#define IX_ETH_ACC_MAC_MDIO_STS_2      0x094
#define IX_ETH_ACC_MAC_MDIO_STS_3      0x098
#define IX_ETH_ACC_MAC_MDIO_STS_4      0x09c
#define IX_ETH_ACC_MAC_ADDR_MASK_1     0x0A0
#define IX_ETH_ACC_MAC_ADDR_MASK_2     0x0A4
#define IX_ETH_ACC_MAC_ADDR_MASK_3     0x0A8
#define IX_ETH_ACC_MAC_ADDR_MASK_4     0x0AC
#define IX_ETH_ACC_MAC_ADDR_MASK_5     0x0B0
#define IX_ETH_ACC_MAC_ADDR_MASK_6     0x0B4
#define IX_ETH_ACC_MAC_ADDR_1          0x0C0
#define IX_ETH_ACC_MAC_ADDR_2          0x0C4
#define IX_ETH_ACC_MAC_ADDR_3          0x0C8
#define IX_ETH_ACC_MAC_ADDR_4          0x0CC
#define IX_ETH_ACC_MAC_ADDR_5          0x0D0
#define IX_ETH_ACC_MAC_ADDR_6          0x0D4
#define IX_ETH_ACC_MAC_INT_CLK_THRESH  0x0E0
#define IX_ETH_ACC_MAC_UNI_ADDR_1      0x0F0
#define IX_ETH_ACC_MAC_UNI_ADDR_2      0x0F4
#define IX_ETH_ACC_MAC_UNI_ADDR_3      0x0F8
#define IX_ETH_ACC_MAC_UNI_ADDR_4      0x0FC
#define IX_ETH_ACC_MAC_UNI_ADDR_5      0x100
#define IX_ETH_ACC_MAC_UNI_ADDR_6      0x104
#define IX_ETH_ACC_MAC_CORE_CNTRL      0x1FC


/*
 *
 *Bit definitions
 *
 */

/* TX Control Register 1*/

#define IX_ETH_ACC_TX_CNTRL1_TX_EN         BIT(0)
#define IX_ETH_ACC_TX_CNTRL1_DUPLEX        BIT(1)
#define IX_ETH_ACC_TX_CNTRL1_RETRY         BIT(2)
#define IX_ETH_ACC_TX_CNTRL1_PAD_EN        BIT(3)
#define IX_ETH_ACC_TX_CNTRL1_FCS_EN        BIT(4)
#define IX_ETH_ACC_TX_CNTRL1_2DEFER        BIT(5)
#define IX_ETH_ACC_TX_CNTRL1_RMII          BIT(6)

/* TX Control Register 2 */
#define IX_ETH_ACC_TX_CNTRL2_RETRIES_MASK  0xf

/* RX Control Register 1 */
#define IX_ETH_ACC_RX_CNTRL1_RX_EN         BIT(0)
#define IX_ETH_ACC_RX_CNTRL1_PADSTRIP_EN   BIT(1)
#define IX_ETH_ACC_RX_CNTRL1_CRC_EN        BIT(2)
#define IX_ETH_ACC_RX_CNTRL1_PAUSE_EN      BIT(3)
#define IX_ETH_ACC_RX_CNTRL1_LOOP_EN       BIT(4)
#define IX_ETH_ACC_RX_CNTRL1_ADDR_FLTR_EN  BIT(5)
#define IX_ETH_ACC_RX_CNTRL1_RX_RUNT_EN    BIT(6)
#define IX_ETH_ACC_RX_CNTRL1_BCAST_DIS     BIT(7)

/* RX Control Register 2 */
#define IX_ETH_ACC_RX_CNTRL2_DEFER_EN      BIT(0)



/* Core Control Register */
#define IX_ETH_ACC_CORE_RESET              BIT(0)
#define IX_ETH_ACC_CORE_RX_FIFO_FLUSH      BIT(1)
#define IX_ETH_ACC_CORE_TX_FIFO_FLUSH      BIT(2)
#define IX_ETH_ACC_CORE_SEND_JAM           BIT(3)
#define IX_ETH_ACC_CORE_MDC_EN             BIT(4)

/* 1st bit of 1st MAC octet */
#define IX_ETH_ACC_ETH_MAC_BCAST_MCAST_BIT ( 1) 


/*
 *
 * Default values
 *
 */


#define IX_ETH_ACC_TX_CNTRL1_DEFAULT  (IX_ETH_ACC_TX_CNTRL1_TX_EN | \
 IX_ETH_ACC_TX_CNTRL1_RETRY  | \
 IX_ETH_ACC_TX_CNTRL1_FCS_EN | \
 IX_ETH_ACC_TX_CNTRL1_2DEFER | \
 IX_ETH_ACC_TX_CNTRL1_PAD_EN)

#define IX_ETH_ACC_TX_MAX_RETRIES_DEFAULT      0x0f 

#define IX_ETH_ACC_RX_CNTRL1_DEFAULT    (IX_ETH_ACC_RX_CNTRL1_CRC_EN    \
					 | IX_ETH_ACC_RX_CNTRL1_RX_EN)

#define IX_ETH_ACC_RX_CNTRL2_DEFAULT           0x0

/* Thresholds determined by NPE firmware FS */
#define IX_ETH_ACC_MAC_THRESH_P_EMPTY_DEFAULT  0x12
#define IX_ETH_ACC_MAC_THRESH_P_FULL_DEFAULT   0x30

/* Number of bytes that must be in the tx fifo before
   transmission commences*/
#define IX_ETH_ACC_MAC_BUF_SIZE_TX_DEFAULT     0x8

/* One-part deferral values */
#define IX_ETH_ACC_MAC_TX_DEFER_DEFAULT        0x15
#define IX_ETH_ACC_MAC_RX_DEFER_DEFAULT        0x16

/* Two-part deferral values... */
#define IX_ETH_ACC_MAC_TX_TWO_DEFER_1_DEFAULT  0x08
#define IX_ETH_ACC_MAC_TX_TWO_DEFER_2_DEFAULT  0x07

/* This value applies to MII */
#define IX_ETH_ACC_MAC_SLOT_TIME_DEFAULT       0x80

/* This value applies to RMII */
#define IX_ETH_ACC_MAC_SLOT_TIME_RMII_DEFAULT  0xFF

#define IX_ETH_ACC_MAC_ADDR_MASK_DEFAULT       0xFF

#define IX_ETH_ACC_MAC_INT_CLK_THRESH_DEFAULT  0x1
/*The following is a value chosen at random*/
#define IX_ETH_ACC_RANDOM_SEED_DEFAULT         0x8

/*By default we must configure the MAC to generate the 
  MDC clock*/
#define IX_ETH_ACC_CORE_DEFAULT                (IX_ETH_ACC_CORE_MDC_EN)

#define IXP425_ETH_ACC_MAX_PHY 2
#define IXP425_ETH_ACC_MAX_AN_ENTRIES 20
#define IX_ETH_ACC_MAC_RESET_DELAY    1

#define IX_ETH_ACC_MAC_ALL_BITS_SET   0xFF

#define IX_ETH_ACC_MAC_MSGID_SHL      24

#define IX_ETH_ACC_PORT_DISABLE_DELAY_MSECS 20
#define IX_ETH_ACC_PORT_DISABLE_DELAY_COUNT 200  /* 4 seconds timeout */
#define IX_ETH_ACC_PORT_DISABLE_RETRY_COUNT 3
#define IX_ETH_ACC_MIB_STATS_DELAY_MSECS 2000 /* 2 seconds delay for ethernet stats */ 

/*Register access macros*/
#if  (CPU == SIMSPARCSOLARIS)
extern void registerWriteStub (UINT32 base, UINT32 offset, UINT32 val);
extern UINT32 registerReadStub (UINT32 base, UINT32 offset);

#define REG_WRITE(b,o,v) registerWriteStub(b, o, v)
#define REG_READ(b,o,v)  do { v = registerReadStub(b, o); } while (0)
#else
#define REG_WRITE(b,o,v) IX_OSAL_WRITE_LONG((volatile UINT32 *)(b + o), v)
#define REG_READ(b,o,v)  (v = IX_OSAL_READ_LONG((volatile UINT32 *)(b + o)))

#endif

void ixEthAccMacUnload(void);
IxEthAccStatus ixEthAccMacMemInit(void);

/* MAC core loopback */
IxEthAccStatus ixEthAccPortLoopbackEnable(IxEthAccPortId portId);
IxEthAccStatus ixEthAccPortLoopbackDisable(IxEthAccPortId portId);

/* MAC core traffic control */
IxEthAccStatus ixEthAccPortTxEnablePriv(IxEthAccPortId portId);
IxEthAccStatus ixEthAccPortTxDisablePriv(IxEthAccPortId portId);
IxEthAccStatus ixEthAccPortRxEnablePriv(IxEthAccPortId portId);
IxEthAccStatus ixEthAccPortRxDisablePriv(IxEthAccPortId portId);
IxEthAccStatus ixEthAccPortMacResetPriv(IxEthAccPortId portId);

/* NPE software loopback */
IxEthAccStatus ixEthAccNpeLoopbackDisablePriv(IxEthAccPortId portId);
IxEthAccStatus ixEthAccNpeLoopbackEnablePriv(IxEthAccPortId portId);

#endif /*IxEthAccMac_p_H*/

