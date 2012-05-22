/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 *	Aneesh V <aneesh@ti.com>
 *	Sricharan R <r.sricharan@ti.com>
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
#ifndef _CLOCKS_OMAP5_H_
#define _CLOCKS_OMAP5_H_
#include <common.h>

/*
 * Assuming a maximum of 1.5 GHz ARM speed and a minimum of 2 cycles per
 * loop, allow for a minimum of 2 ms wait (in reality the wait will be
 * much more than that)
 */
#define LDELAY		1000000

#define CM_CLKMODE_DPLL_CORE		(OMAP54XX_L4_CORE_BASE + 0x4120)
#define CM_CLKMODE_DPLL_PER		(OMAP54XX_L4_CORE_BASE + 0x8140)
#define CM_CLKMODE_DPLL_MPU		(OMAP54XX_L4_CORE_BASE + 0x4160)
#define CM_CLKSEL_CORE			(OMAP54XX_L4_CORE_BASE + 0x4100)

struct omap5_prcm_regs {
	/* cm1.ckgen */
	u32 cm_clksel_core;			/* 4a004100 */
	u32 pad001[1];				/* 4a004104 */
	u32 cm_clksel_abe;			/* 4a004108 */
	u32 pad002[1];				/* 4a00410c */
	u32 cm_dll_ctrl;			/* 4a004110 */
	u32 pad003[3];				/* 4a004114 */
	u32 cm_clkmode_dpll_core;		/* 4a004120 */
	u32 cm_idlest_dpll_core;		/* 4a004124 */
	u32 cm_autoidle_dpll_core;		/* 4a004128 */
	u32 cm_clksel_dpll_core;		/* 4a00412c */
	u32 cm_div_m2_dpll_core;		/* 4a004130 */
	u32 cm_div_m3_dpll_core;		/* 4a004134 */
	u32 cm_div_h11_dpll_core;		/* 4a004138 */
	u32 cm_div_h12_dpll_core;		/* 4a00413c */
	u32 cm_div_h13_dpll_core;		/* 4a004140 */
	u32 cm_div_h14_dpll_core;		/* 4a004144 */
	u32 cm_ssc_deltamstep_dpll_core;	/* 4a004148 */
	u32 cm_ssc_modfreqdiv_dpll_core;	/* 4a00414c */
	u32 cm_emu_override_dpll_core;		/* 4a004150 */

	u32 cm_div_h22_dpllcore;		/* 4a004154 */
	u32 cm_div_h23_dpll_core;		/* 4a004158 */
	u32 pad0041[1];				/* 4a00415c */
	u32 cm_clkmode_dpll_mpu;		/* 4a004160 */
	u32 cm_idlest_dpll_mpu;			/* 4a004164 */
	u32 cm_autoidle_dpll_mpu;		/* 4a004168 */
	u32 cm_clksel_dpll_mpu;			/* 4a00416c */
	u32 cm_div_m2_dpll_mpu;			/* 4a004170 */
	u32 pad005[5];				/* 4a004174 */
	u32 cm_ssc_deltamstep_dpll_mpu;		/* 4a004188 */
	u32 cm_ssc_modfreqdiv_dpll_mpu;		/* 4a00418c */
	u32 pad006[3];				/* 4a004190 */
	u32 cm_bypclk_dpll_mpu;			/* 4a00419c */
	u32 cm_clkmode_dpll_iva;		/* 4a0041a0 */
	u32 cm_idlest_dpll_iva;			/* 4a0041a4 */
	u32 cm_autoidle_dpll_iva;		/* 4a0041a8 */
	u32 cm_clksel_dpll_iva;			/* 4a0041ac */
	u32 pad007[2];				/* 4a0041b0 */
	u32 cm_div_h11_dpll_iva;		/* 4a0041b8 */
	u32 cm_div_h12_dpll_iva;		/* 4a0041bc */
	u32 pad008[2];				/* 4a0041c0 */
	u32 cm_ssc_deltamstep_dpll_iva;		/* 4a0041c8 */
	u32 cm_ssc_modfreqdiv_dpll_iva;		/* 4a0041cc */
	u32 pad009[3];				/* 4a0041d0 */
	u32 cm_bypclk_dpll_iva;			/* 4a0041dc */
	u32 cm_clkmode_dpll_abe;		/* 4a0041e0 */
	u32 cm_idlest_dpll_abe;			/* 4a0041e4 */
	u32 cm_autoidle_dpll_abe;		/* 4a0041e8 */
	u32 cm_clksel_dpll_abe;			/* 4a0041ec */
	u32 cm_div_m2_dpll_abe;			/* 4a0041f0 */
	u32 cm_div_m3_dpll_abe;			/* 4a0041f4 */
	u32 pad010[4];				/* 4a0041f8 */
	u32 cm_ssc_deltamstep_dpll_abe;		/* 4a004208 */
	u32 cm_ssc_modfreqdiv_dpll_abe;		/* 4a00420c */
	u32 pad011[4];				/* 4a004210 */
	u32 cm_clkmode_dpll_ddrphy;		/* 4a004220 */
	u32 cm_idlest_dpll_ddrphy;		/* 4a004224 */
	u32 cm_autoidle_dpll_ddrphy;		/* 4a004228 */
	u32 cm_clksel_dpll_ddrphy;		/* 4a00422c */
	u32 cm_div_m2_dpll_ddrphy;		/* 4a004230 */
	u32 pad012[1];				/* 4a004234 */
	u32 cm_div_h11_dpll_ddrphy;		/* 4a004238 */
	u32 cm_div_h12_dpll_ddrphy;		/* 4a00423c */
	u32 cm_div_h13_dpll_ddrphy;		/* 4a004240 */
	u32 pad013[1];				/* 4a004244 */
	u32 cm_ssc_deltamstep_dpll_ddrphy;	/* 4a004248 */
	u32 pad014[5];				/* 4a00424c */
	u32 cm_shadow_freq_config1;		/* 4a004260 */
	u32 pad0141[47];			/* 4a004264 */
	u32 cm_mpu_mpu_clkctrl;			/* 4a004320 */


