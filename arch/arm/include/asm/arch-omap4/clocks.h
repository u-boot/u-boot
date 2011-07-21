/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _CLOCKS_OMAP4_H_
#define _CLOCKS_OMAP4_H_
#include <common.h>

/*
 * Assuming a maximum of 1.5 GHz ARM speed and a minimum of 2 cycles per
 * loop, allow for a minimum of 2 ms wait (in reality the wait will be
 * much more than that)
 */
#define LDELAY		1000000

#define CM_CLKMODE_DPLL_CORE		0x4A004120
#define CM_CLKMODE_DPLL_PER		0x4A008140
#define CM_CLKMODE_DPLL_MPU		0x4A004160
#define CM_CLKSEL_CORE			0x4A004100

struct omap4_prcm_regs {
	/* cm1.ckgen */
	u32 cm_clksel_core;
	u32 pad001[1];
	u32 cm_clksel_abe;
	u32 pad002[1];
	u32 cm_dll_ctrl;
	u32 pad003[3];
	u32 cm_clkmode_dpll_core;
	u32 cm_idlest_dpll_core;
	u32 cm_autoidle_dpll_core;
	u32 cm_clksel_dpll_core;
	u32 cm_div_m2_dpll_core;
	u32 cm_div_m3_dpll_core;
	u32 cm_div_m4_dpll_core;
	u32 cm_div_m5_dpll_core;
	u32 cm_div_m6_dpll_core;
	u32 cm_div_m7_dpll_core;
	u32 cm_ssc_deltamstep_dpll_core;
	u32 cm_ssc_modfreqdiv_dpll_core;
	u32 cm_emu_override_dpll_core;
	u32 pad004[3];
	u32 cm_clkmode_dpll_mpu;
	u32 cm_idlest_dpll_mpu;
	u32 cm_autoidle_dpll_mpu;
	u32 cm_clksel_dpll_mpu;
	u32 cm_div_m2_dpll_mpu;
	u32 pad005[5];
	u32 cm_ssc_deltamstep_dpll_mpu;
	u32 cm_ssc_modfreqdiv_dpll_mpu;
	u32 pad006[3];
	u32 cm_bypclk_dpll_mpu;
	u32 cm_clkmode_dpll_iva;
	u32 cm_idlest_dpll_iva;
	u32 cm_autoidle_dpll_iva;
	u32 cm_clksel_dpll_iva;
	u32 pad007[2];
	u32 cm_div_m4_dpll_iva;
	u32 cm_div_m5_dpll_iva;
	u32 pad008[2];
	u32 cm_ssc_deltamstep_dpll_iva;
	u32 cm_ssc_modfreqdiv_dpll_iva;
	u32 pad009[3];
	u32 cm_bypclk_dpll_iva;
	u32 cm_clkmode_dpll_abe;
	u32 cm_idlest_dpll_abe;
	u32 cm_autoidle_dpll_abe;
	u32 cm_clksel_dpll_abe;
	u32 cm_div_m2_dpll_abe;
	u32 cm_div_m3_dpll_abe;
	u32 pad010[4];
	u32 cm_ssc_deltamstep_dpll_abe;
	u32 cm_ssc_modfreqdiv_dpll_abe;
	u32 pad011[4];
	u32 cm_clkmode_dpll_ddrphy;
	u32 cm_idlest_dpll_ddrphy;
	u32 cm_autoidle_dpll_ddrphy;
	u32 cm_clksel_dpll_ddrphy;
	u32 cm_div_m2_dpll_ddrphy;
	u32 pad012[1];
	u32 cm_div_m4_dpll_ddrphy;
	u32 cm_div_m5_dpll_ddrphy;
	u32 cm_div_m6_dpll_ddrphy;
	u32 pad013[1];
	u32 cm_ssc_deltamstep_dpll_ddrphy;
	u32 pad014[5];
	u32 cm_shadow_freq_config1;

	/* cm1.dsp */
	u32 pad015[103];
	u32 cm_dsp_clkstctrl;
	u32 pad016[7];
	u32 cm_dsp_dsp_clkctrl;

