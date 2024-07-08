// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Marek Behún <kabel@kernel.org>
 * Copyright (C) 2016 Tomas Hlavacek <tomas.hlavacek@nic.cz>
 *
 * Derived from the code for
 *   Marvell/db-88f6820-gp by Stefan Roese <sr@denx.de>
 */

#include <config.h>
#include <env.h>
#include <i2c.h>
#include <init.h>
#include <log.h>
#include <miiphy.h>
#include <mtd.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/unaligned.h>
#include <dm/uclass.h>
#include <dt-bindings/gpio/gpio.h>
#include <fdt_support.h>
#include <hexdump.h>
#include <time.h>
#include <turris-omnia-mcu-interface.h>
#include <linux/bitops.h>
#include <linux/bitrev.h>
#include <linux/delay.h>
#include <u-boot/crc.h>

#include "../drivers/ddr/marvell/a38x/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>
#include "../turris_atsha_otp.h"
#include "../turris_common.h"

DECLARE_GLOBAL_DATA_PTR;

#define OMNIA_I2C_BUS_NAME		"i2c@11000->i2cmux@70->i2c@0"

#define OMNIA_I2C_MCU_CHIP_ADDR		0x2a
#define OMNIA_I2C_MCU_CHIP_LEN		1

#define OMNIA_I2C_EEPROM_CHIP_ADDR	0x54
#define OMNIA_I2C_EEPROM_CHIP_LEN	2
#define OMNIA_I2C_EEPROM_MAGIC		0x0341a034

#define A385_SYS_RSTOUT_MASK		MVEBU_REGISTER(0x18260)
#define   A385_SYS_RSTOUT_MASK_WD	BIT(10)

#define A385_WDT_GLOBAL_CTRL		MVEBU_REGISTER(0x20300)
#define   A385_WDT_GLOBAL_RATIO_MASK	GENMASK(18, 16)
#define   A385_WDT_GLOBAL_RATIO_SHIFT	16
#define   A385_WDT_GLOBAL_25MHZ		BIT(10)
#define   A385_WDT_GLOBAL_ENABLE	BIT(8)

#define A385_WDT_GLOBAL_STATUS		MVEBU_REGISTER(0x20304)
#define   A385_WDT_GLOBAL_EXPIRED	BIT(31)

#define A385_WDT_DURATION		MVEBU_REGISTER(0x20334)

#define A385_WD_RSTOUT_UNMASK		MVEBU_REGISTER(0x20704)
#define   A385_WD_RSTOUT_UNMASK_GLOBAL	BIT(8)

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

static struct serdes_map board_serdes_map[] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

static struct udevice *omnia_get_i2c_chip(const char *name, uint addr,
					  uint offset_len)
{
	struct udevice *bus, *dev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_I2C, OMNIA_I2C_BUS_NAME, &bus);
	if (ret) {
		printf("Cannot get I2C bus %s: uclass_get_device_by_name failed: %i\n",
		       OMNIA_I2C_BUS_NAME, ret);
		return NULL;
	}

	ret = i2c_get_chip(bus, addr, offset_len, &dev);
	if (ret) {
		printf("Cannot get %s I2C chip: i2c_get_chip failed: %i\n",
		       name, ret);
		return NULL;
	}

	return dev;
}

static int omnia_mcu_read(u8 cmd, void *buf, int len)
{
	struct udevice *chip;

	chip = omnia_get_i2c_chip("MCU", OMNIA_I2C_MCU_CHIP_ADDR,
				  OMNIA_I2C_MCU_CHIP_LEN);
	if (!chip)
		return -ENODEV;

	return dm_i2c_read(chip, cmd, buf, len);
}

static int omnia_mcu_write(u8 cmd, const void *buf, int len)
{
	struct udevice *chip;

	chip = omnia_get_i2c_chip("MCU", OMNIA_I2C_MCU_CHIP_ADDR,
				  OMNIA_I2C_MCU_CHIP_LEN);
	if (!chip)
		return -ENODEV;

	return dm_i2c_write(chip, cmd, buf, len);
}

static int omnia_mcu_get_sts_and_features(u16 *psts, u32 *pfeatures)
{
	u16 sts, feat16;
	int ret;

	ret = omnia_mcu_read(CMD_GET_STATUS_WORD, &sts, sizeof(sts));
	if (ret)
		return ret;

	if (psts)
		*psts = sts;

	if (!pfeatures)
		return 0;

	if (sts & STS_FEATURES_SUPPORTED) {
		/* try read 32-bit features */
		ret = omnia_mcu_read(CMD_GET_FEATURES, pfeatures,
				     sizeof(*pfeatures));
		if (ret) {
			/* try read 16-bit features */
			ret = omnia_mcu_read(CMD_GET_FEATURES, &feat16,
					     sizeof(&feat16));
			if (ret)
				return ret;

			*pfeatures = feat16;
		} else {
			if (*pfeatures & FEAT_FROM_BIT_16_INVALID)
				*pfeatures &= GENMASK(15, 0);
		}
	} else {
		*pfeatures = 0;
	}

	return 0;
}

