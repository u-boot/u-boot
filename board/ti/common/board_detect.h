/*
 * Library to support early TI EVM EEPROM handling
 *
 * Copyright (C) 2015-2016 Texas Instruments Incorporated - http://www.ti.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __BOARD_DETECT_H
#define __BOARD_DETECT_H

/* TI EEPROM MAGIC Header identifier */
#define TI_EEPROM_HEADER_MAGIC	0xEE3355AA
#define TI_DEAD_EEPROM_MAGIC	0xADEAD12C

#define TI_EEPROM_HDR_NAME_LEN		8
#define TI_EEPROM_HDR_REV_LEN		4
#define TI_EEPROM_HDR_SERIAL_LEN	12
#define TI_EEPROM_HDR_CONFIG_LEN	32
#define TI_EEPROM_HDR_NO_OF_MAC_ADDR	3
#define TI_EEPROM_HDR_ETH_ALEN		6

/**
 * struct ti_am_eeprom - This structure holds data read in from the
 *                     AM335x, AM437x, AM57xx TI EVM EEPROMs.
 * @header: This holds the magic number
 * @name: The name of the board
 * @version: Board revision
 * @serial: Board serial number
 * @config: Reserved
 * @mac_addr: Any MAC addresses written in the EEPROM
 *
 * The data is this structure is read from the EEPROM on the board.
 * It is used for board detection which is based on name. It is used
 * to configure specific TI boards. This allows booting of multiple
 * TI boards with a single MLO and u-boot.
 */
struct ti_am_eeprom {
	unsigned int header;
	char name[TI_EEPROM_HDR_NAME_LEN];
	char version[TI_EEPROM_HDR_REV_LEN];
	char serial[TI_EEPROM_HDR_SERIAL_LEN];
	char config[TI_EEPROM_HDR_CONFIG_LEN];
	char mac_addr[TI_EEPROM_HDR_NO_OF_MAC_ADDR][TI_EEPROM_HDR_ETH_ALEN];
} __attribute__ ((__packed__));

/**
 * struct ti_common_eeprom - Null terminated, usable EEPROM contents.
 * header:	Magic number
 * @name:	NULL terminated name
 * @version:	NULL terminated version
 * @serial:	NULL terminated serial number
 * @config:	NULL terminated Board specific config options
 * @mac_addr:	MAC addresses
 */
struct ti_common_eeprom {
	u32 header;
	char name[TI_EEPROM_HDR_NAME_LEN + 1];
	char version[TI_EEPROM_HDR_REV_LEN + 1];
	char serial[TI_EEPROM_HDR_SERIAL_LEN + 1];
	char config[TI_EEPROM_HDR_CONFIG_LEN + 1];
	char mac_addr[TI_EEPROM_HDR_NO_OF_MAC_ADDR][TI_EEPROM_HDR_ETH_ALEN];
};

#define TI_EEPROM_DATA ((struct ti_common_eeprom *)\
				OMAP_SRAM_SCRATCH_BOARD_EEPROM_START)

/**
 * ti_i2c_eeprom_am_get() - Consolidated eeprom data collection for AM* TI EVMs
 * @bus_addr:	I2C bus address
 * @dev_addr:	I2C slave address
 *
 * ep in SRAM is populated by the this AM generic function that consolidates
 * the basic initialization logic common across all AM* platforms.
 */
int ti_i2c_eeprom_am_get(int bus_addr, int dev_addr);

/**
 * board_ti_is() - Board detection logic for TI EVMs
 * @name_tag:	Tag used in eeprom for the board
 *
 * Return: false if board information does not match OR eeprom wasn't read.
 *	   true otherwise
 */
bool board_ti_is(char *name_tag);

/**
 * board_ti_rev_is() - Compare board revision for TI EVMs
 * @rev_tag:	Revision tag to check in eeprom
 * @cmp_len:	How many chars to compare?
 *
 * NOTE: revision information is often messed up (hence the str len match) :(
 *
 * Return: false if board information does not match OR eeprom was'nt read.
 *	   true otherwise
 */
bool board_ti_rev_is(char *rev_tag, int cmp_len);

/**
 * board_ti_get_rev() - Get board revision for TI EVMs
 *
 * Return: NULL if eeprom was'nt read.
 *	   Board revision otherwise
 */
char *board_ti_get_rev(void);

/**
 * board_ti_get_config() - Get board config for TI EVMs
 *
 * Return: NULL if eeprom was'nt read.
 *	   Board config otherwise
 */
char *board_ti_get_config(void);

/**
 * board_ti_get_name() - Get board name for TI EVMs
 *
 * Return: NULL if eeprom was'nt read.
 *	   Board name otherwise
 */
char *board_ti_get_name(void);

/**
 * board_ti_get_eth_mac_addr() - Get Ethernet MAC address from EEPROM MAC list
 * @index:	0 based index within the list of MAC addresses
 * @mac_addr:	MAC address contained at the index is returned here
 *
 * Does not sanity check the mac_addr. Whatever is stored in EEPROM is returned.
 */
void board_ti_get_eth_mac_addr(int index, u8 mac_addr[TI_EEPROM_HDR_ETH_ALEN]);

/**
 * set_board_info_env() - Setup commonly used board information environment vars
 * @name:	Name of the board
 *
 * If name is NULL, default_name is used.
 */
void set_board_info_env(char *name);

#endif	/* __BOARD_DETECT_H */
