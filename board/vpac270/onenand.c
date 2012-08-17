/*
 * Voipac PXA270 OneNAND SPL
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
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

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <onenand_uboot.h>
#include <asm/arch/pxa.h>

void board_init_f(unsigned long unused)
{
	extern uint32_t _end;
	uint32_t tmp;

	asm volatile("mov %0, pc" : "=r"(tmp));
	tmp >>= 24;

	/* The code runs from OneNAND RAM, copy SPL to SRAM and execute it. */
	if (tmp == 0) {
		tmp = (uint32_t)&_end - CONFIG_SPL_TEXT_BASE;
		onenand_spl_load_image(0, tmp, (void *)CONFIG_SPL_TEXT_BASE);
		asm volatile("mov pc, %0" : : "r"(CONFIG_SPL_TEXT_BASE));
	}

	/* Hereby, the code runs from (S)RAM, copy U-Boot and execute it. */
	arch_cpu_init();
	pxa2xx_dram_init();
	onenand_spl_load_image(CONFIG_SPL_ONENAND_LOAD_ADDR,
				CONFIG_SPL_ONENAND_LOAD_SIZE,
				(void *)CONFIG_SYS_TEXT_BASE);
	asm volatile("mov pc, %0" : : "r"(CONFIG_SYS_TEXT_BASE));

	for (;;)
		;
}

void __attribute__((noreturn)) hang(void)
{
	for (;;)
		;
}
