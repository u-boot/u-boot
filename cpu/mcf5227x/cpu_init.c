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
	volatile pll_t *pll = (volatile pll_t *)MMAP_PLL;

#if !defined(CONFIG_CF_SBF)
	/* Workaround, must place before fbcs */
	pll->psr = 0x12;

	scm1->mpr = 0x77777777;
	scm1->pacra = 0;
	scm1->pacrb = 0;
	scm1->pacrc = 0;
	scm1->pacrd = 0;
	scm1->pacre = 0;
	scm1->pacrf = 0;
	scm1->pacrg = 0;
	scm1->pacri = 0;

#if (defined(CONFIG_SYS_CS0_BASE) && defined(CONFIG_SYS_CS0_MASK) \
     && defined(CONFIG_SYS_CS0_CTRL))
	fbcs->csar0 = CONFIG_SYS_CS0_BASE;
	fbcs->cscr0 = CONFIG_SYS_CS0_CTRL;
	fbcs->csmr0 = CONFIG_SYS_CS0_MASK;
#endif
#endif				/* CONFIG_CF_SBF */

#if (defined(CONFIG_SYS_CS1_BASE) && defined(CONFIG_SYS_CS1_MASK) \
     && defined(CONFIG_SYS_CS1_CTRL))
	fbcs->csar1 = CONFIG_SYS_CS1_BASE;
	fbcs->cscr1 = CONFIG_SYS_CS1_CTRL;
	fbcs->csmr1 = CONFIG_SYS_CS1_MASK;
#endif

#if (defined(CONFIG_SYS_CS2_BASE) && defined(CONFIG_SYS_CS2_MASK) \
     && defined(CONFIG_SYS_CS2_CTRL))
	fbcs->csar2 = CONFIG_SYS_CS2_BASE;
	fbcs->cscr2 = CONFIG_SYS_CS2_CTRL;
	fbcs->csmr2 = CONFIG_SYS_CS2_MASK;
#endif

#if (defined(CONFIG_SYS_CS3_BASE) && defined(CONFIG_SYS_CS3_MASK) \
     && defined(CONFIG_SYS_CS3_CTRL))
	fbcs->csar3 = CONFIG_SYS_CS3_BASE;
	fbcs->cscr3 = CONFIG_SYS_CS3_CTRL;
	fbcs->csmr3 = CONFIG_SYS_CS3_MASK;
#endif

#if (defined(CONFIG_SYS_CS4_BASE) && defined(CONFIG_SYS_CS4_MASK) \
     && defined(CONFIG_SYS_CS4_CTRL))
	fbcs->csar4 = CONFIG_SYS_CS4_BASE;
	fbcs->cscr4 = CONFIG_SYS_CS4_CTRL;
	fbcs->csmr4 = CONFIG_SYS_CS4_MASK;
#endif

#if (defined(CONFIG_SYS_CS5_BASE) && defined(CONFIG_SYS_CS5_MASK) \
     && defined(CONFIG_SYS_CS5_CTRL))
	fbcs->csar5 = CONFIG_SYS_CS5_BASE;
	fbcs->cscr5 = CONFIG_SYS_CS5_CTRL;
	fbcs->csmr5 = CONFIG_SYS_CS5_MASK;
#endif

#ifdef CONFIG_FSL_I2C
	gpio->par_i2c = GPIO_PAR_I2C_SCL_SCL | GPIO_PAR_I2C_SDA_SDA;
#endif

	icache_enable();
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
#ifdef CONFIG_MCFRTC
	volatile rtc_t *rtc = (volatile rtc_t *)(CONFIG_SYS_MCFRTC_BASE);
	volatile rtcex_t *rtcex = (volatile rtcex_t *)&rtc->extended;
	u32 oscillator = CONFIG_SYS_RTC_OSCILLATOR;

	rtcex->gocu = (CONFIG_SYS_RTC_OSCILLATOR >> 16) & 0xFFFF;
	rtcex->gocl = CONFIG_SYS_RTC_OSCILLATOR & 0xFFFF;
#endif

	return (0);
}

void uart_port_conf(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	/* Setup Ports: */
	switch (CONFIG_SYS_UART_PORT) {
	case 0:
		gpio->par_uart &=
		    (GPIO_PAR_UART_U0TXD_MASK & GPIO_PAR_UART_U0RXD_MASK);
		gpio->par_uart |=
		    (GPIO_PAR_UART_U0TXD_U0TXD | GPIO_PAR_UART_U0RXD_U0RXD);
		break;
	case 1:
		gpio->par_uart &=
		    (GPIO_PAR_UART_U1TXD_MASK & GPIO_PAR_UART_U1RXD_MASK);
		gpio->par_uart |=
		    (GPIO_PAR_UART_U1TXD_U1TXD | GPIO_PAR_UART_U1RXD_U1RXD);
		break;
	case 2:
		gpio->par_dspi &=
		    (GPIO_PAR_DSPI_SIN_MASK & GPIO_PAR_DSPI_SOUT_MASK);
		gpio->par_dspi =
		    (GPIO_PAR_DSPI_SIN_U2RXD | GPIO_PAR_DSPI_SOUT_U2TXD);
		break;
	}
}

#ifdef CONFIG_CF_DSPI
void cfspi_port_conf(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	gpio->par_dspi =
	    GPIO_PAR_DSPI_SIN_SIN | GPIO_PAR_DSPI_SOUT_SOUT |
	    GPIO_PAR_DSPI_SCK_SCK;
}

int cfspi_claim_bus(uint bus, uint cs)
{
	volatile dspi_t *dspi = (dspi_t *) MMAP_DSPI;
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	if ((dspi->sr & DSPI_SR_TXRXS) != DSPI_SR_TXRXS)
		return -1;

	/* Clear FIFO and resume transfer */
	dspi->mcr &= ~(DSPI_MCR_CTXF | DSPI_MCR_CRXF);

	switch (cs) {
	case 0:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS0_MASK;
		gpio->par_dspi |= GPIO_PAR_DSPI_PCS0_PCS0;
		break;
	case 2:
		gpio->par_timer &= GPIO_PAR_TIMER_T2IN_MASK;
		gpio->par_timer |= GPIO_PAR_TIMER_T2IN_DSPIPCS2;
		break;
	}

	return 0;
}

void cfspi_release_bus(uint bus, uint cs)
{
	volatile dspi_t *dspi = (dspi_t *) MMAP_DSPI;
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	dspi->mcr &= ~(DSPI_MCR_CTXF | DSPI_MCR_CRXF);	/* Clear FIFO */

	switch (cs) {
	case 0:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS0_PCS0;
		break;
	case 2:
		gpio->par_timer &= GPIO_PAR_TIMER_T2IN_MASK;
		break;
	}
}
#endif
