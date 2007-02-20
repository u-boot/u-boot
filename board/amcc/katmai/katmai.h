/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

#ifndef __KATMAI_H_
#define __KATMAI_H_

/*----------------------------------------------------------------------------
 *                    XX
 *   XXXX    XX XXX   XXX     XXXX
 * XX        XX  XX   XX    XX  XX
 * XX  XXX   XX  XX   XX    XX  XX
 * XX  XX    XXXXX    XX    XX  XX
 *  XXXX     XX      XXXX    XXXX
 *          XXXX
 *
 *  The 440SPe provices 32 bits of GPIO.  By default all GPIO pins
 *  are disabled, and must be explicitly enabled by setting a
 *  bit in the SDR0_PFC0 indirect DCR.  Each GPIO maps 1-to-1 with the
 *  corresponding bit in the SDR0_PFC0 register (note that bit numbers
 *  reflect the PowerPC convention where bit 0 is the most-significant
 *  bit).
 *
 *   Katmai specific:
 *      RS232_RX_EN# is held HIGH during reset by hardware, keeping the
 *      RS232_CTS, DSR & DCD  signals coming from the MAX3411 (U26) in
 *      Hi-Z condition. This prevents contention between the MAX3411 (U26)
 *      and 74CBTLV3125PG (U2) during reset.
 *
 *      RS232_RX_EN# is connected as GPIO pin 30.  Once the processor
 *      is released from reset, this pin must be configured as an output and
 *      then driven high to enable the receive signals from the UART transciever.
 *----------------------------------------------------------------------------*/
#define GPIO_ENABLE(gpio)       (0x80000000 >> (gpio))

#define PFC0_KATMAI             GPIO_ENABLE(30)
#define GPIO_OR_KATMAI          GPIO_ENABLE(30)     /* Drive all outputs low except GPIO 30 */
#define GPIO_TCR_KATMAI         GPIO_ENABLE(30)
#define GPIO_ODR_KATMAI         0                   /* Disable open drain for all outputs */

#define GPIO0_OR_ADDR           (CFG_PERIPHERAL_BASE + 0x700)
#define GPIO0_TCR_ADDR          (CFG_PERIPHERAL_BASE + 0x704)
#define GPIO0_ODR_ADDR          (CFG_PERIPHERAL_BASE + 0x718)
#define GPIO0_IR_ADDR           (CFG_PERIPHERAL_BASE + 0x71C)

#endif /* __KATMAI_H_ */
