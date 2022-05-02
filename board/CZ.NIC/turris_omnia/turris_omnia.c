// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Marek Behun <marek.behun@nic.cz>
 * Copyright (C) 2016 Tomas Hlavacek <tomas.hlavacek@nic.cz>
 *
 * Derived from the code for
 *   Marvell/db-88f6820-gp by Stefan Roese <sr@denx.de>
 */

#include <common.h>
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
#include <dm/uclass.h>
#include <fdt_support.h>
#include <time.h>
#include <linux/bitops.h>
#include <u-boot/crc.h>

#include "../drivers/ddr/marvell/a38x/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>
#include "../turris_atsha_otp.h"

DECLARE_GLOBAL_DATA_PTR;

#define OMNIA_SPI_NOR_PATH		"/soc/spi@10600/spi-nor@0"

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

enum mcu_commands {
	CMD_GET_STATUS_WORD	= 0x01,
	CMD_GET_RESET		= 0x09,
	CMD_WATCHDOG_STATE	= 0x0b,
};

enum status_word_bits {
	CARD_DET_STSBIT		= 0x0010,
	MSATA_IND_STSBIT	= 0x0020,
};

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

	ret = omnia_mcu_write(CMD_WATCHDOG_STATE, "\x00", 1);
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
	u16 stsword;

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

	ret = omnia_mcu_read(CMD_GET_STATUS_WORD, &stsword, sizeof(stsword));
	if (ret) {
		printf("omnia_mcu_read failed: %i, defaulting to MiniPCIe card\n",
		       ret);
		return false;
	}

	if (!(stsword & CARD_DET_STSBIT)) {
		puts("none\n");
		return false;
	}

	if (stsword & MSATA_IND_STSBIT)
		puts("mSATA\n");
	else
		puts("MiniPCIe\n");

	return stsword & MSATA_IND_STSBIT ? true : false;
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

void *env_sf_get_env_addr(void)
{
	/* SPI Flash is mapped to address 0xD4000000 only in SPL */
#ifdef CONFIG_SPL_BUILD
	return (void *)0xD4000000 + CONFIG_ENV_OFFSET;
#else
	return NULL;
#endif
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
};

static bool omnia_read_eeprom(struct omnia_eeprom *oep)
{
	struct udevice *chip;
	u32 crc;
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

	crc = crc32(0, (void *)oep, sizeof(*oep) - 4);
	if (crc != oep->crc) {
		printf("bad EEPROM CRC (stored %08x, computed %08x)\n",
		       oep->crc, crc);
		return false;
	}

	return true;
}

static int omnia_get_ram_size_gb(void)
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

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
{
	if (omnia_get_ram_size_gb() == 2)
		return &board_topology_map_2g;
	else
		return &board_topology_map_1g;
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
		const char * const vars[2] = {
			"bootcmd",
			"distro_bootcmd",
		};

		/*
		 * Set the above envs to their default values, in case the user
		 * managed to break them.
		 */
		env_set_default_vars(2, (char * const *)vars, 0);

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

#endif

#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *blob)
{
	fixup_msata_port_nodes(blob);
	fixup_wwan_port_nodes(blob);

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

int show_board_info(void)
{
	u32 version_num, serial_num;
	int err;

	err = turris_atsha_otp_get_serial_number(&version_num, &serial_num);
	printf("Model: Turris Omnia\n");
	printf("  RAM size: %i MiB\n", omnia_get_ram_size_gb() * 1024);
	if (err)
		printf("  Serial Number: unknown\n");
	else
		printf("  Serial Number: %08X%08X\n", be32_to_cpu(version_num),
		       be32_to_cpu(serial_num));

	return 0;
}

int misc_init_r(void)
{
	turris_atsha_otp_init_mac_addresses(1);
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
	if (parts < 0)
		return false;

	if (fdt_del_node(blob, parts) < 0)
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
	struct mtd_info *mtd;
	int node;

	mtd = get_mtd_device_nm(OMNIA_SPI_NOR_PATH);
	if (IS_ERR_OR_NULL(mtd))
		goto fail;

	node = fdt_path_offset(blob, OMNIA_SPI_NOR_PATH);
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
	fixup_spi_nor_partitions(blob);
	fixup_msata_port_nodes(blob);
	fixup_wwan_port_nodes(blob);

	return 0;
}
#endif
