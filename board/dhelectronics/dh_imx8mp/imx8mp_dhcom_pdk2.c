// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <i2c_eeprom.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>
#include <power/regulator.h>

#include "lpddr4_timing.h"
#include "../common/dh_common.h"
#include "../common/dh_imx.h"

DECLARE_GLOBAL_DATA_PTR;

int mach_cpu_init(void)
{
	icache_enable();
	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	const u16 memsz[] = { 512, 1024, 1536, 2048, 3072, 4096, 6144, 8192 };
	const u8 ecc = readl(DDRC_ECCCFG0(0)) & DDRC_ECCCFG0_ECC_MODE_MASK;
	u8 memcfg = dh_get_memcfg();

	/* 896 kiB, i.e. 1 MiB without 12.5% reserved for in-band ECC */
	*size = (u64)memsz[memcfg] * (SZ_1M - (ecc ? (SZ_1M / 8) : 0));

	return 0;
}

static int dh_imx8_setup_ethaddr(struct eeprom_id_page *eip)
{
	unsigned char enetaddr[6];

	if (dh_mac_is_in_env("ethaddr"))
		return 0;

	if (dh_get_mac_is_enabled("ethernet0"))
		return 0;

	if (!dh_imx_get_mac_from_fuse(enetaddr))
		goto out;

	if (!dh_get_value_from_eeprom_buffer(DH_MAC0, enetaddr, sizeof(enetaddr), eip))
		goto out;

	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom0"))
		goto out;

	return -ENXIO;

out:
	return eth_env_set_enetaddr("ethaddr", enetaddr);
}

static int dh_imx8_setup_eth1addr(struct eeprom_id_page *eip)
{
	unsigned char enetaddr[6];

	if (dh_mac_is_in_env("eth1addr"))
		return 0;

	if (dh_get_mac_is_enabled("ethernet1"))
		return 0;

	if (!dh_imx_get_mac_from_fuse(enetaddr))
		goto increment_out;

	if (!dh_get_value_from_eeprom_buffer(DH_MAC1, enetaddr, sizeof(enetaddr), eip))
		goto out;

	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom1"))
		goto out;

	/*
	 * Populate second ethernet MAC from first ethernet EEPROM with MAC
	 * address LSByte incremented by 1. This is only used on SoMs without
	 * second ethernet EEPROM, i.e. early prototypes.
	 */
	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom0"))
		goto increment_out;

	return -ENXIO;

increment_out:
	enetaddr[5]++;

out:
	return eth_env_set_enetaddr("eth1addr", enetaddr);
}

int dh_setup_mac_address(struct eeprom_id_page *eip)
{
	int ret;

	ret = dh_imx8_setup_ethaddr(eip);
	if (ret)
		printf("%s: Unable to setup ethaddr! ret = %d\n", __func__, ret);

	ret = dh_imx8_setup_eth1addr(eip);
	if (ret)
		printf("%s: Unable to setup eth1addr! ret = %d\n", __func__, ret);

	return ret;
}

void dh_add_item_number_and_serial_to_env(struct eeprom_id_page *eip)
{
	char *item_number_env;
	char item_number[8];	/* String with 7 characters + string termination */
	char *serial_env;
	char serial[10];	/* String with 9 characters + string termination */
	int ret;

	ret = dh_get_value_from_eeprom_buffer(DH_ITEM_NUMBER, item_number, sizeof(item_number),
					      eip);
	if (ret) {
		printf("%s: Unable to get DHSOM item number from EEPROM ID page! ret = %d\n",
		       __func__, ret);
	} else {
		item_number_env = env_get("dh_som_item_number");
		if (!item_number_env)
			env_set("dh_som_item_number", item_number);
		else if (strcmp(item_number_env, item_number))
			printf("Warning: Environment dh_som_item_number differs from EEPROM ID page value (%s != %s)\n",
			       item_number_env, item_number);
	}

	ret = dh_get_value_from_eeprom_buffer(DH_SERIAL_NUMBER, serial, sizeof(serial),
					      eip);
	if (ret) {
		printf("%s: Unable to get DHSOM serial number from EEPROM ID page! ret = %d\n",
		       __func__, ret);
	} else {
		serial_env = env_get("dh_som_serial_number");
		if (!serial_env)
			env_set("dh_som_serial_number", serial);
		else if (strcmp(serial_env, serial))
			printf("Warning: Environment dh_som_serial_number differs from EEPROM ID page value (%s != %s)\n",
			       serial_env, serial);
	}
}

int board_late_init(void)
{
	u8 eeprom_buffer[DH_EEPROM_ID_PAGE_MAX_SIZE] = { 0 };
	struct eeprom_id_page *eip = (struct eeprom_id_page *)eeprom_buffer;
	int ret;

	ret = dh_read_eeprom_id_page(eeprom_buffer, "eeprom0wl");
	if (ret) {
		/*
		 * The EEPROM ID page is available on SoM rev. 200 and greater.
		 * For SoM rev. 100 the return value will be -ENODEV. Suppress
		 * the error message for that, because the absence cannot be
		 * treated as an error.
		 */
		if (ret != -ENODEV)
			printf("%s: Cannot read valid data from EEPROM ID page! ret = %d\n",
			       __func__, ret);
		dh_setup_mac_address(NULL);
	} else {
		dh_setup_mac_address(eip);
		dh_add_item_number_and_serial_to_env(eip);
	}

	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	return prio ? ENVL_UNKNOWN : CONFIG_IS_ENABLED(ENV_IS_IN_SPI_FLASH,
						       (ENVL_SPI_FLASH),
						       (ENVL_NOWHERE));
}
