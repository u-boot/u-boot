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
#ifndef __AT32AP7000_MEMORY_MAP_H__
#define __AT32AP7000_MEMORY_MAP_H__

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
#define LCDC_BASE				0xFF000000
#define DMAC_BASE				0xFF200000
#define USB_FIFO				0xFF300000

/* Devices on Peripheral Bus A (PBA) */
#define SPI0_BASE				0xFFE00000
#define SPI1_BASE				0xFFE00400
#define TWI_BASE				0xFFE00800
#define USART0_BASE				0xFFE00C00
#define USART1_BASE				0xFFE01000
#define USART2_BASE				0xFFE01400
#define USART3_BASE				0xFFE01800
#define SSC0_BASE				0xFFE01C00
#define SSC1_BASE				0xFFE02000
#define SSC2_BASE				0xFFE02400
#define PIOA_BASE				0xFFE02800
#define PIOB_BASE				0xFFE02C00
#define PIOC_BASE				0xFFE03000
#define PIOD_BASE				0xFFE03400
#define PIOE_BASE				0xFFE03800
#define PSIF_BASE				0xFFE03C00

/* Devices on Peripheral Bus B (PBB) */
#define SM_BASE					0xFFF00000
#define INTC_BASE				0xFFF00400
#define HMATRIX_BASE				0xFFF00800
#define TIMER0_BASE				0xFFF00C00
#define TIMER1_BASE				0xFFF01000
#define PWM_BASE				0xFFF01400
#define MACB0_BASE				0xFFF01800
#define MACB1_BASE				0xFFF01C00
#define DAC_BASE				0xFFF02000
#define MMCI_BASE				0xFFF02400
#define AUDIOC_BASE				0xFFF02800
#define HISI_BASE				0xFFF02C00
#define USB_BASE				0xFFF03000
#define HSMC_BASE				0xFFF03400
#define HSDRAMC_BASE				0xFFF03800
#define ECC_BASE				0xFFF03C00

#endif /* __AT32AP7000_MEMORY_MAP_H__ */