static int omnia_mcu_get_sts(u16 *sts)
{
	return omnia_mcu_get_sts_and_features(sts, NULL);
}

static bool omnia_mcu_has_feature(u32 feature)
{
	u32 features;

	if (omnia_mcu_get_sts_and_features(NULL, &features))
		return false;

	return feature & features;
}

static u32 omnia_mcu_crc32(const void *p, size_t len)
{
	u32 val, crc = 0;

	compiletime_assert(!(len % 4), "length has to be a multiple of 4");

	while (len) {
		val = bitrev32(get_unaligned_le32(p));
		crc = crc32(crc, (void *)&val, 4);
		p += 4;
		len -= 4;
	}

	return ~bitrev32(crc);
}

/* Can only be called after relocation, since it needs cleared BSS */
static int omnia_mcu_board_info(char *serial, u8 *mac, char *version)
{
	static u8 reply[17];
	static bool cached;

	if (!cached) {
		u8 csum;
		int ret;

		ret = omnia_mcu_read(CMD_BOARD_INFO_GET, reply, sizeof(reply));
		if (ret)
			return ret;

		if (reply[0] != 16)
			return -EBADMSG;

		csum = reply[16];
		reply[16] = 0;

		if ((omnia_mcu_crc32(&reply[1], 16) & 0xff) != csum)
			return -EBADMSG;

		cached = true;
	}

	if (serial) {
		const char *serial_env;

		serial_env = env_get("serial#");
		if (serial_env && strlen(serial_env) == 16) {
			strcpy(serial, serial_env);
		} else {
			sprintf(serial, "%016llX",
				get_unaligned_le64(&reply[1]));
			env_set("serial#", serial);
		}
	}

	if (mac)
		memcpy(mac, &reply[9], ETH_ALEN);

	if (version)
		sprintf(version, "%u", reply[15]);

	return 0;
}

static int omnia_mcu_get_board_public_key(char pub_key[static 67])
{
	u8 reply[34];
	int ret;

	ret = omnia_mcu_read(CMD_CRYPTO_GET_PUBLIC_KEY, reply, sizeof(reply));
	if (ret)
		return ret;

	if (reply[0] != 33)
		return -EBADMSG;

	bin2hex(pub_key, &reply[1], 33);
	pub_key[66] = '\0';

	return 0;
}

static void enable_a385_watchdog(unsigned int timeout_minutes)
{
	struct sar_freq_modes sar_freq;
	u32 watchdog_freq;

	printf("Enabling A385 watchdog with %u minutes timeout...\n",
	       timeout_minutes);

	/*
	 * Use NBCLK clock (a.k.a. L2 clock) as watchdog input clock with
	 * its maximal ratio 7 instead of default fixed 25 MHz clock.
	 * It allows to set watchdog duration up to the 22 minutes.
	 */
	clrsetbits_32(A385_WDT_GLOBAL_CTRL,
		      A385_WDT_GLOBAL_25MHZ | A385_WDT_GLOBAL_RATIO_MASK,
		      7 << A385_WDT_GLOBAL_RATIO_SHIFT);

	/*
	 * Calculate watchdog clock frequency. It is defined by formula:
	 *   freq = NBCLK / 2 / (2 ^ ratio)
	 * We set ratio to the maximal possible value 7.
	 */
	get_sar_freq(&sar_freq);
	watchdog_freq = sar_freq.nb_clk * 1000000 / 2 / (1 << 7);

	/* Set watchdog duration */
	writel(timeout_minutes * 60 * watchdog_freq, A385_WDT_DURATION);

	/* Clear the watchdog expiration bit */
	clrbits_32(A385_WDT_GLOBAL_STATUS, A385_WDT_GLOBAL_EXPIRED);

	/* Enable watchdog timer */
	setbits_32(A385_WDT_GLOBAL_CTRL, A385_WDT_GLOBAL_ENABLE);

	/* Enable reset on watchdog */
	setbits_32(A385_WD_RSTOUT_UNMASK, A385_WD_RSTOUT_UNMASK_GLOBAL);

	/* Unmask reset for watchdog */
	clrbits_32(A385_SYS_RSTOUT_MASK, A385_SYS_RSTOUT_MASK_WD);
}

static bool disable_mcu_watchdog(void)
{
	int ret;

	puts("Disabling MCU watchdog... ");

	ret = omnia_mcu_write(CMD_SET_WATCHDOG_STATE, "\x00", 1);
	if (ret) {
		printf("omnia_mcu_write failed: %i\n", ret);
		return false;
	}

	puts("disabled\n");

	return true;
}

