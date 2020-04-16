/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Tests for logging functions
 */

#ifndef __TEST_LOG_H__
#define __TEST_LOG_H__

#include <test/test.h>

/* Declare a new logging test */
#define LOG_TEST(_name) UNIT_TEST(_name, 0, log_test)

#endif /* __TEST_LOG_H__ */
