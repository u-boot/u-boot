/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for the SEI510
 *
 * Copyright (C) 2019 Baylibre, SAS
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define LOGO_UUID "43a3305d-150f-4cc9-bd3b-38fca8693846;"
#define CACHE_UUID "99207ae6-5207-11e9-999e-6f77a3612069;"
#define SYSTEM_UUID "99f9b7ac-5207-11e9-8507-c3c037e393f3;"
#define VENDOR_UUID "9d082802-5207-11e9-954c-cbbce08ba108;"
#define USERDATA_UUID "9b976e42-5207-11e9-8f16-ff47ac594b22;"
#define ROOT_UUID "ddb8c3f6-d94d-4394-b633-3134139cc2e0;"

#define PARTS_DEFAULT                                        \
	"uuid_disk=${uuid_gpt_disk};"  			\
	"name=boot,size=64M,bootable,uuid=${uuid_gpt_boot};" \
	"name=logo,size=2M,uuid=" LOGO_UUID             \
	"name=cache,size=256M,uuid=" CACHE_UUID             \
	"name=system,size=1536M,uuid=" SYSTEM_UUID           \
	"name=vendor,size=256M,uuid=" VENDOR_UUID            \
	"name=userdata,size=5341M,uuid=" USERDATA_UUID	\
	"name=rootfs,size=-,uuid=" ROOT_UUID


#include <configs/meson64_android.h>

#endif /* __CONFIG_H */
