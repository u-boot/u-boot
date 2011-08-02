/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
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
#include <asm/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx35_pins.h>
#include <asm/arch/iomux.h>
#include <i2c.h>
#include <fsl_pmic.h>
#include <mc9sdz60.h>
#include <mc13892.h>
#include <linux/types.h>
#include <mxc_gpio.h>
#include <asm/arch/sys_proto.h>
#include <netdev.h>

#ifndef BOARD_LATE_INIT
#error "BOARD_LATE_INIT must be set for this board"
#endif

#ifndef CONFIG_BOARD_EARLY_INIT_F
#error "CONFIG_BOARD_EARLY_INIT_F must be set for this board"
#endif

#define mdelay(n) ({unsigned long msec = (n); while (msec--) udelay(1000); })

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

static void setup_iomux_i2c(void)
{
	int pad;

	/* setup pins for I2C1 */
	mxc_request_iomux(MX35_PIN_I2C1_CLK, MUX_CONFIG_SION);
	mxc_request_iomux(MX35_PIN_I2C1_DAT, MUX_CONFIG_SION);

	pad = (PAD_CTL_HYS_SCHMITZ | PAD_CTL_PKE_ENABLE \
			| PAD_CTL_PUE_PUD | PAD_CTL_ODE_OpenDrain);

	mxc_iomux_set_pad(MX35_PIN_I2C1_CLK, pad);
	mxc_iomux_set_pad(MX35_PIN_I2C1_DAT, pad);
}


static void setup_iomux_spi(void)
{
	mxc_request_iomux(MX35_PIN_CSPI1_MOSI, MUX_CONFIG_SION);
	mxc_request_iomux(MX35_PIN_CSPI1_MISO, MUX_CONFIG_SION);
	mxc_request_iomux(MX35_PIN_CSPI1_SS0, MUX_CONFIG_SION);
	mxc_request_iomux(MX35_PIN_CSPI1_SS1, MUX_CONFIG_SION);
	mxc_request_iomux(MX35_PIN_CSPI1_SCLK, MUX_CONFIG_SION);
}

static void setup_iomux_fec(void)
{
	int pad;

	/* setup pins for FEC */
	mxc_request_iomux(MX35_PIN_FEC_TX_CLK, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RX_CLK, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RX_DV, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_COL, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA0, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA0, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TX_EN, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_MDC, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_MDIO, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TX_ERR, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RX_ERR, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_CRS, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA1, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA1, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA2, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA2, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA3, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA3, MUX_CONFIG_FUNC);

	pad = (PAD_CTL_DRV_3_3V | PAD_CTL_PUE_PUD | PAD_CTL_ODE_CMOS | \
			PAD_CTL_DRV_NORMAL | PAD_CTL_SRE_SLOW);

	mxc_iomux_set_pad(MX35_PIN_FEC_TX_CLK, pad | PAD_CTL_HYS_SCHMITZ | \
			PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RX_CLK, pad | PAD_CTL_HYS_SCHMITZ | \
			PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RX_DV, pad | PAD_CTL_HYS_SCHMITZ | \
			 PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_COL, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RDATA0, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_TDATA0, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_TX_EN, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_MDC, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_MDIO, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_22K_PU);
	mxc_iomux_set_pad(MX35_PIN_FEC_TX_ERR, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RX_ERR, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_CRS, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RDATA1, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_TDATA1, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RDATA2, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_TDATA2, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RDATA3, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_TDATA3, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
}

int board_early_init_f(void)
{
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;

	/* enable clocks */
	writel(readl(&ccm->cgr0) |
		MXC_CCM_CGR0_EMI_MASK |
		MXC_CCM_CGR0_EDI0_MASK |
		MXC_CCM_CGR0_EPIT1_MASK,
		&ccm->cgr0);

	writel(readl(&ccm->cgr1) |
		MXC_CCM_CGR1_FEC_MASK |
		MXC_CCM_CGR1_GPIO1_MASK |
		MXC_CCM_CGR1_GPIO2_MASK |
		MXC_CCM_CGR1_GPIO3_MASK |
		MXC_CCM_CGR1_I2C1_MASK |
		MXC_CCM_CGR1_I2C2_MASK |
		MXC_CCM_CGR1_IPU_MASK,
		&ccm->cgr1);

	/* Setup NAND */
	__raw_writel(readl(&ccm->rcsr) | MXC_CCM_RCSR_NFC_FMS, &ccm->rcsr);

	setup_iomux_i2c();
	setup_iomux_fec();
	setup_iomux_spi();

	return 0;
}

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_MX35_3DS;	/* board id for linux */
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

static inline int pmic_detect(void)
{
	int id;

	id = pmic_reg_read(REG_IDENTIFICATION);

	id = (id >> 6) & 0x7;
	if (id == 0x7)
		return 1;
	return 0;
}

u32 get_board_rev(void)
{
	int rev;

	rev = pmic_detect();

	return (get_cpu_rev() & ~(0xF << 8)) | (rev & 0xF) << 8;
}

int board_late_init(void)
{
	u8 val;
	u32 pmic_val;

	if (pmic_detect()) {
		mxc_request_iomux(MX35_PIN_WATCHDOG_RST, MUX_CONFIG_SION |
					MUX_CONFIG_ALT1);

		pmic_val = pmic_reg_read(REG_SETTING_0);
		pmic_reg_write(REG_SETTING_0, pmic_val | VO_1_30V | VO_1_50V);
		pmic_val = pmic_reg_read(REG_MODE_0);
		pmic_reg_write(REG_MODE_0, pmic_val | VGEN3EN);

		mxc_request_iomux(MX35_PIN_COMPARE, MUX_CONFIG_GPIO);
		mxc_iomux_set_input(MUX_IN_GPIO1_IN_5, INPUT_CTL_PATH0);

		mxc_gpio_direction(37, MXC_GPIO_DIRECTION_OUT);
		mxc_gpio_set(37, 1);
	}

	val = mc9sdz60_reg_read(MC9SDZ60_REG_GPIO_1) | 0x04;
	mc9sdz60_reg_write(MC9SDZ60_REG_GPIO_1, val);
	mdelay(200);

	val = mc9sdz60_reg_read(MC9SDZ60_REG_RESET_1) & 0x7F;
	mc9sdz60_reg_write(MC9SDZ60_REG_RESET_1, val);
	mdelay(200);

	val |= 0x80;
	mc9sdz60_reg_write(MC9SDZ60_REG_RESET_1, val);

	return 0;
}

int checkboard(void)
{
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;
	u32 cpu_rev = get_cpu_rev();

	/*
	 * Be sure that I2C is initialized to check
	 * the board revision
	 */
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	/* Print board revision */
	printf("Board: MX35 PDK %d.0 ", ((get_board_rev() >> 8) + 1) & 0x0F);

	/* Print CPU revision */
	printf("i.MX35 %d.%d [", (cpu_rev & 0xF0) >> 4, cpu_rev & 0x0F);

	switch (readl(&ccm->rcsr) & 0x0F) {
	case 0x0000:
		puts("POR");
		break;
	case 0x0002:
		puts("JTAG");
		break;
	case 0x0004:
		puts("RST");
		break;
	case 0x0008:
		puts("WDT");
		break;
	default:
		puts("unknown");
	}
	puts("]\n");

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = -ENODEV;
#if defined(CONFIG_SMC911X)
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif

	cpu_eth_init(bis);

	return rc;
}
