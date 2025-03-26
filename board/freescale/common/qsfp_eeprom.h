/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2022 NXP
 */

#ifndef __QSFP_EEPROM_H_
#define __QSFP_EEPROM_H_
/*
 * QSFP eeprom reader external API interface.
 */

/* return the ethernet compatibility field 0 */
unsigned char get_qsfp_compat0(void);
#endif  /* __QSFP_EEPROM_H_ */
