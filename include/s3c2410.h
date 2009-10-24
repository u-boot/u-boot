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
 * NAME	    : s3c2410.h
 * Version  : 31.3.2003
 *
 * Based on S3C2410X User's manual Rev 1.1
 ************************************************/

#ifndef __S3C2410_H__
#define __S3C2410_H__

#define S3C24X0_UART_CHANNELS	3
#define S3C24X0_SPI_CHANNELS	2

/* S3C2410 only supports 512 Byte HW ECC */
#define S3C2410_ECCSIZE		512
#define S3C2410_ECCBYTES	3

enum s3c24x0_uarts_nr {
	S3C24X0_UART0,
	S3C24X0_UART1,
	S3C24X0_UART2
};

/* S3C2410 device base addresses */
#define S3C24X0_MEMCTL_BASE		0x48000000
#define S3C24X0_USB_HOST_BASE		0x49000000
#define S3C24X0_INTERRUPT_BASE		0x4A000000
#define S3C24X0_DMA_BASE		0x4B000000
#define S3C24X0_CLOCK_POWER_BASE	0x4C000000
#define S3C24X0_LCD_BASE		0x4D000000
#define S3C2410_NAND_BASE		0x4E000000
#define S3C24X0_UART_BASE		0x50000000
#define S3C24X0_TIMER_BASE		0x51000000
#define S3C24X0_USB_DEVICE_BASE		0x52000140
#define S3C24X0_WATCHDOG_BASE		0x53000000
#define S3C24X0_I2C_BASE		0x54000000
#define S3C24X0_I2S_BASE		0x55000000
#define S3C24X0_GPIO_BASE		0x56000000
#define S3C24X0_RTC_BASE		0x57000000
#define S3C2410_ADC_BASE		0x58000000
#define S3C24X0_SPI_BASE		0x59000000
#define S3C2410_SDI_BASE		0x5A000000


/* include common stuff */
#include <s3c24x0.h>


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
static inline struct s3c2410_nand *s3c2410_get_base_nand(void)
{
	return (struct s3c2410_nand *)S3C2410_NAND_BASE;
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
static inline struct s3c2410_adc *s3c2410_get_base_adc(void)
{
	return (struct s3c2410_adc *)S3C2410_ADC_BASE;
}
static inline struct s3c24x0_spi *s3c24x0_get_base_spi(void)
{
	return (struct s3c24x0_spi *)S3C24X0_SPI_BASE;
}
static inline struct s3c2410_sdi *s3c2410_get_base_sdi(void)
{
	return (struct s3c2410_sdi *)S3C2410_SDI_BASE;
}

#endif /*__S3C2410_H__*/
