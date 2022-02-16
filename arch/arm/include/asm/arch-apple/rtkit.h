// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#define APPLE_RTKIT_PWR_STATE_SLEEP	0x01
#define APPLE_RTKIT_PWR_STATE_QUIESCED	0x10
#define APPLE_RTKIT_PWR_STATE_ON	0x20

int apple_rtkit_init(struct mbox_chan *);
int apple_rtkit_shutdown(struct mbox_chan *, int);