	/* cm1.abe */
	u32 pad017[55];
	u32 cm1_abe_clkstctrl;
	u32 pad018[7];
	u32 cm1_abe_l4abe_clkctrl;
	u32 pad019[1];
	u32 cm1_abe_aess_clkctrl;
	u32 pad020[1];
	u32 cm1_abe_pdm_clkctrl;
	u32 pad021[1];
	u32 cm1_abe_dmic_clkctrl;
	u32 pad022[1];
	u32 cm1_abe_mcasp_clkctrl;
	u32 pad023[1];
	u32 cm1_abe_mcbsp1_clkctrl;
	u32 pad024[1];
	u32 cm1_abe_mcbsp2_clkctrl;
	u32 pad025[1];
	u32 cm1_abe_mcbsp3_clkctrl;
	u32 pad026[1];
	u32 cm1_abe_slimbus_clkctrl;
	u32 pad027[1];
	u32 cm1_abe_timer5_clkctrl;
	u32 pad028[1];
	u32 cm1_abe_timer6_clkctrl;
	u32 pad029[1];
	u32 cm1_abe_timer7_clkctrl;
	u32 pad030[1];
	u32 cm1_abe_timer8_clkctrl;
	u32 pad031[1];
	u32 cm1_abe_wdt3_clkctrl;

	/* cm2.ckgen */
	u32 pad032[3805];
	u32 cm_clksel_mpu_m3_iss_root;
	u32 cm_clksel_usb_60mhz;
	u32 cm_scale_fclk;
	u32 pad033[1];
	u32 cm_core_dvfs_perf1;
	u32 cm_core_dvfs_perf2;
	u32 cm_core_dvfs_perf3;
	u32 cm_core_dvfs_perf4;
	u32 pad034[1];
	u32 cm_core_dvfs_current;
	u32 cm_iva_dvfs_perf_tesla;
	u32 cm_iva_dvfs_perf_ivahd;
	u32 cm_iva_dvfs_perf_abe;
	u32 pad035[1];
	u32 cm_iva_dvfs_current;
	u32 pad036[1];
	u32 cm_clkmode_dpll_per;
	u32 cm_idlest_dpll_per;
	u32 cm_autoidle_dpll_per;
	u32 cm_clksel_dpll_per;
	u32 cm_div_m2_dpll_per;
	u32 cm_div_m3_dpll_per;
	u32 cm_div_m4_dpll_per;
	u32 cm_div_m5_dpll_per;
	u32 cm_div_m6_dpll_per;
	u32 cm_div_m7_dpll_per;
	u32 cm_ssc_deltamstep_dpll_per;
	u32 cm_ssc_modfreqdiv_dpll_per;
	u32 cm_emu_override_dpll_per;
	u32 pad037[3];
	u32 cm_clkmode_dpll_usb;
	u32 cm_idlest_dpll_usb;
	u32 cm_autoidle_dpll_usb;
	u32 cm_clksel_dpll_usb;
	u32 cm_div_m2_dpll_usb;
	u32 pad038[5];
	u32 cm_ssc_deltamstep_dpll_usb;
	u32 cm_ssc_modfreqdiv_dpll_usb;
	u32 pad039[1];
	u32 cm_clkdcoldo_dpll_usb;
	u32 pad040[2];
	u32 cm_clkmode_dpll_unipro;
	u32 cm_idlest_dpll_unipro;
	u32 cm_autoidle_dpll_unipro;
	u32 cm_clksel_dpll_unipro;
	u32 cm_div_m2_dpll_unipro;
	u32 pad041[5];
	u32 cm_ssc_deltamstep_dpll_unipro;
	u32 cm_ssc_modfreqdiv_dpll_unipro;

