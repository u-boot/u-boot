/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#ifndef __SMBIOS_PLAT_H
#define __SMBIOS_PLAT_H

#include <smbios.h>

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
