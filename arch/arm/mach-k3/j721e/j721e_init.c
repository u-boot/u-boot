// SPDX-License-Identifier: GPL-2.0+
/*
 * J721E: SoC specific initialization
 *
 * Copyright (C) 2018-2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <init.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/armv7_mpu.h>
#include <asm/arch/hardware.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <fdtdec.h>
#include <mmc.h>
#include <remoteproc.h>
#include <k3-avs.h>

#include "../sysfw-loader.h"
#include "../common.h"

/* NAVSS North Bridge (NB) registers */
#define NAVSS0_NBSS_NB0_CFG_MMRS		0x03802000
#define NAVSS0_NBSS_NB1_CFG_MMRS		0x03803000
#define NAVSS0_NBSS_NB0_CFG_NB_THREADMAP	(NAVSS0_NBSS_NB0_CFG_MMRS + 0x10)
#define NAVSS0_NBSS_NB1_CFG_NB_THREADMAP	(NAVSS0_NBSS_NB1_CFG_MMRS + 0x10)
/*
 * Thread Map for North Bridge Configuration
 * Each bit is for each VBUSM source.
 * Bit[0] maps orderID 0-7 to VBUSM.C thread number
 * Bit[1] maps orderID 8-15 to VBUSM.C thread number
 * When bit has value 0: VBUSM.C thread 0 (non-real time traffic)
 * When bit has value 1: VBUSM.C thread 2 (real time traffic)
 */
#define NB_THREADMAP_BIT0				BIT(0)
#define NB_THREADMAP_BIT1				BIT(1)

/* TISCI DEV ID for A72, MSMC Clock */
#define DEV_A72SS0_CORE0_0_ID 202
#define DEV_A72SS0_CORE0_0_ARM_CLK_CLK_ID 2
#define DEV_A72SS0_CORE0_ID 4
#define DEV_A72SS0_CORE0_MSMC_CLK_ID 1

#ifdef CONFIG_K3_LOAD_SYSFW
struct fwl_data cbass_hc_cfg0_fwls[] = {
#if defined(CONFIG_SOC_K3_J721E)
	{ "PCIE0_CFG", 2560, 8 },
	{ "PCIE1_CFG", 2561, 8 },
	{ "USB3SS0_CORE", 2568, 4 },
	{ "USB3SS1_CORE", 2570, 4 },
	{ "EMMC8SS0_CFG", 2576, 4 },
	{ "UFS_HCI0_CFG", 2580, 4 },
	{ "SERDES0", 2584, 1 },
	{ "SERDES1", 2585, 1 },
#elif defined(CONFIG_SOC_K3_J7200)
	{ "PCIE1_CFG", 2561, 7 },
#endif
}, cbass_hc0_fwls[] = {
#if defined(CONFIG_SOC_K3_J721E)
	{ "PCIE0_HP", 2528, 24 },
	{ "PCIE0_LP", 2529, 24 },
	{ "PCIE1_HP", 2530, 24 },
	{ "PCIE1_LP", 2531, 24 },
#endif
}, cbass_rc_cfg0_fwls[] = {
	{ "EMMCSD4SS0_CFG", 2380, 4 },
}, cbass_rc0_fwls[] = {
	{ "GPMC0", 2310, 8 },
}, infra_cbass0_fwls[] = {
	{ "PLL_MMR0", 8, 26 },
	{ "CTRL_MMR0", 9, 16 },
}, mcu_cbass0_fwls[] = {
	{ "MCU_R5FSS0_CORE0", 1024, 4 },
	{ "MCU_R5FSS0_CORE0_CFG", 1025, 2 },
	{ "MCU_R5FSS0_CORE1", 1028, 4 },
	{ "MCU_FSS0_CFG", 1032, 12 },
	{ "MCU_FSS0_S1", 1033, 8 },
	{ "MCU_FSS0_S0", 1036, 8 },
	{ "MCU_PSROM49152X32", 1048, 1 },
	{ "MCU_MSRAM128KX64", 1050, 8 },
	{ "MCU_CTRL_MMR0", 1200, 8 },
	{ "MCU_PLL_MMR0", 1201, 3 },
	{ "MCU_CPSW0", 1220, 2 },
}, wkup_cbass0_fwls[] = {
	{ "WKUP_CTRL_MMR0", 131, 16 },
};
#endif

