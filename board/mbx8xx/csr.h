#ifndef __csr_h
#define __csr_h

/*
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Control and Status Register definitions for the MBX
 *
 *--------------------------------------------------------------------
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

/* bits for control register #1 / status register #1 */
#define CSR1_ETEN       0x80    /* Ethernet Transceiver Enabled             */
#define CSR1_ELEN       0x40    /* Ethernet XCVR in Internal Loopback       */
#define CSR1_EAEN       0x20    /* Auto selection TP/AUI Enabled            */
#define CSR1_TPEN       0x10    /* TP manually selected                     */
#define CSR1_FDDIS      0x08    /* Full Duplex Mode disabled                */
#define CSR1_FCTEN      0x04    /* Collision Testing of XCVR disabled       */
#define CSR1_COM1EN     0x02    /* COM1 signals routed to RS232 Transceiver */
#define CSR1_XCVRDIS    0x01    /* Onboard RS232 Transceiver Disabled       */

/* bits for control register #2 */
#define CR2_VDDSEL      0xC0    /* PCMCIA Supply Voltage                    */
#define CR2_VPPSEL      0x30    /* PCMCIA Programming Voltage               */
#define CR2_BRDFAIL     0x08    /* Board fail                               */
#define CR2_SWS1        0x04    /* Software Status #2 LED                   */
#define CR2_SWS2        0x02    /* Software Status #2 LED                   */
#define CR2_QSPANRST    0x01    /* Reset QSPAN                              */

/* bits for status register #2 */
#define SR2_VDDSEL      0xC0    /* PCMCIA Supply Voltage                    */
#define SR2_VPPSEL      0x30    /* PCMCIA Programming Voltage               */
#define SR2_BATGD       0x08    /* Low Voltage indication for onboard bat   */
#define SR2_NVBATGD     0x04    /* Low Voltage indication for NVRAM         */
#define SR2_RDY         0x02    /* Flash programming status bit             */
#define SR2_FT          0x01    /* Reserved for Factory test purposes       */

#define MBX_CSR1 (*((uchar *)CFG_CSR_BASE))
#define MBX_CSR2 (*((uchar *)CFG_CSR_BASE + 1))

#endif /* __csr_h */