	/* cm2.core */
	u32 pad0411[324];
	u32 cm_l3_1_clkstctrl;
	u32 pad042[1];
	u32 cm_l3_1_dynamicdep;
	u32 pad043[5];
	u32 cm_l3_1_l3_1_clkctrl;
	u32 pad044[55];
	u32 cm_l3_2_clkstctrl;
	u32 pad045[1];
	u32 cm_l3_2_dynamicdep;
	u32 pad046[5];
	u32 cm_l3_2_l3_2_clkctrl;
	u32 pad047[1];
	u32 cm_l3_2_gpmc_clkctrl;
	u32 pad048[1];
	u32 cm_l3_2_ocmc_ram_clkctrl;
	u32 pad049[51];
	u32 cm_mpu_m3_clkstctrl;
	u32 cm_mpu_m3_staticdep;
	u32 cm_mpu_m3_dynamicdep;
	u32 pad050[5];
	u32 cm_mpu_m3_mpu_m3_clkctrl;
	u32 pad051[55];
	u32 cm_sdma_clkstctrl;
	u32 cm_sdma_staticdep;
	u32 cm_sdma_dynamicdep;
	u32 pad052[5];
	u32 cm_sdma_sdma_clkctrl;
	u32 pad053[55];
	u32 cm_memif_clkstctrl;
	u32 pad054[7];
	u32 cm_memif_dmm_clkctrl;
	u32 pad055[1];
	u32 cm_memif_emif_fw_clkctrl;
	u32 pad056[1];
	u32 cm_memif_emif_1_clkctrl;
	u32 pad057[1];
	u32 cm_memif_emif_2_clkctrl;
	u32 pad058[1];
	u32 cm_memif_dll_clkctrl;
	u32 pad059[3];
	u32 cm_memif_emif_h1_clkctrl;
	u32 pad060[1];
	u32 cm_memif_emif_h2_clkctrl;
	u32 pad061[1];
	u32 cm_memif_dll_h_clkctrl;
	u32 pad062[39];
	u32 cm_c2c_clkstctrl;
	u32 cm_c2c_staticdep;
	u32 cm_c2c_dynamicdep;
	u32 pad063[5];
	u32 cm_c2c_sad2d_clkctrl;
	u32 pad064[1];
	u32 cm_c2c_modem_icr_clkctrl;
	u32 pad065[1];
	u32 cm_c2c_sad2d_fw_clkctrl;
	u32 pad066[51];
	u32 cm_l4cfg_clkstctrl;
	u32 pad067[1];
	u32 cm_l4cfg_dynamicdep;
	u32 pad068[5];
	u32 cm_l4cfg_l4_cfg_clkctrl;
	u32 pad069[1];
	u32 cm_l4cfg_hw_sem_clkctrl;
	u32 pad070[1];
	u32 cm_l4cfg_mailbox_clkctrl;
	u32 pad071[1];
	u32 cm_l4cfg_sar_rom_clkctrl;
	u32 pad072[49];
	u32 cm_l3instr_clkstctrl;
	u32 pad073[7];
	u32 cm_l3instr_l3_3_clkctrl;
	u32 pad074[1];
	u32 cm_l3instr_l3_instr_clkctrl;
	u32 pad075[5];
	u32 cm_l3instr_intrconn_wp1_clkctrl;


	/* cm2.ivahd */
	u32 pad076[47];
	u32 cm_ivahd_clkstctrl;
	u32 pad077[7];
	u32 cm_ivahd_ivahd_clkctrl;
	u32 pad078[1];
	u32 cm_ivahd_sl2_clkctrl;

	/* cm2.cam */
	u32 pad079[53];
	u32 cm_cam_clkstctrl;
	u32 pad080[7];
	u32 cm_cam_iss_clkctrl;
	u32 pad081[1];
	u32 cm_cam_fdif_clkctrl;

	/* cm2.dss */
	u32 pad082[53];
	u32 cm_dss_clkstctrl;
	u32 pad083[7];
	u32 cm_dss_dss_clkctrl;

	/* cm2.sgx */
	u32 pad084[55];
	u32 cm_sgx_clkstctrl;
	u32 pad085[7];
	u32 cm_sgx_sgx_clkctrl;

	/* cm2.l3init */
	u32 pad086[55];
	u32 cm_l3init_clkstctrl;

