/***********************************************************************
 *
 * Copyright (C) 2004 by FS Forth-Systeme GmbH.
 * All rights reserved.
 *
 * $Id: ns9750_eth.h,v 1.2 2004/02/24 13:25:39 mpietrek Exp $
 * @Author: Markus Pietrek
 * @References: [1] NS9750 Hardware Reference, December 2003
 *              [2] Intel LXT971 Datasheet #249414 Rev. 02
 *              [3] NS7520 Linux Ethernet Driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 ***********************************************************************/

#ifndef __LXT971A_H__
#define __LXT971A_H__

/* PHY definitions (LXT971A) [2] */
#define PHY_COMMON_CTRL    	 	(0x00)
#define PHY_COMMON_STAT    	 	(0x01)
#define PHY_COMMON_ID1    	 	(0x02)
#define PHY_COMMON_ID2           	(0x03)
#define PHY_COMMON_AUTO_ADV      	(0x04)
#define PHY_COMMON_AUTO_LNKB     	(0x05)
#define PHY_COMMON_AUTO_EXP      	(0x06)
#define PHY_COMMON_AUTO_NEXT     	(0x07)
#define PHY_COMMON_AUTO_LNKN     	(0x08)
#define PHY_LXT971_PORT_CFG      	(0x10)
#define PHY_LXT971_STAT2         	(0x11)
#define PHY_LXT971_INT_ENABLE    	(0x12)
#define PHY_LXT971_INT_STATUS    	(0x13)
#define PHY_LXT971_LED_CFG       	(0x14)
#define PHY_LXT971_DIG_CFG       	(0x1A)
#define PHY_LXT971_TX_CTRL       	(0x1E)

/* CTRL PHY Control Register Bit Fields */
#define PHY_COMMON_CTRL_RESET  	 	(0x8000)
#define PHY_COMMON_CTRL_LOOPBACK 	(0x4000)
#define PHY_COMMON_CTRL_SPD_MA   	(0x2040)
#define PHY_COMMON_CTRL_SPD_10   	(0x0000)
#define PHY_COMMON_CTRL_SPD_100  	(0x2000)
#define PHY_COMMON_CTRL_SPD_1000 	(0x0040)
#define PHY_COMMON_CTRL_SPD_RES  	(0x2040)
#define PHY_COMMON_CTRL_AUTO_NEG 	(0x1000)
#define PHY_COMMON_CTRL_POWER_DN 	(0x0800)
#define PHY_COMMON_CTRL_ISOLATE	 	(0x0400)
#define PHY_COMMON_CTRL_RES_AUTO 	(0x0200)
#define PHY_COMMON_CTRL_DUPLEX	 	(0x0100)
#define PHY_COMMON_CTRL_COL_TEST 	(0x0080)
#define PHY_COMMON_CTRL_RES1     	(0x003F)

/* STAT Status Register Bit Fields */
#define PHY_COMMON_STAT_100BT4	 	(0x8000)
#define PHY_COMMON_STAT_100BXFD	 	(0x4000)
#define PHY_COMMON_STAT_100BXHD	 	(0x2000)
#define PHY_COMMON_STAT_10BTFD	 	(0x1000)
#define PHY_COMMON_STAT_10BTHD	 	(0x0800)
#define PHY_COMMON_STAT_100BT2FD 	(0x0400)
#define PHY_COMMON_STAT_100BT2HD 	(0x0200)
#define PHY_COMMON_STAT_EXT_STAT 	(0x0100)
#define PHY_COMMON_STAT_RES1	 	(0x0080)
#define PHY_COMMON_STAT_MF_PSUP	 	(0x0040)
#define PHY_COMMON_STAT_AN_COMP  	(0x0020)
#define PHY_COMMON_STAT_RMT_FLT	 	(0x0010)
#define PHY_COMMON_STAT_AN_CAP	 	(0x0008)
#define PHY_COMMON_STAT_LNK_STAT 	(0x0004)
#define PHY_COMMON_STAT_JAB_DTCT 	(0x0002)
#define PHY_COMMON_STAT_EXT_CAP	 	(0x0001)