	/* cm1.dsp */
	u32 pad015[55];				/* 4a004324 */
	u32 cm_dsp_clkstctrl;			/* 4a004400 */
	u32 pad016[7];				/* 4a004404 */
	u32 cm_dsp_dsp_clkctrl;			/* 4a004420 */

	/* cm1.abe */
	u32 pad017[55];				/* 4a004424 */
	u32 cm1_abe_clkstctrl;			/* 4a004500 */
	u32 pad018[7];				/* 4a004504 */
	u32 cm1_abe_l4abe_clkctrl;		/* 4a004520 */
	u32 pad019[1];				/* 4a004524 */
	u32 cm1_abe_aess_clkctrl;		/* 4a004528 */
	u32 pad020[1];				/* 4a00452c */
	u32 cm1_abe_pdm_clkctrl;		/* 4a004530 */
	u32 pad021[1];				/* 4a004534 */
	u32 cm1_abe_dmic_clkctrl;		/* 4a004538 */
	u32 pad022[1];				/* 4a00453c */
	u32 cm1_abe_mcasp_clkctrl;		/* 4a004540 */
	u32 pad023[1];				/* 4a004544 */
	u32 cm1_abe_mcbsp1_clkctrl;		/* 4a004548 */
	u32 pad024[1];				/* 4a00454c */
	u32 cm1_abe_mcbsp2_clkctrl;		/* 4a004550 */
	u32 pad025[1];				/* 4a004554 */
	u32 cm1_abe_mcbsp3_clkctrl;		/* 4a004558 */
	u32 pad026[1];				/* 4a00455c */
	u32 cm1_abe_slimbus_clkctrl;		/* 4a004560 */
	u32 pad027[1];				/* 4a004564 */
	u32 cm1_abe_timer5_clkctrl;		/* 4a004568 */
	u32 pad028[1];				/* 4a00456c */
	u32 cm1_abe_timer6_clkctrl;		/* 4a004570 */
	u32 pad029[1];				/* 4a004574 */
	u32 cm1_abe_timer7_clkctrl;		/* 4a004578 */
	u32 pad030[1];				/* 4a00457c */
	u32 cm1_abe_timer8_clkctrl;		/* 4a004580 */
	u32 pad031[1];				/* 4a004584 */
	u32 cm1_abe_wdt3_clkctrl;		/* 4a004588 */

