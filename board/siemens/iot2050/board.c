// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for IOT2050
 * Copyright (c) Siemens AG, 2018-2021
 *
 * Authors:
 *   Le Jin <le.jin@siemens.com>
 *   Jan Kiszka <jan.kiszka@siemens.com>
 */

#include <common.h>
#include <bootstage.h>
#include <dm.h>
#include <i2c.h>
#include <led.h>
#include <malloc.h>
#include <net.h>
#include <phy.h>
#include <spl.h>
#include <version.h>
#include <linux/delay.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/gpio.h>
#include <asm/io.h>

#define IOT2050_INFO_MAGIC		0x20502050

struct iot2050_info {
	u32 magic;
	u16 size;
	char name[20 + 1];
	char serial[16 + 1];
	char mlfb[18 + 1];
	char uuid[32 + 1];
	char a5e[18 + 1];
	u8 mac_addr_cnt;
	u8 mac_addr[8][ARP_HLEN];
	char seboot_version[40 + 1];
} __packed;

/*
 * Scratch SRAM (available before DDR RAM) contains extracted EEPROM data.
 */
#define IOT2050_INFO_DATA ((struct iot2050_info *) \
			     TI_SRAM_SCRATCH_BOARD_EEPROM_START)

DECLARE_GLOBAL_DATA_PTR;

static bool board_is_advanced(void)
{
	struct iot2050_info *info = IOT2050_INFO_DATA;

	return info->magic == IOT2050_INFO_MAGIC &&
		strstr((char *)info->name, "IOT2050-ADVANCED") != NULL;
}

static bool board_is_sr1(void)
{
	struct iot2050_info *info = IOT2050_INFO_DATA;

	return info->magic == IOT2050_INFO_MAGIC &&
		!strstr((char *)info->name, "-PG2");
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

void set_board_info_env(void)
{
	struct iot2050_info *info = IOT2050_INFO_DATA;
	u8 __maybe_unused mac_cnt;
	const char *fdtfile;

	if (info->magic != IOT2050_INFO_MAGIC) {
		pr_err("IOT2050: Board info parsing error!\n");
		return;
	}

	if (env_get("board_uuid"))
		return;

	env_set("board_name", info->name);
	env_set("board_serial", info->serial);
	env_set("mlfb", info->mlfb);
	env_set("board_uuid", info->uuid);
	env_set("board_a5e", info->a5e);
	env_set("fw_version", PLAIN_VERSION);
	env_set("seboot_version", info->seboot_version);

	if (IS_ENABLED(CONFIG_NET)) {
		/* set MAC addresses to ensure forwarding to the OS */
		for (mac_cnt = 0; mac_cnt < info->mac_addr_cnt; mac_cnt++) {
			if (is_valid_ethaddr(info->mac_addr[mac_cnt]))
				eth_env_set_enetaddr_by_index("eth",
							      mac_cnt + 1,
							      info->mac_addr[mac_cnt]);
		}
	}

	if (board_is_advanced()) {
		if (board_is_sr1())
			fdtfile = "ti/k3-am6548-iot2050-advanced.dtb";
		else
			fdtfile = "ti/k3-am6548-iot2050-advanced-pg2.dtb";
	} else {
		if (board_is_sr1())
			fdtfile = "ti/k3-am6528-iot2050-basic.dtb";
		else
			fdtfile = "ti/k3-am6528-iot2050-basic-pg2.dtb";
		/* remove the unavailable eMMC (mmc1) from the list */
		remove_mmc1_target();
	}
	env_set("fdtfile", fdtfile);

	env_save();
}

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	if (board_is_advanced())
		gd->ram_size = SZ_2G;
	else
		gd->ram_size = SZ_1G;

	return 0;
}

int dram_init_banksize(void)
{
	dram_init();

	/* Bank 0 declares the memory available in the DDR low region */
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;

	/* Bank 1 declares the memory available in the DDR high region */
	gd->bd->bi_dram[1].start = 0;
	gd->bd->bi_dram[1].size = 0;

	return 0;
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	struct iot2050_info *info = IOT2050_INFO_DATA;
	char upper_name[32];

	if (info->magic != IOT2050_INFO_MAGIC ||
	    strlen(name) >= sizeof(upper_name))
		return -1;

	str_to_upper(name, upper_name, sizeof(upper_name));
	if (!strcmp(upper_name, (char *)info->name))
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

	if (dm_gpio_lookup_name("25", &gpio) < 0 ||
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

	set_board_info_env();

	/* remove the eMMC if requested via button */
	if (IS_ENABLED(CONFIG_IOT2050_BOOT_SWITCH) && board_is_advanced() &&
	    user_button_pressed())
		remove_mmc1_target();

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int ret;

	ret = fdt_fixup_msmc_ram(blob, "/bus@100000", "sram@70000000");
	if (ret < 0)
		ret = fdt_fixup_msmc_ram(blob, "/interconnect@100000",
					 "sram@70000000");
	if (ret)
		pr_err("%s: fixing up msmc ram failed %d\n", __func__, ret);

	return ret;
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
