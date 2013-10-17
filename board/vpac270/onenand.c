/*
 * Voipac PXA270 OneNAND SPL
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