	/* cm2.l3init */
	u32 pad087[9];
	u32 cm_l3init_hsmmc1_clkctrl;
	u32 pad088[1];
	u32 cm_l3init_hsmmc2_clkctrl;
	u32 pad089[1];
	u32 cm_l3init_hsi_clkctrl;
	u32 pad090[7];
	u32 cm_l3init_hsusbhost_clkctrl;
	u32 pad091[1];
	u32 cm_l3init_hsusbotg_clkctrl;
	u32 pad092[1];
	u32 cm_l3init_hsusbtll_clkctrl;
	u32 pad093[3];
	u32 cm_l3init_p1500_clkctrl;
	u32 pad094[21];
	u32 cm_l3init_fsusb_clkctrl;
	u32 pad095[3];
	u32 cm_l3init_usbphy_clkctrl;

	/* cm2.l4per */
	u32 pad096[7];
	u32 cm_l4per_clkstctrl;
	u32 pad097[1];
	u32 cm_l4per_dynamicdep;
	u32 pad098[5];
	u32 cm_l4per_adc_clkctrl;
	u32 pad100[1];
	u32 cm_l4per_gptimer10_clkctrl;
	u32 pad101[1];
	u32 cm_l4per_gptimer11_clkctrl;
	u32 pad102[1];
	u32 cm_l4per_gptimer2_clkctrl;
	u32 pad103[1];
	u32 cm_l4per_gptimer3_clkctrl;
	u32 pad104[1];
	u32 cm_l4per_gptimer4_clkctrl;
	u32 pad105[1];
	u32 cm_l4per_gptimer9_clkctrl;
	u32 pad106[1];
	u32 cm_l4per_elm_clkctrl;
	u32 pad107[1];
	u32 cm_l4per_gpio2_clkctrl;
	u32 pad108[1];
	u32 cm_l4per_gpio3_clkctrl;
	u32 pad109[1];
	u32 cm_l4per_gpio4_clkctrl;
	u32 pad110[1];
	u32 cm_l4per_gpio5_clkctrl;
	u32 pad111[1];
	u32 cm_l4per_gpio6_clkctrl;
	u32 pad112[1];
	u32 cm_l4per_hdq1w_clkctrl;
	u32 pad113[1];
	u32 cm_l4per_hecc1_clkctrl;
	u32 pad114[1];
	u32 cm_l4per_hecc2_clkctrl;
	u32 pad115[1];
	u32 cm_l4per_i2c1_clkctrl;
	u32 pad116[1];
	u32 cm_l4per_i2c2_clkctrl;
	u32 pad117[1];
	u32 cm_l4per_i2c3_clkctrl;
	u32 pad118[1];
	u32 cm_l4per_i2c4_clkctrl;
	u32 pad119[1];
	u32 cm_l4per_l4per_clkctrl;
	u32 pad1191[3];
	u32 cm_l4per_mcasp2_clkctrl;
	u32 pad120[1];
	u32 cm_l4per_mcasp3_clkctrl;
	u32 pad121[1];
	u32 cm_l4per_mcbsp4_clkctrl;
	u32 pad122[1];
	u32 cm_l4per_mgate_clkctrl;
	u32 pad123[1];
	u32 cm_l4per_mcspi1_clkctrl;
	u32 pad124[1];
	u32 cm_l4per_mcspi2_clkctrl;
	u32 pad125[1];
	u32 cm_l4per_mcspi3_clkctrl;
	u32 pad126[1];
	u32 cm_l4per_mcspi4_clkctrl;
	u32 pad127[5];
	u32 cm_l4per_mmcsd3_clkctrl;
	u32 pad128[1];
	u32 cm_l4per_mmcsd4_clkctrl;
	u32 pad129[1];
	u32 cm_l4per_msprohg_clkctrl;
	u32 pad130[1];
	u32 cm_l4per_slimbus2_clkctrl;
	u32 pad131[1];
	u32 cm_l4per_uart1_clkctrl;
	u32 pad132[1];
	u32 cm_l4per_uart2_clkctrl;
	u32 pad133[1];
	u32 cm_l4per_uart3_clkctrl;
	u32 pad134[1];
	u32 cm_l4per_uart4_clkctrl;
	u32 pad135[1];
	u32 cm_l4per_mmcsd5_clkctrl;
	u32 pad136[1];
	u32 cm_l4per_i2c5_clkctrl;
	u32 pad137[5];
	u32 cm_l4sec_clkstctrl;
	u32 cm_l4sec_staticdep;
	u32 cm_l4sec_dynamicdep;
	u32 pad138[5];
	u32 cm_l4sec_aes1_clkctrl;
	u32 pad139[1];
	u32 cm_l4sec_aes2_clkctrl;
	u32 pad140[1];
	u32 cm_l4sec_des3des_clkctrl;
	u32 pad141[1];
	u32 cm_l4sec_pkaeip29_clkctrl;
	u32 pad142[1];
	u32 cm_l4sec_rng_clkctrl;
	u32 pad143[1];
	u32 cm_l4sec_sha2md51_clkctrl;
	u32 pad144[3];
	u32 cm_l4sec_cryptodma_clkctrl;
	u32 pad145[776841];

