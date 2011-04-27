/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _PANTHEON_CONFIG_H
#define _PANTHEON_CONFIG_H

#define CONFIG_ARM926EJS	1	/* Basic Architecture */

#define CONFIG_SYS_TCLK		(14745600)	/* NS16550 clk config */
#define CONFIG_SYS_HZ_CLOCK	(3250000)	/* Timer Freq. 3.25MHZ */
#define CONFIG_MARVELL_MFP			/* Enable mvmfp driver */
#define MV_MFPR_BASE		PANTHEON_MFPR_BASE
#define MV_UART_CONSOLE_BASE	PANTHEON_UART1_BASE
#define CONFIG_SYS_NS16550_IER	(1 << 6)	/* Bit 6 in UART_IER register
						represents UART Unit Enable */
/*
 * I2C definition
 */
#ifdef CONFIG_CMD_I2C
#define CONFIG_I2C_MV			1
#define CONFIG_MV_I2C_REG		0xd4011000
#define CONFIG_HARD_I2C			1
#define CONFIG_SYS_I2C_SPEED		0
#define CONFIG_SYS_I2C_SLAVE		0xfe
#endif

#endif /* _PANTHEON_CONFIG_H */
