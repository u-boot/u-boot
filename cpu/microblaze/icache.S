/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

	.text
	.globl	icache_enable
	.ent	icache_enable
	.align	2
icache_enable:
	/* Make space on stack for a temporary */
	addi	r1, r1, -4
	/* Save register r12 */
	swi	r12, r1, 0
	/* Read the MSR register */
	mfs	r12, rmsr
	/* Set the instruction enable bit */
	ori	r12, r12, 0x20
	/* Save the MSR register */
	mts	rmsr, r12
	/* Load register r12 */
	lwi	r12, r1, 0
	/* Return */
	rtsd	r15, 8
	/* Update stack in the delay slot */
	addi	r1, r1, 4
	.end	icache_enable

	.text
	.globl	icache_disable
	.ent	icache_disable
	.align	2
icache_disable:
	/* Make space on stack for a temporary */
	addi	r1, r1, -4
	/* Save register r12 */
	swi	r12, r1, 0
	/* Read the MSR register */
	mfs	r12, rmsr
	/* Clear the instruction enable bit */
	andi	r12, r12, ~0x20
	/* Save the MSR register */
	mts	rmsr, r12
	/* Load register r12 */
	lwi	r12, r1, 0
	/* Return */
	rtsd	r15, 8
	/* Update stack in the delay slot */
	addi	r1, r1, 4
	.end	icache_disable
