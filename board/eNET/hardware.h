/*
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_

#define LED_LATCH_ADDRESS	0x1002
#define LED_RUN_BITMASK		0x01
#define LED_1_BITMASK		0x02
#define LED_2_BITMASK		0x04
#define LED_RX_BITMASK		0x08
#define LED_TX_BITMASK		0x10
#define LED_ERR_BITMASK		0x20
#define WATCHDOG_PIO_BIT	0x8000

#endif /* HARDWARE_H_ */