	/* l4 wkup regs */
	u32 pad201[6211];
	u32 cm_abe_pll_ref_clksel;
	u32 cm_sys_clksel;
	u32 pad202[1467];
	u32 cm_wkup_clkstctrl;
	u32 pad203[7];
	u32 cm_wkup_l4wkup_clkctrl;
	u32 pad204;
	u32 cm_wkup_wdtimer1_clkctrl;
	u32 pad205;
	u32 cm_wkup_wdtimer2_clkctrl;
	u32 pad206;
	u32 cm_wkup_gpio1_clkctrl;
	u32 pad207;
	u32 cm_wkup_gptimer1_clkctrl;
	u32 pad208;
	u32 cm_wkup_gptimer12_clkctrl;
	u32 pad209;
	u32 cm_wkup_synctimer_clkctrl;
	u32 pad210;
	u32 cm_wkup_usim_clkctrl;
	u32 pad211;
	u32 cm_wkup_sarram_clkctrl;
	u32 pad212[5];
	u32 cm_wkup_keyboard_clkctrl;
	u32 pad213;
	u32 cm_wkup_rtc_clkctrl;
	u32 pad214;
	u32 cm_wkup_bandgap_clkctrl;
	u32 pad215[197];
	u32 prm_vc_val_bypass;
	u32 prm_vc_cfg_channel;
	u32 prm_vc_cfg_i2c_mode;
	u32 prm_vc_cfg_i2c_clk;

};

/* DPLL register offsets */
#define CM_CLKMODE_DPLL		0
#define CM_IDLEST_DPLL		0x4
#define CM_AUTOIDLE_DPLL	0x8
#define CM_CLKSEL_DPLL		0xC
#define CM_DIV_M2_DPLL		0x10
#define CM_DIV_M3_DPLL		0x14
#define CM_DIV_M4_DPLL		0x18
#define CM_DIV_M5_DPLL		0x1C
#define CM_DIV_M6_DPLL		0x20
#define CM_DIV_M7_DPLL		0x24

#define DPLL_CLKOUT_DIV_MASK	0x1F /* post-divider mask */

/* CM_CLKMODE_DPLL */
#define CM_CLKMODE_DPLL_REGM4XEN_SHIFT		11
#define CM_CLKMODE_DPLL_REGM4XEN_MASK		(1 << 11)
#define CM_CLKMODE_DPLL_LPMODE_EN_SHIFT		10
#define CM_CLKMODE_DPLL_LPMODE_EN_MASK		(1 << 10)
#define CM_CLKMODE_DPLL_RELOCK_RAMP_EN_SHIFT	9
#define CM_CLKMODE_DPLL_RELOCK_RAMP_EN_MASK	(1 << 9)
#define CM_CLKMODE_DPLL_DRIFTGUARD_EN_SHIFT	8
#define CM_CLKMODE_DPLL_DRIFTGUARD_EN_MASK	(1 << 8)
#define CM_CLKMODE_DPLL_RAMP_RATE_SHIFT		5
#define CM_CLKMODE_DPLL_RAMP_RATE_MASK		(0x7 << 5)
#define CM_CLKMODE_DPLL_EN_SHIFT		0
#define CM_CLKMODE_DPLL_EN_MASK			(0x7 << 0)

#define CM_CLKMODE_DPLL_DPLL_EN_SHIFT		0
#define CM_CLKMODE_DPLL_DPLL_EN_MASK		7