static void ctrl_mmr_unlock(void)
{
	/* Unlock all WKUP_CTRL_MMR0 module registers */
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 0);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 1);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 2);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 3);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 4);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 6);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 7);

	/* Unlock all MCU_CTRL_MMR0 module registers */
	mmr_unlock(MCU_CTRL_MMR0_BASE, 0);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 1);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 2);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 3);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 4);

	/* Unlock all CTRL_MMR0 module registers */
	mmr_unlock(CTRL_MMR0_BASE, 0);
	mmr_unlock(CTRL_MMR0_BASE, 1);
	mmr_unlock(CTRL_MMR0_BASE, 2);
	mmr_unlock(CTRL_MMR0_BASE, 3);
	mmr_unlock(CTRL_MMR0_BASE, 5);
	if (soc_is_j721e())
		mmr_unlock(CTRL_MMR0_BASE, 6);
	mmr_unlock(CTRL_MMR0_BASE, 7);
}

#if defined(CONFIG_K3_LOAD_SYSFW)
void k3_mmc_stop_clock(void)
{
	if (spl_boot_device() == BOOT_DEVICE_MMC1) {
		struct mmc *mmc = find_mmc_device(0);

		if (!mmc)
			return;

		mmc->saved_clock = mmc->clock;
		mmc_set_clock(mmc, 0, true);
	}
}

void k3_mmc_restart_clock(void)
{
	if (spl_boot_device() == BOOT_DEVICE_MMC1) {
		struct mmc *mmc = find_mmc_device(0);

		if (!mmc)
			return;

		mmc_set_clock(mmc, mmc->saved_clock, false);
	}
}
#endif

/* Setup North Bridge registers to map ORDERID 8-15 to RT traffic */
static void setup_navss_nb(void)
{
	writel(NB_THREADMAP_BIT1, (uintptr_t)NAVSS0_NBSS_NB0_CFG_NB_THREADMAP);
	writel(NB_THREADMAP_BIT1, (uintptr_t)NAVSS0_NBSS_NB1_CFG_NB_THREADMAP);
}

#if defined(CONFIG_CPU_V7R) && defined(CONFIG_K3_AVS0)
static int get_clock_index_by_dev_id(ofnode node, u32 dev_id, u32 clk_id)
{
	ofnode clknode;
	int count, i;
	struct ofnode_phandle_args phandle_args;

	clknode = ofnode_by_compatible(ofnode_null(), "ti,k2g-sci-clk");
	if (!ofnode_valid(clknode)) {
		printf("%s: clock-controller not found\n", __func__);
		return -ENODEV;
	}

	count = ofnode_count_phandle_with_args(node,  "assigned-clocks", "#clock-cells", 0);
	for (i = 0; i < count; i++) {
		if (ofnode_parse_phandle_with_args(node, "assigned-clocks",
						   "#clock-cells", 0, i, &phandle_args)) {
			printf("%s: Could not parse assigned-clocks at index %d\n", __func__, i);
			continue;
		}
		if (ofnode_equal(clknode, phandle_args.node) &&
		    phandle_args.args[0] == dev_id && phandle_args.args[1] == clk_id)
			return i;
	}
	return -1;
}

