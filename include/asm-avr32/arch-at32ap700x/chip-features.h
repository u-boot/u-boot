/*
 * Copyright (C) 2007 Atmel Corporation
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
#ifndef __ASM_AVR32_ARCH_CHIP_FEATURES_H__
#define __ASM_AVR32_ARCH_CHIP_FEATURES_H__

/* Currently, all the AP700x chips have these */
#define AT32AP700x_CHIP_HAS_USART
#define AT32AP700x_CHIP_HAS_MMCI

/* Only AP7000 has ethernet interface */
#ifdef CONFIG_AT32AP7000
#define AT32AP700x_CHIP_HAS_MACB
#endif

#endif /* __ASM_AVR32_ARCH_CHIP_FEATURES_H__ */
