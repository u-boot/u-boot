/*
 * (C) Copyright 2013 SAMSUNG Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cros_ec.h>
#include <errno.h>
#include <fdtdec.h>
#include <spi.h>
#include <tmu.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/board.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <power/pmic.h>
#include <asm/arch/sromc.h>
#include <lcd.h>
#include <samsung/misc.h>

DECLARE_GLOBAL_DATA_PTR;

int __exynos_early_init_f(void)
{
	return 0;
}
int exynos_early_init_f(void)
	__attribute__((weak, alias("__exynos_early_init_f")));

int __exynos_power_init(void)
{
	return 0;
}
int exynos_power_init(void)
	__attribute__((weak, alias("__exynos_power_init")));

#if defined CONFIG_EXYNOS_TMU
/* Boot Time Thermal Analysis for SoC temperature threshold breach */
static void boot_temp_check(void)
{
	int temp;

	switch (tmu_monitor(&temp)) {
	case TMU_STATUS_NORMAL:
		break;
	case TMU_STATUS_TRIPPED:
		/*
		 * Status TRIPPED ans WARNING means corresponding threshold
		 * breach
		 */
		puts("EXYNOS_TMU: TRIPPING! Device power going down ...\n");
		set_ps_hold_ctrl();
		hang();
		break;
	case TMU_STATUS_WARNING:
		puts("EXYNOS_TMU: WARNING! Temperature very high\n");
		break;
	case TMU_STATUS_INIT:
		/*
		 * TMU_STATUS_INIT means something is wrong with temperature
		 * sensing and TMU status was changed back from NORMAL to INIT.
		 */
		puts("EXYNOS_TMU: WARNING! Temperature sensing not done\n");
		break;
	default:
		debug("EXYNOS_TMU: Unknown TMU state\n");
	}
}
#endif

int board_init(void)
{
	gd->bd->bi_boot_params = (PHYS_SDRAM_1 + 0x100UL);
#if defined CONFIG_EXYNOS_TMU
	if (tmu_init(gd->fdt_blob) != TMU_STATUS_NORMAL) {
		debug("%s: Failed to init TMU\n", __func__);
		return -1;
	}
	boot_temp_check();
#endif

#ifdef CONFIG_EXYNOS_SPI
	spi_init();
#endif
	return exynos_init();
}

int dram_init(void)
{
	int i;
	u32 addr;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		addr = CONFIG_SYS_SDRAM_BASE + (i * SDRAM_BANK_SIZE);
		gd->ram_size += get_ram_size((long *)addr, SDRAM_BANK_SIZE);
	}
	return 0;
}

void dram_init_banksize(void)
{
	int i;
	u32 addr, size;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		addr = CONFIG_SYS_SDRAM_BASE + (i * SDRAM_BANK_SIZE);
		size = get_ram_size((long *)addr, SDRAM_BANK_SIZE);

		gd->bd->bi_dram[i].start = addr;
		gd->bd->bi_dram[i].size = size;
	}
}

static int board_uart_init(void)
{
	int err, uart_id, ret = 0;

	for (uart_id = PERIPH_ID_UART0; uart_id <= PERIPH_ID_UART3; uart_id++) {
		err = exynos_pinmux_config(uart_id, PINMUX_FLAG_NONE);
		if (err) {
			debug("UART%d not configured\n",
			      (uart_id - PERIPH_ID_UART0));
			ret |= err;
		}
	}
	return ret;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	int err;

	err = board_uart_init();
	if (err) {
		debug("UART init failed\n");
		return err;
	}

#ifdef CONFIG_SYS_I2C_INIT_BOARD
	board_i2c_init(gd->fdt_blob);
#endif

	return exynos_early_init_f();
}
#endif

#if defined(CONFIG_POWER)
int power_init_board(void)
{
	set_ps_hold_ctrl();

	return exynos_power_init();
}
#endif

