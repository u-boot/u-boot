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
#ifndef	_OMAP_COMMON_H_
#define	_OMAP_COMMON_H_

#include <common.h>

#define NUM_SYS_CLKS	8

struct prcm_regs {
	/* cm1.ckgen */
	u32 cm_clksel_core;
	u32 cm_clksel_abe;
	u32 cm_dll_ctrl;
	u32 cm_clkmode_dpll_core;
	u32 cm_idlest_dpll_core;
	u32 cm_autoidle_dpll_core;
	u32 cm_clksel_dpll_core;
	u32 cm_div_m2_dpll_core;
	u32 cm_div_m3_dpll_core;
	u32 cm_div_h11_dpll_core;
	u32 cm_div_h12_dpll_core;
	u32 cm_div_h13_dpll_core;
	u32 cm_div_h14_dpll_core;
	u32 cm_div_h21_dpll_core;
	u32 cm_div_h24_dpll_core;
	u32 cm_ssc_deltamstep_dpll_core;
	u32 cm_ssc_modfreqdiv_dpll_core;
	u32 cm_emu_override_dpll_core;
	u32 cm_div_h22_dpllcore;
	u32 cm_div_h23_dpll_core;
	u32 cm_clkmode_dpll_mpu;
	u32 cm_idlest_dpll_mpu;
	u32 cm_autoidle_dpll_mpu;
	u32 cm_clksel_dpll_mpu;
	u32 cm_div_m2_dpll_mpu;
	u32 cm_ssc_deltamstep_dpll_mpu;
	u32 cm_ssc_modfreqdiv_dpll_mpu;
	u32 cm_bypclk_dpll_mpu;
	u32 cm_clkmode_dpll_iva;
	u32 cm_idlest_dpll_iva;
	u32 cm_autoidle_dpll_iva;
	u32 cm_clksel_dpll_iva;
	u32 cm_div_h11_dpll_iva;
	u32 cm_div_h12_dpll_iva;
	u32 cm_ssc_deltamstep_dpll_iva;
	u32 cm_ssc_modfreqdiv_dpll_iva;
	u32 cm_bypclk_dpll_iva;
	u32 cm_clkmode_dpll_abe;
	u32 cm_idlest_dpll_abe;
	u32 cm_autoidle_dpll_abe;
	u32 cm_clksel_dpll_abe;
	u32 cm_div_m2_dpll_abe;
	u32 cm_div_m3_dpll_abe;
	u32 cm_ssc_deltamstep_dpll_abe;
	u32 cm_ssc_modfreqdiv_dpll_abe;
	u32 cm_clkmode_dpll_ddrphy;
	u32 cm_idlest_dpll_ddrphy;
	u32 cm_autoidle_dpll_ddrphy;
	u32 cm_clksel_dpll_ddrphy;
	u32 cm_div_m2_dpll_ddrphy;
	u32 cm_div_h11_dpll_ddrphy;
	u32 cm_div_h12_dpll_ddrphy;
	u32 cm_div_h13_dpll_ddrphy;
	u32 cm_ssc_deltamstep_dpll_ddrphy;
	u32 cm_clkmode_dpll_dsp;
	u32 cm_shadow_freq_config1;
	u32 cm_mpu_mpu_clkctrl;

	/* cm1.dsp */
	u32 cm_dsp_clkstctrl;
	u32 cm_dsp_dsp_clkctrl;

	/* cm1.abe */
	u32 cm1_abe_clkstctrl;
	u32 cm1_abe_l4abe_clkctrl;
	u32 cm1_abe_aess_clkctrl;
	u32 cm1_abe_pdm_clkctrl;
	u32 cm1_abe_dmic_clkctrl;
	u32 cm1_abe_mcasp_clkctrl;
	u32 cm1_abe_mcbsp1_clkctrl;
	u32 cm1_abe_mcbsp2_clkctrl;
	u32 cm1_abe_mcbsp3_clkctrl;
	u32 cm1_abe_slimbus_clkctrl;
	u32 cm1_abe_timer5_clkctrl;
	u32 cm1_abe_timer6_clkctrl;
	u32 cm1_abe_timer7_clkctrl;
	u32 cm1_abe_timer8_clkctrl;
	u32 cm1_abe_wdt3_clkctrl;

