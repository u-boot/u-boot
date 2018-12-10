// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <audio_codec.h>
#include <asm/sound.h>
#include <asm/sdl.h>

struct sandbox_codec_priv {
	int interface;
	int rate;
	int mclk_freq;
	int bits_per_sample;
	uint channels;
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
