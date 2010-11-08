/*
 *  (C) Copyright 2008
 *  Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@uam.es
 *  This work has been supported by: QTechnology  http://qtec.com/
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ppc_asm.tmpl>
#include <config.h>
#include <asm/mmu.h>

.section .bootpg,"ax"
.globl tlbtab

tlbtab:
tlbtab_start
tlbentry(0x00000000, SZ_256M, 0x00000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0x10000000, SZ_256M, 0x10000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0x20000000, SZ_256M, 0x20000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0x30000000, SZ_256M, 0x30000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0x40000000, SZ_256M, 0x40000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0x50000000, SZ_256M, 0x50000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0x60000000, SZ_256M, 0x60000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0x70000000, SZ_256M, 0x70000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0x80000000, SZ_256M, 0x80000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0x90000000, SZ_256M, 0x90000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0xa0000000, SZ_256M, 0xa0000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0xb0000000, SZ_256M, 0xb0000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0xc0000000, SZ_256M, 0xc0000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0xd0000000, SZ_256M, 0xd0000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0xe0000000, SZ_256M, 0xe0000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbentry(0xf0000000, SZ_256M, 0xf0000000, 0, AC_R | AC_W | AC_X | SA_G | SA_I)
tlbtab_end
