/*
 * Copyright (C) 2012 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
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


#ifndef __SOUND_ARCH_H__
#define __SOUND_ARCH_H__

/* I2S values */
#define I2S_PLL_CLK		192000000
#define I2S_SAMPLING_RATE	48000
#define I2S_BITS_PER_SAMPLE	16
#define I2S_CHANNELS		2
#define I2S_RFS			256
#define I2S_BFS			32

/* I2C values */
#define AUDIO_I2C_BUS		1
#define AUDIO_I2C_REG		0x1a

/* Audio Codec */
#define AUDIO_CODEC		"wm8994"

#define AUDIO_COMPAT		1
#endif
