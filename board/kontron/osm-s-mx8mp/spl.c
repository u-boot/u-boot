// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Kontron Electronics GmbH
 */

#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <power/pca9450.h>
#include <power/pmic.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	if (IS_ENABLED(CONFIG_SPL_BOOTROM_SUPPORT))
		return BOOT_DEVICE_BOOTROM;

	switch (boot_dev_spl) {
	case SD1_BOOT:
	case MMC1_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC2;
	default:
		return BOOT_DEVICE_NONE;
	}
}

bool check_ram_available(long size)
{
	long sz = get_ram_size((long *)PHYS_SDRAM, size);

	if (sz == size)
		return true;

	return false;
}

static void spl_dram_init(void)
{
	u32 size = 0;
	int i, ret;

	/*
	 * First check the largest possible DDR size of 8GB, then try the lower
	 * configs.
	 */
	ret = ddr_init(&dram_timing);
	if (!ret && check_ram_available(SZ_4G << 1))
		size = 8;
	// assume 4GB if ddr_init() succeeds but 8GB can not be addressed
	else if (!ret) {
		/*
		 * For some strange, unknown reason freezes are occurring if we run
		 * check_ram_available() after the second call to ddr_init(). This
		 * doesn't seem to be an issue with caching as it also happens with
		 * caches disable. The workaround that does the trick is accessing the
		 * DDR areas once more before calling ddr_init() and
		 * check_ram_available() again. This can be done by rerunning the size
		 * check on the first few bytes of the DDR.
		 * On the other hand this breaks the 1GB DDR init, but we can skip this
		 * as the 1GB is single rank and the training fails. We can detect that
		 * by checking the return code of ddr_init().
		 */
		check_ram_available(256);

		/*
		 * Overwrite some values to comply with the 4GB DDR.
		 */
		dram_timing.ddrc_cfg[5].val = 0x7a0118;
		dram_timing.ddrc_cfg[12].val = 0x650048;
		dram_timing.ddrc_cfg[13].val = 0x140048;
		dram_timing.ddrc_cfg[23].val = 0x120;
		dram_timing.ddrc_cfg[39].val = 0x17;
		dram_timing.ddrc_cfg[46].val = 0xf0f;
		dram_timing.ddrc_cfg[62].val = 0xc001c;
		dram_timing.ddrc_cfg[66].val = 0x160048;
		dram_timing.ddrc_cfg[77].val = 0x1d;
		dram_timing.ddrc_cfg[87].val = 0x30007;
		dram_timing.ddrc_cfg[91].val = 0x160048;
		dram_timing.ddrc_cfg[102].val = 0x8;

		for (i = 114; i < 138; i++)
			dram_timing.ddrphy_cfg[i].val = 0xe38;

		dram_timing.ddrphy_cfg[155].val = 0xdc;
		dram_timing.ddrphy_cfg[164].val = 0xdc;
		dram_timing.ddrphy_cfg[173].val = 0xdc;

		dram_timing.fsp_msg[0].fsp_cfg[3].val = 0x283c;
		dram_timing.fsp_msg[0].fsp_cfg[4].val = 0x11;
		dram_timing.fsp_msg[0].fsp_cfg[12].val = 0x4865;
		dram_timing.fsp_msg[0].fsp_cfg[13].val = 0x4800;
		dram_timing.fsp_msg[0].fsp_cfg[17].val = 0x4865;
		dram_timing.fsp_msg[0].fsp_cfg[18].val = 0x4800;
		dram_timing.fsp_msg[0].fsp_cfg[24].val = 0x6500;
		dram_timing.fsp_msg[0].fsp_cfg[26].val = 0x48;
		dram_timing.fsp_msg[0].fsp_cfg[30].val = 0x6500;
		dram_timing.fsp_msg[0].fsp_cfg[32].val = 0x48;

		dram_timing.fsp_msg[1].fsp_cfg[4].val = 0x283c;
		dram_timing.fsp_msg[1].fsp_cfg[5].val = 0x11;
		dram_timing.fsp_msg[1].fsp_cfg[14].val = 0x4800;
		dram_timing.fsp_msg[1].fsp_cfg[19].val = 0x4800;
		dram_timing.fsp_msg[1].fsp_cfg[27].val = 0x48;
		dram_timing.fsp_msg[1].fsp_cfg[33].val = 0x48;

		dram_timing.fsp_msg[2].fsp_cfg[4].val = 0x283c;
		dram_timing.fsp_msg[2].fsp_cfg[5].val = 0x11;
		dram_timing.fsp_msg[2].fsp_cfg[14].val = 0x4800;
		dram_timing.fsp_msg[2].fsp_cfg[19].val = 0x4800;
		dram_timing.fsp_msg[2].fsp_cfg[27].val = 0x48;
		dram_timing.fsp_msg[2].fsp_cfg[33].val = 0x48;

		dram_timing.fsp_msg[3].fsp_cfg[3].val = 0x283c;
		dram_timing.fsp_msg[3].fsp_cfg[4].val = 0x11;
		dram_timing.fsp_msg[3].fsp_cfg[14].val = 0x4865;
		dram_timing.fsp_msg[3].fsp_cfg[15].val = 0x4800;
		dram_timing.fsp_msg[3].fsp_cfg[19].val = 0x4865;
		dram_timing.fsp_msg[3].fsp_cfg[20].val = 0x4800;
		dram_timing.fsp_msg[3].fsp_cfg[26].val = 0x6500;
		dram_timing.fsp_msg[3].fsp_cfg[28].val = 0x48;
		dram_timing.fsp_msg[3].fsp_cfg[32].val = 0x6500;
		dram_timing.fsp_msg[3].fsp_cfg[34].val = 0x48;

		if (!ddr_init(&dram_timing) && check_ram_available(SZ_4G))
			size = 4;
	}
	// assume 1GB if 8GB training failed
	else if (size == 0) {
		/*
		 * Overwrite some values to comply with the 1GB DDR.
		 */
		dram_timing.ddrc_cfg[2].val = 0xa1080020;
		dram_timing.ddrc_cfg[5].val = 0x7a00b4;
		dram_timing.ddrc_cfg[12].val = 0x550048;
		dram_timing.ddrc_cfg[13].val = 0x15002f;
		dram_timing.ddrc_cfg[23].val = 0xbc;
		dram_timing.ddrc_cfg[39].val = 0x1f;
		dram_timing.ddrc_cfg[45].val = 0xf070707;
		dram_timing.ddrc_cfg[62].val = 0xc0012;
		dram_timing.ddrc_cfg[66].val = 0x16002f;
		dram_timing.ddrc_cfg[77].val = 0x13;
		dram_timing.ddrc_cfg[87].val = 0x30005;
		dram_timing.ddrc_cfg[91].val = 0x16002f;
		dram_timing.ddrc_cfg[102].val = 0x5;

		for (i = 90; i < 114; i++)
			dram_timing.ddrphy_cfg[i].val = 0x680;

		for (; i < 138; i++)
			dram_timing.ddrphy_cfg[i].val = 0x69a;

		dram_timing.ddrphy_cfg[155].val = 0x104;
		dram_timing.ddrphy_cfg[164].val = 0x104;
		dram_timing.ddrphy_cfg[173].val = 0x104;

		dram_timing.fsp_msg[0].fsp_cfg[3].val = 0x3030;
		dram_timing.fsp_msg[0].fsp_cfg[4].val = 0x14;
		dram_timing.fsp_msg[0].fsp_cfg[9].val = 0x110;
		dram_timing.fsp_msg[0].fsp_cfg[12].val = 0x4855;
		dram_timing.fsp_msg[0].fsp_cfg[13].val = 0x2f00;
		dram_timing.fsp_msg[0].fsp_cfg[14].val = 0x15;
		dram_timing.fsp_msg[0].fsp_cfg[17].val = 0x4855;
		dram_timing.fsp_msg[0].fsp_cfg[18].val = 0x2f00;
		dram_timing.fsp_msg[0].fsp_cfg[19].val = 0x15;
		dram_timing.fsp_msg[0].fsp_cfg[21].val = 0x1;
		dram_timing.fsp_msg[0].fsp_cfg[24].val = 0x5500;
		dram_timing.fsp_msg[0].fsp_cfg[26].val = 0x2f;
		dram_timing.fsp_msg[0].fsp_cfg[27].val = 0x1500;
		dram_timing.fsp_msg[0].fsp_cfg[30].val = 0x5500;
		dram_timing.fsp_msg[0].fsp_cfg[32].val = 0x2f;
		dram_timing.fsp_msg[0].fsp_cfg[33].val = 0x1500;

		dram_timing.fsp_msg[1].fsp_cfg[4].val = 0x3030;
		dram_timing.fsp_msg[1].fsp_cfg[5].val = 0x14;
		dram_timing.fsp_msg[1].fsp_cfg[10].val = 0x110;
		dram_timing.fsp_msg[1].fsp_cfg[14].val = 0x2f00;
		dram_timing.fsp_msg[1].fsp_cfg[19].val = 0x2f00;
		dram_timing.fsp_msg[1].fsp_cfg[22].val = 0x1;
		dram_timing.fsp_msg[1].fsp_cfg[27].val = 0x2f;
		dram_timing.fsp_msg[1].fsp_cfg[33].val = 0x2f;

		dram_timing.fsp_msg[2].fsp_cfg[4].val = 0x3030;
		dram_timing.fsp_msg[2].fsp_cfg[5].val = 0x14;
		dram_timing.fsp_msg[2].fsp_cfg[10].val = 0x110;
		dram_timing.fsp_msg[2].fsp_cfg[14].val = 0x2f00;
		dram_timing.fsp_msg[2].fsp_cfg[19].val = 0x2f00;
		dram_timing.fsp_msg[2].fsp_cfg[22].val = 0x1;
		dram_timing.fsp_msg[2].fsp_cfg[27].val = 0x2f;
		dram_timing.fsp_msg[2].fsp_cfg[33].val = 0x2f;

		dram_timing.fsp_msg[3].fsp_cfg[3].val = 0x3030;
		dram_timing.fsp_msg[3].fsp_cfg[4].val = 0x14;
		dram_timing.fsp_msg[3].fsp_cfg[11].val = 0x110;
		dram_timing.fsp_msg[3].fsp_cfg[14].val = 0x4855;
		dram_timing.fsp_msg[3].fsp_cfg[15].val = 0x2f00;
		dram_timing.fsp_msg[3].fsp_cfg[16].val = 0x15;
		dram_timing.fsp_msg[3].fsp_cfg[19].val = 0x4855;
		dram_timing.fsp_msg[3].fsp_cfg[20].val = 0x2f00;
		dram_timing.fsp_msg[3].fsp_cfg[21].val = 0x15;
		dram_timing.fsp_msg[3].fsp_cfg[23].val = 0x1;
		dram_timing.fsp_msg[3].fsp_cfg[26].val = 0x5500;
		dram_timing.fsp_msg[3].fsp_cfg[28].val = 0x2f;
		dram_timing.fsp_msg[3].fsp_cfg[29].val = 0x1500;
		dram_timing.fsp_msg[3].fsp_cfg[32].val = 0x5500;
		dram_timing.fsp_msg[3].fsp_cfg[34].val = 0x2f;
		dram_timing.fsp_msg[3].fsp_cfg[35].val = 0x1500;

		if (!ddr_init(&dram_timing) && check_ram_available(SZ_1G))
			size = 1;
	}

	if (size == 0) {
		printf("Failed to initialize DDR RAM!\n");
		size = 1;
	} else {
		printf("DDR:   LPDDR4 initialized (%dGB)\n", size);
	}

	gd->ram_size = size;
}