	/* cm2.ckgen */
	u32 pad032[3805];			/* 4a00458c */
	u32 cm_clksel_mpu_m3_iss_root;		/* 4a008100 */
	u32 cm_clksel_usb_60mhz;		/* 4a008104 */
	u32 cm_scale_fclk;			/* 4a008108 */
	u32 pad033[1];				/* 4a00810c */
	u32 cm_core_dvfs_perf1;			/* 4a008110 */
	u32 cm_core_dvfs_perf2;			/* 4a008114 */
	u32 cm_core_dvfs_perf3;			/* 4a008118 */
	u32 cm_core_dvfs_perf4;			/* 4a00811c */
	u32 pad034[1];				/* 4a008120 */
	u32 cm_core_dvfs_current;		/* 4a008124 */
	u32 cm_iva_dvfs_perf_tesla;		/* 4a008128 */
	u32 cm_iva_dvfs_perf_ivahd;		/* 4a00812c */
	u32 cm_iva_dvfs_perf_abe;		/* 4a008130 */
	u32 pad035[1];				/* 4a008134 */
	u32 cm_iva_dvfs_current;		/* 4a008138 */
	u32 pad036[1];				/* 4a00813c */
	u32 cm_clkmode_dpll_per;		/* 4a008140 */
	u32 cm_idlest_dpll_per;			/* 4a008144 */
	u32 cm_autoidle_dpll_per;		/* 4a008148 */
	u32 cm_clksel_dpll_per;			/* 4a00814c */
	u32 cm_div_m2_dpll_per;			/* 4a008150 */
	u32 cm_div_m3_dpll_per;			/* 4a008154 */
	u32 cm_div_h11_dpll_per;		/* 4a008158 */
	u32 cm_div_h12_dpll_per;		/* 4a00815c */
	u32 pad0361[1];				/* 4a008160 */
	u32 cm_div_h14_dpll_per;		/* 4a008164 */
	u32 cm_ssc_deltamstep_dpll_per;		/* 4a008168 */
	u32 cm_ssc_modfreqdiv_dpll_per;		/* 4a00816c */
	u32 cm_emu_override_dpll_per;		/* 4a008170 */
	u32 pad037[3];				/* 4a008174 */
	u32 cm_clkmode_dpll_usb;		/* 4a008180 */
	u32 cm_idlest_dpll_usb;			/* 4a008184 */
	u32 cm_autoidle_dpll_usb;		/* 4a008188 */
	u32 cm_clksel_dpll_usb;			/* 4a00818c */
	u32 cm_div_m2_dpll_usb;			/* 4a008190 */
	u32 pad038[5];				/* 4a008194 */
	u32 cm_ssc_deltamstep_dpll_usb;		/* 4a0081a8 */
	u32 cm_ssc_modfreqdiv_dpll_usb;		/* 4a0081ac */
	u32 pad039[1];				/* 4a0081b0 */
	u32 cm_clkdcoldo_dpll_usb;		/* 4a0081b4 */
	u32 pad040[2];				/* 4a0081b8 */
	u32 cm_clkmode_dpll_unipro;		/* 4a0081c0 */
	u32 cm_idlest_dpll_unipro;		/* 4a0081c4 */
	u32 cm_autoidle_dpll_unipro;		/* 4a0081c8 */
	u32 cm_clksel_dpll_unipro;		/* 4a0081cc */
	u32 cm_div_m2_dpll_unipro;		/* 4a0081d0 */
	u32 pad041[5];				/* 4a0081d4 */
	u32 cm_ssc_deltamstep_dpll_unipro;	/* 4a0081e8 */
	u32 cm_ssc_modfreqdiv_dpll_unipro;	/* 4a0081ec */

