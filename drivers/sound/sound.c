/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar <rcsekar@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <malloc.h>
#include <common.h>
#include <asm/io.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <i2c.h>
#include <i2s.h>
#include <sound.h>
#include <asm/arch/sound.h>
#include "wm8994.h"
#include "max98095.h"

/* defines */
#define SOUND_400_HZ 400
#define SOUND_BITS_IN_BYTE 8

static struct i2stx_info g_i2stx_pri;

/*
 * get_sound_i2s_values gets values for i2s parameters
 *
 * @param i2stx_info	i2s transmitter transfer param structure
 * @param blob		FDT blob if enabled else NULL
 */
static int get_sound_i2s_values(struct i2stx_info *i2s, const void *blob)
{
#ifdef CONFIG_OF_CONTROL
	int node;
	int error = 0;
	int base;

	node = fdt_path_offset(blob, "i2s");
	if (node <= 0) {
		debug("EXYNOS_SOUND: No node for sound in device tree\n");
		return -1;
	}

	/*
	 * Get the pre-defined sound specific values from FDT.
	 * All of these are expected to be correct otherwise
	 * wrong register values in i2s setup parameters
	 * may result in no sound play.
	 */
	base = fdtdec_get_addr(blob, node, "reg");
	if (base == FDT_ADDR_T_NONE) {
		debug("%s: Missing  i2s base\n", __func__);
		return -1;
	}
	i2s->base_address = base;

	i2s->audio_pll_clk = fdtdec_get_int(blob,
				node, "samsung,i2s-epll-clock-frequency", -1);
	error |= i2s->audio_pll_clk;
	debug("audio_pll_clk = %d\n", i2s->audio_pll_clk);
	i2s->samplingrate = fdtdec_get_int(blob,
				node, "samsung,i2s-sampling-rate", -1);
	error |= i2s->samplingrate;
	debug("samplingrate = %d\n", i2s->samplingrate);
	i2s->bitspersample = fdtdec_get_int(blob,
				node, "samsung,i2s-bits-per-sample", -1);
	error |= i2s->bitspersample;
	debug("bitspersample = %d\n", i2s->bitspersample);
	i2s->channels = fdtdec_get_int(blob,
			node, "samsung,i2s-channels", -1);
	error |= i2s->channels;
	debug("channels = %d\n", i2s->channels);
	i2s->rfs = fdtdec_get_int(blob,
				node, "samsung,i2s-lr-clk-framesize", -1);
	error |= i2s->rfs;
	debug("rfs = %d\n", i2s->rfs);
	i2s->bfs = fdtdec_get_int(blob,
				node, "samsung,i2s-bit-clk-framesize", -1);
	error |= i2s->bfs;
	debug("bfs = %d\n", i2s->bfs);

	i2s->id = fdtdec_get_int(blob, node, "samsung,i2s-id", -1);
	error |= i2s->id;
	debug("id = %d\n", i2s->id);

	if (error == -1) {
		debug("fail to get sound i2s node properties\n");
		return -1;
	}
#else
	i2s->base_address = samsung_get_base_i2s();
	i2s->audio_pll_clk = I2S_PLL_CLK;
	i2s->samplingrate = I2S_SAMPLING_RATE;
	i2s->bitspersample = I2S_BITS_PER_SAMPLE;
	i2s->channels = I2S_CHANNELS;
	i2s->rfs = I2S_RFS;
	i2s->bfs = I2S_BFS;
	i2s->id = 0;
#endif
	return 0;
}

/*
 * Init codec
 *
 * @param blob          FDT blob
 * @param pi2s_tx	i2s parameters required by codec
 * @return              int value, 0 for success
 */
static int codec_init(const void *blob, struct i2stx_info *pi2s_tx)
{
	int ret;
	const char *codectype;
#ifdef CONFIG_OF_CONTROL
	int node;

	/* Get the node from FDT for sound */
	node = fdt_path_offset(blob, "i2s");
	if (node <= 0) {
		debug("EXYNOS_SOUND: No node for sound in device tree\n");
		debug("node = %d\n", node);
		return -1;
	}

	/*
	 * Get the pre-defined sound codec specific values from FDT.
	 * All of these are expected to be correct otherwise sound
	 * can not be played
	 */
	codectype = fdt_getprop(blob, node, "samsung,codec-type", NULL);
	debug("device = %s\n", codectype);
#else
	codectype =  AUDIO_CODEC;
#endif
	if (!strcmp(codectype, "wm8994")) {
		/* Check the codec type and initialise the same */
		ret = wm8994_init(blob, pi2s_tx->id + 1,
				  pi2s_tx->samplingrate,
				  (pi2s_tx->samplingrate * (pi2s_tx->rfs)),
				  pi2s_tx->bitspersample, pi2s_tx->channels);
	} else if (!strcmp(codectype, "max98095")) {
		ret = max98095_init(blob, pi2s_tx->id + 1,
				    pi2s_tx->samplingrate,
				    (pi2s_tx->samplingrate * (pi2s_tx->rfs)),
				    pi2s_tx->bitspersample);
	} else {
		debug("%s: Unknown codec type %s\n", __func__, codectype);
		return -1;
	}

	if (ret) {
		debug("%s: Codec init failed\n", __func__);
		return -1;
	}

	return 0;
}

int sound_init(const void *blob)
{
	int ret;
	struct i2stx_info *pi2s_tx = &g_i2stx_pri;

	/* Get the I2S Values */
	if (get_sound_i2s_values(pi2s_tx, blob) < 0) {
		debug(" FDT I2S values failed\n");
		return -1;
	}

	if (codec_init(blob, pi2s_tx) < 0) {
		debug(" Codec init failed\n");
		return -1;
	}

	ret = i2s_tx_init(pi2s_tx);
	if (ret) {
		debug("%s: Failed to init i2c transmit: ret=%d\n", __func__,
		      ret);
		return ret;
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
