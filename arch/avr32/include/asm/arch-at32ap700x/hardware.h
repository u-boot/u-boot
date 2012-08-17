/*
 * Copyright (C) 2005-2006 Atmel Corporation
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
#ifndef __AT32AP7000_HARDWARE_H__
#define __AT32AP7000_HARDWARE_H__

/* Internal and external memories */
#define EBI_SRAM_CS0_BASE			0x00000000
#define EBI_SRAM_CS0_SIZE			0x04000000
#define EBI_SRAM_CS4_BASE			0x04000000
#define EBI_SRAM_CS4_SIZE			0x04000000
#define EBI_SRAM_CS2_BASE			0x08000000
#define EBI_SRAM_CS2_SIZE			0x04000000
#define EBI_SRAM_CS3_BASE			0x0c000000
#define EBI_SRAM_CS3_SIZE			0x04000000
#define EBI_SRAM_CS1_BASE			0x10000000
#define EBI_SRAM_CS1_SIZE			0x10000000
#define EBI_SRAM_CS5_BASE			0x20000000
#define EBI_SRAM_CS5_SIZE			0x04000000

#define EBI_SDRAM_BASE				EBI_SRAM_CS1_BASE
#define EBI_SDRAM_SIZE				EBI_SRAM_CS1_SIZE

#define INTERNAL_SRAM_BASE			0x24000000
#define INTERNAL_SRAM_SIZE			0x00008000

/* Devices on the High Speed Bus (HSB) */
#define LCDC_BASE					0xFF000000
#define DMAC_BASE					0xFF200000
#define USB_FIFO					0xFF300000

/* Devices on Peripheral Bus A (PBA) */
#define ATMEL_BASE_SPI0				0xFFE00000
#define ATMEL_BASE_SPI1				0xFFE00400
#define ATMEL_BASE_TWI0				0xFFE00800
#define ATMEL_BASE_USART0			0xFFE00C00
#define ATMEL_BASE_USART1			0xFFE01000
#define ATMEL_BASE_USART2			0xFFE01400
#define ATMEL_BASE_USART3			0xFFE01800
#define ATMEL_BASE_SSC0				0xFFE01C00
#define ATMEL_BASE_SSC1				0xFFE02000
#define ATMEL_BASE_SSC2				0xFFE02400
#define ATMEL_BASE_PIOA				0xFFE02800
#define ATMEL_BASE_PIOB				0xFFE02C00
#define ATMEL_BASE_PIOC				0xFFE03000
#define ATMEL_BASE_PIOD				0xFFE03400
#define ATMEL_BASE_PIOE				0xFFE03800
#define ATMEL_BASE_PSIF				0xFFE03C00

/* Devices on Peripheral Bus B (PBB) */
#define ATMEL_BASE_SM				0xFFF00000
#define ATMEL_BASE_INTC				0xFFF00400
#define ATMEL_BASE_HMATRIX			0xFFF00800
#define ATMEL_BASE_TIMER0			0xFFF00C00
#define ATMEL_BASE_TIMER1			0xFFF01000
#define ATMEL_BASE_PWM				0xFFF01400
#define ATMEL_BASE_MACB0			0xFFF01800
#define ATMEL_BASE_MACB1			0xFFF01C00
#define ATMEL_BASE_DAC				0xFFF02000
#define ATMEL_BASE_MMCI				0xFFF02400
#define ATMEL_BASE_AUDIOC			0xFFF02800
#define ATMEL_BASE_HISI				0xFFF02C00
#define ATMEL_BASE_USB				0xFFF03000
#define ATMEL_BASE_HSMC				0xFFF03400
#define ATMEL_BASE_HSDRAMC			0xFFF03800
#define ATMEL_BASE_ECC				0xFFF03C00

#endif /* __AT32AP7000_HARDWARE_H__ */