	/* cm2.core */
	u32 pad0411[324];			/* 4a0081f0 */
	u32 cm_l3_1_clkstctrl;			/* 4a008700 */
	u32 pad042[1];				/* 4a008704 */
	u32 cm_l3_1_dynamicdep;			/* 4a008708 */
	u32 pad043[5];				/* 4a00870c */
	u32 cm_l3_1_l3_1_clkctrl;		/* 4a008720 */
	u32 pad044[55];				/* 4a008724 */
	u32 cm_l3_2_clkstctrl;			/* 4a008800 */
	u32 pad045[1];				/* 4a008804 */
	u32 cm_l3_2_dynamicdep;			/* 4a008808 */
	u32 pad046[5];				/* 4a00880c */
	u32 cm_l3_2_l3_2_clkctrl;		/* 4a008820 */
	u32 pad047[1];				/* 4a008824 */
	u32 cm_l3_2_gpmc_clkctrl;		/* 4a008828 */
	u32 pad048[1];				/* 4a00882c */
	u32 cm_l3_2_ocmc_ram_clkctrl;		/* 4a008830 */
	u32 pad049[51];				/* 4a008834 */
	u32 cm_mpu_m3_clkstctrl;		/* 4a008900 */
	u32 cm_mpu_m3_staticdep;		/* 4a008904 */
	u32 cm_mpu_m3_dynamicdep;		/* 4a008908 */
	u32 pad050[5];				/* 4a00890c */
	u32 cm_mpu_m3_mpu_m3_clkctrl;		/* 4a008920 */
	u32 pad051[55];				/* 4a008924 */
	u32 cm_sdma_clkstctrl;			/* 4a008a00 */
	u32 cm_sdma_staticdep;			/* 4a008a04 */
	u32 cm_sdma_dynamicdep;			/* 4a008a08 */
	u32 pad052[5];				/* 4a008a0c */
	u32 cm_sdma_sdma_clkctrl;		/* 4a008a20 */
	u32 pad053[55];				/* 4a008a24 */
	u32 cm_memif_clkstctrl;			/* 4a008b00 */
	u32 pad054[7];				/* 4a008b04 */
	u32 cm_memif_dmm_clkctrl;		/* 4a008b20 */
	u32 pad055[1];				/* 4a008b24 */
	u32 cm_memif_emif_fw_clkctrl;		/* 4a008b28 */
	u32 pad056[1];				/* 4a008b2c */
	u32 cm_memif_emif_1_clkctrl;		/* 4a008b30 */
	u32 pad057[1];				/* 4a008b34 */
	u32 cm_memif_emif_2_clkctrl;		/* 4a008b38 */
	u32 pad058[1];				/* 4a008b3c */
	u32 cm_memif_dll_clkctrl;		/* 4a008b40 */
	u32 pad059[3];				/* 4a008b44 */
	u32 cm_memif_emif_h1_clkctrl;		/* 4a008b50 */
	u32 pad060[1];				/* 4a008b54 */
	u32 cm_memif_emif_h2_clkctrl;		/* 4a008b58 */
	u32 pad061[1];				/* 4a008b5c */
	u32 cm_memif_dll_h_clkctrl;		/* 4a008b60 */
	u32 pad062[39];				/* 4a008b64 */
	u32 cm_c2c_clkstctrl;			/* 4a008c00 */
	u32 cm_c2c_staticdep;			/* 4a008c04 */
	u32 cm_c2c_dynamicdep;			/* 4a008c08 */
	u32 pad063[5];				/* 4a008c0c */
	u32 cm_c2c_sad2d_clkctrl;		/* 4a008c20 */
	u32 pad064[1];				/* 4a008c24 */
	u32 cm_c2c_modem_icr_clkctrl;		/* 4a008c28 */
	u32 pad065[1];				/* 4a008c2c */
	u32 cm_c2c_sad2d_fw_clkctrl;		/* 4a008c30 */
	u32 pad066[51];				/* 4a008c34 */
	u32 cm_l4cfg_clkstctrl;			/* 4a008d00 */
	u32 pad067[1];				/* 4a008d04 */
	u32 cm_l4cfg_dynamicdep;		/* 4a008d08 */
	u32 pad068[5];				/* 4a008d0c */
	u32 cm_l4cfg_l4_cfg_clkctrl;		/* 4a008d20 */
	u32 pad069[1];				/* 4a008d24 */
	u32 cm_l4cfg_hw_sem_clkctrl;		/* 4a008d28 */
	u32 pad070[1];				/* 4a008d2c */
	u32 cm_l4cfg_mailbox_clkctrl;		/* 4a008d30 */
	u32 pad071[1];				/* 4a008d34 */
	u32 cm_l4cfg_sar_rom_clkctrl;		/* 4a008d38 */
	u32 pad072[49];				/* 4a008d3c */
	u32 cm_l3instr_clkstctrl;		/* 4a008e00 */
	u32 pad073[7];				/* 4a008e04 */
	u32 cm_l3instr_l3_3_clkctrl;		/* 4a008e20 */
	u32 pad074[1];				/* 4a008e24 */
	u32 cm_l3instr_l3_instr_clkctrl;	/* 4a008e28 */
	u32 pad075[5];				/* 4a008e2c */
	u32 cm_l3instr_intrconn_wp1_clkctrl;	/* 4a008e40 */


	/* cm2.ivahd */
	u32 pad076[47];				/* 4a008e44 */
	u32 cm_ivahd_clkstctrl;			/* 4a008f00 */
	u32 pad077[7];				/* 4a008f04 */
	u32 cm_ivahd_ivahd_clkctrl;		/* 4a008f20 */
	u32 pad078[1];				/* 4a008f24 */
	u32 cm_ivahd_sl2_clkctrl;		/* 4a008f28 */

	/* cm2.cam */
	u32 pad079[53];				/* 4a008f2c */
	u32 cm_cam_clkstctrl;			/* 4a009000 */
	u32 pad080[7];				/* 4a009004 */
	u32 cm_cam_iss_clkctrl;			/* 4a009020 */
	u32 pad081[1];				/* 4a009024 */
	u32 cm_cam_fdif_clkctrl;		/* 4a009028 */

	/* cm2.dss */
	u32 pad082[53];				/* 4a00902c */
	u32 cm_dss_clkstctrl;			/* 4a009100 */
	u32 pad083[7];				/* 4a009104 */
	u32 cm_dss_dss_clkctrl;			/* 4a009120 */

