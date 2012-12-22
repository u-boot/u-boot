/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar <rcsekar@samsung.com>
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

#ifndef __I2S_REGS_H__
#define __I2S_REGS_H__

#define CON_TXFIFO_FULL		(1 << 8)
#define CON_TXCH_PAUSE		(1 << 4)
#define CON_ACTIVE		(1 << 0)

#define MOD_BLCP_SHIFT		24
#define MOD_BLCP_16BIT		(0 << MOD_BLCP_SHIFT)
#define MOD_BLCP_8BIT		(1 << MOD_BLCP_SHIFT)
#define MOD_BLCP_24BIT		(2 << MOD_BLCP_SHIFT)
#define MOD_BLCP_MASK		(3 << MOD_BLCP_SHIFT)

#define MOD_BLC_16BIT		(0 << 13)
#define MOD_BLC_8BIT		(1 << 13)
#define MOD_BLC_24BIT		(2 << 13)
#define MOD_BLC_MASK		(3 << 13)

#define MOD_SLAVE		(1 << 11)
#define MOD_MASK		(3 << 8)
#define MOD_LR_LLOW		(0 << 7)
#define MOD_LR_RLOW		(1 << 7)
#define MOD_SDF_IIS		(0 << 5)
#define MOD_SDF_MSB		(1 << 5)
#define MOD_SDF_LSB		(2 << 5)
#define MOD_SDF_MASK		(3 << 5)
#define MOD_RCLK_256FS		(0 << 3)
#define MOD_RCLK_512FS		(1 << 3)
#define MOD_RCLK_384FS		(2 << 3)
#define MOD_RCLK_768FS		(3 << 3)
#define MOD_RCLK_MASK		(3 << 3)
#define MOD_BCLK_32FS		(0 << 1)
#define MOD_BCLK_48FS		(1 << 1)
#define MOD_BCLK_16FS		(2 << 1)
#define MOD_BCLK_24FS		(3 << 1)
#define MOD_BCLK_MASK		(3 << 1)

#define MOD_CDCLKCON		(1 << 12)

#define FIC_TXFLUSH		(1 << 15)
#define FIC_RXFLUSH		(1 << 7)

#endif /* __I2S_REGS_H__ */
