/*
 * Copyright 2010 eXMeritus, A Boeing Company
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

#include <asm/mpc85xx_gpio.h>

/* Common CPU A/B GPIOs (GPIO8-GPIO15 and IRQ4-IRQ6) */
#define GPIO_CPU_ID		(1UL << (31 -  8))
#define GPIO_BLUE_LED		(1UL << (31 -  9))
#define GPIO_DIMM_RESET		(1UL << (31 - 10))
#define GPIO_USB_RESET		(1UL << (31 - 11))
#define GPIO_UNUSED_12		(1UL << (31 - 12))
#define GPIO_GETH0_RESET	(1UL << (31 - 13))
#define GPIO_RS422_RE		(1UL << (31 - 14))
#define GPIO_RS422_DE		(1UL << (31 - 15))
#define IRQ_I2CINT		(1UL << (31 - 20))
#define IRQ_FANINT		(1UL << (31 - 21))
#define IRQ_DIMM_EVENT		(1UL << (31 - 22))

#define GPIO_RESETS (GPIO_DIMM_RESET|GPIO_USB_RESET|GPIO_GETH0_RESET)

/* CPU A GPIOS (GPIO0-GPIO7 and IRQ0-IRQ3) */
#define GPIO_CPUA_UNUSED_0	(1UL << (31 -  0))
#define GPIO_CPUA_CPU_READY	(1UL << (31 -  1))
#define GPIO_CPUA_DEBUG_LED2	(1UL << (31 -  2))
#define GPIO_CPUA_DEBUG_LED1	(1UL << (31 -  3))
#define GPIO_CPUA_TDIS2B	(1UL << (31 -  4)) /* MAC 2 TX B */
#define GPIO_CPUA_TDIS2A	(1UL << (31 -  5)) /* MAC 2 TX A */
#define GPIO_CPUA_TDIS1B	(1UL << (31 -  6)) /* MAC 1 TX B */
#define GPIO_CPUA_TDIS1A	(1UL << (31 -  7)) /* MAC 1 TX A */
#define IRQ_CPUA_UNUSED_0	(1UL << (31 - 16))
#define IRQ_CPUA_UNUSED_1	(1UL << (31 - 17))
#define IRQ_CPUA_UNUSED_2	(1UL << (31 - 18))
#define IRQ_CPUA_UNUSED_3	(1UL << (31 - 19))

/* CPU B GPIOS (GPIO0-GPIO7 and IRQ0-IRQ3) */
#define GPIO_CPUB_RMUX_SEL1B	(1UL << (31 -  0))
#define GPIO_CPUB_RMUX_SEL0B	(1UL << (31 -  1))
#define GPIO_CPUB_RMUX_SEL1A	(1UL << (31 -  2))
#define GPIO_CPUB_RMUX_SEL0A	(1UL << (31 -  3))
#define GPIO_CPUB_UNUSED_4	(1UL << (31 -  4))
#define GPIO_CPUB_CPU_READY	(1UL << (31 -  5))
#define GPIO_CPUB_DEBUG_LED2	(1UL << (31 -  6))
#define GPIO_CPUB_DEBUG_LED1	(1UL << (31 -  7))
#define IRQ_CPUB_SD_1A		(1UL << (31 - 16))
#define IRQ_CPUB_SD_2B		(1UL << (31 - 17))
#define IRQ_CPUB_SD_2A		(1UL << (31 - 18))
#define IRQ_CPUB_SD_1B		(1UL << (31 - 19))

/* If it isn't CPU A then it's CPU B */
static inline unsigned int hww1u1a_is_cpu_a(void)
{
	return !mpc85xx_gpio_get(GPIO_CPU_ID);
}
