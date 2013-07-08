/*
 *
 * BRIEF MODULE DESCRIPTION
 *   TI H2 and P2 Debug Board hardware map
 *
 * Copyright (C) 2004 MPC-Data Limited. (http://www.mpc-data.co.uk)
 * Author: MPC-Data Limited
 *	   Dave Peverley
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __INCLUDED_H2_P2_DBH_BOARD_H
#define __INCLUDED_H2_P2_DBH_BOARD_H

#include <asm/sizes.h>

/*
 * The Debug board is designed to function with the P2 Sample, H2
 * Sample and 1610 Innovator boards. The main difference AFAICT is
 * the chip selects used with each system ;
 *
 *   P2 Sample : CS1 of OMAP730 is used to select the CPLD & LAN regs
 *   H2 Sample : CS1a is used to select the CPLD registers.
 *
 */

/***************************************************************************
 * CPLD Registers
 **************************************************************************/

#define H2DBG_CPLD_REVISION        0x04000010
#define H2DBG_BOARD_REVISION       0x04000012
#define H2DBG_GPIO_REGISTER        0x04000014
#define H2DBG_LED_CONTROL          0x04000016
#define H2DBG_MISC_INPUT           0x04000018
#define H2DBG_LAN_STATUS           0x0400001A
#define H2DBG_LAN_RESET            0x0400001C
#define H2DBG_ETH_REG_BASE         0x04000300

/***************************************************************************
 * Ethernet Control Registers
 * These are for the LAN91C96 on the debug board
 **************************************************************************/

/* Bank 0 in IO space */

#define ETH_TCR            (H2DBG_ETH_REG_BASE + 0x00)   /* Transmit Control Register */
#define ETH_EPH_STATUS     (H2DBG_ETH_REG_BASE + 0x02)   /* EPH Status Register */
#define ETH_RCR            (H2DBG_ETH_REG_BASE + 0x04)   /* Receive Control Register */
#define ETH_COUNTER        (H2DBG_ETH_REG_BASE + 0x06)   /* Counter Register */
#define ETH_MIR            (H2DBG_ETH_REG_BASE + 0x08)   /* Memory Information Register */
#define ETH_MCR            (H2DBG_ETH_REG_BASE + 0x0A)   /* Memory Configuration Register */

/* Bank 1 in IO space */

#define ETH_CONFIG         (H2DBG_ETH_REG_BASE + 0x00)   /* Configuration Register */
#define ETH_BASE           (H2DBG_ETH_REG_BASE + 0x02)   /* Base Address Register */
#define ETH_IA0            (H2DBG_ETH_REG_BASE + 0x04)   /* Individual Address Register - 0 */
#define ETH_IA1            (H2DBG_ETH_REG_BASE + 0x05)   /* Individual Address Register - 1 */
#define ETH_IA2            (H2DBG_ETH_REG_BASE + 0x06)   /* Individual Address Register - 2 */
#define ETH_IA3            (H2DBG_ETH_REG_BASE + 0x07)   /* Individual Address Register - 3 */
#define ETH_IA4            (H2DBG_ETH_REG_BASE + 0x08)   /* Individual Address Register - 4 */
#define ETH_IA5            (H2DBG_ETH_REG_BASE + 0x09)   /* Individual Address Register - 5 */
#define ETH_GEN_PURPOSE    (H2DBG_ETH_REG_BASE + 0x0A)   /* General Address Registers */
#define ETH_CONTROL        (H2DBG_ETH_REG_BASE + 0x0B)   /* Control Register */

/* Bank 2 in IO space */

#define ETH_MMU            (H2DBG_ETH_REG_BASE + 0x00)   /* MMU Command Register */
#define ETH_AUTO_TX_START  (H2DBG_ETH_REG_BASE + 0x01)   /* Auto Tx Start Register */
#define ETH_PNR            (H2DBG_ETH_REG_BASE + 0x02)   /* Packet Number Register */
#define ETH_ARR            (H2DBG_ETH_REG_BASE + 0x03)   /* Allocation Result Register */
#define ETH_FIFO           (H2DBG_ETH_REG_BASE + 0x04)   /* FIFO Ports Register */
#define ETH_POINTER        (H2DBG_ETH_REG_BASE + 0x06)   /* Pointer Register */
#define ETH_DATA_HIGH      (H2DBG_ETH_REG_BASE + 0x08)   /* Data High Register */
#define ETH_DATA_LOW       (H2DBG_ETH_REG_BASE + 0x0A)   /* Data Low Register */
#define ETH_INT_STATS      (H2DBG_ETH_REG_BASE + 0x0C)   /* Interrupt Status Register - RO */
#define ETH_INT_ACK        (H2DBG_ETH_REG_BASE + 0x0C)   /* Interrupt Acknowledge Register -WO */
#define ETH_INT_MASK       (H2DBG_ETH_REG_BASE + 0x0D)   /* Interrupt Mask Register */


#ifndef __ASSEMBLY__

/*
 * A couple of utility inlines to aid debugging using the LED's on the
 * debug board.
 */

static inline void set_led_state(int state)
{
	static unsigned long hw_led_state = 0;
	volatile unsigned short *led_address = (volatile unsigned short *)0x04000016;

	hw_led_state = ((unsigned long)state);
	*((unsigned short *) (led_address)) = (unsigned short) (~hw_led_state & 0xFFFF);
}


static inline void spin_up_leds(void)
{
	volatile int i, j, k;

	for (k = 0; k < 2; k++) {
		for (i = 0; i < 16; i++) {
			for (j = 0; j < 5000; j++) {
				set_led_state(1 << i);
			}
		}
		for (i = 15; i >= 0; i--) {
			for (j = 0; j < 5000; j++) {
				set_led_state(1 << i);
			}
		}
	}
}

#endif    /* !  __ASSEMBLY__ */

#endif    /* !  __INCLUDED_H2_P2_DBH_BOARD_H */
