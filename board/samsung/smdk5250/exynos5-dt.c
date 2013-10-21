/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cros_ec.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <errno.h>
#include <i2c.h>
#include <netdev.h>
#include <spi.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/sromc.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>
#include <tmu.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined CONFIG_EXYNOS_TMU
/*
 * Boot Time Thermal Analysis for SoC temperature threshold breach
 */
static void boot_temp_check(void)
{
	int temp;

	switch (tmu_monitor(&temp)) {
	/* Status TRIPPED ans WARNING means corresponding threshold breach */
	case TMU_STATUS_TRIPPED:
		puts("EXYNOS_TMU: TRIPPING! Device power going down ...\n");
		set_ps_hold_ctrl();
		hang();
		break;
	case TMU_STATUS_WARNING:
		puts("EXYNOS_TMU: WARNING! Temperature very high\n");
		break;
	/*
	 * TMU_STATUS_INIT means something is wrong with temperature sensing
	 * and TMU status was changed back from NORMAL to INIT.
	 */
	case TMU_STATUS_INIT:
	default:
		debug("EXYNOS_TMU: Unknown TMU state\n");
	}
}
#endif

struct local_info {
	struct cros_ec_dev *cros_ec_dev;	/* Pointer to cros_ec device */
	int cros_ec_err;			/* Error for cros_ec, 0 if ok */
};

static struct local_info local;

#ifdef CONFIG_SOUND_MAX98095
static void  board_enable_audio_codec(void)
{
	struct exynos5_gpio_part1 *gpio1 = (struct exynos5_gpio_part1 *)
						samsung_get_base_gpio_part1();

	/* Enable MAX98095 Codec */
	s5p_gpio_direction_output(&gpio1->x1, 7, 1);
	s5p_gpio_set_pull(&gpio1->x1, 7, GPIO_PULL_NONE);
}
#endif

struct cros_ec_dev *board_get_cros_ec_dev(void)
{
	return local.cros_ec_dev;
}

static int board_init_cros_ec_devices(const void *blob)
{
	local.cros_ec_err = cros_ec_init(blob, &local.cros_ec_dev);
	if (local.cros_ec_err)
		return -1;  /* Will report in board_late_init() */

	return 0;
}

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

	if (board_init_cros_ec_devices(gd->fdt_blob))
		return -1;

#ifdef CONFIG_SOUND_MAX98095
	board_enable_audio_codec();
#endif
	return 0;
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

#if defined(CONFIG_POWER)
static int pmic_reg_update(struct pmic *p, int reg, uint regval)
{
	u32 val;
	int ret = 0;

	ret = pmic_reg_read(p, reg, &val);
	if (ret) {
		debug("%s: PMIC %d register read failed\n", __func__, reg);
		return -1;
	}
	val |= regval;
	ret = pmic_reg_write(p, reg, val);
	if (ret) {
		debug("%s: PMIC %d register write failed\n", __func__, reg);
		return -1;
	}
	return 0;
}

