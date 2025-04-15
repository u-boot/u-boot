/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019, Theobroma Systems Design und Consulting GmbH
 */

#ifndef __TEST_OPTEE_H__
#define __TEST_OPTEE_H__

#include <test/test.h>

/* Declare a new optee test */
#define OPTEE_TEST(_name, _flags)	UNIT_TEST(_name, _flags, optee)
#define OPTEE_TEST_INIT(_name, _flags)	UNIT_TEST_INIT(_name, _flags, optee)
#define OPTEE_TEST_UNINIT(_name, _flags) UNIT_TEST_UNINIT(_name, _flags, optee)

#endif /* __TEST_OPTEE_H__ */
