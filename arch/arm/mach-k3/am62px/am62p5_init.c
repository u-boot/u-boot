// SPDX-License-Identifier: GPL-2.0
/*
 * AM62P5: SoC specific initialization
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <spl.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <dm/pinctrl.h>
#include <dm/ofnode.h>

#include "../sysfw-loader.h"
#include "../common.h"

/* TISCI DEV ID for A53 Clock */
#define AM62PX_DEV_A53SS0_CORE_0_DEV_ID 135

#define CTRLMMR_MCU_RST_CTRL             0x04518170
#define RST_CTRL_ESM_ERROR_RST_EN_Z_MASK 0xFFFDFFFF

struct fwl_data cbass_main_fwls[] = {
	{ "FSS_DAT_REG3", 7, 8 },
};

/*
 * This uninitialized global variable would normal end up in the .bss section,
 * but the .bss is cleared between writing and reading this variable, so move
 * it to the .data section.
 */
u32 bootindex __section(".data");
static struct rom_extended_boot_data bootdata __section(".data");

static void store_boot_info_from_rom(void)
{
	bootindex = *(u32 *)(CONFIG_SYS_K3_BOOT_PARAM_TABLE_INDEX);
	memcpy(&bootdata, (uintptr_t *)ROM_EXTENDED_BOOT_DATA_INFO,
	       sizeof(struct rom_extended_boot_data));
}

static void ctrl_mmr_unlock(void)
{
	/* Unlock all WKUP_CTRL_MMR0 module registers */
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 0);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 1);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 2);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 3);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 4);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 5);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 6);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 7);

	/* Unlock all CTRL_MMR0 module registers */
	mmr_unlock(CTRL_MMR0_BASE, 0);
	mmr_unlock(CTRL_MMR0_BASE, 1);
	mmr_unlock(CTRL_MMR0_BASE, 2);
	mmr_unlock(CTRL_MMR0_BASE, 4);
	mmr_unlock(CTRL_MMR0_BASE, 5);
	mmr_unlock(CTRL_MMR0_BASE, 6);

	/* Unlock all MCU_CTRL_MMR0 module registers */
	mmr_unlock(MCU_CTRL_MMR0_BASE, 0);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 1);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 2);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 3);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 4);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 6);

	/* Unlock PADCFG_CTRL_MMR padconf registers */
	mmr_unlock(PADCFG_MMR0_BASE, 1);
	mmr_unlock(PADCFG_MMR1_BASE, 1);
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int get_a53_cpu_clock_index(ofnode node)
{
	int count, i;
	struct ofnode_phandle_args *args;
	ofnode clknode;

	clknode = ofnode_path("/bus@f0000/system-controller@44043000/clock-controller");
	if (!ofnode_valid(clknode))
		return -1;

	count = ofnode_count_phandle_with_args(node,  "assigned-clocks", "#clock-cells", 0);

	for (i  = 0; i < count; i++) {
		if (!ofnode_parse_phandle_with_args(node, "assigned-clocks",
						    "#clock-cells", 0, i, args)) {
			if (ofnode_equal(clknode, args->node) &&
			    args->args[0] == AM62PX_DEV_A53SS0_CORE_0_DEV_ID)
				return i;
		}
	}

	return -1;
}

static void fixup_a53_cpu_freq_by_speed_grade(void)
{
	int index, size;
	u32 *rates;
	ofnode node;

	node =  ofnode_path("/a53@0");
	if (!ofnode_valid(node))
		return;

	rates = fdt_getprop_w(ofnode_to_fdt(node), ofnode_to_offset(node),
			      "assigned-clock-rates", &size);

	index = get_a53_cpu_clock_index(node);

	if (!rates || index < 0 || index >= (size / sizeof(u32))) {
		printf("Wrong A53 assigned-clocks configuration\n");
		return;
	}

	rates[index] = cpu_to_fdt32(k3_get_a53_max_frequency());

	printf("Changed A53 CPU frequency to %dHz (%c grade) in DT\n",
	       k3_get_a53_max_frequency(), k3_get_speed_grade());
}
#else
static void fixup_a53_cpu_freq_by_speed_grade(void)
{
}
#endif