int power_init_board(void)
{
	struct pmic *p;

	set_ps_hold_ctrl();

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	if (pmic_init(I2C_PMIC))
		return -1;

	p = pmic_get("MAX77686_PMIC");
	if (!p)
		return -ENODEV;

	if (pmic_probe(p))
		return -1;

	if (pmic_reg_update(p, MAX77686_REG_PMIC_32KHZ, MAX77686_32KHCP_EN))
		return -1;

	if (pmic_reg_update(p, MAX77686_REG_PMIC_BBAT,
			    MAX77686_BBCHOSTEN | MAX77686_BBCVS_3_5V))
		return -1;

	/* VDD_MIF */
	if (pmic_reg_write(p, MAX77686_REG_PMIC_BUCK1OUT,
			   MAX77686_BUCK1OUT_1V)) {
		debug("%s: PMIC %d register write failed\n", __func__,
		      MAX77686_REG_PMIC_BUCK1OUT);
		return -1;
	}

	if (pmic_reg_update(p, MAX77686_REG_PMIC_BUCK1CRTL,
			    MAX77686_BUCK1CTRL_EN))
		return -1;

	/* VDD_ARM */
	if (pmic_reg_write(p, MAX77686_REG_PMIC_BUCK2DVS1,
			   MAX77686_BUCK2DVS1_1_3V)) {
		debug("%s: PMIC %d register write failed\n", __func__,
		      MAX77686_REG_PMIC_BUCK2DVS1);
		return -1;
	}

	if (pmic_reg_update(p, MAX77686_REG_PMIC_BUCK2CTRL1,
			    MAX77686_BUCK2CTRL_ON))
		return -1;

	/* VDD_INT */
	if (pmic_reg_write(p, MAX77686_REG_PMIC_BUCK3DVS1,
			   MAX77686_BUCK3DVS1_1_0125V)) {
		debug("%s: PMIC %d register write failed\n", __func__,
		      MAX77686_REG_PMIC_BUCK3DVS1);
		return -1;
	}

	if (pmic_reg_update(p, MAX77686_REG_PMIC_BUCK3CTRL,
			    MAX77686_BUCK3CTRL_ON))
		return -1;

	/* VDD_G3D */
	if (pmic_reg_write(p, MAX77686_REG_PMIC_BUCK4DVS1,
			   MAX77686_BUCK4DVS1_1_2V)) {
		debug("%s: PMIC %d register write failed\n", __func__,
		      MAX77686_REG_PMIC_BUCK4DVS1);
		return -1;
	}

	if (pmic_reg_update(p, MAX77686_REG_PMIC_BUCK4CTRL1,
			    MAX77686_BUCK3CTRL_ON))
		return -1;

	/* VDD_LDO2 */
	if (pmic_reg_update(p, MAX77686_REG_PMIC_LDO2CTRL1,
			    MAX77686_LD02CTRL1_1_5V | EN_LDO))
		return -1;

	/* VDD_LDO3 */
	if (pmic_reg_update(p, MAX77686_REG_PMIC_LDO3CTRL1,
			    MAX77686_LD03CTRL1_1_8V | EN_LDO))
		return -1;

	/* VDD_LDO5 */
	if (pmic_reg_update(p, MAX77686_REG_PMIC_LDO5CTRL1,
			    MAX77686_LD05CTRL1_1_8V | EN_LDO))
		return -1;

	/* VDD_LDO10 */
	if (pmic_reg_update(p, MAX77686_REG_PMIC_LDO10CTRL1,
			    MAX77686_LD10CTRL1_1_8V | EN_LDO))
		return -1;

	return 0;
}
#endif

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

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	const char *board_name;

	board_name = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
	if (board_name == NULL)
		printf("\nUnknown Board\n");
	else
		printf("\nBoard: %s\n", board_name);

	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	int ret;
	/* dwmmc initializattion for available channels */
	ret = exynos_dwmmc_init(gd->fdt_blob);
	if (ret)
		debug("dwmmc init failed\n");

	return ret;
}
#endif

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
	return err;
}
#endif

#ifdef CONFIG_LCD
void exynos_cfg_lcd_gpio(void)
{
	struct exynos5_gpio_part1 *gpio1 =
		(struct exynos5_gpio_part1 *)samsung_get_base_gpio_part1();

	/* For Backlight */
	s5p_gpio_cfg_pin(&gpio1->b2, 0, GPIO_OUTPUT);
	s5p_gpio_set_value(&gpio1->b2, 0, 1);

	/* LCD power on */
	s5p_gpio_cfg_pin(&gpio1->x1, 5, GPIO_OUTPUT);
	s5p_gpio_set_value(&gpio1->x1, 5, 1);

	/* Set Hotplug detect for DP */
	s5p_gpio_cfg_pin(&gpio1->x0, 7, GPIO_FUNC(0x3));
}

void exynos_set_dp_phy(unsigned int onoff)
{
	set_dp_phy_ctrl(onoff);
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	stdio_print_current_devices();

	if (local.cros_ec_err) {
		/* Force console on */
		gd->flags &= ~GD_FLG_SILENT;

		printf("cros-ec communications failure %d\n",
		       local.cros_ec_err);
		puts("\nPlease reset with Power+Refresh\n\n");
		panic("Cannot init cros-ec device");
		return -1;
	}
	return 0;
}
#endif
