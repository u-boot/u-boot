/*
 * linux/include/asm-arm/arch-at91/hardware.h
 *
 *  Copyright (C) 2003 SAN People
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __M501SK_H
#define __M501SK_H

#ifndef __ASSEMBLY__
#include <asm-arm/arch-at91rm9200/AT91RM9200.h>
#else
#include <asm-arm/arch-at91rm9200/AT91RM9200_inc.h>
#endif

#define AT91C_PIO_PA22 ((unsigned int) 1 << 22) /* Pin Controlled by PA22 */
#define AT91C_PA22_RXD2 ((unsigned int) AT91C_PIO_PA22) /* USART 2 RxD */
#define AT91C_PA5_TXD3 ((unsigned int) 1 <<  5) /* USART 3 TxD */
#define AT91C_PA6_RXD3 ((unsigned int) 1 << 6) /* USART 3 RxD */

/* ========== Register definition for PIOD peripheral ========== */
#define AT91C_PIOD_PDSR ((AT91_REG *) 0xFFFFFA3C) /* Pin Data stat Reg */
#define AT91C_PIOD_CODR ((AT91_REG *) 0xFFFFFA34) /* Clear Output Data Reg */
#define AT91C_PIOD_OWER ((AT91_REG *) 0xFFFFFAA0) /* Output Write Enable Reg */
#define AT91C_PIOD_MDER ((AT91_REG *) 0xFFFFFA50) /* Multi-driver Enable Reg */
#define AT91C_PIOD_IMR  ((AT91_REG *) 0xFFFFFA48) /* Interrupt Mask Reg */
#define AT91C_PIOD_IER  ((AT91_REG *) 0xFFFFFA40) /* Interrupt Enable Reg */
#define AT91C_PIOD_ODSR ((AT91_REG *) 0xFFFFFA38) /* Output Data stat Reg */
#define AT91C_PIOD_SODR ((AT91_REG *) 0xFFFFFA30) /* Set Output Data Reg */
#define AT91C_PIOD_PER  ((AT91_REG *) 0xFFFFFA00) /* PIO Enable Reg */
#define AT91C_PIOD_OWDR ((AT91_REG *) 0xFFFFFAA4) /* Output Write Disable Reg */
#define AT91C_PIOD_PPUER ((AT91_REG *) 0xFFFFFA64) /* Pull-up Enable Reg */
#define AT91C_PIOD_MDDR ((AT91_REG *) 0xFFFFFA54) /* Multi-driver Disable Reg */
#define AT91C_PIOD_ISR  ((AT91_REG *) 0xFFFFFA4C) /* Interrupt stat Reg */
#define AT91C_PIOD_IDR  ((AT91_REG *) 0xFFFFFA44) /* Interrupt Disable Reg */
#define AT91C_PIOD_PDR  ((AT91_REG *) 0xFFFFFA04) /* PIO Disable Reg */
#define AT91C_PIOD_ODR  ((AT91_REG *) 0xFFFFFA14) /* Output Disable Regr */
#define AT91C_PIOD_OWSR ((AT91_REG *) 0xFFFFFAA8) /* Output Write stat Reg */
#define AT91C_PIOD_ABSR ((AT91_REG *) 0xFFFFFA78) /* AB Select stat Reg */
#define AT91C_PIOD_ASR  ((AT91_REG *) 0xFFFFFA70) /* Select A Reg */
#define AT91C_PIOD_PPUSR ((AT91_REG *) 0xFFFFFA68) /* Pad Pull-up stat Reg */
#define AT91C_PIOD_PPUDR ((AT91_REG *) 0xFFFFFA60) /* Pull-up Disable Reg */
#define AT91C_PIOD_MDSR ((AT91_REG *) 0xFFFFFA58) /* Multi-driver stat Reg */
#define AT91C_PIOD_PSR  ((AT91_REG *) 0xFFFFFA08) /* PIO stat Reg */
#define AT91C_PIOD_OER  ((AT91_REG *) 0xFFFFFA10) /* Output Enable Reg */
#define AT91C_PIOD_OSR  ((AT91_REG *) 0xFFFFFA18) /* Output stat Reg */
#define AT91C_PIOD_IFER ((AT91_REG *) 0xFFFFFA20) /* Input Filter Enable Reg */
#define AT91C_PIOD_BSR  ((AT91_REG *) 0xFFFFFA74) /* Select B Reg */
#define AT91C_PIOD_IFDR ((AT91_REG *) 0xFFFFFA24) /* Input Filter Disable Reg */
#define AT91C_PIOD_IFSR ((AT91_REG *) 0xFFFFFA28) /* Input Filter stat Reg */

