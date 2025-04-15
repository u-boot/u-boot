// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for AM642 EVM
 *
 * Copyright (C) 2020-2022 Texas Instruments Incorporated - https://www.ti.com/
 *	Keerthy <j-keerthy@ti.com>
 *
 */

#include <efi_loader.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <k3-ddrss.h>
#include <spl.h>
#include <fdt_support.h>
#include <asm/arch/hardware.h>
#include <env.h>
#include <asm/arch/k3-ddr.h>

#include "../common/board_detect.h"
#include "../common/fdt_ops.h"

#define board_is_am64x_gpevm() (board_ti_k3_is("AM64-GPEVM") || \
				board_ti_k3_is("AM64-EVM") || \
				board_ti_k3_is("AM64-HSEVM"))

#define board_is_am64x_skevm() (board_ti_k3_is("AM64-SKEVM") || \
				board_ti_k3_is("AM64B-SKEVM"))

DECLARE_GLOBAL_DATA_PTR;

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = AM64X_SK_TIBOOT3_IMAGE_GUID,
		.fw_name = u"AM64X_SK_TIBOOT3",
		.image_index = 1,
	},
	{
		.image_type_id = AM64X_SK_SPL_IMAGE_GUID,
		.fw_name = u"AM64X_SK_SPL",
		.image_index = 2,
	},
	{
		.image_type_id = AM64X_SK_UBOOT_IMAGE_GUID,
		.fw_name = u"AM64X_SK_UBOOT",
		.image_index = 3,
	}
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "sf 0:0=tiboot3.bin raw 0 100000;"
	"tispl.bin raw 100000 200000;u-boot.img raw 300000 400000",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

int board_init(void)
{
	return 0;
}

#if defined(CONFIG_SPL_LOAD_FIT)
int board_fit_config_name_match(const char *name)
{
	bool eeprom_read = board_ti_was_eeprom_read();

	if (!eeprom_read || board_is_am64x_gpevm()) {
		if (!strcmp(name, "k3-am642-r5-evm") || !strcmp(name, "k3-am642-evm"))
			return 0;
	} else if (board_is_am64x_skevm()) {
		if (!strcmp(name, "k3-am642-r5-sk") || !strcmp(name, "k3-am642-sk"))
			return 0;
	}

	return -1;
}
#endif

#if defined(CONFIG_XPL_BUILD)
#if CONFIG_IS_ENABLED(USB_STORAGE)
static int fixup_usb_boot(const void *fdt_blob)
{
	int ret = 0;

	switch (spl_boot_device()) {
	case BOOT_DEVICE_USB:
		/*
		 * If the boot mode is host, fixup the dr_mode to host
		 * before cdns3 bind takes place
		 */
		ret = fdt_find_and_setprop((void *)fdt_blob,
					   "/bus@f4000/cdns-usb@f900000/usb@f400000",
					   "dr_mode", "host", 5, 0);
		if (ret)
			printf("%s: fdt_find_and_setprop() failed:%d\n",
			       __func__, ret);
		fallthrough;
	default:
		break;
	}

	return ret;
}
#endif

void spl_perform_fixups(struct spl_image_info *spl_image)
{
	if (IS_ENABLED(CONFIG_K3_DDRSS)) {
		if (IS_ENABLED(CONFIG_K3_INLINE_ECC))
			fixup_ddr_driver_for_ecc(spl_image);
	} else {
		fixup_memory_node(spl_image);
	}

#if CONFIG_IS_ENABLED(USB_STORAGE)
	fixup_usb_boot(spl_image->fdt_addr);
#endif
}
#endif

#ifdef CONFIG_TI_I2C_BOARD_DETECT
int do_board_detect(void)
{
	int ret;

	ret = ti_i2c_eeprom_am6_get_base(CONFIG_EEPROM_BUS_ADDRESS,
					 CONFIG_EEPROM_CHIP_ADDRESS);
	if (ret) {
		printf("EEPROM not available at 0x%02x, trying to read at 0x%02x\n",
			CONFIG_EEPROM_CHIP_ADDRESS, CONFIG_EEPROM_CHIP_ADDRESS + 1);
		ret = ti_i2c_eeprom_am6_get_base(CONFIG_EEPROM_BUS_ADDRESS,
						 CONFIG_EEPROM_CHIP_ADDRESS + 1);
		if (ret)
			pr_err("Reading on-board EEPROM at 0x%02x failed %d\n",
			       CONFIG_EEPROM_CHIP_ADDRESS + 1, ret);
	}

	return ret;
}

int checkboard(void)
{
	struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;

	if (!do_board_detect())
		printf("Board: %s rev %s\n", ep->name, ep->version);

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
static struct ti_fdt_map ti_am64_evm_fdt_map[] = {
	{"am64x_gpevm", "ti/k3-am642-evm.dtb"},
	{"am64x_skevm", "ti/k3-am642-sk.dtb"},
	{ /* Sentinel. */ }
};

static void setup_board_eeprom_env(void)
{
	char *name = "am64x_gpevm";

	if (do_board_detect())
		goto invalid_eeprom;

	if (board_is_am64x_gpevm())
		name = "am64x_gpevm";
	else if (board_is_am64x_skevm())
		name = "am64x_skevm";
	else
		printf("Unidentified board claims %s in eeprom header\n",
		       board_ti_get_name());

invalid_eeprom:
	set_board_info_env_am6(name);
	ti_set_fdt_env(name, ti_am64_evm_fdt_map);
}

static void setup_serial(void)
{
	struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;
	unsigned long board_serial;
	char *endp;
	char serial_string[17] = { 0 };

	if (env_get("serial#"))
		return;

	board_serial = hextoul(ep->serial, &endp);
	if (*endp != '\0') {
		pr_err("Error: Can't set serial# to %s\n", ep->serial);
		return;
	}

	snprintf(serial_string, sizeof(serial_string), "%016lx", board_serial);
	env_set("serial#", serial_string);
}
#endif
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_TI_I2C_BOARD_DETECT)) {
		struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;

		setup_board_eeprom_env();
		setup_serial();
		/*
		 * The first MAC address for ethernet a.k.a. ethernet0 comes from
		 * efuse populated via the am654 gigabit eth switch subsystem driver.
		 * All the other ones are populated via EEPROM, hence continue with
		 * an index of 1.
		 */
		board_ti_am6_set_ethaddr(1, ep->mac_addr_cnt);
	}

	return 0;
}
#endif

#define CTRLMMR_USB0_PHY_CTRL	0x43004008
#define CORE_VOLTAGE		0x80000000

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void)
{
	u32 val;
	/* Set USB PHY core voltage to 0.85V */
	val = readl(CTRLMMR_USB0_PHY_CTRL);
	val &= ~(CORE_VOLTAGE);
	writel(val, CTRLMMR_USB0_PHY_CTRL);

	/* Init DRAM size for R5/A53 SPL */
	dram_init_banksize();
}
#endif
