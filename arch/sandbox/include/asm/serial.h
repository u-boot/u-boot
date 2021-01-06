/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __asm_serial_h
#define __asm_serial_h

#include <dt-structs.h>

struct sandbox_serial_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_sandbox_serial dtplat;
#endif
	int colour;	/* Text colour to use for output, -1 for none */
};

/**
 * struct sandbox_serial_priv - Private data for this driver
 *
 * @buf: holds input characters available to be read by this driver
 */
struct sandbox_serial_priv {
	struct membuff buf;
	char serial_buf[16];
	bool start_of_line;
};

#endif /* __asm_serial_h */