	/* cm2.sgx */
	u32 pad084[55];				/* 4a009124 */
	u32 cm_sgx_clkstctrl;			/* 4a009200 */
	u32 pad085[7];				/* 4a009204 */
	u32 cm_sgx_sgx_clkctrl;			/* 4a009220 */

	/* cm2.l3init */
	u32 pad086[55];				/* 4a009224 */
	u32 cm_l3init_clkstctrl;		/* 4a009300 */

	/* cm2.l3init */
	u32 pad087[9];				/* 4a009304 */
	u32 cm_l3init_hsmmc1_clkctrl;		/* 4a009328 */
	u32 pad088[1];				/* 4a00932c */
	u32 cm_l3init_hsmmc2_clkctrl;		/* 4a009330 */
	u32 pad089[1];				/* 4a009334 */
	u32 cm_l3init_hsi_clkctrl;		/* 4a009338 */
	u32 pad090[7];				/* 4a00933c */
	u32 cm_l3init_hsusbhost_clkctrl;	/* 4a009358 */
	u32 pad091[1];				/* 4a00935c */
	u32 cm_l3init_hsusbotg_clkctrl;		/* 4a009360 */
	u32 pad092[1];				/* 4a009364 */
	u32 cm_l3init_hsusbtll_clkctrl;		/* 4a009368 */
	u32 pad093[3];				/* 4a00936c */
	u32 cm_l3init_p1500_clkctrl;		/* 4a009378 */
	u32 pad094[21];				/* 4a00937c */
	u32 cm_l3init_fsusb_clkctrl;		/* 4a0093d0 */
	u32 pad095[3];				/* 4a0093d4 */
	u32 cm_l3init_ocp2scp1_clkctrl;

