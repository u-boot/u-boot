/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _ASM_ARCH_GPIO_H
#define _ASM_ARCH_GPIO_H

#include <asm/types.h>
#include <asm/arch/armada100.h>

#define GPIO_HIGH		1
#define GPIO_LOW		0

#define GPIO_TO_REG(gp)		(gp >> 5)
#define GPIO_TO_BIT(gp)		(1 << (gp & 0x1F))
#define GPIO_VAL(gp, val)	((val >> (gp & 0x1F)) & 0x01)

static inline void *get_gpio_base(int bank)
{
	const unsigned int offset[4] = {0, 4, 8, 0x100};
	/* gpio register bank offset - refer Appendix A.36 */
	return (struct gpio_reg *)(ARMD1_GPIO_BASE + offset[bank]);
}

#endif /* _ASM_ARCH_GPIO_H */
