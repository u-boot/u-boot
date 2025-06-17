// SPDX-License-Identifier: GPL-2.0+
/*
 * BTT[3C] iMX28 board
 *
 * Copyright (C) 2025 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <fdt_support.h>
#include <init.h>
#include <log.h>
#include <net.h>
#include <env.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx28.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <linux/delay.h>
#include <netdev.h>
#include <errno.h>
#include <serial.h>
#include <u-boot/crc.h>
#include "boot_img_scr.h"

#include <spi.h>
#include <spi_flash.h>

#ifdef CONFIG_XPL_BUILD
#include <spl.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * Functions
 */

static void init_clocks(void)
{
	/* IO0 clock at 480MHz */
	mxs_set_ioclk(MXC_IOCLK0, 480000);
	/* IO1 clock at 480MHz */
	mxs_set_ioclk(MXC_IOCLK1, 480000);

	/* SSP0 clock at 96MHz */
	mxs_set_sspclk(MXC_SSPCLK0, 96000, 0);
	/* SSP2 clock at 160MHz */
	mxs_set_sspclk(MXC_SSPCLK2, 160000, 0);
	/* SSP3 clock at 96MHz */
	mxs_set_sspclk(MXC_SSPCLK3, 96000, 0);
}

#if defined(CONFIG_XPL_BUILD) && defined(CONFIG_SPL_FRAMEWORK)
void board_init_f(ulong arg)
{
	init_clocks();
	spl_early_init();
	preloader_console_init();
}

static struct boot_img_src img_src[2];
static int spi_load_boot_info(void)
{
	struct spi_flash *flash;
	int err;

	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	if (!flash) {
		printf("%s: SPI probe err\n", __func__);
		return -ENODEV;
	}

	/*
	 * Load both boot info structs from SPI flash
	 */
	err = spi_flash_read(flash, SPI_FLASH_BOOT_SRC_OFFS,
			     sizeof(img_src[0]),
			     (void *)&img_src[0]);
	if (err) {
		debug("%s: First boot info NOR sector read error %d\n",
		      __func__, err);
		return err;
	}

	err = spi_flash_read(flash,
			     SPI_FLASH_BOOT_SRC_OFFS + SPI_FLASH_SECTOR_SIZE,
			     sizeof(img_src[0]),
			     (void *)&img_src[1]);
	if (err) {
		debug("%s: First boot info NOR sector read error %d\n",
		      __func__, err);
		return err;
	}

	debug("%s: BI0 0x%x 0x%x 0x%x\n", __func__,
	      img_src[0].magic, img_src[0].flags, img_src[0].crc8);

	debug("%s: BI1 0x%x 0x%x 0x%x\n", __func__,
	      img_src[1].magic, img_src[1].flags, img_src[1].crc8);

	return 0;
}

#define BTT_MONITORING_DEVICE_TIMEOUT 100
static int rescue_val;

