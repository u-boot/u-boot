/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include<common.h>
#include<config.h>

#include <asm/arch/clock.h>
#include <asm/arch/clk.h>
#include <asm/arch/dmc.h>
#include <asm/arch/power.h>
#include <asm/arch/spl.h>

#include "common_setup.h"
#include "clock_init.h"

DECLARE_GLOBAL_DATA_PTR;
#define OM_STAT         (0x1f << 1)

/* Index into irom ptr table */
enum index {
	MMC_INDEX,
	EMMC44_INDEX,
	EMMC44_END_INDEX,
	SPI_INDEX,
	USB_INDEX,
};

/* IROM Function Pointers Table */
u32 irom_ptr_table[] = {
	[MMC_INDEX] = 0x02020030,	/* iROM Function Pointer-SDMMC boot */
	[EMMC44_INDEX] = 0x02020044,	/* iROM Function Pointer-EMMC4.4 boot*/
	[EMMC44_END_INDEX] = 0x02020048,/* iROM Function Pointer
						-EMMC4.4 end boot operation */
	[SPI_INDEX] = 0x02020058,	/* iROM Function Pointer-SPI boot */
	[USB_INDEX] = 0x02020070,	/* iROM Function Pointer-USB boot*/
	};

void *get_irom_func(int index)
{
	return (void *)*(u32 *)irom_ptr_table[index];
}

#ifdef CONFIG_USB_BOOTING
/*
 * Set/clear program flow prediction and return the previous state.
 */
static int config_branch_prediction(int set_cr_z)
{
	unsigned int cr;

	/* System Control Register: 11th bit Z Branch prediction enable */
	cr = get_cr();
	set_cr(set_cr_z ? cr | CR_Z : cr & ~CR_Z);

	return cr & CR_Z;
}
#endif

/*
* Copy U-boot from mmc to RAM:
* COPY_BL2_FNPTR_ADDR: Address in iRAM, which Contains
* Pointer to API (Data transfer from mmc to ram)
*/
void copy_uboot_to_ram(void)
{
	enum boot_mode bootmode = BOOT_MODE_OM;

	u32 (*copy_bl2)(u32 offset, u32 nblock, u32 dst) = NULL;
	u32 offset = 0, size = 0;
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	u32 (*copy_bl2_from_emmc)(u32 nblock, u32 dst);
	void (*end_bootop_from_emmc)(void);
#endif
#ifdef CONFIG_USB_BOOTING
	u32 (*usb_copy)(void);
	int is_cr_z_set;
	unsigned int sec_boot_check;

	/* Read iRAM location to check for secondary USB boot mode */
	sec_boot_check = readl(EXYNOS_IRAM_SECONDARY_BASE);
	if (sec_boot_check == EXYNOS_USB_SECONDARY_BOOT)
		bootmode = BOOT_MODE_USB;
#endif

	if (bootmode == BOOT_MODE_OM)
		bootmode = readl(samsung_get_base_power()) & OM_STAT;

	switch (bootmode) {
#ifdef CONFIG_SPI_BOOTING
	case BOOT_MODE_SERIAL:
		offset = SPI_FLASH_UBOOT_POS;
		size = CONFIG_BL2_SIZE;
		copy_bl2 = get_irom_func(SPI_INDEX);
		break;
#endif
	case BOOT_MODE_MMC:
		offset = BL2_START_OFFSET;
		size = BL2_SIZE_BLOC_COUNT;
		copy_bl2 = get_irom_func(MMC_INDEX);
		break;
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	case BOOT_MODE_EMMC:
		/* Set the FSYS1 clock divisor value for EMMC boot */
		emmc_boot_clk_div_set();

		copy_bl2_from_emmc = get_irom_func(EMMC44_INDEX);
		end_bootop_from_emmc = get_irom_func(EMMC44_END_INDEX);

		copy_bl2_from_emmc(BL2_SIZE_BLOC_COUNT, CONFIG_SYS_TEXT_BASE);
		end_bootop_from_emmc();
		break;
#endif
#ifdef CONFIG_USB_BOOTING
	case BOOT_MODE_USB:
		/*
		 * iROM needs program flow prediction to be disabled
		 * before copy from USB device to RAM
		 */
		is_cr_z_set = config_branch_prediction(0);
		usb_copy = get_irom_func(USB_INDEX);
		usb_copy();
		config_branch_prediction(is_cr_z_set);
		break;
#endif
	default:
		break;
	}

	if (copy_bl2)
		copy_bl2(offset, size, CONFIG_SYS_TEXT_BASE);
}

void memzero(void *s, size_t n)
{
	char *ptr = s;
	size_t i;

	for (i = 0; i < n; i++)
		*ptr++ = '\0';
}

/**
 * Set up the U-Boot global_data pointer
 *
 * This sets the address of the global data, and sets up basic values.
 *
 * @param gdp   Value to give to gd
 */
static void setup_global_data(gd_t *gdp)
{
	gd = gdp;
	memzero((void *)gd, sizeof(gd_t));
	gd->flags |= GD_FLG_RELOC;
	gd->baudrate = CONFIG_BAUDRATE;
	gd->have_console = 1;
}

void board_init_f(unsigned long bootflag)
{
	__aligned(8) gd_t local_gd;
	__attribute__((noreturn)) void (*uboot)(void);

	setup_global_data(&local_gd);

	if (do_lowlevel_init())
		power_exit_wakeup();

	copy_uboot_to_ram();

	/* Jump to U-Boot image */
	uboot = (void *)CONFIG_SYS_TEXT_BASE;
	(*uboot)();
	/* Never returns Here */
}

/* Place Holders */
void board_init_r(gd_t *id, ulong dest_addr)
{
	/* Function attribute is no-return */
	/* This Function never executes */
	while (1)
		;
}
void save_boot_params(u32 r0, u32 r1, u32 r2, u32 r3) {}
