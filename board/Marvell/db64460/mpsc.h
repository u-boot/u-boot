/*
 * (C) Copyright 2001
 * John Clemens <clemens@mclx.com>, Mission Critical Linux, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 */

/*************************************************************************
 * changes for Marvell DB64460 eval board 2003 by Ingo Assmus <ingo.assmus@keymile.com>
 *
  ************************************************************************/


/*
 * mpsc.h - header file for MPSC in uart mode (console driver)
 */

#ifndef __MPSC_H__
#define __MPSC_H__

/* include actual Galileo defines */
#include "../include/mv_gen_reg.h"

/* driver related defines */

int mpsc_init(int baud);
void mpsc_sdma_init(void);
void mpsc_init2(void);
int galbrg_set_baudrate(int channel, int rate);

int mpsc_putchar_early(char ch);
char mpsc_getchar_debug(void);
int mpsc_test_char_debug(void);

int mpsc_test_char_sdma(void);

extern int (*mpsc_putchar)(char ch);
extern char (*mpsc_getchar)(void);
extern int (*mpsc_test_char)(void);

#define CHANNEL CONFIG_MPSC_PORT

#define TX_DESC     5
#define RX_DESC     20

#define DESC_FIRST  0x00010000
#define DESC_LAST   0x00020000
#define DESC_OWNER_BIT  0x80000000

#define TX_DEMAND   0x00800000
#define TX_STOP     0x00010000
#define RX_ENABLE   0x00000080

#define SDMA_RX_ABORT		  (1 << 15)
#define SDMA_TX_ABORT		  (1 << 31)
#define MPSC_TX_ABORT		  (1 << 7)
#define MPSC_RX_ABORT             (1 << 23)
#define MPSC_ENTER_HUNT           (1 << 31)

/* MPSC defines */

#define GALMPSC_CONNECT            0x1
#define GALMPSC_DISCONNECT         0x0

#define GALMPSC_UART               0x1

#define GALMPSC_STOP_BITS_1        0x0
#define GALMPSC_STOP_BITS_2        0x1
#define GALMPSC_CHAR_LENGTH_8      0x3
#define GALMPSC_CHAR_LENGTH_7      0x2

#define GALMPSC_PARITY_ODD         0x0
#define GALMPSC_PARITY_EVEN        0x2
#define GALMPSC_PARITY_MARK        0x3
#define GALMPSC_PARITY_SPACE       0x1
#define GALMPSC_PARITY_NONE        -1

#define GALMPSC_SERIAL_MULTIPLEX   SERIAL_PORT_MULTIPLEX           /* 0xf010 */
#define GALMPSC_ROUTING_REGISTER   MAIN_ROUTING_REGISTER           /* 0xb400 */
#define GALMPSC_RxC_ROUTE          RECEIVE_CLOCK_ROUTING_REGISTER  /* 0xb404 */
#define GALMPSC_TxC_ROUTE          TRANSMIT_CLOCK_ROUTING_REGISTER /* 0xb408 */
#define GALMPSC_MCONF_LOW          MPSC0_MAIN_CONFIGURATION_LOW    /* 0x8000 */
#define GALMPSC_MCONF_HIGH         MPSC0_MAIN_CONFIGURATION_HIGH   /* 0x8004 */
#define GALMPSC_PROTOCONF_REG      MPSC0_PROTOCOL_CONFIGURATION    /* 0x8008 */

#define GALMPSC_REG_GAP            0x1000

#define GALMPSC_MCONF_CHREG_BASE   CHANNEL0_REGISTER1  /* 0x800c */
#define GALMPSC_CHANNELREG_1       CHANNEL0_REGISTER1  /* 0x800c */
#define GALMPSC_CHANNELREG_2       CHANNEL0_REGISTER2  /* 0x8010 */
#define GALMPSC_CHANNELREG_3       CHANNEL0_REGISTER3  /* 0x8014 */
#define GALMPSC_CHANNELREG_4       CHANNEL0_REGISTER4  /* 0x8018 */
#define GALMPSC_CHANNELREG_5       CHANNEL0_REGISTER5  /* 0x801c */
#define GALMPSC_CHANNELREG_6       CHANNEL0_REGISTER6  /* 0x8020 */
#define GALMPSC_CHANNELREG_7       CHANNEL0_REGISTER7  /* 0x8024 */
#define GALMPSC_CHANNELREG_8       CHANNEL0_REGISTER8  /* 0x8028 */
#define GALMPSC_CHANNELREG_9       CHANNEL0_REGISTER9  /* 0x802c */
#define GALMPSC_CHANNELREG_10      CHANNEL0_REGISTER10 /* 0x8030 */
#define GALMPSC_CHANNELREG_11      CHANNEL0_REGISTER11 /* 0x8034 */

#define GALSDMA_COMMAND_FIRST     (1 << 16)
#define GALSDMA_COMMAND_LAST      (1 << 17)
#define GALSDMA_COMMAND_ENABLEINT (1 << 23)
#define GALSDMA_COMMAND_AUTO      (1 << 30)
#define GALSDMA_COMMAND_OWNER     (1 << 31)

#define GALSDMA_RX                 0
#define GALSDMA_TX                 1

/* CHANNEL2 should be CHANNEL1, according to documentation,
 * but to work with the current GTREGS file...
 */
#define GALSDMA_0_CONF_REG         CHANNEL0_CONFIGURATION_REGISTER   /* 0x4000 */
#define GALSDMA_1_CONF_REG         CHANNEL2_CONFIGURATION_REGISTER   /* 0x6000 */
#define GALSDMA_0_COM_REG          CHANNEL0_COMMAND_REGISTER         /* 0x4008 */
#define GALSDMA_1_COM_REG          CHANNEL2_COMMAND_REGISTER         /* 0x6008 */
#define GALSDMA_0_CUR_RX_PTR       CHANNEL0_CURRENT_RX_DESCRIPTOR_POINTER  /* 0x4810 */
#define GALSDMA_0_CUR_TX_PTR       CHANNEL0_CURRENT_TX_DESCRIPTOR_POINTER  /* 0x4c10 */
#define GALSDMA_0_FIR_TX_PTR       CHANNEL0_FIRST_TX_DESCRIPTOR_POINTER    /* 0x4c14 */
#define GALSDMA_1_CUR_RX_PTR       CHANNEL2_CURRENT_RX_DESCRIPTOR_POINTER  /* 0x6810 */
#define GALSDMA_1_CUR_TX_PTR       CHANNEL2_CURRENT_TX_DESCRIPTOR_POINTER  /* 0x6c10 */
#define GALSDMA_1_FIR_TX_PTR       CHANNEL2_FIRST_TX_DESCRIPTOR_POINTER    /* 0x6c14 */
#define GALSDMA_REG_DIFF           0x2000

/* WRONG in gt64260R.h */
#define GALSDMA_INT_CAUSE          0xb800   /* SDMA_CAUSE */
#define GALSDMA_INT_MASK           0xb880   /* SDMA_MASK  */
#define GALMPSC_0_INT_CAUSE        0xb804
#define GALMPSC_0_INT_MASK         0xb884

#define GALSDMA_MODE_UART          0
#define GALSDMA_MODE_BISYNC        1
#define GALSDMA_MODE_HDLC          2
#define GALSDMA_MODE_TRANSPARENT   3

#define GALBRG_0_CONFREG           BRG0_CONFIGURATION_REGISTER  /*  0xb200  */
#define GALBRG_REG_GAP             0x0008
#define GALBRG_0_BTREG             BRG0_BAUDE_TUNING_REGISTER   /*  0xb204  */

#endif /* __MPSC_H__ */
