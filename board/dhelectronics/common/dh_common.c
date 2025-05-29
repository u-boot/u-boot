// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 * Copyright 2022 DENX Software Engineering GmbH, Philip Oberfichtner <pro@denx.de>
 */

#include <dm.h>
#include <env.h>
#include <i2c_eeprom.h>
#include <net.h>
#include <u-boot/crc.h>

#include "dh_common.h"

static int on_dh_som_serial_number(const char *name, const char *value, enum env_op op,
				   int flags)
{
	env_set("SN", value);
	return 0;
}

U_BOOT_ENV_CALLBACK(dh_som_serial_number, on_dh_som_serial_number);

static int on_SN(const char *name, const char *value, enum env_op op, int flags)
{
	env_set("dh_som_serial_number", value);
	return 0;
}

U_BOOT_ENV_CALLBACK(SN, on_SN);

bool dh_mac_is_in_env(const char *env)
{
	unsigned char enetaddr[6];

	return eth_env_get_enetaddr(env, enetaddr);
}

int dh_get_mac_is_enabled(const char *alias)
{
	ofnode node = ofnode_path(alias);

	if (!ofnode_valid(node))
		return -EINVAL;

	if (!ofnode_is_enabled(node))
		return -ENODEV;

	return 0;
}

int dh_read_eeprom_id_page(u8 *eeprom_buffer, const char *alias)
{
	struct eeprom_id_page *eip = (struct eeprom_id_page *)eeprom_buffer;
	struct udevice *dev;
	size_t payload_len;
	int eeprom_size;
	u16 crc16_calc;
	u16 crc16_eip;
	u8 crc8_calc;
	ofnode node;
	int ret;

	node = ofnode_path(alias);

	ret = uclass_get_device_by_ofnode(UCLASS_I2C_EEPROM, node, &dev);
	if (ret)
		return ret;

	eeprom_size = i2c_eeprom_size(dev);
	if (eeprom_size < 0) {
		printf("%s: Error getting EEPROM ID page size! ret = %d\n", __func__, ret);
		return eeprom_size;
	}

	if (eeprom_size == 0 || eeprom_size > DH_EEPROM_ID_PAGE_MAX_SIZE) {
		eeprom_size = DH_EEPROM_ID_PAGE_MAX_SIZE;
		printf("Get invalid EEPROM ID page size %d bytes! Try to read %d bytes.\n",
		       eeprom_size, DH_EEPROM_ID_PAGE_MAX_SIZE);
	}

	ret = i2c_eeprom_read(dev, 0x0, eeprom_buffer, eeprom_size);
	if (ret) {
		printf("%s: Error reading EEPROM ID page! ret = %d\n", __func__, ret);
		return ret;
	}

	/* Validate header ID */
	if (eip->hdr.id[0] != 'D' || eip->hdr.id[1] != 'H' || eip->hdr.id[2] != 'E') {
		printf("%s: Error validating header ID! (got %c%c%c (0x%02x 0x%02x 0x%02x) != expected DHE)\n",
		       __func__, isprint(eip->hdr.id[0]) ? eip->hdr.id[0] : '.',
		       isprint(eip->hdr.id[1]) ? eip->hdr.id[1] : '.',
		       isprint(eip->hdr.id[2]) ? eip->hdr.id[2] : '.',
		       eip->hdr.id[0], eip->hdr.id[1], eip->hdr.id[2]);
		return -EINVAL;
	}

	/* Validate header checksum */
	crc8_calc = crc8(0xff, eeprom_buffer, offsetof(struct eeprom_id_page, hdr.crc8_hdr));
	if (eip->hdr.crc8_hdr != crc8_calc) {
		printf("%s: Error validating header checksum! (got 0x%02x != calc 0x%02x)\n",
		       __func__, eip->hdr.crc8_hdr, crc8_calc);
		return -EINVAL;
	}

	/*
	 * Validate header version
	 * The payload is defined by the version specified in the header.
	 * Currently only version 0x10 is defined, so take the length of
	 * the only defined payload as the payload length.
	 */
	if (eip->hdr.version != DH_EEPROM_ID_PAGE_V1_0) {
		printf("%s: Error validating version! (0x%02X is not supported)\n",
		       __func__, eip->hdr.version);
		return -EINVAL;
	}
	payload_len = sizeof(eip->pl);

	/* Validate payload checksum */
	crc16_eip = (eip->hdr.crc16_pl[1] << 8) | eip->hdr.crc16_pl[0];
	crc16_calc = crc16(0xffff, eeprom_buffer + sizeof(eip->hdr), payload_len);
	if (crc16_eip != crc16_calc) {
		printf("%s: Error validating data checksum! (got 0x%02x != calc 0x%02x)\n",
		       __func__, crc16_eip, crc16_calc);
		return -EINVAL;
	}

	return 0;
}