static bool omnia_detect_sata(const char *msata_slot)
{
	int ret;
	u16 sts;

	puts("MiniPCIe/mSATA card detection... ");

	if (msata_slot) {
		if (strcmp(msata_slot, "pcie") == 0) {
			puts("forced to MiniPCIe via env\n");
			return false;
		} else if (strcmp(msata_slot, "sata") == 0) {
			puts("forced to mSATA via env\n");
			return true;
		} else if (strcmp(msata_slot, "auto") != 0) {
			printf("unsupported env value '%s', fallback to... ", msata_slot);
		}
	}

	ret = omnia_mcu_get_sts(&sts);
	if (ret) {
		printf("omnia_mcu_read failed: %i, defaulting to MiniPCIe card\n",
		       ret);
		return false;
	}

	if (!(sts & STS_CARD_DET)) {
		puts("none\n");
		return false;
	}

	if (sts & STS_MSATA_IND)
		puts("mSATA\n");
	else
		puts("MiniPCIe\n");

	return sts & STS_MSATA_IND;
}

static bool omnia_detect_wwan_usb3(const char *wwan_slot)
{
	puts("WWAN slot configuration... ");

	if (wwan_slot && strcmp(wwan_slot, "usb3") == 0) {
		puts("USB3.0\n");
		return true;
	}

	if (wwan_slot && strcmp(wwan_slot, "pcie") != 0)
		printf("unsupported env value '%s', fallback to... ", wwan_slot);

	puts("PCIe+USB2.0\n");
	return false;
}

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
#ifdef CONFIG_SPL_ENV_SUPPORT
	/* Do not use env_load() as malloc() pool is too small at this stage */
	bool has_env = (env_init() == 0);
#endif
	const char *env_value = NULL;

#ifdef CONFIG_SPL_ENV_SUPPORT
	/* beware that env_get() returns static allocated memory */
	env_value = has_env ? env_get("omnia_msata_slot") : NULL;
#endif

	if (omnia_detect_sata(env_value)) {
		/* Change SerDes for first mPCIe port (mSATA) from PCIe to SATA */
		board_serdes_map[0].serdes_type = SATA0;
		board_serdes_map[0].serdes_speed = SERDES_SPEED_6_GBPS;
		board_serdes_map[0].serdes_mode = SERDES_DEFAULT_MODE;
	}

#ifdef CONFIG_SPL_ENV_SUPPORT
	/* beware that env_get() returns static allocated memory */
	env_value = has_env ? env_get("omnia_wwan_slot") : NULL;
#endif

	if (omnia_detect_wwan_usb3(env_value)) {
		/* Disable SerDes for USB 3.0 pins on the front USB-A port */
		board_serdes_map[1].serdes_type = DEFAULT_SERDES;
		/* Change SerDes for third mPCIe port (WWAN) from PCIe to USB 3.0 */
		board_serdes_map[4].serdes_type = USB3_HOST0;
		board_serdes_map[4].serdes_speed = SERDES_SPEED_5_GBPS;
		board_serdes_map[4].serdes_mode = SERDES_DEFAULT_MODE;
	}

	*serdes_map_array = board_serdes_map;
	*count = ARRAY_SIZE(board_serdes_map);

	return 0;
}

struct omnia_eeprom {
	u32 magic;
	u32 ramsize;
	char region[4];
	u32 crc;

	/* second part (only considered if crc2 is not all-ones) */
	char ddr_speed[5];
	u8 old_ddr_training;
	u8 reserved[38];
	u32 crc2;
};

static bool is_omnia_eeprom_second_part_valid(const struct omnia_eeprom *oep)
{
	return oep->crc2 != 0xffffffff;
}

static void make_omnia_eeprom_second_part_invalid(struct omnia_eeprom *oep)
{
	oep->crc2 = 0xffffffff;
}

static bool check_eeprom_crc(const void *buf, size_t size, u32 expected,
			     const char *name)
{
	u32 crc;

	crc = crc32(0, buf, size);
	if (crc != expected) {
		printf("bad %s EEPROM CRC (stored %08x, computed %08x)\n",
		       name, expected, crc);
		return false;
	}

	return true;
}

static bool omnia_read_eeprom(struct omnia_eeprom *oep)
{
	struct udevice *chip;
	int ret;

	chip = omnia_get_i2c_chip("EEPROM", OMNIA_I2C_EEPROM_CHIP_ADDR,
				  OMNIA_I2C_EEPROM_CHIP_LEN);

	if (!chip)
		return false;

	ret = dm_i2c_read(chip, 0, (void *)oep, sizeof(*oep));
	if (ret) {
		printf("dm_i2c_read failed: %i, cannot read EEPROM\n", ret);
		return false;
	}

	if (oep->magic != OMNIA_I2C_EEPROM_MAGIC) {
		printf("bad EEPROM magic number (%08x, should be %08x)\n",
		       oep->magic, OMNIA_I2C_EEPROM_MAGIC);
		return false;
	}

	if (!check_eeprom_crc(oep, offsetof(struct omnia_eeprom, crc), oep->crc,
			      "first"))
		return false;

	if (is_omnia_eeprom_second_part_valid(oep) &&
	    !check_eeprom_crc(oep, offsetof(struct omnia_eeprom, crc2),
			      oep->crc2, "second"))
		make_omnia_eeprom_second_part_invalid(oep);

	return true;
}

