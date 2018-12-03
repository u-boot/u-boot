// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Marek Behun <marek.behun@nic.cz>
 * Copyright (C) 2016 Tomas Hlavacek <tomas.hlavacek@nic.cz>
 *
 * Derived from the code for
 *   Marvell/db-88f6820-gp by Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <environment.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <dm/uclass.h>
#include <fdt_support.h>
#include <time.h>

#ifdef CONFIG_ATSHA204A
# include <atsha204a-i2c.h>
#endif

#ifdef CONFIG_WDT_ORION
# include <wdt.h>
#endif

#include "../drivers/ddr/marvell/a38x/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>

DECLARE_GLOBAL_DATA_PTR;

#define OMNIA_I2C_EEPROM_DM_NAME	"i2c@0"
#define OMNIA_I2C_EEPROM		0x54
#define OMNIA_I2C_EEPROM_CONFIG_ADDR	0x0
#define OMNIA_I2C_EEPROM_ADDRLEN	2
#define OMNIA_I2C_EEPROM_MAGIC		0x0341a034

#define OMNIA_I2C_MCU_DM_NAME		"i2c@0"
#define OMNIA_I2C_MCU_ADDR_STATUS	0x1
#define OMNIA_I2C_MCU_SATA		0x20
#define OMNIA_I2C_MCU_CARDDET		0x10
#define OMNIA_I2C_MCU			0x2a
#define OMNIA_I2C_MCU_WDT_ADDR		0x0b

#define OMNIA_ATSHA204_OTP_VERSION	0
#define OMNIA_ATSHA204_OTP_SERIAL	1
#define OMNIA_ATSHA204_OTP_MAC0		3
#define OMNIA_ATSHA204_OTP_MAC1		4

#define MVTWSI_ARMADA_DEBUG_REG		0x8c

/*
 * Those values and defines are taken from the Marvell U-Boot version
 * "u-boot-2013.01-2014_T3.0"
 */
#define OMNIA_GPP_OUT_ENA_LOW					\
	(~(BIT(1)  | BIT(4)  | BIT(6)  | BIT(7)  | BIT(8)  | BIT(9)  |	\
	   BIT(10) | BIT(11) | BIT(19) | BIT(22) | BIT(23) | BIT(25) |	\
	   BIT(26) | BIT(27) | BIT(29) | BIT(30) | BIT(31)))
#define OMNIA_GPP_OUT_ENA_MID					\
	(~(BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4) | BIT(15) |	\
	   BIT(16) | BIT(17) | BIT(18)))

#define OMNIA_GPP_OUT_VAL_LOW	0x0
#define OMNIA_GPP_OUT_VAL_MID	0x0
#define OMNIA_GPP_POL_LOW	0x0
#define OMNIA_GPP_POL_MID	0x0

static struct serdes_map board_serdes_map_pex[] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

static struct serdes_map board_serdes_map_sata[] = {
	{SATA0, SERDES_SPEED_6_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

static bool omnia_detect_sata(void)
{
	struct udevice *bus, *dev;
	int ret, retry = 3;
	u16 mode;

	puts("SERDES0 card detect: ");

	if (uclass_get_device_by_name(UCLASS_I2C, OMNIA_I2C_MCU_DM_NAME, &bus)) {
		puts("Cannot find MCU bus!\n");
		return false;
	}

	ret = i2c_get_chip(bus, OMNIA_I2C_MCU, 1, &dev);
	if (ret) {
		puts("Cannot get MCU chip!\n");
		return false;
	}

	for (; retry > 0; --retry) {
		ret = dm_i2c_read(dev, OMNIA_I2C_MCU_ADDR_STATUS, (uchar *) &mode, 2);
		if (!ret)
			break;
	}

	if (!retry) {
		puts("I2C read failed! Default PEX\n");
		return false;
	}

	if (!(mode & OMNIA_I2C_MCU_CARDDET)) {
		puts("NONE\n");
		return false;
	}

	if (mode & OMNIA_I2C_MCU_SATA) {
		puts("SATA\n");
		return true;
	} else {
		puts("PEX\n");
		return false;
	}
}

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
	if (omnia_detect_sata()) {
		*serdes_map_array = board_serdes_map_sata;
		*count = ARRAY_SIZE(board_serdes_map_sata);
	} else {
		*serdes_map_array = board_serdes_map_pex;
		*count = ARRAY_SIZE(board_serdes_map_pex);
	}

