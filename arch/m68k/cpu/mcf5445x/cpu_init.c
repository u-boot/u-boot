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
#include <asm/processor.h>
#include <asm/rtc.h>

#if defined(CONFIG_CMD_NET)
#include <config.h>
#include <net.h>
#include <asm/fec.h>
#endif

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

#if !defined(CONFIG_CF_SBF)
#if (defined(CONFIG_SYS_CS0_BASE) && defined(CONFIG_SYS_CS0_MASK) && defined(CONFIG_SYS_CS0_CTRL))
	fbcs->csar0 = CONFIG_SYS_CS0_BASE;
	fbcs->cscr0 = CONFIG_SYS_CS0_CTRL;
	fbcs->csmr0 = CONFIG_SYS_CS0_MASK;
#endif
#endif

#if (defined(CONFIG_SYS_CS1_BASE) && defined(CONFIG_SYS_CS1_MASK) && defined(CONFIG_SYS_CS1_CTRL))
	/* Latch chipselect */
	fbcs->csar1 = CONFIG_SYS_CS1_BASE;
	fbcs->cscr1 = CONFIG_SYS_CS1_CTRL;
	fbcs->csmr1 = CONFIG_SYS_CS1_MASK;
#endif

#if (defined(CONFIG_SYS_CS2_BASE) && defined(CONFIG_SYS_CS2_MASK) && defined(CONFIG_SYS_CS2_CTRL))
	fbcs->csar2 = CONFIG_SYS_CS2_BASE;
	fbcs->cscr2 = CONFIG_SYS_CS2_CTRL;
	fbcs->csmr2 = CONFIG_SYS_CS2_MASK;
#endif

#if (defined(CONFIG_SYS_CS3_BASE) && defined(CONFIG_SYS_CS3_MASK) && defined(CONFIG_SYS_CS3_CTRL))
	fbcs->csar3 = CONFIG_SYS_CS3_BASE;
	fbcs->cscr3 = CONFIG_SYS_CS3_CTRL;
	fbcs->csmr3 = CONFIG_SYS_CS3_MASK;
#endif

#if (defined(CONFIG_SYS_CS4_BASE) && defined(CONFIG_SYS_CS4_MASK) && defined(CONFIG_SYS_CS4_CTRL))
	fbcs->csar4 = CONFIG_SYS_CS4_BASE;
	fbcs->cscr4 = CONFIG_SYS_CS4_CTRL;
	fbcs->csmr4 = CONFIG_SYS_CS4_MASK;
#endif

#if (defined(CONFIG_SYS_CS5_BASE) && defined(CONFIG_SYS_CS5_MASK) && defined(CONFIG_SYS_CS5_CTRL))
	fbcs->csar5 = CONFIG_SYS_CS5_BASE;
	fbcs->cscr5 = CONFIG_SYS_CS5_CTRL;
	fbcs->csmr5 = CONFIG_SYS_CS5_MASK;
#endif

	/*
	 * now the flash base address is no longer at 0 (Newer ColdFire family
	 * boot at address 0 instead of 0xFFnn_nnnn). The vector table must
	 * also move to the new location.
	 */
	if (CONFIG_SYS_CS0_BASE != 0)
		setvbr(CONFIG_SYS_CS0_BASE);

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
#ifdef CONFIG_MCFRTC
	volatile rtc_t *rtc = (volatile rtc_t *)(CONFIG_SYS_MCFRTC_BASE);
	volatile rtcex_t *rtcex = (volatile rtcex_t *)&rtc->extended;

	rtcex->gocu = (CONFIG_SYS_RTC_OSCILLATOR >> 16) & 0xFFFF;
	rtcex->gocl = CONFIG_SYS_RTC_OSCILLATOR & 0xFFFF;
#endif

	return (0);
}

