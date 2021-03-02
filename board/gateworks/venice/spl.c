// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Gateworks Corporation
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <i2c.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/ddr.h>
#include <asm-generic/gpio.h>

#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

#include <power/mp5416.h>

#include "gsc.h"
#include "lpddr4_timing.h"

#define PCIE_RSTN IMX_GPIO_NR(4, 6)

DECLARE_GLOBAL_DATA_PTR;

static void spl_dram_init(int size)
{
	struct dram_timing_info *dram_timing;

	switch (size) {
	case 1:
		dram_timing = &dram_timing_1gb;
		break;
	case 4:
		dram_timing = &dram_timing_4gb;
		break;
	default:
		printf("Unknown DDR configuration: %d GiB\n", size);
		dram_timing = &dram_timing_1gb;
		size = 1;
	}

	printf("DRAM    : LPDDR4 %d GiB\n", size);
	ddr_init(dram_timing);
	writel(size, M4_BOOTROM_BASE_ADDR);
}

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

static iomux_v3_cfg_t const uart_pads[] = {
	IMX8MM_PAD_UART2_RXD_UART2_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MM_PAD_UART2_TXD_UART2_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MM_PAD_GPIO1_IO02_WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	return 0;
}

/*
 * Model specific PMIC adjustments necessary prior to DRAM init
 *
 * Note that we can not use pmic dm drivers here as we have a generic
 * venice dt that does not have board-specific pmic's defined.
 *
 * Instead we must use dm_i2c.
 */
static int power_init_board(void)
{
	const char *model = gsc_get_model();
	struct udevice *bus;
	struct udevice *dev;
	int ret;

	if ((!strncmp(model, "GW71", 4)) ||
	    (!strncmp(model, "GW72", 4)) ||
	    (!strncmp(model, "GW73", 4))) {
		ret = uclass_get_device_by_name(UCLASS_I2C, "i2c@30a20000", &bus);
		if (ret) {
			printf("PMIC    : failed I2C1 probe: %d\n", ret);
			return ret;
		}
		ret = dm_i2c_probe(bus, 0x69, 0, &dev);
		if (ret) {
			printf("PMIC    : failed probe: %d\n", ret);
			return ret;
		}
		puts("PMIC    : MP5416\n");

		/* set VDD_ARM SW3 to 0.92V for 1.6GHz */
		dm_i2c_reg_write(dev, MP5416_VSET_SW3,
				 BIT(7) | MP5416_VSET_SW3_SVAL(920000));
	}

	return 0;
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;
	int dram_sz;

	arch_cpu_init();

	init_uart_clk(1);

	board_early_init_f();

	timer_init();

	preloader_console_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0) {
		printf("Failed to find clock node. Check device tree\n");
		hang();
	}

	enable_tzc380();

	/* need to hold PCIe switch in reset otherwise it can lock i2c bus EEPROM is on */
	gpio_request(PCIE_RSTN, "perst#");
	gpio_direction_output(PCIE_RSTN, 0);

	/* GSC */
	dram_sz = gsc_init(0);

	/* PMIC */
	power_init_board();

	/* DDR initialization */
	spl_dram_init(dram_sz);

	board_init_r(NULL, 0);
}

/* determine prioritized order of boot devices to load U-Boot from */
void board_boot_order(u32 *spl_boot_list)
{
	/*
	 * If the SPL was loaded via serial loader, we try to get
	 * U-Boot proper via USB SDP.
	 */
	if (spl_boot_device() == BOOT_DEVICE_BOARD)
		spl_boot_list[0] = BOOT_DEVICE_BOARD;

	/* we have only eMMC in default venice dt */
	spl_boot_list[0] = BOOT_DEVICE_MMC1;
}

/* return boot device based on where the SPL was loaded from */
int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	switch (boot_dev_spl) {
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	/* SDHC2 */
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC1;
	/* SDHC3 */
	case SD3_BOOT:
	case MMC3_BOOT:
		return BOOT_DEVICE_MMC2;
	default:
		return BOOT_DEVICE_NONE;
	}
}
