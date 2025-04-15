// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for IOT2050
 * Copyright (c) Siemens AG, 2018-2023
 *
 * Authors:
 *   Le Jin <le.jin@siemens.com>
 *   Jan Kiszka <jan.kiszka@siemens.com>
 */

#include <config.h>
#include <bootstage.h>
#include <dm.h>
#include <fdt_support.h>
#include <i2c.h>
#include <led.h>
#include <malloc.h>
#include <mapmem.h>
#include <net.h>
#include <phy.h>
#include <spl.h>
#include <version.h>
#include <linux/delay.h>
#include <asm/arch/hardware.h>
#include <asm/gpio.h>
#include <asm/io.h>

#include "../../../../drivers/sysinfo/iot2050.h"

DECLARE_GLOBAL_DATA_PTR;

struct gpio_config {
	const char *gpio_name;
	const char *label;
};

enum m2_connector_mode {
	BKEY_PCIEX2 = 0,
	BKEY_PCIE_EKEY_PCIE,
	BKEY_USB30_EKEY_PCIE,
	CONNECTOR_MODE_INVALID
};

struct m2_config_pins {
	int config[4];
};

struct serdes_mux_control {
	int ctrl_usb30_pcie0_lane0;
	int ctrl_pcie1_pcie0;
	int ctrl_usb30_pcie0_lane1;
};

struct m2_config_table {
	struct m2_config_pins config_pins;
	enum m2_connector_mode mode;
};

static const struct gpio_config serdes_mux_ctl_pin_info[] = {
	{"gpio@600000_88", "CTRL_USB30_PCIE0_LANE0"},
	{"gpio@600000_82", "CTRL_PCIE1_PCIE0"},
	{"gpio@600000_89", "CTRL_USB30_PCIE0_LANE1"},
};

static const struct gpio_config m2_bkey_cfg_pin_info[] = {
	{"gpio@601000_18", "KEY_CONFIG_0"},
	{"gpio@601000_19", "KEY_CONFIG_1"},
	{"gpio@601000_88", "KEY_CONFIG_2"},
	{"gpio@601000_89", "KEY_CONFIG_3"},
};

static const struct m2_config_table m2_config_table[] = {
	{{{0, 1, 0, 0}}, BKEY_PCIEX2},
	{{{0, 0, 1, 0}}, BKEY_PCIE_EKEY_PCIE},
	{{{0, 1, 1, 0}}, BKEY_PCIE_EKEY_PCIE},
	{{{1, 0, 0, 1}}, BKEY_PCIE_EKEY_PCIE},
	{{{1, 1, 0, 1}}, BKEY_PCIE_EKEY_PCIE},
	{{{0, 0, 0, 1}}, BKEY_USB30_EKEY_PCIE},
	{{{0, 1, 0, 1}}, BKEY_USB30_EKEY_PCIE},
	{{{0, 0, 1, 1}}, BKEY_USB30_EKEY_PCIE},
	{{{0, 1, 1, 1}}, BKEY_USB30_EKEY_PCIE},
	{{{1, 0, 1, 1}}, BKEY_USB30_EKEY_PCIE},
};

static const struct serdes_mux_control serdes_mux_ctrl[] = {
	[BKEY_PCIEX2]          = {0, 0, 1},
	[BKEY_PCIE_EKEY_PCIE]  = {0, 1, 0},
	[BKEY_USB30_EKEY_PCIE] = {1, 1, 0},
};

static const char *m2_connector_mode_name[] = {
	[BKEY_PCIEX2]          = "PCIe x2 (key B)",
	[BKEY_PCIE_EKEY_PCIE]  = "PCIe (key B) / PCIe (key E)",
	[BKEY_USB30_EKEY_PCIE] = "USB 3.0 (key B) / PCIe (key E)",
};

static enum m2_connector_mode connector_mode;

static char iot2050_board_name[21];

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
static void *connector_overlay;
static u32 connector_overlay_size;
#endif

static int get_pinvalue(const char *gpio_name, const char *label)
{
	struct gpio_desc gpio;

	if (dm_gpio_lookup_name(gpio_name, &gpio) < 0 ||
	    dm_gpio_request(&gpio, label) < 0 ||
	    dm_gpio_set_dir_flags(&gpio, GPIOD_IS_IN) < 0) {
		pr_err("Cannot get pin %s for M.2 configuration\n", gpio_name);
		return 0;
	}

	return dm_gpio_get_value(&gpio);
}