/* AUTO_ADV Auto-neg Advert Register Bit Fields */
#define PHY_COMMON_AUTO_ADV_NP       	(0x8000)
#define PHY_COMMON_AUTO_ADV_RES1        (0x4000)
#define PHY_COMMON_AUTO_ADV_RMT_FLT     (0x2000)
#define PHY_COMMON_AUTO_ADV_RES2        (0x1000)
#define PHY_COMMON_AUTO_ADV_AS_PAUSE    (0x0800)
#define PHY_COMMON_AUTO_ADV_PAUSE       (0x0400)
#define PHY_COMMON_AUTO_ADV_100BT4      (0x0200)
#define PHY_COMMON_AUTO_ADV_100BTXFD   	(0x0100)
#define PHY_COMMON_AUTO_ADV_100BTX      (0x0080)
#define PHY_COMMON_AUTO_ADV_10BTFD   	(0x0040)
#define PHY_COMMON_AUTO_ADV_10BT     	(0x0020)
#define PHY_COMMON_AUTO_ADV_SEL_FLD_MA  (0x001F)
#define PHY_COMMON_AUTO_ADV_802_9       (0x0002)
#define PHY_COMMON_AUTO_ADV_802_3       (0x0001)

/* AUTO_LNKB Auto-neg Link Ability Register Bit Fields */
#define PHY_COMMON_AUTO_LNKB_NP       	(0x8000)
#define PHY_COMMON_AUTO_LNKB_ACK        (0x4000)
#define PHY_COMMON_AUTO_LNKB_RMT_FLT    (0x2000)
#define PHY_COMMON_AUTO_LNKB_RES2       (0x1000)
#define PHY_COMMON_AUTO_LNKB_AS_PAUSE   (0x0800)
#define PHY_COMMON_AUTO_LNKB_PAUSE      (0x0400)
#define PHY_COMMON_AUTO_LNKB_100BT4     (0x0200)
#define PHY_COMMON_AUTO_LNKB_100BTXFD   (0x0100)
#define PHY_COMMON_AUTO_LNKB_100BTX     (0x0080)
#define PHY_COMMON_AUTO_LNKB_10BTFD   	(0x0040)
#define PHY_COMMON_AUTO_LNKB_10BT     	(0x0020)
#define PHY_COMMON_AUTO_LNKB_SEL_FLD_MA (0x001F)
#define PHY_COMMON_AUTO_LNKB_802_9      (0x0002)
#define PHY_COMMON_AUTO_LNKB_802_3      (0x0001)

/* AUTO_EXP Auto-neg Expansion Register Bit Fields */
#define PHY_COMMON_AUTO_EXP_RES1        (0xFFC0)
#define PHY_COMMON_AUTO_EXP_BASE_PAGE   (0x0020)
#define PHY_COMMON_AUTO_EXP_PAR_DT_FLT  (0x0010)
#define PHY_COMMON_AUTO_EXP_LNK_NP_CAP  (0x0008)
#define PHY_COMMON_AUTO_EXP_NP_CAP      (0x0004)
#define PHY_COMMON_AUTO_EXP_PAGE_REC    (0x0002)
#define PHY_COMMON_AUTO_EXP_LNK_AN_CAP  (0x0001)

/* AUTO_NEXT Aut-neg Next Page Tx Register Bit Fields */
#define PHY_COMMON_AUTO_NEXT_NP         (0x8000)
#define PHY_COMMON_AUTO_NEXT_RES1       (0x4000)
#define PHY_COMMON_AUTO_NEXT_MSG_PAGE   (0x2000)
#define PHY_COMMON_AUTO_NEXT_ACK_2      (0x1000)
#define PHY_COMMON_AUTO_NEXT_TOGGLE     (0x0800)
#define PHY_COMMON_AUTO_NEXT_MSG        (0x07FF)