	/* cm2.l4per */
	u32 pad096[7];				/* 4a0093e4 */
	u32 cm_l4per_clkstctrl;			/* 4a009400 */
	u32 pad097[1];				/* 4a009404 */
	u32 cm_l4per_dynamicdep;		/* 4a009408 */
	u32 pad098[5];				/* 4a00940c */
	u32 cm_l4per_adc_clkctrl;		/* 4a009420 */
	u32 pad100[1];				/* 4a009424 */
	u32 cm_l4per_gptimer10_clkctrl;		/* 4a009428 */
	u32 pad101[1];				/* 4a00942c */
	u32 cm_l4per_gptimer11_clkctrl;		/* 4a009430 */
	u32 pad102[1];				/* 4a009434 */
	u32 cm_l4per_gptimer2_clkctrl;		/* 4a009438 */
	u32 pad103[1];				/* 4a00943c */
	u32 cm_l4per_gptimer3_clkctrl;		/* 4a009440 */
	u32 pad104[1];				/* 4a009444 */
	u32 cm_l4per_gptimer4_clkctrl;		/* 4a009448 */
	u32 pad105[1];				/* 4a00944c */
	u32 cm_l4per_gptimer9_clkctrl;		/* 4a009450 */
	u32 pad106[1];				/* 4a009454 */
	u32 cm_l4per_elm_clkctrl;		/* 4a009458 */
	u32 pad107[1];				/* 4a00945c */
	u32 cm_l4per_gpio2_clkctrl;		/* 4a009460 */
	u32 pad108[1];				/* 4a009464 */
	u32 cm_l4per_gpio3_clkctrl;		/* 4a009468 */
	u32 pad109[1];				/* 4a00946c */
	u32 cm_l4per_gpio4_clkctrl;		/* 4a009470 */
	u32 pad110[1];				/* 4a009474 */
	u32 cm_l4per_gpio5_clkctrl;		/* 4a009478 */
	u32 pad111[1];				/* 4a00947c */
	u32 cm_l4per_gpio6_clkctrl;		/* 4a009480 */
	u32 pad112[1];				/* 4a009484 */
	u32 cm_l4per_hdq1w_clkctrl;		/* 4a009488 */
	u32 pad113[1];				/* 4a00948c */
	u32 cm_l4per_hecc1_clkctrl;		/* 4a009490 */
	u32 pad114[1];				/* 4a009494 */
	u32 cm_l4per_hecc2_clkctrl;		/* 4a009498 */
	u32 pad115[1];				/* 4a00949c */
	u32 cm_l4per_i2c1_clkctrl;		/* 4a0094a0 */
	u32 pad116[1];				/* 4a0094a4 */
	u32 cm_l4per_i2c2_clkctrl;		/* 4a0094a8 */
	u32 pad117[1];				/* 4a0094ac */
	u32 cm_l4per_i2c3_clkctrl;		/* 4a0094b0 */
	u32 pad118[1];				/* 4a0094b4 */
	u32 cm_l4per_i2c4_clkctrl;		/* 4a0094b8 */
	u32 pad119[1];				/* 4a0094bc */
	u32 cm_l4per_l4per_clkctrl;		/* 4a0094c0 */
	u32 pad1191[3];				/* 4a0094c4 */
	u32 cm_l4per_mcasp2_clkctrl;		/* 4a0094d0 */
	u32 pad120[1];				/* 4a0094d4 */
	u32 cm_l4per_mcasp3_clkctrl;		/* 4a0094d8 */
	u32 pad121[3];				/* 4a0094dc */
	u32 cm_l4per_mgate_clkctrl;		/* 4a0094e8 */
	u32 pad123[1];				/* 4a0094ec */
	u32 cm_l4per_mcspi1_clkctrl;		/* 4a0094f0 */
	u32 pad124[1];				/* 4a0094f4 */
	u32 cm_l4per_mcspi2_clkctrl;		/* 4a0094f8 */
	u32 pad125[1];				/* 4a0094fc */
	u32 cm_l4per_mcspi3_clkctrl;		/* 4a009500 */
	u32 pad126[1];				/* 4a009504 */
	u32 cm_l4per_mcspi4_clkctrl;		/* 4a009508 */
	u32 pad127[1];				/* 4a00950c */
	u32 cm_l4per_gpio7_clkctrl;		/* 4a009510 */
	u32 pad1271[1];				/* 4a009514 */
	u32 cm_l4per_gpio8_clkctrl;		/* 4a009518 */
	u32 pad1272[1];				/* 4a00951c */
	u32 cm_l4per_mmcsd3_clkctrl;		/* 4a009520 */
	u32 pad128[1];				/* 4a009524 */
	u32 cm_l4per_mmcsd4_clkctrl;		/* 4a009528 */
	u32 pad129[1];				/* 4a00952c */
	u32 cm_l4per_msprohg_clkctrl;		/* 4a009530 */
	u32 pad130[1];				/* 4a009534 */
	u32 cm_l4per_slimbus2_clkctrl;		/* 4a009538 */
	u32 pad131[1];				/* 4a00953c */
	u32 cm_l4per_uart1_clkctrl;		/* 4a009540 */
	u32 pad132[1];				/* 4a009544 */
	u32 cm_l4per_uart2_clkctrl;		/* 4a009548 */
	u32 pad133[1];				/* 4a00954c */
	u32 cm_l4per_uart3_clkctrl;		/* 4a009550 */
	u32 pad134[1];				/* 4a009554 */
	u32 cm_l4per_uart4_clkctrl;		/* 4a009558 */
	u32 pad135[1];				/* 4a00955c */
	u32 cm_l4per_mmcsd5_clkctrl;		/* 4a009560 */
	u32 pad136[1];				/* 4a009564 */
	u32 cm_l4per_i2c5_clkctrl;		/* 4a009568 */
	u32 pad1371[1];				/* 4a00956c */
	u32 cm_l4per_uart5_clkctrl;		/* 4a009570 */
	u32 pad1372[1];				/* 4a009574 */
	u32 cm_l4per_uart6_clkctrl;		/* 4a009578 */
	u32 pad1374[1];				/* 4a00957c */
	u32 cm_l4sec_clkstctrl;			/* 4a009580 */
	u32 cm_l4sec_staticdep;			/* 4a009584 */
	u32 cm_l4sec_dynamicdep;		/* 4a009588 */
	u32 pad138[5];				/* 4a00958c */
	u32 cm_l4sec_aes1_clkctrl;		/* 4a0095a0 */
	u32 pad139[1];				/* 4a0095a4 */
	u32 cm_l4sec_aes2_clkctrl;		/* 4a0095a8 */
	u32 pad140[1];				/* 4a0095ac */
	u32 cm_l4sec_des3des_clkctrl;		/* 4a0095b0 */
	u32 pad141[1];				/* 4a0095b4 */
	u32 cm_l4sec_pkaeip29_clkctrl;		/* 4a0095b8 */
	u32 pad142[1];				/* 4a0095bc */
	u32 cm_l4sec_rng_clkctrl;		/* 4a0095c0 */
	u32 pad143[1];				/* 4a0095c4 */
	u32 cm_l4sec_sha2md51_clkctrl;		/* 4a0095c8 */
	u32 pad144[3];				/* 4a0095cc */
	u32 cm_l4sec_cryptodma_clkctrl;		/* 4a0095d8 */
	u32 pad145[3660425];			/* 4a0095dc */

