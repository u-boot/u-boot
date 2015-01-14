/*
 * Freescale i.MX28 Boot setup
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <linux/compiler.h>

#include "mxs_init.h"

DECLARE_GLOBAL_DATA_PTR;
static gd_t gdata __section(".data");
#ifdef CONFIG_SPL_SERIAL_SUPPORT
static bd_t bdata __section(".data");
#endif

/*
 * This delay function is intended to be used only in early stage of boot, where
 * clock are not set up yet. The timer used here is reset on every boot and
 * takes a few seconds to roll. The boot doesn't take that long, so to keep the
 * code simple, it doesn't take rolling into consideration.
 */
void early_delay(int delay)
{
	struct mxs_digctl_regs *digctl_regs =
		(struct mxs_digctl_regs *)MXS_DIGCTL_BASE;

	uint32_t st = readl(&digctl_regs->hw_digctl_microseconds);
	st += delay;
	while (st > readl(&digctl_regs->hw_digctl_microseconds))
		;
}

#define	MUX_CONFIG_BOOTMODE_PAD	(MXS_PAD_3V3 | MXS_PAD_4MA | MXS_PAD_NOPULL)
static const iomux_cfg_t iomux_boot[] = {
#if defined(CONFIG_MX23)
	MX23_PAD_LCD_D00__GPIO_1_0 | MUX_CONFIG_BOOTMODE_PAD,
	MX23_PAD_LCD_D01__GPIO_1_1 | MUX_CONFIG_BOOTMODE_PAD,
	MX23_PAD_LCD_D02__GPIO_1_2 | MUX_CONFIG_BOOTMODE_PAD,
	MX23_PAD_LCD_D03__GPIO_1_3 | MUX_CONFIG_BOOTMODE_PAD,
	MX23_PAD_LCD_D04__GPIO_1_4 | MUX_CONFIG_BOOTMODE_PAD,
	MX23_PAD_LCD_D05__GPIO_1_5 | MUX_CONFIG_BOOTMODE_PAD,
#elif defined(CONFIG_MX28)
	MX28_PAD_LCD_D00__GPIO_1_0 | MUX_CONFIG_BOOTMODE_PAD,
	MX28_PAD_LCD_D01__GPIO_1_1 | MUX_CONFIG_BOOTMODE_PAD,
	MX28_PAD_LCD_D02__GPIO_1_2 | MUX_CONFIG_BOOTMODE_PAD,
	MX28_PAD_LCD_D03__GPIO_1_3 | MUX_CONFIG_BOOTMODE_PAD,
	MX28_PAD_LCD_D04__GPIO_1_4 | MUX_CONFIG_BOOTMODE_PAD,
	MX28_PAD_LCD_D05__GPIO_1_5 | MUX_CONFIG_BOOTMODE_PAD,
#endif
};

static uint8_t mxs_get_bootmode_index(void)
{
	uint8_t bootmode = 0;
	int i;
	uint8_t masked;

	/* Setup IOMUX of bootmode pads to GPIO */
	mxs_iomux_setup_multiple_pads(iomux_boot, ARRAY_SIZE(iomux_boot));

#if defined(CONFIG_MX23)
	/* Setup bootmode pins as GPIO input */
	gpio_direction_input(MX23_PAD_LCD_D00__GPIO_1_0);
	gpio_direction_input(MX23_PAD_LCD_D01__GPIO_1_1);
	gpio_direction_input(MX23_PAD_LCD_D02__GPIO_1_2);
	gpio_direction_input(MX23_PAD_LCD_D03__GPIO_1_3);
	gpio_direction_input(MX23_PAD_LCD_D05__GPIO_1_5);

	/* Read bootmode pads */
	bootmode |= (gpio_get_value(MX23_PAD_LCD_D00__GPIO_1_0) ? 1 : 0) << 0;
	bootmode |= (gpio_get_value(MX23_PAD_LCD_D01__GPIO_1_1) ? 1 : 0) << 1;
	bootmode |= (gpio_get_value(MX23_PAD_LCD_D02__GPIO_1_2) ? 1 : 0) << 2;
	bootmode |= (gpio_get_value(MX23_PAD_LCD_D03__GPIO_1_3) ? 1 : 0) << 3;
	bootmode |= (gpio_get_value(MX23_PAD_LCD_D05__GPIO_1_5) ? 1 : 0) << 5;
#elif defined(CONFIG_MX28)
	/* Setup bootmode pins as GPIO input */
	gpio_direction_input(MX28_PAD_LCD_D00__GPIO_1_0);
	gpio_direction_input(MX28_PAD_LCD_D01__GPIO_1_1);
	gpio_direction_input(MX28_PAD_LCD_D02__GPIO_1_2);
	gpio_direction_input(MX28_PAD_LCD_D03__GPIO_1_3);
	gpio_direction_input(MX28_PAD_LCD_D04__GPIO_1_4);
	gpio_direction_input(MX28_PAD_LCD_D05__GPIO_1_5);

	/* Read bootmode pads */
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D00__GPIO_1_0) ? 1 : 0) << 0;
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D01__GPIO_1_1) ? 1 : 0) << 1;
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D02__GPIO_1_2) ? 1 : 0) << 2;
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D03__GPIO_1_3) ? 1 : 0) << 3;
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D04__GPIO_1_4) ? 1 : 0) << 4;
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D05__GPIO_1_5) ? 1 : 0) << 5;
#endif

	for (i = 0; i < ARRAY_SIZE(mxs_boot_modes); i++) {
		masked = bootmode & mxs_boot_modes[i].boot_mask;
		if (masked == mxs_boot_modes[i].boot_pads)
			break;
	}

	return i;
}

static void mxs_spl_fixup_vectors(void)
{
	/*
	 * Copy our vector table to 0x0, since due to HAB, we cannot
	 * be loaded to 0x0. We want to have working vectoring though,
	 * thus this fixup. Our vectoring table is PIC, so copying is
	 * fine.
	 */
	extern uint32_t _start;

	/* cppcheck-suppress nullPointer */
	memcpy(0x0, &_start, 0x60);
}

static void mxs_spl_console_init(void)
{
#ifdef CONFIG_SPL_SERIAL_SUPPORT
	gd->bd = &bdata;
	gd->baudrate = CONFIG_BAUDRATE;
	serial_init();
	gd->have_console = 1;
#endif
}

void mxs_common_spl_init(const uint32_t arg, const uint32_t *resptr,
			 const iomux_cfg_t *iomux_setup,
			 const unsigned int iomux_size)
{
	struct mxs_spl_data *data = (struct mxs_spl_data *)
		((CONFIG_SYS_TEXT_BASE - sizeof(struct mxs_spl_data)) & ~0xf);
	uint8_t bootmode = mxs_get_bootmode_index();
	gd = &gdata;

	mxs_spl_fixup_vectors();

	mxs_iomux_setup_multiple_pads(iomux_setup, iomux_size);

	mxs_spl_console_init();

	mxs_power_init();

	mxs_mem_init();
	data->mem_dram_size = mxs_mem_get_size();

	data->boot_mode_idx = bootmode;

	mxs_power_wait_pswitch();
}

/* Support aparatus */
inline void board_init_f(unsigned long bootflag)
{
	for (;;)
		;
}

inline void board_init_r(gd_t *id, ulong dest_addr)
{
	for (;;)
		;
}
