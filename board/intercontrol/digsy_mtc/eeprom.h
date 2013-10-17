/*
 * (C) Copyright 2009 Semihalf.
 * Written by: Grzegorz Bernacki <gjb@semihalf.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef CMD_EEPROM_H
#define CMD_EEPROM_H

#define EEPROM_ADDR		CONFIG_SYS_I2C_EEPROM_ADDR
#define EEPROM_LEN		1024	/* eeprom length */
#define EEPROM_IDENT		2408	/* identification word */
#define EEPROM_ADDR_IDENT	0	/* identification word offset */
#define EEPROM_ADDR_LEN_SYS	2	/* system area lenght offset */
#define EEPROM_ADDR_LEN_SYSCFG	4	/* system config area length offset */
#define EEPROM_ADDR_ETHADDR	23	/* ethernet address offset */

#endif