int omnia_get_ram_size_gb(void)
{
	static int ram_size;
	struct omnia_eeprom oep;

	if (!ram_size) {
		/* Get the board config from EEPROM */
		if (omnia_read_eeprom(&oep)) {
			debug("Memory config in EEPROM: 0x%02x\n", oep.ramsize);

			if (oep.ramsize == 0x2)
				ram_size = 2;
			else
				ram_size = 1;
		} else {
			/* Hardcoded fallback */
			puts("Memory config from EEPROM read failed!\n");
			puts("Falling back to default 1 GiB!\n");
			ram_size = 1;
		}
	}

	return ram_size;
}

bool board_use_old_ddr3_training(void)
{
	struct omnia_eeprom oep;

	if (!omnia_read_eeprom(&oep))
		return false;

	if (!is_omnia_eeprom_second_part_valid(&oep))
		return false;

	return oep.old_ddr_training == 1;
}

static const char *omnia_get_ddr_speed(void)
{
	struct omnia_eeprom oep;
	static char speed[sizeof(oep.ddr_speed) + 1];

	if (!omnia_read_eeprom(&oep))
		return NULL;

	if (!is_omnia_eeprom_second_part_valid(&oep))
		return NULL;

	if (!oep.ddr_speed[0] || oep.ddr_speed[0] == 0xff)
		return NULL;

	memcpy(&speed, &oep.ddr_speed, sizeof(oep.ddr_speed));
	speed[sizeof(speed) - 1] = '\0';

	return speed;
}

static const char * const omnia_get_mcu_type(void)
{
	static char result[] = "xxxxxxx (with peripheral resets)";
	u16 sts;
	int ret;

	ret = omnia_mcu_get_sts(&sts);
	if (ret)
		return "unknown";

	switch (sts & STS_MCU_TYPE_MASK) {
	case STS_MCU_TYPE_STM32:
		strcpy(result, "STM32");
		break;
	case STS_MCU_TYPE_GD32:
		strcpy(result, "GD32");
		break;
	case STS_MCU_TYPE_MKL:
		strcpy(result, "MKL");
		break;
	default:
		strcpy(result, "unknown");
		break;
	}

	if (omnia_mcu_has_feature(FEAT_PERIPH_MCU))
		strcat(result, " (with peripheral resets)");

	return result;
}

static const char * const omnia_get_mcu_version(void)
{
	static char version[82];
	u8 version_app[20];
	u8 version_boot[20];
	int ret;

	ret = omnia_mcu_read(CMD_GET_FW_VERSION_APP, &version_app, sizeof(version_app));
	if (ret)
		return "unknown";

	ret = omnia_mcu_read(CMD_GET_FW_VERSION_BOOT, &version_boot, sizeof(version_boot));
	if (ret)
		return "unknown";

	/*
	 * If git commits of MCU bootloader and MCU application are same then
	 * show version only once. If they are different then show both commits.
	 */
	if (!memcmp(version_app, version_boot, 20)) {
		bin2hex(version, version_app, 20);
		version[40] = '\0';
	} else {
		bin2hex(version, version_boot, 20);
		version[40] = '/';
		bin2hex(version + 41, version_app, 20);
		version[81] = '\0';
	}

	return version;
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
	NOT_COMBINED,			/* ddr twin-die combined */
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
	NOT_COMBINED,			/* ddr twin-die combined */
	{ {0} },			/* raw spd data */
	{0}				/* timing parameters */
};

static const struct omnia_ddr_speed {
	char name[5];
	u8 speed_bin;
	u8 freq;
} omnia_ddr_speeds[] = {
	{ "1066F", SPEED_BIN_DDR_1066F, MV_DDR_FREQ_533 },
	{ "1333H", SPEED_BIN_DDR_1333H, MV_DDR_FREQ_667 },
	{ "1600K", SPEED_BIN_DDR_1600K, MV_DDR_FREQ_800 },
};

static const struct omnia_ddr_speed *find_ddr_speed_setting(const char *name)
{
	for (int i = 0; i < ARRAY_SIZE(omnia_ddr_speeds); ++i)
		if (!strncmp(name, omnia_ddr_speeds[i].name, 5))
			return &omnia_ddr_speeds[i];

	return NULL;
}

bool omnia_valid_ddr_speed(const char *name)
{
	return find_ddr_speed_setting(name) != NULL;
}

void omnia_print_ddr_speeds(void)
{
	for (int i = 0; i < ARRAY_SIZE(omnia_ddr_speeds); ++i)
		printf("%.5s%s", omnia_ddr_speeds[i].name,
		       i == ARRAY_SIZE(omnia_ddr_speeds) - 1 ? "\n" : ", ");
}