/* AUTO_LNKN Auto-neg Link Partner Rx Reg Bit Fields */
#define PHY_COMMON_AUTO_LNKN_NP         (0x8000)
#define PHY_COMMON_AUTO_LNKN_ACK        (0x4000)
#define PHY_COMMON_AUTO_LNKN_MSG_PAGE   (0x2000)
#define PHY_COMMON_AUTO_LNKN_ACK_2      (0x1000)
#define PHY_COMMON_AUTO_LNKN_TOGGLE     (0x0800)
#define PHY_COMMON_AUTO_LNKN_MSG        (0x07FF)

/* PORT_CFG Port Configuration Register Bit Fields */
#define PHY_LXT971_PORT_CFG_RES1        (0x8000)
#define PHY_LXT971_PORT_CFG_FORCE_LNK   (0x4000)
#define PHY_LXT971_PORT_CFG_TX_DISABLE  (0x2000)
#define PHY_LXT971_PORT_CFG_BYPASS_SCR  (0x1000)
#define PHY_LXT971_PORT_CFG_RES2        (0x0800)
#define PHY_LXT971_PORT_CFG_JABBER      (0x0400)
#define PHY_LXT971_PORT_CFG_SQE	        (0x0200)
#define PHY_LXT971_PORT_CFG_TP_LOOPBACK (0x0100)
#define PHY_LXT971_PORT_CFG_CRS_SEL     (0x0080)
#define PHY_LXT971_PORT_CFG_SLEEP_MODE  (0x0040)
#define PHY_LXT971_PORT_CFG_PRE_EN      (0x0020)
#define PHY_LXT971_PORT_CFG_SLEEP_T_MA  (0x0018)
#define PHY_LXT971_PORT_CFG_SLEEP_T_104 (0x0010)
#define PHY_LXT971_PORT_CFG_SLEEP_T_200 (0x0001)
#define PHY_LXT971_PORT_CFG_SLEEP_T_304 (0x0000)
#define PHY_LXT971_PORT_CFG_FLT_CODE_EN (0x0004)
#define PHY_LXT971_PORT_CFG_ALT_NP      (0x0002)
#define PHY_LXT971_PORT_CFG_FIBER_SEL   (0x0001)

/* STAT2 Status Register #2 Bit Fields */
#define PHY_LXT971_STAT2_RES1   	(0x8000)
#define PHY_LXT971_STAT2_100BTX 	(0x4000)
#define PHY_LXT971_STAT2_TX_STATUS	(0x2000)
#define PHY_LXT971_STAT2_RX_STATUS	(0x1000)
#define PHY_LXT971_STAT2_COL_STATUS	(0x0800)
#define PHY_LXT971_STAT2_LINK   	(0x0400)
#define PHY_LXT971_STAT2_DUPLEX_MODE	(0x0200)
#define PHY_LXT971_STAT2_AUTO_NEG	(0x0100)
#define PHY_LXT971_STAT2_AUTO_NEG_COMP 	(0x0080)
#define PHY_LXT971_STAT2_RES2   	(0x0040)
#define PHY_LXT971_STAT2_POLARITY	(0x0020)
#define PHY_LXT971_STAT2_PAUSE  	(0x0010)
#define PHY_LXT971_STAT2_ERROR  	(0x0008)
#define PHY_LXT971_STAT2_RES3   	(0x0007)

/* INT_ENABLE Interrupt Enable Register Bit Fields */
#define PHY_LXT971_INT_ENABLE_RES1      (0xFF00)
#define PHY_LXT971_INT_ENABLE_ANMSK     (0x0080)
#define PHY_LXT971_INT_ENABLE_SPEEDMSK  (0x0040)
#define PHY_LXT971_INT_ENABLE_DUPLEXMSK (0x0020)
#define PHY_LXT971_INT_ENABLE_LINKMSK   (0x0010)
#define PHY_LXT971_INT_ENABLE_RES2      (0x000C)
#define PHY_LXT971_INT_ENABLE_INTEN     (0x0002)
#define PHY_LXT971_INT_ENABLE_TINT      (0x0001)

