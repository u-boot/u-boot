// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang<yanhong.wang@starfivetech.com>
 */

#include <asm/arch/eeprom.h>
#include <asm/arch/gpio.h>
#include <asm/arch/regs.h>
#include <asm/arch/spl.h>
#include <asm/io.h>
#include <dt-bindings/clock/starfive,jh7110-crg.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <log.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;
#define JH7110_CLK_CPU_ROOT_OFFSET		0x0U
#define JH7110_CLK_CPU_ROOT_SHIFT		24
#define JH7110_CLK_CPU_ROOT_MASK		GENMASK(29, 24)

void spl_perform_fixups(struct spl_image_info *spl_image)
{
	/* Update the memory size which read from eeprom or DT */
	fdt_fixup_memory(spl_image->fdt_addr, 0x40000000, gd->ram_size);
}

static void jh7110_jtag_init(void)
{
	/* nTRST: GPIO36 */
	SYS_IOMUX_DOEN(36, HIGH);
	SYS_IOMUX_DIN(36, 4);
	/* TDI: GPIO61 */
	SYS_IOMUX_DOEN(61, HIGH);
	SYS_IOMUX_DIN(61, 19);
	/* TMS: GPIO63 */
	SYS_IOMUX_DOEN(63, HIGH);
	SYS_IOMUX_DIN(63, 20);
	/* TCK: GPIO60 */
	SYS_IOMUX_DOEN(60, HIGH);
	SYS_IOMUX_DIN(60, 29);
	/* TDO: GPIO44 */
	SYS_IOMUX_DOEN(44, 8);
	SYS_IOMUX_DOUT(44, 22);
}

int spl_board_init_f(void)
{
	int ret;

	jh7110_jtag_init();

	ret = spl_dram_init();
	if (ret) {
		debug("JH7110 DRAM init failed: %d\n", ret);
		return ret;
	}

	return 0;
}

u32 spl_boot_device(void)
{
	u32 mode;

	mode = in_le32(JH7110_BOOT_MODE_SELECT_REG)
				& JH7110_BOOT_MODE_SELECT_MASK;
	switch (mode) {
	case 0:
		return BOOT_DEVICE_SPI;

	case 1:
		return BOOT_DEVICE_MMC2;

	case 2:
		return BOOT_DEVICE_MMC1;

	case 3:
		return BOOT_DEVICE_UART;

	default:
		debug("Unsupported boot device 0x%x.\n", mode);
		return BOOT_DEVICE_NONE;
	}
}

void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret)
		panic("spl_early_init() failed: %d\n", ret);

	riscv_cpu_setup();
	preloader_console_init();

	/* Set the parent clock of cpu_root clock to pll0,
	 * it must be initialized here
	 */
	clrsetbits_le32(JH7110_SYS_CRG + JH7110_CLK_CPU_ROOT_OFFSET,
			JH7110_CLK_CPU_ROOT_MASK,
			BIT(JH7110_CLK_CPU_ROOT_SHIFT));

	/* Set USB overcurrent overflow pin disable */
	SYS_IOMUX_DIN_DISABLED(2);

	ret = spl_board_init_f();
	if (ret) {
		debug("spl_board_init_f init failed: %d\n", ret);
		return;
	}
}

#if CONFIG_IS_ENABLED(LOAD_FIT)
int board_fit_config_name_match(const char *name)
{
	if (!strcmp(name, "starfive/jh7110-deepcomputing-fml13v01") &&
		    !strncmp(get_product_id_from_eeprom(), "FML13V01", 8)) {
		return 0;
	} else if (!strcmp(name, "starfive/jh7110-milkv-mars") &&
		    !strncmp(get_product_id_from_eeprom(), "MARS", 4)) {
		return 0;
	} else if (!strcmp(name, "starfive/jh7110-pine64-star64") &&
		    !strncmp(get_product_id_from_eeprom(), "STAR64", 6)) {
		return 0;
	} else if (!strcmp(name, "starfive/jh7110-starfive-visionfive-2-v1.2a") &&
		    !strncmp(get_product_id_from_eeprom(), "VF7110", 6)) {
		switch (get_pcb_revision_from_eeprom()) {
		case 'a':
		case 'A':
			return 0;
		}
	} else if (!strcmp(name, "starfive/jh7110-starfive-visionfive-2-v1.3b") &&
		    !strncmp(get_product_id_from_eeprom(), "VF7110", 6)) {
		switch (get_pcb_revision_from_eeprom()) {
		case 'b':
		case 'B':
			return 0;
		}
	}

	return -EINVAL;
}
#endif
