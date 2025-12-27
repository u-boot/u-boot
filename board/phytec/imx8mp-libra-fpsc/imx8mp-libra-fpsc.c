// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 PHYTEC Messtechnik GmbH
 */

#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <linux/io.h>
#include <asm/mach-imx/boot_mode.h>
#include <dwc3-uboot.h>
#include <env.h>
#include <init.h>
#include <fdt_support.h>
#include <jffs2/load_kernel.h>
#include <miiphy.h>
#include <mtd_node.h>
#include <usb.h>
#include <i2c.h>

#define EEPROM_ADDR		0x51

#define TUSB_PORT_POL_CRTL_REG	0xB
#define TUSB_CUSTOM_POL		BIT(7)
#define TUSB_P0_POL		BIT(0)

/*
 * WORKAROUND for PCM-937-L 1618.0, 1618.1.
 * USB HUB TUSB8042A has swapped upstream pin polarity.
 * Set i2c registers to inform the hub that the lines
 * are swapped.
 */
void tusb8042a_swap_lines(void)
{
	const u8 pol_swap_val = (TUSB_CUSTOM_POL | TUSB_P0_POL);
	const int addr = 0x44;
	struct udevice *dev = 0;
	int ret = i2c_get_chip_for_busnum(2, addr, 1, &dev);

	if (!ret)
		dm_i2c_write(dev, TUSB_PORT_POL_CRTL_REG, &pol_swap_val, 1);
	else
		printf("TUSB8042A: Failed to fixup USB HUB.\n");
}

int board_init(void)
{
	tusb8042a_swap_lines();

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int board_late_init(void)
{
	switch (get_boot_device()) {
	case SD2_BOOT:
		env_set_ulong("mmcdev", 1);
		if (!env_get("boot_targets"))
			env_set("boot_targets", "mmc1 mmc2 ethernet");
		break;
	case MMC3_BOOT:
		env_set_ulong("mmcdev", 2);
		break;
	case USB_BOOT:
		printf("Detect USB boot. Will enter fastboot mode!\n");
		if (!strcmp(env_get("bootcmd"), env_get_default("bootcmd")))
			env_set("bootcmd", "fastboot 0; bootflow scan -lb;");
		break;
	default:
		break;
	}

	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	if (!size)
		return -EINVAL;

	*size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE + PHYS_SDRAM_2_SIZE);

	return 0;
}
