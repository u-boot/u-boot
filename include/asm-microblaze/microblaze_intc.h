/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.cz>
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

typedef volatile struct microblaze_intc_t {
	int isr; /* interrupt status register */
	int ipr; /* interrupt pending register */
	int ier; /* interrupt enable register */
	int iar; /* interrupt acknowledge register */
	int sie; /* set interrupt enable bits */
	int cie; /* clear interrupt enable bits */
	int ivr; /* interrupt vector register */
	int mer; /* master enable register */
} microblaze_intc_t;

struct irq_action {
	interrupt_handler_t *handler; /* pointer to interrupt rutine */
	void *arg;
	int count; /* number of interrupt */
};

void install_interrupt_handler (int irq, interrupt_handler_t * hdlr,
				       void *arg);
