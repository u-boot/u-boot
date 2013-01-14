/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 * Jason Liu <r64343@freescale.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx5x_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/clock.h>
#include <asm/errno.h>
#include <asm/imx-common/mx5_video.h>
#include <netdev.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <asm/gpio.h>
#include <power/pmic.h>
#include <dialog_pmic.h>
#include <fsl_pmic.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>

#define MX53LOCO_LCD_POWER		IMX_GPIO_NR(3, 24)

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	u32 size1, size2;

	size1 = get_ram_size((void *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	size2 = get_ram_size((void *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE);

	gd->ram_size = size1 + size2;

	return 0;
}
void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
}

u32 get_board_rev(void)
{
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	struct fuse_bank *bank = &iim->bank[0];
	struct fuse_bank0_regs *fuse =
		(struct fuse_bank0_regs *)bank->fuse_regs;

	int rev = readl(&fuse->gp[6]);

	if (!i2c_probe(CONFIG_SYS_DIALOG_PMIC_I2C_ADDR))
		rev = 0;

	return (get_cpu_rev() & ~(0xF << 8)) | (rev & 0xF) << 8;
}

static void setup_iomux_uart(void)
{
	/* UART1 RXD */
	mxc_request_iomux(MX53_PIN_CSI0_D11, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D11,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PU |
				PAD_CTL_ODE_OPENDRAIN_ENABLE);
	mxc_iomux_set_input(MX53_UART1_IPP_UART_RXD_MUX_SELECT_INPUT, 0x1);

	/* UART1 TXD */
	mxc_request_iomux(MX53_PIN_CSI0_D10, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D10,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PU |
				PAD_CTL_ODE_OPENDRAIN_ENABLE);
}

#ifdef CONFIG_USB_EHCI_MX5
int board_ehci_hcd_init(int port)
{
	/* request VBUS power enable pin, GPIO7_8 */
	mxc_request_iomux(MX53_PIN_ATA_DA_2, IOMUX_CONFIG_ALT1);
	gpio_direction_output(IOMUX_TO_GPIO(MX53_PIN_ATA_DA_2), 1);
	return 0;
}
#endif

static void setup_iomux_fec(void)
{
	/*FEC_MDIO*/
	mxc_request_iomux(MX53_PIN_FEC_MDIO, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_MDIO,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_22K_PU | PAD_CTL_ODE_OPENDRAIN_ENABLE);
	mxc_iomux_set_input(MX53_FEC_FEC_MDI_SELECT_INPUT, 0x1);

	/*FEC_MDC*/
	mxc_request_iomux(MX53_PIN_FEC_MDC, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_MDC, PAD_CTL_DRV_HIGH);

	/* FEC RXD1 */
	mxc_request_iomux(MX53_PIN_FEC_RXD1, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_RXD1,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);

	/* FEC RXD0 */
	mxc_request_iomux(MX53_PIN_FEC_RXD0, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_RXD0,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);

	 /* FEC TXD1 */
	mxc_request_iomux(MX53_PIN_FEC_TXD1, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_TXD1, PAD_CTL_DRV_HIGH);

	/* FEC TXD0 */
	mxc_request_iomux(MX53_PIN_FEC_TXD0, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_TXD0, PAD_CTL_DRV_HIGH);

	/* FEC TX_EN */
	mxc_request_iomux(MX53_PIN_FEC_TX_EN, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_TX_EN, PAD_CTL_DRV_HIGH);

	/* FEC TX_CLK */
	mxc_request_iomux(MX53_PIN_FEC_REF_CLK, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_REF_CLK,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);

	/* FEC RX_ER */
	mxc_request_iomux(MX53_PIN_FEC_RX_ER, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_RX_ER,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);

	/* FEC CRS */
	mxc_request_iomux(MX53_PIN_FEC_CRS_DV, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_CRS_DV,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR},
	{MMC_SDHC3_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret;

	mxc_request_iomux(MX53_PIN_EIM_DA11, IOMUX_CONFIG_ALT1);
	gpio_direction_input(IMX_GPIO_NR(3, 11));
	mxc_request_iomux(MX53_PIN_EIM_DA13, IOMUX_CONFIG_ALT1);
	gpio_direction_input(IMX_GPIO_NR(3, 13));

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		ret = !gpio_get_value(IMX_GPIO_NR(3, 13));
	else
		ret = !gpio_get_value(IMX_GPIO_NR(3, 11));

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	u32 index;
	s32 status = 0;

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	esdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM; index++) {
		switch (index) {
		case 0:
			mxc_request_iomux(MX53_PIN_SD1_CMD, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD1_CLK, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD1_DATA0,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD1_DATA1,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD1_DATA2,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD1_DATA3,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_EIM_DA13,
						IOMUX_CONFIG_ALT1);

			mxc_iomux_set_pad(MX53_PIN_SD1_CMD,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PU);
			mxc_iomux_set_pad(MX53_PIN_SD1_CLK,
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_DRV_HIGH);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA0,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA1,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA2,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA3,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			break;
		case 1:
			mxc_request_iomux(MX53_PIN_ATA_RESET_B,
						IOMUX_CONFIG_ALT2);
			mxc_request_iomux(MX53_PIN_ATA_IORDY,
						IOMUX_CONFIG_ALT2);
			mxc_request_iomux(MX53_PIN_ATA_DATA8,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA9,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA10,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA11,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA0,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA1,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA2,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA3,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_EIM_DA11,
						IOMUX_CONFIG_ALT1);

			mxc_iomux_set_pad(MX53_PIN_ATA_RESET_B,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_IORDY,
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_DRV_HIGH);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA8,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA9,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA10,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA11,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA0,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA1,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA2,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA3,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);

			break;
		default:
			printf("Warning: you configured more ESDHC controller"
				"(%d) as supported by the board(2)\n",
				CONFIG_SYS_FSL_ESDHC_NUM);
			return status;
		}
		status |= fsl_esdhc_initialize(bis, &esdhc_cfg[index]);
	}

	return status;
}
#endif

