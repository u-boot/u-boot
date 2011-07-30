/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
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

#ifndef _U_BOOT_I386_H_
#define _U_BOOT_I386_H_	1

/* cpu/.../cpu.c */
int x86_cpu_init_r(void);
int cpu_init_r(void);
int x86_cpu_init_f(void);
int cpu_init_f(void);

/* cpu/.../timer.c */
void timer_isr(void *);
typedef void (timer_fnc_t) (void);
int register_timer_isr (timer_fnc_t *isr_func);

/* Architecture specific - can be in arch/x86/cpu/, arch/x86/lib/, or $(BOARD)/ */
int dram_init_f(void);

/* cpu/.../interrupts.c */
int cpu_init_interrupts(void);

/* board/.../... */
int board_init(void);
int dram_init(void);

void setup_pcat_compatibility(void);

void isa_unmap_rom(u32 addr);
u32 isa_map_rom(u32 bus_addr, int size);

/* arch/x86/lib/... */
int video_bios_init(void);
int video_init(void);


#endif	/* _U_BOOT_I386_H_ */
