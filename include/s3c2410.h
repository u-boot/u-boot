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

typedef enum {
	S3C24X0_UART0,
	S3C24X0_UART1,
	S3C24X0_UART2
} S3C24X0_UARTS_NR;

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


static inline S3C24X0_MEMCTL * S3C24X0_GetBase_MEMCTL(void)
{
	return (S3C24X0_MEMCTL * const)S3C24X0_MEMCTL_BASE;
}
static inline S3C24X0_USB_HOST * S3C24X0_GetBase_USB_HOST(void)
{
	return (S3C24X0_USB_HOST * const)S3C24X0_USB_HOST_BASE;
}
static inline S3C24X0_INTERRUPT * S3C24X0_GetBase_INTERRUPT(void)
{
	return (S3C24X0_INTERRUPT * const)S3C24X0_INTERRUPT_BASE;
}
static inline S3C24X0_DMAS * S3C24X0_GetBase_DMAS(void)
{
	return (S3C24X0_DMAS * const)S3C24X0_DMA_BASE;
}
static inline S3C24X0_CLOCK_POWER * S3C24X0_GetBase_CLOCK_POWER(void)
{
	return (S3C24X0_CLOCK_POWER * const)S3C24X0_CLOCK_POWER_BASE;
}
static inline S3C24X0_LCD * S3C24X0_GetBase_LCD(void)
{
	return (S3C24X0_LCD * const)S3C24X0_LCD_BASE;
}
static inline S3C2410_NAND * S3C2410_GetBase_NAND(void)
{
	return (S3C2410_NAND * const)S3C2410_NAND_BASE;
}
static inline S3C24X0_UART * S3C24X0_GetBase_UART(S3C24X0_UARTS_NR nr)
{
	return (S3C24X0_UART * const)(S3C24X0_UART_BASE + (nr * 0x4000));
}
static inline S3C24X0_TIMERS * S3C24X0_GetBase_TIMERS(void)
{
	return (S3C24X0_TIMERS * const)S3C24X0_TIMER_BASE;
}
static inline S3C24X0_USB_DEVICE * S3C24X0_GetBase_USB_DEVICE(void)
{
	return (S3C24X0_USB_DEVICE * const)S3C24X0_USB_DEVICE_BASE;
}
static inline S3C24X0_WATCHDOG * S3C24X0_GetBase_WATCHDOG(void)
{
	return (S3C24X0_WATCHDOG * const)S3C24X0_WATCHDOG_BASE;
}
static inline S3C24X0_I2C * S3C24X0_GetBase_I2C(void)
{
	return (S3C24X0_I2C * const)S3C24X0_I2C_BASE;
}
static inline S3C24X0_I2S * S3C24X0_GetBase_I2S(void)
{
	return (S3C24X0_I2S * const)S3C24X0_I2S_BASE;
}
static inline S3C24X0_GPIO * S3C24X0_GetBase_GPIO(void)
{
	return (S3C24X0_GPIO * const)S3C24X0_GPIO_BASE;
}
static inline S3C24X0_RTC * S3C24X0_GetBase_RTC(void)
{
	return (S3C24X0_RTC * const)S3C24X0_RTC_BASE;
}
static inline S3C2410_ADC * S3C2410_GetBase_ADC(void)
{
	return (S3C2410_ADC * const)S3C2410_ADC_BASE;
}
static inline S3C24X0_SPI * S3C24X0_GetBase_SPI(void)
{
	return (S3C24X0_SPI * const)S3C24X0_SPI_BASE;
}
static inline S3C2410_SDI * S3C2410_GetBase_SDI(void)
{
	return (S3C2410_SDI * const)S3C2410_SDI_BASE;
}


/* ISR */
#define pISR_RESET		(*(unsigned *)(_ISR_STARTADDRESS+0x0))
#define pISR_UNDEF		(*(unsigned *)(_ISR_STARTADDRESS+0x4))
#define pISR_SWI		(*(unsigned *)(_ISR_STARTADDRESS+0x8))
#define pISR_PABORT		(*(unsigned *)(_ISR_STARTADDRESS+0xC))
#define pISR_DABORT		(*(unsigned *)(_ISR_STARTADDRESS+0x10))
#define pISR_RESERVED		(*(unsigned *)(_ISR_STARTADDRESS+0x14))
#define pISR_IRQ		(*(unsigned *)(_ISR_STARTADDRESS+0x18))
#define pISR_FIQ		(*(unsigned *)(_ISR_STARTADDRESS+0x1C))

