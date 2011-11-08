/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
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

#ifndef __ASM_REALMODE_H_
#define __ASM_REALMODE_H_
#include <asm/ptrace.h>

extern ulong __realmode_start;
extern ulong __realmode_size;
extern char realmode_enter;

int bios_setup(void);
int enter_realmode(u16 seg, u16 off, struct pt_regs *in, struct pt_regs *out);
int enter_realmode_int(u8 lvl, struct pt_regs *in, struct pt_regs *out);

#endif
