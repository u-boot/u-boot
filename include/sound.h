/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar < rcsekar@samsung.com>
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

#ifndef __SOUND_H__
#define __SOUND_H__

/* sound codec enum */
enum en_sound_codec {
	CODEC_WM_8994,
	CODEC_WM_8995,
	CODEC_MAX
};

/* sound codec enum */
enum sound_compat {
	AUDIO_COMPAT_SPI,
	AUDIO_COMPAT_I2C,
};

/* Codec information structure to store the info from device tree */
struct sound_codec_info {
	int i2c_bus;
	int i2c_dev_addr;
	enum en_sound_codec codec_type;
};

/*
 * Initialises audio sub system
 *
 * @return	int value 0 for success, -1 for error
 */
int sound_init(void);

/*
 * plays the pcm data buffer in pcm_data.h through i2s1 to make the
 * sine wave sound
 *
 * @return	int 0 for success, -1 for error
 */
int sound_play(uint32_t msec, uint32_t frequency);

#endif  /* __SOUND__H__ */