void spl_board_init(void)
{
	struct gpio_desc phy_rst, boot, rescue, wifi_en, bt_en;
	int ret, i;

	/*
	 * On the new HW version of BTTC/3 (with LAN8720ai PHY) the !RST pin
	 * (15) is pulled LOW by external resistor. As a result it needs to be
	 * set HIGH as soon as possible to allow correct generation of RESET
	 * pulse.
	 *
	 * In the old BTTC (with TLK105 PHY) the RC circuit was used instead
	 * to set the RESET pin to HIGH after 100us, so there was no need to
	 * set it explicitly.
	 */
	ret = dm_gpio_lookup_name("GPIO4_12", &phy_rst);
	if (ret)
		printf("Cannot get GPIO4_12\n");

	ret = dm_gpio_request(&phy_rst, "phy-rst");
	if (ret)
		printf("Cannot request GPIO4_12\n");

	dm_gpio_set_dir_flags(&phy_rst, GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);

	/*
	 * Explicitly set GPIO, which controls WL_EN (wifi) to LOW. On the BTT3
	 * it is directly connected to Jody module without any externa pull up
	 * down register.
	 */
	ret = dm_gpio_lookup_name("GPIO0_27", &wifi_en);
	if (ret)
		printf("Cannot get GPIO0_27\n");

	ret = dm_gpio_request(&wifi_en, "wifi-en");
	if (ret)
		printf("Cannot request GPIO0_27\n");

	dm_gpio_set_dir_flags(&wifi_en, GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			      GPIOD_IS_OUT_ACTIVE);

	/*
	 * Explicitly set GPIO, which controls BT_EN (Bluetooth) to LOW. On the
	 * BTT3 it is connected to Jody module via RC circuit (after some R*C
	 * time this pin is set to HIGH). However, the manual recommends setting
	 * it high from LOW state.
	 */
	ret = dm_gpio_lookup_name("GPIO3_27", &bt_en);
	if (ret)
		printf("Cannot get GPIO3_27\n");

	ret = dm_gpio_request(&bt_en, "bt-en");
	if (ret)
		printf("Cannot request GPIO3_27\n");

	dm_gpio_set_dir_flags(&bt_en, GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			      GPIOD_IS_OUT_ACTIVE);

	/* 'boot' and 'rescue' pins */
	ret = dm_gpio_lookup_name("GPIO4_9", &boot);
	if (ret)
		printf("Cannot get GPIO4_9\n");

	ret = dm_gpio_request(&boot, "boot");
	if (ret)
		printf("Cannot request GPIO4_9\n");

	dm_gpio_set_dir_flags(&boot, GPIOD_IS_IN);

	ret = dm_gpio_lookup_name("GPIO4_11", &rescue);
	if (ret)
		printf("Cannot get GPIO4_11\n");

	ret = dm_gpio_request(&rescue, "rescue");
	if (ret)
		printf("Cannot request GPIO4_11\n");

	dm_gpio_set_dir_flags(&rescue, GPIOD_IS_IN);

	/* Wait for ready signal from system "monitoring" device */
	for (i = 0; i < BTT_MONITORING_DEVICE_TIMEOUT; i++) {
		if (dm_gpio_get_value(&boot))
			break;
		mdelay(10);
	}

	rescue_val = dm_gpio_get_value(&rescue);
}

int spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	int i, src_idx = -1, ret;

	ret = spi_load_boot_info();
	if (ret) {
		printf("%s: Cannot read BTT boot info! [%d]\n", __func__, ret);
		/* To avoid bricking board - by default boot from boot0 eMMC */
		return 1;
	}

	for (i = 0; i < 2; i++) {
		if (img_src[i].magic == 'B' &&
		    img_src[i].crc8 == crc8(0, &img_src[i].magic, 2)) {
			src_idx = i;
			break;
		}
	}

	debug("%s: src idx: %d\n", __func__, src_idx);

	if (src_idx < 0)
		/*
		 * Always use eMMC (mmcblkX) boot0 if no
		 * valid image source description found
		 */
		return 1;

	if (img_src[src_idx].flags & BOOT_SRC_PART1)
		return 2;

	return 1;
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = BOOT_DEVICE_MMC1;
	spl_boot_list[1] = BOOT_DEVICE_SPI;
	spl_boot_list[2] = BOOT_DEVICE_UART;
}

int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

	debug("%s: rescue: %d\n", __func__, rescue_val);
	return rescue_val;
}
#else

/*
 * Providing proper board name - i.e. 'bttc' vs 'btt3'
 * The distinction is made on the size of DRAM memory - i.e.
 * bttc has only 128 MiB, whereas btt3 has 256 MiB.
 */
#define STR_BTTC "bttc"
#define STR_BTT3 "btt3"

static const char *get_board_name(void)
{
	if (gd->bd->bi_dram[0].size == SZ_128M)
		return STR_BTTC;

	return STR_BTT3;
}

/*
 * Reading the HW ID number for BTT3 device
 *
 * GPIOs from Port 4:
 * E0: GPIO4_10
 * E1: GPIO4_5
 * E2: GPIO4_14
 * E3: GPIO4_15
 * are used on BTT3 to store HW revision information.
 *
 * From rev 1+ the REV GPIOs are properly connected on the PCB, so PULL UPs
 * shall be disabled (as they are by default on pins' SPL configuration)
 *.
 * Rev 0: - read all '1' (first production version without HW rev set)
 * Rev 1: - read 0x1 (E0 set)
 * Rev 2: - read 0x2 (E1 set)
 *
 */
