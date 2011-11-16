/*
 * EMIF programming
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com> for OMAP4
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

#include <common.h>
#include <asm/emif.h>
#include <asm/arch/sys_proto.h>
#include <asm/utils.h>

#ifndef CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS
#define print_timing_reg(reg) debug(#reg" - 0x%08x\n", (reg))
static u32 *const T_num = (u32 *)OMAP5_SRAM_SCRATCH_EMIF_T_NUM;
static u32 *const T_den = (u32 *)OMAP5_SRAM_SCRATCH_EMIF_T_DEN;
static u32 *const emif_sizes = (u32 *)OMAP5_SRAM_SCRATCH_EMIF_SIZE;
#endif

#ifdef CONFIG_SYS_DEFAULT_LPDDR2_TIMINGS
/* Base AC Timing values specified by JESD209-2 for 532MHz operation */
static const struct lpddr2_ac_timings timings_jedec_532_mhz = {
	.max_freq = 532000000,
	.RL = 8,
	.tRPab = 21,
	.tRCD = 18,
	.tWR = 15,
	.tRASmin = 42,
	.tRRD = 10,
	.tWTRx2 = 15,
	.tXSR = 140,
	.tXPx2 = 15,
	.tRFCab = 130,
	.tRTPx2 = 15,
	.tCKE = 3,
	.tCKESR = 15,
	.tZQCS = 90,
	.tZQCL = 360,
	.tZQINIT = 1000,
	.tDQSCKMAXx2 = 11,
	.tRASmax = 70,
	.tFAW = 50
};

/*
 * Min tCK values specified by JESD209-2
 * Min tCK specifies the minimum duration of some AC timing parameters in terms
 * of the number of cycles. If the calculated number of cycles based on the
 * absolute time value is less than the min tCK value, min tCK value should
 * be used instead. This typically happens at low frequencies.
 */
static const struct lpddr2_min_tck min_tck_jedec = {
	.tRL = 3,
	.tRP_AB = 3,
	.tRCD = 3,
	.tWR = 3,
	.tRAS_MIN = 3,
	.tRRD = 2,
	.tWTR = 2,
	.tXP = 2,
	.tRTP = 2,
	.tCKE = 3,
	.tCKESR = 3,
	.tFAW = 8
};

static const struct lpddr2_ac_timings const*
			jedec_ac_timings[MAX_NUM_SPEEDBINS] = {
	&timings_jedec_532_mhz
};

static const struct lpddr2_device_timings jedec_default_timings = {
	.ac_timings = jedec_ac_timings,
	.min_tck = &min_tck_jedec
};

void emif_get_device_timings(u32 emif_nr,
		const struct lpddr2_device_timings **cs0_device_timings,
		const struct lpddr2_device_timings **cs1_device_timings)
{
	/* Assume Identical devices on EMIF1 & EMIF2 */
	*cs0_device_timings = &jedec_default_timings;
	*cs1_device_timings = NULL;
}
#endif /* CONFIG_SYS_DEFAULT_LPDDR2_TIMINGS */