static int fdt_fixup_a72ss_clock_frequency(void)
{
	int index, size;
	u32 *rates;
	ofnode node;

	node = ofnode_by_compatible(ofnode_null(), "ti,am654-rproc");
	if (!ofnode_valid(node)) {
		printf("%s: A72 not found\n", __func__);
		return -ENODEV;
	}

	rates = fdt_getprop_w(ofnode_to_fdt(node), ofnode_to_offset(node),
			      "assigned-clock-rates", &size);
	if (!rates) {
		printf("%s: Wrong A72 assigned-clocks-rates configuration\n", __func__);
		return -1;
	}

	/* Update A72 Clock Frequency to OPP_LOW spec */
	index = get_clock_index_by_dev_id(node,
					  DEV_A72SS0_CORE0_0_ID,
					  DEV_A72SS0_CORE0_0_ARM_CLK_CLK_ID);
	if (index < 0 || index >= (size / sizeof(u32))) {
		printf("%s: Wrong A72 assigned-clocks configuration\n", __func__);
		return -1;
	}
	rates[index] = cpu_to_fdt32(1000000000);
	printf("Changed A72 CPU frequency to %dHz in DT\n", 1000000000);

	/* Update MSMC Clock Frequency to OPP_LOW spec */
	index = get_clock_index_by_dev_id(node,
					  DEV_A72SS0_CORE0_ID,
					  DEV_A72SS0_CORE0_MSMC_CLK_ID);
	if (index < 0 || index >= (size / sizeof(u32))) {
		printf("%s: Wrong A72 assigned-clocks configuration\n", __func__);
		return -1;
	}
	rates[index] = cpu_to_fdt32(500000000);
	printf("Changed MSMC frequency to %dHz in DT\n", 500000000);

	return 0;
}
#endif

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

#ifdef CONFIG_SPL_OF_LIST
void do_dt_magic(void)
{
	int ret, rescan, mmc_dev = -1;
	static struct mmc *mmc;

	/* Perform board detection */
	do_board_detect();

	/*
	 * Board detection has been done.
	 * Let us see if another dtb wouldn't be a better match
	 * for our board
	 */
	if (IS_ENABLED(CONFIG_CPU_V7R)) {
		ret = fdtdec_resetup(&rescan);
		if (!ret && rescan) {
			dm_uninit();
			dm_init_and_scan(true);
		}
	}

	/*
	 * Because of multi DTB configuration, the MMC device has
	 * to be re-initialized after reconfiguring FDT inorder to
	 * boot from MMC. Do this when boot mode is MMC and ROM has
	 * not loaded SYSFW.
	 */
	switch (spl_boot_device()) {
	case BOOT_DEVICE_MMC1:
		mmc_dev = 0;
		break;
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		mmc_dev = 1;
		break;
	}

	if (mmc_dev > 0 && !is_rom_loaded_sysfw(&bootdata)) {
		ret = mmc_init_device(mmc_dev);
		if (!ret) {
			mmc = find_mmc_device(mmc_dev);
			if (mmc) {
				ret = mmc_init(mmc);
				if (ret) {
					printf("mmc init failed with error: %d\n", ret);
				}
			}
		}
	}
}
#endif

