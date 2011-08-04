/*
 * (C) Copyright 2008,2009
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
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
#include <malloc.h>
#include <asm/io.h>
#include <asm/i8254.h>
#include <asm/ibmpc.h>

struct timer_isr_function {
	struct timer_isr_function *next;
	timer_fnc_t *isr_func;
};

static struct timer_isr_function *first_timer_isr = NULL;
static volatile unsigned long system_ticks = 0;

/*
 * register_timer_isr() allows multiple architecture and board specific
 * functions to be called every millisecond. Keep the execution time of
 * each function as low as possible
 */
int register_timer_isr (timer_fnc_t *isr_func)
{
	struct timer_isr_function *new_func;
	struct timer_isr_function *temp;
	int flag;

	new_func = malloc(sizeof(struct timer_isr_function));

	if (new_func == NULL)
		return 1;

	new_func->isr_func = isr_func;
	new_func->next = NULL;

	/*
	 *  Don't allow timer interrupts while the
	 *  linked list is being modified
	 */
	flag = disable_interrupts ();

	if (first_timer_isr == NULL) {
		first_timer_isr = new_func;
	} else {
		temp = first_timer_isr;
		while (temp->next != NULL)
			temp = temp->next;
		temp->next = new_func;
	}

	if (flag)
		enable_interrupts ();

	return 0;
}

/*
 * timer_isr() MUST be the registered interrupt handler for
 */
void timer_isr(void *unused)
{
	struct timer_isr_function *temp = first_timer_isr;

	system_ticks++;

	/* Execute each registered function */
	while (temp != NULL) {
		temp->isr_func ();
		temp = temp->next;
	}
}

ulong get_timer (ulong base)
{
	return (system_ticks - base);
}
