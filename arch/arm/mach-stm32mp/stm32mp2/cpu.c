// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <clk.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <env_internal.h>
#include <init.h>
#include <misc.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>
#include <asm/system.h>
#include <dm/device.h>
#include <dm/lists.h>
#include <dm/uclass.h>

/*
 * early TLB into the .data section so that it not get cleared
 * with 16kB alignment
 */
#define EARLY_TLB_SIZE 0x10000
u8 early_tlb[EARLY_TLB_SIZE] __section(".data") __aligned(0x4000);

/*
 * initialize the MMU and activate cache in U-Boot pre-reloc stage
 * MMU/TLB is updated in enable_caches() for U-Boot after relocation
 */
static void early_enable_caches(void)
{
	if (CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		return;

	if (!(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))) {
		gd->arch.tlb_size = EARLY_TLB_SIZE;
		gd->arch.tlb_addr = (unsigned long)&early_tlb;
	}
	/* enable MMU (default configuration) */
	dcache_enable();
}

/*
 * Early system init
 */
int arch_cpu_init(void)
{
	icache_enable();
	early_enable_caches();

	return 0;
}

int mach_cpu_init(void)
{
	u32 boot_mode;

	boot_mode = get_bootmode();

	if (IS_ENABLED(CONFIG_CMD_STM32PROG_SERIAL) &&
	    (boot_mode & TAMP_BOOT_DEVICE_MASK) == BOOT_SERIAL_UART)
		gd->flags |= GD_FLG_SILENT | GD_FLG_DISABLE_CONSOLE;

	return 0;
}

void enable_caches(void)
{
	/* deactivate the data cache, early enabled in arch_cpu_init() */
	dcache_disable();
	/*
	 * Force the call of setup_all_pgtables() in mmu_setup() by clearing tlb_fillptr
	 * to update the TLB location udpated in board_f.c::reserve_mmu
	 */
	gd->arch.tlb_fillptr = 0;
	dcache_enable();
}

/*
 * Force data-section, as .bss will not be valid
 * when save_boot_params is invoked.
 */
static uintptr_t nt_fw_dtb __section(".data");

uintptr_t get_stm32mp_bl2_dtb(void)
{
	return nt_fw_dtb;
}

/*
 * Save the FDT address provided by TF-A in r2 at boot time
 * This function is called from start.S
 */
void save_boot_params(unsigned long r0, unsigned long r1, unsigned long r2,
		      unsigned long r3)
{
	nt_fw_dtb = r2;

	save_boot_params_ret();
}

u32 get_bootmode(void)
{
	/* read bootmode from TAMP backup register */
	return (readl(TAMP_BOOT_CONTEXT) & TAMP_BOOT_MODE_MASK) >>
		    TAMP_BOOT_MODE_SHIFT;
}

