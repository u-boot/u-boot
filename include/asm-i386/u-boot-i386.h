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

/* for the following variables, see start.S */
extern ulong i386boot_start;	    /* code start (in flash) */
extern ulong i386boot_end;	    /* code end (in flash) */
extern ulong i386boot_romdata_start;/* datasegment in flash (also code+rodata end) */
extern ulong i386boot_romdata_dest; /* data location segment in ram */
extern ulong i386boot_romdata_size; /* size of data segment */
extern ulong i386boot_bss_start;    /* bss start */
extern ulong i386boot_bss_size;     /* bss size */
extern ulong i386boot_stack_end;    /* first usable RAM address after bss and stack */
extern ulong i386boot_ram_end;      /* end of ram */

extern ulong i386boot_realmode;     /* start of realmode entry code */
extern ulong i386boot_realmode_size;/* size of realmode entry code */
extern ulong i386boot_bios;         /* start of BIOS emulation code */
extern ulong i386boot_bios_size;    /* size of BIOS emulation code */


/* cpu/.../cpu.c */
int cpu_init(void);
int timer_init(void);

/* board/.../... */
int board_init(void);
int dram_init(void);

void isa_unmap_rom(u32 addr);
u32 isa_map_rom(u32 bus_addr, int size);

/* lib_i386/... */
int video_bios_init(void);
int video_init(void);


#endif	/* _U_BOOT_I386_H_ */