/* INT_STATUS Interrupt Status Register Bit Fields */
#define PHY_LXT971_INT_STATUS_RES1      (0xFF00)
#define PHY_LXT971_INT_STATUS_ANDONE    (0x0080)
#define PHY_LXT971_INT_STATUS_SPEEDCHG  (0x0040)
#define PHY_LXT971_INT_STATUS_DUPLEXCHG (0x0020)
#define PHY_LXT971_INT_STATUS_LINKCHG   (0x0010)
#define PHY_LXT971_INT_STATUS_RES2      (0x0008)
#define PHY_LXT971_INT_STATUS_MDINT     (0x0004)
#define PHY_LXT971_INT_STATUS_RES3      (0x0003)

/* LED_CFG Interrupt LED Configuration Register Bit Fields */
#define PHY_LXT971_LED_CFG_SHIFT_LED1   (0x000C)
#define PHY_LXT971_LED_CFG_SHIFT_LED2   (0x0008)
#define PHY_LXT971_LED_CFG_SHIFT_LED3   (0x0004)
#define PHY_LXT971_LED_CFG_LEDFREQ_MA	(0x000C)
#define PHY_LXT971_LED_CFG_LEDFREQ_RES	(0x000C)
#define PHY_LXT971_LED_CFG_LEDFREQ_100	(0x0008)
#define PHY_LXT971_LED_CFG_LEDFREQ_60	(0x0004)
#define PHY_LXT971_LED_CFG_LEDFREQ_30	(0x0000)
#define PHY_LXT971_LED_CFG_PULSE_STR    (0x0002)
#define PHY_LXT971_LED_CFG_RES1         (0x0001)

/* only one of these values must be shifted for each SHIFT_LED?  */
#define PHY_LXT971_LED_CFG_UNUSED1      (0x000F)
#define PHY_LXT971_LED_CFG_DUPLEX_COL   (0x000E)
#define PHY_LXT971_LED_CFG_LINK_ACT     (0x000D)
#define PHY_LXT971_LED_CFG_LINK_RX      (0x000C)
#define PHY_LXT971_LED_CFG_TEST_BLK_SLW (0x000B)
#define PHY_LXT971_LED_CFG_TEST_BLK_FST (0x000A)
#define PHY_LXT971_LED_CFG_TEST_OFF     (0x0009)
#define PHY_LXT971_LED_CFG_TEST_ON      (0x0008)
#define PHY_LXT971_LED_CFG_RX_OR_TX     (0x0007)
#define PHY_LXT971_LED_CFG_UNUSED2      (0x0006)
#define PHY_LXT971_LED_CFG_DUPLEX       (0x0005)
#define PHY_LXT971_LED_CFG_LINK	        (0x0004)
#define PHY_LXT971_LED_CFG_COLLISION    (0x0003)
#define PHY_LXT971_LED_CFG_RECEIVE      (0x0002)
#define PHY_LXT971_LED_CFG_TRANSMIT     (0x0001)
#define PHY_LXT971_LED_CFG_SPEED        (0x0000)

/* DIG_CFG Digitial Configuration Register Bit Fields */
#define PHY_LXT971_DIG_CFG_RES1 	(0xF000)
#define PHY_LXT971_DIG_CFG_MII_DRIVE	(0x0800)
#define PHY_LXT971_DIG_CFG_RES2 	(0x0400)
#define PHY_LXT971_DIG_CFG_SHOW_SYMBOL	(0x0200)
#define PHY_LXT971_DIG_CFG_RES3 	(0x01FF)

#define PHY_LXT971_MDIO_MAX_CLK		(8000000)
#define PHY_MDIO_MAX_CLK		(2500000)

/* TX_CTRL Transmit Control Register Bit Fields
   documentation is buggy for this register, therefore setting not included */

typedef enum
{
	PHY_NONE    = 0x0000, /* no PHY detected yet */
	PHY_LXT971A = 0x0013
} PhyType;

#endif /* __LXT971A_H__ */
