// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (C) 2025 Toradex */

#include <asm/arch/clock.h>
#include <asm/arch/mu.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/sections.h>
#include <asm/global_data.h>
#include <clk.h>
#include <dm/ofnode.h>
#include <dm/uclass.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

#define EC_I2C_BUS_NODE_PATH	"/soc/bus@42000000/i2c@42540000"

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	switch (boot_dev_spl) {
	case SD1_BOOT:
	case MMC1_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC2;
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	default:
		return BOOT_DEVICE_NONE;
	}
}

static void ec_boot_notify(void)
{
	struct udevice *bus;
	struct udevice *i2c_dev;
	ofnode node;
	int ret;
	u8 val = 0x03;

	node = ofnode_path(EC_I2C_BUS_NODE_PATH);
	if (!ofnode_valid(node)) {
		puts("Failed to find Toradex EC I2C BUS node\n");
		return;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_I2C, node, &bus);
	if (ret) {
		puts("Failed to get Toradex EC I2C BUS\n");
		return;
	}

	ret = dm_i2c_probe(bus, 0x28, 0, &i2c_dev);
	if (ret) {
		puts("Toradex EC not found\n");
		return;
	}

	/* USB configuration before this command (when SoC starts in recovery):
	 * - SoC USB1 -> Smarc USB0
	 * - SoC USB2 -> NC
	 * After the command:
	 * - SoC USB1 -> SoM hub -> Smarc USB1, USB2, USB3 and USB4
	 * - SoC USB2 -> Smarc USB0
	 */
	ret = dm_i2c_write(i2c_dev, 0xD0, &val, 1);
	if (ret)
		puts("Cannot send command to Toradex EC\n");
}

void spl_board_init(void)
{
	int ret;

	ret = ele_start_rng();
	if (ret)
		printf("Fail to start RNG: %d\n", ret);
}

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	if (IS_ENABLED(CONFIG_SPL_RECOVER_DATA_SECTION))
		spl_save_restore_data();

	timer_init();

	/* Need dm_init() to run before any SCMI calls */
	spl_early_init();

	/* Need to enable SCMI drivers and ELE driver before console */
	ret = imx9_probe_mu();
	if (ret)
		hang(); /* MU not probed, nothing can be outputed, hang */

	arch_cpu_init();

	board_early_init_f();

	preloader_console_init();

	debug("SOC: 0x%x\n", gd->arch.soc_rev);
	debug("LC: 0x%x\n", gd->arch.lifecycle);

	ec_boot_notify();
	get_reset_reason(true, false);

	board_init_r(NULL, 0);
}
