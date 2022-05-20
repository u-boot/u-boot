// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <common.h>
#include <clk.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <env.h>
#include <init.h>
#include <log.h>
#include <lmb.h>
#include <misc.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <linux/bitops.h>

/*
 * early TLB into the .data section so that it not get cleared
 * with 16kB allignment (see TTBR0_BASE_ADDR_MASK)
 */
u8 early_tlb[PGTABLE_SIZE] __section(".data") __aligned(0x4000);

struct lmb lmb;

u32 get_bootmode(void)
{
	/* read bootmode from TAMP backup register */
	return (readl(TAMP_BOOT_CONTEXT) & TAMP_BOOT_MODE_MASK) >>
		    TAMP_BOOT_MODE_SHIFT;
}

/*
 * weak function overidde: set the DDR/SYSRAM executable before to enable the
 * MMU and configure DACR, for early early_enable_caches (SPL or pre-reloc)
 */
void dram_bank_mmu_setup(int bank)
{
	struct bd_info *bd = gd->bd;
	int	i;
	phys_addr_t start;
	phys_size_t size;
	bool use_lmb = false;
	enum dcache_option option;

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
/* STM32_SYSRAM_BASE exist only when SPL is supported */
#ifdef CONFIG_SPL
		start = ALIGN_DOWN(STM32_SYSRAM_BASE, MMU_SECTION_SIZE);
		size = ALIGN(STM32_SYSRAM_SIZE, MMU_SECTION_SIZE);
#endif
	} else if (gd->flags & GD_FLG_RELOC) {
		/* bd->bi_dram is available only after relocation */
		start = bd->bi_dram[bank].start;
		size =  bd->bi_dram[bank].size;
		use_lmb = true;
	} else {
		/* mark cacheable and executable the beggining of the DDR */
		start = STM32_DDR_BASE;
		size = CONFIG_DDR_CACHEABLE_SIZE;
	}

	for (i = start >> MMU_SECTION_SHIFT;
	     i < (start >> MMU_SECTION_SHIFT) + (size >> MMU_SECTION_SHIFT);
	     i++) {
		option = DCACHE_DEFAULT_OPTION;
		if (use_lmb && lmb_is_reserved_flags(&lmb, i << MMU_SECTION_SHIFT, LMB_NOMAP))
			option = 0; /* INVALID ENTRY in TLB */
		set_section_dcache(i, option);
	}
}
/*
 * initialize the MMU and activate cache in SPL or in U-Boot pre-reloc stage
 * MMU/TLB is updated in enable_caches() for U-Boot after relocation
 * or is deactivated in U-Boot entry function start.S::cpu_init_cp15
 */
static void early_enable_caches(void)
{
	/* I-cache is already enabled in start.S: cpu_init_cp15 */

	if (CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		return;

	if (!(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))) {
		gd->arch.tlb_size = PGTABLE_SIZE;
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
	early_enable_caches();

	/* early armv7 timer init: needed for polling */
	timer_init();

	return 0;
}

/* weak function for SOC specific initialization */
__weak void stm32mp_cpu_init(void)
{
}

int mach_cpu_init(void)
{
	u32 boot_mode;

	stm32mp_cpu_init();

	boot_mode = get_bootmode();

	if (IS_ENABLED(CONFIG_CMD_STM32PROG_SERIAL) &&
	    (boot_mode & TAMP_BOOT_DEVICE_MASK) == BOOT_SERIAL_UART)
		gd->flags |= GD_FLG_SILENT | GD_FLG_DISABLE_CONSOLE;
	else if (IS_ENABLED(CONFIG_DEBUG_UART) && IS_ENABLED(CONFIG_SPL_BUILD))
		debug_uart_init();

	return 0;
}

void enable_caches(void)
{
	/* parse device tree when data cache is still activated */
	lmb_init_and_reserve(&lmb, gd->bd, (void *)gd->fdt_blob);

	/* I-cache is already enabled in start.S: icache_enable() not needed */

	/* deactivate the data cache, early enabled in arch_cpu_init() */
	dcache_disable();
	/*
	 * update MMU after relocation and enable the data cache
	 * warning: the TLB location udpated in board_f.c::reserve_mmu
	 */
	dcache_enable();
}

/* used when CONFIG_DISPLAY_CPUINFO is activated */
int print_cpuinfo(void)
{
	char name[SOC_NAME_SIZE];

	get_soc_name(name);
	printf("CPU: %s\n", name);

	return 0;
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
		STM32_UART8_BASE
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

/*
 * If there is no MAC address in the environment, then it will be initialized
 * (silently) from the value in the OTP.
 */
__weak int setup_mac_address(void)
{
	int ret;
	int i;
	u32 otp[3];
	uchar enetaddr[6];
	struct udevice *dev;
	int nb_eth, nb_otp, index;

	if (!IS_ENABLED(CONFIG_NET))
		return 0;

	nb_eth = get_eth_nb();

	/* 6 bytes for each MAC addr and 4 bytes for each OTP */
	nb_otp = DIV_ROUND_UP(6 * nb_eth, 4);

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_MAC), otp, 4 * nb_otp);
	if (ret < 0)
		return ret;

	for (index = 0; index < nb_eth; index++) {
		/* MAC already in environment */
		if (eth_env_get_enetaddr_by_index("eth", index, enetaddr))
			continue;

		for (i = 0; i < 6; i++)
			enetaddr[i] = ((uint8_t *)&otp)[i + 6 * index];

		if (!is_valid_ethaddr(enetaddr)) {
			log_err("invalid MAC address %d in OTP %pM\n",
				index, enetaddr);
			return -EINVAL;
		}
		log_debug("OTP MAC address %d = %pM\n", index, enetaddr);
		ret = eth_env_set_enetaddr_by_index("eth", index, enetaddr);
		if (ret) {
			log_err("Failed to set mac address %pM from OTP: %d\n",
				enetaddr, ret);
			return ret;
		}
	}

	return 0;
}

static int setup_serial_number(void)
{
	char serial_string[25];
	u32 otp[3] = {0, 0, 0 };
	struct udevice *dev;
	int ret;

	if (env_get("serial#"))
		return 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_SERIAL),
			otp, sizeof(otp));
	if (ret < 0)
		return ret;

	sprintf(serial_string, "%08X%08X%08X", otp[0], otp[1], otp[2]);
	env_set("serial#", serial_string);

	return 0;
}

__weak void stm32mp_misc_init(void)
{
}

int arch_misc_init(void)
{
	setup_boot_mode();
	setup_mac_address();
	setup_serial_number();
	stm32mp_misc_init();

	return 0;
}