#define DPLL_EN_STOP			1
#define DPLL_EN_MN_BYPASS		4
#define DPLL_EN_LOW_POWER_BYPASS	5
#define DPLL_EN_FAST_RELOCK_BYPASS	6
#define DPLL_EN_LOCK			7

/* CM_IDLEST_DPLL fields */
#define ST_DPLL_CLK_MASK		1

/* CM_CLKSEL_DPLL */
#define CM_CLKSEL_DPLL_DPLL_SD_DIV_SHIFT	24
#define CM_CLKSEL_DPLL_DPLL_SD_DIV_MASK		(0xFF << 24)
#define CM_CLKSEL_DPLL_M_SHIFT			8
#define CM_CLKSEL_DPLL_M_MASK			(0x7FF << 8)
#define CM_CLKSEL_DPLL_N_SHIFT			0
#define CM_CLKSEL_DPLL_N_MASK			0x7F

#define OMAP4_DPLL_MAX_N	127

/* CM_SYS_CLKSEL */
#define CM_SYS_CLKSEL_SYS_CLKSEL_MASK	7

/* CM_CLKSEL_CORE */
#define CLKSEL_CORE_SHIFT	0
#define CLKSEL_L3_SHIFT		4
#define CLKSEL_L4_SHIFT		8

#define CLKSEL_CORE_X2_DIV_1	0
#define CLKSEL_L3_CORE_DIV_2	1
#define CLKSEL_L4_L3_DIV_2	1

/* CM_ABE_PLL_REF_CLKSEL */
#define CM_ABE_PLL_REF_CLKSEL_CLKSEL_SHIFT	0
#define CM_ABE_PLL_REF_CLKSEL_CLKSEL_MASK	1
#define CM_ABE_PLL_REF_CLKSEL_CLKSEL_SYSCLK	0
#define CM_ABE_PLL_REF_CLKSEL_CLKSEL_32KCLK	1

/* CM_BYPCLK_DPLL_IVA */
#define CM_BYPCLK_DPLL_IVA_CLKSEL_SHIFT		0
#define CM_BYPCLK_DPLL_IVA_CLKSEL_MASK		3

#define DPLL_IVA_CLKSEL_CORE_X2_DIV_2		1

/* CM_SHADOW_FREQ_CONFIG1 */
#define SHADOW_FREQ_CONFIG1_FREQ_UPDATE_MASK	1
#define SHADOW_FREQ_CONFIG1_DLL_OVERRIDE_MASK	4
#define SHADOW_FREQ_CONFIG1_DLL_RESET_MASK	8

#define SHADOW_FREQ_CONFIG1_DPLL_EN_SHIFT	8
#define SHADOW_FREQ_CONFIG1_DPLL_EN_MASK	(7 << 8)

#define SHADOW_FREQ_CONFIG1_M2_DIV_SHIFT	11
#define SHADOW_FREQ_CONFIG1_M2_DIV_MASK		(0x1F << 11)

/*CM_<clock_domain>__CLKCTRL */
#define CD_CLKCTRL_CLKTRCTRL_SHIFT		0
#define CD_CLKCTRL_CLKTRCTRL_MASK		3

#define CD_CLKCTRL_CLKTRCTRL_NO_SLEEP		0
#define CD_CLKCTRL_CLKTRCTRL_SW_SLEEP		1
#define CD_CLKCTRL_CLKTRCTRL_SW_WKUP		2
#define CD_CLKCTRL_CLKTRCTRL_HW_AUTO		3


/* CM_<clock_domain>_<module>_CLKCTRL */
#define MODULE_CLKCTRL_MODULEMODE_SHIFT		0
#define MODULE_CLKCTRL_MODULEMODE_MASK		3
#define MODULE_CLKCTRL_IDLEST_SHIFT		16
#define MODULE_CLKCTRL_IDLEST_MASK		(3 << 16)

#define MODULE_CLKCTRL_MODULEMODE_SW_DISABLE		0
#define MODULE_CLKCTRL_MODULEMODE_HW_AUTO		1
#define MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN	2

