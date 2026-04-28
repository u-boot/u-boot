// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Common sysinfo helpers for TQ-Systems SOMs
 *
 * Copyright (c) 2020-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>
 * D-82229 Seefeld, Germany.
 * Author: Nora Schiffer
 */

#include <env.h>
#include <fdt_support.h>
#include <log.h>
#include <mtd_node.h>
#include <net.h>
#include <spi_flash.h>
#include <sysinfo/tq_eeprom.h>

#include "tq_sysinfo.h"

#define MAX_NAME_LENGTH	80

static int tq_common_sysinfo_set_macaddrs(u8 start_index, u8 macaddr_num, u8 *macaddr)
{
	u8 i;
	int ret;

	for (i = 0; i < macaddr_num; i++) {
		ret = eth_env_set_enetaddr_by_index("eth",
						    start_index + i,
						    macaddr);
		if (ret && ret != -EEXIST) {
			log_debug("Failed to set macaddress by index %u: %d\n",
				  start_index + i, ret);
			return ret;
		}

		if (i >= macaddr_num - 1)
			break;

		if (++macaddr[5])
			continue;
		if (++macaddr[4])
			continue;
		if (++macaddr[3])
			continue;

		printf("Warning: End of MAC address block\n");
		break;
	}

	return 0;
}

int tq_common_sysinfo_setup(void)
{
	struct udevice *sysinfo;
	size_t macaddr_size;
	void *macaddr;
	char buf[MAX_NAME_LENGTH];
	int ret;

	ret = sysinfo_get_and_detect(&sysinfo);
	if (ret) {
		log_debug("Failed to get sysinfo data: %d\n", ret);
		return ret;
	}

	if (!sysinfo_get_str(sysinfo, SYSID_TQ_MODEL, sizeof(buf), buf))
		env_set_runtime("boardtype", buf);

	if (!sysinfo_get_str(sysinfo, SYSID_TQ_SERIAL, sizeof(buf), buf))
		env_set_runtime("serial#", buf);

	if (!sysinfo_get_data(sysinfo, SYSID_TQ_MAC_ADDR, &macaddr, &macaddr_size) &&
	    macaddr_size == ETH_ALEN) {
		ret = tq_common_sysinfo_set_macaddrs(CONFIG_TQ_COMMON_SYSINFO_MACADDR_OFFSET,
						     CONFIG_TQ_COMMON_SYSINFO_MACADDR_NUM,
						     macaddr);
		if (ret)
			log_debug("Failed to set macaddrs: %d\n", ret);
	}

	return 0;
}

int tq_common_sysinfo_set_eeprom_macaddrs_by_index(struct udevice *sysinfo, u8 start_index,
						   u8 macaddr_num)
{
	size_t macaddr_size;
	void *macaddr;
	int ret;

	if (!sysinfo_get_data(sysinfo, SYSID_TQ_MAC_ADDR, &macaddr, &macaddr_size) &&
	    macaddr_size == ETH_ALEN) {
		ret = tq_common_sysinfo_set_macaddrs(start_index, macaddr_num, macaddr);
		if (ret)
			log_debug("Failed to set macaddrs: %d\n", ret);
	}

	return 0;
}
