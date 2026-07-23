// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 *           2021 Purism
 *           2026 Phosh.mobi e.V.
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#include <config.h>
#include <asm/io.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <power/pmic.h>
#include <power/bd71837.h>
#include <hang.h>
#include <init.h>
#include <spl.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <linux/delay.h>
#include <linux/usb/gadget.h>
#include <i2c.h>

extern struct dram_timing_info dram_timing_b0;

void spl_dram_init(void)
{
	/* ddr init, the default SOM uses B0 */
	if ((get_cpu_rev() & 0xfff) == CHIP_REV_2_1)
		ddr_init(&dram_timing);
	else
		ddr_init(&dram_timing_b0);
}

#define USDHC1_PWR_GPIO IMX_GPIO_NR(2, 10)
#define USDHC2_PWR_GPIO IMX_GPIO_NR(2, 19)

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = 1;
		break;
	case USDHC2_BASE_ADDR:
		ret = 1;
		return ret;
	}

	return 1;
}

#define USDHC_PAD_CTRL \
	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_FSEL1)
#define USDHC_GPIO_PAD_CTRL (PAD_CTL_PUE | PAD_CTL_DSE1)

static const iomux_v3_cfg_t usdhc1_pads[] = {
	IMX8MQ_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA0__USDHC1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA1__USDHC1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA2__USDHC1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA3__USDHC1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA4__USDHC1_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA5__USDHC1_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA6__USDHC1_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA7__USDHC1_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_RESET_B__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static const iomux_v3_cfg_t usdhc2_pads[] = {
	IMX8MQ_PAD_SD2_CLK__USDHC2_CLK |
		MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_CMD__USDHC2_CMD |
		MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA0__USDHC2_DATA0 |
		MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA1__USDHC2_DATA1 |
		MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA2__USDHC2_DATA2 |
		MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0x16 */
	IMX8MQ_PAD_SD2_DATA3__USDHC2_DATA3 |
		MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_RESET_B__GPIO2_IO19 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
};

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{ USDHC1_BASE_ADDR, 0, 8 },
	{ USDHC2_BASE_ADDR, 0, 4 },
};

int board_mmc_init(struct bd_info *bis)
{
	int i, ret;
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	for (i = 0; i < CFG_SYS_FSL_USDHC_NUM; i++) {
		log_debug("Initializing FSL USDHC port %d\n", i);
		switch (i) {
		case 0:
			init_clk_usdhc(0);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(USDHC1_CLK_ROOT);
			imx_iomux_v3_setup_multiple_pads(usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
			gpio_request(USDHC1_PWR_GPIO, "usdhc1_reset");
			gpio_direction_output(USDHC1_PWR_GPIO, 0);
			udelay(500);
			gpio_direction_output(USDHC1_PWR_GPIO, 1);
			break;
		case 1:
			init_clk_usdhc(1);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(USDHC2_CLK_ROOT);
			imx_iomux_v3_setup_multiple_pads(usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
			gpio_request(USDHC2_PWR_GPIO, "usdhc2_reset");
			gpio_direction_output(USDHC2_PWR_GPIO, 0);
			udelay(500);
			gpio_direction_output(USDHC2_PWR_GPIO, 1);
			break;
		default:
			log_err("Warning: USDHC controller(%d) not supported\n",
				i + 1);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret)
			return ret;
	}

	return 0;
}

#define LDO_VOLT_EN BIT(6)

#define I2C_PAD_CTRL (PAD_CTL_DSE6 | PAD_CTL_HYS)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = IMX8MQ_PAD_I2C1_SCL__I2C1_SCL | PC,
		.gpio_mode = IMX8MQ_PAD_I2C1_SCL__GPIO5_IO14 | PC,
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = IMX8MQ_PAD_I2C1_SDA__I2C1_SDA | PC,
		.gpio_mode = IMX8MQ_PAD_I2C1_SDA__GPIO5_IO15 | PC,
		.gp = IMX_GPIO_NR(5, 15),
	},
};

#define I2C_PMIC 0
#define POWER_BD71837_I2C_BUS 0
#define POWER_BD71837_I2C_ADDR 0x4B

