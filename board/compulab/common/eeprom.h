/*
 * (C) Copyright 2011 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _EEPROM_
#define _EEPROM_

#ifdef CONFIG_SYS_I2C_OMAP34XX
int cl_eeprom_read_mac_addr(uchar *buf);
u32 cl_eeprom_get_board_rev(void);
#else
static inline int cl_eeprom_read_mac_addr(uchar *buf)
{
	return 1;
}
static inline u32 cl_eeprom_get_board_rev(void)
{
	return 0;
}
#endif

#endif
