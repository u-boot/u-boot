/*
 * Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef _LACIE_COMMON_H
#define _LACIE_COMMON_H

#if defined(CONFIG_CMD_NET) && defined(CONFIG_RESET_PHY_R)
void mv_phy_88e1116_init(const char *name, u16 phyaddr);
void mv_phy_88e1318_init(const char *name, u16 phyaddr);
#endif
#if defined(CONFIG_CMD_I2C) && defined(CONFIG_SYS_I2C_EEPROM_ADDR)
int lacie_read_mac_address(uchar *mac);
#endif

#endif /* _LACIE_COMMON_H */
