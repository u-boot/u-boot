/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __ASM_ARM_ARCH_HARDWARE_H__
#define __ASM_ARM_ARCH_HARDWARE_H__

#if defined(CONFIG_AT91RM9200)
# include <asm/arch/at91rm9200.h>
#elif defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9G20) || \
	defined(CONFIG_AT91SAM9XE)
# include <asm/arch/at91sam9260.h>
#elif defined(CONFIG_AT91SAM9261) || defined(CONFIG_AT91SAM9G10)
# include <asm/arch/at91sam9261.h>
#elif defined(CONFIG_AT91SAM9263)
# include <asm/arch/at91sam9263.h>
#elif defined(CONFIG_AT91SAM9RL)
# include <asm/arch/at91sam9rl.h>
#elif defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
# include <asm/arch/at91sam9g45.h>
#elif defined(CONFIG_AT91SAM9X5)
# include <asm/arch/at91sam9x5.h>
#elif defined(CONFIG_AT91CAP9)
# include <asm/arch/at91cap9.h>
#elif defined(CONFIG_AT91X40)
# include <asm/arch/at91x40.h>
#else
# error "Unsupported AT91 processor"
#endif

#endif /* __ASM_ARM_ARCH_HARDWARE_H__ */