static void setup_boot_mode(void)
{
	const u32 serial_addr[] = {
		STM32_USART1_BASE,
		STM32_USART2_BASE,
		STM32_USART3_BASE,
		STM32_UART4_BASE,
		STM32_UART5_BASE,
		STM32_USART6_BASE,
		STM32_UART7_BASE,
		STM32_UART8_BASE,
		STM32_UART9_BASE
	};
	const u32 sdmmc_addr[] = {
		STM32_SDMMC1_BASE,
		STM32_SDMMC2_BASE,
		STM32_SDMMC3_BASE
	};
	char cmd[60];
	u32 boot_ctx = readl(TAMP_BOOT_CONTEXT);
	u32 boot_mode =
		(boot_ctx & TAMP_BOOT_MODE_MASK) >> TAMP_BOOT_MODE_SHIFT;
	unsigned int instance = (boot_mode & TAMP_BOOT_INSTANCE_MASK) - 1;
	u32 forced_mode = (boot_ctx & TAMP_BOOT_FORCED_MASK);
	struct udevice *dev;

	log_debug("%s: boot_ctx=0x%x => boot_mode=%x, instance=%d forced=%x\n",
		  __func__, boot_ctx, boot_mode, instance, forced_mode);
	switch (boot_mode & TAMP_BOOT_DEVICE_MASK) {
	case BOOT_SERIAL_UART:
		if (instance > ARRAY_SIZE(serial_addr))
			break;
		/* serial : search associated node in devicetree */
		sprintf(cmd, "serial@%x", serial_addr[instance]);
		if (uclass_get_device_by_name(UCLASS_SERIAL, cmd, &dev)) {
			/* restore console on error */
			if (IS_ENABLED(CONFIG_CMD_STM32PROG_SERIAL))
				gd->flags &= ~(GD_FLG_SILENT |
					       GD_FLG_DISABLE_CONSOLE);
			log_err("uart%d = %s not found in device tree!\n",
				instance + 1, cmd);
			break;
		}
		sprintf(cmd, "%d", dev_seq(dev));
		env_set("boot_device", "serial");
		env_set("boot_instance", cmd);

		/* restore console on uart when not used */
		if (IS_ENABLED(CONFIG_CMD_STM32PROG_SERIAL) && gd->cur_serial_dev != dev) {
			gd->flags &= ~(GD_FLG_SILENT |
				       GD_FLG_DISABLE_CONSOLE);
			log_info("serial boot with console enabled!\n");
		}
		break;
	case BOOT_SERIAL_USB:
		env_set("boot_device", "usb");
		env_set("boot_instance", "0");
		break;
	case BOOT_FLASH_SD:
	case BOOT_FLASH_EMMC:
		if (instance > ARRAY_SIZE(sdmmc_addr))
			break;
		/* search associated sdmmc node in devicetree */
		sprintf(cmd, "mmc@%x", sdmmc_addr[instance]);
		if (uclass_get_device_by_name(UCLASS_MMC, cmd, &dev)) {
			printf("mmc%d = %s not found in device tree!\n",
			       instance, cmd);
			break;
		}
		sprintf(cmd, "%d", dev_seq(dev));
		env_set("boot_device", "mmc");
		env_set("boot_instance", cmd);
		break;
	case BOOT_FLASH_NAND:
		env_set("boot_device", "nand");
		env_set("boot_instance", "0");
		break;
	case BOOT_FLASH_SPINAND:
		env_set("boot_device", "spi-nand");
		env_set("boot_instance", "0");
		break;
	case BOOT_FLASH_NOR:
		env_set("boot_device", "nor");
		if (IS_ENABLED(CONFIG_SYS_MAX_FLASH_BANKS))
			sprintf(cmd, "%d", CONFIG_SYS_MAX_FLASH_BANKS);
		else
			sprintf(cmd, "%d", 0);
		env_set("boot_instance", cmd);
		break;
	case BOOT_FLASH_HYPERFLASH:
		env_set("boot_device", "nor");
		env_set("boot_instance", "0");
		break;
	default:
		env_set("boot_device", "invalid");
		env_set("boot_instance", "");
		log_err("unexpected boot mode = %x\n", boot_mode);
		break;
	}

	switch (forced_mode) {
	case BOOT_FASTBOOT:
		log_info("Enter fastboot!\n");
		env_set("preboot", "env set preboot; fastboot 0");
		break;
	case BOOT_STM32PROG:
		env_set("boot_device", "usb");
		env_set("boot_instance", "0");
		break;
	case BOOT_UMS_MMC0:
	case BOOT_UMS_MMC1:
	case BOOT_UMS_MMC2:
		log_info("Enter UMS!\n");
		instance = forced_mode - BOOT_UMS_MMC0;
		sprintf(cmd, "env set preboot; ums 0 mmc %d", instance);
		env_set("preboot", cmd);
		break;
	case BOOT_RECOVERY:
		env_set("preboot", "env set preboot; run altbootcmd");
		break;
	case BOOT_NORMAL:
		break;
	default:
		log_debug("unexpected forced boot mode = %x\n", forced_mode);
		break;
	}

	/* clear TAMP for next reboot */
	clrsetbits_le32(TAMP_BOOT_CONTEXT, TAMP_BOOT_FORCED_MASK, BOOT_NORMAL);
}

int arch_misc_init(void)
{
	setup_boot_mode();
	setup_serial_number();
	setup_mac_address();

	return 0;
}