#define MODULE_CLKCTRL_IDLEST_FULLY_FUNCTIONAL	0
#define MODULE_CLKCTRL_IDLEST_TRANSITIONING	1
#define MODULE_CLKCTRL_IDLEST_IDLE		2
#define MODULE_CLKCTRL_IDLEST_DISABLED		3

/* CM_L4PER_GPIO4_CLKCTRL */
#define GPIO4_CLKCTRL_OPTFCLKEN_MASK		(1 << 8)

/* CM_L3INIT_HSMMCn_CLKCTRL */
#define HSMMC_CLKCTRL_CLKSEL_MASK		(1 << 24)

/* CM_WKUP_GPTIMER1_CLKCTRL */
#define GPTIMER1_CLKCTRL_CLKSEL_MASK		(1 << 24)

/* CM_CAM_ISS_CLKCTRL */
#define ISS_CLKCTRL_OPTFCLKEN_MASK		(1 << 8)

/* CM_DSS_DSS_CLKCTRL */
#define DSS_CLKCTRL_OPTFCLKEN_MASK		0xF00

/* CM_L3INIT_USBPHY_CLKCTRL */
#define USBPHY_CLKCTRL_OPTFCLKEN_PHY_48M_MASK	8

/* Clock frequencies */
#define OMAP_SYS_CLK_FREQ_38_4_MHZ	38400000
#define OMAP_SYS_CLK_IND_38_4_MHZ	6
#define OMAP_32K_CLK_FREQ		32768

/* PRM_VC_CFG_I2C_CLK */
#define PRM_VC_CFG_I2C_CLK_SCLH_SHIFT		0
#define PRM_VC_CFG_I2C_CLK_SCLH_MASK		0xFF
#define PRM_VC_CFG_I2C_CLK_SCLL_SHIFT		8
#define PRM_VC_CFG_I2C_CLK_SCLL_MASK		(0xFF << 8)

/* PRM_VC_VAL_BYPASS */
#define PRM_VC_I2C_CHANNEL_FREQ_KHZ	400

#define PRM_VC_VAL_BYPASS_VALID_BIT	0x1000000
#define PRM_VC_VAL_BYPASS_SLAVEADDR_SHIFT	0
#define PRM_VC_VAL_BYPASS_SLAVEADDR_MASK	0x7F
#define PRM_VC_VAL_BYPASS_REGADDR_SHIFT		8
#define PRM_VC_VAL_BYPASS_REGADDR_MASK		0xFF
#define PRM_VC_VAL_BYPASS_DATA_SHIFT		16
#define PRM_VC_VAL_BYPASS_DATA_MASK		0xFF

#define SMPS_I2C_SLAVE_ADDR	0x12
#define SMPS_REG_ADDR_VCORE1	0x55
#define SMPS_REG_ADDR_VCORE2	0x5B
#define SMPS_REG_ADDR_VCORE3	0x61

#define PHOENIX_SMPS_BASE_VOLT_STD_MODE_UV		607700
#define PHOENIX_SMPS_BASE_VOLT_STD_MODE_WITH_OFFSET_UV	709000

/* Defines for DPLL setup */
#define DPLL_LOCKED_FREQ_TOLERANCE_0		0
#define DPLL_LOCKED_FREQ_TOLERANCE_500_KHZ	500
#define DPLL_LOCKED_FREQ_TOLERANCE_1_MHZ	1000

#define DPLL_NO_LOCK	0
#define DPLL_LOCK	1

#define NUM_SYS_CLKS	7

struct dpll_regs {
	u32 cm_clkmode_dpll;
	u32 cm_idlest_dpll;
	u32 cm_autoidle_dpll;
	u32 cm_clksel_dpll;
	u32 cm_div_m2_dpll;
	u32 cm_div_m3_dpll;
	u32 cm_div_m4_dpll;
	u32 cm_div_m5_dpll;
	u32 cm_div_m6_dpll;
	u32 cm_div_m7_dpll;
};

/* DPLL parameter table */
struct dpll_params {
	u32 m;
	u32 n;
	u8 m2;
	u8 m3;
	u8 m4;
	u8 m5;
	u8 m6;
	u8 m7;
};

#endif /* _CLOCKS_OMAP4_H_ */