#define BTT3_HW_ID_GPIO_PORT (MXS_PINCTRL_BASE + (0x0900 + ((4) * 0x10)))
#define BTT3_HW_ID_E0 BIT(10)
#define BTT3_HW_ID_E1 BIT(5)
#define BTT3_HW_ID_E2 BIT(14)
#define BTT3_HW_ID_E3 BIT(15)

static u8 get_som_rev(void)
{
	struct mxs_register_32 *reg =
		(struct mxs_register_32 *)BTT3_HW_ID_GPIO_PORT;
	u32 tmp = ~readl(&reg->reg);
	u8 id = 0;

	if (tmp & BTT3_HW_ID_E0)
		id += 1;

	if (tmp & BTT3_HW_ID_E1)
		id += 2;

	if (tmp & BTT3_HW_ID_E2)
		id += 4;

	if (tmp & BTT3_HW_ID_E3)
		id += 8;

	/*
	 * Special case for first production BTT3 version, without HW
	 * revision support (so it reads 0x0s as pullups are disabled
	 * and hence 0xF is set for ID)
	 */
	if (id == 0xF)
		id = 0;

	return id;
}

int board_early_init_f(void)
{
	init_clocks();

	return 0;
}

int board_init(void)
{
	struct gpio_desc phy_rst;
	int ret;

	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	cpu_eth_init(NULL);

	/* PHY INT#/PWDN# */
	ret = dm_gpio_lookup_name("GPIO4_13", &phy_rst);
	if (ret) {
		printf("Cannot get GPIO4_13\n");
		return ret;
	}

	ret = dm_gpio_request(&phy_rst, "phy-rst");
	if (ret) {
		printf("Cannot request GPIO4_13\n");
		return ret;
	}

	dm_gpio_set_dir_flags(&phy_rst, GPIOD_IS_IN);
	udelay(1000);

	return 0;
}

#if defined(CONFIG_BOARD_LATE_INIT)
int board_late_init(void)
{
	int ret = env_set_ulong("board_som_rev", get_som_rev());

	if (ret)
		printf("Cannot set BTT's SoM revision env variable!\n");

	ret = env_set("arch", get_board_name());
	if (ret)
		printf("Cannot set SoM 'arch' env variable!\n");

	return 0;
}
#endif

#if defined(CONFIG_DISPLAY_BOARDINFO)
int checkboard(void)
{
	printf("Board: LWE BTT SoM HW rev %d\n", get_som_rev());

	return 0;
}
#endif

int dram_init(void)
{
	return mxs_dram_init();
}

#if defined(CONFIG_OF_BOARD)
int board_fdt_blob_setup(void **fdtp)
{
	/*
	 * The only purpose of this function is the specific BTT's DTB
	 * setup in u-boot proper. To be more specific - the SPL
	 * cannot support DTB selection due to size constraints
	 * (SPL < 50 KiB).
	 *
	 * Hence, the DTB selection is done in u-boot, which due to
	 * board's partition sizes (and backward compatibility) has also
	 * size constrain (~448 KiB).
	 *
	 * To support multiple DTBs appended, the compression has been used
	 * for them. Unfortunately, the initf_malloc() is called
	 * after the DTB needs to be selected. To fix this problem for this
	 * particular setup (i.e. BTT board) the initf_malloc() is called here.
	 */
	initf_malloc();

	return -EEXIST;
}
#endif

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	u8 rev_id = get_som_rev();
	char board[12];

	sprintf(board, "imx28-btt3-%d", rev_id);

	if (!strncmp(name, board, sizeof(board)))
		return 0;

	return -EINVAL;
}
#endif

/*
 * NOTE:
 *
 * IMX28 clock "stub" DM driver!
 *
 * Only used for SPL stage, which is NOT using DM; serial and
 * eMMC configuration.
 *
 * It is required for SPL_OF_PLATDATA proper code generation as,
 * this device has hard constrain on the size of the SPL binary
 * (u-boot.sb).
 */
static const struct udevice_id imx28_clk_ids[] = {
	{ .compatible = "fsl,imx28-clkctrl", },
	{ }
};

U_BOOT_DRIVER(fsl_imx28_clkctrl) = {
	.name           = "fsl_imx28_clkctrl",
	.id             = UCLASS_CLK,
	.of_match       = imx28_clk_ids,
};
#endif	/* CONFIG_XPL_BUILD */