void spl_board_init(void)
{
	struct udevice *dev;
	int ret;

	if (IS_ENABLED(CONFIG_FSL_CAAM)) {
		ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(caam_jr), &dev);
		if (ret)
			printf("Failed to initialize %s: %d\n", dev->name, ret);
	}

	/*
	 * Set GIC clock to 500Mhz for OD VDD_SOC. Kernel driver does
	 * not allow to change it. Should set the clock after PMIC
	 * setting done. Default is 400Mhz (system_pll1_800m with div = 2)
	 * set by ROM for ND VDD_SOC
	 */
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);
}

static int power_init_board(void)
{
	struct udevice *dev;
	int ret  = pmic_get("pmic@25", &dev);

	if (ret == -ENODEV)
		puts("No pmic found\n");

	if (ret)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/* Increase VDD_SOC and VDD_ARM to OD voltage 0.95V */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1C);
	pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1C);

	/* Set BUCK1 DVS1 to suspend controlled through PMIC_STBY_REQ */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* Set WDOG_B_CFG to cold reset */
	pmic_reg_write(dev, PCA9450_RESET_CTRL, 0xA1);

	/* Enable I2C level translator */
	pmic_reg_write(dev, PCA9450_CONFIG2, 0x01);

	return 0;
}

unsigned long board_spl_mmc_get_uboot_raw_sector(struct mmc *mmc, unsigned long raw_sect)
{
	/*
	 * The image offset on SD/MMC devices is 32 KiB, except for eMMC boot from
	 * HW boot part. In this case it is 0 KiB. In order to make the bootloader
	 * universal, check for HW boot part and adjust the offset.
	 */
	if (!IS_SD(mmc)) {
		switch (EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config)) {
		case EMMC_BOOT_PART_BOOT1:
		case EMMC_BOOT_PART_BOOT2:
			raw_sect -= 32 * 2;
			break;
		}
	}

	return raw_sect;
}

const char *spl_board_loader_name(u32 boot_device)
{
	static char name[16];
	struct mmc *mmc;

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		mmc_init_device(0);
		mmc = find_mmc_device(0);
		mmc_init(mmc);
		snprintf(name, sizeof(name), "eMMC %s",
			 emmc_hwpart_names[EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config)]);
		return name;
	case BOOT_DEVICE_MMC2:
		sprintf(name, "SD card");
		return name;
	}

	return NULL;
}

void board_init_f(ulong dummy)
{
	int ret;

	arch_cpu_init();

	init_uart_clk(2);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	/* PMIC initialization */
	power_init_board();

	/* DDR initialization */
	spl_dram_init();
}