void board_init_f(ulong dummy)
{
	int ret;
#if defined(CONFIG_K3_J721E_DDRSS) || defined(CONFIG_K3_LOAD_SYSFW)
	struct udevice *dev;
#endif
	/*
	 * Cannot delay this further as there is a chance that
	 * K3_BOOT_PARAM_TABLE_INDEX can be over written by SPL MALLOC section.
	 */
	store_boot_info_from_rom();

	/* Make all control module registers accessible */
	ctrl_mmr_unlock();

#ifdef CONFIG_CPU_V7R
	disable_linefill_optimization();
	setup_k3_mpu_regions();
#endif

	/* Init DM early */
	spl_early_init();

#ifdef CONFIG_K3_LOAD_SYSFW
	/*
	 * Process pinctrl for the serial0 a.k.a. MCU_UART0 module and continue
	 * regardless of the result of pinctrl. Do this without probing the
	 * device, but instead by searching the device that would request the
	 * given sequence number if probed. The UART will be used by the system
	 * firmware (SYSFW) image for various purposes and SYSFW depends on us
	 * to initialize its pin settings.
	 */
	ret = uclass_find_device_by_seq(UCLASS_SERIAL, 0, &dev);
	if (!ret)
		pinctrl_select_state(dev, "default");

	/*
	 * Force probe of clk_k3 driver here to ensure basic default clock
	 * configuration is always done.
	 */
	if (IS_ENABLED(CONFIG_SPL_CLK_K3)) {
		ret = uclass_get_device_by_driver(UCLASS_CLK,
						  DM_DRIVER_GET(ti_clk),
						  &dev);
		if (ret)
			panic("Failed to initialize clk-k3!\n");
	}

	/*
	 * Load, start up, and configure system controller firmware. Provide
	 * the U-Boot console init function to the SYSFW post-PM configuration
	 * callback hook, effectively switching on (or over) the console
	 * output.
	 */
	k3_sysfw_loader(is_rom_loaded_sysfw(&bootdata),
			k3_mmc_stop_clock, k3_mmc_restart_clock);

#ifdef CONFIG_SPL_OF_LIST
	do_dt_magic();
#endif

	/* Prepare console output */
	preloader_console_init();

	/* Disable ROM configured firewalls right after loading sysfw */
	remove_fwl_configs(cbass_hc_cfg0_fwls, ARRAY_SIZE(cbass_hc_cfg0_fwls));
	remove_fwl_configs(cbass_hc0_fwls, ARRAY_SIZE(cbass_hc0_fwls));
	remove_fwl_configs(cbass_rc_cfg0_fwls, ARRAY_SIZE(cbass_rc_cfg0_fwls));
	remove_fwl_configs(cbass_rc0_fwls, ARRAY_SIZE(cbass_rc0_fwls));
	remove_fwl_configs(infra_cbass0_fwls, ARRAY_SIZE(infra_cbass0_fwls));
	remove_fwl_configs(mcu_cbass0_fwls, ARRAY_SIZE(mcu_cbass0_fwls));
	remove_fwl_configs(wkup_cbass0_fwls, ARRAY_SIZE(wkup_cbass0_fwls));
#else
	/* Prepare console output */
	preloader_console_init();
#endif

	/* Shutdown MCU_R5 Core 1 in Split mode at A72 SPL Stage */
	if (IS_ENABLED(CONFIG_ARM64)) {
		ret = shutdown_mcu_r5_core1();
		if (ret)
			printf("Unable to shutdown MCU R5 core 1, %d\n", ret);
	}

	/* Output System Firmware version info */
	k3_sysfw_print_ver();

	/* Output DM Firmware version info */
	if (IS_ENABLED(CONFIG_ARM64))
		k3_dm_print_ver();

	/* Perform board detection */
	do_board_detect();

#if defined(CONFIG_CPU_V7R) && defined(CONFIG_K3_AVS0)
	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(k3_avs),
					  &dev);
	if (ret) {
		printf("AVS init failed: %d\n", ret);
	} else if (IS_ENABLED(CONFIG_K3_OPP_LOW)) {
		ret = k3_avs_check_opp(dev, J721E_VDD_MPU, AM6_OPP_LOW);
		if (ret) {
			printf("OPP_LOW: k3_avs_check_opp failed: %d\n", ret);
		} else {
			ret = fdt_fixup_a72ss_clock_frequency();
			if (ret)
				printf("OPP_LOW: fdt_fixup_a72ss_clock_frequency failed: %d\n",
				       ret);
		}
	}
#endif

#if defined(CONFIG_K3_J721E_DDRSS)
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret)
		panic("DRAM init failed: %d\n", ret);
#endif
	spl_enable_cache();

	if (IS_ENABLED(CONFIG_CPU_V7R))
		setup_navss_nb();

	setup_qos();
}

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		if (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT)) {
			if (spl_mmc_emmc_boot_partition(mmc))
				return MMCSD_MODE_EMMCBOOT;
			return MMCSD_MODE_FS;
		}
		if (IS_ENABLED(CONFIG_SPL_FS_FAT) || IS_ENABLED(CONFIG_SPL_FS_EXT4))
			return MMCSD_MODE_FS;
		return MMCSD_MODE_EMMCBOOT;
	case BOOT_DEVICE_MMC2:
		return MMCSD_MODE_FS;
	default:
		return MMCSD_MODE_RAW;
	}
}