	/* cm2.ckgen */
	u32 cm_clksel_mpu_m3_iss_root;
	u32 cm_clksel_usb_60mhz;
	u32 cm_scale_fclk;
	u32 cm_core_dvfs_perf1;
	u32 cm_core_dvfs_perf2;
	u32 cm_core_dvfs_perf3;
	u32 cm_core_dvfs_perf4;
	u32 cm_core_dvfs_current;
	u32 cm_iva_dvfs_perf_tesla;
	u32 cm_iva_dvfs_perf_ivahd;
	u32 cm_iva_dvfs_perf_abe;
	u32 cm_iva_dvfs_current;
	u32 cm_clkmode_dpll_per;
	u32 cm_idlest_dpll_per;
	u32 cm_autoidle_dpll_per;
	u32 cm_clksel_dpll_per;
	u32 cm_div_m2_dpll_per;
	u32 cm_div_m3_dpll_per;
	u32 cm_div_h11_dpll_per;
	u32 cm_div_h12_dpll_per;
	u32 cm_div_h13_dpll_per;
	u32 cm_div_h14_dpll_per;
	u32 cm_ssc_deltamstep_dpll_per;
	u32 cm_ssc_modfreqdiv_dpll_per;
	u32 cm_emu_override_dpll_per;
	u32 cm_clkmode_dpll_usb;
	u32 cm_idlest_dpll_usb;
	u32 cm_autoidle_dpll_usb;
	u32 cm_clksel_dpll_usb;
	u32 cm_div_m2_dpll_usb;
	u32 cm_ssc_deltamstep_dpll_usb;
	u32 cm_ssc_modfreqdiv_dpll_usb;
	u32 cm_clkdcoldo_dpll_usb;
	u32 cm_clkmode_dpll_pcie_ref;
	u32 cm_clkmode_apll_pcie;
	u32 cm_idlest_apll_pcie;
	u32 cm_div_m2_apll_pcie;
	u32 cm_clkvcoldo_apll_pcie;
	u32 cm_clkmode_dpll_unipro;
	u32 cm_idlest_dpll_unipro;
	u32 cm_autoidle_dpll_unipro;
	u32 cm_clksel_dpll_unipro;
	u32 cm_div_m2_dpll_unipro;
	u32 cm_ssc_deltamstep_dpll_unipro;
	u32 cm_ssc_modfreqdiv_dpll_unipro;

	/* cm2.core */
	u32 cm_coreaon_bandgap_clkctrl;
	u32 cm_coreaon_io_srcomp_clkctrl;
	u32 cm_l3_1_clkstctrl;
	u32 cm_l3_1_dynamicdep;
	u32 cm_l3_1_l3_1_clkctrl;
	u32 cm_l3_2_clkstctrl;
	u32 cm_l3_2_dynamicdep;
	u32 cm_l3_2_l3_2_clkctrl;
	u32 cm_l3_gpmc_clkctrl;
	u32 cm_l3_2_ocmc_ram_clkctrl;
	u32 cm_mpu_m3_clkstctrl;
	u32 cm_mpu_m3_staticdep;
	u32 cm_mpu_m3_dynamicdep;
	u32 cm_mpu_m3_mpu_m3_clkctrl;
	u32 cm_sdma_clkstctrl;
	u32 cm_sdma_staticdep;
	u32 cm_sdma_dynamicdep;
	u32 cm_sdma_sdma_clkctrl;
	u32 cm_memif_clkstctrl;
	u32 cm_memif_dmm_clkctrl;
	u32 cm_memif_emif_fw_clkctrl;
	u32 cm_memif_emif_1_clkctrl;
	u32 cm_memif_emif_2_clkctrl;
	u32 cm_memif_dll_clkctrl;
	u32 cm_memif_emif_h1_clkctrl;
	u32 cm_memif_emif_h2_clkctrl;
	u32 cm_memif_dll_h_clkctrl;
	u32 cm_c2c_clkstctrl;
	u32 cm_c2c_staticdep;
	u32 cm_c2c_dynamicdep;
	u32 cm_c2c_sad2d_clkctrl;
	u32 cm_c2c_modem_icr_clkctrl;
	u32 cm_c2c_sad2d_fw_clkctrl;
	u32 cm_l4cfg_clkstctrl;
	u32 cm_l4cfg_dynamicdep;
	u32 cm_l4cfg_l4_cfg_clkctrl;
	u32 cm_l4cfg_hw_sem_clkctrl;
	u32 cm_l4cfg_mailbox_clkctrl;
	u32 cm_l4cfg_sar_rom_clkctrl;
	u32 cm_l3instr_clkstctrl;
	u32 cm_l3instr_l3_3_clkctrl;
	u32 cm_l3instr_l3_instr_clkctrl;
	u32 cm_l3instr_intrconn_wp1_clkctrl;

