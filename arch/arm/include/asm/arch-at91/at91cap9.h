/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91cap9.h]
 *
 *  Copyright (C) 2007 Stelian Pop <stelian@popies.net>
 *  Copyright (C) 2007 Lead Tech Design <www.leadtechdesign.com>
 *  Copyright (C) 2007 Atmel Corporation.
 *
 * Common definitions.
 * Based on AT91CAP9 datasheet revision B (Preliminary).
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef AT91CAP9_H
#define AT91CAP9_H

/*
 * Peripheral identifiers/interrupts.
 */
#define AT91_ID_FIQ		0	/* Advanced Interrupt Controller (FIQ) */
#define AT91_ID_SYS		1	/* System Peripherals */
#define AT91CAP9_ID_PIOABCD	2	/* Parallel IO Controller A, B, C and D */
#define AT91CAP9_ID_MPB0	3	/* MP Block Peripheral 0 */
#define AT91CAP9_ID_MPB1	4	/* MP Block Peripheral 1 */
#define AT91CAP9_ID_MPB2	5	/* MP Block Peripheral 2 */
#define AT91CAP9_ID_MPB3	6	/* MP Block Peripheral 3 */
#define AT91CAP9_ID_MPB4	7	/* MP Block Peripheral 4 */
#define AT91CAP9_ID_US0		8	/* USART 0 */
#define AT91CAP9_ID_US1		9	/* USART 1 */
#define AT91CAP9_ID_US2		10	/* USART 2 */
#define AT91CAP9_ID_MCI0	11	/* Multimedia Card Interface 0 */
#define AT91CAP9_ID_MCI1	12	/* Multimedia Card Interface 1 */
#define AT91CAP9_ID_CAN		13	/* CAN */
#define AT91CAP9_ID_TWI		14	/* Two-Wire Interface */
#define AT91CAP9_ID_SPI0	15	/* Serial Peripheral Interface 0 */
#define AT91CAP9_ID_SPI1	16	/* Serial Peripheral Interface 0 */
#define AT91CAP9_ID_SSC0	17	/* Serial Synchronous Controller 0 */
#define AT91CAP9_ID_SSC1	18	/* Serial Synchronous Controller 1 */
#define AT91CAP9_ID_AC97C	19	/* AC97 Controller */
#define AT91CAP9_ID_TCB		20	/* Timer Counter 0, 1 and 2 */
#define AT91CAP9_ID_PWMC	21	/* Pulse Width Modulation Controller */
#define AT91CAP9_ID_EMAC	22	/* Ethernet */
#define AT91CAP9_ID_AESTDES	23	/* Advanced Encryption Standard, Triple DES */
#define AT91CAP9_ID_ADC		24	/* Analog-to-Digital Converter */
#define AT91CAP9_ID_ISI		25	/* Image Sensor Interface */
#define AT91CAP9_ID_LCDC	26	/* LCD Controller */
#define AT91CAP9_ID_DMA		27	/* DMA Controller */
#define AT91CAP9_ID_UDPHS	28	/* USB High Speed Device Port */
#define AT91CAP9_ID_UHP		29	/* USB Host Port */
#define AT91CAP9_ID_IRQ0	30	/* Advanced Interrupt Controller (IRQ0) */
#define AT91CAP9_ID_IRQ1	31	/* Advanced Interrupt Controller (IRQ1) */

#define AT91_PIO_BASE	0xfffff200
#define AT91_PMC_BASE	0xfffffc00
#define AT91_RSTC_BASE	0xfffffd00
#define AT91_PIT_BASE	0xfffffd30

/*
 * Internal Memory.
 */
#define AT91CAP9_SRAM_BASE	0x00100000	/* Internal SRAM base address */
#define AT91CAP9_SRAM_SIZE	(32 * SZ_1K)	/* Internal SRAM size (32Kb) */

#define AT91CAP9_ROM_BASE	0x00400000	/* Internal ROM base address */
#define AT91CAP9_ROM_SIZE	(32 * SZ_1K)	/* Internal ROM size (32Kb) */

#define AT91CAP9_LCDC_BASE	0x00500000	/* LCD Controller */
#define AT91CAP9_UDPHS_BASE	0x00600000	/* USB High Speed Device Port */
#define AT91CAP9_UHP_BASE	0x00700000	/* USB Host controller */

#define CONFIG_DRAM_BASE	AT91_CHIPSELECT_6

/*
 * Cpu Name
 */
#define CONFIG_SYS_AT91_CPU_NAME	"AT91CAP9"

#endif
