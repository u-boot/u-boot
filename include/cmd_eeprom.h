/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * EEPROM support
 */
#ifndef	_CMD_EEPROM_H
#define	_CMD_EEPROM_H

#include <common.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_EEPROM)

#ifdef CFG_I2C_MULTI_EEPROMS
#define	CMD_TBL_EEPROM	MK_CMD_TBL_ENTRY(   \
	"eeprom",	3,	6,	1,	do_eeprom,			\
	"eeprom  - EEPROM sub-system\n",					\
	"read  devaddr addr off cnt\n"						\
	"eeprom write devaddr addr off cnt\n"					\
	"       - read/write `cnt' bytes from `devaddr` EEPROM at offset `off'\n" \
),
#else /* One EEPROM */
#define	CMD_TBL_EEPROM	MK_CMD_TBL_ENTRY(					\
	"eeprom",	3,	5,	1,	do_eeprom,			\
	"eeprom  - EEPROM sub-system\n",					\
	"read  addr off cnt\n"							\
	"eeprom write addr off cnt\n"						\
	"       - read/write `cnt' bytes at EEPROM offset `off'\n"		\
),
#endif /* CFG_I2C_MULTI_EEPROMS */
int do_eeprom (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#else

#define CMD_TBL_EEPROM

#endif	/* CFG_CMD_EEPROM */

#endif	/* _CMD_EEPROM_H */