	/* cm2.ivahd */
	u32 cm_ivahd_clkstctrl;
	u32 cm_ivahd_ivahd_clkctrl;
	u32 cm_ivahd_sl2_clkctrl;

	/* cm2.cam */
	u32 cm_cam_clkstctrl;
	u32 cm_cam_iss_clkctrl;
	u32 cm_cam_fdif_clkctrl;
	u32 cm_cam_vip1_clkctrl;
	u32 cm_cam_vip2_clkctrl;
	u32 cm_cam_vip3_clkctrl;
	u32 cm_cam_lvdsrx_clkctrl;
	u32 cm_cam_csi1_clkctrl;
	u32 cm_cam_csi2_clkctrl;

	/* cm2.dss */
	u32 cm_dss_clkstctrl;
	u32 cm_dss_dss_clkctrl;

	/* cm2.sgx */
	u32 cm_sgx_clkstctrl;
	u32 cm_sgx_sgx_clkctrl;

	/* cm2.l3init */
	u32 cm_l3init_clkstctrl;

	/* cm2.l3init */
	u32 cm_l3init_hsmmc1_clkctrl;
	u32 cm_l3init_hsmmc2_clkctrl;
	u32 cm_l3init_hsi_clkctrl;
	u32 cm_l3init_hsusbhost_clkctrl;
	u32 cm_l3init_hsusbotg_clkctrl;
	u32 cm_l3init_hsusbtll_clkctrl;
	u32 cm_l3init_p1500_clkctrl;
	u32 cm_l3init_fsusb_clkctrl;
	u32 cm_l3init_ocp2scp1_clkctrl;

	/* cm2.l4per */
	u32 cm_l4per_clkstctrl;
	u32 cm_l4per_dynamicdep;
	u32 cm_l4per_adc_clkctrl;
	u32 cm_l4per_gptimer10_clkctrl;
	u32 cm_l4per_gptimer11_clkctrl;
	u32 cm_l4per_gptimer2_clkctrl;
	u32 cm_l4per_gptimer3_clkctrl;
	u32 cm_l4per_gptimer4_clkctrl;
	u32 cm_l4per_gptimer9_clkctrl;
	u32 cm_l4per_elm_clkctrl;
	u32 cm_l4per_gpio2_clkctrl;
	u32 cm_l4per_gpio3_clkctrl;
	u32 cm_l4per_gpio4_clkctrl;
	u32 cm_l4per_gpio5_clkctrl;
	u32 cm_l4per_gpio6_clkctrl;
	u32 cm_l4per_hdq1w_clkctrl;
	u32 cm_l4per_hecc1_clkctrl;
	u32 cm_l4per_hecc2_clkctrl;
	u32 cm_l4per_i2c1_clkctrl;
	u32 cm_l4per_i2c2_clkctrl;
	u32 cm_l4per_i2c3_clkctrl;
	u32 cm_l4per_i2c4_clkctrl;
	u32 cm_l4per_l4per_clkctrl;
	u32 cm_l4per_mcasp2_clkctrl;
	u32 cm_l4per_mcasp3_clkctrl;
	u32 cm_l4per_mgate_clkctrl;
	u32 cm_l4per_mcspi1_clkctrl;
	u32 cm_l4per_mcspi2_clkctrl;
	u32 cm_l4per_mcspi3_clkctrl;
	u32 cm_l4per_mcspi4_clkctrl;
	u32 cm_l4per_gpio7_clkctrl;
	u32 cm_l4per_gpio8_clkctrl;
	u32 cm_l4per_mmcsd3_clkctrl;
	u32 cm_l4per_mmcsd4_clkctrl;
	u32 cm_l4per_msprohg_clkctrl;
	u32 cm_l4per_slimbus2_clkctrl;
	u32 cm_l4per_uart1_clkctrl;
	u32 cm_l4per_uart2_clkctrl;
	u32 cm_l4per_uart3_clkctrl;
	u32 cm_l4per_uart4_clkctrl;
	u32 cm_l4per_mmcsd5_clkctrl;
	u32 cm_l4per_i2c5_clkctrl;
	u32 cm_l4per_uart5_clkctrl;
	u32 cm_l4per_uart6_clkctrl;
	u32 cm_l4sec_clkstctrl;
	u32 cm_l4sec_staticdep;
	u32 cm_l4sec_dynamicdep;
	u32 cm_l4sec_aes1_clkctrl;
	u32 cm_l4sec_aes2_clkctrl;
	u32 cm_l4sec_des3des_clkctrl;
	u32 cm_l4sec_pkaeip29_clkctrl;
	u32 cm_l4sec_rng_clkctrl;
	u32 cm_l4sec_sha2md51_clkctrl;
	u32 cm_l4sec_cryptodma_clkctrl;

