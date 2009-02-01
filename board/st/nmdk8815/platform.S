/*
 * Board specific setup info
 *
 * (C) Copyright 2005
 * STMicrolelctronics, <www.st.com>
 *
 * (C) Copyright 2004, ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
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

#include <config.h>
#include <version.h>

.globl lowlevel_init
lowlevel_init:
	/* Jump to the flash address */
	ldr r0, =CFG_ONENAND_BASE

	/*
	 * Make it independent whether we boot from 0x0 or 0x30000000.
	 * Non-portable: it relies on the knowledge that ip has to be updated
	 */
	orr ip, ip, r0	/* adjust return address of cpu_init_crit */
	orr lr, lr, r0	/* adjust return address */
	orr pc, pc, r0	/* jump to the normal address */
	nop

	/* Initialize PLL, Remap clear, FSMC, MPMC here! */
	/* What about GPIO, CLCD and UART */

	/* PLL Initialization */
	/* Prog the PLL1 @ 266 MHz ==> SDRAM Clock = 100.8 MHz */
	ldr r0, =NOMADIK_SRC_BASE

	ldr r1, =0x2B013502

	str r1, [r0, #0x14]

	/* Used to set all the timers clock to 2.4MHZ */
	ldr r1, =0x2AAAA004
	str r1, [r0]

	ldr r1, =0x10000000
	str r1, [r0, #0x10]

	/* FSMC setup ---- */
	ldr r0, =NOMADIK_FSMC_BASE

	ldr r1, =0x10DB		/* For 16-bit NOR flash */
	str r1, [r0, #0x08]

	ldr r1, =0x03333333	/* For 16-bit NOR flash */
	str r1, [r0, #0xc]

	/* oneNAND setting */
	ldr r1, =0x0000105B	/* BCR0 Prog control register */
	str r1, [r0]

	ldr r1, =0x0A200551	/* BTR0 Prog timing register */
	str r1, [r0, #0x04]

	/* preload the instructions into icache */
	add r0, pc, #0x1F
	bic r0, r0, #0x1F
	mcr p15, 0, r0, c7, c13, 1
	add r0, r0, #0x20
	mcr p15, 0, r0, c7, c13, 1

	/* Now Clear Remap */
	ldr r0, =NOMADIK_SRC_BASE

	ldr r1, =0x2004
	str r1, [r0]

	ldr r1, =0x10000000
	str r1, [r0, #0x10]

	ldr r0, =0x101E9000
	ldr r1, =0x2004
	str r1, [r0]

	ldr r0, =NOMADIK_SRC_BASE
	ldr r1, =0x2104
	str r1, [r0]

	/* FSMC setup -- */
	mov r0, #(NOMADIK_FSMC_BASE & 0x10000000)
	orr r0, r0, #(NOMADIK_FSMC_BASE & 0x0FFFFFFF)

	ldr r1, =0x10DB		/* For 16-bit NOR flash */
	str r1, [r0, #0x8]

	ldr r1, =0x03333333	/* For 16-bit NOR flash */
	str r1, [r0, #0xc]

	/* MPMC Setup */
	ldr r0, =NOMADIK_MPMC_BASE

	ldr r1, =0xF00003
	str r1, [r0]		/* Enable the MPMC and the DLL */

	ldr r1, =0x183
	str r1, [r0, #0x20]

	ldr r2, =NOMADIK_PMU_BASE

	ldr r1, =0x1111
	str r1, [r2]

	ldr r1, =0x1111		/* Prog the, mand delay strategy */
	str r1, [r0, #0x28]

	ldr r1, =0x103		/* NOP ,mand */
	str r1, [r0, #0x20]

	/* FIXME -- Wait required here */

	ldr r1, =0x103		/* PALL ,mand*/
	str r1, [r0, #0x20]

	ldr r1, =0x1
	str r1, [r0, #0x24]	/* To do at least two auto-refresh */

	/* FIXME -- Wait required here */

	/* Auto-refresh period = 7.8us @ SDRAM Clock = 100.8 MHz */
	ldr r1, =0x31
	str r1, [r0, #0x24]

	/* Prog Little Endian, Not defined in 8800 board */
	ldr r1, =0x0
	str r1,	[r0, #0x8]


	ldr r1, =0x2
	str r1, [r0, #0x30]		/* Prog tRP timing */

	ldr r1, =0x4			/* Change for 8815 */
	str r1, [r0, #0x34]		/* Prog tRAS timing */

	ldr r1, =0xB
	str r1, [r0, #0x38]		/* Prog tSREX timing */


	ldr r1, =0x1
	str r1, [r0, #0x44]		/* Prog tWR timing */

	ldr r1, =0x8
	str r1, [r0, #0x48]		/* Prog tRC timing */

	ldr r1, =0xA
	str r1, [r0, #0x4C]		/* Prog tRFC timing */

	ldr r1, =0xB
	str r1, [r0, #0x50]		/* Prog tXSR timing */

	ldr r1, =0x1
	str r1, [r0, #0x54]		/* Prog tRRD timing */

	ldr r1, =0x1
	str r1, [r0, #0x58]		/* Prog tMRD timing */

	ldr r1, =0x1
	str r1, [r0, #0x5C]		/* Prog tCDLR timing */

	/* DDR-SDRAM MEMORY IS ON BANK0 8815 */
	ldr r1, =0x304			/* Prog RAS and CAS for CS 0 */
	str r1, [r0, #0x104]

	/* SDR-SDRAM MEMORY IS ON BANK1 8815 */
	ldr r1, =0x304			/* Prog RAS and CAS for CS 1 */
	str r1, [r0, #0x124]
	/* THE DATA BUS WIDE IS PROGRAM FOR 16-BITS */
	/* DDR-SDRAM MEMORY IS ON BANK0*/

	ldr r1, =0x884			/* 8815 : config reg in BRC for CS0 */
	str r1, [r0, #0x100]

	/*SDR-SDRAM MEMORY IS ON BANK1*/

	ldr r1, =0x884			/* 8815 : config reg in BRC for CS1 */
	str r1, [r0, #0x120]

	ldr r1, =0x83			/*MODE Mand*/
	str r1, [r0, #0x20]

	/* LOAD MODE REGISTER FOR 2 bursts of 16b, with DDR mem ON BANK0 */

	ldr r1, =0x62000			/*Data in*/
	ldr r1, [r1]

	/* LOAD MODE REGISTER FOR 2 bursts of 16b, with DDR mem ON BANK1 */

	ldr r1, =0x8062000
	ldr r1, [r1]

	ldr r1, =0x003
	str r1, [r0, #0x20]

	/* ENABLE ALL THE BUFFER FOR EACH AHB PORT*/

	ldr r1, =0x01			/* Enable buffer 0 */
	str r1, [r0, #0x400]

	ldr r1, =0x01			/* Enable buffer 1 */
	str r1, [r0, #0x420]

	ldr r1, =0x01			/* Enable buffer 2 */
	str r1, [r0, #0x440]

	ldr r1, =0x01			/* Enable buffer 3 */
	str r1, [r0, #0x460]

	ldr r1, =0x01			/* Enable buffer 4 */
	str r1, [r0, #0x480]

	ldr r1, =0x01			/* Enable buffer 5 */
	str r1, [r0, #0x4A0]

	/* GPIO settings */

	ldr r0, =NOMADIK_GPIO1_BASE

	ldr r1, =0xC0600000
	str r1, [r0, #0x20]

	ldr r1, =0x3F9FFFFF		/* ABHI change this for uart1 */
	str r1, [r0, #0x24]

	ldr r1, =0x3F9FFFFF		/* ABHI change this for uart1 */
	str r1, [r0, #0x28]

	ldr r0, =NOMADIK_GPIO0_BASE

	ldr r1, =0xFFFFFFFF
	str r1, [r0, #0x20]

	ldr r1, =0x00
	str r1, [r0, #0x24]

	ldr r1, =0x00
	str r1, [r0, #0x28]

	/* Configure CPLD_CTRL register for enabling MUX logic for UART0/UART2 */

	ldr r0, =NOMADIK_FSMC_BASE

	ldr r1, =0x10DB			/* INIT FSMC bank 0 */
	str r1, [r0, #0x00]

	ldr r1, =0x0FFFFFFF
	str r1, [r0, #0x04]

	ldr r1, =0x010DB		/* INIT FSMC bank 1 */
	str r1, [r0, #0x08]

	ldr r1, =0x00FFFFFFF
	str r1, [r0, #0x0C]

	ldr r0, =NOMADIK_UART0_BASE

	ldr r1, =0x00000000
	str r1, [r0, #0x30]

	ldr r1, =0x0000004e
	str r1, [r0, #0x24]

	ldr r1, =0x00000008
	str r1, [r0, #0x28]

	ldr r1, =0x00000060
	str r1, [r0, #0x2C]

	ldr r1, =0x00000301
	str r1, [r0, #0x30]

	ldr r1, =0x00000066
	str r1, [r0]

	ldr r0, =NOMADIK_UART1_BASE

	ldr r1, =0x00000000
	str r1, [r0, #0x30]

	ldr r1, =0x0000004e
	str r1, [r0, #0x24]

	ldr r1, =0x00000008
	str r1, [r0, #0x28]

	ldr r1, =0x00000060
	str r1, [r0, #0x2C]

	ldr r1, =0x00000301
	str r1, [r0, #0x30]

	ldr r1, =0x00000066
	str r1, [r0]

	ldr r0, =NOMADIK_UART2_BASE

	ldr r1, =0x00000000
	str r1, [r0, #0x30]

	ldr r1, =0x0000004e
	str r1, [r0, #0x24]

	ldr r1, =0x00000008
	str r1, [r0, #0x28]

	ldr r1, =0x00000060
	str r1, [r0, #0x2C]

	ldr r1, =0x00000301
	str r1, [r0, #0x30]

	ldr r1, =0x00000066
	str r1, [r0]

	/* Configure CPLD to enable UART0 */

	mov pc, lr