static __maybe_unused void enable_mcu_esm_reset(void)
{
	/* Set CTRLMMR_MCU_RST_CTRL:MCU_ESM_ERROR_RST_EN_Z  to '0' (low active) */
	u32 stat = readl(CTRLMMR_MCU_RST_CTRL);

	stat &= RST_CTRL_ESM_ERROR_RST_EN_Z_MASK;
	writel(stat, CTRLMMR_MCU_RST_CTRL);
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	if (IS_ENABLED(CONFIG_CPU_V7R))
		setup_k3_mpu_regions();

	/*
	 * Cannot delay this further as there is a chance that
	 * K3_BOOT_PARAM_TABLE_INDEX can be over written by SPL MALLOC section.
	 */
	store_boot_info_from_rom();

	ctrl_mmr_unlock();

	/* Init DM early */
	ret = spl_early_init();
	if (ret)
		panic("spl_early_init() failed: %d\n", ret);

	/*
	 * Process pinctrl for the serial0 and serial3, aka WKUP_UART0 and
	 * MAIN_UART1 modules and continue regardless of the result of pinctrl.
	 * Do this without probing the device, but instead by searching the
	 * device that would request the given sequence number if probed. The
	 * UARTs will be used by the DM firmware and TIFS firmware images
	 * respectively and the firmware depend on SPL to initialize the pin
	 * settings.
	 */
	ret = uclass_find_device_by_seq(UCLASS_SERIAL, 0, &dev);
	if (!ret)
		pinctrl_select_state(dev, "default");

	ret = uclass_find_device_by_seq(UCLASS_SERIAL, 3, &dev);
	if (!ret)
		pinctrl_select_state(dev, "default");

	/*
	 * Allow establishing an early console as required for example when
	 * doing a UART-based boot. Note that this console may not "survive"
	 * through a SYSFW PM-init step and will need a re-init in some way
	 * due to changing module clock frequencies.
	 */
	if (IS_ENABLED(CONFIG_K3_EARLY_CONS)) {
		ret = early_console_init();
		if (ret)
			panic("early_console_init() failed: %d\n", ret);
	}

	/*
	 * Configure and start up system controller firmware. Provide
	 * the U-Boot console init function to the SYSFW post-PM configuration
	 * callback hook, effectively switching on (or over) the console
	 * output.
	 */
	if (IS_ENABLED(CONFIG_K3_LOAD_SYSFW)) {
		ret = is_rom_loaded_sysfw(&bootdata);
		if (!ret)
			panic("ROM has not loaded TIFS firmware\n");

		k3_sysfw_loader(true, NULL, NULL);

		/* Disable ROM configured firewalls */
		remove_fwl_configs(cbass_main_fwls,
				   ARRAY_SIZE(cbass_main_fwls));
	}

	/*
	 * Force probe of clk_k3 driver here to ensure basic default clock
	 * configuration is always done.
	 */
	if (IS_ENABLED(CONFIG_SPL_CLK_K3)) {
		ret = uclass_get_device_by_driver(UCLASS_CLK,
						  DM_DRIVER_GET(ti_clk),
						  &dev);
		if (ret)
			printf("Failed to initialize clk-k3!\n");
	}

	preloader_console_init();

	/* Output System Firmware version info */
	k3_sysfw_print_ver();

	if (IS_ENABLED(CONFIG_K3_AM62A_DDRSS)) {
		ret = uclass_get_device(UCLASS_RAM, 0, &dev);
		if (ret)
			panic("DRAM init failed: %d\n", ret);
	}

	if (IS_ENABLED(CONFIG_ESM_K3)) {
		/* Probe/configure ESM0 */
		ret = uclass_get_device_by_name(UCLASS_MISC, "esm@420000", &dev);
		if (ret)
			printf("esm main init failed: %d\n", ret);

		/* Probe/configure MCUESM */
		ret = uclass_get_device_by_name(UCLASS_MISC, "esm@4100000", &dev);
		if (ret)
			printf("esm mcu init failed: %d\n", ret);

		enable_mcu_esm_reset();
	}

	spl_enable_cache();

	setup_qos();
	debug("am62px_init: %s done\n", __func__);

	fixup_a53_cpu_freq_by_speed_grade();
}

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);
	u32 bootmode = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
				MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT;
	u32 bootmode_cfg = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK) >>
			    MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT;

	switch (bootmode) {
	case BOOT_DEVICE_EMMC:
		return MMCSD_MODE_EMMCBOOT;
	case BOOT_DEVICE_MMC:
		if (bootmode_cfg & MAIN_DEVSTAT_PRIMARY_MMC_FS_RAW_MASK)
			return MMCSD_MODE_RAW;
		fallthrough;
	default:
		return MMCSD_MODE_FS;
	}
}

