/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SANDBOX_SOUND_H
#define __SANDBOX_SOUND_H

int sound_play(unsigned int msec, unsigned int frequency);

int sound_init(const void *blob);

#endif
