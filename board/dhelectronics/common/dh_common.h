/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2022 DENX Software Engineering GmbH, Philip Oberfichtner <pro@denx.de>
 */

#define DH_EEPROM_ID_PAGE_MAX_SIZE	64

enum eip_request_values {
	DH_MAC0,
	DH_MAC1,
	DH_ITEM_NUMBER,
	DH_SERIAL_NUMBER,
};

/* DH item: Vendor coding */
#define DH_ITEM_PREFIX_NXP	0x01
#define DH_ITEM_PREFIX_NXP_CHR	'I'
#define DH_ITEM_PREFIX_ST	0x02
#define DH_ITEM_PREFIX_ST_CHR	'S'

/*
 * DH item: Finished state coding
 * Bit = 0 means half finished
 *         Prefix is 'H'
 * Bit = 1 means finished with a customer image flashed
 *         Prefix is 'F'
 */
#define DH_ITEM_PREFIX_FIN_BIT		BIT(7)
#define DH_ITEM_PREFIX_FIN_HALF_CHR	'H'
#define DH_ITEM_PREFIX_FIN_FLASHED_CHR	'F'

struct eeprom_id_page {
	/* Header */
	struct {
		u8	id[3];		/* Identifier 'D', 'H', 'E' - 'D' is at index 0 */
		u8	version;	/* 0x10 -- Version 1.0 */
		u8	crc16_pl[2];	/* Checksum payload, [1] is MSbyte */
		u8	crc8_hdr;	/* Checksum header */
	} hdr;
	/* Payload */
	struct {
		u8	mac0[6];
		u8	mac1[6];
		u8	item_prefix;	/* H/F is coded in MSbits, Vendor coding starts at LSbits */
		u8	item_num[3];	/* [2] is MSbyte */
		u8	serial[9];	/* [8] is MSbyte */
	} pl;
};

#define DH_EEPROM_ID_PAGE_V1_0		0x10

/*
 * dh_mac_is_in_env - Check if MAC address is already set
 *
 * @env: name of environment variable
 * Return: true if MAC is set, false otherwise
 */
bool dh_mac_is_in_env(const char *env);

/*
 * dh_get_mac_is_enabled - Test if ethernet MAC is enabled in DT
 *
 * @alias: alias for ethernet MAC device tree node
 * Return: 0 if OK, other value on error
 */
int dh_get_mac_is_enabled(const char *alias);

/*
 * dh_get_mac_from_eeprom - Get MAC address from eeprom and write it to enetaddr
 *
 * @enetaddr: buffer where address is to be stored
 * @alias: alias for EEPROM device tree node
 * Return: 0 if OK, other value on error
 */
int dh_get_mac_from_eeprom(unsigned char *enetaddr, const char *alias);

/*
 * dh_read_eeprom_id_page() - Read EEPROM ID page content into given buffer
 * @eeprom_buffer:	Buffer for EEPROM ID page content
 * @alias:		Alias for EEPROM ID page device tree node
 *
 * Read the content of the EEPROM ID page into the given buffer (parameter
 * eeprom_buffer). The EEPROM ID page device is selected via alias device
 * tree name (parameter alias). The data of the EEPROM ID page is verified.
 * An error is returned for reading failures and invalid data.
 *
 * Return: 0 if OK, other value on error
 */
int dh_read_eeprom_id_page(u8 *eeprom_buffer, const char *alias);

/*
 * dh_get_value_from_eeprom_buffer() - Get value from EEPROM buffer
 * @eip_request_values:	Requested value as enum
 * @data:		Buffer where value is to be stored
 * @data_len:		Length of the value buffer
 * @eip:		Pointer to EEPROM ID page struct from which the data is parsed
 *
 * Gets the value specified by the parameter eip_request_values from the EEPROM
 * data struct (parameter eip). The data is written to the specified data
 * buffer (parameter data). If the length of the data (parameter data_len) is
 * not sufficient to copy the data into the buffer, an error is returned.
 *
 * Return: 0 if OK, other value on error
 */
int dh_get_value_from_eeprom_buffer(enum eip_request_values request, u8 *data, int data_len,
				    struct eeprom_id_page *eip);

/*
 * dh_setup_mac_address - Try to get MAC address from various locations and write it to env
 *
 * Return: 0 if OK, other value on error
 */
int dh_setup_mac_address(struct eeprom_id_page *eip);