int power_bd71837_init(unsigned char bus)
{
	static const char name[] = BD718XX_REGULATOR_DRIVER;
	struct pmic *p = pmic_alloc();

	if (!p) {
		log_err("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->interface = I2C_PMIC;
	p->number_of_regs = BD718XX_MAX_REGISTER;
	p->hw.i2c.addr = POWER_BD71837_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}

#define PWR_EN IMX_GPIO_NR(1, 8)
#define HAPTIC_nEN IMX_GPIO_NR(5, 4)
#define IMU_INT IMX_GPIO_NR(3, 19)
#define TS_INT IMX_GPIO_NR(3, 0)
static const iomux_v3_cfg_t pwr_en_pads[] = {
	IMX8MQ_PAD_GPIO1_IO08__GPIO1_IO8 |
		MUX_PAD_CTRL(PAD_CTL_DSE6 | PAD_CTL_FSEL0),
	IMX8MQ_PAD_SPDIF_RX__GPIO5_IO4 |
		MUX_PAD_CTRL(PAD_CTL_DSE6 | PAD_CTL_FSEL1),
	IMX8MQ_PAD_SAI5_RXFS__GPIO3_IO19 | MUX_PAD_CTRL(PAD_CTL_ODE),
	IMX8MQ_PAD_NAND_ALE__GPIO3_IO0 | MUX_PAD_CTRL(PAD_CTL_ODE),
};

int power_init_board(void)
{
	struct pmic *p;
	int ldo[] = { BD718XX_LDO5_VOLT, BD718XX_LDO6_VOLT, BD71837_LDO7_VOLT };
	u32 val;
	int i, rv;

	/* Set the i2c bus */
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

	/*
	 * Init PMIC
	 */
	rv = power_bd71837_init(POWER_BD71837_I2C_BUS);
	if (rv) {
		log_err("%s: power_bd71837_init(%d) error %d\n", __func__,
			POWER_BD71837_I2C_BUS, rv);
		goto out;
	}

	p = pmic_get(BD718XX_REGULATOR_DRIVER);
	if (!p) {
		log_err("%s: pmic_get(%s) failed\n", __func__,
			BD718XX_REGULATOR_DRIVER);
		rv = -ENODEV;
		goto out;
	}

	rv = pmic_probe(p);
	if (rv) {
		log_err("%s: pmic_probe() error %d\n", __func__, rv);
		goto out;
	}

	/*
	 * Unlock all regs
	 */
	pmic_reg_write(p, BD718XX_REGLOCK, 0);

	/* find the reset cause */
	pmic_reg_read(p, 0x29, &val);
	log_debug("%s: reset cause %d\n", __func__, val);

	/*
	 * Reconfigure default voltages and disable:
	 * - BUCK3: VDD_GPU_0V9 (1.00 -> 0.90)
	 * - BUCK4: VDD_VPU_0V9 (1.00 -> 0.90)
	 */
	pmic_reg_write(p, BD71837_BUCK3_VOLT_RUN, 0x14);
	pmic_reg_write(p, BD71837_BUCK4_VOLT_RUN, 0x14);

	/*
	 * Enable PHYs voltages: LDO5-7
	 */
	for (i = 0; i < ARRAY_SIZE(ldo); i++) {
		rv = pmic_reg_read(p, ldo[i], &val);
		if (rv) {
			log_err("%s: pmic_read(%x) error %d\n", __func__,
				ldo[i], rv);
			continue;
		}

		pmic_reg_write(p, ldo[i], val | LDO_VOLT_EN);
	}

	imx_iomux_v3_setup_multiple_pads(pwr_en_pads, ARRAY_SIZE(pwr_en_pads));

	/* set IMU_INT as input */
	gpio_request(IMU_INT, "imu_int");
	gpio_direction_input(IMU_INT);

	/* set TS_INT as input */
	gpio_request(TS_INT, "ts_int");
	gpio_direction_output(TS_INT, 0);

	/* disable the haptic motor */
	gpio_request(HAPTIC_nEN, "haptic_en");
	gpio_direction_output(HAPTIC_nEN, 1);

	gpio_request(PWR_EN, "pwr_en");
	gpio_direction_output(PWR_EN, 1);

	udelay(500);

	rv = 0;
out:
	return 0;
}

void spl_board_init(void)
{
}

/*
 * set some safe defaults for the battery charger
 */
int init_charger_bq25896(void)
{
	int ret;
	u8 val;

	ret = i2c_set_bus_num(0);
	if (ret < 0)
		log_err("Failed to set i2c bus 0: %d", ret);

	/* limit the charge current to 1.6A */
	i2c_reg_write(0x6b, 0x12, 0x20);

	/* set the max voltage to 4.192V */
	val = i2c_reg_read(0x6b, 0x6);
	val = (val & 0xFC) | 0x16 << 2;
	i2c_reg_write(0x6b, 0x6, val);

	return 0;
}

#define SPEAKER_PAD_MUTE_CTRL PAD_CTL_HYS | PAD_CTL_DSE1
#define SPEAKER_MUTE_GPIO IMX_GPIO_NR(5, 3)
static const iomux_v3_cfg_t speaker_pad = IMX8MQ_PAD_SPDIF_TX__GPIO5_IO3 |
					  MUX_PAD_CTRL(SPEAKER_PAD_MUTE_CTRL);

static void mute_speaker(void)
{
	imx_iomux_v3_setup_pad(speaker_pad);
	gpio_request(SPEAKER_MUTE_GPIO, "speaker_mute");
	gpio_direction_output(SPEAKER_MUTE_GPIO, 0);
}

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	init_uart_clk(0);

	board_early_init_f();

	timer_init();

	ret = spl_early_init();
	if (ret) {
		log_err("spl_early_init() failed: %d\n", ret);
		hang();
	}
	preloader_console_init();

	enable_tzc380();

	/* PMIC initialization */
	power_init_board();
	init_charger_bq25896();

	mute_speaker();

	/* DDR initialization */
	printf("Initializing DRAM\n");
	spl_dram_init();

	init_clk_usdhc(0);
	init_clk_usdhc(1);

	log_debug("Board init\n");
	board_init_r(NULL, 0);
}