static void fixup_speed_in_ddr_topology(struct mv_ddr_topology_map *topology)
{
	typeof(topology->interface_params[0]) *params;
	const struct omnia_ddr_speed *setting;
	const char *speed;
	static bool done;

	if (done)
		return;

	done = true;

	speed = omnia_get_ddr_speed();
	if (!speed)
		return;

	setting = find_ddr_speed_setting(speed);
	if (!setting) {
		printf("Unsupported value %s for DDR3 speed in EEPROM!\n",
		       speed);
		return;
	}

	params = &topology->interface_params[0];

	/* don't inform if we are not changing the speed from the default one */
	if (params->speed_bin_index == setting->speed_bin)
		return;

	printf("Fixing up DDR3 speed (EEPROM defines %s)\n", speed);

	params->speed_bin_index = setting->speed_bin;
	params->memory_freq = setting->freq;
}

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
{
	struct mv_ddr_topology_map *topology;

	if (omnia_get_ram_size_gb() == 2)
		topology = &board_topology_map_2g;
	else
		topology = &board_topology_map_1g;

	fixup_speed_in_ddr_topology(topology);

	return topology;
}

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

static void handle_reset_button(void)
{
	const char * const vars[1] = { "bootcmd_rescue", };
	int ret;
	u8 reset_status;

	/*
	 * Ensure that bootcmd_rescue has always stock value, so that running
	 *   run bootcmd_rescue
	 * always works correctly.
	 */
	env_set_default_vars(1, (char * const *)vars, 0);

	ret = omnia_mcu_read(CMD_GET_RESET, &reset_status, 1);
	if (ret) {
		printf("omnia_mcu_read failed: %i, reset status unknown!\n",
		       ret);
		return;
	}

	env_set_ulong("omnia_reset", reset_status);

	if (reset_status) {
		const char * const vars[3] = {
			"bootcmd",
			"bootdelay",
			"distro_bootcmd",
		};

		/*
		 * Set the above envs to their default values, in case the user
		 * managed to break them.
		 */
		env_set_default_vars(3, (char * const *)vars, 0);

		/* Ensure bootcmd_rescue is used by distroboot */
		env_set("boot_targets", "rescue");

		printf("RESET button was pressed, overwriting boot_targets!\n");
	} else {
		/*
		 * In case the user somehow managed to save environment with
		 * boot_targets=rescue, reset boot_targets to default value.
		 * This could happen in subsequent commands if bootcmd_rescue
		 * failed.
		 */
		if (!strcmp(env_get("boot_targets"), "rescue")) {
			const char * const vars[1] = {
				"boot_targets",
			};

			env_set_default_vars(1, (char * const *)vars, 0);
		}
	}
}

static void initialize_switch(void)
{
	u32 val, val04, val08, val10, val14;
	u16 ctrl[2];
	int err;

	printf("Initializing LAN eth switch... ");

	/* Change RGMII pins to GPIO mode */

	val = val04 = readl(MVEBU_MPP_BASE + 0x04);
	val &= ~GENMASK(19, 16); /* MPP[12] := GPIO */
	val &= ~GENMASK(23, 20); /* MPP[13] := GPIO */
	val &= ~GENMASK(27, 24); /* MPP[14] := GPIO */
	val &= ~GENMASK(31, 28); /* MPP[15] := GPIO */
	writel(val, MVEBU_MPP_BASE + 0x04);

	val = val08 = readl(MVEBU_MPP_BASE + 0x08);
	val &= ~GENMASK(3, 0);   /* MPP[16] := GPIO */
	val &= ~GENMASK(23, 20); /* MPP[21] := GPIO */
	writel(val, MVEBU_MPP_BASE + 0x08);

	val = val10 = readl(MVEBU_MPP_BASE + 0x10);
	val &= ~GENMASK(27, 24); /* MPP[38] := GPIO */
	val &= ~GENMASK(31, 28); /* MPP[39] := GPIO */
	writel(val, MVEBU_MPP_BASE + 0x10);

	val = val14 = readl(MVEBU_MPP_BASE + 0x14);
	val &= ~GENMASK(3, 0);   /* MPP[40] := GPIO */
	val &= ~GENMASK(7, 4);   /* MPP[41] := GPIO */
	writel(val, MVEBU_MPP_BASE + 0x14);

	/* Set initial values for switch reset strapping pins */

	val = readl(MVEBU_GPIO0_BASE + 0x00);
	val |= BIT(12);		/* GPIO[12] := 1 */
	val |= BIT(13);		/* GPIO[13] := 1 */
	val |= BIT(14);		/* GPIO[14] := 1 */
	val |= BIT(15);		/* GPIO[15] := 1 */
	val &= ~BIT(16);	/* GPIO[16] := 0 */
	val |= BIT(21);		/* GPIO[21] := 1 */
	writel(val, MVEBU_GPIO0_BASE + 0x00);

	val = readl(MVEBU_GPIO1_BASE + 0x00);
	val |= BIT(6);		/* GPIO[38] := 1 */
	val |= BIT(7);		/* GPIO[39] := 1 */
	val |= BIT(8);		/* GPIO[40] := 1 */
	val &= ~BIT(9);		/* GPIO[41] := 0 */
	writel(val, MVEBU_GPIO1_BASE + 0x00);

	val = readl(MVEBU_GPIO0_BASE + 0x04);
	val &= ~BIT(12);	/* GPIO[12] := Out Enable */
	val &= ~BIT(13);	/* GPIO[13] := Out Enable */
	val &= ~BIT(14);	/* GPIO[14] := Out Enable */
	val &= ~BIT(15);	/* GPIO[15] := Out Enable */
	val &= ~BIT(16);	/* GPIO[16] := Out Enable */
	val &= ~BIT(21);	/* GPIO[21] := Out Enable */
	writel(val, MVEBU_GPIO0_BASE + 0x04);

	val = readl(MVEBU_GPIO1_BASE + 0x04);
	val &= ~BIT(6);		/* GPIO[38] := Out Enable */
	val &= ~BIT(7);		/* GPIO[39] := Out Enable */
	val &= ~BIT(8);		/* GPIO[40] := Out Enable */
	val &= ~BIT(9);		/* GPIO[41] := Out Enable */
	writel(val, MVEBU_GPIO1_BASE + 0x04);

	/* Release switch reset */

	ctrl[0] = EXT_CTL_nRES_LAN;
	ctrl[1] = EXT_CTL_nRES_LAN;
	err = omnia_mcu_write(CMD_EXT_CONTROL, ctrl, sizeof(ctrl));

	mdelay(50);

	/* Change RGMII pins back to RGMII mode */

	writel(val04, MVEBU_MPP_BASE + 0x04);
	writel(val08, MVEBU_MPP_BASE + 0x08);
	writel(val10, MVEBU_MPP_BASE + 0x10);
	writel(val14, MVEBU_MPP_BASE + 0x14);

	puts(err ? "failed\n" : "done\n");
}

