/*
 * Copyright (C) 2008 Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
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

#ifndef __MACRO_H__
#define __MACRO_H__
#ifdef __ASSEMBLY__

.macro	write32, addr, data
	mov.l \addr ,r1
	mov.l \data ,r0
	mov.l r0, @r1
.endm

.macro	write16, addr, data
	mov.l \addr ,r1
	mov.w \data ,r0
	mov.w r0, @r1
.endm

.macro	write8, addr, data
	mov.l \addr ,r1
	mov.l \data ,r0
	mov.b r0, @r1
.endm

.macro	wait_timer, time
	mov.l	\time ,r3
1:
	nop
	tst	r3, r3
	bf/s	1b
	dt	r3
.endm

#endif /* __ASSEMBLY__ */
#endif /* __MACRO_H__ */
