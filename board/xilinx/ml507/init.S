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
#include <asm-ppc/mmu.h>

.section .bootpg,"ax"
.globl tlbtab

tlbtab:
tlbtab_start
	/* SDRAM */
tlbentry(XPAR_DDR2_SDRAM_MEM_BASEADDR, SZ_256M, CFG_SDRAM_BASE, 0,
	 AC_R | AC_W | AC_X | SA_G | SA_I)
	/* UART */
tlbentry(XPAR_UARTLITE_0_BASEADDR, SZ_64K, XPAR_UARTLITE_0_BASEADDR, 0,
	 AC_R | AC_W | SA_G | SA_I)
	/* PIC */
tlbentry(XPAR_INTC_0_BASEADDR, SZ_64K, XPAR_INTC_0_BASEADDR, 0,
	 AC_R | AC_W | SA_G | SA_I)
#ifdef XPAR_IIC_EEPROM_BASEADDR
	/* I2C */
tlbentry(XPAR_IIC_EEPROM_BASEADDR, SZ_64K, XPAR_IIC_EEPROM_BASEADDR, 0,
	 AC_R | AC_W | SA_G | SA_I)
#endif
#ifdef XPAR_LLTEMAC_0_BASEADDR
	/* Net */
tlbentry(XPAR_LLTEMAC_0_BASEADDR, SZ_64K, XPAR_LLTEMAC_0_BASEADDR, 0,
	 AC_R | AC_W | SA_G | SA_I)
#endif
#ifdef XPAR_FLASH_MEM0_BASEADDR
	/*Flash*/
tlbentry(XPAR_FLASH_MEM0_BASEADDR, SZ_256M, XPAR_FLASH_MEM0_BASEADDR, 0,
	 AC_R | AC_W | AC_X | SA_G | SA_I)
#endif
tlbtab_end
