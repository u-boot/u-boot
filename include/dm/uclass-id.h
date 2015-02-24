/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DM_UCLASS_ID_H
#define _DM_UCLASS_ID_H

/* TODO(sjg@chromium.org): this could be compile-time generated */
enum uclass_id {
	/* These are used internally by driver model */
	UCLASS_ROOT = 0,
	UCLASS_DEMO,
	UCLASS_TEST,
	UCLASS_TEST_FDT,
	UCLASS_TEST_BUS,
	UCLASS_SPI_EMUL,	/* sandbox SPI device emulator */
	UCLASS_I2C_EMUL,	/* sandbox I2C device emulator */
	UCLASS_SIMPLE_BUS,

	/* U-Boot uclasses start here */
	UCLASS_GPIO,		/* Bank of general-purpose I/O pins */
	UCLASS_SERIAL,		/* Serial UART */
	UCLASS_SPI,		/* SPI bus */
	UCLASS_SPI_GENERIC,	/* Generic SPI flash target */
	UCLASS_SPI_FLASH,	/* SPI flash */
	UCLASS_CROS_EC,	/* Chrome OS EC */
	UCLASS_THERMAL,		/* Thermal sensor */
	UCLASS_I2C,		/* I2C bus */
	UCLASS_I2C_GENERIC,	/* Generic I2C device */
	UCLASS_I2C_EEPROM,	/* I2C EEPROM device */
	UCLASS_MOD_EXP,		/* RSA Mod Exp device */

	UCLASS_COUNT,
	UCLASS_INVALID = -1,
};

#endif
