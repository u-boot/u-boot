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
};

/* weak function for the platforms not yet supported */
__weak int sysinfo_get_processor_info(struct processor_info *pinfo)
{
	return -ENOSYS;
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

	if (!str)
		return -ENOSYS;

	strlcpy(val, str, size);

	return 0;
}

static int sysinfo_plat_get_int(struct udevice *dev, int id, int *val)
{
	struct sysinfo_plat_priv *priv = dev_get_priv(dev);

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
	default:
		break;
	}
	return 0;
}

static int sysinfo_plat_probe(struct udevice *dev)
{
	struct sysinfo_plat_priv *priv = dev_get_priv(dev);
	struct sysinfo_plat *plat = dev_get_plat(dev);

	priv->t1 = &plat->sys;
	priv->t2 = &plat->board;
	priv->t3 = &plat->chassis;

	if (!sysinfo_get_processor_info(plat->processor))
		priv->t4 = plat->processor;

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
