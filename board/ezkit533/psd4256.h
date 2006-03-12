/*
 * U-boot - psd4256.h
 *
 * Copyright (c) 2005 blackfin.uclinux.org
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * Flash A/B Port A configuration registers.
 * Addresses are offset values to CFG_FLASH1_BASE
 * for Flash A and CFG_FLASH2_BASE for Flash B.
 */

#define	PSD_PORTA_DIN	0x070000
#define	PSD_PORTA_DOUT	0x070004
#define	PSD_PORTA_DIR	0x070006

/*
 * Flash A/B Port B configuration registers
 * Addresses are offset values to CFG_FLASH1_BASE
 * for Flash A and CFG_FLASH2_BASE for Flash B.
 */

#define	PSD_PORTB_DIN	0x070001
#define	PSD_PORTB_DOUT	0x070005
#define	PSD_PORTB_DIR	0x070007

/*
 * Flash A Port A Bit definitions
 */

#define	PSDA_PPICLK1	0x20		/* PPI Clock select bit 1		*/
#define	PSDA_PPICLK0	0x10		/* PPI Clock select bit 0		*/
#define	PSDA_VDEC_RST	0x08		/* Video decoder reset, 0 = RESET	*/
#define	PSDA_VENC_RST	0x04		/* Video encoder reset, 0 = RESET	*/
#define	PSDA_CODEC_RST	0x01		/* Codec reset, 0 = RESET		*/

/*
 * Flash A Port B Bit definitions
 */

#define	PSDA_LED9	0x20		/* LED 9, 1 = LED ON			*/
#define	PSDA_LED8	0x10		/* LED 8, 1 = LED ON			*/
#define	PSDA_LED7	0x08		/* LED 7, 1 = LED ON			*/
#define	PSDA_LED6	0x04		/* LED 6, 1 = LED ON			*/
#define	PSDA_LED5	0x02		/* LED 5, 1 = LED ON			*/
#define	PSDA_LED4	0x01		/* LED 4, 1 = LED ON			*/