	return 0;
}

struct omnia_eeprom {
	u32 magic;
	u32 ramsize;
	char region[4];
	u32 crc;
};

static bool omnia_read_eeprom(struct omnia_eeprom *oep)
{
	struct udevice *bus, *dev;
	int ret, crc, retry = 3;

	if (uclass_get_device_by_name(UCLASS_I2C, OMNIA_I2C_EEPROM_DM_NAME, &bus)) {
		puts("Cannot find EEPROM bus\n");
		return false;
	}

	ret = i2c_get_chip(bus, OMNIA_I2C_EEPROM, OMNIA_I2C_EEPROM_ADDRLEN, &dev);
	if (ret) {
		puts("Cannot get EEPROM chip\n");
		return false;
	}

	for (; retry > 0; --retry) {
		ret = dm_i2c_read(dev, OMNIA_I2C_EEPROM_CONFIG_ADDR, (uchar *) oep, sizeof(struct omnia_eeprom));
		if (ret)
			continue;

		if (oep->magic != OMNIA_I2C_EEPROM_MAGIC) {
			puts("I2C EEPROM missing magic number!\n");
			continue;
		}

		crc = crc32(0, (unsigned char *) oep,
			    sizeof(struct omnia_eeprom) - 4);
		if (crc == oep->crc) {
			break;
		} else {
			printf("CRC of EEPROM memory config failed! "
			       "calc=0x%04x saved=0x%04x\n", crc, oep->crc);
		}
	}

	if (!retry) {
		puts("I2C EEPROM read failed!\n");
		return false;
	}

	return true;
}

/*
 * Define the DDR layout / topology here in the board file. This will
 * be used by the DDR3 init code in the SPL U-Boot version to configure
 * the DDR3 controller.
 */
static struct mv_ddr_topology_map board_topology_map_1g = {
	DEBUG_LEVEL_ERROR,
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    MV_DDR_DEV_WIDTH_16BIT,	/* memory_width */
	    MV_DDR_DIE_CAP_4GBIT,			/* mem_size */
	    MV_DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_wl cas_l */
	    MV_DDR_TEMP_NORMAL,		/* temperature */
	    MV_DDR_TIM_2T} },		/* timing */
	BUS_MASK_32BIT,			/* Busses mask */
	MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
	{ {0} },			/* raw spd data */
	{0}				/* timing parameters */
};

static struct mv_ddr_topology_map board_topology_map_2g = {
	DEBUG_LEVEL_ERROR,
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    MV_DDR_DEV_WIDTH_16BIT,	/* memory_width */
	    MV_DDR_DIE_CAP_8GBIT,			/* mem_size */
	    MV_DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_wl cas_l */
	    MV_DDR_TEMP_NORMAL,		/* temperature */
	    MV_DDR_TIM_2T} },		/* timing */
	BUS_MASK_32BIT,			/* Busses mask */
	MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
	{ {0} },			/* raw spd data */
	{0}				/* timing parameters */
};

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
{
	static int mem = 0;
	struct omnia_eeprom oep;

	/* Get the board config from EEPROM */
	if (mem == 0) {
		if(!omnia_read_eeprom(&oep))
			goto out;

		printf("Memory config in EEPROM: 0x%02x\n", oep.ramsize);

		if (oep.ramsize == 0x2)
			mem = 2;
		else
			mem = 1;
	}

out:
	/* Hardcoded fallback */
	if (mem == 0) {
		puts("WARNING: Memory config from EEPROM read failed.\n");
		puts("Falling back to default 1GiB map.\n");
		mem = 1;
	}

	/* Return the board topology as defined in the board code */
	if (mem == 1)
		return &board_topology_map_1g;
	if (mem == 2)
		return &board_topology_map_2g;

	return &board_topology_map_1g;
}

#ifndef CONFIG_SPL_BUILD
static int set_regdomain(void)
{
	struct omnia_eeprom oep;
	char rd[3] = {' ', ' ', 0};

	if (omnia_read_eeprom(&oep))
		memcpy(rd, &oep.region, 2);
	else
		puts("EEPROM regdomain read failed.\n");

	printf("Regdomain set to %s\n", rd);
	return env_set("regdomain", rd);
}
#endif

