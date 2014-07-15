/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/sound.h>
#include <asm/sdl.h>

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
