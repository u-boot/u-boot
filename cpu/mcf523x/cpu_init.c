/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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
#include <watchdog.h>

#include <asm/immap.h>

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	volatile fbcs_t *fbcs = (fbcs_t *) MMAP_FBCS;
	volatile wdog_t *wdog = (wdog_t *) MMAP_WDOG;
	volatile scm_t *scm = (scm_t *) MMAP_SCM;

	/* watchdog is enabled by default - disable the watchdog */
#ifndef CONFIG_WATCHDOG
	wdog->cr = 0;
#endif

	scm->rambar = (CFG_INIT_RAM_ADDR | SCM_RAMBAR_BDE);

	/* Port configuration */
	gpio->par_cs = 0;

#if (defined(CFG_CS0_BASE) && defined(CFG_CS0_MASK) && defined(CFG_CS0_CTRL))
	fbcs->csar0 = CFG_CS0_BASE;
	fbcs->cscr0 = CFG_CS0_CTRL;
	fbcs->csmr0 = CFG_CS0_MASK;
#endif

#if (defined(CFG_CS1_BASE) && defined(CFG_CS1_MASK) && defined(CFG_CS1_CTRL))
	gpio->par_cs |= GPIO_PAR_CS_CS1;
	fbcs->csar1 = CFG_CS1_BASE;
	fbcs->cscr1 = CFG_CS1_CTRL;
	fbcs->csmr1 = CFG_CS1_MASK;
#endif

#if (defined(CFG_CS2_BASE) && defined(CFG_CS2_MASK) && defined(CFG_CS2_CTRL))
	gpio->par_cs |= GPIO_PAR_CS_CS2;
	fbcs->csar2 = CFG_CS2_BASE;
	fbcs->cscr2 = CFG_CS2_CTRL;
	fbcs->csmr2 = CFG_CS2_MASK;
#endif

#if (defined(CFG_CS3_BASE) && defined(CFG_CS3_MASK) && defined(CFG_CS3_CTRL))
	gpio->par_cs |= GPIO_PAR_CS_CS3;
	fbcs->csar3 = CFG_CS3_BASE;
	fbcs->cscr3 = CFG_CS3_CTRL;
	fbcs->csmr3 = CFG_CS3_MASK;
#endif

#if (defined(CFG_CS4_BASE) && defined(CFG_CS4_MASK) && defined(CFG_CS4_CTRL))
	gpio->par_cs |= GPIO_PAR_CS_CS4;
	fbcs->csar4 = CFG_CS4_BASE;
	fbcs->cscr4 = CFG_CS4_CTRL;
	fbcs->csmr4 = CFG_CS4_MASK;
#endif

#if (defined(CFG_CS5_BASE) && defined(CFG_CS5_MASK) && defined(CFG_CS5_CTRL))
	gpio->par_cs |= GPIO_PAR_CS_CS5;
	fbcs->csar5 = CFG_CS5_BASE;
	fbcs->cscr5 = CFG_CS5_CTRL;
	fbcs->csmr5 = CFG_CS5_MASK;
#endif

#if (defined(CFG_CS6_BASE) && defined(CFG_CS6_MASK) && defined(CFG_CS6_CTRL))
	gpio->par_cs |= GPIO_PAR_CS_CS6;
	fbcs->csar6 = CFG_CS6_BASE;
	fbcs->cscr6 = CFG_CS6_CTRL;
	fbcs->csmr6 = CFG_CS6_MASK;
#endif

#if (defined(CFG_CS7_BASE) && defined(CFG_CS7_MASK) && defined(CFG_CS7_CTRL))
	gpio->par_cs |= GPIO_PAR_CS_CS7;
	fbcs->csar7 = CFG_CS7_BASE;
	fbcs->cscr7 = CFG_CS7_CTRL;
	fbcs->csmr7 = CFG_CS7_MASK;
#endif

#ifdef CONFIG_FSL_I2C
	gpio->par_feci2c &= ~(GPIO_PAR_FECI2C_SCL_MASK | GPIO_PAR_FECI2C_SDA_MASK);
	gpio->par_feci2c |= (GPIO_PAR_FECI2C_SCL_I2CSCL | GPIO_PAR_FECI2C_SDA_I2CSDA);
#endif

	icache_enable();
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
	return (0);
}

void uart_port_conf(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	/* Setup Ports: */
	switch (CFG_UART_PORT) {
	case 0:
		gpio->par_uart = (GPIO_PAR_UART_U0RXD | GPIO_PAR_UART_U0TXD);
		break;
	case 1:
		gpio->par_uart =
			(GPIO_PAR_UART_U1RXD_U1RXD | GPIO_PAR_UART_U1TXD_U1TXD);
		break;
	case 2:
		gpio->par_timer = (GPIO_PAR_UART_U2RXD | GPIO_PAR_UART_U2TXD);
		break;
	}
}