	/* l4 wkup regs */
	u32 pad201[6211];			/* 4ae00000 */
	u32 cm_abe_pll_ref_clksel;		/* 4ae0610c */
	u32 cm_sys_clksel;			/* 4ae06110 */
	u32 pad202[1467];			/* 4ae06114 */
	u32 cm_wkup_clkstctrl;			/* 4ae07800 */
	u32 pad203[7];				/* 4ae07804 */
	u32 cm_wkup_l4wkup_clkctrl;		/* 4ae07820 */
	u32 pad204;				/* 4ae07824 */
	u32 cm_wkup_wdtimer1_clkctrl;		/* 4ae07828 */
	u32 pad205;				/* 4ae0782c */
	u32 cm_wkup_wdtimer2_clkctrl;		/* 4ae07830 */
	u32 pad206;				/* 4ae07834 */
	u32 cm_wkup_gpio1_clkctrl;		/* 4ae07838 */
	u32 pad207;				/* 4ae0783c */
	u32 cm_wkup_gptimer1_clkctrl;		/* 4ae07840 */
	u32 pad208;				/* 4ae07844 */
	u32 cm_wkup_gptimer12_clkctrl;		/* 4ae07848 */
	u32 pad209;				/* 4ae0784c */
	u32 cm_wkup_synctimer_clkctrl;		/* 4ae07850 */
	u32 pad210;				/* 4ae07854 */
	u32 cm_wkup_usim_clkctrl;		/* 4ae07858 */
	u32 pad211;				/* 4ae0785c */
	u32 cm_wkup_sarram_clkctrl;		/* 4ae07860 */
	u32 pad212[5];				/* 4ae07864 */
	u32 cm_wkup_keyboard_clkctrl;		/* 4ae07878 */
	u32 pad213;				/* 4ae0787c */
	u32 cm_wkup_rtc_clkctrl;		/* 4ae07880 */
	u32 pad214;				/* 4ae07884 */
	u32 cm_wkup_bandgap_clkctrl;		/* 4ae07888 */
	u32 pad215[1];				/* 4ae0788c */
	u32 cm_wkupaon_scrm_clkctrl;		/* 4ae07890 */
	u32 pad216[195];
	u32 prm_vc_val_bypass;			/* 4ae07ba0 */
	u32 pad217[4];
	u32 prm_vc_cfg_i2c_mode;		/* 4ae07bb4 */
	u32 prm_vc_cfg_i2c_clk;			/* 4ae07bb8 */
	u32 pad218[2];
	u32 prm_sldo_core_setup;		/* 4ae07bc4 */
	u32 prm_sldo_core_ctrl;			/* 4ae07bc8 */
	u32 prm_sldo_mpu_setup;			/* 4ae07bcc */
	u32 prm_sldo_mpu_ctrl;			/* 4ae07bd0 */
	u32 prm_sldo_mm_setup;			/* 4ae07bd4 */
	u32 prm_sldo_mm_ctrl;			/* 4ae07bd8 */
};

/* DPLL register offsets */
#define CM_CLKMODE_DPLL		0
#define CM_IDLEST_DPLL		0x4
#define CM_AUTOIDLE_DPLL	0x8
#define CM_CLKSEL_DPLL		0xC

#define DPLL_CLKOUT_DIV_MASK	0x1F /* post-divider mask */

/* CM_DLL_CTRL */
#define CM_DLL_CTRL_OVERRIDE_SHIFT		0
#define CM_DLL_CTRL_OVERRIDE_MASK		(1 << 0)
#define CM_DLL_CTRL_NO_OVERRIDE			0

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

/* SGX */
#define CLKSEL_GPU_HYD_GCLK_MASK		(1 << 25)
#define CLKSEL_GPU_CORE_GCLK_MASK		(1 << 24)

/* CM_CLKSEL_DPLL */
#define CM_CLKSEL_DPLL_DPLL_SD_DIV_SHIFT	24
#define CM_CLKSEL_DPLL_DPLL_SD_DIV_MASK		(0xFF << 24)
#define CM_CLKSEL_DPLL_M_SHIFT			8
#define CM_CLKSEL_DPLL_M_MASK			(0x7FF << 8)
#define CM_CLKSEL_DPLL_N_SHIFT			0
#define CM_CLKSEL_DPLL_N_MASK			0x7F
#define CM_CLKSEL_DCC_EN_SHIFT			22
#define CM_CLKSEL_DCC_EN_MASK			(1 << 22)

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
#define HSMMC_CLKCTRL_CLKSEL_DIV_MASK		(1 << 25)

/* CM_WKUP_GPTIMER1_CLKCTRL */
#define GPTIMER1_CLKCTRL_CLKSEL_MASK		(1 << 24)

/* CM_CAM_ISS_CLKCTRL */
#define ISS_CLKCTRL_OPTFCLKEN_MASK		(1 << 8)

/* CM_DSS_DSS_CLKCTRL */
#define DSS_CLKCTRL_OPTFCLKEN_MASK		0xF00

