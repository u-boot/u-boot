/*
 * Copyright (C) 2011 Samsung Electronics
 * Heungjun Kim <riverful.kim@samsung.com>
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

#ifndef __ASM_ARM_ARCH_POWER_H_
#define __ASM_ARM_ARCH_POWER_H_

#ifndef __ASSEMBLY__
struct exynos4_power {
	unsigned int	om_stat;
	unsigned char	res1[0x8];
	unsigned int	rtc_clko_sel;
	unsigned int	gnss_rtc_out_ctrl;
	unsigned char	res2[0x1ec];
	unsigned int	system_power_down_ctrl;
	unsigned char	res3[0x1];
	unsigned int	system_power_down_option;
	unsigned char	res4[0x1f4];
	unsigned int	swreset;
	unsigned int	rst_stat;
	unsigned char	res5[0x1f8];
	unsigned int	wakeup_stat;
	unsigned int	eint_wakeup_mask;
	unsigned int	wakeup_mask;
	unsigned char	res6[0xf4];
	unsigned int	hdmi_phy_control;
	unsigned int	usbdevice_phy_control;
	unsigned int	usbhost_phy_control;
	unsigned int	dac_phy_control;
	unsigned int	mipi_phy0_control;
	unsigned int	mipi_phy1_control;
	unsigned int	adc_phy_control;
	unsigned int	pcie_phy_control;
	unsigned int	sata_phy_control;
	unsigned char	res7[0xdc];
	unsigned int	inform0;
	unsigned int	inform1;
	unsigned int	inform2;
	unsigned int	inform3;
	unsigned int	inform4;
	unsigned int	inform5;
	unsigned int	inform6;
	unsigned int	inform7;
	unsigned char	res8[0x1e0];
	unsigned int	pmu_debug;
	unsigned char	res9[0x5fc];
	unsigned int	arm_core0_sys_pwr_reg;
	unsigned char	res10[0xc];
	unsigned int	arm_core1_sys_pwr_reg;
	unsigned char	res11[0x6c];
	unsigned int	arm_common_sys_pwr_reg;
	unsigned char	res12[0x3c];
	unsigned int	arm_cpu_l2_0_sys_pwr_reg;
	unsigned int	arm_cpu_l2_1_sys_pwr_reg;
	unsigned char	res13[0x38];
	unsigned int	cmu_aclkstop_sys_pwr_reg;
	unsigned int	cmu_sclkstop_sys_pwr_reg;
	unsigned char	res14[0x4];
	unsigned int	cmu_reset_sys_pwr_reg;
	unsigned char	res15[0x10];
	unsigned int	apll_sysclk_sys_pwr_reg;
	unsigned int	mpll_sysclk_sys_pwr_reg;
	unsigned int	vpll_sysclk_sys_pwr_reg;
	unsigned int	epll_sysclk_sys_pwr_reg;
	unsigned char	res16[0x8];
	unsigned int	cmu_clkstop_gps_alive_sys_pwr_reg;
	unsigned int	cmu_reset_gps_alive_sys_pwr_reg;
	unsigned int	cmu_clkstop_cam_sys_pwr_reg;
	unsigned int	cmu_clkstop_tv_sys_pwr_reg;
	unsigned int	cmu_clkstop_mfc_sys_pwr_reg;
	unsigned int	cmu_clkstop_g3d_sys_pwr_reg;
	unsigned int	cmu_clkstop_lcd0_sys_pwr_reg;
	unsigned int	cmu_clkstop_lcd1_sys_pwr_reg;
	unsigned int	cmu_clkstop_maudio_sys_pwr_reg;
	unsigned int	cmu_clkstop_gps_sys_pwr_reg;
	unsigned int	cmu_reset_cam_sys_pwr_reg;
	unsigned int	cmu_reset_tv_sys_pwr_reg;
	unsigned int	cmu_reset_mfc_sys_pwr_reg;
	unsigned int	cmu_reset_g3d_sys_pwr_reg;
	unsigned int	cmu_reset_lcd0_sys_pwr_reg;
	unsigned int	cmu_reset_lcd1_sys_pwr_reg;
	unsigned int	cmu_reset_maudio_sys_pwr_reg;
	unsigned int	cmu_reset_gps_sys_pwr_reg;
	unsigned int	top_bus_sys_pwr_reg;
	unsigned int	top_retention_sys_pwr_reg;
	unsigned int	top_pwr_sys_pwr_reg;
	unsigned char	res17[0x1c];
	unsigned int	logic_reset_sys_pwr_reg;
	unsigned char	res18[0x14];
	unsigned int	onenandxl_mem_sys_pwr_reg;
	unsigned int	modemif_mem_sys_pwr_reg;
	unsigned char	res19[0x4];
	unsigned int	usbdevice_mem_sys_pwr_reg;
	unsigned int	sdmmc_mem_sys_pwr_reg;
	unsigned int	cssys_mem_sys_pwr_reg;
	unsigned int	secss_mem_sys_pwr_reg;
	unsigned char	res20[0x4];
	unsigned int	pcie_mem_sys_pwr_reg;
	unsigned int	sata_mem_sys_pwr_reg;
	unsigned char	res21[0x18];
	unsigned int	pad_retention_dram_sys_pwr_reg;
	unsigned int	pad_retention_maudio_sys_pwr_reg;
	unsigned char	res22[0x18];
	unsigned int	pad_retention_gpio_sys_pwr_reg;
	unsigned int	pad_retention_uart_sys_pwr_reg;
	unsigned int	pad_retention_mmca_sys_pwr_reg;
	unsigned int	pad_retention_mmcb_sys_pwr_reg;
	unsigned int	pad_retention_ebia_sys_pwr_reg;
	unsigned int	pad_retention_ebib_sys_pwr_reg;
	unsigned char	res23[0x8];
	unsigned int	pad_isolation_sys_pwr_reg;
	unsigned char	res24[0x1c];
	unsigned int	pad_alv_sel_sys_pwr_reg;
	unsigned char	res25[0x1c];
	unsigned int	xusbxti_sys_pwr_reg;
	unsigned int	xxti_sys_pwr_reg;
	unsigned char	res26[0x38];
	unsigned int	ext_regulator_sys_pwr_reg;
	unsigned char	res27[0x3c];
	unsigned int	gpio_mode_sys_pwr_reg;
	unsigned char	res28[0x3c];
	unsigned int	gpio_mode_maudio_sys_pwr_reg;
	unsigned char	res29[0x3c];
	unsigned int	cam_sys_pwr_reg;
	unsigned int	tv_sys_pwr_reg;
	unsigned int	mfc_sys_pwr_reg;
	unsigned int	g3d_sys_pwr_reg;
	unsigned int	lcd0_sys_pwr_reg;
	unsigned int	lcd1_sys_pwr_reg;
	unsigned int	maudio_sys_pwr_reg;
	unsigned int	gps_sys_pwr_reg;
	unsigned int	gps_alive_sys_pwr_reg;
	unsigned char	res30[0xc5c];
	unsigned int	arm_core0_configuration;
	unsigned int	arm_core0_status;
	unsigned int	arm_core0_option;
	unsigned char	res31[0x74];
	unsigned int	arm_core1_configuration;
	unsigned int	arm_core1_status;
	unsigned int	arm_core1_option;
	unsigned char	res32[0x37c];
	unsigned int	arm_common_option;
	unsigned char	res33[0x1f4];
	unsigned int	arm_cpu_l2_0_configuration;
	unsigned int	arm_cpu_l2_0_status;
	unsigned char	res34[0x18];
	unsigned int	arm_cpu_l2_1_configuration;
	unsigned int	arm_cpu_l2_1_status;
	unsigned char	res35[0xa00];
	unsigned int	pad_retention_maudio_option;
	unsigned char	res36[0xdc];
	unsigned int	pad_retention_gpio_option;
	unsigned char	res37[0x1c];
	unsigned int	pad_retention_uart_option;
	unsigned char	res38[0x1c];
	unsigned int	pad_retention_mmca_option;
	unsigned char	res39[0x1c];
	unsigned int	pad_retention_mmcb_option;
	unsigned char	res40[0x1c];
	unsigned int	pad_retention_ebia_option;
	unsigned char	res41[0x1c];
	unsigned int	pad_retention_ebib_option;
	unsigned char	res42[0x160];
	unsigned int	ps_hold_control;
	unsigned char	res43[0xf0];
	unsigned int	xusbxti_configuration;
	unsigned int	xusbxti_status;
	unsigned char	res44[0x14];
	unsigned int	xusbxti_duration;
	unsigned int	xxti_configuration;
	unsigned int	xxti_status;
	unsigned char	res45[0x14];
	unsigned int	xxti_duration;
	unsigned char	res46[0x1dc];
	unsigned int	ext_regulator_duration;
	unsigned char	res47[0x5e0];
	unsigned int	cam_configuration;
	unsigned int	cam_status;
	unsigned int	cam_option;
	unsigned char	res48[0x14];
	unsigned int	tv_configuration;
	unsigned int	tv_status;
	unsigned int	tv_option;
	unsigned char	res49[0x14];
	unsigned int	mfc_configuration;
	unsigned int	mfc_status;
	unsigned int	mfc_option;
	unsigned char	res50[0x14];
	unsigned int	g3d_configuration;
	unsigned int	g3d_status;
	unsigned int	g3d_option;
	unsigned char	res51[0x14];
	unsigned int	lcd0_configuration;
	unsigned int	lcd0_status;
	unsigned int	lcd0_option;
	unsigned char	res52[0x14];
	unsigned int	lcd1_configuration;
	unsigned int	lcd1_status;
	unsigned int	lcd1_option;
	unsigned char	res53[0x34];
	unsigned int	gps_configuration;
	unsigned int	gps_status;
	unsigned int	gps_option;
	unsigned char	res54[0x14];
	unsigned int	gps_alive_configuration;
	unsigned int	gps_alive_status;
	unsigned int	gps_alive_option;
};
#endif	/* __ASSEMBLY__ */

#endif
