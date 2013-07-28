/**
 * @file IxEthMii_p.h
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
 * SPDX-License-Identifier:	BSD-3-Clause
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IxEthMii_p_H
#define IxEthMii_p_H


/* MII definitions - these have been verified against the LXT971 and
 LXT972 PHYs*/

#define IX_ETH_MII_MAX_REG_NUM  0x20  /* max number of registers */

#define IX_ETH_MII_CTRL_REG     0x0	/* Control Register */
#define IX_ETH_MII_STAT_REG	0x1	/* Status Register */
#define IX_ETH_MII_PHY_ID1_REG  0x2	/* PHY identifier 1 Register */
#define IX_ETH_MII_PHY_ID2_REG  0x3	/* PHY identifier 2 Register */
#define IX_ETH_MII_AN_ADS_REG   0x4	/* Auto-Negotiation 	  */
					/* Advertisement Register */
#define IX_ETH_MII_AN_PRTN_REG  0x5	/* Auto-Negotiation 	    */
					/* partner ability Register */
#define IX_ETH_MII_AN_EXP_REG   0x6	/* Auto-Negotiation   */
					/* Expansion Register */
#define IX_ETH_MII_AN_NEXT_REG  0x7	/* Auto-Negotiation 	       */
					/* next-page transmit Register */

#define IX_ETH_MII_STAT2_REG    0x11    /* Status Register 2*/


/* MII control register bit  */

#define IX_ETH_MII_CR_COLL_TEST  0x0080  /* collision test */
#define IX_ETH_MII_CR_FDX	 0x0100  /* FDX =1, half duplex =0 */
#define IX_ETH_MII_CR_RESTART    0x0200  /* restart auto negotiation */
#define IX_ETH_MII_CR_ISOLATE    0x0400  /* isolate PHY from MII */
#define IX_ETH_MII_CR_POWER_DOWN 0x0800  /* power down */
#define IX_ETH_MII_CR_AUTO_EN    0x1000  /* auto-negotiation enable */
#define IX_ETH_MII_CR_100	 0x2000  /* 0 = 10mb, 1 = 100mb */
#define IX_ETH_MII_CR_LOOPBACK   0x4000  /* 0 = normal, 1 = loopback */
#define IX_ETH_MII_CR_RESET	 0x8000  /* 0 = normal, 1 = PHY reset */
#define IX_ETH_MII_CR_NORM_EN    0x0000  /* just enable the PHY */
#define IX_ETH_MII_CR_DEF_0_MASK 0xca7f  /* they must return zero */
#define IX_ETH_MII_CR_RES_MASK   0x007f  /* reserved bits, return zero */

/* MII Status register bit definitions */

#define IX_ETH_MII_SR_LINK_STATUS   0x0004 /* link Status -- 1 = link */
#define IX_ETH_MII_SR_AUTO_SEL      0x0008 /* auto speed select capable */
#define IX_ETH_MII_SR_REMOTE_FAULT  0x0010 /* Remote fault detect */
#define IX_ETH_MII_SR_AUTO_NEG      0x0020 /* auto negotiation complete */
#define IX_ETH_MII_SR_10T_HALF_DPX  0x0800 /* 10BaseT HD capable */
#define IX_ETH_MII_SR_10T_FULL_DPX  0x1000 /* 10BaseT FD capable */
#define IX_ETH_MII_SR_TX_HALF_DPX   0x2000 /* TX HD capable */
#define IX_ETH_MII_SR_TX_FULL_DPX   0x4000 /* TX FD capable */
#define IX_ETH_MII_SR_T4            0x8000 /* T4 capable */
#define IX_ETH_MII_SR_ABIL_MASK     0xff80 /* abilities mask */
#define IX_ETH_MII_SR_EXT_CAP       0x0001 /* extended capabilities */


/* LXT971/2 Status 2 register bit definitions */
#define IX_ETH_MII_SR2_100          0x4000
#define IX_ETH_MII_SR2_TX           0x2000
#define IX_ETH_MII_SR2_RX           0x1000
#define IX_ETH_MII_SR2_COL          0x0800
#define IX_ETH_MII_SR2_LINK         0x0400
#define IX_ETH_MII_SR2_FD           0x0200
#define IX_ETH_MII_SR2_AUTO         0x0100
#define IX_ETH_MII_SR2_AUTO_CMPLT   0x0080
#define IX_ETH_MII_SR2_POLARITY     0x0020
#define IX_ETH_MII_SR2_PAUSE        0x0010
#define IX_ETH_MII_SR2_ERROR        0x0008