#define pISR_EINT0		(*(unsigned *)(_ISR_STARTADDRESS+0x20))
#define pISR_EINT1		(*(unsigned *)(_ISR_STARTADDRESS+0x24))
#define pISR_EINT2		(*(unsigned *)(_ISR_STARTADDRESS+0x28))
#define pISR_EINT3		(*(unsigned *)(_ISR_STARTADDRESS+0x2C))
#define pISR_EINT4_7		(*(unsigned *)(_ISR_STARTADDRESS+0x30))
#define pISR_EINT8_23		(*(unsigned *)(_ISR_STARTADDRESS+0x34))
#define pISR_BAT_FLT		(*(unsigned *)(_ISR_STARTADDRESS+0x3C))
#define pISR_TICK		(*(unsigned *)(_ISR_STARTADDRESS+0x40))
#define pISR_WDT		(*(unsigned *)(_ISR_STARTADDRESS+0x44))
#define pISR_TIMER0		(*(unsigned *)(_ISR_STARTADDRESS+0x48))
#define pISR_TIMER1		(*(unsigned *)(_ISR_STARTADDRESS+0x4C))
#define pISR_TIMER2		(*(unsigned *)(_ISR_STARTADDRESS+0x50))
#define pISR_TIMER3		(*(unsigned *)(_ISR_STARTADDRESS+0x54))
#define pISR_TIMER4		(*(unsigned *)(_ISR_STARTADDRESS+0x58))
#define pISR_UART2		(*(unsigned *)(_ISR_STARTADDRESS+0x5C))
#define pISR_NOTUSED		(*(unsigned *)(_ISR_STARTADDRESS+0x60))
#define pISR_DMA0		(*(unsigned *)(_ISR_STARTADDRESS+0x64))
#define pISR_DMA1		(*(unsigned *)(_ISR_STARTADDRESS+0x68))
#define pISR_DMA2		(*(unsigned *)(_ISR_STARTADDRESS+0x6C))
#define pISR_DMA3		(*(unsigned *)(_ISR_STARTADDRESS+0x70))
#define pISR_SDI		(*(unsigned *)(_ISR_STARTADDRESS+0x74))
#define pISR_SPI0		(*(unsigned *)(_ISR_STARTADDRESS+0x78))
#define pISR_UART1		(*(unsigned *)(_ISR_STARTADDRESS+0x7C))
#define pISR_USBD		(*(unsigned *)(_ISR_STARTADDRESS+0x84))
#define pISR_USBH		(*(unsigned *)(_ISR_STARTADDRESS+0x88))
#define pISR_IIC		(*(unsigned *)(_ISR_STARTADDRESS+0x8C))
#define pISR_UART0		(*(unsigned *)(_ISR_STARTADDRESS+0x90))
#define pISR_SPI1		(*(unsigned *)(_ISR_STARTADDRESS+0x94))
#define pISR_RTC		(*(unsigned *)(_ISR_STARTADDRESS+0x98))
#define pISR_ADC		(*(unsigned *)(_ISR_STARTADDRESS+0xA0))


/* PENDING BIT */
#define BIT_EINT0		(0x1)
#define BIT_EINT1		(0x1<<1)
#define BIT_EINT2		(0x1<<2)
#define BIT_EINT3		(0x1<<3)
#define BIT_EINT4_7		(0x1<<4)
#define BIT_EINT8_23		(0x1<<5)
#define BIT_BAT_FLT		(0x1<<7)
#define BIT_TICK		(0x1<<8)
#define BIT_WDT			(0x1<<9)
#define BIT_TIMER0		(0x1<<10)
#define BIT_TIMER1		(0x1<<11)
#define BIT_TIMER2		(0x1<<12)
#define BIT_TIMER3		(0x1<<13)
#define BIT_TIMER4		(0x1<<14)
#define BIT_UART2		(0x1<<15)
#define BIT_LCD			(0x1<<16)
#define BIT_DMA0		(0x1<<17)
#define BIT_DMA1		(0x1<<18)
#define BIT_DMA2		(0x1<<19)
#define BIT_DMA3		(0x1<<20)
#define BIT_SDI			(0x1<<21)
#define BIT_SPI0		(0x1<<22)
#define BIT_UART1		(0x1<<23)
#define BIT_USBD		(0x1<<25)
#define BIT_USBH		(0x1<<26)
#define BIT_IIC			(0x1<<27)
#define BIT_UART0		(0x1<<28)
#define BIT_SPI1		(0x1<<29)
#define BIT_RTC			(0x1<<30)
#define BIT_ADC			(0x1<<31)
#define BIT_ALLMSK		(0xFFFFFFFF)

#define ClearPending(bit) {\
		 rSRCPND = bit;\
		 rINTPND = bit;\
		 rINTPND;\
		 }
/* Wait until rINTPND is changed for the case that the ISR is very short. */
#endif /*__S3C2410_H__*/