void uart_port_conf(int port)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	/* Setup Ports: */
	switch (port) {
	case 0:
		gpio->par_uart &=
		    ~(GPIO_PAR_UART_U0TXD_U0TXD | GPIO_PAR_UART_U0RXD_U0RXD);
		gpio->par_uart |=
		    (GPIO_PAR_UART_U0TXD_U0TXD | GPIO_PAR_UART_U0RXD_U0RXD);
		break;
	case 1:
#ifdef CONFIG_SYS_UART1_PRI_GPIO
		gpio->par_uart &=
		    ~(GPIO_PAR_UART_U1TXD_U1TXD | GPIO_PAR_UART_U1RXD_U1RXD);
		gpio->par_uart |=
		    (GPIO_PAR_UART_U1TXD_U1TXD | GPIO_PAR_UART_U1RXD_U1RXD);
#elif defined(CONFIG_SYS_UART1_ALT1_GPIO)
		gpio->par_ssi &=
		    (GPIO_PAR_SSI_SRXD_UNMASK | GPIO_PAR_SSI_STXD_UNMASK);
		gpio->par_ssi |=
		    (GPIO_PAR_SSI_SRXD_U1RXD | GPIO_PAR_SSI_STXD_U1TXD);
#endif
		break;
	case 2:
#if defined(CONFIG_SYS_UART2_ALT1_GPIO)
		gpio->par_timer &=
		    (GPIO_PAR_TIMER_T3IN_UNMASK | GPIO_PAR_TIMER_T2IN_UNMASK);
		gpio->par_timer |=
		    (GPIO_PAR_TIMER_T3IN_U2RXD | GPIO_PAR_TIMER_T2IN_U2TXD);
#elif defined(CONFIG_SYS_UART2_ALT2_GPIO)
		gpio->par_timer &=
		    (GPIO_PAR_FECI2C_SCL_UNMASK | GPIO_PAR_FECI2C_SDA_UNMASK);
		gpio->par_timer |=
		    (GPIO_PAR_FECI2C_SCL_U2TXD | GPIO_PAR_FECI2C_SDA_U2RXD);
#endif
		break;
	}
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(struct eth_device *dev, int setclear)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	struct fec_info_s *info = (struct fec_info_s *)dev->priv;

	if (setclear) {
#ifdef CONFIG_SYS_FEC_NO_SHARED_PHY
		if (info->iobase == CONFIG_SYS_FEC0_IOBASE)
			gpio->par_feci2c |=
			    (GPIO_PAR_FECI2C_MDC0_MDC0 |
			     GPIO_PAR_FECI2C_MDIO0_MDIO0);
		else
			gpio->par_feci2c |=
			    (GPIO_PAR_FECI2C_MDC1_MDC1 |
			     GPIO_PAR_FECI2C_MDIO1_MDIO1);
#else
		gpio->par_feci2c |=
		    (GPIO_PAR_FECI2C_MDC0_MDC0 | GPIO_PAR_FECI2C_MDIO0_MDIO0);
#endif

		if (info->iobase == CONFIG_SYS_FEC0_IOBASE)
			gpio->par_fec |= GPIO_PAR_FEC_FEC0_RMII_GPIO;
		else
			gpio->par_fec |= GPIO_PAR_FEC_FEC1_RMII_ATA;
	} else {
		gpio->par_feci2c &=
		    ~(GPIO_PAR_FECI2C_MDC0_MDC0 | GPIO_PAR_FECI2C_MDIO0_MDIO0);

		if (info->iobase == CONFIG_SYS_FEC0_IOBASE) {
#ifdef CONFIG_SYS_FEC_FULL_MII
			gpio->par_fec |= GPIO_PAR_FEC_FEC0_MII;
#else
			gpio->par_fec &= GPIO_PAR_FEC_FEC0_UNMASK;
#endif
		} else {
#ifdef CONFIG_SYS_FEC_FULL_MII
			gpio->par_fec |= GPIO_PAR_FEC_FEC1_MII;
#else
			gpio->par_fec &= GPIO_PAR_FEC_FEC1_UNMASK;
#endif
		}
	}
	return 0;
}
#endif

#ifdef CONFIG_CF_DSPI
void cfspi_port_conf(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	gpio->par_dspi = GPIO_PAR_DSPI_SIN_SIN | GPIO_PAR_DSPI_SOUT_SOUT |
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
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS0_PCS0;
		gpio->par_dspi |= GPIO_PAR_DSPI_PCS0_PCS0;
		break;
	case 1:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS1_PCS1;
		gpio->par_dspi |= GPIO_PAR_DSPI_PCS1_PCS1;
		break;
	case 2:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS2_PCS2;
		gpio->par_dspi |= GPIO_PAR_DSPI_PCS2_PCS2;
		break;
	case 3:
		gpio->par_dma &= GPIO_PAR_DMA_DACK0_UNMASK;
		gpio->par_dma |= GPIO_PAR_DMA_DACK0_PCS3;
		break;
	case 5:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS5_PCS5;
		gpio->par_dspi |= GPIO_PAR_DSPI_PCS5_PCS5;
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
	case 1:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS1_PCS1;
		break;
	case 2:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS2_PCS2;
		break;
	case 3:
		gpio->par_dma &= GPIO_PAR_DMA_DACK0_UNMASK;
		break;
	case 5:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS5_PCS5;
		break;
	}
}
#endif
