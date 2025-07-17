// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Board specific initialization for Verdin AM62P SoM
 *
 * Copyright 2025 Toradex - https://www.toradex.com/
 *
 */

#include <config.h>
#include <asm/arch/hardware.h>
#include <asm/arch/k3-common-fdt.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <k3-ddrss.h>
#include <spl.h>
#include <linux/sizes.h>

#include "../common/tdx-cfg-block.h"

DECLARE_GLOBAL_DATA_PTR;
static u8 hw_cfg;

static void read_hw_cfg(void)
{
	struct gpio_desc gpio_hw_cfg;
	static const int gpios[] = { 58, 61, 62 }; /* HW_CFG0, HW_CFG1, HW_CFG2 */
	char gpio_name[20];
	int i;

	printf("HW CFG: ");

	for (i = 0; i < ARRAY_SIZE(gpios); i++) {
		snprintf(gpio_name, sizeof(gpio_name), "gpio@600000_%d", gpios[i]);

		if (dm_gpio_lookup_name(gpio_name, &gpio_hw_cfg) < 0) {
			printf("Lookup error: GPIO %d\n", gpios[i]);
			continue;
		}

		if (dm_gpio_request(&gpio_hw_cfg, "hw_cfg")) {
			printf("GPIO request error: %d\n", gpios[i]);
			continue;
		}

		if (dm_gpio_get_value(&gpio_hw_cfg) == 1)
			hw_cfg |= BIT(i);

		dm_gpio_free(NULL, &gpio_hw_cfg);
	}

	printf("0x%02x\n", hw_cfg);
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CFG_SYS_SDRAM_BASE, CFG_SYS_SDRAM_SIZE);

	if (gd->ram_size < SZ_1G)
		puts("## WARNING: Less than 1GB RAM detected\n");

	return 0;
}

int dram_init_banksize(void)
{
	s32 ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		printf("Error setting up memory banksize. %d\n", ret);

	/* Use the detected RAM size, we only support 1 bank right now. */
	gd->bd->bi_dram[0].size = gd->ram_size;

	return ret;
}

#if IS_ENABLED(CONFIG_SPL_LOAD_FIT)
int board_fit_config_name_match(const char *name)
{
	return 0;
}
#endif

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif

#if IS_ENABLED(CONFIG_OF_BOARD_SETUP_EXTENDED)
void ft_board_setup_ex(void *blob, struct bd_info *bd)
{
	fdt_fixup_thermal_critical_trips_k3(blob, 105);
}
#endif

static void select_dt_from_module_version(void)
{
	char variant[32];
	char *env_variant = env_get("variant");
	int is_wifi = 0;

	if (IS_ENABLED(CONFIG_TDX_CFG_BLOCK)) {
		/*
		 * If we have a valid config block and it says we are a module with
		 * Wi-Fi/Bluetooth make sure we use the -wifi device tree.
		 */
		is_wifi = (tdx_hw_tag.prodid == VERDIN_AM62PQ_2G_WIFI_BT_IT);
	}

	if (is_wifi)
		strlcpy(&variant[0], "wifi", sizeof(variant));
	else
		strlcpy(&variant[0], "nonwifi", sizeof(variant));

	if (!env_variant || strcmp(variant, env_variant)) {
		printf("Setting variant to %s\n", variant);
		env_set("variant", variant);
	}
}

int board_late_init(void)
{
	select_dt_from_module_version();

	return 0;
}

#define MCU_CTRL_LFXOSC_32K_BYPASS_VAL	BIT(4)

void spl_board_init(void)
{
	u32 val;

	/*
	 * We use the 32k FOUT from the Epson RX8130CE RTC chip,
	 * configure LFXOSC accordingly, see AM62P datasheet,
	 * Table 6-23, LFXOSC Modes of Operation.
	 */
	val = readl(MCU_CTRL_LFXOSC_CTRL);
	val &= ~MCU_CTRL_LFXOSC_32K_DISABLE_VAL;
	val |= MCU_CTRL_LFXOSC_32K_BYPASS_VAL;
	writel(val, MCU_CTRL_LFXOSC_CTRL);
	/* Make sure to mux up to take the SoC 32k from the LFOSC input */
	writel(MCU_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL,
	       MCU_CTRL_DEVICE_CLKOUT_32K_CTRL);

	read_hw_cfg();
}