static void setup_iomux_i2c(void)
{
	/* I2C1 SDA */
	mxc_request_iomux(MX53_PIN_CSI0_D8,
		IOMUX_CONFIG_ALT5 | IOMUX_CONFIG_SION);
	mxc_iomux_set_input(MX53_I2C1_IPP_SDA_IN_SELECT_INPUT,
		INPUT_CTL_PATH0);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D8,
		PAD_CTL_SRE_FAST | PAD_CTL_DRV_HIGH |
		PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE |
		PAD_CTL_PUE_PULL |
		PAD_CTL_ODE_OPENDRAIN_ENABLE);
	/* I2C1 SCL */
	mxc_request_iomux(MX53_PIN_CSI0_D9,
		IOMUX_CONFIG_ALT5 | IOMUX_CONFIG_SION);
	mxc_iomux_set_input(MX53_I2C1_IPP_SCL_IN_SELECT_INPUT,
		INPUT_CTL_PATH0);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D9,
		PAD_CTL_SRE_FAST | PAD_CTL_DRV_HIGH |
		PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE |
		PAD_CTL_PUE_PULL |
		PAD_CTL_ODE_OPENDRAIN_ENABLE);
}

static int power_init(void)
{
	unsigned int val;
	int ret;
	struct pmic *p;

	if (!i2c_probe(CONFIG_SYS_DIALOG_PMIC_I2C_ADDR)) {
		ret = pmic_dialog_init(I2C_PMIC);
		if (ret)
			return ret;

		p = pmic_get("DIALOG_PMIC");
		if (!p)
			return -ENODEV;

		/* Set VDDA to 1.25V */
		val = DA9052_BUCKCORE_BCOREEN | DA_BUCKCORE_VBCORE_1_250V;
		ret = pmic_reg_write(p, DA9053_BUCKCORE_REG, val);
		if (ret) {
			printf("Writing to BUCKCORE_REG failed: %d\n", ret);
			return ret;
		}

		pmic_reg_read(p, DA9053_SUPPLY_REG, &val);
		val |= DA9052_SUPPLY_VBCOREGO;
		ret = pmic_reg_write(p, DA9053_SUPPLY_REG, val);
		if (ret) {
			printf("Writing to SUPPLY_REG failed: %d\n", ret);
			return ret;
		}

		/* Set Vcc peripheral to 1.30V */
		ret = pmic_reg_write(p, DA9053_BUCKPRO_REG, 0x62);
		if (ret) {
			printf("Writing to BUCKPRO_REG failed: %d\n", ret);
			return ret;
		}

		ret = pmic_reg_write(p, DA9053_SUPPLY_REG, 0x62);
		if (ret) {
			printf("Writing to SUPPLY_REG failed: %d\n", ret);
			return ret;
		}

		return ret;
	}

	if (!i2c_probe(CONFIG_SYS_FSL_PMIC_I2C_ADDR)) {
		ret = pmic_init(I2C_PMIC);
		if (ret)
			return ret;

		p = pmic_get("FSL_PMIC");
		if (!p)
			return -ENODEV;

		/* Set VDDGP to 1.25V for 1GHz on SW1 */
		pmic_reg_read(p, REG_SW_0, &val);
		val = (val & ~SWx_VOLT_MASK_MC34708) | SWx_1_250V_MC34708;
		ret = pmic_reg_write(p, REG_SW_0, val);
		if (ret) {
			printf("Writing to REG_SW_0 failed: %d\n", ret);
			return ret;
		}

		/* Set VCC as 1.30V on SW2 */
		pmic_reg_read(p, REG_SW_1, &val);
		val = (val & ~SWx_VOLT_MASK_MC34708) | SWx_1_300V_MC34708;
		ret = pmic_reg_write(p, REG_SW_1, val);
		if (ret) {
			printf("Writing to REG_SW_1 failed: %d\n", ret);
			return ret;
		}

		/* Set global reset timer to 4s */
		pmic_reg_read(p, REG_POWER_CTL2, &val);
		val = (val & ~TIMER_MASK_MC34708) | TIMER_4S_MC34708;
		ret = pmic_reg_write(p, REG_POWER_CTL2, val);
		if (ret) {
			printf("Writing to REG_POWER_CTL2 failed: %d\n", ret);
			return ret;
		}

		/* Set VUSBSEL and VUSBEN for USB PHY supply*/
		pmic_reg_read(p, REG_MODE_0, &val);
		val |= (VUSBSEL_MC34708 | VUSBEN_MC34708);
		ret = pmic_reg_write(p, REG_MODE_0, val);
		if (ret) {
			printf("Writing to REG_MODE_0 failed: %d\n", ret);
			return ret;
		}

		/* Set SWBST to 5V in auto mode */
		val = SWBST_AUTO;
		ret = pmic_reg_write(p, SWBST_CTRL, val);
		if (ret) {
			printf("Writing to SWBST_CTRL failed: %d\n", ret);
			return ret;
		}

		return ret;
	}

	return -1;
}