	/* l4 wkup regs */
	u32 cm_abe_pll_ref_clksel;
	u32 cm_sys_clksel;
	u32 cm_wkup_clkstctrl;
	u32 cm_wkup_l4wkup_clkctrl;
	u32 cm_wkup_wdtimer1_clkctrl;
	u32 cm_wkup_wdtimer2_clkctrl;
	u32 cm_wkup_gpio1_clkctrl;
	u32 cm_wkup_gptimer1_clkctrl;
	u32 cm_wkup_gptimer12_clkctrl;
	u32 cm_wkup_synctimer_clkctrl;
	u32 cm_wkup_usim_clkctrl;
	u32 cm_wkup_sarram_clkctrl;
	u32 cm_wkup_keyboard_clkctrl;
	u32 cm_wkup_rtc_clkctrl;
	u32 cm_wkup_bandgap_clkctrl;
	u32 cm_wkupaon_scrm_clkctrl;
	u32 cm_wkupaon_io_srcomp_clkctrl;
	u32 prm_rstctrl;
	u32 prm_rstst;
	u32 prm_vc_val_bypass;
	u32 prm_vc_cfg_i2c_mode;
	u32 prm_vc_cfg_i2c_clk;
	u32 prm_sldo_core_setup;
	u32 prm_sldo_core_ctrl;
	u32 prm_sldo_mpu_setup;
	u32 prm_sldo_mpu_ctrl;
	u32 prm_sldo_mm_setup;
	u32 prm_sldo_mm_ctrl;

	u32 cm_div_m4_dpll_core;
	u32 cm_div_m5_dpll_core;
	u32 cm_div_m6_dpll_core;
	u32 cm_div_m7_dpll_core;
	u32 cm_div_m4_dpll_iva;
	u32 cm_div_m5_dpll_iva;
	u32 cm_div_m4_dpll_ddrphy;
	u32 cm_div_m5_dpll_ddrphy;
	u32 cm_div_m6_dpll_ddrphy;
	u32 cm_div_m4_dpll_per;
	u32 cm_div_m5_dpll_per;
	u32 cm_div_m6_dpll_per;
	u32 cm_div_m7_dpll_per;
	u32 cm_l3instr_intrconn_wp1_clkct;
	u32 cm_l3init_usbphy_clkctrl;
	u32 cm_l4per_mcbsp4_clkctrl;
	u32 prm_vc_cfg_channel;
};