int board_early_init_f(void)
{
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

	return 0;
}

void spl_board_init(void)
{
	/*
	 * If booting from UART, disable MCU watchdog in SPL, since uploading
	 * U-Boot proper can take too much time and trigger it. Instead enable
	 * A385 watchdog with very high timeout (10 minutes) to prevent hangup.
	 */
	if (get_boot_device() == BOOT_DEVICE_UART) {
		enable_a385_watchdog(10);
		disable_mcu_watchdog();
	}

	/*
	 * When MCU controls peripheral resets then release LAN eth switch from
	 * the reset and initialize it. When MCU does not control peripheral
	 * resets then LAN eth switch is initialized automatically by bootstrap
	 * pins when A385 is released from the reset.
	 */
	if (omnia_mcu_has_feature(FEAT_PERIPH_MCU))
		initialize_switch();
}

#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP) || IS_ENABLED(CONFIG_OF_BOARD_SETUP)

static void disable_sata_node(void *blob)
{
	int node;

	fdt_for_each_node_by_compatible(node, blob, -1, "marvell,armada-380-ahci") {
		if (!fdtdec_get_is_enabled(blob, node))
			continue;

		if (fdt_status_disabled(blob, node) < 0)
			printf("Cannot disable SATA DT node!\n");
		else
			debug("Disabled SATA DT node\n");

		return;
	}

	printf("Cannot find SATA DT node!\n");
}

static void disable_pcie_node(void *blob, int port)
{
	int node;

	fdt_for_each_node_by_compatible(node, blob, -1, "marvell,armada-370-pcie") {
		int port_node;

		if (!fdtdec_get_is_enabled(blob, node))
			continue;

		fdt_for_each_subnode (port_node, blob, node) {
			if (!fdtdec_get_is_enabled(blob, port_node))
				continue;

			if (fdtdec_get_int(blob, port_node, "marvell,pcie-port", -1) != port)
				continue;

			if (fdt_status_disabled(blob, port_node) < 0)
				printf("Cannot disable PCIe port %d DT node!\n", port);
			else
				debug("Disabled PCIe port %d DT node\n", port);

			return;
		}
	}

	printf("Cannot find PCIe port %d DT node!\n", port);
}

static void fixup_msata_port_nodes(void *blob)
{
	bool mode_sata;

	/*
	 * Determine if SerDes 0 is configured to SATA mode.
	 * We do this instead of calling omnia_detect_sata() to avoid another
	 * call to the MCU. By this time the common PHYs are initialized (it is
	 * done in SPL), so we can read this common PHY register.
	 */
	mode_sata = (readl(MVEBU_REGISTER(0x183fc)) & GENMASK(3, 0)) == 2;

	/*
	 * We're either adding status = "disabled" property, or changing
	 * status = "okay" to status = "disabled". In both cases we'll need more
	 * space. Increase the size a little.
	 */
	if (fdt_increase_size(blob, 32) < 0) {
		printf("Cannot increase FDT size!\n");
		return;
	}

	if (!mode_sata) {
		/* If mSATA card is not present, disable SATA DT node */
		disable_sata_node(blob);
	} else {
		/* Otherwise disable PCIe port 0 DT node (MiniPCIe / mSATA port) */
		disable_pcie_node(blob, 0);
	}
}