static u32 __get_backup_bootmedia(u32 devstat)
{
	u32 bkup_bootmode = (devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_MASK) >>
				MAIN_DEVSTAT_BACKUP_BOOTMODE_SHIFT;
	u32 bkup_bootmode_cfg =
			(devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_MASK) >>
				MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_SHIFT;

	switch (bkup_bootmode) {
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;

	case BACKUP_BOOT_DEVICE_USB:
		return BOOT_DEVICE_USB;

	case BACKUP_BOOT_DEVICE_ETHERNET:
		return BOOT_DEVICE_ETHERNET;

	case BACKUP_BOOT_DEVICE_MMC:
		if (bkup_bootmode_cfg)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BACKUP_BOOT_DEVICE_I2C:
		return BOOT_DEVICE_I2C;

	case BACKUP_BOOT_DEVICE_DFU:
		if (bkup_bootmode_cfg & MAIN_DEVSTAT_BACKUP_USB_MODE_MASK)
			return BOOT_DEVICE_USB;
		return BOOT_DEVICE_DFU;
	};

	return BOOT_DEVICE_RAM;
}

static u32 __get_primary_bootmedia(u32 devstat)
{
	u32 bootmode = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
				MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT;
	u32 bootmode_cfg = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK) >>
				MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT;

	switch (bootmode) {
	case BOOT_DEVICE_OSPI:
		fallthrough;
	case BOOT_DEVICE_QSPI:
		fallthrough;
	case BOOT_DEVICE_XSPI:
		fallthrough;
	case BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BOOT_DEVICE_ETHERNET_RGMII:
		fallthrough;
	case BOOT_DEVICE_ETHERNET_RMII:
		return BOOT_DEVICE_ETHERNET;

	case BOOT_DEVICE_EMMC:
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_SPI_NAND:
		return BOOT_DEVICE_SPINAND;

	case BOOT_DEVICE_MMC:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_MMC_PORT_MASK) >>
				MAIN_DEVSTAT_PRIMARY_MMC_PORT_SHIFT)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_DFU:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_USB_MODE_MASK) >>
		    MAIN_DEVSTAT_PRIMARY_USB_MODE_SHIFT)
			return BOOT_DEVICE_USB;
		return BOOT_DEVICE_DFU;

	case BOOT_DEVICE_NOBOOT:
		return BOOT_DEVICE_RAM;
	}

	return bootmode;
}

u32 spl_boot_device(void)
{
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);
	u32 bootmedia;

	if (bootindex == K3_PRIMARY_BOOTMODE)
		bootmedia = __get_primary_bootmedia(devstat);
	else
		bootmedia = __get_backup_bootmedia(devstat);

	debug("am62px_init: %s: devstat = 0x%x bootmedia = 0x%x bootindex = %d\n",
	      __func__, devstat, bootmedia, bootindex);
	return bootmedia;
}