struct omap_sys_ctrl_regs {
	u32 control_status;
	u32 control_core_mmr_lock1;
	u32 control_core_mmr_lock2;
	u32 control_core_mmr_lock3;
	u32 control_core_mmr_lock4;
	u32 control_core_mmr_lock5;
	u32 control_core_control_io1;
	u32 control_core_control_io2;
	u32 control_id_code;
	u32 control_std_fuse_opp_bgap;
	u32 control_ldosram_iva_voltage_ctrl;
	u32 control_ldosram_mpu_voltage_ctrl;
	u32 control_ldosram_core_voltage_ctrl;
	u32 control_padconf_core_base;
	u32 control_paconf_global;
	u32 control_paconf_mode;
	u32 control_smart1io_padconf_0;
	u32 control_smart1io_padconf_1;
	u32 control_smart1io_padconf_2;
	u32 control_smart2io_padconf_0;
	u32 control_smart2io_padconf_1;
	u32 control_smart2io_padconf_2;
	u32 control_smart3io_padconf_0;
	u32 control_smart3io_padconf_1;
	u32 control_pbias;
	u32 control_i2c_0;
	u32 control_camera_rx;
	u32 control_hdmi_tx_phy;
	u32 control_uniportm;
	u32 control_dsiphy;
	u32 control_mcbsplp;
	u32 control_usb2phycore;
	u32 control_hdmi_1;
	u32 control_hsi;
	u32 control_ddr3ch1_0;
	u32 control_ddr3ch2_0;
	u32 control_ddrch1_0;
	u32 control_ddrch1_1;
	u32 control_ddrch2_0;
	u32 control_ddrch2_1;
	u32 control_lpddr2ch1_0;
	u32 control_lpddr2ch1_1;
	u32 control_ddrio_0;
	u32 control_ddrio_1;
	u32 control_ddrio_2;
	u32 control_lpddr2io1_0;
	u32 control_lpddr2io1_1;
	u32 control_lpddr2io1_2;
	u32 control_lpddr2io1_3;
	u32 control_lpddr2io2_0;
	u32 control_lpddr2io2_1;
	u32 control_lpddr2io2_2;
	u32 control_lpddr2io2_3;
	u32 control_hyst_1;
	u32 control_usbb_hsic_control;
	u32 control_c2c;
	u32 control_core_control_spare_rw;
	u32 control_core_control_spare_r;
	u32 control_core_control_spare_r_c0;
	u32 control_srcomp_north_side;
	u32 control_srcomp_south_side;
	u32 control_srcomp_east_side;
	u32 control_srcomp_west_side;
	u32 control_srcomp_code_latch;
	u32 control_pbiaslite;
	u32 control_port_emif1_sdram_config;
	u32 control_port_emif1_lpddr2_nvm_config;
	u32 control_port_emif2_sdram_config;
	u32 control_emif1_sdram_config_ext;
	u32 control_emif2_sdram_config_ext;
	u32 control_smart1nopmio_padconf_0;
	u32 control_smart1nopmio_padconf_1;
	u32 control_padconf_mode;
	u32 control_xtal_oscillator;
	u32 control_i2c_2;
	u32 control_ckobuffer;
	u32 control_wkup_control_spare_rw;
	u32 control_wkup_control_spare_r;
	u32 control_wkup_control_spare_r_c0;
	u32 control_srcomp_east_side_wkup;
	u32 control_efuse_1;
	u32 control_efuse_2;
	u32 control_efuse_3;
	u32 control_efuse_4;
	u32 control_efuse_5;
	u32 control_efuse_6;
	u32 control_efuse_7;
	u32 control_efuse_8;
	u32 control_efuse_9;
	u32 control_efuse_10;
	u32 control_efuse_11;
	u32 control_efuse_12;
	u32 control_efuse_13;
	u32 control_padconf_wkup_base;
};

struct dpll_params {
	u32 m;
	u32 n;
	s8 m2;
	s8 m3;
	s8 m4_h11;
	s8 m5_h12;
	s8 m6_h13;
	s8 m7_h14;
	s8 h21;
	s8 h22;
	s8 h23;
	s8 h24;
};

struct dpll_regs {
	u32 cm_clkmode_dpll;
	u32 cm_idlest_dpll;
	u32 cm_autoidle_dpll;
	u32 cm_clksel_dpll;
	u32 cm_div_m2_dpll;
	u32 cm_div_m3_dpll;
	u32 cm_div_m4_h11_dpll;
	u32 cm_div_m5_h12_dpll;
	u32 cm_div_m6_h13_dpll;
	u32 cm_div_m7_h14_dpll;
	u32 reserved[2];
	u32 cm_div_h21_dpll;
	u32 cm_div_h22_dpll;
	u32 cm_div_h23_dpll;
	u32 cm_div_h24_dpll;
};

