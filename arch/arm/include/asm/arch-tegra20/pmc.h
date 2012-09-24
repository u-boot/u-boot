/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
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

#ifndef _PMC_H_
#define _PMC_H_

/* Power Management Controller (APBDEV_PMC_) registers */
struct pmc_ctlr {
	uint pmc_cntrl;			/* _CNTRL_0, offset 00 */
	uint pmc_sec_disable;		/* _SEC_DISABLE_0, offset 04 */
	uint pmc_pmc_swrst;		/* _PMC_SWRST_0, offset 08 */
	uint pmc_wake_mask;		/* _WAKE_MASK_0, offset 0C */
	uint pmc_wake_lvl;		/* _WAKE_LVL_0, offset 10 */
	uint pmc_wake_status;		/* _WAKE_STATUS_0, offset 14 */
	uint pmc_sw_wake_status;	/* _SW_WAKE_STATUS_0, offset 18 */
	uint pmc_dpd_pads_oride;	/* _DPD_PADS_ORIDE_0, offset 1C */
	uint pmc_dpd_sample;		/* _DPD_PADS_SAMPLE_0, offset 20 */
	uint pmc_dpd_enable;		/* _DPD_PADS_ENABLE_0, offset 24 */
	uint pmc_pwrgate_timer_off;	/* _PWRGATE_TIMER_OFF_0, offset 28 */
	uint pmc_pwrgate_timer_on;	/* _PWRGATE_TIMER_ON_0, offset 2C */
	uint pmc_pwrgate_toggle;	/* _PWRGATE_TOGGLE_0, offset 30 */
	uint pmc_remove_clamping;	/* _REMOVE_CLAMPING_CMD_0, offset 34 */
	uint pmc_pwrgate_status;	/* _PWRGATE_STATUS_0, offset 38 */
	uint pmc_pwrgood_timer;		/* _PWRGOOD_TIMER_0, offset 3C */
	uint pmc_blink_timer;		/* _BLINK_TIMER_0, offset 40 */
	uint pmc_no_iopower;		/* _NO_IOPOWER_0, offset 44 */
	uint pmc_pwr_det;		/* _PWR_DET_0, offset 48 */
	uint pmc_pwr_det_latch;		/* _PWR_DET_LATCH_0, offset 4C */

	uint pmc_scratch0;		/* _SCRATCH0_0, offset 50 */
	uint pmc_scratch1;		/* _SCRATCH1_0, offset 54 */
	uint pmc_scratch2;		/* _SCRATCH2_0, offset 58 */
	uint pmc_scratch3;		/* _SCRATCH3_0, offset 5C */
	uint pmc_scratch4;		/* _SCRATCH4_0, offset 60 */
	uint pmc_scratch5;		/* _SCRATCH5_0, offset 64 */
	uint pmc_scratch6;		/* _SCRATCH6_0, offset 68 */
	uint pmc_scratch7;		/* _SCRATCH7_0, offset 6C */
	uint pmc_scratch8;		/* _SCRATCH8_0, offset 70 */
	uint pmc_scratch9;		/* _SCRATCH9_0, offset 74 */
	uint pmc_scratch10;		/* _SCRATCH10_0, offset 78 */
	uint pmc_scratch11;		/* _SCRATCH11_0, offset 7C */
	uint pmc_scratch12;		/* _SCRATCH12_0, offset 80 */
	uint pmc_scratch13;		/* _SCRATCH13_0, offset 84 */
	uint pmc_scratch14;		/* _SCRATCH14_0, offset 88 */
	uint pmc_scratch15;		/* _SCRATCH15_0, offset 8C */
	uint pmc_scratch16;		/* _SCRATCH16_0, offset 90 */
	uint pmc_scratch17;		/* _SCRATCH17_0, offset 94 */
	uint pmc_scratch18;		/* _SCRATCH18_0, offset 98 */
	uint pmc_scratch19;		/* _SCRATCH19_0, offset 9C */
	uint pmc_scratch20;		/* _SCRATCH20_0, offset A0 */
	uint pmc_scratch21;		/* _SCRATCH21_0, offset A4 */
	uint pmc_scratch22;		/* _SCRATCH22_0, offset A8 */
	uint pmc_scratch23;		/* _SCRATCH23_0, offset AC */

