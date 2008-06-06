/**
 * @file IxEthAccMii_p.h
 *
 * @author Intel Corporation
 * @date 
 *
 * @brief  MII Header file
 *
 * Design Notes:
 *
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

#ifndef IxEthAccMii_p_H
#define IxEthAccMii_p_H

/* MII definitions - these have been verified against the LXT971 and LXT972 PHYs*/

#define IXP425_ETH_ACC_MII_MAX_REG      32      /* max register per phy */

#define IX_ETH_ACC_MII_REG_SHL    16
#define IX_ETH_ACC_MII_ADDR_SHL   21

/* Definitions for MII access routines*/
 
#define IX_ETH_ACC_MII_GO                  BIT(31)
#define IX_ETH_ACC_MII_WRITE               BIT(26)
#define IX_ETH_ACC_MII_TIMEOUT_10TH_SECS        5    
#define IX_ETH_ACC_MII_10TH_SEC_IN_MILLIS     100
#define IX_ETH_ACC_MII_READ_FAIL           BIT(31)
 
#define IX_ETH_ACC_MII_PHY_DEF_DELAY   300  /* max delay before link up, etc. */
#define IX_ETH_ACC_MII_PHY_NO_DELAY    0x0  /* do not delay */
#define IX_ETH_ACC_MII_PHY_NULL        0xff /* PHY is not present */
#define IX_ETH_ACC_MII_PHY_DEF_ADDR    0x0  /* default PHY's logical address */

#ifndef IX_ETH_ACC_MII_MONITOR_DELAY
#   define IX_ETH_ACC_MII_MONITOR_DELAY   0x5    /* in seconds */
#endif

/* Register definition */  

#define IX_ETH_ACC_MII_CTRL_REG	    0x0	/* Control Register */
#define IX_ETH_ACC_MII_STAT_REG	    0x1	/* Status Register */
#define IX_ETH_ACC_MII_PHY_ID1_REG  0x2	/* PHY identifier 1 Register */
#define IX_ETH_ACC_MII_PHY_ID2_REG  0x3	/* PHY identifier 2 Register */
#define IX_ETH_ACC_MII_AN_ADS_REG   0x4	/* Auto-Negotiation	  */
					/* Advertisement Register */
#define IX_ETH_ACC_MII_AN_PRTN_REG  0x5	/* Auto-Negotiation	    */
					/* partner ability Register */
#define IX_ETH_ACC_MII_AN_EXP_REG   0x6	/* Auto-Negotiation   */
					/* Expansion Register */
#define IX_ETH_ACC_MII_AN_NEXT_REG  0x7	/* Auto-Negotiation	       */
					/* next-page transmit Register */

IxEthAccStatus ixEthAccMdioShow (void);
IxEthAccStatus ixEthAccMiiInit(void);
void ixEthAccMiiUnload(void);

#endif  /*IxEthAccMii_p_H*/