/* MII Link Code word  bit definitions */

#define IX_ETH_MII_BP_FAULT	0x2000       	/* remote fault */
#define IX_ETH_MII_BP_ACK	0x4000       	/* acknowledge */
#define IX_ETH_MII_BP_NP	0x8000       	/* nexp page is supported */

/* MII Next Page bit definitions */

#define IX_ETH_MII_NP_TOGGLE 0x0800       	/* toggle bit */
#define IX_ETH_MII_NP_ACK2	 0x1000       	/* acknowledge two */
#define IX_ETH_MII_NP_MSG	 0x2000       	/* message page */
#define IX_ETH_MII_NP_ACK1	 0x4000       	/* acknowledge one */
#define IX_ETH_MII_NP_NP	 0x8000       	/* nexp page will follow */

/* MII Expansion Register bit definitions */

#define IX_ETH_MII_EXP_FAULT    0x0010  /* parallel detection fault */
#define IX_ETH_MII_EXP_PRTN_NP  0x0008  /* link partner next-page able */
#define IX_ETH_MII_EXP_LOC_NP   0x0004  /* local PHY next-page able */
#define IX_ETH_MII_EXP_PR	    0x0002  /* full page received */
#define IX_ETH_MII_EXP_PRT_AN   0x0001  /* link partner auto neg able */

/* technology ability field bit definitions */

#define IX_ETH_MII_TECH_10BASE_T	  0x0020  /* 10Base-T */
#define IX_ETH_MII_TECH_10BASE_FD	  0x0040  /* 10Base-T Full Duplex */
#define IX_ETH_MII_TECH_100BASE_TX	  0x0080  /* 100Base-TX */
#define IX_ETH_MII_TECH_100BASE_TX_FD 0x0100  /* 100Base-TX Full Duplex */

#define IX_ETH_MII_TECH_100BASE_T4	0x0200	/* 100Base-T4 */
#define IX_ETH_MII_ADS_TECH_MASK	0x1fe0	/* technology abilities mask */
#define IX_ETH_MII_TECH_MASK	IX_ETH_MII_ADS_TECH_MASK
#define IX_ETH_MII_ADS_SEL_MASK	0x001f	/* selector field mask */

#define IX_ETH_MII_AN_FAIL     0x10    /* auto-negotiation fail */
#define IX_ETH_MII_STAT_FAIL   0x20    /* errors in the status register */
#define IX_ETH_MII_PHY_NO_ABLE 0x40    /* the PHY lacks some abilities */

/* Definitions for MII access routines*/

#define IX_ETH_MII_GO                  BIT(31)
#define IX_ETH_MII_WRITE               BIT(26)
#define IX_ETH_MII_TIMEOUT_10TH_SECS       (5)    
#define IX_ETH_MII_10TH_SEC_IN_MILLIS    (100)              
#define IX_ETH_MII_READ_FAIL           BIT(31)

/* When we reset the PHY we delay for 2 seconds to allow the reset to 
   complete*/
#define IX_ETH_MII_RESET_DELAY_MS  (2000)     
#define IX_ETH_MII_RESET_POLL_MS     (50)

#define IX_ETH_MII_REG_SHL    16
#define IX_ETH_MII_ADDR_SHL   21

/* supported PHYs */
#define IX_ETH_MII_LXT971_PHY_ID	0x001378E0
#define IX_ETH_MII_LXT972_PHY_ID	0x001378E2
#define IX_ETH_MII_LXT973_PHY_ID	0x00137A10
#define IX_ETH_MII_LXT973A3_PHY_ID	0x00137A11
#define IX_ETH_MII_KS8995_PHY_ID	0x00221450
#define IX_ETH_MII_LXT9785_PHY_ID	0x001378FF	


#define IX_ETH_MII_INVALID_PHY_ID 0x00000000
#define IX_ETH_MII_UNKNOWN_PHY_ID 0xffffffff

#endif  /*IxEthAccMii_p_H*/
