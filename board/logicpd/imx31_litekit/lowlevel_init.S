/*
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
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

#include <asm/arch/imx-regs.h>

.macro REG reg, val
	ldr r2, =\reg
	ldr r3, =\val
	str r3, [r2]
.endm

.macro REG8 reg, val
	ldr r2, =\reg
	ldr r3, =\val
	strb r3, [r2]
.endm

.macro DELAY loops
	ldr r2, =\loops
1:
	subs	r2, r2, #1
	nop
	bcs 1b
.endm

.globl lowlevel_init
lowlevel_init:

	REG	IPU_CONF, IPU_CONF_DI_EN
	REG	CCM_CCMR, 0x074B0BF5

	DELAY 0x40000

	REG	CCM_CCMR, 0x074B0BF5 | CCMR_MPE
	REG	CCM_CCMR, (0x074B0BF5 | CCMR_MPE) & ~CCMR_MDS

	REG	CCM_PDR0, PDR0_CSI_PODF(0x3f) | PDR0_CSI_PRDF(7) | PDR0_PER_PODF(7) | PDR0_HSP_PODF(2) | PDR0_NFC_PODF(6) | PDR0_IPG_PODF(1) | PDR0_MAX_PODF(2) | PDR0_MCU_PODF(0)

	REG	CCM_MPCTL, PLL_PD(0) | PLL_MFD(0x33) | PLL_MFI(7) | PLL_MFN(0x23)
	REG	CCM_SPCTL, PLL_PD(1) | PLL_MFD(4) | PLL_MFI(12) | PLL_MFN(1)

	REG	0x43FAC26C, 0 /* SDCLK */
	REG	0x43FAC270, 0 /* CAS */
	REG	0x43FAC274, 0 /* RAS */
	REG	0x43FAC27C, 0x1000 /* CS2 (CSD0) */
	REG	0x43FAC284, 0 /* DQM3 */
	REG	0x43FAC288, 0 /* DQM2, DQM1, DQM0, SD31-SD0, A25-A0, MA10 (0x288..0x2DC) */
	REG	0x43FAC28C, 0
	REG	0x43FAC290, 0
	REG	0x43FAC294, 0
	REG	0x43FAC298, 0
	REG	0x43FAC29C, 0
	REG	0x43FAC2A0, 0
	REG	0x43FAC2A4, 0
	REG	0x43FAC2A8, 0
	REG	0x43FAC2AC, 0
	REG	0x43FAC2B0, 0
	REG	0x43FAC2B4, 0
	REG	0x43FAC2B8, 0
	REG	0x43FAC2BC, 0
	REG	0x43FAC2C0, 0
	REG	0x43FAC2C4, 0
	REG	0x43FAC2C8, 0
	REG	0x43FAC2CC, 0
	REG	0x43FAC2D0, 0
	REG	0x43FAC2D4, 0
	REG	0x43FAC2D8, 0
	REG	0x43FAC2DC, 0
	REG	0xB8001010, 0x00000004
	REG	0xB8001004, 0x006ac73a
	REG	0xB8001000, 0x92100000
	REG	0x80000f00, 0x12344321
	REG	0xB8001000, 0xa2100000
	REG	0x80000000, 0x12344321
	REG	0x80000000, 0x12344321
	REG	0xB8001000, 0xb2100000
	REG8	0x80000033, 0xda
	REG8	0x81000000, 0xff
	REG	0xB8001000, 0x82226080
	REG	0x80000000, 0xDEADBEEF
	REG	0xB8001010, 0x0000000c

	mov	pc, lr
