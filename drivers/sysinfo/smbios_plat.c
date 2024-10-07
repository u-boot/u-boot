// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#include <dm.h>
#include <smbios_plat.h>
#include <sysinfo.h>

struct sysinfo_plat_priv {
	struct sys_info *t1;
	struct baseboard_info *t2;
	struct enclosure_info *t3;
	struct processor_info *t4;
	struct smbios_type7 t7[SYSINFO_CACHE_LVL_MAX];
	char *cache_socket_design[SYSINFO_CACHE_LVL_MAX];
	u16 cache_handles[SYSINFO_CACHE_LVL_MAX];
	u8 cache_level;
	/*
	 * TODO: add other types here:
	 * Type 9 - System Slots
	 * Type 16 - Physical Memory Array
	 * Type 17 - Memory Device
	 * Type 19 - Memory Array Mapped Address
	 */
};

static void smbios_cache_info_dump(struct smbios_type7 *cache_info)
{
	log_debug("SMBIOS Type 7 (Cache Information):\n");
	log_debug("Cache Configuration: 0x%04x\n", cache_info->config.data);
	log_debug("Maximum Cache Size: %u KB\n", cache_info->max_size.data);
	log_debug("Installed Size: %u KB\n", cache_info->inst_size.data);
	log_debug("Supported SRAM Type: 0x%04x\n",
		  cache_info->supp_sram_type.data);
	log_debug("Current SRAM Type: 0x%04x\n",
		  cache_info->curr_sram_type.data);
	log_debug("Cache Speed: %u\n", cache_info->speed);
	log_debug("Error Correction Type: %u\n", cache_info->err_corr_type);
	log_debug("System Cache Type: %u\n", cache_info->sys_cache_type);
	log_debug("Associativity: %u\n", cache_info->associativity);
	log_debug("Maximum Cache Size 2: %u KB\n", cache_info->max_size2.data);
	log_debug("Installed Cache Size 2: %u KB\n",
		  cache_info->inst_size2.data);
}

/* weak function for the platforms not yet supported */
__weak int sysinfo_get_cache_info(u8 level, struct cache_info *cache_info)
{
	return -ENOSYS;
}
__weak int sysinfo_get_processor_info(struct processor_info *pinfo)
{
	return -ENOSYS;
}

void sysinfo_cache_info_default(struct cache_info *ci)
{
	memset(ci, 0, sizeof(*ci));
	ci->config.fields.locate = SMBIOS_CACHE_LOCATE_UNKNOWN;
	ci->config.fields.opmode = SMBIOS_CACHE_OP_UND;
	ci->supp_sram_type.fields.unknown = 1;
	ci->curr_sram_type.fields.unknown = 1;
	ci->speed = SMBIOS_CACHE_SPEED_UNKNOWN;
	ci->err_corr_type = SMBIOS_CACHE_ERRCORR_UNKNOWN;
	ci->cache_type = SMBIOS_CACHE_SYSCACHE_TYPE_UNKNOWN;
}

static int sysinfo_plat_detect(struct udevice *dev)
{
	return 0;
}

static int sysinfo_plat_get_str(struct udevice *dev, int id,
				size_t size, char *val)
{
	struct sysinfo_plat_priv *priv = dev_get_priv(dev);
	const char *str = NULL;
	u8 i;

	if (id >= SYSINFO_ID_SMBIOS_CACHE_INFO_START &&
	    id <= SYSINFO_ID_SMBIOS_CACHE_INFO_END) {
		/* For smbios type 7 */
		for (i = 0; i < priv->cache_level; i++) {
			switch (id - i) {
			case SYSINFO_ID_SMBIOS_CACHE_SOCKET:
				str = priv->cache_socket_design[i];
				break;
			default:
				break;
			}
		}
		goto handle_str;
	}

	switch (id) {
	case SYSINFO_ID_SMBIOS_SYSTEM_MANUFACTURER:
		str = priv->t1->manufacturer;
		break;
	case SYSINFO_ID_SMBIOS_SYSTEM_PRODUCT:
		str = priv->t1->prod_name;
		break;
	case SYSINFO_ID_SMBIOS_SYSTEM_VERSION:
		str = priv->t1->version;
		break;
	case SYSINFO_ID_SMBIOS_SYSTEM_SERIAL:
		str = priv->t1->sn;
		break;
	case SYSINFO_ID_SMBIOS_SYSTEM_SKU:
		str = priv->t1->sku_num;
		break;
	case SYSINFO_ID_SMBIOS_SYSTEM_FAMILY:
		str = priv->t1->family;
		break;
	case SYSINFO_ID_SMBIOS_BASEBOARD_MANUFACTURER:
		str = priv->t2->manufacturer;
		break;
	case SYSINFO_ID_SMBIOS_BASEBOARD_PRODUCT:
		str = priv->t2->prod_name;
		break;
	case SYSINFO_ID_SMBIOS_BASEBOARD_VERSION:
		str = priv->t2->version;
		break;
	case SYSINFO_ID_SMBIOS_BASEBOARD_SERIAL:
		str = priv->t2->sn;
		break;
	case SYSINFO_ID_SMBIOS_BASEBOARD_ASSET_TAG:
		str = priv->t2->asset_tag;
		break;
	case SYSINFO_ID_SMBIOS_BASEBOARD_CHASSIS_LOCAT:
		str = priv->t2->chassis_locat;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_MANUFACTURER:
		str = priv->t3->manufacturer;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_VERSION:
		str = priv->t3->version;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_SERIAL:
		str = priv->t3->sn;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_ASSET_TAG:
		str = priv->t3->asset_tag;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_SKU:
		str = priv->t3->sku_num;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_SOCKET:
		str = priv->t4->socket_design;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_MANUFACT:
		str = priv->t4->manufacturer;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_VERSION:
		str = priv->t4->version;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_SN:
		str = priv->t4->sn;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_ASSET_TAG:
		str = priv->t4->asset_tag;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_PN:
		str = priv->t4->pn;
		break;
	default:
		break;
	}

handle_str:
	if (!str)
		return -ENOSYS;

	strlcpy(val, str, size);

	return 0;
}

