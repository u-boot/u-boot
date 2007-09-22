/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004-2007 Freescale Semiconductor, Inc.
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
#include <asm/rtc.h>

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(void)
{
	volatile scm1_t *scm1 = (scm1_t *) MMAP_SCM1;
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	volatile fbcs_t *fbcs = (fbcs_t *) MMAP_FBCS;

	scm1->mpr = 0x77777777;
	scm1->pacra = 0;
	scm1->pacrb = 0;
	scm1->pacrc = 0;
	scm1->pacrd = 0;
	scm1->pacre = 0;
	scm1->pacrf = 0;
	scm1->pacrg = 0;

	/* FlexBus */
	gpio->par_be =
	    GPIO_PAR_BE_BE3_BE3 | GPIO_PAR_BE_BE2_BE2 | GPIO_PAR_BE_BE1_BE1 |
	    GPIO_PAR_BE_BE0_BE0;
	gpio->par_fbctl =
	    GPIO_PAR_FBCTL_OE | GPIO_PAR_FBCTL_TA_TA | GPIO_PAR_FBCTL_RW_RW |
	    GPIO_PAR_FBCTL_TS_TS;

#if (defined(CFG_CS0_BASE) && defined(CFG_CS0_MASK) && defined(CFG_CS0_CTRL))
	fbcs->csar0 = CFG_CS0_BASE;
	fbcs->cscr0 = CFG_CS0_CTRL;
	fbcs->csmr0 = CFG_CS0_MASK;
#endif

#if (defined(CFG_CS1_BASE) && defined(CFG_CS1_MASK) && defined(CFG_CS1_CTRL))
	/* Latch chipselect */
	fbcs->csar1 = CFG_CS1_BASE;
	fbcs->cscr1 = CFG_CS1_CTRL;
	fbcs->csmr1 = CFG_CS1_MASK;
#endif

#if (defined(CFG_CS2_BASE) && defined(CFG_CS2_MASK) && defined(CFG_CS2_CTRL))
	fbcs->csar2 = CFG_CS2_BASE;
	fbcs->cscr2 = CFG_CS2_CTRL;
	fbcs->csmr2 = CFG_CS2_MASK;
#endif

#if (defined(CFG_CS3_BASE) && defined(CFG_CS3_MASK) && defined(CFG_CS3_CTRL))
	fbcs->csar3 = CFG_CS3_BASE;
	fbcs->cscr3 = CFG_CS3_CTRL;
	fbcs->csmr3 = CFG_CS3_MASK;
#endif

#if (defined(CFG_CS4_BASE) && defined(CFG_CS4_MASK) && defined(CFG_CS4_CTRL))
	fbcs->csar4 = CFG_CS4_BASE;
	fbcs->cscr4 = CFG_CS4_CTRL;
	fbcs->csmr4 = CFG_CS4_MASK;
#endif

#if (defined(CFG_CS5_BASE) && defined(CFG_CS5_MASK) && defined(CFG_CS5_CTRL))
	fbcs->csar5 = CFG_CS5_BASE;
	fbcs->cscr5 = CFG_CS5_CTRL;
	fbcs->csmr5 = CFG_CS5_MASK;
#endif

#ifdef CONFIG_FSL_I2C
	gpio->par_feci2c = GPIO_PAR_FECI2C_SCL_SCL | GPIO_PAR_FECI2C_SDA_SDA;
#endif

	icache_enable();
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
#ifdef CONFIG_MCFTMR
	volatile rtc_t *rtc = (volatile rtc_t *)(CFG_MCFRTC_BASE);
	volatile rtcex_t *rtcex = (volatile rtcex_t *)&rtc->extended;
	u32 oscillator = CFG_RTC_OSCILLATOR;

	rtcex->gocu = (CFG_RTC_OSCILLATOR >> 16) & 0xFFFF;
	rtcex->gocl = CFG_RTC_OSCILLATOR & 0xFFFF;
#endif

	return (0);
}

void uart_port_conf(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	/* Setup Ports: */
	switch (CFG_UART_PORT) {
	case 0:
		gpio->par_uart =
		    (GPIO_PAR_UART_U0TXD_U0TXD | GPIO_PAR_UART_U0RXD_U0RXD);
		break;
	case 1:
		gpio->par_uart =
		    (GPIO_PAR_UART_U1TXD_U1TXD | GPIO_PAR_UART_U1RXD_U1RXD);
		break;
	}
}
