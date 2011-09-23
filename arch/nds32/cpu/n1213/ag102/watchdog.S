/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
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

#include <asm/arch-ag102/ag102.h>
#include <linux/linkage.h>

.text

#ifndef CONFIG_SKIP_TRUNOFF_WATCHDOG
ENTRY(turnoff_watchdog)

#define WD_CR		0xC
#define WD_ENABLE	0x1

	! Turn off the watchdog, according to Faraday FTWDT010 spec
	li 	$p0, (CONFIG_FTWDT010_BASE+WD_CR)	! Get the addr of WD CR
	lwi	$p1, [$p0]				! Get the config of WD
	andi	$p1, $p1, 0x1f				! Wipe out useless bits
	li	$r0, ~WD_ENABLE
	and	$p1, $p1, $r0				! Set WD disable
	sw	$p1, [$p0]				! Write back to WD CR

	! Disable Interrupts by clear GIE in $PSW reg
	setgie.d

	ret

ENDPROC(turnoff_watchdog)
#endif
