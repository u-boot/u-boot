// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Board specific initialization for Verdin AM62 SoM
 *
 * Copyright 2023 Toradex - https://www.toradex.com/
 *
 */

#include <config.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <k3-ddrss.h>
#include <power/regulator.h>
#include <spl.h>

#include "../common/tdx-cfg-block.h"

#define VDD_CORE_REG "buck1"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CFG_SYS_SDRAM_BASE, CFG_SYS_SDRAM_SIZE);

	if (gd->ram_size < SZ_512M)
		puts("## WARNING: Less than 512MB RAM detected\n");

	return 0;
}

/*
 * Avoid relocated U-Boot clash with Linux reserved-memory on 512 MB SoM
 */
phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	return 0x9C000000;
}

#if defined(CONFIG_SPL_LOAD_FIT)
int board_fit_config_name_match(const char *name)
{
	return 0;
}
#endif

static u32 get_vdd_core_nominal(void)
{
	int core_uvolt;

	switch (k3_get_speed_grade()) {
	case 'G':
	case 'K':
	case 'S':
		core_uvolt = 750000;
		break;
	case 'T':
	default:
		core_uvolt = 850000;
		break;
	}
	return core_uvolt;
}

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int core_uvolt;

	core_uvolt = get_vdd_core_nominal();
	if (core_uvolt != 850000) {
		do_fixup_by_path_u32(blob, "/bus@f0000/i2c@20000000/pmic@30/regulators/buck1",
				     "regulator-max-microvolt", core_uvolt, 0);
		do_fixup_by_path_u32(blob, "/bus@f0000/i2c@20000000/pmic@30/regulators/buck1",
				     "regulator-min-microvolt", core_uvolt, 0);
	}

	return ft_common_board_setup(blob, bd);
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
		is_wifi = (tdx_hw_tag.prodid == VERDIN_AM62Q_WIFI_BT_IT) ||
			  (tdx_hw_tag.prodid == VERDIN_AM62S_512MB_WIFI_BT_IT) ||
			  (tdx_hw_tag.prodid == VERDIN_AM62D_1G_WIFI_BT_IT) ||
			  (tdx_hw_tag.prodid == VERDIN_AM62Q_2G_WIFI_BT_IT);
	}

	if (is_wifi)
		strlcpy(&variant[0], "wifi", sizeof(variant));
	else
		strlcpy(&variant[0], "nonwifi", sizeof(variant));

	if (strcmp(variant, env_variant)) {
		printf("Setting variant to %s\n", variant);
		env_set("variant", variant);
	}
}

int board_late_init(void)
{
	int ret;
	int core_uvolt;
	struct udevice *dev = NULL;

	core_uvolt = get_vdd_core_nominal();
	if (core_uvolt != 850000) {
		/* Set CPU core voltage to 0.75V for slower speed grades */
		ret = regulator_get_by_devname(VDD_CORE_REG, &dev);
		if (ret)
			pr_err("VDD CORE Regulator get error: %d\n", ret);

		ret = regulator_set_value_force(dev, core_uvolt);
		if (ret)
			pr_err("VDD CORE Regulator value setting error: %d\n", ret);
	}

	select_dt_from_module_version();

	return 0;
}

#define CTRLMMR_USB0_PHY_CTRL		0x43004008
#define CTRLMMR_USB1_PHY_CTRL		0x43004018
#define CORE_VOLTAGE			0x80000000
#define MCU_CTRL_LFXOSC_32K_BYPASS_VAL	BIT(4)

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void)
{
	u32 val;

	/* Clear USB0_PHY_CTRL_CORE_VOLTAGE */
	/* TI recommends to clear the bit independent of VDDA_CORE_USB */
	val = readl(CTRLMMR_USB0_PHY_CTRL);
	val &= ~(CORE_VOLTAGE);
	writel(val, CTRLMMR_USB0_PHY_CTRL);

	/* Clear USB1_PHY_CTRL_CORE_VOLTAGE */
	val = readl(CTRLMMR_USB1_PHY_CTRL);
	val &= ~(CORE_VOLTAGE);
	writel(val, CTRLMMR_USB1_PHY_CTRL);

	/* We use the 32k FOUT from the Epson RX8130CE RTC chip */
	/* In WKUP_LFOSC0 clear the power down bit and set the bypass bit
	 * The bypass bit is required as we provide a CMOS clock signal and
	 * the power down seems to be required also in the bypass case
	 * despite of the datasheet stating otherwise
	 */
	/* Compare with the AM62 datasheet,
	 * Table 7-21. LFXOSC Modes of Operation
	 */
	val = readl(MCU_CTRL_LFXOSC_CTRL);
	val &= ~MCU_CTRL_LFXOSC_32K_DISABLE_VAL;
	val |= MCU_CTRL_LFXOSC_32K_BYPASS_VAL;
	writel(val, MCU_CTRL_LFXOSC_CTRL);
	/* Make sure to mux up to take the SoC 32k from the LFOSC input */
	writel(MCU_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL,
	       MCU_CTRL_DEVICE_CLKOUT_32K_CTRL);
}
#endif
