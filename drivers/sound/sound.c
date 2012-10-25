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

#include <malloc.h>
#include <common.h>
#include <asm/io.h>
#include <i2c.h>
#include <i2s.h>
#include <sound.h>
#include "wm8994.h"
#include <asm/arch/sound.h>

/* defines */
#define SOUND_400_HZ 400
#define SOUND_BITS_IN_BYTE 8

static struct i2stx_info g_i2stx_pri;
static struct sound_codec_info g_codec_info;

/*
 * get_sound_fdt_values gets fdt values for i2s parameters
 *
 * @param i2stx_info	i2s transmitter transfer param structure
 * @param blob		FDT blob
 */
static void get_sound_i2s_values(struct i2stx_info *i2s)
{
	i2s->base_address = samsung_get_base_i2s();
	i2s->audio_pll_clk = I2S_PLL_CLK;
	i2s->samplingrate = I2S_SAMPLING_RATE;
	i2s->bitspersample = I2S_BITS_PER_SAMPLE;
	i2s->channels = I2S_CHANNELS;
	i2s->rfs = I2S_RFS;
	i2s->bfs = I2S_BFS;
}

/*
 * Gets fdt values for wm8994 config parameters
 *
 * @param pcodec_info	codec information structure
 * @param blob		FDT blob
 * @return		int value, 0 for success
 */
static int get_sound_wm8994_values(struct sound_codec_info *pcodec_info)
{
	int error = 0;

	switch (AUDIO_COMPAT) {
	case AUDIO_COMPAT_SPI:
		debug("%s: Support not added for SPI interface\n", __func__);
		return -1;
		break;
	case AUDIO_COMPAT_I2C:
		pcodec_info->i2c_bus = AUDIO_I2C_BUS;
		pcodec_info->i2c_dev_addr = AUDIO_I2C_REG;
		debug("i2c dev addr = %d\n", pcodec_info->i2c_dev_addr);
		break;
	default:
		debug("%s: Unknown compat id %d\n", __func__, AUDIO_COMPAT);
		return -1;
	}

	if (error == -1) {
		debug("fail to get wm8994 codec node properties\n");
		return -1;
	}

	return 0;
}

/*
 * Gets fdt values for codec config parameters
 *
 * @param pcodec_info	codec information structure
 * @param blob		FDT blob
 * @return		int value, 0 for success
 */
static int get_sound_codec_values(struct sound_codec_info *pcodec_info)
{
	int error = 0;
	const char *codectype;

	codectype =  AUDIO_CODEC;

	if (!strcmp(codectype, "wm8994")) {
		pcodec_info->codec_type = CODEC_WM_8994;
		error = get_sound_wm8994_values(pcodec_info);
	} else {
		error = -1;
	}

	if (error == -1) {
		debug("fail to get sound codec node properties\n");
		return -1;
	}

	return 0;
}

int sound_init(void)
{
	int ret;
	struct i2stx_info *pi2s_tx = &g_i2stx_pri;
	struct sound_codec_info *pcodec_info = &g_codec_info;

	/* Get the I2S Values */
	get_sound_i2s_values(pi2s_tx);

	/* Get the codec Values */
	if (get_sound_codec_values(pcodec_info) < 0)
		return -1;

	ret = i2s_tx_init(pi2s_tx);
	if (ret) {
		debug("%s: Failed to init i2c transmit: ret=%d\n", __func__,
		      ret);
		return ret;
	}

	/* Check the codec type and initialise the same */
	if (pcodec_info->codec_type == CODEC_WM_8994) {
		ret = wm8994_init(pcodec_info, WM8994_AIF2,
			pi2s_tx->samplingrate,
			(pi2s_tx->samplingrate * (pi2s_tx->rfs)),
			pi2s_tx->bitspersample, pi2s_tx->channels);
	} else {
		debug("%s: Unknown code type %d\n", __func__,
		      pcodec_info->codec_type);
		return -1;
	}
	if (ret) {
		debug("%s: Codec init failed\n", __func__);
		return -1;
	}

	return ret;
}

/*
 * Generates square wave sound data for 1 second
 *
 * @param data          data buffer pointer
 * @param size          size of the buffer
 * @param freq          frequency of the wave
 */
static void sound_prepare_buffer(unsigned short *data, int size, uint32_t freq)
{
	const int sample = 48000;
	const unsigned short amplitude = 16000; /* between 1 and 32767 */
	const int period = freq ? sample / freq : 0;
	const int half = period / 2;

	assert(freq);

	/* Make sure we don't overflow our buffer */
	if (size % 2)
		size--;

	while (size) {
		int i;
		for (i = 0; size && i < half; i++) {
			size -= 2;
			*data++ = amplitude;
			*data++ = amplitude;
		}
		for (i = 0; size && i < period - half; i++) {
			size -= 2;
			*data++ = -amplitude;
			*data++ = -amplitude;
		}
	}
}

int sound_play(uint32_t msec, uint32_t frequency)
{
	unsigned int *data;
	unsigned long data_size;
	unsigned int ret = 0;

	/*Buffer length computation */
	data_size = g_i2stx_pri.samplingrate * g_i2stx_pri.channels;
	data_size *= (g_i2stx_pri.bitspersample / SOUND_BITS_IN_BYTE);
	data = malloc(data_size);

	if (data == NULL) {
		debug("%s: malloc failed\n", __func__);
		return -1;
	}

	sound_prepare_buffer((unsigned short *)data,
				data_size / sizeof(unsigned short), frequency);

	while (msec >= 1000) {
		ret = i2s_transfer_tx_data(&g_i2stx_pri, data,
					   (data_size / sizeof(int)));
		msec -= 1000;
	}
	if (msec) {
		unsigned long size =
			(data_size * msec) / (sizeof(int) * 1000);

		ret = i2s_transfer_tx_data(&g_i2stx_pri, data, size);
	}

	free(data);

	return ret;
}
