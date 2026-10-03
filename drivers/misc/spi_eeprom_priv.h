/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Command set shared by the AT25-style SPI EEPROM driver and its sandbox
 * emulator. These opcodes are an implementation detail of the driver, not
 * part of the UCLASS_SPI_EEPROM API.
 *
 * Copyright (c) 2024 Koninklijke Philips N.V.
 * Copyright (c) 2026 João Loureiro <joaofl@gmail.com>
 */

#ifndef __SPI_EEPROM_PRIV_H
#define __SPI_EEPROM_PRIV_H

/* AT25-style command set */
#define AT25_CMD_READ_DATA	0x03	/* Read data from memory array */
#define AT25_CMD_READ_STATUS	0x05	/* Read status register */

#endif /* __SPI_EEPROM_PRIV_H */