int dh_get_value_from_eeprom_buffer(enum eip_request_values request, u8 *data, int data_len,
				    struct eeprom_id_page *eip)
{
	const char fin_chr = (eip->pl.item_prefix & DH_ITEM_PREFIX_FIN_BIT) ?
			     DH_ITEM_PREFIX_FIN_FLASHED_CHR : DH_ITEM_PREFIX_FIN_HALF_CHR;
	const u8 soc_coded = eip->pl.item_prefix & 0xf;
	char soc_chr;

	if (!eip)
		return -EINVAL;

	/* Copy requested data */
	switch (request) {
	case DH_MAC0:
		if (!is_valid_ethaddr(eip->pl.mac0))
			return -EINVAL;

		if (data_len >= sizeof(eip->pl.mac0))
			memcpy(data, eip->pl.mac0, sizeof(eip->pl.mac0));
		else
			return -EINVAL;
		break;
	case DH_MAC1:
		if (!is_valid_ethaddr(eip->pl.mac1))
			return -EINVAL;

		if (data_len >= sizeof(eip->pl.mac1))
			memcpy(data, eip->pl.mac1, sizeof(eip->pl.mac1));
		else
			return -EINVAL;
		break;
	case DH_ITEM_NUMBER:
		if (data_len < 8) /* String length must be 7 characters + string termination */
			return -EINVAL;

		if (soc_coded == DH_ITEM_PREFIX_NXP)
			soc_chr = DH_ITEM_PREFIX_NXP_CHR;
		else if (soc_coded == DH_ITEM_PREFIX_ST)
			soc_chr = DH_ITEM_PREFIX_ST_CHR;
		else
			return -EINVAL;

		snprintf(data, data_len, "%c%c%05d", fin_chr, soc_chr,
			 (eip->pl.item_num[0] << 16) | (eip->pl.item_num[1] << 8) |
			 eip->pl.item_num[2]);
		break;
	case DH_SERIAL_NUMBER:
		/*
		 * data_len must be greater than the size of eip->pl.serial,
		 * because there is a string termination needed.
		 */
		if (data_len <= sizeof(eip->pl.serial))
			return -EINVAL;

		data[sizeof(eip->pl.serial)] = 0;
		memcpy(data, eip->pl.serial, sizeof(eip->pl.serial));
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int dh_get_mac_from_eeprom(unsigned char *enetaddr, const char *alias)
{
	struct udevice *dev;
	int ret;
	ofnode node;

	node = ofnode_path(alias);
	if (!ofnode_valid(node)) {
		printf("%s: ofnode for %s not found!", __func__, alias);
		return -ENOENT;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_I2C_EEPROM, node, &dev);
	if (ret) {
		printf("%s: Cannot find EEPROM! ret = %d\n", __func__, ret);
		return ret;
	}

	ret = i2c_eeprom_read(dev, 0xfa, enetaddr, 0x6);
	if (ret) {
		printf("%s: Error reading EEPROM! ret = %d\n", __func__, ret);
		return ret;
	}

	if (!is_valid_ethaddr(enetaddr)) {
		printf("%s: Address read from EEPROM is invalid!\n", __func__);
		return -EINVAL;
	}

	return 0;
}

__weak int dh_setup_mac_address(struct eeprom_id_page *eip)
{
	unsigned char enetaddr[6];

	if (dh_mac_is_in_env("ethaddr"))
		return 0;

	if (dh_get_mac_is_enabled("ethernet0"))
		return 0;

	if (!dh_get_value_from_eeprom_buffer(DH_MAC0, enetaddr, sizeof(enetaddr), eip))
		return eth_env_set_enetaddr("ethaddr", enetaddr);

	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom0"))
		return eth_env_set_enetaddr("ethaddr", enetaddr);

	printf("%s: Unable to set mac address!\n", __func__);
	return -ENXIO;
}
