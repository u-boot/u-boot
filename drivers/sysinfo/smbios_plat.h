/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#ifndef __SMBIOS_PLAT_H
#define __SMBIOS_PLAT_H

#include <smbios.h>

/*
 * TODO:
 * sysinfo_plat and all sub data structure should be moved to <asm/sysinfo.h>
 * if we have this defined for each arch.
 */
struct __packed cache_info {
	char *socket_design;
	union cache_config config;
	u32 line_size;
	u32 associativity;
	u32 max_size;
	u32 inst_size;
	u8 cache_type;
	union cache_sram_type supp_sram_type;
	union cache_sram_type curr_sram_type;
	u8 speed;
	u8 err_corr_type;
};

struct __packed sys_info {
	char *manufacturer;
	char *prod_name;
	char *version;
	char *sn;
	u8 wakeup_type;
	char *sku_num;
	char *family;
};

struct __packed baseboard_info {
	char *manufacturer;
	char *prod_name;
	char *version;
	char *sn;
	char *asset_tag;
	union baseboard_feat feature;
	char *chassis_locat;
	u8 type;
	u8 objs_num;
	void *objs;
	size_t objs_size;
};

struct __packed enclosure_info {
	char *manufacturer;
	char *version;
	char *sn;
	char *asset_tag;
	u8 chassis_type;
	u8 bootup_state;
	u8 power_supply_state;
	u8 thermal_state;
	u8 security_status;
	u32 oem_defined;
	u8 height;
	u8 number_of_power_cords;
	u8 element_count;
	u8 element_record_length;
	void *elements;
	size_t elements_size;
	char *sku_num;
};

struct __packed processor_info {
	char *socket_design;
	u8 type;
	u8 family;
	char *manufacturer;
	u32 id[2];
	char *version;
	u8 voltage;
	u16 ext_clock;
	u16 max_speed;
	u16 curr_speed;
	u8 status;
	u8 upgrade;
	char *sn;
	char *asset_tag;
	char *pn;
	u8 core_count;
	u8 core_enabled;
	u8 thread_count;
	u16 characteristics;
	u16 family2;
	u16 core_count2;
	u16 core_enabled2;
	u16 thread_count2;
	u16 thread_enabled;
};

struct sysinfo_plat {
	struct sys_info sys;
	struct baseboard_info board;
	struct enclosure_info chassis;
	struct processor_info *processor;
	struct cache_info *cache;
	/* add other sysinfo structure here */
};

#if CONFIG_IS_ENABLED(SYSINFO_SMBIOS)
int sysinfo_get_cache_info(u8 level, struct cache_info *cache_info);
void sysinfo_cache_info_default(struct cache_info *ci);
int sysinfo_get_processor_info(struct processor_info *pinfo);
#else
static inline int sysinfo_get_cache_info(u8 level,
					 struct cache_info *cache_info)
{
	return -ENOSYS;
}

static inline void sysinfo_cache_info_default(struct cache_info *ci)
{
}

static inline int sysinfo_get_processor_info(struct processor_info *pinfo)
{
	return -ENOSYS;
}
#endif

#endif	/* __SMBIOS_PLAT_H */
