// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <audio_codec.h>
#include <dm.h>
#include <i2s.h>
#include <asm/sound.h>
#include <asm/sdl.h>

struct sandbox_codec_priv {
	int interface;
	int rate;
	int mclk_freq;
	int bits_per_sample;
	uint channels;
};

struct sandbox_i2s_priv {
	int sum;	/* Use to sum the provided audio data */
};

int sound_play(uint32_t msec, uint32_t frequency)
{
	sandbox_sdl_sound_start(frequency);
	mdelay(msec);
	sandbox_sdl_sound_stop();

	return 0;
}

int sound_init(const void *blob)
{
	return sandbox_sdl_sound_init();
}

void sandbox_get_codec_params(struct udevice *dev, int *interfacep, int *ratep,
			      int *mclk_freqp, int *bits_per_samplep,
			      uint *channelsp)
{
	struct sandbox_codec_priv *priv = dev_get_priv(dev);

	*interfacep = priv->interface;
	*ratep = priv->rate;
	*mclk_freqp = priv->mclk_freq;
	*bits_per_samplep = priv->bits_per_sample;
	*channelsp = priv->channels;
}

int sandbox_get_i2s_sum(struct udevice *dev)
{
	struct sandbox_i2s_priv *priv = dev_get_priv(dev);

	return priv->sum;
}

static int sandbox_codec_set_params(struct udevice *dev, int interface,
				    int rate, int mclk_freq,
				    int bits_per_sample, uint channels)
{
	struct sandbox_codec_priv *priv = dev_get_priv(dev);

	priv->interface = interface;
	priv->rate = rate;
	priv->mclk_freq = mclk_freq;
	priv->bits_per_sample = bits_per_sample;
	priv->channels = channels;

	return 0;
}

static int sandbox_i2s_tx_data(struct udevice *dev, void *data,
			       uint data_size)
{
	struct sandbox_i2s_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < data_size; i++)
		priv->sum += ((uint8_t *)data)[i];

	return 0;
}

static int sandbox_i2s_probe(struct udevice *dev)
{
	struct i2s_uc_priv *uc_priv = dev_get_uclass_priv(dev);

	/* Use hard-coded values here */
	uc_priv->rfs = 256;
	uc_priv->bfs = 32;
	uc_priv->audio_pll_clk = 192000000;
	uc_priv->samplingrate = 48000;
	uc_priv->bitspersample = 16;
	uc_priv->channels = 2;
	uc_priv->id = 1;

	return 0;
}

static const struct audio_codec_ops sandbox_codec_ops = {
	.set_params	= sandbox_codec_set_params,
};

static const struct udevice_id sandbox_codec_ids[] = {
	{ .compatible = "sandbox,audio-codec" },
	{ }
};

U_BOOT_DRIVER(sandbox_codec) = {
	.name		= "sandbox_codec",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= sandbox_codec_ids,
	.ops		= &sandbox_codec_ops,
	.priv_auto_alloc_size	= sizeof(struct sandbox_codec_priv),
};

static const struct i2s_ops sandbox_i2s_ops = {
	.tx_data	= sandbox_i2s_tx_data,
};

static const struct udevice_id sandbox_i2s_ids[] = {
	{ .compatible = "sandbox,i2s" },
	{ }
};

U_BOOT_DRIVER(sandbox_i2s) = {
	.name		= "sandbox_i2s",
	.id		= UCLASS_I2S,
	.of_match	= sandbox_i2s_ids,
	.ops		= &sandbox_i2s_ops,
	.probe		= sandbox_i2s_probe,
	.priv_auto_alloc_size	= sizeof(struct sandbox_i2s_priv),
};
