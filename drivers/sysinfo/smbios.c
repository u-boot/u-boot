// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <dm.h>
#include <smbios_plat.h>
#include <sysinfo.h>

/* platform information storage */
struct processor_info processor_info;
struct cache_info cache_info[SYSINFO_CACHE_LVL_MAX];
struct sysinfo_plat sysinfo_smbios_p = {
	/* Processor Information */
	.processor = &processor_info,
	/* Cache Information */
	.cache = &cache_info[0],
};

/* structure for smbios private data storage */
struct sysinfo_plat_priv {
	struct processor_info *t4;
	struct smbios_type7 t7[SYSINFO_CACHE_LVL_MAX];
	u16 cache_handles[SYSINFO_CACHE_LVL_MAX];
	u8 cache_level;
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
	ci->config.data = SMBIOS_CACHE_LOCATE_UNKNOWN | SMBIOS_CACHE_OP_UND;
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

	switch (id) {
	case SYSID_SM_PROCESSOR_MANUFACT:
		str = priv->t4->manufacturer;
		break;
	default:
		break;
	}

	if (!str)
		return -ENOSYS;

	strlcpy(val, str, size);

	return 0;
}

static int sysinfo_plat_get_int(struct udevice *dev, int id, int *val)
{
	struct sysinfo_plat_priv *priv = dev_get_priv(dev);
	u8 i;

	if (id >= SYSID_SM_CACHE_INFO_START &&
	    id <= SYSID_SM_CACHE_INFO_END) {
		/* For smbios type 7 */
		for (i = 0; i < priv->cache_level; i++) {
			switch (id - i) {
			case SYSID_SM_CACHE_MAX_SIZE:
				*val = priv->t7[i].max_size.data;
				return 0;
			case SYSID_SM_CACHE_INST_SIZE:
				*val = priv->t7[i].inst_size.data;
				return 0;
			case SYSID_SM_CACHE_SCACHE_TYPE:
				*val = priv->t7[i].sys_cache_type;
				return 0;
			case SYSID_SM_CACHE_ASSOC:
				*val = priv->t7[i].associativity;
				return 0;
			case SYSID_SM_CACHE_MAX_SIZE2:
				*val = priv->t7[i].max_size2.data;
				return 0;
			case SYSID_SM_CACHE_INST_SIZE2:
				*val = priv->t7[i].inst_size2.data;
				return 0;
			default:
				break;
			}
		}
		return -ENOSYS;
	}

	switch (id) {
	case SYSID_SM_PROCESSOR_CORE_CNT:
		*val = priv->t4->core_count;
		break;
	case SYSID_SM_PROCESSOR_CORE_EN:
		*val = priv->t4->core_enabled;
		break;
	case SYSID_SM_PROCESSOR_CHARA:
		*val = priv->t4->characteristics;
		break;
	case SYSID_SM_CACHE_LEVEL:
		if (!priv->cache_level)	/* No cache detected */
			return -ENOSYS;
		*val = priv->cache_level - 1;
		break;
	default:
		return -ENOSYS;
	}

	return 0;
}

static int sysinfo_plat_get_data(struct udevice *dev, int id, void **buf,
				 size_t *size)
{
	struct sysinfo_plat_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSID_SM_PROCESSOR_ID:
		*buf = priv->t4->id;
		*size = sizeof(priv->t4->id);
		break;
	case SYSID_SM_CACHE_HANDLE:
		*buf = &priv->cache_handles[0];
		*size = sizeof(priv->cache_handles);
		break;
	default:
		return -EOPNOTSUPP;
	}
	return 0;
}

static int sysinfo_plat_probe(struct udevice *dev)
{
	struct sysinfo_plat_priv *priv = dev_get_priv(dev);
	struct sysinfo_plat *plat = &sysinfo_smbios_p;
	u8 level;

	if (!sysinfo_get_processor_info(plat->processor))
		priv->t4 = plat->processor;

	for (level = 0; level < SYSINFO_CACHE_LVL_MAX; level++) {
		struct cache_info *pcache = plat->cache + level;

		if (sysinfo_get_cache_info(level, pcache))
			break; /* no more levels */

		/*
		 * Fill in the SMBIOS type 7 structure,
		 * skip the header members (type, length, handle),
		 * and the ones in DT smbios node.
		 */
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
		smbios_cache_info_dump(&priv->t7[level]);
	}
	if (!level) /* no cache detected */
		return -ENOSYS;

	priv->cache_level = level;

	return 0;
}

static const struct udevice_id sysinfo_smbios_ids[] = {
	{ .compatible = "u-boot,sysinfo-smbios" },
	{ /* sentinel */ }
};

static const struct sysinfo_ops sysinfo_smbios_ops = {
	.detect = sysinfo_plat_detect,
	.get_str = sysinfo_plat_get_str,
	.get_int = sysinfo_plat_get_int,
	.get_data = sysinfo_plat_get_data,
};

U_BOOT_DRIVER(sysinfo_smbios) = {
	.name           = "sysinfo_smbios",
	.id             = UCLASS_SYSINFO,
	.of_match       = sysinfo_smbios_ids,
	.ops		= &sysinfo_smbios_ops,
	.priv_auto	= sizeof(struct sysinfo_plat_priv),
	.probe		= sysinfo_plat_probe,
};
