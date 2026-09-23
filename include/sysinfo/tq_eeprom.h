/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Nora Schiffer
 */

#ifndef __SYSINFO_TQ_EEPROM_H__
#define __SYSINFO_TQ_EEPROM_H__

#include <sysinfo.h>

enum {
	/* Model string of TQ-Systems SOM. This is different from BOARD_MODEL,
	 * which usually combines SOM and baseboard names for TQ hardware
	 */
	SYSID_TQ_MODEL = SYSID_USER,
	/* SOM serial number */
	SYSID_TQ_SERIAL,
	/* MAC address */
	SYSID_TQ_MAC_ADDR,
};

#endif /* __SYSINFO_TQ_EEPROM_H__ */
