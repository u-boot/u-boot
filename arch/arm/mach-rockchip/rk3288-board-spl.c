/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <fdtdec.h>
#include <led.h>
#include <malloc.h>
#include <ram.h>
#include <spl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/periph.h>
#include <asm/arch/sdram.h>
#include <asm/arch/timer.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/util.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
	const void *blob = gd->fdt_blob;
	struct udevice *dev;
	const char *bootdev;
	int node;
	int ret;

	bootdev = fdtdec_get_config_string(blob, "u-boot,boot0");
	debug("Boot device %s\n", bootdev);
	if (!bootdev)
		goto fallback;

	node = fdt_path_offset(blob, bootdev);
	if (node < 0) {
		debug("node=%d\n", node);
		goto fallback;
	}
	ret = device_get_global_by_of_offset(node, &dev);
	if (ret) {
		debug("device at node %s/%d not found: %d\n", bootdev, node,
		      ret);
		goto fallback;
	}
	debug("Found device %s\n", dev->name);
	switch (device_get_uclass_id(dev)) {
	case UCLASS_SPI_FLASH:
		return BOOT_DEVICE_SPI;
	case UCLASS_MMC:
		return BOOT_DEVICE_MMC1;
	default:
		debug("Booting from device uclass '%s' not supported\n",
		      dev_get_uclass_name(dev));
	}

fallback:
	return BOOT_DEVICE_MMC1;
}

u32 spl_boot_mode(void)
{
	return MMCSD_MODE_RAW;
}

/* read L2 control register (L2CTLR) */
static inline uint32_t read_l2ctlr(void)
{
	uint32_t val = 0;

	asm volatile ("mrc p15, 1, %0, c9, c0, 2" : "=r" (val));

	return val;
}

/* write L2 control register (L2CTLR) */
static inline void write_l2ctlr(uint32_t val)
{
	/*
	 * Note: L2CTLR can only be written when the L2 memory system
	 * is idle, ie before the MMU is enabled.
	 */
	asm volatile("mcr p15, 1, %0, c9, c0, 2" : : "r" (val) : "memory");
	isb();
}

static void configure_l2ctlr(void)
{
	uint32_t l2ctlr;

	l2ctlr = read_l2ctlr();
	l2ctlr &= 0xfffc0000; /* clear bit0~bit17 */

	/*
	* Data RAM write latency: 2 cycles
	* Data RAM read latency: 2 cycles
	* Data RAM setup latency: 1 cycle
	* Tag RAM write latency: 1 cycle
	* Tag RAM read latency: 1 cycle
	* Tag RAM setup latency: 1 cycle
	*/
	l2ctlr |= (1 << 3 | 1 << 0);
	write_l2ctlr(l2ctlr);
}

static int configure_emmc(struct udevice *pinctrl)
{
	struct gpio_desc desc;
	int ret;

	pinctrl_request_noflags(pinctrl, PERIPH_ID_EMMC);

	/*
	 * TODO(sjg@chromium.org): Pick this up from device tree or perhaps
	 * use the EMMC_PWREN setting.
	 */
	ret = dm_gpio_lookup_name("D9", &desc);
	if (ret) {
		debug("gpio ret=%d\n", ret);
		return ret;
	}
	ret = dm_gpio_request(&desc, "emmc_pwren");
	if (ret) {
		debug("gpio_request ret=%d\n", ret);
		return ret;
	}
	ret = dm_gpio_set_dir_flags(&desc, GPIOD_IS_OUT);
	if (ret) {
		debug("gpio dir ret=%d\n", ret);
		return ret;
	}
	ret = dm_gpio_set_value(&desc, 1);
	if (ret) {
		debug("gpio value ret=%d\n", ret);
		return ret;
	}

	return 0;
}

void board_init_f(ulong dummy)
{
	struct udevice *pinctrl;
	struct udevice *dev;
	int ret;

	/* Example code showing how to enable the debug UART on RK3288 */
#ifdef EARLY_UART
#include <asm/arch/grf_rk3288.h>
	/* Enable early UART on the RK3288 */
#define GRF_BASE	0xff770000
	struct rk3288_grf * const grf = (void *)GRF_BASE;

	rk_clrsetreg(&grf->gpio7ch_iomux, GPIO7C7_MASK << GPIO7C7_SHIFT |
		     GPIO7C6_MASK << GPIO7C6_SHIFT,
		     GPIO7C7_UART2DBG_SOUT << GPIO7C7_SHIFT |
		     GPIO7C6_UART2DBG_SIN << GPIO7C6_SHIFT);
	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
	debug_uart_init();
#endif

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	rockchip_timer_init();
	configure_l2ctlr();

	ret = uclass_get_device(UCLASS_CLK, 0, &dev);
	if (ret) {
		debug("CLK init failed: %d\n", ret);
		return;
	}

	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		debug("Pinctrl init failed: %d\n", ret);
		return;
	}

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return;
	}
}

static int setup_led(void)
{
#ifdef CONFIG_SPL_LED
	struct udevice *dev;
	char *led_name;
	int ret;

	led_name = fdtdec_get_config_string(gd->fdt_blob, "u-boot,boot-led");
	if (!led_name)
		return 0;
	ret = led_get_by_label(led_name, &dev);
	if (ret) {
		debug("%s: get=%d\n", __func__, ret);
		return ret;
	}
	ret = led_set_on(dev, 1);
	if (ret)
		return ret;
#endif

	return 0;
}

void spl_board_init(void)
{
	struct udevice *pinctrl;
	int ret;

	ret = setup_led();

	if (ret) {
		debug("LED ret=%d\n", ret);
		hang();
	}

	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		debug("%s: Cannot find pinctrl device\n", __func__);
		goto err;
	}
	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_SDCARD);
	if (ret) {
		debug("%s: Failed to set up SD card\n", __func__);
		goto err;
	}
	ret = configure_emmc(pinctrl);
	if (ret) {
		debug("%s: Failed to set up eMMC\n", __func__);
		goto err;
	}

	/* Enable debug UART */
	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_UART_DBG);
	if (ret) {
		debug("%s: Failed to set up console UART\n", __func__);
		goto err;
	}

	preloader_console_init();
	return;
err:
	printf("spl_board_init: Error %d\n", ret);

	/* No way to report error here */
	hang();
}