static u32 __get_backup_bootmedia(u32 main_devstat)
{
	u32 bkup_boot = (main_devstat & MAIN_DEVSTAT_BKUP_BOOTMODE_MASK) >>
			MAIN_DEVSTAT_BKUP_BOOTMODE_SHIFT;

	switch (bkup_boot) {
	case BACKUP_BOOT_DEVICE_USB:
		return BOOT_DEVICE_DFU;
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;
	case BACKUP_BOOT_DEVICE_ETHERNET:
		return BOOT_DEVICE_ETHERNET;
	case BACKUP_BOOT_DEVICE_MMC2:
	{
		u32 port = (main_devstat & MAIN_DEVSTAT_BKUP_MMC_PORT_MASK) >>
			    MAIN_DEVSTAT_BKUP_MMC_PORT_SHIFT;
		if (port == 0x0)
			return BOOT_DEVICE_MMC1;
		return BOOT_DEVICE_MMC2;
	}
	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;
	case BACKUP_BOOT_DEVICE_I2C:
		return BOOT_DEVICE_I2C;
	}

	return BOOT_DEVICE_RAM;
}

static u32 __get_primary_bootmedia(u32 main_devstat, u32 wkup_devstat)
{

	u32 bootmode = (wkup_devstat & WKUP_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
			WKUP_DEVSTAT_PRIMARY_BOOTMODE_SHIFT;

	bootmode |= (main_devstat & MAIN_DEVSTAT_BOOT_MODE_B_MASK) <<
			BOOT_MODE_B_SHIFT;

	if (bootmode == BOOT_DEVICE_OSPI || bootmode == BOOT_DEVICE_QSPI ||
	    bootmode == BOOT_DEVICE_XSPI)
		bootmode = BOOT_DEVICE_SPI;

	if (bootmode == BOOT_DEVICE_MMC2) {
		u32 port = (main_devstat &
			    MAIN_DEVSTAT_PRIM_BOOTMODE_MMC_PORT_MASK) >>
			   MAIN_DEVSTAT_PRIM_BOOTMODE_PORT_SHIFT;
		if (port == 0x0)
			bootmode = BOOT_DEVICE_MMC1;
	}

	return bootmode;
}

u32 spl_spi_boot_bus(void)
{
	u32 wkup_devstat = readl(CTRLMMR_WKUP_DEVSTAT);
	u32 main_devstat = readl(CTRLMMR_MAIN_DEVSTAT);
	u32 bootmode = ((wkup_devstat & WKUP_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
			WKUP_DEVSTAT_PRIMARY_BOOTMODE_SHIFT) |
			((main_devstat & MAIN_DEVSTAT_BOOT_MODE_B_MASK) << BOOT_MODE_B_SHIFT);

	return (bootmode == BOOT_DEVICE_QSPI) ? 1 : 0;
}

u32 spl_boot_device(void)
{
	u32 wkup_devstat = readl(CTRLMMR_WKUP_DEVSTAT);
	u32 main_devstat;

	if (wkup_devstat & WKUP_DEVSTAT_MCU_ONLY_MASK) {
		printf("ERROR: MCU only boot is not yet supported\n");
		return BOOT_DEVICE_RAM;
	}

	/* MAIN CTRL MMR can only be read if MCU ONLY is 0 */
	main_devstat = readl(CTRLMMR_MAIN_DEVSTAT);

	if (bootindex == K3_PRIMARY_BOOTMODE)
		return __get_primary_bootmedia(main_devstat, wkup_devstat);
	else
		return __get_backup_bootmedia(main_devstat);
}
