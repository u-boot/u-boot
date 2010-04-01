/*
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
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

#define SDRAM_DDR	0		/* is SDR */

/* Settings for XLB = 132 MHz */
#define SDRAM_MODE	0x00CD0000
/* #define SDRAM_MODE	0x008D0000 */ /* CAS latency 2 */
#define SDRAM_CONTROL	0x504F0000
#define SDRAM_CONFIG1	0xD2322800
/* #define SDRAM_CONFIG1	0xD2222800 */ /* CAS latency 2 */
/*#define SDRAM_CONFIG1	0xD7322800 */ /* SDRAM controller bug workaround */
#define SDRAM_CONFIG2	0x8AD70000
/*#define SDRAM_CONFIG2	0xDDD70000 */ /* SDRAM controller bug workaround */