int board_early_init_f(void)
{
	u32 i2c_debug_reg;

	/* Configure MPP */
	writel(0x11111111, MVEBU_MPP_BASE + 0x00);
	writel(0x11111111, MVEBU_MPP_BASE + 0x04);
	writel(0x11244011, MVEBU_MPP_BASE + 0x08);
	writel(0x22222111, MVEBU_MPP_BASE + 0x0c);
	writel(0x22200002, MVEBU_MPP_BASE + 0x10);
	writel(0x30042022, MVEBU_MPP_BASE + 0x14);
	writel(0x55550555, MVEBU_MPP_BASE + 0x18);
	writel(0x00005550, MVEBU_MPP_BASE + 0x1c);

	/* Set GPP Out value */
	writel(OMNIA_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(OMNIA_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(OMNIA_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(OMNIA_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(OMNIA_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(OMNIA_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);

	/*
	 * Disable I2C debug mode blocking 0x64 I2C address.
	 * Note: that would be redundant once Turris Omnia migrates to DM_I2C,
	 * because the mvtwsi driver includes equivalent code.
	 */
	i2c_debug_reg = readl(MVEBU_TWSI_BASE + MVTWSI_ARMADA_DEBUG_REG);
	i2c_debug_reg &= ~(1<<18);
	writel(i2c_debug_reg, MVEBU_TWSI_BASE + MVTWSI_ARMADA_DEBUG_REG);

	return 0;
}

#ifndef CONFIG_SPL_BUILD
static bool disable_mcu_watchdog(void)
{
	struct udevice *bus, *dev;
	int ret, retry = 3;
	uchar buf[1] = {0x0};

	if (uclass_get_device_by_name(UCLASS_I2C, OMNIA_I2C_MCU_DM_NAME, &bus)) {
		puts("Cannot find MCU bus! Can not disable MCU WDT.\n");
		return false;
	}

	ret = i2c_get_chip(bus, OMNIA_I2C_MCU, 1, &dev);
	if (ret) {
		puts("Cannot get MCU chip! Can not disable MCU WDT.\n");
		return false;
	}

	for (; retry > 0; --retry)
		if (!dm_i2c_write(dev, OMNIA_I2C_MCU_WDT_ADDR, (uchar *) buf, 1))
			break;

	if (retry <= 0) {
		puts("I2C MCU watchdog failed to disable!\n");
		return false;
	}

	return true;
}
#endif

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_WDT_ORION)
static struct udevice *watchdog_dev = NULL;
#endif

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

#ifndef CONFIG_SPL_BUILD
# ifdef CONFIG_WDT_ORION
	if (uclass_get_device(UCLASS_WDT, 0, &watchdog_dev)) {
		puts("Cannot find Armada 385 watchdog!\n");
	} else {
		puts("Enabling Armada 385 watchdog.\n");
		wdt_start(watchdog_dev, (u32) 25000000 * 120, 0);
	}
# endif

	if (disable_mcu_watchdog())
		puts("Disabled MCU startup watchdog.\n");

	set_regdomain();
#endif

	return 0;
}

#ifdef CONFIG_WATCHDOG
/* Called by macro WATCHDOG_RESET */
void watchdog_reset(void)
{
# if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_WDT_ORION)
	static ulong next_reset = 0;
	ulong now;

	if (!watchdog_dev)
		return;

	now = timer_get_us();

	/* Do not reset the watchdog too often */
	if (now > next_reset) {
		wdt_reset(watchdog_dev);
		next_reset = now + 1000;
	}
# endif
}
#endif

int board_late_init(void)
{
#ifndef CONFIG_SPL_BUILD
	set_regdomain();
#endif

	return 0;
}

#ifdef CONFIG_ATSHA204A
static struct udevice *get_atsha204a_dev(void)
{
	static struct udevice *dev = NULL;

	if (dev != NULL)
		return dev;

	if (uclass_get_device_by_name(UCLASS_MISC, "atsha204a@64", &dev)) {
		puts("Cannot find ATSHA204A on I2C bus!\n");
		dev = NULL;
	}

	return dev;
}
#endif

int checkboard(void)
{
	u32 version_num, serial_num;
	int err = 1;

#ifdef CONFIG_ATSHA204A
	struct udevice *dev = get_atsha204a_dev();

	if (dev) {
		err = atsha204a_wakeup(dev);
		if (err)
			goto out;

		err = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
				     OMNIA_ATSHA204_OTP_VERSION,
				     (u8 *) &version_num);
		if (err)
			goto out;

		err = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
				     OMNIA_ATSHA204_OTP_SERIAL,
				     (u8 *) &serial_num);
		if (err)
			goto out;

		atsha204a_sleep(dev);
	}

out:
#endif

	if (err)
		printf("Board: Turris Omnia (ver N/A). SN: N/A\n");
	else
		printf("Board: Turris Omnia SNL %08X%08X\n",
		       be32_to_cpu(version_num), be32_to_cpu(serial_num));

	return 0;
}

static void increment_mac(u8 *mac)
{
	int i;

	for (i = 5; i >= 3; i--) {
		mac[i] += 1;
		if (mac[i])
			break;
	}
}

int misc_init_r(void)
{
#ifdef CONFIG_ATSHA204A
	int err;
	struct udevice *dev = get_atsha204a_dev();
	u8 mac0[4], mac1[4], mac[6];

	if (!dev)
		goto out;

	err = atsha204a_wakeup(dev);
	if (err)
		goto out;

	err = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
			     OMNIA_ATSHA204_OTP_MAC0, mac0);
	if (err)
		goto out;

	err = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
			     OMNIA_ATSHA204_OTP_MAC1, mac1);
	if (err)
		goto out;

	atsha204a_sleep(dev);

	mac[0] = mac0[1];
	mac[1] = mac0[2];
	mac[2] = mac0[3];
	mac[3] = mac1[1];
	mac[4] = mac1[2];
	mac[5] = mac1[3];

	if (is_valid_ethaddr(mac))
		eth_env_set_enetaddr("ethaddr", mac);

	increment_mac(mac);

	if (is_valid_ethaddr(mac))
		eth_env_set_enetaddr("eth1addr", mac);

	increment_mac(mac);

	if (is_valid_ethaddr(mac))
		eth_env_set_enetaddr("eth2addr", mac);

out:
#endif

	return 0;
}