static void fixup_wwan_port_nodes(void *blob)
{
	bool mode_usb3;

	/* Determine if SerDes 4 is configured to USB3 mode */
	mode_usb3 = ((readl(MVEBU_REGISTER(0x183fc)) & GENMASK(19, 16)) >> 16) == 4;

	/* If SerDes 4 is not configured to USB3 mode then nothing is needed to fixup */
	if (!mode_usb3)
		return;

	/*
	 * We're either adding status = "disabled" property, or changing
	 * status = "okay" to status = "disabled". In both cases we'll need more
	 * space. Increase the size a little.
	 */
	if (fdt_increase_size(blob, 32) < 0) {
		printf("Cannot increase FDT size!\n");
		return;
	}

	/* Disable PCIe port 2 DT node (WWAN) */
	disable_pcie_node(blob, 2);
}

static int insert_mcu_gpio_prop(void *blob, int node, const char *prop,
				unsigned int phandle, u32 bank, u32 gpio,
				u32 flags)
{
	fdt32_t val[4] = { cpu_to_fdt32(phandle), cpu_to_fdt32(bank),
			   cpu_to_fdt32(gpio), cpu_to_fdt32(flags) };
	return fdt_setprop(blob, node, prop, &val, sizeof(val));
}

static int fixup_mcu_gpio_in_pcie_nodes(void *blob)
{
	unsigned int mcu_phandle;
	int port, gpio;
	int pcie_node;
	int port_node;
	int ret;

	ret = fdt_increase_size(blob, 128);
	if (ret < 0) {
		printf("Cannot increase FDT size!\n");
		return ret;
	}

	mcu_phandle = fdt_create_phandle_by_compatible(blob, "cznic,turris-omnia-mcu");
	if (!mcu_phandle)
		return -FDT_ERR_NOPHANDLES;

	fdt_for_each_node_by_compatible(pcie_node, blob, -1, "marvell,armada-370-pcie") {
		if (!fdtdec_get_is_enabled(blob, pcie_node))
			continue;

		fdt_for_each_subnode(port_node, blob, pcie_node) {
			if (!fdtdec_get_is_enabled(blob, port_node))
				continue;

			port = fdtdec_get_int(blob, port_node, "marvell,pcie-port", -1);

			if (port == 0)
				gpio = ilog2(EXT_CTL_nPERST0);
			else if (port == 1)
				gpio = ilog2(EXT_CTL_nPERST1);
			else if (port == 2)
				gpio = ilog2(EXT_CTL_nPERST2);
			else
				continue;

			/* insert: reset-gpios = <&mcu 2 gpio GPIO_ACTIVE_LOW>; */
			ret = insert_mcu_gpio_prop(blob, port_node, "reset-gpios",
						   mcu_phandle, 2, gpio, GPIO_ACTIVE_LOW);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}

static int get_phy_wan_node_offset(const void *blob)
{
	u32 phy_wan_phandle;

	phy_wan_phandle = fdt_getprop_u32_default(blob, "ethernet2", "phy-handle", 0);
	if (!phy_wan_phandle)
		return -FDT_ERR_NOTFOUND;

	return fdt_node_offset_by_phandle(blob, phy_wan_phandle);
}

static int fixup_mcu_gpio_in_phy_wan_node(void *blob)
{
	unsigned int mcu_phandle;
	int phy_wan_node, ret;

	ret = fdt_increase_size(blob, 64);
	if (ret < 0) {
		printf("Cannot increase FDT size!\n");
		return ret;
	}

	phy_wan_node = get_phy_wan_node_offset(blob);
	if (phy_wan_node < 0)
		return phy_wan_node;

	mcu_phandle = fdt_create_phandle_by_compatible(blob, "cznic,turris-omnia-mcu");
	if (!mcu_phandle)
		return -FDT_ERR_NOPHANDLES;

	/* insert: reset-gpios = <&mcu 2 gpio GPIO_ACTIVE_LOW>; */
	return insert_mcu_gpio_prop(blob, phy_wan_node, "reset-gpios",
				    mcu_phandle, 2, ilog2(EXT_CTL_nRES_PHY), GPIO_ACTIVE_LOW);
}

static void fixup_atsha_node(void *blob)
{
	int node;

	if (!omnia_mcu_has_feature(FEAT_CRYPTO))
		return;

	node = fdt_node_offset_by_compatible(blob, -1, "atmel,atsha204a");
	if (node < 0) {
		printf("Cannot find ATSHA204A node!\n");
		return;
	}

	if (fdt_status_disabled(blob, node) < 0)
		printf("Cannot disable ATSHA204A node!\n");
	else
		debug("Disabled ATSHA204A node\n");
}

#endif

#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *blob)
{
	if (omnia_mcu_has_feature(FEAT_PERIPH_MCU)) {
		fixup_mcu_gpio_in_pcie_nodes(blob);
		fixup_mcu_gpio_in_phy_wan_node(blob);
	}

	fixup_msata_port_nodes(blob);
	fixup_wwan_port_nodes(blob);

	fixup_atsha_node(blob);

	return 0;
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int board_late_init(void)
{
	/*
	 * If not booting from UART, MCU watchdog was not disabled in SPL,
	 * disable it now.
	 */
	if (get_boot_device() != BOOT_DEVICE_UART)
		disable_mcu_watchdog();

	set_regdomain();
	handle_reset_button();
	pci_init();

	return 0;
}

int checkboard(void)
{
	char serial[17], version[4], pub_key[67];
	bool has_version;
	int err;

	printf("  MCU type: %s\n", omnia_get_mcu_type());
	printf("  MCU version: %s\n", omnia_get_mcu_version());
	printf("  RAM size: %i MiB\n", omnia_get_ram_size_gb() * 1024);

	if (omnia_mcu_has_feature(FEAT_BOARD_INFO)) {
		err = omnia_mcu_board_info(serial, NULL, version);
		has_version = !err;
	} else {
		err = turris_atsha_otp_get_serial_number(serial);
		has_version = false;
	}

	printf("  Board version: %s\n", has_version ? version : "unknown");
	printf("  Serial Number: %s\n", !err ? serial : "unknown");

	if (omnia_mcu_has_feature(FEAT_CRYPTO)) {
		err = omnia_mcu_get_board_public_key(pub_key);
		printf("  ECDSA Public Key: %s\n", !err ? pub_key : "unknown");
	}

	return 0;
}

int misc_init_r(void)
{
	if (omnia_mcu_has_feature(FEAT_BOARD_INFO)) {
		char serial[17];
		u8 first_mac[6];

		if (!omnia_mcu_board_info(serial, first_mac, NULL))
			turris_init_mac_addresses(1, first_mac);
	} else {
		turris_atsha_otp_init_mac_addresses(1);
		turris_atsha_otp_init_serial_number();
	}

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
/*
 * I plan to generalize this function and move it to common/fdt_support.c.
 * This will require some more work on multiple boards, though, so for now leave
 * it here.
 */
static bool fixup_mtd_partitions(void *blob, int offset, struct mtd_info *mtd)
{
	struct mtd_info *slave;
	int parts;

	parts = fdt_subnode_offset(blob, offset, "partitions");
	if (parts >= 0) {
		if (fdt_del_node(blob, parts) < 0)
			return false;
	}

	if (fdt_increase_size(blob, 512) < 0)
		return false;

	parts = fdt_add_subnode(blob, offset, "partitions");
	if (parts < 0)
		return false;

	if (fdt_setprop_u32(blob, parts, "#address-cells", 1) < 0)
		return false;

	if (fdt_setprop_u32(blob, parts, "#size-cells", 1) < 0)
		return false;

	if (fdt_setprop_string(blob, parts, "compatible",
			       "fixed-partitions") < 0)
		return false;

	mtd_probe_devices();

	list_for_each_entry_reverse(slave, &mtd->partitions, node) {
		char name[32];
		int part;

		snprintf(name, sizeof(name), "partition@%llx", slave->offset);
		part = fdt_add_subnode(blob, parts, name);
		if (part < 0)
			return false;

		if (fdt_setprop_u32(blob, part, "reg", slave->offset) < 0)
			return false;

		if (fdt_appendprop_u32(blob, part, "reg", slave->size) < 0)
			return false;

		if (fdt_setprop_string(blob, part, "label", slave->name) < 0)
			return false;

		if (!(slave->flags & MTD_WRITEABLE))
			if (fdt_setprop_empty(blob, part, "read-only") < 0)
				return false;

		if (slave->flags & MTD_POWERUP_LOCK)
			if (fdt_setprop_empty(blob, part, "lock") < 0)
				return false;
	}

	return true;
}

static void fixup_spi_nor_partitions(void *blob)
{
	struct mtd_info *mtd = NULL;
	char mtd_path[64];
	int node;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, -1, "jedec,spi-nor");
	if (node < 0)
		goto fail;

	if (fdt_get_path(gd->fdt_blob, node, mtd_path, sizeof(mtd_path)) < 0)
		goto fail;

	mtd = get_mtd_device_nm(mtd_path);
	if (IS_ERR_OR_NULL(mtd))
		goto fail;

	node = fdt_node_offset_by_compatible(blob, -1, "jedec,spi-nor");
	if (node < 0)
		goto fail;

	if (!fixup_mtd_partitions(blob, node, mtd))
		goto fail;

	put_mtd_device(mtd);
	return;

fail:
	printf("Failed fixing SPI NOR partitions!\n");
	if (!IS_ERR_OR_NULL(mtd))
		put_mtd_device(mtd);
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int node;

	/*
	 * U-Boot's FDT blob contains reset-gpios in ethernet2 PHY node when MCU
	 * controls all peripherals resets.
	 * Fixup MCU GPIO nodes in PCIe and eth wan nodes in this case.
	 */
	node = get_phy_wan_node_offset(gd->fdt_blob);
	if (node >= 0 && fdt_getprop(gd->fdt_blob, node, "reset-gpios", NULL)) {
		fixup_mcu_gpio_in_pcie_nodes(blob);
		fixup_mcu_gpio_in_phy_wan_node(blob);
	}

	fixup_spi_nor_partitions(blob);
	fixup_msata_port_nodes(blob);
	fixup_wwan_port_nodes(blob);

	fixup_atsha_node(blob);

	return 0;
}
#endif