	uint pmc_secure_scratch0;	/* _SECURE_SCRATCH0_0, offset B0 */
	uint pmc_secure_scratch1;	/* _SECURE_SCRATCH1_0, offset B4 */
	uint pmc_secure_scratch2;	/* _SECURE_SCRATCH2_0, offset B8 */
	uint pmc_secure_scratch3;	/* _SECURE_SCRATCH3_0, offset BC */
	uint pmc_secure_scratch4;	/* _SECURE_SCRATCH4_0, offset C0 */
	uint pmc_secure_scratch5;	/* _SECURE_SCRATCH5_0, offset C4 */

	uint pmc_cpupwrgood_timer;	/* _CPUPWRGOOD_TIMER_0, offset C8 */
	uint pmc_cpupwroff_timer;	/* _CPUPWROFF_TIMER_0, offset CC */
	uint pmc_pg_mask;		/* _PG_MASK_0, offset D0 */
	uint pmc_pg_mask_1;		/* _PG_MASK_1_0, offset D4 */
	uint pmc_auto_wake_lvl;		/* _AUTO_WAKE_LVL_0, offset D8 */
	uint pmc_auto_wake_lvl_mask;	/* _AUTO_WAKE_LVL_MASK_0, offset DC */
	uint pmc_wake_delay;		/* _WAKE_DELAY_0, offset E0 */
	uint pmc_pwr_det_val;		/* _PWR_DET_VAL_0, offset E4 */
	uint pmc_ddr_pwr;		/* _DDR_PWR_0, offset E8 */
	uint pmc_usb_debounce_del;	/* _USB_DEBOUNCE_DEL_0, offset EC */
	uint pmc_usb_ao;		/* _USB_AO_0, offset F0 */
	uint pmc_crypto_op;		/* _CRYPTO_OP__0, offset F4 */
	uint pmc_pllp_wb0_override;	/* _PLLP_WB0_OVERRIDE_0, offset F8 */

	uint pmc_scratch24;		/* _SCRATCH24_0, offset FC */
	uint pmc_scratch25;		/* _SCRATCH24_0, offset 100 */
	uint pmc_scratch26;		/* _SCRATCH24_0, offset 104 */
	uint pmc_scratch27;		/* _SCRATCH24_0, offset 108 */
	uint pmc_scratch28;		/* _SCRATCH24_0, offset 10C */
	uint pmc_scratch29;		/* _SCRATCH24_0, offset 110 */
	uint pmc_scratch30;		/* _SCRATCH24_0, offset 114 */
	uint pmc_scratch31;		/* _SCRATCH24_0, offset 118 */
	uint pmc_scratch32;		/* _SCRATCH24_0, offset 11C */
	uint pmc_scratch33;		/* _SCRATCH24_0, offset 120 */
	uint pmc_scratch34;		/* _SCRATCH24_0, offset 124 */
	uint pmc_scratch35;		/* _SCRATCH24_0, offset 128 */
	uint pmc_scratch36;		/* _SCRATCH24_0, offset 12C */
	uint pmc_scratch37;		/* _SCRATCH24_0, offset 130 */
	uint pmc_scratch38;		/* _SCRATCH24_0, offset 134 */
	uint pmc_scratch39;		/* _SCRATCH24_0, offset 138 */
	uint pmc_scratch40;		/* _SCRATCH24_0, offset 13C */
	uint pmc_scratch41;		/* _SCRATCH24_0, offset 140 */
	uint pmc_scratch42;		/* _SCRATCH24_0, offset 144 */

	uint pmc_bo_mirror0;		/* _BOUNDOUT_MIRROR0_0, offset 148 */
	uint pmc_bo_mirror1;		/* _BOUNDOUT_MIRROR1_0, offset 14C */
	uint pmc_bo_mirror2;		/* _BOUNDOUT_MIRROR2_0, offset 150 */
	uint pmc_sys_33v_en;		/* _SYS_33V_EN_0, offset 154 */
	uint pmc_bo_mirror_access;	/* _BOUNDOUT_MIRROR_ACCESS_0, off158 */
	uint pmc_gate;			/* _GATE_0, offset 15C */
};

#define CPU_PWRED	1
#define CPU_CLMP	1

#define PARTID_CP	0xFFFFFFF8
#define START_CP	(1 << 8)

#define CPUPWRREQ_OE	(1 << 16)

#endif	/* PMC_H */
