/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __TEST_COMPRESSION_H__
#define __TEST_COMPRESSION_H__

#include <test/test.h>

/* Declare a new compression test */
#define COMPRESSION_TEST(_name, _flags) \
		UNIT_TEST(_name, _flags, compression_test)

#endif /* __TEST_ENV_H__ */
