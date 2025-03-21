// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Siemens AG, 2025
 */

#include <dm.h>
#include <sysinfo.h>
#include <net.h>
#include <u-boot/uuid.h>
#include <asm/arch/hardware.h>

#include "iot2050.h"

#define IOT2050_INFO_MAGIC		0x20502050

#define IOT2050_UUID_STR_LEN		(32)

struct iot2050_info {
	u32 magic;
	u16 size;
	char name[20 + 1];
	char serial[16 + 1];
	char mlfb[18 + 1];
	char uuid[IOT2050_UUID_STR_LEN + 1];
	char a5e[18 + 1];
	u8 mac_addr_cnt;
	u8 mac_addr[8][ARP_HLEN];
	char seboot_version[40 + 1];
	u8 padding[3];
	u32 ddr_size_mb;
} __packed;

/**
 * struct sysinfo_iot2050_priv - sysinfo private data
 * @info: iot2050 board info
 */
struct sysinfo_iot2050_priv {
	struct iot2050_info *info;
	u8 uuid_smbios[16];
};

static int sysinfo_iot2050_detect(struct udevice *dev)
{
	struct sysinfo_iot2050_priv *priv = dev_get_priv(dev);

	if (!priv->info || priv->info->magic != IOT2050_INFO_MAGIC)
		return -EFAULT;

	return 0;
}

static int sysinfo_iot2050_get_str(struct udevice *dev, int id, size_t size,
				   char *val)
{
	struct sysinfo_iot2050_priv *priv = dev_get_priv(dev);

	switch (id) {
	case BOARD_NAME:
	case SYSID_SM_BASEBOARD_VERSION:
		strlcpy(val, priv->info->name, size);
		break;
	case SYSID_SM_SYSTEM_SERIAL:
		strlcpy(val, priv->info->serial, size);
		break;
	case BOARD_MLFB:
	case SYSID_SM_SYSTEM_VERSION:
		strlcpy(val, priv->info->mlfb, size);
		break;
	case BOARD_UUID:
		strlcpy(val, priv->info->uuid, size);
		break;
	case BOARD_A5E:
	case SYSID_SM_BASEBOARD_PRODUCT:
		strlcpy(val, priv->info->a5e, size);
		break;
	case BOARD_SEBOOT_VER:
	case SYSID_PRIOR_STAGE_VERSION:
		strlcpy(val, priv->info->seboot_version, size);
		break;
	default:
		return -EINVAL;
	};

	val[size - 1] = '\0';
	return 0;
}

static int sysinfo_iot2050_get_int(struct udevice *dev, int id, int *val)
{
	struct sysinfo_iot2050_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSID_BOARD_RAM_SIZE_MB:
		*val = priv->info->ddr_size_mb;
		return 0;
	default:
		return -EINVAL;
	};
}

static int sysinfo_iot2050_get_data(struct udevice *dev, int id, void **data,
				    size_t *size)
{
	struct sysinfo_iot2050_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSID_SM_SYSTEM_UUID:
		*data = priv->uuid_smbios;
		*size = 16;
		return 0;
	default:
		return -EINVAL;
	};
}

static int sysinfo_iot2050_get_item_count(struct udevice *dev, int id)
{
	struct sysinfo_iot2050_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSID_BOARD_MAC_ADDR:
		return priv->info->mac_addr_cnt;
	default:
		return -EINVAL;
	};
}

static int sysinfo_iot2050_get_data_by_index(struct udevice *dev, int id,
					     int index, void **data,
					     size_t *size)
{
	struct sysinfo_iot2050_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSID_BOARD_MAC_ADDR:
		if (index >= priv->info->mac_addr_cnt)
			return -EINVAL;
		*data = priv->info->mac_addr[index];
		*size = ARP_HLEN;
		return 0;
	default:
		return -EINVAL;
	};
}

static const struct sysinfo_ops sysinfo_iot2050_ops = {
	.detect = sysinfo_iot2050_detect,
	.get_str = sysinfo_iot2050_get_str,
	.get_int = sysinfo_iot2050_get_int,
	.get_data = sysinfo_iot2050_get_data,
	.get_item_count = sysinfo_iot2050_get_item_count,
	.get_data_by_index = sysinfo_iot2050_get_data_by_index,
};

/**
 * @brief Convert the IOT2050 UUID string to the SMBIOS format
 *
 * @param uuid_raw The IOT2050 UUID string parsed from the eeprom
 * @param uuid_smbios The buffer to hold the SMBIOS formatted UUID
 */
static void sysinfo_iot2050_convert_uuid(const char *uuid_iot2050,
					 u8 *uuid_smbios)
{
	char uuid_rfc4122_str[IOT2050_UUID_STR_LEN + 4 + 1] = {0};
	char *tmp = uuid_rfc4122_str;

	for (int i = 0; i < 16; i++) {
		memcpy(tmp, uuid_iot2050 + i * 2, 2);
		tmp += 2;
		if (i == 3 || i == 5 || i == 7 || i == 9)
			*tmp++ = '-';
	}
	uuid_str_to_bin(uuid_rfc4122_str, uuid_smbios, UUID_STR_FORMAT_GUID);
}

static int sysinfo_iot2050_probe(struct udevice *dev)
{
	struct sysinfo_iot2050_priv *priv = dev_get_priv(dev);
	unsigned long offset;

	offset = dev_read_u32_default(dev, "offset",
				      TI_SRAM_SCRATCH_BOARD_EEPROM_START);
	priv->info = (struct iot2050_info *)offset;

	sysinfo_iot2050_convert_uuid(priv->info->uuid, priv->uuid_smbios);

	return 0;
}

static const struct udevice_id sysinfo_iot2050_ids[] = {
	{ .compatible = "siemens,sysinfo-iot2050" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sysinfo_iot2050) = {
	.name           = "sysinfo_iot2050",
	.id             = UCLASS_SYSINFO,
	.of_match       = sysinfo_iot2050_ids,
	.ops		= &sysinfo_iot2050_ops,
	.priv_auto	= sizeof(struct sysinfo_iot2050_priv),
	.probe          = sysinfo_iot2050_probe,
};