static void set_pinvalue(const char *gpio_name, const char *label, int value)
{
	struct gpio_desc gpio;

	if (dm_gpio_lookup_name(gpio_name, &gpio) < 0 ||
	    dm_gpio_request(&gpio, label) < 0 ||
	    dm_gpio_set_dir_flags(&gpio, GPIOD_IS_OUT) < 0) {
		pr_err("Cannot set pin %s for M.2 configuration\n", gpio_name);
		return;
	}
	dm_gpio_set_value(&gpio, value);
}

static bool setup_sysinfo(struct udevice **sysinfo_ptr)
{
	if (sysinfo_get(sysinfo_ptr)) {
		pr_err("Could not find sysinfo device.\n");
		return false;
	}
	if (sysinfo_detect(*sysinfo_ptr)) {
		pr_err("Board info parsing error\n");
		return false;
	}
	return true;
}

static void get_board_name(void)
{
	struct udevice *sysinfo;

	if (iot2050_board_name[0] != 0)
		return;

	if (!setup_sysinfo(&sysinfo))
		return;

	sysinfo_get_str(sysinfo, BOARD_NAME, sizeof(iot2050_board_name),
			iot2050_board_name);
}

static bool board_is_advanced(void)
{

	get_board_name();
	return strstr(iot2050_board_name, "IOT2050-ADVANCED") != NULL;
}

static bool board_is_pg1(void)
{
	get_board_name();
	return strcmp(iot2050_board_name, "IOT2050-BASIC") == 0 ||
	       strcmp(iot2050_board_name, "IOT2050-ADVANCED") == 0;
}

static bool board_is_m2(void)
{
	get_board_name();
	return strcmp(iot2050_board_name, "IOT2050-ADVANCED-M2") == 0;
}

static bool board_is_sm(void)
{
	get_board_name();
	return strcmp(iot2050_board_name, "IOT2050-ADVANCED-SM") == 0;
}

static void remove_mmc1_target(void)
{
	char *boot_targets = strdup(env_get("boot_targets"));
	char *mmc1 = strstr(boot_targets, "mmc1");

	if (mmc1) {
		memmove(mmc1, mmc1 + 4, strlen(mmc1 + 4) + 1);
		env_set("boot_targets", boot_targets);
	}

	free(boot_targets);
}

static void enable_pcie_connector_power(void)
{
	if (board_is_sm())
		set_pinvalue("gpio@601000_22", "P3V3_PCIE_CON_EN", 1);
	else
		set_pinvalue("gpio@601000_17", "P3V3_PCIE_CON_EN", 1);
	udelay(4 * 100);
}

void set_board_info_env(void)
{
	struct udevice *sysinfo;
	const char *fdtfile;
	char buf[41];

	if (env_get("board_uuid"))
		return;

	if (!setup_sysinfo(&sysinfo))
		return;

	if (sysinfo_get_str(sysinfo, BOARD_NAME, sizeof(buf), buf) == 0)
		env_set("board_name", buf);
	if (sysinfo_get_str(sysinfo, SYSID_SM_SYSTEM_SERIAL, sizeof(buf), buf) == 0)
		env_set("board_serial", buf);
	if (sysinfo_get_str(sysinfo, BOARD_MLFB, sizeof(buf), buf) == 0)
		env_set("mlfb", buf);
	if (sysinfo_get_str(sysinfo, BOARD_UUID, sizeof(buf), buf) == 0)
		env_set("board_uuid", buf);
	if (sysinfo_get_str(sysinfo, BOARD_A5E, sizeof(buf), buf) == 0)
		env_set("board_a5e", buf);
	if (sysinfo_get_str(sysinfo, BOARD_SEBOOT_VER, sizeof(buf), buf) == 0)
		env_set("seboot_version", buf);
	env_set("fw_version", PLAIN_VERSION);

	if (IS_ENABLED(CONFIG_NET)) {
		int mac_cnt;

		mac_cnt = sysinfo_get_item_count(sysinfo, SYSID_BOARD_MAC_ADDR);
		/* set MAC addresses to ensure forwarding to the OS */
		for (int i = 0; i < mac_cnt; i++) {
			u8 *mac = NULL;
			size_t bytes = 0;

			sysinfo_get_data_by_index(sysinfo, SYSID_BOARD_MAC_ADDR,
						  i, (void **)&mac, &bytes);
			if (bytes == ARP_HLEN && is_valid_ethaddr(mac))
				eth_env_set_enetaddr_by_index("eth", i + 1, mac);
		}
	}

	if (board_is_advanced()) {
		if (board_is_pg1())
			fdtfile = "ti/k3-am6548-iot2050-advanced.dtb";
		else if (board_is_m2())
			fdtfile = "ti/k3-am6548-iot2050-advanced-m2.dtb";
		else if (board_is_sm())
			fdtfile = "ti/k3-am6548-iot2050-advanced-sm.dtb";
		else
			fdtfile = "ti/k3-am6548-iot2050-advanced-pg2.dtb";
	} else {
		if (board_is_pg1())
			fdtfile = "ti/k3-am6528-iot2050-basic.dtb";
		else
			fdtfile = "ti/k3-am6528-iot2050-basic-pg2.dtb";
		/* remove the unavailable eMMC (mmc1) from the list */
		remove_mmc1_target();
	}
	env_set("fdtfile", fdtfile);

	env_save();
}