static int sysinfo_plat_get_int(struct udevice *dev, int id, int *val)
{
	struct sysinfo_plat_priv *priv = dev_get_priv(dev);
	u8 i;

	if (id >= SYSINFO_ID_SMBIOS_CACHE_INFO_START &&
	    id <= SYSINFO_ID_SMBIOS_CACHE_INFO_END) {
		/* For smbios type 7 */
		for (i = 0; i < priv->cache_level; i++) {
			switch (id - i) {
			case SYSINFO_ID_SMBIOS_CACHE_CONFIG:
				*val = priv->t7[i].config.data;
				break;
			case SYSINFO_ID_SMBIOS_CACHE_MAX_SIZE:
				*val = priv->t7[i].max_size.data;
				break;
			case SYSINFO_ID_SMBIOS_CACHE_INST_SIZE:
				*val = priv->t7[i].inst_size.data;
				break;
			case SYSINFO_ID_SMBIOS_CACHE_SUPSRAM_TYPE:
				*val = priv->t7[i].supp_sram_type.data;
				break;
			case SYSINFO_ID_SMBIOS_CACHE_CURSRAM_TYPE:
				*val = priv->t7[i].curr_sram_type.data;
				break;
			case SYSINFO_ID_SMBIOS_CACHE_SPEED:
				*val = priv->t7[i].speed;
				break;
			case SYSINFO_ID_SMBIOS_CACHE_ERRCOR_TYPE:
				*val = priv->t7[i].err_corr_type;
				break;
			case SYSINFO_ID_SMBIOS_CACHE_SCACHE_TYPE:
				*val = priv->t7[i].sys_cache_type;
				break;
			case SYSINFO_ID_SMBIOS_CACHE_ASSOC:
				*val = priv->t7[i].associativity;
				break;
			case SYSINFO_ID_SMBIOS_CACHE_MAX_SIZE2:
				*val = priv->t7[i].max_size2.data;
				break;
			case SYSINFO_ID_SMBIOS_CACHE_INST_SIZE2:
				*val = priv->t7[i].inst_size2.data;
				break;
			default:
				break;
			}
		}
		return 0;
	}

	switch (id) {
	case SYSINFO_ID_SMBIOS_SYSTEM_WAKEUP:
		*val = priv->t1->wakeup_type;
		break;
	case SYSINFO_ID_SMBIOS_BASEBOARD_FEATURE:
		*val = priv->t2->feature.data;
		break;
	case SYSINFO_ID_SMBIOS_BASEBOARD_TYPE:
		*val = priv->t2->type;
		break;
	case SYSINFO_ID_SMBIOS_BASEBOARD_OBJS_NUM:
		*val = priv->t2->objs_num;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_TYPE:
		*val = priv->t3->chassis_type;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_BOOTUP:
		*val = priv->t3->bootup_state;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_POW:
		*val = priv->t3->power_supply_state;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_THERMAL:
		*val = priv->t3->thermal_state;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_SECURITY:
		*val = priv->t3->security_status;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_OEM:
		*val = priv->t3->oem_defined;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_HEIGHT:
		*val = priv->t3->height;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_POWCORE_NUM:
		*val = priv->t3->number_of_power_cords;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_ELEMENT_CNT:
		*val = priv->t3->element_count;
		break;
	case SYSINFO_ID_SMBIOS_ENCLOSURE_ELEMENT_LEN:
		*val = priv->t3->element_record_length;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_TYPE:
		*val = priv->t4->type;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_VOLTAGE:
		*val = priv->t4->voltage;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_EXT_CLOCK:
		*val = priv->t4->ext_clock;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_MAX_SPEED:
		*val = priv->t4->max_speed;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_CUR_SPEED:
		*val = priv->t4->curr_speed;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_STATUS:
		*val = priv->t4->status;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_UPGRADE:
		*val = priv->t4->upgrade;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_CORE_CNT:
		*val = priv->t4->core_count;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_CORE_EN:
		*val = priv->t4->core_enabled;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_THREAD_CNT:
		*val = priv->t4->thread_count;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_CHARA:
		*val = priv->t4->characteristics;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_FAMILY2:
		*val = priv->t4->family2;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_CORE_CNT2:
		*val = priv->t4->core_count2;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_CORE_EN2:
		*val = priv->t4->core_enabled2;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_THREAD_CNT2:
		*val = priv->t4->thread_count2;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_THREAD_EN:
		*val = priv->t4->thread_enabled;
		break;
	case SYSINFO_ID_SMBIOS_CACHE_LEVEL:
		if (!priv->cache_level)	/* No cache detected */
			return -ENOSYS;
		*val = priv->cache_level - 1;
		break;
	default:
		break;
	}

	return 0;
}

