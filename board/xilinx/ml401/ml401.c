/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* This is a board specific file.  It's OK to include board specific
 * header files */

#include <common.h>
#include <config.h>
#include <asm/microblaze_intc.h>
#include <asm/asm.h>

void do_reset (void)
{
#ifdef CFG_GPIO_0
	*((unsigned long *)(CFG_GPIO_0_ADDR)) =
	    ++(*((unsigned long *)(CFG_GPIO_0_ADDR)));
#endif
#ifdef CFG_RESET_ADDRESS
	puts ("Reseting board\n");
	asm ("bra r0");
#endif
}

int gpio_init (void)
{
#ifdef CFG_GPIO_0
	*((unsigned long *)(CFG_GPIO_0_ADDR)) = 0xFFFFFFFF;
#endif
	return 0;
}

#ifdef CFG_FSL_2
void fsl_isr2 (void *arg) {
	volatile int num;
	*((unsigned int *)(CFG_GPIO_0_ADDR + 0x4)) =
	    ++(*((unsigned int *)(CFG_GPIO_0_ADDR + 0x4)));
	GET (num, 2);
	NGET (num, 2);
	puts("*");
}

void fsl_init2 (void) {
	puts("fsl_init2\n");
	install_interrupt_handler (FSL_INTR_2,\
 fsl_isr2,\
 NULL);
}
#endif
