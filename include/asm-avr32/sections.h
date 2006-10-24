/*
 * Copyright (C) 2006 Atmel Corporation
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
#ifndef __ASM_AVR32_SECTIONS_H
#define __ASM_AVR32_SECTIONS_H

/* References to section boundaries */

extern char _text[], _etext[];
extern char __flashprog_start[], __flashprog_end[];
extern char _data[], __data_lma[], _edata[], __edata_lma[];
extern char __got_start[], __got_lma[], __got_end[];
extern char _end[];

/*
 * Everything in .flashprog will be locked in the icache so it doesn't
 * get disturbed when executing flash commands.
 */
#define __flashprog __attribute__((section(".flashprog"), __noinline__))

#endif /* __ASM_AVR32_SECTIONS_H */