#ifdef CONFIG_OF_CONTROL
#ifdef CONFIG_SMC911X
static int decode_sromc(const void *blob, struct fdt_sromc *config)
{
	int err;
	int node;

	node = fdtdec_next_compatible(blob, 0, COMPAT_SAMSUNG_EXYNOS5_SROMC);
	if (node < 0) {
		debug("Could not find SROMC node\n");
		return node;
	}

	config->bank = fdtdec_get_int(blob, node, "bank", 0);
	config->width = fdtdec_get_int(blob, node, "width", 2);

	err = fdtdec_get_int_array(blob, node, "srom-timing", config->timing,
			FDT_SROM_TIMING_COUNT);
	if (err < 0) {
		debug("Could not decode SROMC configuration Error: %s\n",
		      fdt_strerror(err));
		return -FDT_ERR_NOTFOUND;
	}
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_SMC911X
	u32 smc_bw_conf, smc_bc_conf;
	struct fdt_sromc config;
	fdt_addr_t base_addr;
	int node;

	node = decode_sromc(gd->fdt_blob, &config);
	if (node < 0) {
		debug("%s: Could not find sromc configuration\n", __func__);
		return 0;
	}
	node = fdtdec_next_compatible(gd->fdt_blob, node, COMPAT_SMSC_LAN9215);
	if (node < 0) {
		debug("%s: Could not find lan9215 configuration\n", __func__);
		return 0;
	}

	/* We now have a node, so any problems from now on are errors */
	base_addr = fdtdec_get_addr(gd->fdt_blob, node, "reg");
	if (base_addr == FDT_ADDR_T_NONE) {
		debug("%s: Could not find lan9215 address\n", __func__);
		return -1;
	}

	/* Ethernet needs data bus width of 16 bits */
	if (config.width != 2) {
		debug("%s: Unsupported bus width %d\n", __func__,
		      config.width);
		return -1;
	}
	smc_bw_conf = SROMC_DATA16_WIDTH(config.bank)
			| SROMC_BYTE_ENABLE(config.bank);

	smc_bc_conf = SROMC_BC_TACS(config.timing[FDT_SROM_TACS])   |
			SROMC_BC_TCOS(config.timing[FDT_SROM_TCOS]) |
			SROMC_BC_TACC(config.timing[FDT_SROM_TACC]) |
			SROMC_BC_TCOH(config.timing[FDT_SROM_TCOH]) |
			SROMC_BC_TAH(config.timing[FDT_SROM_TAH])   |
			SROMC_BC_TACP(config.timing[FDT_SROM_TACP]) |
			SROMC_BC_PMC(config.timing[FDT_SROM_PMC]);

	/* Select and configure the SROMC bank */
	exynos_pinmux_config(PERIPH_ID_SROMC, config.bank);
	s5p_config_sromc(config.bank, smc_bw_conf, smc_bc_conf);
	return smc911x_initialize(0, base_addr);
#endif
	return 0;
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	int ret;

#ifdef CONFIG_SDHCI
	/* mmc initializattion for available channels */
	ret = exynos_mmc_init(gd->fdt_blob);
	if (ret)
		debug("mmc init failed\n");
#endif
#ifdef CONFIG_DWMMC
	/* dwmmc initializattion for available channels */
	ret = exynos_dwmmc_init(gd->fdt_blob);
	if (ret)
		debug("dwmmc init failed\n");
#endif

	return ret;
}
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	const char *board_name;

	board_name = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
	printf("Board: %s\n", board_name ? board_name : "unknown");

	return 0;
}
#endif
#endif /* CONFIG_OF_CONTROL */

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	stdio_print_current_devices();

	if (cros_ec_get_error()) {
		/* Force console on */
		gd->flags &= ~GD_FLG_SILENT;

		printf("cros-ec communications failure %d\n",
		       cros_ec_get_error());
		puts("\nPlease reset with Power+Refresh\n\n");
		panic("Cannot init cros-ec device");
		return -1;
	}
	return 0;
}
#endif

int arch_early_init_r(void)
{
#ifdef CONFIG_CROS_EC
	if (cros_ec_board_init()) {
		printf("%s: Failed to init EC\n", __func__);
		return 0;
	}
#endif

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	set_board_info();
#endif
#ifdef CONFIG_LCD_MENU
	keys_init();
	check_boot_mode();
#endif
#ifdef CONFIG_CMD_BMP
	if (panel_info.logo_on)
		draw_logo();
#endif
	return 0;
}
#endif
