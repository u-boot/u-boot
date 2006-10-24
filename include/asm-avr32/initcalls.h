/*
 * Copyright (C) 2005, 2006 Atmel Corporation
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
#ifndef __ASM_AVR32_INITCALLS_H__
#define __ASM_AVR32_INITCALLS_H__

#include <config.h>

extern int cpu_init(void);
extern int timer_init(void);
extern void board_init_memories(void);
extern void board_init_pio(void);
extern void board_init_info(void);

#endif /* __ASM_AVR32_INITCALLS_H__ */
