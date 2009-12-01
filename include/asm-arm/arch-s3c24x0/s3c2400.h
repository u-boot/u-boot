/*
 * (C) Copyright 2003
 * David Müller ELSOFT AG Switzerland. d.mueller@elsoft.ch
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

/************************************************
 * NAME	    : s3c2400.h
 * Version  : 31.3.2003
 *
 * Based on S3C2400X User's manual Rev 1.1
 ************************************************/

#ifndef __S3C2400_H__
#define __S3C2400_H__

#define S3C24X0_UART_CHANNELS	2
#define S3C24X0_SPI_CHANNELS	1
#define PALETTE			(0x14A00400)	/* SJS */

enum s3c24x0_uarts_nr {
	S3C24X0_UART0,
	S3C24X0_UART1,
};

/*S3C2400 device base addresses */
#define S3C24X0_MEMCTL_BASE		0x14000000
#define S3C24X0_USB_HOST_BASE		0x14200000
#define S3C24X0_INTERRUPT_BASE		0x14400000
#define S3C24X0_DMA_BASE		0x14600000
#define S3C24X0_CLOCK_POWER_BASE	0x14800000
#define S3C24X0_LCD_BASE		0x14A00000
#define S3C24X0_UART_BASE		0x15000000
#define S3C24X0_TIMER_BASE		0x15100000
#define S3C24X0_USB_DEVICE_BASE		0x15200140
#define S3C24X0_WATCHDOG_BASE		0x15300000
#define S3C24X0_I2C_BASE		0x15400000
#define S3C24X0_I2S_BASE		0x15508000
#define S3C24X0_GPIO_BASE		0x15600000
#define S3C24X0_RTC_BASE		0x15700000
#define S3C24X0_ADC_BASE		0x15800000
#define S3C24X0_SPI_BASE		0x15900000
#define S3C2400_MMC_BASE		0x15A00000

/* include common stuff */
#include <asm/arch/s3c24x0.h>


static inline struct s3c24x0_memctl *s3c24x0_get_base_memctl(void)
{
	return (struct s3c24x0_memctl *)S3C24X0_MEMCTL_BASE;
}

static inline struct s3c24x0_usb_host *s3c24x0_get_base_usb_host(void)
{
	return (struct s3c24x0_usb_host *)S3C24X0_USB_HOST_BASE;
}

static inline struct s3c24x0_interrupt *s3c24x0_get_base_interrupt(void)
{
	return (struct s3c24x0_interrupt *)S3C24X0_INTERRUPT_BASE;
}

static inline struct s3c24x0_dmas *s3c24x0_get_base_dmas(void)
{
	return (struct s3c24x0_dmas *)S3C24X0_DMA_BASE;
}

static inline struct s3c24x0_clock_power *s3c24x0_get_base_clock_power(void)
{
	return (struct s3c24x0_clock_power *)S3C24X0_CLOCK_POWER_BASE;
}

static inline struct s3c24x0_lcd *s3c24x0_get_base_lcd(void)
{
	return (struct s3c24x0_lcd *)S3C24X0_LCD_BASE;
}

static inline struct s3c24x0_uart
	*s3c24x0_get_base_uart(enum s3c24x0_uarts_nr n)
{
	return (struct s3c24x0_uart *)(S3C24X0_UART_BASE + (n * 0x4000));
}

static inline struct s3c24x0_timers *s3c24x0_get_base_timers(void)
{
	return (struct s3c24x0_timers *)S3C24X0_TIMER_BASE;
}

static inline struct s3c24x0_usb_device *s3c24x0_get_base_usb_device(void)
{
	return (struct s3c24x0_usb_device *)S3C24X0_USB_DEVICE_BASE;
}

static inline struct s3c24x0_watchdog *s3c24x0_get_base_watchdog(void)
{
	return (struct s3c24x0_watchdog *)S3C24X0_WATCHDOG_BASE;
}

static inline struct s3c24x0_i2c *s3c24x0_get_base_i2c(void)
{
	return (struct s3c24x0_i2c *)S3C24X0_I2C_BASE;
}

static inline struct s3c24x0_i2s *s3c24x0_get_base_i2s(void)
{
	return (struct s3c24x0_i2s *)S3C24X0_I2S_BASE;
}

static inline struct s3c24x0_gpio *s3c24x0_get_base_gpio(void)
{
	return (struct s3c24x0_gpio *)S3C24X0_GPIO_BASE;
}

static inline struct s3c24x0_rtc *s3c24x0_get_base_rtc(void)
{
	return (struct s3c24x0_rtc *)S3C24X0_RTC_BASE;
}

static inline struct s3c2400_adc *s3c2400_get_base_adc(void)
{
	return (struct s3c2400_adc *)S3C24X0_ADC_BASE;
}

static inline struct s3c24x0_spi *s3c24x0_get_base_spi(void)
{
	return (struct s3c24x0_spi *)S3C24X0_SPI_BASE;
}

static inline struct s3c2400_mmc *s3c2400_get_base_mmc(void)
{
	return (struct s3c2400_mmc *)S3C2400_MMC_BASE;
}

#endif /*__S3C2400_H__*/