static int sysinfo_plat_get_data(struct udevice *dev, int id, uchar **buf,
				 size_t *size)
{
	struct sysinfo_plat_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSINFO_ID_SMBIOS_ENCLOSURE_ELEMENTS:
		*buf = priv->t3->elements;
		*size = priv->t3->elements_size;
		break;
	case SYSINFO_ID_SMBIOS_BASEBOARD_OBJS_HANDLE:
		*buf = priv->t2->objs;
		*size = priv->t2->objs_size;
		break;
	case SYSINFO_ID_SMBIOS_PROCESSOR_ID:
		*buf = (uchar *)priv->t4->id;
		*size = sizeof(priv->t4->id);
		break;
	case SYSINFO_ID_SMBIOS_CACHE_HANDLE:
		*buf = (uchar *)(&priv->cache_handles[0]);
		*size = sizeof(priv->cache_handles);
		break;
	default:
		break;
	}
	return 0;
}

static int sysinfo_plat_probe(struct udevice *dev)
{
	struct sysinfo_plat_priv *priv = dev_get_priv(dev);
	struct sysinfo_plat *plat = dev_get_plat(dev);
	u8 level;

	priv->t1 = &plat->sys;
	priv->t2 = &plat->board;
	priv->t3 = &plat->chassis;

	if (!sysinfo_get_processor_info(plat->processor))
		priv->t4 = plat->processor;

	for (level = 0; level < SYSINFO_CACHE_LVL_MAX; level++) {
		struct cache_info *pcache = plat->cache + level;

		if (sysinfo_get_cache_info(level, pcache))
			break; /* no more levels */

		/*
		 * Fill in the SMBIOS type 7 structure,
		 * skip the header members - type, length, handle
		 */
		priv->t7[level].config.data = pcache->config.data;
		priv->t7[level].supp_sram_type.data =
			pcache->supp_sram_type.data;
		priv->t7[level].curr_sram_type.data =
			pcache->curr_sram_type.data;
		priv->t7[level].speed = pcache->speed;
		priv->t7[level].err_corr_type = pcache->err_corr_type;
		priv->t7[level].sys_cache_type = pcache->cache_type;
		priv->t7[level].associativity = pcache->associativity;

		if (pcache->max_size > SMBIOS_CACHE_SIZE_EXT_KB) {
			priv->t7[level].max_size.data = 0xFFFF;
			priv->t7[level].max_size2.fields.size =
				pcache->max_size / 64;
			priv->t7[level].max_size2.fields.granu =
				SMBIOS_CACHE_GRANU_64K;
		} else {
			priv->t7[level].max_size.fields.size = pcache->max_size;
			priv->t7[level].max_size.fields.granu =
				SMBIOS_CACHE_GRANU_1K;
			priv->t7[level].max_size2.data = 0;
		}
		if (pcache->inst_size > SMBIOS_CACHE_SIZE_EXT_KB) {
			priv->t7[level].inst_size.data = 0xFFFF;
			priv->t7[level].inst_size2.fields.size =
				pcache->inst_size / 64;
			priv->t7[level].inst_size2.fields.granu =
				SMBIOS_CACHE_GRANU_64K;
		} else {
			priv->t7[level].inst_size.fields.size =
				pcache->inst_size;
			priv->t7[level].inst_size.fields.granu =
				SMBIOS_CACHE_GRANU_1K;
			priv->t7[level].inst_size2.data = 0;
		}
		priv->cache_socket_design[level] = pcache->socket_design;
		smbios_cache_info_dump(&priv->t7[level]);
	}
	if (!level) /* no cache detected */
		return -ENOSYS;

	priv->cache_level = level;

	return 0;
}

static const struct sysinfo_ops sysinfo_smbios_ops = {
	.detect = sysinfo_plat_detect,
	.get_str = sysinfo_plat_get_str,
	.get_int = sysinfo_plat_get_int,
	.get_data = sysinfo_plat_get_data,
};

U_BOOT_DRIVER(sysinfo_smbios_plat) = {
	.name           = "sysinfo_smbios_plat",
	.id             = UCLASS_SYSINFO,
	.ops		= &sysinfo_smbios_ops,
	.priv_auto	= sizeof(struct sysinfo_plat_priv),
	.probe		= sysinfo_plat_probe,
};
