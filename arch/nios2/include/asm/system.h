/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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
#ifndef __ASM_NIOS2_SYSTEM_H_
#define __ASM_NIOS2_SYSTEM_H_

#define local_irq_enable() __asm__ __volatile__ (  \
	"rdctl	r8, status\n"			   \
	"ori	r8, r8, 1\n"			   \
	"wrctl	status, r8\n"			   \
	: : : "r8")

#define local_irq_disable() __asm__ __volatile__ ( \
	"rdctl	r8, status\n"			   \
	"andi	r8, r8, 0xfffe\n"		   \
	"wrctl	status, r8\n"			   \
	: : : "r8")

#define local_save_flags(x) __asm__ __volatile__ (	\
	"rdctl	r8, status\n"				\
	"mov	%0, r8\n"				\
	: "=r" (x) : : "r8", "memory")

#define local_irq_restore(x) __asm__ __volatile__ (	\
	"mov	r8, %0\n"				\
	"wrctl	status, r8\n"				\
	: : "r" (x) : "r8", "memory")

/* For spinlocks etc */
#define local_irq_save(x) do { local_save_flags(x); local_irq_disable(); } \
	while (0)

#define	irqs_disabled()					\
({							\
	unsigned long flags;				\
	local_save_flags(flags);			\
	((flags & NIOS2_STATUS_PIE_MSK) == 0x0);	\
})

#endif /* __ASM_NIOS2_SYSTEM_H */