#define AT91C_PIO_PD0   ((unsigned int) 1 <<  0) /* Pin Controlled by PD0 */
#define AT91C_PD0_ETX0  ((unsigned int) AT91C_PIO_PD0) /*  Enet MAC Tx Data 0*/
#define AT91C_PIO_PD1   ((unsigned int) 1 <<  1) /* Pin Controlled by PD1 */
#define AT91C_PD1_ETX1  ((unsigned int) AT91C_PIO_PD1) /*  Enet MAC Tx Data 1*/
#define AT91C_PIO_PD10  ((unsigned int) 1 << 10) /* Pin Controlled by PD10 */
#define AT91C_PD10_PCK3 ((unsigned int) AT91C_PIO_PD10) /* PMC Prog Clk Oput 3*/
#define AT91C_PD10_TPS1 ((unsigned int) AT91C_PIO_PD10) /* ETMARM9 pl stat1 */
#define AT91C_PIO_PD11  ((unsigned int) 1 << 11) /* Pin Controlled by PD11 */
#define AT91C_PD11_     ((unsigned int) AT91C_PIO_PD11) /*   */
#define AT91C_PD11_TPS2 ((unsigned int) AT91C_PIO_PD11) /* ETMARM9 pl stat2 */
#define AT91C_PIO_PD12  ((unsigned int) 1 << 12) /* Pin Controlled by PD12 */
#define AT91C_PD12_     ((unsigned int) AT91C_PIO_PD12) /*   */
#define AT91C_PD12_TPK0 ((unsigned int) AT91C_PIO_PD12) /* ETM Trace Pkt 0 */
#define AT91C_PIO_PD13  ((unsigned int) 1 << 13) /* Pin Controlled by PD13 */
#define AT91C_PD13_     ((unsigned int) AT91C_PIO_PD13) /*   */
#define AT91C_PD13_TPK1 ((unsigned int) AT91C_PIO_PD13) /* ETM Trace Pkt 1 */
#define AT91C_PIO_PD14  ((unsigned int) 1 << 14) /* Pin Controlled by PD14 */
#define AT91C_PD14_     ((unsigned int) AT91C_PIO_PD14) /*   */
#define AT91C_PD14_TPK2 ((unsigned int) AT91C_PIO_PD14) /* ETM Trace Pkt 2 */
#define AT91C_PIO_PD15  ((unsigned int) 1 << 15) /* Pin Controlled by PD15 */
#define AT91C_PD15_TD0  ((unsigned int) AT91C_PIO_PD15) /* SSC TxD */
#define AT91C_PD15_TPK3 ((unsigned int) AT91C_PIO_PD15) /* ETM Trace Pkt 3 */
#define AT91C_PIO_PD16  ((unsigned int) 1 << 16) /* Pin Controlled by PD16 */
#define AT91C_PD16_TD1  ((unsigned int) AT91C_PIO_PD16) /* SSC TxD 1 */
#define AT91C_PD16_TPK4 ((unsigned int) AT91C_PIO_PD16) /* ETM Trace Pkt 4 */
#define AT91C_PIO_PD17  ((unsigned int) 1 << 17) /* Pin Controlled by PD17 */
#define AT91C_PD17_TD2  ((unsigned int) AT91C_PIO_PD17) /* SSC TxD 2 */
#define AT91C_PD17_TPK5 ((unsigned int) AT91C_PIO_PD17) /* ETM Trace Pkt 5 */
#define AT91C_PIO_PD18  ((unsigned int) 1 << 18) /* Pin Controlled by PD18 */
#define AT91C_PD18_NPCS1 ((unsigned int) AT91C_PIO_PD18) /*  SPI Perip CS 1 */
#define AT91C_PD18_TPK6 ((unsigned int) AT91C_PIO_PD18) /* ETM Trace Pkt 6 */
#define AT91C_PIO_PD19  ((unsigned int) 1 << 19) /* Pin Controlled by PD19 */
#define AT91C_PD19_NPCS2 ((unsigned int) AT91C_PIO_PD19) /*  SPI Perip CS 2 */
#define AT91C_PD19_TPK7 ((unsigned int) AT91C_PIO_PD19) /* ETM Trace Pkt 7 */
#define AT91C_PIO_PD2   ((unsigned int) 1 <<  2) /* Pin Controlled by PD2 */
#define AT91C_PD2_ETX2  ((unsigned int) AT91C_PIO_PD2) /*  Ethernet MAC TxD 2 */
#define AT91C_PIO_PD20  ((unsigned int) 1 << 20) /* Pin Controlled by PD20 */
#define AT91C_PD20_NPCS3 ((unsigned int) AT91C_PIO_PD20) /* SPI Perip CS 3 */
#define AT91C_PD20_TPK8 ((unsigned int) AT91C_PIO_PD20) /* ETM Trace Pkt 8 */
#define AT91C_PIO_PD21  ((unsigned int) 1 << 21) /* Pin Controlled by PD21 */
#define AT91C_PD21_RTS0 ((unsigned int) AT91C_PIO_PD21) /* Usart 0 RTS */
#define AT91C_PD21_TPK9 ((unsigned int) AT91C_PIO_PD21) /* ETM Trace Pkt 9 */
#define AT91C_PIO_PD22  ((unsigned int) 1 << 22) /* Pin Controlled by PD22 */
#define AT91C_PD22_RTS1 ((unsigned int) AT91C_PIO_PD22) /*  Usart 0 RTS */
#define AT91C_PD22_TPK10 ((unsigned int) AT91C_PIO_PD22) /* ETM Trace Pkt 10 */
#define AT91C_PIO_PD23  ((unsigned int) 1 << 23) /* Pin Controlled by PD23 */
#define AT91C_PD23_RTS2 ((unsigned int) AT91C_PIO_PD23) /* USART 2 RTS */
#define AT91C_PD23_TPK11 ((unsigned int) AT91C_PIO_PD23) /* ETM Trace Pkt 11 */
#define AT91C_PIO_PD24  ((unsigned int) 1 << 24) /* Pin Controlled by PD24 */
#define AT91C_PD24_RTS3 ((unsigned int) AT91C_PIO_PD24) /*  USART 3 RTS */
#define AT91C_PD24_TPK12 ((unsigned int) AT91C_PIO_PD24) /* ETM Trace Pkt 12 */
#define AT91C_PIO_PD25  ((unsigned int) 1 << 25) /* Pin Controlled by PD25 */
#define AT91C_PD25_DTR1 ((unsigned int) AT91C_PIO_PD25) /* USART 1 DTR */
#define AT91C_PD25_TPK13 ((unsigned int) AT91C_PIO_PD25) /* ETM Trace Pkt 13 */
#define AT91C_PIO_PD26  ((unsigned int) 1 << 26) /* Pin Controlled by PD26 */
#define AT91C_PD26_TPK14 ((unsigned int) AT91C_PIO_PD26) /* ETM Trace Pkt 14 */
#define AT91C_PIO_PD27  ((unsigned int) 1 << 27) /* Pin Controlled by PD27 */
#define AT91C_PD27_TPK15 ((unsigned int) AT91C_PIO_PD27) /* ETM Trace Pkt 15 */
#define AT91C_PIO_PD3   ((unsigned int) 1 <<  3) /* Pin Controlled by PD3 */
#define AT91C_PD3_ETX3  ((unsigned int) AT91C_PIO_PD3) /*  Enet MAC TxD 3 */
#define AT91C_PIO_PD4   ((unsigned int) 1 <<  4) /* Pin Controlled by PD4 */
#define AT91C_PD4_ETXEN ((unsigned int) AT91C_PIO_PD4) /* Enet MAC TxEn */
#define AT91C_PIO_PD5   ((unsigned int) 1 <<  5) /* Pin Controlled by PD5 */
#define AT91C_PD5_ETXER ((unsigned int) AT91C_PIO_PD5) /*  Enet MAC TxCE */
#define AT91C_PIO_PD6   ((unsigned int) 1 <<  6) /* Pin Controlled by PD6 */
#define AT91C_PD6_DTXD  ((unsigned int) AT91C_PIO_PD6) /* DBGU Debug TxD */
#define AT91C_PIO_PD7   ((unsigned int) 1 <<  7) /* Pin Controlled by PD7 */
#define AT91C_PD7_PCK0  ((unsigned int) AT91C_PIO_PD7) /* PMC Prog Clk Oput 0*/
#define AT91C_PD7_TSYNC ((unsigned int) AT91C_PIO_PD7) /* ETM Sync signal */
#define AT91C_PIO_PD8   ((unsigned int) 1 <<  8) /* Pin Controlled by PD8 */
#define AT91C_PD8_PCK1  ((unsigned int) AT91C_PIO_PD8) /* PMC Prog Clk Oput 1*/
#define AT91C_PD8_TCLK  ((unsigned int) AT91C_PIO_PD8) /* ETM Trace Clk sig */
#define AT91C_PIO_PD9   ((unsigned int) 1 <<  9) /* Pin Controlled by PD9 */
#define AT91C_PD9_PCK2  ((unsigned int) AT91C_PIO_PD9) /* PMC Prog Clk 2 */
#define AT91C_PD9_TPS0  ((unsigned int) AT91C_PIO_PD9) /* ETM ARM9 pl stat0 */
#define AT91C_PIO_PB6   ((unsigned int) 1 <<  6) /* Pin Controlled by PB6 */
#define AT91C_PIO_PC5   ((unsigned int) 1 <<  5)
#define AT91C_PIO_PC14  ((unsigned int) 1 <<  14) /* Pin Controlled by PC1 */
#define AT91C_PIO_PC15  ((unsigned int) 1 <<  15) /* Pin Controlled by PC1 */
#define AT91C_PIO_PA19  ((unsigned int) 1 <<  19) /* Pin Controlled by PC1 */
#define AT91C_PIO_PB2   ((unsigned int) 1 <<  2) /* Pin Controlled by PC1 */
#define AT91C_PIO_PB8   ((unsigned int) 1 <<  8)
#define AT91C_PIO_PB9   ((unsigned int) 1 <<  9)
#define AT91C_PIO_PB10  ((unsigned int) 1 <<  10)
#define AT91C_PIO_PB11  ((unsigned int) 1 <<  11)
#define AT91C_PIO_PB17  ((unsigned int) 1 <<  17)
#define AT91C_PIO_PB28  ((unsigned int) 1 <<  28)
#define AT91C_PIO_PB29  ((unsigned int) 1 <<  29)

typedef enum {
	M501SK_BUZZER = 38,
	M501SK_DEBUG_LED1 = 96,
	M501SK_DEBUG_LED2,
	M501SK_DEBUG_LED3,
	M501SK_DEBUG_LED4,
	M501SK_READY_LED = 102,
} M501SK_PIO;

void m501sk_gpio_init(void);
uchar m501sk_gpio_set(M501SK_PIO io);
uchar m501sk_gpio_clear(M501SK_PIO io);

#endif
