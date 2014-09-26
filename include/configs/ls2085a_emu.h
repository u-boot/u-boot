/*
 * Copyright 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __LS2_EMU_H
#define __LS2_EMU_H

#include "ls2085a_common.h"

#define CONFIG_DDR_SPD
#define CONFIG_SYS_FSL_DDR_EMU		/* Support emulator */
#define SPD_EEPROM_ADDRESS1	0x51
#define SPD_EEPROM_ADDRESS2	0x52
#define SPD_EEPROM_ADDRESS3	0x53
#define SPD_EEPROM_ADDRESS	SPD_EEPROM_ADDRESS1
#define CONFIG_SYS_SPD_BUS_NUM	1	/* SPD on I2C bus 1 */

#endif /* __LS2_EMU_H */