/* CM_L3INIT_USBPHY_CLKCTRL */
#define USBPHY_CLKCTRL_OPTFCLKEN_PHY_48M_MASK	8

/* CM_MPU_MPU_CLKCTRL */
#define MPU_CLKCTRL_CLKSEL_EMIF_DIV_MODE_SHIFT	24
#define MPU_CLKCTRL_CLKSEL_EMIF_DIV_MODE_MASK	(1 << 24)
#define MPU_CLKCTRL_CLKSEL_ABE_DIV_MODE_SHIFT	25
#define MPU_CLKCTRL_CLKSEL_ABE_DIV_MODE_MASK	(1 << 25)

/* CM_WKUPAON_SCRM_CLKCTRL */
#define OPTFCLKEN_SCRM_PER_SHIFT		9
#define OPTFCLKEN_SCRM_PER_MASK			(1 << 9)
#define OPTFCLKEN_SCRM_CORE_SHIFT		8
#define OPTFCLKEN_SCRM_CORE_MASK		(1 << 8)

/* Clock frequencies */
#define OMAP_SYS_CLK_FREQ_38_4_MHZ	38400000
#define OMAP_SYS_CLK_IND_38_4_MHZ	6
#define OMAP_32K_CLK_FREQ		32768

/* PRM_VC_VAL_BYPASS */
#define PRM_VC_I2C_CHANNEL_FREQ_KHZ	400

/* SMPS */
#define SMPS_I2C_SLAVE_ADDR	0x12
#define SMPS_REG_ADDR_12_MPU	0x23
#define SMPS_REG_ADDR_45_IVA	0x2B
#define SMPS_REG_ADDR_8_CORE	0x37

/* PALMAS VOLTAGE SETTINGS in mv for OPP_NOMINAL */
#define VDD_MPU		1000
#define VDD_MM		1000
#define VDD_CORE	1040
#define VDD_MPU_5432	1150
#define VDD_MM_5432	1150
#define VDD_CORE_5432	1150

/* Standard offset is 0.5v expressed in uv */
#define PALMAS_SMPS_BASE_VOLT_UV 500000

/* TPS */
#define TPS62361_I2C_SLAVE_ADDR		0x60
#define TPS62361_REG_ADDR_SET0		0x0
#define TPS62361_REG_ADDR_SET1		0x1
#define TPS62361_REG_ADDR_SET2		0x2
#define TPS62361_REG_ADDR_SET3		0x3
#define TPS62361_REG_ADDR_CTRL		0x4
#define TPS62361_REG_ADDR_TEMP		0x5
#define TPS62361_REG_ADDR_RMP_CTRL	0x6
#define TPS62361_REG_ADDR_CHIP_ID	0x8
#define TPS62361_REG_ADDR_CHIP_ID_2	0x9

#define TPS62361_BASE_VOLT_MV	500
#define TPS62361_VSEL0_GPIO	7

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
	u32 cm_div_h11_dpll;
	u32 cm_div_h12_dpll;
	u32 cm_div_h13_dpll;
	u32 cm_div_h14_dpll;
	u32 reserved[3];
	u32 cm_div_h22_dpll;
	u32 cm_div_h23_dpll;
};

/* DPLL parameter table */
struct dpll_params {
	u32 m;
	u32 n;
	s8 m2;
	s8 m3;
	s8 h11;
	s8 h12;
	s8 h13;
	s8 h14;
	s8 h22;
	s8 h23;
};

extern struct omap5_prcm_regs *const prcm;
extern const u32 sys_clk_array[8];

void scale_vcores(void);
void do_scale_tps62361(int gpio, u32 reg, u32 volt_mv);
u32 get_offset_code(u32 offset);
u32 omap_ddr_clk(void);
void do_scale_vcore(u32 vcore_reg, u32 volt_mv);
void setup_post_dividers(u32 *const base, const struct dpll_params *params);
u32 get_sys_clk_index(void);
void enable_basic_clocks(void);
void enable_non_essential_clocks(void);
void enable_basic_uboot_clocks(void);
void do_enable_clocks(u32 *const *clk_domains,
		      u32 *const *clk_modules_hw_auto,
		      u32 *const *clk_modules_explicit_en,
		      u8 wait_for_enable);
const struct dpll_params *get_mpu_dpll_params(void);
const struct dpll_params *get_core_dpll_params(void);
const struct dpll_params *get_per_dpll_params(void);
const struct dpll_params *get_iva_dpll_params(void);
const struct dpll_params *get_usb_dpll_params(void);
const struct dpll_params *get_abe_dpll_params(void);
#endif /* _CLOCKS_OMAP5_H_ */
