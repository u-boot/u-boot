/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar < rcsekar@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SOUND_H__
#define __SOUND_H__

/* sound codec enum */
enum en_sound_codec {
	CODEC_WM_8994,
	CODEC_WM_8995,
	CODEC_MAX_98095,
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
 * Generates square wave sound data for 1 second
 *
 * @param data          data buffer pointer
 * @param size          size of the buffer
 * @param freq          frequency of the wave
 */
void sound_create_square_wave(unsigned short *data, int size, uint32_t freq);

/*
 * Initialises audio sub system
 * @param blob	Pointer of device tree node or NULL if none.
 * @return	int value 0 for success, -1 for error
 */
int sound_init(const void *blob);

/*
 * plays the pcm data buffer in pcm_data.h through i2s1 to make the
 * sine wave sound
 *
 * @return	int 0 for success, -1 for error
 */
int sound_play(uint32_t msec, uint32_t frequency);

#endif  /* __SOUND__H__ */