static void do_overlay_prepare(const char *overlay_path)
{
#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
	void *overlay;
	u64 loadaddr;
	ofnode node;
	int ret;

	node = ofnode_path(overlay_path);
	if (!ofnode_valid(node))
		goto fit_error;

	ret = ofnode_read_u64(node, "load", &loadaddr);
	if (ret)
		goto fit_error;

	ret = ofnode_read_u32(node, "size", &connector_overlay_size);
	if (ret)
		goto fit_error;

	overlay = map_sysmem(loadaddr, connector_overlay_size);

	connector_overlay = malloc(connector_overlay_size);
	if (!connector_overlay)
		goto fit_error;

	memcpy(connector_overlay, overlay, connector_overlay_size);
	return;

fit_error:
	pr_err("M.2 device tree overlay %s not available.\n", overlay_path);
#endif
}

static void m2_overlay_prepare(void)
{
	const char *overlay_path;

	if (connector_mode == BKEY_PCIEX2)
		return;

	if (connector_mode == BKEY_PCIE_EKEY_PCIE)
		overlay_path = "/fit-images/bkey-ekey-pcie-overlay";
	else
		overlay_path = "/fit-images/bkey-usb3-overlay";

	do_overlay_prepare(overlay_path);
}

static void m2_connector_setup(void)
{
	ulong m2_manual_config = env_get_ulong("m2_manual_config", 10,
					       CONNECTOR_MODE_INVALID);
	const char *mode_info = "";
	struct m2_config_pins config_pins;
	unsigned int n;

	if (m2_manual_config < CONNECTOR_MODE_INVALID) {
		mode_info = " [manual mode]";
		connector_mode = m2_manual_config;
	} else { /* auto detection */
		for (n = 0; n < ARRAY_SIZE(config_pins.config); n++)
			config_pins.config[n] =
				get_pinvalue(m2_bkey_cfg_pin_info[n].gpio_name,
					     m2_bkey_cfg_pin_info[n].label);
		connector_mode = CONNECTOR_MODE_INVALID;
		for (n = 0; n < ARRAY_SIZE(m2_config_table); n++) {
			if (!memcmp(config_pins.config,
				    m2_config_table[n].config_pins.config,
				    sizeof(config_pins.config))) {
				connector_mode = m2_config_table[n].mode;
				break;
			}
		}
		if (connector_mode == CONNECTOR_MODE_INVALID) {
			mode_info = " [fallback, card unknown/unsupported]";
			connector_mode = BKEY_USB30_EKEY_PCIE;
		}
	}

	printf("M.2:   %s%s\n", m2_connector_mode_name[connector_mode],
	       mode_info);

	/* configure serdes mux */
	set_pinvalue(serdes_mux_ctl_pin_info[0].gpio_name,
		     serdes_mux_ctl_pin_info[0].label,
		     serdes_mux_ctrl[connector_mode].ctrl_usb30_pcie0_lane0);
	set_pinvalue(serdes_mux_ctl_pin_info[1].gpio_name,
		     serdes_mux_ctl_pin_info[1].label,
		     serdes_mux_ctrl[connector_mode].ctrl_pcie1_pcie0);
	set_pinvalue(serdes_mux_ctl_pin_info[2].gpio_name,
		     serdes_mux_ctl_pin_info[2].label,
		     serdes_mux_ctrl[connector_mode].ctrl_usb30_pcie0_lane1);

	m2_overlay_prepare();
}

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	struct udevice *sysinfo;
	u32 ddr_size_mb;

	if (!setup_sysinfo(&sysinfo))
		return -ENODEV;

	sysinfo_get_int(sysinfo, SYSID_BOARD_RAM_SIZE_MB, &ddr_size_mb);

	gd->ram_size = ((phys_size_t)(ddr_size_mb)) << 20;

	return 0;
}

ulong board_get_usable_ram_top(ulong total_size)
{
	/* Limit RAM used by U-Boot to the DDR low region */
	if (gd->ram_top > 0x100000000)
		return 0x100000000;

	return gd->ram_top;
}

