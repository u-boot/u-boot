/*
 * (C) Copyright 2009 Semihalf.
 * Written by: Grzegorz Bernacki <gjb@semihalf.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the anty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#ifndef CMD_EEPROM_H
#define CMD_EEPROM_H

#define EEPROM_ADDR		CONFIG_SYS_I2C_EEPROM_ADDR
#define EEPROM_LEN		1024	/* eeprom length */
#define EEPROM_IDENT		2408	/* identification word */
#define EEPROM_ADDR_IDENT	0	/* identification word offset */
#define EEPROM_ADDR_LEN_SYS	2	/* system area lenght offset */
#define EEPROM_ADDR_LEN_SYSCFG	4	/* system config area length offset */
#define EEPROM_ADDR_ETHADDR	23	/* ethernet addres offset */

#endif