static void clock_1GHz(void)
{
	int ret;
	u32 ref_clk = MXC_HCLK;
	/*
	 * After increasing voltage to 1.25V, we can switch
	 * CPU clock to 1GHz and DDR to 400MHz safely
	 */
	ret = mxc_set_clock(ref_clk, 1000, MXC_ARM_CLK);
	if (ret)
		printf("CPU:   Switch CPU clock to 1GHZ failed\n");

	ret = mxc_set_clock(ref_clk, 400, MXC_PERIPH_CLK);
	ret |= mxc_set_clock(ref_clk, 400, MXC_DDR_CLK);
	if (ret)
		printf("CPU:   Switch DDR clock to 400MHz failed\n");
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_fec();
	setup_iomux_lcd();

	return 0;
}

int print_cpuinfo(void)
{
	u32 cpurev;

	cpurev = get_cpu_rev();
	printf("CPU:   Freescale i.MX%x family rev%d.%d at %d MHz\n",
		(cpurev & 0xFF000) >> 12,
		(cpurev & 0x000F0) >> 4,
		(cpurev & 0x0000F) >> 0,
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("Reset cause: %s\n", get_reset_cause());
	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	mxc_set_sata_internal_clock();
	setup_iomux_i2c();

	return 0;
}

int board_late_init(void)
{
	if (!power_init())
		clock_1GHz();
	print_cpuinfo();

	return 0;
}

int checkboard(void)
{
	puts("Board: MX53 LOCO\n");

	return 0;
}
