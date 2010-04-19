/*
 * linux/include/asm-arm/arch-at91/hardware.h
 *
 *  Copyright (C) 2003 SAN People
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <asm/sizes.h>

#ifndef __ASSEMBLY__
#include "AT91RM9200.h"
#endif

/* Virtual and Physical base address for system peripherals */
#define AT91_SYS_BASE		0xFFFFF000 /*4K */

/* Virtual and Physical base addresses of user peripherals */
#define AT91_SPI_BASE		0xFFFE0000 /*16K */
#define AT91_SSC2_BASE		0xFFFD8000 /*16K */
#define AT91_SSC1_BASE		0xFFFD4000 /*16K */
#define AT91_SSC0_BASE		0xFFFD0000 /*16K */
#define AT91_USART3_BASE	0xFFFCC000 /*16K */
#define AT91_USART2_BASE	0xFFFC8000 /*16K */
#define AT91_USART1_BASE	0xFFFC4000 /*16K */
#define AT91_USART0_BASE	0xFFFC0000 /*16K */
#define AT91_EMAC_BASE		0xFFFBC000 /*16K */
#define AT91_TWI_BASE		0xFFFB8000 /*16K */
#define AT91_MCI_BASE		0xFFFB4000 /*16K */
#define AT91_UDP_BASE		0xFFFB0000 /*16K */
#define AT91_TCB1_BASE		0xFFFA4000 /*16K */
#define AT91_TCB0_BASE		0xFFFA0000 /*16K */

#define AT91_USB_HOST_BASE	0x00300000

/*
 * Where in virtual memory the IO devices (timers, system controllers
 * and so on)
 */
#define AT91_IO_BASE		0xF0000000	/* Virt/Phys Address of IO */

/* FLASH */
#define AT91_FLASH_BASE		0x10000000	/* NCS0 */

/* SDRAM */
#define AT91_SDRAM_BASE		0x20000000	/* NCS1 */

/* SmartMedia */
#define AT91_SMARTMEDIA_BASE	0x40000000	/* NCS3 */

/* Definition of interrupt priority levels */
#define AT91C_AIC_PRIOR_0 AT91C_AIC_PRIOR_LOWEST
#define AT91C_AIC_PRIOR_1 ((unsigned int) 0x1)
#define AT91C_AIC_PRIOR_2 ((unsigned int) 0x2)
#define AT91C_AIC_PRIOR_3 ((unsigned int) 0x3)
#define AT91C_AIC_PRIOR_4 ((unsigned int) 0x4)
#define AT91C_AIC_PRIOR_5 ((unsigned int) 0x5)
#define AT91C_AIC_PRIOR_6 ((unsigned int) 0x6)
#define AT91C_AIC_PRIOR_7 AT91C_AIC_PRIOR_HIGEST

#endif