struct dplls {
	const struct dpll_params *mpu;
	const struct dpll_params *core;
	const struct dpll_params *per;
	const struct dpll_params *abe;
	const struct dpll_params *iva;
	const struct dpll_params *usb;
	const struct dpll_params *ddr;
};

struct pmic_data {
	u32 base_offset;
	u32 step;
	u32 start_code;
	unsigned gpio;
	int gpio_en;
};

struct volts {
	u32 value;
	u32 addr;
	struct pmic_data *pmic;
};

struct vcores_data {
	struct volts mpu;
	struct volts core;
	struct volts mm;
};

extern struct prcm_regs const **prcm;
extern struct prcm_regs const omap5_es1_prcm;
extern struct prcm_regs const omap5_es2_prcm;
extern struct prcm_regs const omap4_prcm;
extern struct prcm_regs const dra7xx_prcm;
extern struct dplls const **dplls_data;
extern struct vcores_data const **omap_vcores;
extern const u32 sys_clk_array[8];
extern struct omap_sys_ctrl_regs const **ctrl;
extern struct omap_sys_ctrl_regs const omap4_ctrl;
extern struct omap_sys_ctrl_regs const omap5_ctrl;
extern struct omap_sys_ctrl_regs const dra7xx_ctrl;

void hw_data_init(void);

const struct dpll_params *get_mpu_dpll_params(struct dplls const *);
const struct dpll_params *get_core_dpll_params(struct dplls const *);
const struct dpll_params *get_per_dpll_params(struct dplls const *);
const struct dpll_params *get_iva_dpll_params(struct dplls const *);
const struct dpll_params *get_usb_dpll_params(struct dplls const *);
const struct dpll_params *get_abe_dpll_params(struct dplls const *);

void do_enable_clocks(u32 const *clk_domains,
		      u32 const *clk_modules_hw_auto,
		      u32 const *clk_modules_explicit_en,
		      u8 wait_for_enable);

void setup_post_dividers(u32 const base,
			const struct dpll_params *params);
u32 omap_ddr_clk(void);
u32 get_sys_clk_index(void);
void enable_basic_clocks(void);
void enable_basic_uboot_clocks(void);
void enable_non_essential_clocks(void);
void scale_vcores(struct vcores_data const *);
u32 get_offset_code(u32 volt_offset, struct pmic_data *pmic);
void do_scale_vcore(u32 vcore_reg, u32 volt_mv, struct pmic_data *pmic);

/* Max value for DPLL multiplier M */
#define OMAP_DPLL_MAX_N	127

/* HW Init Context */
#define OMAP_INIT_CONTEXT_SPL			0
#define OMAP_INIT_CONTEXT_UBOOT_FROM_NOR	1
#define OMAP_INIT_CONTEXT_UBOOT_AFTER_SPL	2
#define OMAP_INIT_CONTEXT_UBOOT_AFTER_CH	3

static inline u32 omap_revision(void)
{
	extern u32 *const omap_si_rev;
	return *omap_si_rev;
}

/*
 * silicon revisions.
 * Moving this to common, so that most of code can be moved to common,
 * directories.
 */

/* omap4 */
#define OMAP4430_SILICON_ID_INVALID	0xFFFFFFFF
#define OMAP4430_ES1_0	0x44300100
#define OMAP4430_ES2_0	0x44300200
#define OMAP4430_ES2_1	0x44300210
#define OMAP4430_ES2_2	0x44300220
#define OMAP4430_ES2_3	0x44300230
#define OMAP4460_ES1_0	0x44600100
#define OMAP4460_ES1_1	0x44600110

/* omap5 */
#define OMAP5430_SILICON_ID_INVALID	0
#define OMAP5430_ES1_0	0x54300100
#define OMAP5432_ES1_0	0x54320100
#define OMAP5430_ES2_0  0x54300200
#define OMAP5432_ES2_0  0x54320200

/* DRA7XX */
#define DRA752_ES1_0	0x07520100
#endif /* _OMAP_COMMON_H_ */