int dram_init_banksize(void)
{
	dram_init();

	if (gd->ram_size > SZ_2G) {
		/* Bank 0 declares the memory available in the DDR low region */
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = SZ_2G;

		/* Bank 1 declares the memory available in the DDR high region */
		gd->bd->bi_dram[1].start = CFG_SYS_SDRAM_BASE1;
		gd->bd->bi_dram[1].size = gd->ram_size - SZ_2G;
	} else {
		/* Bank 0 declares the memory available in the DDR low region */
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = gd->ram_size;

		/* Bank 1 declares the memory available in the DDR high region */
		gd->bd->bi_dram[1].start = 0;
		gd->bd->bi_dram[1].size = 0;
	}

	return 0;
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	char upper_name[32];

	get_board_name();

	/* skip the prefix "ti/k3-am65x8-" */
	name += 13;

	if (strlen(name) >= sizeof(upper_name))
		return -1;

	str_to_upper(name, upper_name, sizeof(upper_name));
	if (!strcmp(upper_name, iot2050_board_name))
		return 0;

	return -1;
}
#endif

int do_board_detect(void)
{
	return 0;
}

#ifdef CONFIG_IOT2050_BOOT_SWITCH
static bool user_button_pressed(void)
{
	struct udevice *red_led = NULL;
	unsigned long count = 0;
	struct gpio_desc gpio;

	memset(&gpio, 0, sizeof(gpio));

	if (dm_gpio_lookup_name("gpio@42110000_25", &gpio) < 0 ||
	    dm_gpio_request(&gpio, "USER button") < 0 ||
	    dm_gpio_set_dir_flags(&gpio, GPIOD_IS_IN) < 0)
		return false;

	if (dm_gpio_get_value(&gpio) == 1)
		return false;

	printf("USER button pressed - booting from external media only\n");

	led_get_by_label("status-led-red", &red_led);

	if (red_led)
		led_set_state(red_led, LEDST_ON);

	while (dm_gpio_get_value(&gpio) == 0 && count++ < 10000)
		mdelay(1);

	if (red_led)
		led_set_state(red_led, LEDST_OFF);

	return true;
}
#endif

#define SERDES0_LANE_SELECT	0x00104080

int board_late_init(void)
{
	/* change CTRL_MMR register to let serdes0 not output USB3.0 signals. */
	writel(0x3, SERDES0_LANE_SELECT);

	enable_pcie_connector_power();

	if (board_is_m2())
		m2_connector_setup();

	set_board_info_env();

	/* remove the eMMC if requested via button */
	if (IS_ENABLED(CONFIG_IOT2050_BOOT_SWITCH) && board_is_advanced() &&
	    user_button_pressed())
		remove_mmc1_target();

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
static void variants_fdt_fixup(void *blob)
{
	void *overlay_copy = NULL;
	void *fdt_copy = NULL;
	u32 fdt_size;
	int err;

	if (!connector_overlay)
		return;

	/*
	 * We need to work with temporary copies here because fdt_overlay_apply
	 * is destructive to the overlay and also to the target blob, even if
	 * application fails.
	 */
	fdt_size = fdt_totalsize(blob);
	fdt_copy = malloc(fdt_size);
	if (!fdt_copy)
		goto fixup_error;

	memcpy(fdt_copy, blob, fdt_size);

	overlay_copy = malloc(connector_overlay_size);
	if (!overlay_copy)
		goto fixup_error;

	memcpy(overlay_copy, connector_overlay, connector_overlay_size);

	err = fdt_overlay_apply_verbose(fdt_copy, overlay_copy);
	if (err)
		goto fixup_error;

	memcpy(blob, fdt_copy, fdt_size);

cleanup:
	free(fdt_copy);
	free(overlay_copy);
	return;

fixup_error:
	pr_err("Could not apply device tree overlay\n");
	goto cleanup;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	if (board_is_m2())
		variants_fdt_fixup(blob);

	return 0;
}
#endif

void spl_board_init(void)
{
}

#if CONFIG_IS_ENABLED(LED) && CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
/*
 * Indicate any error or (accidental?) entering of CLI via the red status LED.
 */
void show_boot_progress(int progress)
{
	struct udevice *dev;
	int ret;

	if ((progress < 0 && progress != -BOOTSTAGE_ID_NET_ETH_START) ||
	    progress == BOOTSTAGE_ID_ENTER_CLI_LOOP) {
		ret = led_get_by_label("status-led-green", &dev);
		if (ret == 0)
			led_set_state(dev, LEDST_OFF);

		ret = led_get_by_label("status-led-red", &dev);
		if (ret == 0)
			led_set_state(dev, LEDST_ON);
	}
}
#endif
