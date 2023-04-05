/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _BOARD_CSSI_COMMON_H
#define _BOARD_CSSI_COMMON_H

void ft_cleanup(void *blob, unsigned long id, const char *prop, const char *compatible);
int read_eeprom(u8 *din, int len);

#endif /* _BOARD_CSSI_COMMON_H */
