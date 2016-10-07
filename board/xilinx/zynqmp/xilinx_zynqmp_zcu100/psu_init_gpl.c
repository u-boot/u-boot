/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, see <http://www.gnu.org/licenses/>
*
*
******************************************************************************/

#include <xil_io.h>
#include "psu_init_gpl.h"

static void PSU_Mask_Write (unsigned long offset, unsigned long mask, unsigned long val)
{
	unsigned long RegVal = 0x0;
	RegVal = Xil_In32 (offset);
	RegVal &= ~(mask);
	RegVal |= (val & mask);
	Xil_Out32 (offset, RegVal);
}

unsigned long psu_pll_init_data() {
		// : RPLL INIT
		/*Register : RPLL_CFG @ 0XFF5E0034</p>

		PLL loop filter resistor control
		PSU_CRL_APB_RPLL_CFG_RES                                                        0x2

		PLL charge pump control
		PSU_CRL_APB_RPLL_CFG_CP                                                         0x3

		PLL loop filter high frequency capacitor control
		PSU_CRL_APB_RPLL_CFG_LFHF                                                       0x3

		Lock circuit counter setting
		PSU_CRL_APB_RPLL_CFG_LOCK_CNT                                                   0x258

		Lock circuit configuration settings for lock windowsize
		PSU_CRL_APB_RPLL_CFG_LOCK_DLY                                                   0x3f

		Helper data. Values are to be looked up in a table from Data Sheet
		(OFFSET, MASK, VALUE)      (0XFF5E0034, 0xFE7FEDEFU ,0x7E4B0C62U)
		RegMask = (CRL_APB_RPLL_CFG_RES_MASK | CRL_APB_RPLL_CFG_CP_MASK | CRL_APB_RPLL_CFG_LFHF_MASK | CRL_APB_RPLL_CFG_LOCK_CNT_MASK | CRL_APB_RPLL_CFG_LOCK_DLY_MASK |  0 );

		RegVal = ((0x00000002U << CRL_APB_RPLL_CFG_RES_SHIFT
			| 0x00000003U << CRL_APB_RPLL_CFG_CP_SHIFT
			| 0x00000003U << CRL_APB_RPLL_CFG_LFHF_SHIFT
			| 0x00000258U << CRL_APB_RPLL_CFG_LOCK_CNT_SHIFT
			| 0x0000003FU << CRL_APB_RPLL_CFG_LOCK_DLY_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RPLL_CFG_OFFSET ,0xFE7FEDEFU ,0x7E4B0C62U);
	/*############################################################################################################################ */

		// : UPDATE FB_DIV
		/*Register : RPLL_CTRL @ 0XFF5E0030</p>

		Mux select for determining which clock feeds this PLL. 0XX pss_ref_clk is the source 100 video clk is the source 101 pss_alt_
		ef_clk is the source 110 aux_refclk[X] is the source 111 gt_crx_ref_clk is the source
		PSU_CRL_APB_RPLL_CTRL_PRE_SRC                                                   0x0

		The integer portion of the feedback divider to the PLL
		PSU_CRL_APB_RPLL_CTRL_FBDIV                                                     0x48

		This turns on the divide by 2 that is inside of the PLL. This does not change the VCO frequency, just the output frequency
		PSU_CRL_APB_RPLL_CTRL_DIV2                                                      0x1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFF5E0030, 0x00717F00U ,0x00014800U)
		RegMask = (CRL_APB_RPLL_CTRL_PRE_SRC_MASK | CRL_APB_RPLL_CTRL_FBDIV_MASK | CRL_APB_RPLL_CTRL_DIV2_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RPLL_CTRL_PRE_SRC_SHIFT
			| 0x00000048U << CRL_APB_RPLL_CTRL_FBDIV_SHIFT
			| 0x00000001U << CRL_APB_RPLL_CTRL_DIV2_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RPLL_CTRL_OFFSET ,0x00717F00U ,0x00014800U);
	/*############################################################################################################################ */

		// : BY PASS PLL
		/*Register : RPLL_CTRL @ 0XFF5E0030</p>

		Bypasses the PLL clock. The usable clock will be determined from the POST_SRC field. (This signal may only be toggled after 4
		cycles of the old clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_RPLL_CTRL_BYPASS                                                    1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFF5E0030, 0x00000008U ,0x00000008U)
		RegMask = (CRL_APB_RPLL_CTRL_BYPASS_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_RPLL_CTRL_BYPASS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RPLL_CTRL_OFFSET ,0x00000008U ,0x00000008U);
	/*############################################################################################################################ */

		// : ASSERT RESET
		/*Register : RPLL_CTRL @ 0XFF5E0030</p>

		Asserts Reset to the PLL. When asserting reset, the PLL must already be in BYPASS.
		PSU_CRL_APB_RPLL_CTRL_RESET                                                     1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFF5E0030, 0x00000001U ,0x00000001U)
		RegMask = (CRL_APB_RPLL_CTRL_RESET_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_RPLL_CTRL_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RPLL_CTRL_OFFSET ,0x00000001U ,0x00000001U);
	/*############################################################################################################################ */

		// : DEASSERT RESET
		/*Register : RPLL_CTRL @ 0XFF5E0030</p>

		Asserts Reset to the PLL. When asserting reset, the PLL must already be in BYPASS.
		PSU_CRL_APB_RPLL_CTRL_RESET                                                     0

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFF5E0030, 0x00000001U ,0x00000000U)
		RegMask = (CRL_APB_RPLL_CTRL_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RPLL_CTRL_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RPLL_CTRL_OFFSET ,0x00000001U ,0x00000000U);
	/*############################################################################################################################ */

		// : CHECK PLL STATUS
		/*Register : PLL_STATUS @ 0XFF5E0040</p>

		RPLL is locked
		PSU_CRL_APB_PLL_STATUS_RPLL_LOCK                                                1
		(OFFSET, MASK, VALUE)      (0XFF5E0040, 0x00000002U ,0x00000002U)  */
		mask_poll(CRL_APB_PLL_STATUS_OFFSET,0x00000002U);

	/*############################################################################################################################ */

		// : REMOVE PLL BY PASS
		/*Register : RPLL_CTRL @ 0XFF5E0030</p>

		Bypasses the PLL clock. The usable clock will be determined from the POST_SRC field. (This signal may only be toggled after 4
		cycles of the old clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_RPLL_CTRL_BYPASS                                                    0

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFF5E0030, 0x00000008U ,0x00000000U)
		RegMask = (CRL_APB_RPLL_CTRL_BYPASS_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RPLL_CTRL_BYPASS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RPLL_CTRL_OFFSET ,0x00000008U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : RPLL_TO_FPD_CTRL @ 0XFF5E0048</p>

		Divisor value for this clock.
		PSU_CRL_APB_RPLL_TO_FPD_CTRL_DIVISOR0                                           0x3

		Control for a clock that will be generated in the LPD, but used in the FPD as a clock source for the peripheral clock muxes.
		(OFFSET, MASK, VALUE)      (0XFF5E0048, 0x00003F00U ,0x00000300U)
		RegMask = (CRL_APB_RPLL_TO_FPD_CTRL_DIVISOR0_MASK |  0 );

		RegVal = ((0x00000003U << CRL_APB_RPLL_TO_FPD_CTRL_DIVISOR0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RPLL_TO_FPD_CTRL_OFFSET ,0x00003F00U ,0x00000300U);
	/*############################################################################################################################ */

		// : RPLL FRAC CFG
		/*Register : RPLL_FRAC_CFG @ 0XFF5E0038</p>

		Fractional SDM bypass control. When 0, PLL is in integer mode and it ignores all fractional data. When 1, PLL is in fractiona
		 mode and uses DATA of this register for the fractional portion of the feedback divider.
		PSU_CRL_APB_RPLL_FRAC_CFG_ENABLED                                               0x0

		Fractional value for the Feedback value.
		PSU_CRL_APB_RPLL_FRAC_CFG_DATA                                                  0x0

		Fractional control for the PLL
		(OFFSET, MASK, VALUE)      (0XFF5E0038, 0x8000FFFFU ,0x00000000U)
		RegMask = (CRL_APB_RPLL_FRAC_CFG_ENABLED_MASK | CRL_APB_RPLL_FRAC_CFG_DATA_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RPLL_FRAC_CFG_ENABLED_SHIFT
			| 0x00000000U << CRL_APB_RPLL_FRAC_CFG_DATA_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RPLL_FRAC_CFG_OFFSET ,0x8000FFFFU ,0x00000000U);
	/*############################################################################################################################ */

		// : IOPLL INIT
		/*Register : IOPLL_CFG @ 0XFF5E0024</p>

		PLL loop filter resistor control
		PSU_CRL_APB_IOPLL_CFG_RES                                                       0xc

		PLL charge pump control
		PSU_CRL_APB_IOPLL_CFG_CP                                                        0x3

		PLL loop filter high frequency capacitor control
		PSU_CRL_APB_IOPLL_CFG_LFHF                                                      0x3

		Lock circuit counter setting
		PSU_CRL_APB_IOPLL_CFG_LOCK_CNT                                                  0x339

		Lock circuit configuration settings for lock windowsize
		PSU_CRL_APB_IOPLL_CFG_LOCK_DLY                                                  0x3f

		Helper data. Values are to be looked up in a table from Data Sheet
		(OFFSET, MASK, VALUE)      (0XFF5E0024, 0xFE7FEDEFU ,0x7E672C6CU)
		RegMask = (CRL_APB_IOPLL_CFG_RES_MASK | CRL_APB_IOPLL_CFG_CP_MASK | CRL_APB_IOPLL_CFG_LFHF_MASK | CRL_APB_IOPLL_CFG_LOCK_CNT_MASK | CRL_APB_IOPLL_CFG_LOCK_DLY_MASK |  0 );

		RegVal = ((0x0000000CU << CRL_APB_IOPLL_CFG_RES_SHIFT
			| 0x00000003U << CRL_APB_IOPLL_CFG_CP_SHIFT
			| 0x00000003U << CRL_APB_IOPLL_CFG_LFHF_SHIFT
			| 0x00000339U << CRL_APB_IOPLL_CFG_LOCK_CNT_SHIFT
			| 0x0000003FU << CRL_APB_IOPLL_CFG_LOCK_DLY_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_IOPLL_CFG_OFFSET ,0xFE7FEDEFU ,0x7E672C6CU);
	/*############################################################################################################################ */

		// : UPDATE FB_DIV
		/*Register : IOPLL_CTRL @ 0XFF5E0020</p>

		Mux select for determining which clock feeds this PLL. 0XX pss_ref_clk is the source 100 video clk is the source 101 pss_alt_
		ef_clk is the source 110 aux_refclk[X] is the source 111 gt_crx_ref_clk is the source
		PSU_CRL_APB_IOPLL_CTRL_PRE_SRC                                                  0x0

		The integer portion of the feedback divider to the PLL
		PSU_CRL_APB_IOPLL_CTRL_FBDIV                                                    0x2d

		This turns on the divide by 2 that is inside of the PLL. This does not change the VCO frequency, just the output frequency
		PSU_CRL_APB_IOPLL_CTRL_DIV2                                                     0x0

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFF5E0020, 0x00717F00U ,0x00002D00U)
		RegMask = (CRL_APB_IOPLL_CTRL_PRE_SRC_MASK | CRL_APB_IOPLL_CTRL_FBDIV_MASK | CRL_APB_IOPLL_CTRL_DIV2_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_IOPLL_CTRL_PRE_SRC_SHIFT
			| 0x0000002DU << CRL_APB_IOPLL_CTRL_FBDIV_SHIFT
			| 0x00000000U << CRL_APB_IOPLL_CTRL_DIV2_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_IOPLL_CTRL_OFFSET ,0x00717F00U ,0x00002D00U);
	/*############################################################################################################################ */

		// : BY PASS PLL
		/*Register : IOPLL_CTRL @ 0XFF5E0020</p>

		Bypasses the PLL clock. The usable clock will be determined from the POST_SRC field. (This signal may only be toggled after 4
		cycles of the old clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_IOPLL_CTRL_BYPASS                                                   1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFF5E0020, 0x00000008U ,0x00000008U)
		RegMask = (CRL_APB_IOPLL_CTRL_BYPASS_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_IOPLL_CTRL_BYPASS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_IOPLL_CTRL_OFFSET ,0x00000008U ,0x00000008U);
	/*############################################################################################################################ */

		// : ASSERT RESET
		/*Register : IOPLL_CTRL @ 0XFF5E0020</p>

		Asserts Reset to the PLL. When asserting reset, the PLL must already be in BYPASS.
		PSU_CRL_APB_IOPLL_CTRL_RESET                                                    1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFF5E0020, 0x00000001U ,0x00000001U)
		RegMask = (CRL_APB_IOPLL_CTRL_RESET_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_IOPLL_CTRL_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_IOPLL_CTRL_OFFSET ,0x00000001U ,0x00000001U);
	/*############################################################################################################################ */

		// : DEASSERT RESET
		/*Register : IOPLL_CTRL @ 0XFF5E0020</p>

		Asserts Reset to the PLL. When asserting reset, the PLL must already be in BYPASS.
		PSU_CRL_APB_IOPLL_CTRL_RESET                                                    0

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFF5E0020, 0x00000001U ,0x00000000U)
		RegMask = (CRL_APB_IOPLL_CTRL_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_IOPLL_CTRL_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_IOPLL_CTRL_OFFSET ,0x00000001U ,0x00000000U);
	/*############################################################################################################################ */

		// : CHECK PLL STATUS
		/*Register : PLL_STATUS @ 0XFF5E0040</p>

		IOPLL is locked
		PSU_CRL_APB_PLL_STATUS_IOPLL_LOCK                                               1
		(OFFSET, MASK, VALUE)      (0XFF5E0040, 0x00000001U ,0x00000001U)  */
		mask_poll(CRL_APB_PLL_STATUS_OFFSET,0x00000001U);

	/*############################################################################################################################ */

		// : REMOVE PLL BY PASS
		/*Register : IOPLL_CTRL @ 0XFF5E0020</p>

		Bypasses the PLL clock. The usable clock will be determined from the POST_SRC field. (This signal may only be toggled after 4
		cycles of the old clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_IOPLL_CTRL_BYPASS                                                   0

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFF5E0020, 0x00000008U ,0x00000000U)
		RegMask = (CRL_APB_IOPLL_CTRL_BYPASS_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_IOPLL_CTRL_BYPASS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_IOPLL_CTRL_OFFSET ,0x00000008U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : IOPLL_TO_FPD_CTRL @ 0XFF5E0044</p>

		Divisor value for this clock.
		PSU_CRL_APB_IOPLL_TO_FPD_CTRL_DIVISOR0                                          0x3

		Control for a clock that will be generated in the LPD, but used in the FPD as a clock source for the peripheral clock muxes.
		(OFFSET, MASK, VALUE)      (0XFF5E0044, 0x00003F00U ,0x00000300U)
		RegMask = (CRL_APB_IOPLL_TO_FPD_CTRL_DIVISOR0_MASK |  0 );

		RegVal = ((0x00000003U << CRL_APB_IOPLL_TO_FPD_CTRL_DIVISOR0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_IOPLL_TO_FPD_CTRL_OFFSET ,0x00003F00U ,0x00000300U);
	/*############################################################################################################################ */

		// : IOPLL FRAC CFG
		/*Register : IOPLL_FRAC_CFG @ 0XFF5E0028</p>

		Fractional SDM bypass control. When 0, PLL is in integer mode and it ignores all fractional data. When 1, PLL is in fractiona
		 mode and uses DATA of this register for the fractional portion of the feedback divider.
		PSU_CRL_APB_IOPLL_FRAC_CFG_ENABLED                                              0x0

		Fractional value for the Feedback value.
		PSU_CRL_APB_IOPLL_FRAC_CFG_DATA                                                 0x0

		Fractional control for the PLL
		(OFFSET, MASK, VALUE)      (0XFF5E0028, 0x8000FFFFU ,0x00000000U)
		RegMask = (CRL_APB_IOPLL_FRAC_CFG_ENABLED_MASK | CRL_APB_IOPLL_FRAC_CFG_DATA_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_IOPLL_FRAC_CFG_ENABLED_SHIFT
			| 0x00000000U << CRL_APB_IOPLL_FRAC_CFG_DATA_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_IOPLL_FRAC_CFG_OFFSET ,0x8000FFFFU ,0x00000000U);
	/*############################################################################################################################ */

		// : APU_PLL INIT
		/*Register : APLL_CFG @ 0XFD1A0024</p>

		PLL loop filter resistor control
		PSU_CRF_APB_APLL_CFG_RES                                                        0x2

		PLL charge pump control
		PSU_CRF_APB_APLL_CFG_CP                                                         0x3

		PLL loop filter high frequency capacitor control
		PSU_CRF_APB_APLL_CFG_LFHF                                                       0x3

		Lock circuit counter setting
		PSU_CRF_APB_APLL_CFG_LOCK_CNT                                                   0x258

		Lock circuit configuration settings for lock windowsize
		PSU_CRF_APB_APLL_CFG_LOCK_DLY                                                   0x3f

		Helper data. Values are to be looked up in a table from Data Sheet
		(OFFSET, MASK, VALUE)      (0XFD1A0024, 0xFE7FEDEFU ,0x7E4B0C62U)
		RegMask = (CRF_APB_APLL_CFG_RES_MASK | CRF_APB_APLL_CFG_CP_MASK | CRF_APB_APLL_CFG_LFHF_MASK | CRF_APB_APLL_CFG_LOCK_CNT_MASK | CRF_APB_APLL_CFG_LOCK_DLY_MASK |  0 );

		RegVal = ((0x00000002U << CRF_APB_APLL_CFG_RES_SHIFT
			| 0x00000003U << CRF_APB_APLL_CFG_CP_SHIFT
			| 0x00000003U << CRF_APB_APLL_CFG_LFHF_SHIFT
			| 0x00000258U << CRF_APB_APLL_CFG_LOCK_CNT_SHIFT
			| 0x0000003FU << CRF_APB_APLL_CFG_LOCK_DLY_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_APLL_CFG_OFFSET ,0xFE7FEDEFU ,0x7E4B0C62U);
	/*############################################################################################################################ */

		// : UPDATE FB_DIV
		/*Register : APLL_CTRL @ 0XFD1A0020</p>

		Mux select for determining which clock feeds this PLL. 0XX pss_ref_clk is the source 100 video clk is the source 101 pss_alt_
		ef_clk is the source 110 aux_refclk[X] is the source 111 gt_crx_ref_clk is the source
		PSU_CRF_APB_APLL_CTRL_PRE_SRC                                                   0x0

		The integer portion of the feedback divider to the PLL
		PSU_CRF_APB_APLL_CTRL_FBDIV                                                     0x42

		This turns on the divide by 2 that is inside of the PLL. This does not change the VCO frequency, just the output frequency
		PSU_CRF_APB_APLL_CTRL_DIV2                                                      0x1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A0020, 0x00717F00U ,0x00014200U)
		RegMask = (CRF_APB_APLL_CTRL_PRE_SRC_MASK | CRF_APB_APLL_CTRL_FBDIV_MASK | CRF_APB_APLL_CTRL_DIV2_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_APLL_CTRL_PRE_SRC_SHIFT
			| 0x00000042U << CRF_APB_APLL_CTRL_FBDIV_SHIFT
			| 0x00000001U << CRF_APB_APLL_CTRL_DIV2_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_APLL_CTRL_OFFSET ,0x00717F00U ,0x00014200U);
	/*############################################################################################################################ */

		// : BY PASS PLL
		/*Register : APLL_CTRL @ 0XFD1A0020</p>

		Bypasses the PLL clock. The usable clock will be determined from the POST_SRC field. (This signal may only be toggled after 4
		cycles of the old clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_APLL_CTRL_BYPASS                                                    1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A0020, 0x00000008U ,0x00000008U)
		RegMask = (CRF_APB_APLL_CTRL_BYPASS_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_APLL_CTRL_BYPASS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_APLL_CTRL_OFFSET ,0x00000008U ,0x00000008U);
	/*############################################################################################################################ */

		// : ASSERT RESET
		/*Register : APLL_CTRL @ 0XFD1A0020</p>

		Asserts Reset to the PLL. When asserting reset, the PLL must already be in BYPASS.
		PSU_CRF_APB_APLL_CTRL_RESET                                                     1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A0020, 0x00000001U ,0x00000001U)
		RegMask = (CRF_APB_APLL_CTRL_RESET_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_APLL_CTRL_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_APLL_CTRL_OFFSET ,0x00000001U ,0x00000001U);
	/*############################################################################################################################ */

		// : DEASSERT RESET
		/*Register : APLL_CTRL @ 0XFD1A0020</p>

		Asserts Reset to the PLL. When asserting reset, the PLL must already be in BYPASS.
		PSU_CRF_APB_APLL_CTRL_RESET                                                     0

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A0020, 0x00000001U ,0x00000000U)
		RegMask = (CRF_APB_APLL_CTRL_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_APLL_CTRL_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_APLL_CTRL_OFFSET ,0x00000001U ,0x00000000U);
	/*############################################################################################################################ */

		// : CHECK PLL STATUS
		/*Register : PLL_STATUS @ 0XFD1A0044</p>

		APLL is locked
		PSU_CRF_APB_PLL_STATUS_APLL_LOCK                                                1
		(OFFSET, MASK, VALUE)      (0XFD1A0044, 0x00000001U ,0x00000001U)  */
		mask_poll(CRF_APB_PLL_STATUS_OFFSET,0x00000001U);

	/*############################################################################################################################ */

		// : REMOVE PLL BY PASS
		/*Register : APLL_CTRL @ 0XFD1A0020</p>

		Bypasses the PLL clock. The usable clock will be determined from the POST_SRC field. (This signal may only be toggled after 4
		cycles of the old clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_APLL_CTRL_BYPASS                                                    0

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A0020, 0x00000008U ,0x00000000U)
		RegMask = (CRF_APB_APLL_CTRL_BYPASS_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_APLL_CTRL_BYPASS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_APLL_CTRL_OFFSET ,0x00000008U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : APLL_TO_LPD_CTRL @ 0XFD1A0048</p>

		Divisor value for this clock.
		PSU_CRF_APB_APLL_TO_LPD_CTRL_DIVISOR0                                           0x3

		Control for a clock that will be generated in the FPD, but used in the LPD as a clock source for the peripheral clock muxes.
		(OFFSET, MASK, VALUE)      (0XFD1A0048, 0x00003F00U ,0x00000300U)
		RegMask = (CRF_APB_APLL_TO_LPD_CTRL_DIVISOR0_MASK |  0 );

		RegVal = ((0x00000003U << CRF_APB_APLL_TO_LPD_CTRL_DIVISOR0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_APLL_TO_LPD_CTRL_OFFSET ,0x00003F00U ,0x00000300U);
	/*############################################################################################################################ */

		// : APLL FRAC CFG
		/*Register : APLL_FRAC_CFG @ 0XFD1A0028</p>

		Fractional SDM bypass control. When 0, PLL is in integer mode and it ignores all fractional data. When 1, PLL is in fractiona
		 mode and uses DATA of this register for the fractional portion of the feedback divider.
		PSU_CRF_APB_APLL_FRAC_CFG_ENABLED                                               0x0

		Fractional value for the Feedback value.
		PSU_CRF_APB_APLL_FRAC_CFG_DATA                                                  0x0

		Fractional control for the PLL
		(OFFSET, MASK, VALUE)      (0XFD1A0028, 0x8000FFFFU ,0x00000000U)
		RegMask = (CRF_APB_APLL_FRAC_CFG_ENABLED_MASK | CRF_APB_APLL_FRAC_CFG_DATA_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_APLL_FRAC_CFG_ENABLED_SHIFT
			| 0x00000000U << CRF_APB_APLL_FRAC_CFG_DATA_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_APLL_FRAC_CFG_OFFSET ,0x8000FFFFU ,0x00000000U);
	/*############################################################################################################################ */

		// : DDR_PLL INIT
		/*Register : DPLL_CFG @ 0XFD1A0030</p>

		PLL loop filter resistor control
		PSU_CRF_APB_DPLL_CFG_RES                                                        0x2

		PLL charge pump control
		PSU_CRF_APB_DPLL_CFG_CP                                                         0x3

		PLL loop filter high frequency capacitor control
		PSU_CRF_APB_DPLL_CFG_LFHF                                                       0x3

		Lock circuit counter setting
		PSU_CRF_APB_DPLL_CFG_LOCK_CNT                                                   0x258

		Lock circuit configuration settings for lock windowsize
		PSU_CRF_APB_DPLL_CFG_LOCK_DLY                                                   0x3f

		Helper data. Values are to be looked up in a table from Data Sheet
		(OFFSET, MASK, VALUE)      (0XFD1A0030, 0xFE7FEDEFU ,0x7E4B0C62U)
		RegMask = (CRF_APB_DPLL_CFG_RES_MASK | CRF_APB_DPLL_CFG_CP_MASK | CRF_APB_DPLL_CFG_LFHF_MASK | CRF_APB_DPLL_CFG_LOCK_CNT_MASK | CRF_APB_DPLL_CFG_LOCK_DLY_MASK |  0 );

		RegVal = ((0x00000002U << CRF_APB_DPLL_CFG_RES_SHIFT
			| 0x00000003U << CRF_APB_DPLL_CFG_CP_SHIFT
			| 0x00000003U << CRF_APB_DPLL_CFG_LFHF_SHIFT
			| 0x00000258U << CRF_APB_DPLL_CFG_LOCK_CNT_SHIFT
			| 0x0000003FU << CRF_APB_DPLL_CFG_LOCK_DLY_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DPLL_CFG_OFFSET ,0xFE7FEDEFU ,0x7E4B0C62U);
	/*############################################################################################################################ */

		// : UPDATE FB_DIV
		/*Register : DPLL_CTRL @ 0XFD1A002C</p>

		Mux select for determining which clock feeds this PLL. 0XX pss_ref_clk is the source 100 video clk is the source 101 pss_alt_
		ef_clk is the source 110 aux_refclk[X] is the source 111 gt_crx_ref_clk is the source
		PSU_CRF_APB_DPLL_CTRL_PRE_SRC                                                   0x0

		The integer portion of the feedback divider to the PLL
		PSU_CRF_APB_DPLL_CTRL_FBDIV                                                     0x40

		This turns on the divide by 2 that is inside of the PLL. This does not change the VCO frequency, just the output frequency
		PSU_CRF_APB_DPLL_CTRL_DIV2                                                      0x1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A002C, 0x00717F00U ,0x00014000U)
		RegMask = (CRF_APB_DPLL_CTRL_PRE_SRC_MASK | CRF_APB_DPLL_CTRL_FBDIV_MASK | CRF_APB_DPLL_CTRL_DIV2_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_DPLL_CTRL_PRE_SRC_SHIFT
			| 0x00000040U << CRF_APB_DPLL_CTRL_FBDIV_SHIFT
			| 0x00000001U << CRF_APB_DPLL_CTRL_DIV2_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DPLL_CTRL_OFFSET ,0x00717F00U ,0x00014000U);
	/*############################################################################################################################ */

		// : BY PASS PLL
		/*Register : DPLL_CTRL @ 0XFD1A002C</p>

		Bypasses the PLL clock. The usable clock will be determined from the POST_SRC field. (This signal may only be toggled after 4
		cycles of the old clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_DPLL_CTRL_BYPASS                                                    1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A002C, 0x00000008U ,0x00000008U)
		RegMask = (CRF_APB_DPLL_CTRL_BYPASS_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_DPLL_CTRL_BYPASS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DPLL_CTRL_OFFSET ,0x00000008U ,0x00000008U);
	/*############################################################################################################################ */

		// : ASSERT RESET
		/*Register : DPLL_CTRL @ 0XFD1A002C</p>

		Asserts Reset to the PLL. When asserting reset, the PLL must already be in BYPASS.
		PSU_CRF_APB_DPLL_CTRL_RESET                                                     1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A002C, 0x00000001U ,0x00000001U)
		RegMask = (CRF_APB_DPLL_CTRL_RESET_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_DPLL_CTRL_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DPLL_CTRL_OFFSET ,0x00000001U ,0x00000001U);
	/*############################################################################################################################ */

		// : DEASSERT RESET
		/*Register : DPLL_CTRL @ 0XFD1A002C</p>

		Asserts Reset to the PLL. When asserting reset, the PLL must already be in BYPASS.
		PSU_CRF_APB_DPLL_CTRL_RESET                                                     0

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A002C, 0x00000001U ,0x00000000U)
		RegMask = (CRF_APB_DPLL_CTRL_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_DPLL_CTRL_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DPLL_CTRL_OFFSET ,0x00000001U ,0x00000000U);
	/*############################################################################################################################ */

		// : CHECK PLL STATUS
		/*Register : PLL_STATUS @ 0XFD1A0044</p>

		DPLL is locked
		PSU_CRF_APB_PLL_STATUS_DPLL_LOCK                                                1
		(OFFSET, MASK, VALUE)      (0XFD1A0044, 0x00000002U ,0x00000002U)  */
		mask_poll(CRF_APB_PLL_STATUS_OFFSET,0x00000002U);

	/*############################################################################################################################ */

		// : REMOVE PLL BY PASS
		/*Register : DPLL_CTRL @ 0XFD1A002C</p>

		Bypasses the PLL clock. The usable clock will be determined from the POST_SRC field. (This signal may only be toggled after 4
		cycles of the old clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_DPLL_CTRL_BYPASS                                                    0

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A002C, 0x00000008U ,0x00000000U)
		RegMask = (CRF_APB_DPLL_CTRL_BYPASS_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_DPLL_CTRL_BYPASS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DPLL_CTRL_OFFSET ,0x00000008U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : DPLL_TO_LPD_CTRL @ 0XFD1A004C</p>

		Divisor value for this clock.
		PSU_CRF_APB_DPLL_TO_LPD_CTRL_DIVISOR0                                           0x3

		Control for a clock that will be generated in the FPD, but used in the LPD as a clock source for the peripheral clock muxes.
		(OFFSET, MASK, VALUE)      (0XFD1A004C, 0x00003F00U ,0x00000300U)
		RegMask = (CRF_APB_DPLL_TO_LPD_CTRL_DIVISOR0_MASK |  0 );

		RegVal = ((0x00000003U << CRF_APB_DPLL_TO_LPD_CTRL_DIVISOR0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DPLL_TO_LPD_CTRL_OFFSET ,0x00003F00U ,0x00000300U);
	/*############################################################################################################################ */

		// : DPLL FRAC CFG
		/*Register : DPLL_FRAC_CFG @ 0XFD1A0034</p>

		Fractional SDM bypass control. When 0, PLL is in integer mode and it ignores all fractional data. When 1, PLL is in fractiona
		 mode and uses DATA of this register for the fractional portion of the feedback divider.
		PSU_CRF_APB_DPLL_FRAC_CFG_ENABLED                                               0x0

		Fractional value for the Feedback value.
		PSU_CRF_APB_DPLL_FRAC_CFG_DATA                                                  0x0

		Fractional control for the PLL
		(OFFSET, MASK, VALUE)      (0XFD1A0034, 0x8000FFFFU ,0x00000000U)
		RegMask = (CRF_APB_DPLL_FRAC_CFG_ENABLED_MASK | CRF_APB_DPLL_FRAC_CFG_DATA_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_DPLL_FRAC_CFG_ENABLED_SHIFT
			| 0x00000000U << CRF_APB_DPLL_FRAC_CFG_DATA_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DPLL_FRAC_CFG_OFFSET ,0x8000FFFFU ,0x00000000U);
	/*############################################################################################################################ */

		// : VIDEO_PLL INIT
		/*Register : VPLL_CFG @ 0XFD1A003C</p>

		PLL loop filter resistor control
		PSU_CRF_APB_VPLL_CFG_RES                                                        0x2

		PLL charge pump control
		PSU_CRF_APB_VPLL_CFG_CP                                                         0x3

		PLL loop filter high frequency capacitor control
		PSU_CRF_APB_VPLL_CFG_LFHF                                                       0x3

		Lock circuit counter setting
		PSU_CRF_APB_VPLL_CFG_LOCK_CNT                                                   0x28a

		Lock circuit configuration settings for lock windowsize
		PSU_CRF_APB_VPLL_CFG_LOCK_DLY                                                   0x3f

		Helper data. Values are to be looked up in a table from Data Sheet
		(OFFSET, MASK, VALUE)      (0XFD1A003C, 0xFE7FEDEFU ,0x7E514C62U)
		RegMask = (CRF_APB_VPLL_CFG_RES_MASK | CRF_APB_VPLL_CFG_CP_MASK | CRF_APB_VPLL_CFG_LFHF_MASK | CRF_APB_VPLL_CFG_LOCK_CNT_MASK | CRF_APB_VPLL_CFG_LOCK_DLY_MASK |  0 );

		RegVal = ((0x00000002U << CRF_APB_VPLL_CFG_RES_SHIFT
			| 0x00000003U << CRF_APB_VPLL_CFG_CP_SHIFT
			| 0x00000003U << CRF_APB_VPLL_CFG_LFHF_SHIFT
			| 0x0000028AU << CRF_APB_VPLL_CFG_LOCK_CNT_SHIFT
			| 0x0000003FU << CRF_APB_VPLL_CFG_LOCK_DLY_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_VPLL_CFG_OFFSET ,0xFE7FEDEFU ,0x7E514C62U);
	/*############################################################################################################################ */

		// : UPDATE FB_DIV
		/*Register : VPLL_CTRL @ 0XFD1A0038</p>

		Mux select for determining which clock feeds this PLL. 0XX pss_ref_clk is the source 100 video clk is the source 101 pss_alt_
		ef_clk is the source 110 aux_refclk[X] is the source 111 gt_crx_ref_clk is the source
		PSU_CRF_APB_VPLL_CTRL_PRE_SRC                                                   0x0

		The integer portion of the feedback divider to the PLL
		PSU_CRF_APB_VPLL_CTRL_FBDIV                                                     0x39

		This turns on the divide by 2 that is inside of the PLL. This does not change the VCO frequency, just the output frequency
		PSU_CRF_APB_VPLL_CTRL_DIV2                                                      0x1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A0038, 0x00717F00U ,0x00013900U)
		RegMask = (CRF_APB_VPLL_CTRL_PRE_SRC_MASK | CRF_APB_VPLL_CTRL_FBDIV_MASK | CRF_APB_VPLL_CTRL_DIV2_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_VPLL_CTRL_PRE_SRC_SHIFT
			| 0x00000039U << CRF_APB_VPLL_CTRL_FBDIV_SHIFT
			| 0x00000001U << CRF_APB_VPLL_CTRL_DIV2_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_VPLL_CTRL_OFFSET ,0x00717F00U ,0x00013900U);
	/*############################################################################################################################ */

		// : BY PASS PLL
		/*Register : VPLL_CTRL @ 0XFD1A0038</p>

		Bypasses the PLL clock. The usable clock will be determined from the POST_SRC field. (This signal may only be toggled after 4
		cycles of the old clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_VPLL_CTRL_BYPASS                                                    1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A0038, 0x00000008U ,0x00000008U)
		RegMask = (CRF_APB_VPLL_CTRL_BYPASS_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_VPLL_CTRL_BYPASS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_VPLL_CTRL_OFFSET ,0x00000008U ,0x00000008U);
	/*############################################################################################################################ */

		// : ASSERT RESET
		/*Register : VPLL_CTRL @ 0XFD1A0038</p>

		Asserts Reset to the PLL. When asserting reset, the PLL must already be in BYPASS.
		PSU_CRF_APB_VPLL_CTRL_RESET                                                     1

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A0038, 0x00000001U ,0x00000001U)
		RegMask = (CRF_APB_VPLL_CTRL_RESET_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_VPLL_CTRL_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_VPLL_CTRL_OFFSET ,0x00000001U ,0x00000001U);
	/*############################################################################################################################ */

		// : DEASSERT RESET
		/*Register : VPLL_CTRL @ 0XFD1A0038</p>

		Asserts Reset to the PLL. When asserting reset, the PLL must already be in BYPASS.
		PSU_CRF_APB_VPLL_CTRL_RESET                                                     0

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A0038, 0x00000001U ,0x00000000U)
		RegMask = (CRF_APB_VPLL_CTRL_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_VPLL_CTRL_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_VPLL_CTRL_OFFSET ,0x00000001U ,0x00000000U);
	/*############################################################################################################################ */

		// : CHECK PLL STATUS
		/*Register : PLL_STATUS @ 0XFD1A0044</p>

		VPLL is locked
		PSU_CRF_APB_PLL_STATUS_VPLL_LOCK                                                1
		(OFFSET, MASK, VALUE)      (0XFD1A0044, 0x00000004U ,0x00000004U)  */
		mask_poll(CRF_APB_PLL_STATUS_OFFSET,0x00000004U);

	/*############################################################################################################################ */

		// : REMOVE PLL BY PASS
		/*Register : VPLL_CTRL @ 0XFD1A0038</p>

		Bypasses the PLL clock. The usable clock will be determined from the POST_SRC field. (This signal may only be toggled after 4
		cycles of the old clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_VPLL_CTRL_BYPASS                                                    0

		PLL Basic Control
		(OFFSET, MASK, VALUE)      (0XFD1A0038, 0x00000008U ,0x00000000U)
		RegMask = (CRF_APB_VPLL_CTRL_BYPASS_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_VPLL_CTRL_BYPASS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_VPLL_CTRL_OFFSET ,0x00000008U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : VPLL_TO_LPD_CTRL @ 0XFD1A0050</p>

		Divisor value for this clock.
		PSU_CRF_APB_VPLL_TO_LPD_CTRL_DIVISOR0                                           0x3

		Control for a clock that will be generated in the FPD, but used in the LPD as a clock source for the peripheral clock muxes.
		(OFFSET, MASK, VALUE)      (0XFD1A0050, 0x00003F00U ,0x00000300U)
		RegMask = (CRF_APB_VPLL_TO_LPD_CTRL_DIVISOR0_MASK |  0 );

		RegVal = ((0x00000003U << CRF_APB_VPLL_TO_LPD_CTRL_DIVISOR0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_VPLL_TO_LPD_CTRL_OFFSET ,0x00003F00U ,0x00000300U);
	/*############################################################################################################################ */

		// : VIDEO FRAC CFG
		/*Register : VPLL_FRAC_CFG @ 0XFD1A0040</p>

		Fractional SDM bypass control. When 0, PLL is in integer mode and it ignores all fractional data. When 1, PLL is in fractiona
		 mode and uses DATA of this register for the fractional portion of the feedback divider.
		PSU_CRF_APB_VPLL_FRAC_CFG_ENABLED                                               0x1

		Fractional value for the Feedback value.
		PSU_CRF_APB_VPLL_FRAC_CFG_DATA                                                  0x820c

		Fractional control for the PLL
		(OFFSET, MASK, VALUE)      (0XFD1A0040, 0x8000FFFFU ,0x8000820CU)
		RegMask = (CRF_APB_VPLL_FRAC_CFG_ENABLED_MASK | CRF_APB_VPLL_FRAC_CFG_DATA_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_VPLL_FRAC_CFG_ENABLED_SHIFT
			| 0x0000820CU << CRF_APB_VPLL_FRAC_CFG_DATA_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_VPLL_FRAC_CFG_OFFSET ,0x8000FFFFU ,0x8000820CU);
	/*############################################################################################################################ */


  return 1;
}
unsigned long psu_clock_init_data() {
		// : CLOCK CONTROL SLCR REGISTER
		/*Register : GEM0_REF_CTRL @ 0XFF5E0050</p>

		Clock active for the RX channel
		PSU_CRL_APB_GEM0_REF_CTRL_RX_CLKACT                                             0x1

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_GEM0_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_GEM0_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_GEM0_REF_CTRL_DIVISOR0                                              0x8

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_GEM0_REF_CTRL_SRCSEL                                                0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0050, 0x063F3F07U ,0x06010800U)
		RegMask = (CRL_APB_GEM0_REF_CTRL_RX_CLKACT_MASK | CRL_APB_GEM0_REF_CTRL_CLKACT_MASK | CRL_APB_GEM0_REF_CTRL_DIVISOR1_MASK | CRL_APB_GEM0_REF_CTRL_DIVISOR0_MASK | CRL_APB_GEM0_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_GEM0_REF_CTRL_RX_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_GEM0_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_GEM0_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000008U << CRL_APB_GEM0_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_GEM0_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_GEM0_REF_CTRL_OFFSET ,0x063F3F07U ,0x06010800U);
	/*############################################################################################################################ */

		/*Register : GEM1_REF_CTRL @ 0XFF5E0054</p>

		Clock active for the RX channel
		PSU_CRL_APB_GEM1_REF_CTRL_RX_CLKACT                                             0x1

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_GEM1_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_GEM1_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_GEM1_REF_CTRL_DIVISOR0                                              0x8

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_GEM1_REF_CTRL_SRCSEL                                                0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0054, 0x063F3F07U ,0x06010800U)
		RegMask = (CRL_APB_GEM1_REF_CTRL_RX_CLKACT_MASK | CRL_APB_GEM1_REF_CTRL_CLKACT_MASK | CRL_APB_GEM1_REF_CTRL_DIVISOR1_MASK | CRL_APB_GEM1_REF_CTRL_DIVISOR0_MASK | CRL_APB_GEM1_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_GEM1_REF_CTRL_RX_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_GEM1_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_GEM1_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000008U << CRL_APB_GEM1_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_GEM1_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_GEM1_REF_CTRL_OFFSET ,0x063F3F07U ,0x06010800U);
	/*############################################################################################################################ */

		/*Register : GEM2_REF_CTRL @ 0XFF5E0058</p>

		Clock active for the RX channel
		PSU_CRL_APB_GEM2_REF_CTRL_RX_CLKACT                                             0x1

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_GEM2_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_GEM2_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_GEM2_REF_CTRL_DIVISOR0                                              0x8

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_GEM2_REF_CTRL_SRCSEL                                                0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0058, 0x063F3F07U ,0x06010800U)
		RegMask = (CRL_APB_GEM2_REF_CTRL_RX_CLKACT_MASK | CRL_APB_GEM2_REF_CTRL_CLKACT_MASK | CRL_APB_GEM2_REF_CTRL_DIVISOR1_MASK | CRL_APB_GEM2_REF_CTRL_DIVISOR0_MASK | CRL_APB_GEM2_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_GEM2_REF_CTRL_RX_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_GEM2_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_GEM2_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000008U << CRL_APB_GEM2_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_GEM2_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_GEM2_REF_CTRL_OFFSET ,0x063F3F07U ,0x06010800U);
	/*############################################################################################################################ */

		/*Register : GEM3_REF_CTRL @ 0XFF5E005C</p>

		Clock active for the RX channel
		PSU_CRL_APB_GEM3_REF_CTRL_RX_CLKACT                                             0x1

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_GEM3_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_GEM3_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_GEM3_REF_CTRL_DIVISOR0                                              0x8

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_GEM3_REF_CTRL_SRCSEL                                                0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E005C, 0x063F3F07U ,0x06010800U)
		RegMask = (CRL_APB_GEM3_REF_CTRL_RX_CLKACT_MASK | CRL_APB_GEM3_REF_CTRL_CLKACT_MASK | CRL_APB_GEM3_REF_CTRL_DIVISOR1_MASK | CRL_APB_GEM3_REF_CTRL_DIVISOR0_MASK | CRL_APB_GEM3_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_GEM3_REF_CTRL_RX_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_GEM3_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_GEM3_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000008U << CRL_APB_GEM3_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_GEM3_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_GEM3_REF_CTRL_OFFSET ,0x063F3F07U ,0x06010800U);
	/*############################################################################################################################ */

		/*Register : GEM_TSU_REF_CTRL @ 0XFF5E0100</p>

		6 bit divider
		PSU_CRL_APB_GEM_TSU_REF_CTRL_DIVISOR0                                           0x6

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_GEM_TSU_REF_CTRL_SRCSEL                                             0x2

		6 bit divider
		PSU_CRL_APB_GEM_TSU_REF_CTRL_DIVISOR1                                           0x1

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_GEM_TSU_REF_CTRL_CLKACT                                             0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0100, 0x013F3F07U ,0x01010602U)
		RegMask = (CRL_APB_GEM_TSU_REF_CTRL_DIVISOR0_MASK | CRL_APB_GEM_TSU_REF_CTRL_SRCSEL_MASK | CRL_APB_GEM_TSU_REF_CTRL_DIVISOR1_MASK | CRL_APB_GEM_TSU_REF_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000006U << CRL_APB_GEM_TSU_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_GEM_TSU_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRL_APB_GEM_TSU_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000001U << CRL_APB_GEM_TSU_REF_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_GEM_TSU_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010602U);
	/*############################################################################################################################ */

		/*Register : USB0_BUS_REF_CTRL @ 0XFF5E0060</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_USB0_BUS_REF_CTRL_CLKACT                                            0x1

		6 bit divider
		PSU_CRL_APB_USB0_BUS_REF_CTRL_DIVISOR1                                          0x1

		6 bit divider
		PSU_CRL_APB_USB0_BUS_REF_CTRL_DIVISOR0                                          0x6

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_USB0_BUS_REF_CTRL_SRCSEL                                            0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0060, 0x023F3F07U ,0x02010600U)
		RegMask = (CRL_APB_USB0_BUS_REF_CTRL_CLKACT_MASK | CRL_APB_USB0_BUS_REF_CTRL_DIVISOR1_MASK | CRL_APB_USB0_BUS_REF_CTRL_DIVISOR0_MASK | CRL_APB_USB0_BUS_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_USB0_BUS_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_USB0_BUS_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000006U << CRL_APB_USB0_BUS_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_USB0_BUS_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_USB0_BUS_REF_CTRL_OFFSET ,0x023F3F07U ,0x02010600U);
	/*############################################################################################################################ */

		/*Register : USB1_BUS_REF_CTRL @ 0XFF5E0064</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_USB1_BUS_REF_CTRL_CLKACT                                            0x1

		6 bit divider
		PSU_CRL_APB_USB1_BUS_REF_CTRL_DIVISOR1                                          0x1

		6 bit divider
		PSU_CRL_APB_USB1_BUS_REF_CTRL_DIVISOR0                                          0x6

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_USB1_BUS_REF_CTRL_SRCSEL                                            0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0064, 0x023F3F07U ,0x02010600U)
		RegMask = (CRL_APB_USB1_BUS_REF_CTRL_CLKACT_MASK | CRL_APB_USB1_BUS_REF_CTRL_DIVISOR1_MASK | CRL_APB_USB1_BUS_REF_CTRL_DIVISOR0_MASK | CRL_APB_USB1_BUS_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_USB1_BUS_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_USB1_BUS_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000006U << CRL_APB_USB1_BUS_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_USB1_BUS_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_USB1_BUS_REF_CTRL_OFFSET ,0x023F3F07U ,0x02010600U);
	/*############################################################################################################################ */

		/*Register : USB3_DUAL_REF_CTRL @ 0XFF5E004C</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_USB3_DUAL_REF_CTRL_CLKACT                                           0x1

		6 bit divider
		PSU_CRL_APB_USB3_DUAL_REF_CTRL_DIVISOR1                                         0xf

		6 bit divider
		PSU_CRL_APB_USB3_DUAL_REF_CTRL_DIVISOR0                                         0x5

		000 = IOPLL; 010 = RPLL; 011 = DPLL. (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_USB3_DUAL_REF_CTRL_SRCSEL                                           0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E004C, 0x023F3F07U ,0x020F0500U)
		RegMask = (CRL_APB_USB3_DUAL_REF_CTRL_CLKACT_MASK | CRL_APB_USB3_DUAL_REF_CTRL_DIVISOR1_MASK | CRL_APB_USB3_DUAL_REF_CTRL_DIVISOR0_MASK | CRL_APB_USB3_DUAL_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_USB3_DUAL_REF_CTRL_CLKACT_SHIFT
			| 0x0000000FU << CRL_APB_USB3_DUAL_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000005U << CRL_APB_USB3_DUAL_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_USB3_DUAL_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_USB3_DUAL_REF_CTRL_OFFSET ,0x023F3F07U ,0x020F0500U);
	/*############################################################################################################################ */

		/*Register : QSPI_REF_CTRL @ 0XFF5E0068</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_QSPI_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_QSPI_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_QSPI_REF_CTRL_DIVISOR0                                              0xc

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_QSPI_REF_CTRL_SRCSEL                                                0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0068, 0x013F3F07U ,0x01010C00U)
		RegMask = (CRL_APB_QSPI_REF_CTRL_CLKACT_MASK | CRL_APB_QSPI_REF_CTRL_DIVISOR1_MASK | CRL_APB_QSPI_REF_CTRL_DIVISOR0_MASK | CRL_APB_QSPI_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_QSPI_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_QSPI_REF_CTRL_DIVISOR1_SHIFT
			| 0x0000000CU << CRL_APB_QSPI_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_QSPI_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_QSPI_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010C00U);
	/*############################################################################################################################ */

		/*Register : SDIO0_REF_CTRL @ 0XFF5E006C</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_SDIO0_REF_CTRL_CLKACT                                               0x1

		6 bit divider
		PSU_CRL_APB_SDIO0_REF_CTRL_DIVISOR1                                             0x1

		6 bit divider
		PSU_CRL_APB_SDIO0_REF_CTRL_DIVISOR0                                             0x6

		000 = IOPLL; 010 = RPLL; 011 = VPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_SDIO0_REF_CTRL_SRCSEL                                               0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E006C, 0x013F3F07U ,0x01010602U)
		RegMask = (CRL_APB_SDIO0_REF_CTRL_CLKACT_MASK | CRL_APB_SDIO0_REF_CTRL_DIVISOR1_MASK | CRL_APB_SDIO0_REF_CTRL_DIVISOR0_MASK | CRL_APB_SDIO0_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_SDIO0_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_SDIO0_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000006U << CRL_APB_SDIO0_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_SDIO0_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_SDIO0_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010602U);
	/*############################################################################################################################ */

		/*Register : SDIO1_REF_CTRL @ 0XFF5E0070</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_SDIO1_REF_CTRL_CLKACT                                               0x1

		6 bit divider
		PSU_CRL_APB_SDIO1_REF_CTRL_DIVISOR1                                             0x1

		6 bit divider
		PSU_CRL_APB_SDIO1_REF_CTRL_DIVISOR0                                             0x6

		000 = IOPLL; 010 = RPLL; 011 = VPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_SDIO1_REF_CTRL_SRCSEL                                               0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0070, 0x013F3F07U ,0x01010602U)
		RegMask = (CRL_APB_SDIO1_REF_CTRL_CLKACT_MASK | CRL_APB_SDIO1_REF_CTRL_DIVISOR1_MASK | CRL_APB_SDIO1_REF_CTRL_DIVISOR0_MASK | CRL_APB_SDIO1_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_SDIO1_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_SDIO1_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000006U << CRL_APB_SDIO1_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_SDIO1_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_SDIO1_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010602U);
	/*############################################################################################################################ */

		/*Register : SDIO_CLK_CTRL @ 0XFF18030C</p>

		MIO pad selection for sdio0_rx_clk (feedback clock from the PAD) 00: MIO [22] 01: MIO [38] 10: MIO [64] 11: MIO [64]
		PSU_IOU_SLCR_SDIO_CLK_CTRL_SDIO0_RX_SRC_SEL                                     0

		MIO pad selection for sdio1_rx_clk (feedback clock from the PAD) 0: MIO [51] 1: MIO [76]
		PSU_IOU_SLCR_SDIO_CLK_CTRL_SDIO1_RX_SRC_SEL                                     0

		SoC Debug Clock Control
		(OFFSET, MASK, VALUE)      (0XFF18030C, 0x00020003U ,0x00000000U)
		RegMask = (IOU_SLCR_SDIO_CLK_CTRL_SDIO0_RX_SRC_SEL_MASK | IOU_SLCR_SDIO_CLK_CTRL_SDIO1_RX_SRC_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_SDIO_CLK_CTRL_SDIO0_RX_SRC_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_SDIO_CLK_CTRL_SDIO1_RX_SRC_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_SDIO_CLK_CTRL_OFFSET ,0x00020003U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : UART0_REF_CTRL @ 0XFF5E0074</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_UART0_REF_CTRL_CLKACT                                               0x1

		6 bit divider
		PSU_CRL_APB_UART0_REF_CTRL_DIVISOR1                                             0x1

		6 bit divider
		PSU_CRL_APB_UART0_REF_CTRL_DIVISOR0                                             0xa

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_UART0_REF_CTRL_SRCSEL                                               0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0074, 0x013F3F07U ,0x01010A00U)
		RegMask = (CRL_APB_UART0_REF_CTRL_CLKACT_MASK | CRL_APB_UART0_REF_CTRL_DIVISOR1_MASK | CRL_APB_UART0_REF_CTRL_DIVISOR0_MASK | CRL_APB_UART0_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_UART0_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_UART0_REF_CTRL_DIVISOR1_SHIFT
			| 0x0000000AU << CRL_APB_UART0_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_UART0_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_UART0_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010A00U);
	/*############################################################################################################################ */

		/*Register : UART1_REF_CTRL @ 0XFF5E0078</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_UART1_REF_CTRL_CLKACT                                               0x1

		6 bit divider
		PSU_CRL_APB_UART1_REF_CTRL_DIVISOR1                                             0x1

		6 bit divider
		PSU_CRL_APB_UART1_REF_CTRL_DIVISOR0                                             0xf

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_UART1_REF_CTRL_SRCSEL                                               0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0078, 0x013F3F07U ,0x01010F00U)
		RegMask = (CRL_APB_UART1_REF_CTRL_CLKACT_MASK | CRL_APB_UART1_REF_CTRL_DIVISOR1_MASK | CRL_APB_UART1_REF_CTRL_DIVISOR0_MASK | CRL_APB_UART1_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_UART1_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_UART1_REF_CTRL_DIVISOR1_SHIFT
			| 0x0000000FU << CRL_APB_UART1_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_UART1_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_UART1_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010F00U);
	/*############################################################################################################################ */

		/*Register : I2C0_REF_CTRL @ 0XFF5E0120</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_I2C0_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_I2C0_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_I2C0_REF_CTRL_DIVISOR0                                              0xf

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_I2C0_REF_CTRL_SRCSEL                                                0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0120, 0x013F3F07U ,0x01010F00U)
		RegMask = (CRL_APB_I2C0_REF_CTRL_CLKACT_MASK | CRL_APB_I2C0_REF_CTRL_DIVISOR1_MASK | CRL_APB_I2C0_REF_CTRL_DIVISOR0_MASK | CRL_APB_I2C0_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_I2C0_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_I2C0_REF_CTRL_DIVISOR1_SHIFT
			| 0x0000000FU << CRL_APB_I2C0_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_I2C0_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_I2C0_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010F00U);
	/*############################################################################################################################ */

		/*Register : I2C1_REF_CTRL @ 0XFF5E0124</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_I2C1_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_I2C1_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_I2C1_REF_CTRL_DIVISOR0                                              0xa

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_I2C1_REF_CTRL_SRCSEL                                                0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0124, 0x013F3F07U ,0x01010A00U)
		RegMask = (CRL_APB_I2C1_REF_CTRL_CLKACT_MASK | CRL_APB_I2C1_REF_CTRL_DIVISOR1_MASK | CRL_APB_I2C1_REF_CTRL_DIVISOR0_MASK | CRL_APB_I2C1_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_I2C1_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_I2C1_REF_CTRL_DIVISOR1_SHIFT
			| 0x0000000AU << CRL_APB_I2C1_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_I2C1_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_I2C1_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010A00U);
	/*############################################################################################################################ */

		/*Register : SPI0_REF_CTRL @ 0XFF5E007C</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_SPI0_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_SPI0_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_SPI0_REF_CTRL_DIVISOR0                                              0x6

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_SPI0_REF_CTRL_SRCSEL                                                0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E007C, 0x013F3F07U ,0x01010602U)
		RegMask = (CRL_APB_SPI0_REF_CTRL_CLKACT_MASK | CRL_APB_SPI0_REF_CTRL_DIVISOR1_MASK | CRL_APB_SPI0_REF_CTRL_DIVISOR0_MASK | CRL_APB_SPI0_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_SPI0_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_SPI0_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000006U << CRL_APB_SPI0_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_SPI0_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_SPI0_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010602U);
	/*############################################################################################################################ */

		/*Register : SPI1_REF_CTRL @ 0XFF5E0080</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_SPI1_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_SPI1_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_SPI1_REF_CTRL_DIVISOR0                                              0x7

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_SPI1_REF_CTRL_SRCSEL                                                0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0080, 0x013F3F07U ,0x01010702U)
		RegMask = (CRL_APB_SPI1_REF_CTRL_CLKACT_MASK | CRL_APB_SPI1_REF_CTRL_DIVISOR1_MASK | CRL_APB_SPI1_REF_CTRL_DIVISOR0_MASK | CRL_APB_SPI1_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_SPI1_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_SPI1_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000007U << CRL_APB_SPI1_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_SPI1_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_SPI1_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010702U);
	/*############################################################################################################################ */

		/*Register : CAN0_REF_CTRL @ 0XFF5E0084</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_CAN0_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_CAN0_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_CAN0_REF_CTRL_DIVISOR0                                              0xa

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_CAN0_REF_CTRL_SRCSEL                                                0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0084, 0x013F3F07U ,0x01010A00U)
		RegMask = (CRL_APB_CAN0_REF_CTRL_CLKACT_MASK | CRL_APB_CAN0_REF_CTRL_DIVISOR1_MASK | CRL_APB_CAN0_REF_CTRL_DIVISOR0_MASK | CRL_APB_CAN0_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_CAN0_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_CAN0_REF_CTRL_DIVISOR1_SHIFT
			| 0x0000000AU << CRL_APB_CAN0_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_CAN0_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_CAN0_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010A00U);
	/*############################################################################################################################ */

		/*Register : CAN1_REF_CTRL @ 0XFF5E0088</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_CAN1_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_CAN1_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_CAN1_REF_CTRL_DIVISOR0                                              0xa

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_CAN1_REF_CTRL_SRCSEL                                                0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0088, 0x013F3F07U ,0x01010A00U)
		RegMask = (CRL_APB_CAN1_REF_CTRL_CLKACT_MASK | CRL_APB_CAN1_REF_CTRL_DIVISOR1_MASK | CRL_APB_CAN1_REF_CTRL_DIVISOR0_MASK | CRL_APB_CAN1_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_CAN1_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_CAN1_REF_CTRL_DIVISOR1_SHIFT
			| 0x0000000AU << CRL_APB_CAN1_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_CAN1_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_CAN1_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010A00U);
	/*############################################################################################################################ */

		/*Register : CPU_R5_CTRL @ 0XFF5E0090</p>

		Turing this off will shut down the OCM, some parts of the APM, and prevent transactions going from the FPD to the LPD and cou
		d lead to system hang
		PSU_CRL_APB_CPU_R5_CTRL_CLKACT                                                  0x1

		6 bit divider
		PSU_CRL_APB_CPU_R5_CTRL_DIVISOR0                                                0x3

		000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_CPU_R5_CTRL_SRCSEL                                                  0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0090, 0x01003F07U ,0x01000302U)
		RegMask = (CRL_APB_CPU_R5_CTRL_CLKACT_MASK | CRL_APB_CPU_R5_CTRL_DIVISOR0_MASK | CRL_APB_CPU_R5_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_CPU_R5_CTRL_CLKACT_SHIFT
			| 0x00000003U << CRL_APB_CPU_R5_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_CPU_R5_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_CPU_R5_CTRL_OFFSET ,0x01003F07U ,0x01000302U);
	/*############################################################################################################################ */

		/*Register : IOU_SWITCH_CTRL @ 0XFF5E009C</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_IOU_SWITCH_CTRL_CLKACT                                              0x1

		6 bit divider
		PSU_CRL_APB_IOU_SWITCH_CTRL_DIVISOR0                                            0x6

		000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_IOU_SWITCH_CTRL_SRCSEL                                              0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E009C, 0x01003F07U ,0x01000602U)
		RegMask = (CRL_APB_IOU_SWITCH_CTRL_CLKACT_MASK | CRL_APB_IOU_SWITCH_CTRL_DIVISOR0_MASK | CRL_APB_IOU_SWITCH_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_IOU_SWITCH_CTRL_CLKACT_SHIFT
			| 0x00000006U << CRL_APB_IOU_SWITCH_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_IOU_SWITCH_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_IOU_SWITCH_CTRL_OFFSET ,0x01003F07U ,0x01000602U);
	/*############################################################################################################################ */

		/*Register : CSU_PLL_CTRL @ 0XFF5E00A0</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_CSU_PLL_CTRL_CLKACT                                                 0x1

		6 bit divider
		PSU_CRL_APB_CSU_PLL_CTRL_DIVISOR0                                               0x3

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_CSU_PLL_CTRL_SRCSEL                                                 0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E00A0, 0x01003F07U ,0x01000302U)
		RegMask = (CRL_APB_CSU_PLL_CTRL_CLKACT_MASK | CRL_APB_CSU_PLL_CTRL_DIVISOR0_MASK | CRL_APB_CSU_PLL_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_CSU_PLL_CTRL_CLKACT_SHIFT
			| 0x00000003U << CRL_APB_CSU_PLL_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_CSU_PLL_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_CSU_PLL_CTRL_OFFSET ,0x01003F07U ,0x01000302U);
	/*############################################################################################################################ */

		/*Register : PCAP_CTRL @ 0XFF5E00A4</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_PCAP_CTRL_CLKACT                                                    0x1

		6 bit divider
		PSU_CRL_APB_PCAP_CTRL_DIVISOR0                                                  0x6

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_PCAP_CTRL_SRCSEL                                                    0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E00A4, 0x01003F07U ,0x01000602U)
		RegMask = (CRL_APB_PCAP_CTRL_CLKACT_MASK | CRL_APB_PCAP_CTRL_DIVISOR0_MASK | CRL_APB_PCAP_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_PCAP_CTRL_CLKACT_SHIFT
			| 0x00000006U << CRL_APB_PCAP_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_PCAP_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_PCAP_CTRL_OFFSET ,0x01003F07U ,0x01000602U);
	/*############################################################################################################################ */

		/*Register : LPD_SWITCH_CTRL @ 0XFF5E00A8</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_LPD_SWITCH_CTRL_CLKACT                                              0x1

		6 bit divider
		PSU_CRL_APB_LPD_SWITCH_CTRL_DIVISOR0                                            0x3

		000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_LPD_SWITCH_CTRL_SRCSEL                                              0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E00A8, 0x01003F07U ,0x01000302U)
		RegMask = (CRL_APB_LPD_SWITCH_CTRL_CLKACT_MASK | CRL_APB_LPD_SWITCH_CTRL_DIVISOR0_MASK | CRL_APB_LPD_SWITCH_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_LPD_SWITCH_CTRL_CLKACT_SHIFT
			| 0x00000003U << CRL_APB_LPD_SWITCH_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_LPD_SWITCH_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_LPD_SWITCH_CTRL_OFFSET ,0x01003F07U ,0x01000302U);
	/*############################################################################################################################ */

		/*Register : LPD_LSBUS_CTRL @ 0XFF5E00AC</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_LPD_LSBUS_CTRL_CLKACT                                               0x1

		6 bit divider
		PSU_CRL_APB_LPD_LSBUS_CTRL_DIVISOR0                                             0xf

		000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_LPD_LSBUS_CTRL_SRCSEL                                               0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E00AC, 0x01003F07U ,0x01000F02U)
		RegMask = (CRL_APB_LPD_LSBUS_CTRL_CLKACT_MASK | CRL_APB_LPD_LSBUS_CTRL_DIVISOR0_MASK | CRL_APB_LPD_LSBUS_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_LPD_LSBUS_CTRL_CLKACT_SHIFT
			| 0x0000000FU << CRL_APB_LPD_LSBUS_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_LPD_LSBUS_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_LPD_LSBUS_CTRL_OFFSET ,0x01003F07U ,0x01000F02U);
	/*############################################################################################################################ */

		/*Register : DBG_LPD_CTRL @ 0XFF5E00B0</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_DBG_LPD_CTRL_CLKACT                                                 0x1

		6 bit divider
		PSU_CRL_APB_DBG_LPD_CTRL_DIVISOR0                                               0x6

		000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_DBG_LPD_CTRL_SRCSEL                                                 0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E00B0, 0x01003F07U ,0x01000602U)
		RegMask = (CRL_APB_DBG_LPD_CTRL_CLKACT_MASK | CRL_APB_DBG_LPD_CTRL_DIVISOR0_MASK | CRL_APB_DBG_LPD_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_DBG_LPD_CTRL_CLKACT_SHIFT
			| 0x00000006U << CRL_APB_DBG_LPD_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_DBG_LPD_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_DBG_LPD_CTRL_OFFSET ,0x01003F07U ,0x01000602U);
	/*############################################################################################################################ */

		/*Register : NAND_REF_CTRL @ 0XFF5E00B4</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_NAND_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_NAND_REF_CTRL_DIVISOR1                                              0x1

		6 bit divider
		PSU_CRL_APB_NAND_REF_CTRL_DIVISOR0                                              0xa

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_NAND_REF_CTRL_SRCSEL                                                0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E00B4, 0x013F3F07U ,0x01010A00U)
		RegMask = (CRL_APB_NAND_REF_CTRL_CLKACT_MASK | CRL_APB_NAND_REF_CTRL_DIVISOR1_MASK | CRL_APB_NAND_REF_CTRL_DIVISOR0_MASK | CRL_APB_NAND_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_NAND_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_NAND_REF_CTRL_DIVISOR1_SHIFT
			| 0x0000000AU << CRL_APB_NAND_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_NAND_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_NAND_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010A00U);
	/*############################################################################################################################ */

		/*Register : ADMA_REF_CTRL @ 0XFF5E00B8</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_ADMA_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRL_APB_ADMA_REF_CTRL_DIVISOR0                                              0x3

		000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_ADMA_REF_CTRL_SRCSEL                                                0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E00B8, 0x01003F07U ,0x01000302U)
		RegMask = (CRL_APB_ADMA_REF_CTRL_CLKACT_MASK | CRL_APB_ADMA_REF_CTRL_DIVISOR0_MASK | CRL_APB_ADMA_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_ADMA_REF_CTRL_CLKACT_SHIFT
			| 0x00000003U << CRL_APB_ADMA_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_ADMA_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_ADMA_REF_CTRL_OFFSET ,0x01003F07U ,0x01000302U);
	/*############################################################################################################################ */

		/*Register : PL0_REF_CTRL @ 0XFF5E00C0</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_PL0_REF_CTRL_CLKACT                                                 0x1

		6 bit divider
		PSU_CRL_APB_PL0_REF_CTRL_DIVISOR1                                               0x1

		6 bit divider
		PSU_CRL_APB_PL0_REF_CTRL_DIVISOR0                                               0xf

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_PL0_REF_CTRL_SRCSEL                                                 0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E00C0, 0x013F3F07U ,0x01010F00U)
		RegMask = (CRL_APB_PL0_REF_CTRL_CLKACT_MASK | CRL_APB_PL0_REF_CTRL_DIVISOR1_MASK | CRL_APB_PL0_REF_CTRL_DIVISOR0_MASK | CRL_APB_PL0_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_PL0_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_PL0_REF_CTRL_DIVISOR1_SHIFT
			| 0x0000000FU << CRL_APB_PL0_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_PL0_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_PL0_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010F00U);
	/*############################################################################################################################ */

		/*Register : PL1_REF_CTRL @ 0XFF5E00C4</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_PL1_REF_CTRL_CLKACT                                                 0x1

		6 bit divider
		PSU_CRL_APB_PL1_REF_CTRL_DIVISOR1                                               0x4

		6 bit divider
		PSU_CRL_APB_PL1_REF_CTRL_DIVISOR0                                               0xf

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_PL1_REF_CTRL_SRCSEL                                                 0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E00C4, 0x013F3F07U ,0x01040F00U)
		RegMask = (CRL_APB_PL1_REF_CTRL_CLKACT_MASK | CRL_APB_PL1_REF_CTRL_DIVISOR1_MASK | CRL_APB_PL1_REF_CTRL_DIVISOR0_MASK | CRL_APB_PL1_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_PL1_REF_CTRL_CLKACT_SHIFT
			| 0x00000004U << CRL_APB_PL1_REF_CTRL_DIVISOR1_SHIFT
			| 0x0000000FU << CRL_APB_PL1_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_PL1_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_PL1_REF_CTRL_OFFSET ,0x013F3F07U ,0x01040F00U);
	/*############################################################################################################################ */

		/*Register : PL2_REF_CTRL @ 0XFF5E00C8</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_PL2_REF_CTRL_CLKACT                                                 0x1

		6 bit divider
		PSU_CRL_APB_PL2_REF_CTRL_DIVISOR1                                               0x1

		6 bit divider
		PSU_CRL_APB_PL2_REF_CTRL_DIVISOR0                                               0x4

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_PL2_REF_CTRL_SRCSEL                                                 0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E00C8, 0x013F3F07U ,0x01010402U)
		RegMask = (CRL_APB_PL2_REF_CTRL_CLKACT_MASK | CRL_APB_PL2_REF_CTRL_DIVISOR1_MASK | CRL_APB_PL2_REF_CTRL_DIVISOR0_MASK | CRL_APB_PL2_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_PL2_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_PL2_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000004U << CRL_APB_PL2_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_PL2_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_PL2_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010402U);
	/*############################################################################################################################ */

		/*Register : PL3_REF_CTRL @ 0XFF5E00CC</p>

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_PL3_REF_CTRL_CLKACT                                                 0x1

		6 bit divider
		PSU_CRL_APB_PL3_REF_CTRL_DIVISOR1                                               0x1

		6 bit divider
		PSU_CRL_APB_PL3_REF_CTRL_DIVISOR0                                               0x3

		000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_PL3_REF_CTRL_SRCSEL                                                 0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E00CC, 0x013F3F07U ,0x01010302U)
		RegMask = (CRL_APB_PL3_REF_CTRL_CLKACT_MASK | CRL_APB_PL3_REF_CTRL_DIVISOR1_MASK | CRL_APB_PL3_REF_CTRL_DIVISOR0_MASK | CRL_APB_PL3_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_PL3_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRL_APB_PL3_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000003U << CRL_APB_PL3_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_PL3_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_PL3_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010302U);
	/*############################################################################################################################ */

		/*Register : AMS_REF_CTRL @ 0XFF5E0108</p>

		6 bit divider
		PSU_CRL_APB_AMS_REF_CTRL_DIVISOR1                                               0x1

		6 bit divider
		PSU_CRL_APB_AMS_REF_CTRL_DIVISOR0                                               0x1d

		000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_AMS_REF_CTRL_SRCSEL                                                 0x2

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_AMS_REF_CTRL_CLKACT                                                 0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0108, 0x013F3F07U ,0x01011D02U)
		RegMask = (CRL_APB_AMS_REF_CTRL_DIVISOR1_MASK | CRL_APB_AMS_REF_CTRL_DIVISOR0_MASK | CRL_APB_AMS_REF_CTRL_SRCSEL_MASK | CRL_APB_AMS_REF_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_AMS_REF_CTRL_DIVISOR1_SHIFT
			| 0x0000001DU << CRL_APB_AMS_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRL_APB_AMS_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRL_APB_AMS_REF_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_AMS_REF_CTRL_OFFSET ,0x013F3F07U ,0x01011D02U);
	/*############################################################################################################################ */

		/*Register : DLL_REF_CTRL @ 0XFF5E0104</p>

		000 = IOPLL; 001 = RPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new clock. This
		is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_DLL_REF_CTRL_SRCSEL                                                 0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0104, 0x00000007U ,0x00000000U)
		RegMask = (CRL_APB_DLL_REF_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_DLL_REF_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_DLL_REF_CTRL_OFFSET ,0x00000007U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : TIMESTAMP_REF_CTRL @ 0XFF5E0128</p>

		6 bit divider
		PSU_CRL_APB_TIMESTAMP_REF_CTRL_DIVISOR0                                         0xf

		1XX = pss_ref_clk; 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and
		 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRL_APB_TIMESTAMP_REF_CTRL_SRCSEL                                           0x0

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT                                           0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFF5E0128, 0x01003F07U ,0x01000F00U)
		RegMask = (CRL_APB_TIMESTAMP_REF_CTRL_DIVISOR0_MASK | CRL_APB_TIMESTAMP_REF_CTRL_SRCSEL_MASK | CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x0000000FU << CRL_APB_TIMESTAMP_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRL_APB_TIMESTAMP_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_TIMESTAMP_REF_CTRL_OFFSET ,0x01003F07U ,0x01000F00U);
	/*############################################################################################################################ */

		/*Register : SATA_REF_CTRL @ 0XFD1A00A0</p>

		000 = IOPLL_TO_FPD; 010 = APLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of
		he new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_SATA_REF_CTRL_SRCSEL                                                0x0

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_SATA_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRF_APB_SATA_REF_CTRL_DIVISOR0                                              0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A00A0, 0x01003F07U ,0x01000200U)
		RegMask = (CRF_APB_SATA_REF_CTRL_SRCSEL_MASK | CRF_APB_SATA_REF_CTRL_CLKACT_MASK | CRF_APB_SATA_REF_CTRL_DIVISOR0_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_SATA_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_SATA_REF_CTRL_CLKACT_SHIFT
			| 0x00000002U << CRF_APB_SATA_REF_CTRL_DIVISOR0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_SATA_REF_CTRL_OFFSET ,0x01003F07U ,0x01000200U);
	/*############################################################################################################################ */

		/*Register : PCIE_REF_CTRL @ 0XFD1A00B4</p>

		000 = IOPLL_TO_FPD; 010 = RPLL_TO_FPD; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cyc
		es of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_PCIE_REF_CTRL_SRCSEL                                                0x0

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_PCIE_REF_CTRL_CLKACT                                                0x1

		6 bit divider
		PSU_CRF_APB_PCIE_REF_CTRL_DIVISOR0                                              0x2

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A00B4, 0x01003F07U ,0x01000200U)
		RegMask = (CRF_APB_PCIE_REF_CTRL_SRCSEL_MASK | CRF_APB_PCIE_REF_CTRL_CLKACT_MASK | CRF_APB_PCIE_REF_CTRL_DIVISOR0_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_PCIE_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_PCIE_REF_CTRL_CLKACT_SHIFT
			| 0x00000002U << CRF_APB_PCIE_REF_CTRL_DIVISOR0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_PCIE_REF_CTRL_OFFSET ,0x01003F07U ,0x01000200U);
	/*############################################################################################################################ */

		/*Register : DP_VIDEO_REF_CTRL @ 0XFD1A0070</p>

		6 bit divider
		PSU_CRF_APB_DP_VIDEO_REF_CTRL_DIVISOR1                                          0x1

		6 bit divider
		PSU_CRF_APB_DP_VIDEO_REF_CTRL_DIVISOR0                                          0x3

		000 = VPLL; 010 = DPLL; 011 = RPLL_TO_FPD - might be using extra mux; (This signal may only be toggled after 4 cycles of the
		ld clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_DP_VIDEO_REF_CTRL_SRCSEL                                            0x3

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_DP_VIDEO_REF_CTRL_CLKACT                                            0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A0070, 0x013F3F07U ,0x01010303U)
		RegMask = (CRF_APB_DP_VIDEO_REF_CTRL_DIVISOR1_MASK | CRF_APB_DP_VIDEO_REF_CTRL_DIVISOR0_MASK | CRF_APB_DP_VIDEO_REF_CTRL_SRCSEL_MASK | CRF_APB_DP_VIDEO_REF_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_DP_VIDEO_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000003U << CRF_APB_DP_VIDEO_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000003U << CRF_APB_DP_VIDEO_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_DP_VIDEO_REF_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DP_VIDEO_REF_CTRL_OFFSET ,0x013F3F07U ,0x01010303U);
	/*############################################################################################################################ */

		/*Register : DP_AUDIO_REF_CTRL @ 0XFD1A0074</p>

		6 bit divider
		PSU_CRF_APB_DP_AUDIO_REF_CTRL_DIVISOR1                                          0x1

		6 bit divider
		PSU_CRF_APB_DP_AUDIO_REF_CTRL_DIVISOR0                                          0x27

		000 = VPLL; 010 = DPLL; 011 = RPLL_TO_FPD - might be using extra mux; (This signal may only be toggled after 4 cycles of the
		ld clock and 4 cycles of the new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_DP_AUDIO_REF_CTRL_SRCSEL                                            0x0

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_DP_AUDIO_REF_CTRL_CLKACT                                            0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A0074, 0x013F3F07U ,0x01012700U)
		RegMask = (CRF_APB_DP_AUDIO_REF_CTRL_DIVISOR1_MASK | CRF_APB_DP_AUDIO_REF_CTRL_DIVISOR0_MASK | CRF_APB_DP_AUDIO_REF_CTRL_SRCSEL_MASK | CRF_APB_DP_AUDIO_REF_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_DP_AUDIO_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000027U << CRF_APB_DP_AUDIO_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRF_APB_DP_AUDIO_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_DP_AUDIO_REF_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DP_AUDIO_REF_CTRL_OFFSET ,0x013F3F07U ,0x01012700U);
	/*############################################################################################################################ */

		/*Register : DP_STC_REF_CTRL @ 0XFD1A007C</p>

		6 bit divider
		PSU_CRF_APB_DP_STC_REF_CTRL_DIVISOR1                                            0x1

		6 bit divider
		PSU_CRF_APB_DP_STC_REF_CTRL_DIVISOR0                                            0x11

		000 = VPLL; 010 = DPLL; 011 = RPLL_TO_FPD; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of t
		e new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_DP_STC_REF_CTRL_SRCSEL                                              0x3

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_DP_STC_REF_CTRL_CLKACT                                              0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A007C, 0x013F3F07U ,0x01011103U)
		RegMask = (CRF_APB_DP_STC_REF_CTRL_DIVISOR1_MASK | CRF_APB_DP_STC_REF_CTRL_DIVISOR0_MASK | CRF_APB_DP_STC_REF_CTRL_SRCSEL_MASK | CRF_APB_DP_STC_REF_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_DP_STC_REF_CTRL_DIVISOR1_SHIFT
			| 0x00000011U << CRF_APB_DP_STC_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000003U << CRF_APB_DP_STC_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_DP_STC_REF_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DP_STC_REF_CTRL_OFFSET ,0x013F3F07U ,0x01011103U);
	/*############################################################################################################################ */

		/*Register : ACPU_CTRL @ 0XFD1A0060</p>

		6 bit divider
		PSU_CRF_APB_ACPU_CTRL_DIVISOR0                                                  0x1

		000 = APLL; 010 = DPLL; 011 = VPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		lock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_ACPU_CTRL_SRCSEL                                                    0x0

		Clock active signal. Switch to 0 to disable the clock. For the half speed APU Clock
		PSU_CRF_APB_ACPU_CTRL_CLKACT_HALF                                               0x1

		Clock active signal. Switch to 0 to disable the clock. For the full speed ACPUX Clock. This will shut off the high speed cloc
		 to the entire APU
		PSU_CRF_APB_ACPU_CTRL_CLKACT_FULL                                               0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A0060, 0x03003F07U ,0x03000100U)
		RegMask = (CRF_APB_ACPU_CTRL_DIVISOR0_MASK | CRF_APB_ACPU_CTRL_SRCSEL_MASK | CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK | CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_ACPU_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRF_APB_ACPU_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_ACPU_CTRL_CLKACT_HALF_SHIFT
			| 0x00000001U << CRF_APB_ACPU_CTRL_CLKACT_FULL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_ACPU_CTRL_OFFSET ,0x03003F07U ,0x03000100U);
	/*############################################################################################################################ */

		/*Register : DBG_TRACE_CTRL @ 0XFD1A0064</p>

		6 bit divider
		PSU_CRF_APB_DBG_TRACE_CTRL_DIVISOR0                                             0x2

		000 = IOPLL_TO_FPD; 010 = DPLL; 011 = APLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of
		he new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_DBG_TRACE_CTRL_SRCSEL                                               0x0

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_DBG_TRACE_CTRL_CLKACT                                               0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A0064, 0x01003F07U ,0x01000200U)
		RegMask = (CRF_APB_DBG_TRACE_CTRL_DIVISOR0_MASK | CRF_APB_DBG_TRACE_CTRL_SRCSEL_MASK | CRF_APB_DBG_TRACE_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000002U << CRF_APB_DBG_TRACE_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRF_APB_DBG_TRACE_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_DBG_TRACE_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DBG_TRACE_CTRL_OFFSET ,0x01003F07U ,0x01000200U);
	/*############################################################################################################################ */

		/*Register : DBG_FPD_CTRL @ 0XFD1A0068</p>

		6 bit divider
		PSU_CRF_APB_DBG_FPD_CTRL_DIVISOR0                                               0x2

		000 = IOPLL_TO_FPD; 010 = DPLL; 011 = APLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of
		he new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_DBG_FPD_CTRL_SRCSEL                                                 0x0

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_DBG_FPD_CTRL_CLKACT                                                 0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A0068, 0x01003F07U ,0x01000200U)
		RegMask = (CRF_APB_DBG_FPD_CTRL_DIVISOR0_MASK | CRF_APB_DBG_FPD_CTRL_SRCSEL_MASK | CRF_APB_DBG_FPD_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000002U << CRF_APB_DBG_FPD_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRF_APB_DBG_FPD_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_DBG_FPD_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DBG_FPD_CTRL_OFFSET ,0x01003F07U ,0x01000200U);
	/*############################################################################################################################ */

		/*Register : DDR_CTRL @ 0XFD1A0080</p>

		6 bit divider
		PSU_CRF_APB_DDR_CTRL_DIVISOR0                                                   0x3

		000 = DPLL; 001 = VPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new clock. This
		s not usually an issue, but designers must be aware.)
		PSU_CRF_APB_DDR_CTRL_SRCSEL                                                     0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A0080, 0x00003F07U ,0x00000300U)
		RegMask = (CRF_APB_DDR_CTRL_DIVISOR0_MASK | CRF_APB_DDR_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000003U << CRF_APB_DDR_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRF_APB_DDR_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DDR_CTRL_OFFSET ,0x00003F07U ,0x00000300U);
	/*############################################################################################################################ */

		/*Register : GPU_REF_CTRL @ 0XFD1A0084</p>

		6 bit divider
		PSU_CRF_APB_GPU_REF_CTRL_DIVISOR0                                               0x1

		000 = IOPLL_TO_FPD; 010 = VPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of
		he new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_GPU_REF_CTRL_SRCSEL                                                 0x0

		Clock active signal. Switch to 0 to disable the clock, which will stop clock for GPU (and both Pixel Processors).
		PSU_CRF_APB_GPU_REF_CTRL_CLKACT                                                 0x1

		Clock active signal for Pixel Processor. Switch to 0 to disable the clock only to this Pixel Processor
		PSU_CRF_APB_GPU_REF_CTRL_PP0_CLKACT                                             0x1

		Clock active signal for Pixel Processor. Switch to 0 to disable the clock only to this Pixel Processor
		PSU_CRF_APB_GPU_REF_CTRL_PP1_CLKACT                                             0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A0084, 0x07003F07U ,0x07000100U)
		RegMask = (CRF_APB_GPU_REF_CTRL_DIVISOR0_MASK | CRF_APB_GPU_REF_CTRL_SRCSEL_MASK | CRF_APB_GPU_REF_CTRL_CLKACT_MASK | CRF_APB_GPU_REF_CTRL_PP0_CLKACT_MASK | CRF_APB_GPU_REF_CTRL_PP1_CLKACT_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_GPU_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRF_APB_GPU_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_GPU_REF_CTRL_CLKACT_SHIFT
			| 0x00000001U << CRF_APB_GPU_REF_CTRL_PP0_CLKACT_SHIFT
			| 0x00000001U << CRF_APB_GPU_REF_CTRL_PP1_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_GPU_REF_CTRL_OFFSET ,0x07003F07U ,0x07000100U);
	/*############################################################################################################################ */

		/*Register : GDMA_REF_CTRL @ 0XFD1A00B8</p>

		6 bit divider
		PSU_CRF_APB_GDMA_REF_CTRL_DIVISOR0                                              0x2

		000 = APLL; 010 = VPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		lock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_GDMA_REF_CTRL_SRCSEL                                                0x0

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_GDMA_REF_CTRL_CLKACT                                                0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A00B8, 0x01003F07U ,0x01000200U)
		RegMask = (CRF_APB_GDMA_REF_CTRL_DIVISOR0_MASK | CRF_APB_GDMA_REF_CTRL_SRCSEL_MASK | CRF_APB_GDMA_REF_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000002U << CRF_APB_GDMA_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRF_APB_GDMA_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_GDMA_REF_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_GDMA_REF_CTRL_OFFSET ,0x01003F07U ,0x01000200U);
	/*############################################################################################################################ */

		/*Register : DPDMA_REF_CTRL @ 0XFD1A00BC</p>

		6 bit divider
		PSU_CRF_APB_DPDMA_REF_CTRL_DIVISOR0                                             0x2

		000 = APLL; 010 = VPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		lock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_DPDMA_REF_CTRL_SRCSEL                                               0x0

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_DPDMA_REF_CTRL_CLKACT                                               0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A00BC, 0x01003F07U ,0x01000200U)
		RegMask = (CRF_APB_DPDMA_REF_CTRL_DIVISOR0_MASK | CRF_APB_DPDMA_REF_CTRL_SRCSEL_MASK | CRF_APB_DPDMA_REF_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000002U << CRF_APB_DPDMA_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRF_APB_DPDMA_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_DPDMA_REF_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DPDMA_REF_CTRL_OFFSET ,0x01003F07U ,0x01000200U);
	/*############################################################################################################################ */

		/*Register : TOPSW_MAIN_CTRL @ 0XFD1A00C0</p>

		6 bit divider
		PSU_CRF_APB_TOPSW_MAIN_CTRL_DIVISOR0                                            0x2

		000 = APLL; 010 = VPLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of the new
		lock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_TOPSW_MAIN_CTRL_SRCSEL                                              0x2

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_TOPSW_MAIN_CTRL_CLKACT                                              0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A00C0, 0x01003F07U ,0x01000202U)
		RegMask = (CRF_APB_TOPSW_MAIN_CTRL_DIVISOR0_MASK | CRF_APB_TOPSW_MAIN_CTRL_SRCSEL_MASK | CRF_APB_TOPSW_MAIN_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000002U << CRF_APB_TOPSW_MAIN_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRF_APB_TOPSW_MAIN_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_TOPSW_MAIN_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_TOPSW_MAIN_CTRL_OFFSET ,0x01003F07U ,0x01000202U);
	/*############################################################################################################################ */

		/*Register : TOPSW_LSBUS_CTRL @ 0XFD1A00C4</p>

		6 bit divider
		PSU_CRF_APB_TOPSW_LSBUS_CTRL_DIVISOR0                                           0x5

		000 = APLL; 010 = IOPLL_TO_FPD; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of
		he new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_TOPSW_LSBUS_CTRL_SRCSEL                                             0x2

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_TOPSW_LSBUS_CTRL_CLKACT                                             0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A00C4, 0x01003F07U ,0x01000502U)
		RegMask = (CRF_APB_TOPSW_LSBUS_CTRL_DIVISOR0_MASK | CRF_APB_TOPSW_LSBUS_CTRL_SRCSEL_MASK | CRF_APB_TOPSW_LSBUS_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000005U << CRF_APB_TOPSW_LSBUS_CTRL_DIVISOR0_SHIFT
			| 0x00000002U << CRF_APB_TOPSW_LSBUS_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_TOPSW_LSBUS_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_TOPSW_LSBUS_CTRL_OFFSET ,0x01003F07U ,0x01000502U);
	/*############################################################################################################################ */

		/*Register : GTGREF0_REF_CTRL @ 0XFD1A00C8</p>

		6 bit divider
		PSU_CRF_APB_GTGREF0_REF_CTRL_DIVISOR0                                           0x4

		000 = IOPLL_TO_FPD; 010 = APLL; 011 = DPLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of
		he new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_GTGREF0_REF_CTRL_SRCSEL                                             0x0

		Clock active signal. Switch to 0 to disable the clock
		PSU_CRF_APB_GTGREF0_REF_CTRL_CLKACT                                             0x1

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A00C8, 0x01003F07U ,0x01000400U)
		RegMask = (CRF_APB_GTGREF0_REF_CTRL_DIVISOR0_MASK | CRF_APB_GTGREF0_REF_CTRL_SRCSEL_MASK | CRF_APB_GTGREF0_REF_CTRL_CLKACT_MASK |  0 );

		RegVal = ((0x00000004U << CRF_APB_GTGREF0_REF_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRF_APB_GTGREF0_REF_CTRL_SRCSEL_SHIFT
			| 0x00000001U << CRF_APB_GTGREF0_REF_CTRL_CLKACT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_GTGREF0_REF_CTRL_OFFSET ,0x01003F07U ,0x01000400U);
	/*############################################################################################################################ */

		/*Register : DBG_TSTMP_CTRL @ 0XFD1A00F8</p>

		6 bit divider
		PSU_CRF_APB_DBG_TSTMP_CTRL_DIVISOR0                                             0x2

		000 = IOPLL_TO_FPD; 010 = DPLL; 011 = APLL; (This signal may only be toggled after 4 cycles of the old clock and 4 cycles of
		he new clock. This is not usually an issue, but designers must be aware.)
		PSU_CRF_APB_DBG_TSTMP_CTRL_SRCSEL                                               0x0

		This register controls this reference clock
		(OFFSET, MASK, VALUE)      (0XFD1A00F8, 0x00003F07U ,0x00000200U)
		RegMask = (CRF_APB_DBG_TSTMP_CTRL_DIVISOR0_MASK | CRF_APB_DBG_TSTMP_CTRL_SRCSEL_MASK |  0 );

		RegVal = ((0x00000002U << CRF_APB_DBG_TSTMP_CTRL_DIVISOR0_SHIFT
			| 0x00000000U << CRF_APB_DBG_TSTMP_CTRL_SRCSEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_DBG_TSTMP_CTRL_OFFSET ,0x00003F07U ,0x00000200U);
	/*############################################################################################################################ */

		/*Register : IOU_TTC_APB_CLK @ 0XFF180380</p>

		00" = Select the APB switch clock for the APB interface of TTC0'01" = Select the PLL ref clock for the APB interface of TTC0'
		0" = Select the R5 clock for the APB interface of TTC0
		PSU_IOU_SLCR_IOU_TTC_APB_CLK_TTC0_SEL                                           0

		00" = Select the APB switch clock for the APB interface of TTC1'01" = Select the PLL ref clock for the APB interface of TTC1'
		0" = Select the R5 clock for the APB interface of TTC1
		PSU_IOU_SLCR_IOU_TTC_APB_CLK_TTC1_SEL                                           0

		00" = Select the APB switch clock for the APB interface of TTC2'01" = Select the PLL ref clock for the APB interface of TTC2'
		0" = Select the R5 clock for the APB interface of TTC2
		PSU_IOU_SLCR_IOU_TTC_APB_CLK_TTC2_SEL                                           0

		00" = Select the APB switch clock for the APB interface of TTC3'01" = Select the PLL ref clock for the APB interface of TTC3'
		0" = Select the R5 clock for the APB interface of TTC3
		PSU_IOU_SLCR_IOU_TTC_APB_CLK_TTC3_SEL                                           0

		TTC APB clock select
		(OFFSET, MASK, VALUE)      (0XFF180380, 0x000000FFU ,0x00000000U)
		RegMask = (IOU_SLCR_IOU_TTC_APB_CLK_TTC0_SEL_MASK | IOU_SLCR_IOU_TTC_APB_CLK_TTC1_SEL_MASK | IOU_SLCR_IOU_TTC_APB_CLK_TTC2_SEL_MASK | IOU_SLCR_IOU_TTC_APB_CLK_TTC3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_IOU_TTC_APB_CLK_TTC0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_IOU_TTC_APB_CLK_TTC1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_IOU_TTC_APB_CLK_TTC2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_IOU_TTC_APB_CLK_TTC3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_IOU_TTC_APB_CLK_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : WDT_CLK_SEL @ 0XFD610100</p>

		System watchdog timer clock source selection: 0: Internal APB clock 1: External (PL clock via EMIO or Pinout clock via MIO)
		PSU_FPD_SLCR_WDT_CLK_SEL_SELECT                                                 0

		SWDT clock source select
		(OFFSET, MASK, VALUE)      (0XFD610100, 0x00000001U ,0x00000000U)
		RegMask = (FPD_SLCR_WDT_CLK_SEL_SELECT_MASK |  0 );

		RegVal = ((0x00000000U << FPD_SLCR_WDT_CLK_SEL_SELECT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (FPD_SLCR_WDT_CLK_SEL_OFFSET ,0x00000001U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : WDT_CLK_SEL @ 0XFF180300</p>

		System watchdog timer clock source selection: 0: internal clock APB clock 1: external clock from PL via EMIO, or from pinout
		ia MIO
		PSU_IOU_SLCR_WDT_CLK_SEL_SELECT                                                 0

		SWDT clock source select
		(OFFSET, MASK, VALUE)      (0XFF180300, 0x00000001U ,0x00000000U)
		RegMask = (IOU_SLCR_WDT_CLK_SEL_SELECT_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_WDT_CLK_SEL_SELECT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_WDT_CLK_SEL_OFFSET ,0x00000001U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : CSUPMU_WDT_CLK_SEL @ 0XFF410050</p>

		System watchdog timer clock source selection: 0: internal clock APB clock 1: external clock pss_ref_clk
		PSU_LPD_SLCR_CSUPMU_WDT_CLK_SEL_SELECT                                          0

		SWDT clock source select
		(OFFSET, MASK, VALUE)      (0XFF410050, 0x00000001U ,0x00000000U)
		RegMask = (LPD_SLCR_CSUPMU_WDT_CLK_SEL_SELECT_MASK |  0 );

		RegVal = ((0x00000000U << LPD_SLCR_CSUPMU_WDT_CLK_SEL_SELECT_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (LPD_SLCR_CSUPMU_WDT_CLK_SEL_OFFSET ,0x00000001U ,0x00000000U);
	/*############################################################################################################################ */


  return 1;
}
unsigned long psu_mio_init_data() {
		// : MIO PROGRAMMING
		/*Register : MIO_PIN_0 @ 0XFF180000</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_sclk_out- (QSPI Clock)
		PSU_IOU_SLCR_MIO_PIN_0_L0_SEL                                                   1

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_0_L1_SEL                                                   0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[0]- (Test Scan Port) = test_scan, Outp
		t, test_scan_out[0]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_0_L2_SEL                                                   0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[0]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[0]- (GPIO bank 0) 1= can
		, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL signa
		) 3= pjtag, Input, pjtag_tck- (PJTAG TCK) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4= spi0, Output, spi0_sclk_out- (SPI Cloc
		) 5= ttc3, Input, ttc3_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace, Output, trace_
		lk- (Trace Port Clock)
		PSU_IOU_SLCR_MIO_PIN_0_L3_SEL                                                   0

		Configures MIO Pin 0 peripheral interface mapping. S
		(OFFSET, MASK, VALUE)      (0XFF180000, 0x000000FEU ,0x00000002U)
		RegMask = (IOU_SLCR_MIO_PIN_0_L0_SEL_MASK | IOU_SLCR_MIO_PIN_0_L1_SEL_MASK | IOU_SLCR_MIO_PIN_0_L2_SEL_MASK | IOU_SLCR_MIO_PIN_0_L3_SEL_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_MIO_PIN_0_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_0_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_0_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_0_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_0_OFFSET ,0x000000FEU ,0x00000002U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_1 @ 0XFF180004</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi_mi1- (QSPI Databus) 1= qspi, Output, qspi_so_mo1- (QSPI Data
		us)
		PSU_IOU_SLCR_MIO_PIN_1_L0_SEL                                                   1

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_1_L1_SEL                                                   0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[1]- (Test Scan Port) = test_scan, Outp
		t, test_scan_out[1]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_1_L2_SEL                                                   0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[1]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[1]- (GPIO bank 0) 1= can
		, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA signal
		 3= pjtag, Input, pjtag_tdi- (PJTAG TDI) 4= spi0, Output, spi0_n_ss_out[2]- (SPI Master Selects) 5= ttc3, Output, ttc3_wave_o
		t- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7= trace, Output, trace_ctl- (Trace Port Control
		Signal)
		PSU_IOU_SLCR_MIO_PIN_1_L3_SEL                                                   0

		Configures MIO Pin 1 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180004, 0x000000FEU ,0x00000002U)
		RegMask = (IOU_SLCR_MIO_PIN_1_L0_SEL_MASK | IOU_SLCR_MIO_PIN_1_L1_SEL_MASK | IOU_SLCR_MIO_PIN_1_L2_SEL_MASK | IOU_SLCR_MIO_PIN_1_L3_SEL_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_MIO_PIN_1_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_1_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_1_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_1_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_1_OFFSET ,0x000000FEU ,0x00000002U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_2 @ 0XFF180008</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi2- (QSPI Databus) 1= qspi, Output, qspi_mo2- (QSPI Databus)
		PSU_IOU_SLCR_MIO_PIN_2_L0_SEL                                                   1

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_2_L1_SEL                                                   0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[2]- (Test Scan Port) = test_scan, Outp
		t, test_scan_out[2]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_2_L2_SEL                                                   0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[2]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[2]- (GPIO bank 0) 1= can
		, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL signal
		 3= pjtag, Output, pjtag_tdo- (PJTAG TDO) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Master Selects) 5= ttc2, Input, ttc2_clk_in
		 (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Output, tracedq[0]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_2_L3_SEL                                                   0

		Configures MIO Pin 2 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180008, 0x000000FEU ,0x00000002U)
		RegMask = (IOU_SLCR_MIO_PIN_2_L0_SEL_MASK | IOU_SLCR_MIO_PIN_2_L1_SEL_MASK | IOU_SLCR_MIO_PIN_2_L2_SEL_MASK | IOU_SLCR_MIO_PIN_2_L3_SEL_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_MIO_PIN_2_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_2_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_2_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_2_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_2_OFFSET ,0x000000FEU ,0x00000002U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_3 @ 0XFF18000C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi3- (QSPI Databus) 1= qspi, Output, qspi_mo3- (QSPI Databus)
		PSU_IOU_SLCR_MIO_PIN_3_L0_SEL                                                   1

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_3_L1_SEL                                                   0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[3]- (Test Scan Port) = test_scan, Outp
		t, test_scan_out[3]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_3_L2_SEL                                                   0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[3]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[3]- (GPIO bank 0) 1= can
		, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA signa
		) 3= pjtag, Input, pjtag_tms- (PJTAG TMS) 4= spi0, Input, spi0_n_ss_in- (SPI Master Selects) 4= spi0, Output, spi0_n_ss_out[0
		- (SPI Master Selects) 5= ttc2, Output, ttc2_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial
		output) 7= trace, Output, tracedq[1]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_3_L3_SEL                                                   0

		Configures MIO Pin 3 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18000C, 0x000000FEU ,0x00000002U)
		RegMask = (IOU_SLCR_MIO_PIN_3_L0_SEL_MASK | IOU_SLCR_MIO_PIN_3_L1_SEL_MASK | IOU_SLCR_MIO_PIN_3_L2_SEL_MASK | IOU_SLCR_MIO_PIN_3_L3_SEL_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_MIO_PIN_3_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_3_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_3_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_3_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_3_OFFSET ,0x000000FEU ,0x00000002U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_4 @ 0XFF180010</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_mo_mo0- (QSPI Databus) 1= qspi, Input, qspi_si_mi0- (QSPI Data
		us)
		PSU_IOU_SLCR_MIO_PIN_4_L0_SEL                                                   1

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_4_L1_SEL                                                   0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[4]- (Test Scan Port) = test_scan, Outp
		t, test_scan_out[4]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_4_L2_SEL                                                   0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[4]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[4]- (GPIO bank 0) 1= can
		, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL signa
		) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= spi0, Output, spi0_s
		- (MISO signal) 5= ttc1, Input, ttc1_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace,
		utput, tracedq[2]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_4_L3_SEL                                                   0

		Configures MIO Pin 4 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180010, 0x000000FEU ,0x00000002U)
		RegMask = (IOU_SLCR_MIO_PIN_4_L0_SEL_MASK | IOU_SLCR_MIO_PIN_4_L1_SEL_MASK | IOU_SLCR_MIO_PIN_4_L2_SEL_MASK | IOU_SLCR_MIO_PIN_4_L3_SEL_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_MIO_PIN_4_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_4_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_4_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_4_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_4_OFFSET ,0x000000FEU ,0x00000002U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_5 @ 0XFF180014</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_n_ss_out- (QSPI Slave Select)
		PSU_IOU_SLCR_MIO_PIN_5_L0_SEL                                                   1

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_5_L1_SEL                                                   0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[5]- (Test Scan Port) = test_scan, Outp
		t, test_scan_out[5]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_5_L2_SEL                                                   0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[5]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[5]- (GPIO bank 0) 1= can
		, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA signal
		 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal) 4= spi0, Input, spi0
		si- (MOSI signal) 5= ttc1, Output, ttc1_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7
		 trace, Output, tracedq[3]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_5_L3_SEL                                                   0

		Configures MIO Pin 5 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180014, 0x000000FEU ,0x00000002U)
		RegMask = (IOU_SLCR_MIO_PIN_5_L0_SEL_MASK | IOU_SLCR_MIO_PIN_5_L1_SEL_MASK | IOU_SLCR_MIO_PIN_5_L2_SEL_MASK | IOU_SLCR_MIO_PIN_5_L3_SEL_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_MIO_PIN_5_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_5_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_5_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_5_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_5_OFFSET ,0x000000FEU ,0x00000002U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_6 @ 0XFF180018</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_clk_for_lpbk- (QSPI Clock to be fed-back)
		PSU_IOU_SLCR_MIO_PIN_6_L0_SEL                                                   1

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_6_L1_SEL                                                   0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[6]- (Test Scan Port) = test_scan, Outp
		t, test_scan_out[6]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_6_L2_SEL                                                   0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[6]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[6]- (GPIO bank 0) 1= can
		, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL signal
		 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4= spi1, Output, spi1
		sclk_out- (SPI Clock) 5= ttc0, Input, ttc0_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace,
		Output, tracedq[4]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_6_L3_SEL                                                   0

		Configures MIO Pin 6 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180018, 0x000000FEU ,0x00000002U)
		RegMask = (IOU_SLCR_MIO_PIN_6_L0_SEL_MASK | IOU_SLCR_MIO_PIN_6_L1_SEL_MASK | IOU_SLCR_MIO_PIN_6_L2_SEL_MASK | IOU_SLCR_MIO_PIN_6_L3_SEL_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_MIO_PIN_6_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_6_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_6_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_6_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_6_OFFSET ,0x000000FEU ,0x00000002U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_7 @ 0XFF18001C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_n_ss_out_upper- (QSPI Slave Select upper)
		PSU_IOU_SLCR_MIO_PIN_7_L0_SEL                                                   0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_7_L1_SEL                                                   0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[7]- (Test Scan Port) = test_scan, Outp
		t, test_scan_out[7]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_7_L2_SEL                                                   0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[7]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[7]- (GPIO bank 0) 1= can
		, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA signa
		) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi1, Output, spi1_n_ss_out[2]- (SPI Master Selects) 5=
		tc0, Output, ttc0_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial output) 7= trace, Output,
		racedq[5]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_7_L3_SEL                                                   0

		Configures MIO Pin 7 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18001C, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_7_L0_SEL_MASK | IOU_SLCR_MIO_PIN_7_L1_SEL_MASK | IOU_SLCR_MIO_PIN_7_L2_SEL_MASK | IOU_SLCR_MIO_PIN_7_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_7_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_7_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_7_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_7_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_7_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_8 @ 0XFF180020</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi_upper[0]- (QSPI Upper Databus) 1= qspi, Output, qspi_mo_uppe
		[0]- (QSPI Upper Databus)
		PSU_IOU_SLCR_MIO_PIN_8_L0_SEL                                                   0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_8_L1_SEL                                                   0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[8]- (Test Scan Port) = test_scan, Outp
		t, test_scan_out[8]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_8_L2_SEL                                                   0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[8]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[8]- (GPIO bank 0) 1= can
		, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL signa
		) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi1, Output, spi1_n_ss_out[1]- (SPI Master Selects) 5= ttc
		, Input, ttc3_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace, Output, tracedq[6]- (Tr
		ce Port Databus)
		PSU_IOU_SLCR_MIO_PIN_8_L3_SEL                                                   6

		Configures MIO Pin 8 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180020, 0x000000FEU ,0x000000C0U)
		RegMask = (IOU_SLCR_MIO_PIN_8_L0_SEL_MASK | IOU_SLCR_MIO_PIN_8_L1_SEL_MASK | IOU_SLCR_MIO_PIN_8_L2_SEL_MASK | IOU_SLCR_MIO_PIN_8_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_8_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_8_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_8_L2_SEL_SHIFT
			| 0x00000006U << IOU_SLCR_MIO_PIN_8_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_8_OFFSET ,0x000000FEU ,0x000000C0U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_9 @ 0XFF180024</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi_upper[1]- (QSPI Upper Databus) 1= qspi, Output, qspi_mo_uppe
		[1]- (QSPI Upper Databus)
		PSU_IOU_SLCR_MIO_PIN_9_L0_SEL                                                   0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_ce[1]- (NAND chip enable)
		PSU_IOU_SLCR_MIO_PIN_9_L1_SEL                                                   0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[9]- (Test Scan Port) = test_scan, Outp
		t, test_scan_out[9]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_9_L2_SEL                                                   0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[9]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[9]- (GPIO bank 0) 1= can
		, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA signal
		 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi1, Input, spi1_n_ss_in- (SPI Master Selects) 4= spi1,
		utput, spi1_n_ss_out[0]- (SPI Master Selects) 5= ttc3, Output, ttc3_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (U
		RT receiver serial input) 7= trace, Output, tracedq[7]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_9_L3_SEL                                                   6

		Configures MIO Pin 9 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180024, 0x000000FEU ,0x000000C0U)
		RegMask = (IOU_SLCR_MIO_PIN_9_L0_SEL_MASK | IOU_SLCR_MIO_PIN_9_L1_SEL_MASK | IOU_SLCR_MIO_PIN_9_L2_SEL_MASK | IOU_SLCR_MIO_PIN_9_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_9_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_9_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_9_L2_SEL_SHIFT
			| 0x00000006U << IOU_SLCR_MIO_PIN_9_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_9_OFFSET ,0x000000FEU ,0x000000C0U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_10 @ 0XFF180028</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi_upper[2]- (QSPI Upper Databus) 1= qspi, Output, qspi_mo_uppe
		[2]- (QSPI Upper Databus)
		PSU_IOU_SLCR_MIO_PIN_10_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_rb_n[0]- (NAND Ready/Busy)
		PSU_IOU_SLCR_MIO_PIN_10_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[10]- (Test Scan Port) = test_scan, Out
		ut, test_scan_out[10]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_10_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[10]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[10]- (GPIO bank 0) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= spi1, Output, spi1_
		o- (MISO signal) 5= ttc2, Input, ttc2_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Outp
		t, tracedq[8]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_10_L3_SEL                                                  2

		Configures MIO Pin 10 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180028, 0x000000FEU ,0x00000040U)
		RegMask = (IOU_SLCR_MIO_PIN_10_L0_SEL_MASK | IOU_SLCR_MIO_PIN_10_L1_SEL_MASK | IOU_SLCR_MIO_PIN_10_L2_SEL_MASK | IOU_SLCR_MIO_PIN_10_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_10_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_10_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_10_L2_SEL_SHIFT
			| 0x00000002U << IOU_SLCR_MIO_PIN_10_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_10_OFFSET ,0x000000FEU ,0x00000040U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_11 @ 0XFF18002C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi_upper[3]- (QSPI Upper Databus) 1= qspi, Output, qspi_mo_uppe
		[3]- (QSPI Upper Databus)
		PSU_IOU_SLCR_MIO_PIN_11_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_rb_n[1]- (NAND Ready/Busy)
		PSU_IOU_SLCR_MIO_PIN_11_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[11]- (Test Scan Port) = test_scan, Out
		ut, test_scan_out[11]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_11_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[11]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[11]- (GPIO bank 0) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal) 4= spi1, Input, s
		i1_si- (MOSI signal) 5= ttc2, Output, ttc2_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial o
		tput) 7= trace, Output, tracedq[9]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_11_L3_SEL                                                  2

		Configures MIO Pin 11 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18002C, 0x000000FEU ,0x00000040U)
		RegMask = (IOU_SLCR_MIO_PIN_11_L0_SEL_MASK | IOU_SLCR_MIO_PIN_11_L1_SEL_MASK | IOU_SLCR_MIO_PIN_11_L2_SEL_MASK | IOU_SLCR_MIO_PIN_11_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_11_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_11_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_11_L2_SEL_SHIFT
			| 0x00000002U << IOU_SLCR_MIO_PIN_11_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_11_OFFSET ,0x000000FEU ,0x00000040U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_12 @ 0XFF180030</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_sclk_out_upper- (QSPI Upper Clock)
		PSU_IOU_SLCR_MIO_PIN_12_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dqs_in- (NAND Strobe) 1= nand, Output, nfc_dqs_out- (NAND Strobe

		PSU_IOU_SLCR_MIO_PIN_12_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input, test_scan_in[12]- (Test Scan Port) = test_scan, Out
		ut, test_scan_out[12]- (Test Scan Port) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_12_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[12]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[12]- (GPIO bank 0) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= pjtag, Input, pjtag_tck- (PJTAG TCK) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4= spi0, Output, spi0_sclk_out- (SPI Cl
		ck) 5= ttc1, Input, ttc1_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace, Output, trac
		dq[10]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_12_L3_SEL                                                  0

		Configures MIO Pin 12 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180030, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_12_L0_SEL_MASK | IOU_SLCR_MIO_PIN_12_L1_SEL_MASK | IOU_SLCR_MIO_PIN_12_L2_SEL_MASK | IOU_SLCR_MIO_PIN_12_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_12_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_12_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_12_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_12_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_12_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_13 @ 0XFF180034</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_13_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_ce[0]- (NAND chip enable)
		PSU_IOU_SLCR_MIO_PIN_13_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[0]- (8-bit Data bus) = sd0, Output, sdio0_data_out[0]- (8
		bit Data bus) 2= test_scan, Input, test_scan_in[13]- (Test Scan Port) = test_scan, Output, test_scan_out[13]- (Test Scan Port
		 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_13_L2_SEL                                                  1

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[13]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[13]- (GPIO bank 0) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= pjtag, Input, pjtag_tdi- (PJTAG TDI) 4= spi0, Output, spi0_n_ss_out[2]- (SPI Master Selects) 5= ttc1, Output, ttc1_wave
		out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7= trace, Output, tracedq[11]- (Trace Port Dat
		bus)
		PSU_IOU_SLCR_MIO_PIN_13_L3_SEL                                                  0

		Configures MIO Pin 13 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180034, 0x000000FEU ,0x00000008U)
		RegMask = (IOU_SLCR_MIO_PIN_13_L0_SEL_MASK | IOU_SLCR_MIO_PIN_13_L1_SEL_MASK | IOU_SLCR_MIO_PIN_13_L2_SEL_MASK | IOU_SLCR_MIO_PIN_13_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_13_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_13_L1_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_13_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_13_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_13_OFFSET ,0x000000FEU ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_14 @ 0XFF180038</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_14_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_cle- (NAND Command Latch Enable)
		PSU_IOU_SLCR_MIO_PIN_14_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[1]- (8-bit Data bus) = sd0, Output, sdio0_data_out[1]- (8
		bit Data bus) 2= test_scan, Input, test_scan_in[14]- (Test Scan Port) = test_scan, Output, test_scan_out[14]- (Test Scan Port
		 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_14_L2_SEL                                                  1

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[14]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[14]- (GPIO bank 0) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= pjtag, Output, pjtag_tdo- (PJTAG TDO) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Master Selects) 5= ttc0, Input, ttc0_clk_
		n- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Output, tracedq[12]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_14_L3_SEL                                                  0

		Configures MIO Pin 14 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180038, 0x000000FEU ,0x00000008U)
		RegMask = (IOU_SLCR_MIO_PIN_14_L0_SEL_MASK | IOU_SLCR_MIO_PIN_14_L1_SEL_MASK | IOU_SLCR_MIO_PIN_14_L2_SEL_MASK | IOU_SLCR_MIO_PIN_14_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_14_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_14_L1_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_14_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_14_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_14_OFFSET ,0x000000FEU ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_15 @ 0XFF18003C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_15_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_ale- (NAND Address Latch Enable)
		PSU_IOU_SLCR_MIO_PIN_15_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[2]- (8-bit Data bus) = sd0, Output, sdio0_data_out[2]- (8
		bit Data bus) 2= test_scan, Input, test_scan_in[15]- (Test Scan Port) = test_scan, Output, test_scan_out[15]- (Test Scan Port
		 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_15_L2_SEL                                                  1

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[15]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[15]- (GPIO bank 0) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= pjtag, Input, pjtag_tms- (PJTAG TMS) 4= spi0, Input, spi0_n_ss_in- (SPI Master Selects) 4= spi0, Output, spi0_n_ss_out
		0]- (SPI Master Selects) 5= ttc0, Output, ttc0_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter seri
		l output) 7= trace, Output, tracedq[13]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_15_L3_SEL                                                  0

		Configures MIO Pin 15 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18003C, 0x000000FEU ,0x00000008U)
		RegMask = (IOU_SLCR_MIO_PIN_15_L0_SEL_MASK | IOU_SLCR_MIO_PIN_15_L1_SEL_MASK | IOU_SLCR_MIO_PIN_15_L2_SEL_MASK | IOU_SLCR_MIO_PIN_15_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_15_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_15_L1_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_15_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_15_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_15_OFFSET ,0x000000FEU ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_16 @ 0XFF180040</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_16_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[0]- (NAND Data Bus) 1= nand, Output, nfc_dq_out[0]- (NAND
		ata Bus)
		PSU_IOU_SLCR_MIO_PIN_16_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[3]- (8-bit Data bus) = sd0, Output, sdio0_data_out[3]- (8
		bit Data bus) 2= test_scan, Input, test_scan_in[16]- (Test Scan Port) = test_scan, Output, test_scan_out[16]- (Test Scan Port
		 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_16_L2_SEL                                                  1

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[16]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[16]- (GPIO bank 0) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= spi0, Output, spi0
		so- (MISO signal) 5= ttc3, Input, ttc3_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace
		 Output, tracedq[14]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_16_L3_SEL                                                  0

		Configures MIO Pin 16 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180040, 0x000000FEU ,0x00000008U)
		RegMask = (IOU_SLCR_MIO_PIN_16_L0_SEL_MASK | IOU_SLCR_MIO_PIN_16_L1_SEL_MASK | IOU_SLCR_MIO_PIN_16_L2_SEL_MASK | IOU_SLCR_MIO_PIN_16_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_16_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_16_L1_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_16_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_16_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_16_OFFSET ,0x000000FEU ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_17 @ 0XFF180044</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_17_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[1]- (NAND Data Bus) 1= nand, Output, nfc_dq_out[1]- (NAND
		ata Bus)
		PSU_IOU_SLCR_MIO_PIN_17_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[4]- (8-bit Data bus) = sd0, Output, sdio0_data_out[4]- (8
		bit Data bus) 2= test_scan, Input, test_scan_in[17]- (Test Scan Port) = test_scan, Output, test_scan_out[17]- (Test Scan Port
		 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_17_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[17]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[17]- (GPIO bank 0) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal) 4= spi0, Input, sp
		0_si- (MOSI signal) 5= ttc3, Output, ttc3_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input)
		7= trace, Output, tracedq[15]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_17_L3_SEL                                                  0

		Configures MIO Pin 17 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180044, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_17_L0_SEL_MASK | IOU_SLCR_MIO_PIN_17_L1_SEL_MASK | IOU_SLCR_MIO_PIN_17_L2_SEL_MASK | IOU_SLCR_MIO_PIN_17_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_17_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_17_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_17_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_17_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_17_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_18 @ 0XFF180048</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_18_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[2]- (NAND Data Bus) 1= nand, Output, nfc_dq_out[2]- (NAND
		ata Bus)
		PSU_IOU_SLCR_MIO_PIN_18_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[5]- (8-bit Data bus) = sd0, Output, sdio0_data_out[5]- (8
		bit Data bus) 2= test_scan, Input, test_scan_in[18]- (Test Scan Port) = test_scan, Output, test_scan_out[18]- (Test Scan Port
		 3= csu, Input, csu_ext_tamper- (CSU Ext Tamper)
		PSU_IOU_SLCR_MIO_PIN_18_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[18]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[18]- (GPIO bank 0) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= spi1, Output, spi1_
		o- (MISO signal) 5= ttc2, Input, ttc2_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_18_L3_SEL                                                  0

		Configures MIO Pin 18 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180048, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_18_L0_SEL_MASK | IOU_SLCR_MIO_PIN_18_L1_SEL_MASK | IOU_SLCR_MIO_PIN_18_L2_SEL_MASK | IOU_SLCR_MIO_PIN_18_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_18_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_18_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_18_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_18_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_18_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_19 @ 0XFF18004C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_19_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[3]- (NAND Data Bus) 1= nand, Output, nfc_dq_out[3]- (NAND
		ata Bus)
		PSU_IOU_SLCR_MIO_PIN_19_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[6]- (8-bit Data bus) = sd0, Output, sdio0_data_out[6]- (8
		bit Data bus) 2= test_scan, Input, test_scan_in[19]- (Test Scan Port) = test_scan, Output, test_scan_out[19]- (Test Scan Port
		 3= csu, Input, csu_ext_tamper- (CSU Ext Tamper)
		PSU_IOU_SLCR_MIO_PIN_19_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[19]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[19]- (GPIO bank 0) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi1, Output, spi1_n_ss_out[2]- (SPI Master Selects) 5
		 ttc2, Output, ttc2_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial output) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_19_L3_SEL                                                  0

		Configures MIO Pin 19 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18004C, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_19_L0_SEL_MASK | IOU_SLCR_MIO_PIN_19_L1_SEL_MASK | IOU_SLCR_MIO_PIN_19_L2_SEL_MASK | IOU_SLCR_MIO_PIN_19_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_19_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_19_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_19_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_19_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_19_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_20 @ 0XFF180050</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_20_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[4]- (NAND Data Bus) 1= nand, Output, nfc_dq_out[4]- (NAND
		ata Bus)
		PSU_IOU_SLCR_MIO_PIN_20_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[7]- (8-bit Data bus) = sd0, Output, sdio0_data_out[7]- (8
		bit Data bus) 2= test_scan, Input, test_scan_in[20]- (Test Scan Port) = test_scan, Output, test_scan_out[20]- (Test Scan Port
		 3= csu, Input, csu_ext_tamper- (CSU Ext Tamper)
		PSU_IOU_SLCR_MIO_PIN_20_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[20]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[20]- (GPIO bank 0) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi1, Output, spi1_n_ss_out[1]- (SPI Master Selects) 5= t
		c1, Input, ttc1_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_20_L3_SEL                                                  0

		Configures MIO Pin 20 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180050, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_20_L0_SEL_MASK | IOU_SLCR_MIO_PIN_20_L1_SEL_MASK | IOU_SLCR_MIO_PIN_20_L2_SEL_MASK | IOU_SLCR_MIO_PIN_20_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_20_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_20_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_20_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_20_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_20_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_21 @ 0XFF180054</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_21_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[5]- (NAND Data Bus) 1= nand, Output, nfc_dq_out[5]- (NAND
		ata Bus)
		PSU_IOU_SLCR_MIO_PIN_21_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_cmd_in- (Command Indicator) = sd0, Output, sdio0_cmd_out- (Comman
		 Indicator) 2= test_scan, Input, test_scan_in[21]- (Test Scan Port) = test_scan, Output, test_scan_out[21]- (Test Scan Port)
		= csu, Input, csu_ext_tamper- (CSU Ext Tamper)
		PSU_IOU_SLCR_MIO_PIN_21_L2_SEL                                                  1

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[21]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[21]- (GPIO bank 0) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi1, Input, spi1_n_ss_in- (SPI Master Selects) 4= spi1
		 Output, spi1_n_ss_out[0]- (SPI Master Selects) 5= ttc1, Output, ttc1_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd-
		UART receiver serial input) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_21_L3_SEL                                                  0

		Configures MIO Pin 21 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180054, 0x000000FEU ,0x00000008U)
		RegMask = (IOU_SLCR_MIO_PIN_21_L0_SEL_MASK | IOU_SLCR_MIO_PIN_21_L1_SEL_MASK | IOU_SLCR_MIO_PIN_21_L2_SEL_MASK | IOU_SLCR_MIO_PIN_21_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_21_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_21_L1_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_21_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_21_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_21_OFFSET ,0x000000FEU ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_22 @ 0XFF180058</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_22_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_we_b- (NAND Write Enable)
		PSU_IOU_SLCR_MIO_PIN_22_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_clk_out- (SDSDIO clock) 2= test_scan, Input, test_scan_in[22]-
		(Test Scan Port) = test_scan, Output, test_scan_out[22]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (CSU Ext Tamper)
		PSU_IOU_SLCR_MIO_PIN_22_L2_SEL                                                  1

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[22]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[22]- (GPIO bank 0) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4= spi1, Output, sp
		1_sclk_out- (SPI Clock) 5= ttc0, Input, ttc0_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= Not
		sed
		PSU_IOU_SLCR_MIO_PIN_22_L3_SEL                                                  0

		Configures MIO Pin 22 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180058, 0x000000FEU ,0x00000008U)
		RegMask = (IOU_SLCR_MIO_PIN_22_L0_SEL_MASK | IOU_SLCR_MIO_PIN_22_L1_SEL_MASK | IOU_SLCR_MIO_PIN_22_L2_SEL_MASK | IOU_SLCR_MIO_PIN_22_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_22_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_22_L1_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_22_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_22_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_22_OFFSET ,0x000000FEU ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_23 @ 0XFF18005C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_23_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[6]- (NAND Data Bus) 1= nand, Output, nfc_dq_out[6]- (NAND
		ata Bus)
		PSU_IOU_SLCR_MIO_PIN_23_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_bus_pow- (SD card bus power) 2= test_scan, Input, test_scan_in
		23]- (Test Scan Port) = test_scan, Output, test_scan_out[23]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (CSU Ext Tamper

		PSU_IOU_SLCR_MIO_PIN_23_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[23]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[23]- (GPIO bank 0) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal) 4= spi1, Input, s
		i1_si- (MOSI signal) 5= ttc0, Output, ttc0_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial o
		tput) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_23_L3_SEL                                                  0

		Configures MIO Pin 23 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18005C, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_23_L0_SEL_MASK | IOU_SLCR_MIO_PIN_23_L1_SEL_MASK | IOU_SLCR_MIO_PIN_23_L2_SEL_MASK | IOU_SLCR_MIO_PIN_23_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_23_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_23_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_23_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_23_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_23_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_24 @ 0XFF180060</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_24_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[7]- (NAND Data Bus) 1= nand, Output, nfc_dq_out[7]- (NAND
		ata Bus)
		PSU_IOU_SLCR_MIO_PIN_24_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_cd_n- (SD card detect from connector) 2= test_scan, Input, test
		scan_in[24]- (Test Scan Port) = test_scan, Output, test_scan_out[24]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (CSU Ex
		 Tamper)
		PSU_IOU_SLCR_MIO_PIN_24_L2_SEL                                                  1

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[24]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[24]- (GPIO bank 0) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= Not Used 5= ttc3, Input, ttc3_clk_in- (TTC Clock) 6= ua1,
		Output, ua1_txd- (UART transmitter serial output) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_24_L3_SEL                                                  0

		Configures MIO Pin 24 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180060, 0x000000FEU ,0x00000008U)
		RegMask = (IOU_SLCR_MIO_PIN_24_L0_SEL_MASK | IOU_SLCR_MIO_PIN_24_L1_SEL_MASK | IOU_SLCR_MIO_PIN_24_L2_SEL_MASK | IOU_SLCR_MIO_PIN_24_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_24_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_24_L1_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_24_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_24_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_24_OFFSET ,0x000000FEU ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_25 @ 0XFF180064</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_25_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_re_n- (NAND Read Enable)
		PSU_IOU_SLCR_MIO_PIN_25_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_wp- (SD card write protect from connector) 2= test_scan, Input,
		test_scan_in[25]- (Test Scan Port) = test_scan, Output, test_scan_out[25]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (C
		U Ext Tamper)
		PSU_IOU_SLCR_MIO_PIN_25_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[25]- (GPIO bank 0) 0= gpio0, Output, gpio_0_pin_out[25]- (GPIO bank 0) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= Not Used 5= ttc3, Output, ttc3_wave_out- (TTC Waveform
		lock) 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_25_L3_SEL                                                  0

		Configures MIO Pin 25 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180064, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_25_L0_SEL_MASK | IOU_SLCR_MIO_PIN_25_L1_SEL_MASK | IOU_SLCR_MIO_PIN_25_L2_SEL_MASK | IOU_SLCR_MIO_PIN_25_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_25_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_25_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_25_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_25_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_25_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_26 @ 0XFF180068</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_tx_clk- (TX RGMII clock)
		PSU_IOU_SLCR_MIO_PIN_26_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_ce[1]- (NAND chip enable)
		PSU_IOU_SLCR_MIO_PIN_26_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[0]- (PMU GPI) 2= test_scan, Input, test_scan_in[26]- (Test Sc
		n Port) = test_scan, Output, test_scan_out[26]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (CSU Ext Tamper)
		PSU_IOU_SLCR_MIO_PIN_26_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[0]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[0]- (GPIO bank 1) 1= can
		, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL signal
		 3= pjtag, Input, pjtag_tck- (PJTAG TCK) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4= spi0, Output, spi0_sclk_out- (SPI Clock
		 5= ttc2, Input, ttc2_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Output, tracedq[4]-
		Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_26_L3_SEL                                                  0

		Configures MIO Pin 26 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180068, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_26_L0_SEL_MASK | IOU_SLCR_MIO_PIN_26_L1_SEL_MASK | IOU_SLCR_MIO_PIN_26_L2_SEL_MASK | IOU_SLCR_MIO_PIN_26_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_26_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_26_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_26_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_26_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_26_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_27 @ 0XFF18006C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_txd[0]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_27_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_rb_n[0]- (NAND Ready/Busy)
		PSU_IOU_SLCR_MIO_PIN_27_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[1]- (PMU GPI) 2= test_scan, Input, test_scan_in[27]- (Test Sc
		n Port) = test_scan, Output, test_scan_out[27]- (Test Scan Port) 3= dpaux, Input, dp_aux_data_in- (Dp Aux Data) = dpaux, Outp
		t, dp_aux_data_out- (Dp Aux Data)
		PSU_IOU_SLCR_MIO_PIN_27_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[1]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[1]- (GPIO bank 1) 1= can
		, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA signa
		) 3= pjtag, Input, pjtag_tdi- (PJTAG TDI) 4= spi0, Output, spi0_n_ss_out[2]- (SPI Master Selects) 5= ttc2, Output, ttc2_wave_
		ut- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial output) 7= trace, Output, tracedq[5]- (Trace Port
		atabus)
		PSU_IOU_SLCR_MIO_PIN_27_L3_SEL                                                  0

		Configures MIO Pin 27 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18006C, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_27_L0_SEL_MASK | IOU_SLCR_MIO_PIN_27_L1_SEL_MASK | IOU_SLCR_MIO_PIN_27_L2_SEL_MASK | IOU_SLCR_MIO_PIN_27_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_27_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_27_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_27_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_27_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_27_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_28 @ 0XFF180070</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_txd[1]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_28_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_rb_n[1]- (NAND Ready/Busy)
		PSU_IOU_SLCR_MIO_PIN_28_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[2]- (PMU GPI) 2= test_scan, Input, test_scan_in[28]- (Test Sc
		n Port) = test_scan, Output, test_scan_out[28]- (Test Scan Port) 3= dpaux, Input, dp_hot_plug_detect- (Dp Aux Hot Plug)
		PSU_IOU_SLCR_MIO_PIN_28_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[2]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[2]- (GPIO bank 1) 1= can
		, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL signa
		) 3= pjtag, Output, pjtag_tdo- (PJTAG TDO) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Master Selects) 5= ttc1, Input, ttc1_clk_i
		- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace, Output, tracedq[6]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_28_L3_SEL                                                  0

		Configures MIO Pin 28 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180070, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_28_L0_SEL_MASK | IOU_SLCR_MIO_PIN_28_L1_SEL_MASK | IOU_SLCR_MIO_PIN_28_L2_SEL_MASK | IOU_SLCR_MIO_PIN_28_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_28_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_28_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_28_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_28_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_28_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_29 @ 0XFF180074</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_txd[2]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_29_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (PCIE Reset signal)
		PSU_IOU_SLCR_MIO_PIN_29_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[3]- (PMU GPI) 2= test_scan, Input, test_scan_in[29]- (Test Sc
		n Port) = test_scan, Output, test_scan_out[29]- (Test Scan Port) 3= dpaux, Input, dp_aux_data_in- (Dp Aux Data) = dpaux, Outp
		t, dp_aux_data_out- (Dp Aux Data)
		PSU_IOU_SLCR_MIO_PIN_29_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[3]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[3]- (GPIO bank 1) 1= can
		, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA signal
		 3= pjtag, Input, pjtag_tms- (PJTAG TMS) 4= spi0, Input, spi0_n_ss_in- (SPI Master Selects) 4= spi0, Output, spi0_n_ss_out[0]
		 (SPI Master Selects) 5= ttc1, Output, ttc1_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial inpu
		) 7= trace, Output, tracedq[7]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_29_L3_SEL                                                  0

		Configures MIO Pin 29 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180074, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_29_L0_SEL_MASK | IOU_SLCR_MIO_PIN_29_L1_SEL_MASK | IOU_SLCR_MIO_PIN_29_L2_SEL_MASK | IOU_SLCR_MIO_PIN_29_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_29_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_29_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_29_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_29_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_29_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_30 @ 0XFF180078</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_txd[3]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_30_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (PCIE Reset signal)
		PSU_IOU_SLCR_MIO_PIN_30_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[4]- (PMU GPI) 2= test_scan, Input, test_scan_in[30]- (Test Sc
		n Port) = test_scan, Output, test_scan_out[30]- (Test Scan Port) 3= dpaux, Input, dp_hot_plug_detect- (Dp Aux Hot Plug)
		PSU_IOU_SLCR_MIO_PIN_30_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[4]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[4]- (GPIO bank 1) 1= can
		, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL signal
		 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= spi0, Output, spi0_so
		 (MISO signal) 5= ttc0, Input, ttc0_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Output
		 tracedq[8]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_30_L3_SEL                                                  0

		Configures MIO Pin 30 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180078, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_30_L0_SEL_MASK | IOU_SLCR_MIO_PIN_30_L1_SEL_MASK | IOU_SLCR_MIO_PIN_30_L2_SEL_MASK | IOU_SLCR_MIO_PIN_30_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_30_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_30_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_30_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_30_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_30_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_31 @ 0XFF18007C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_tx_ctl- (TX RGMII control)
		PSU_IOU_SLCR_MIO_PIN_31_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (PCIE Reset signal)
		PSU_IOU_SLCR_MIO_PIN_31_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[5]- (PMU GPI) 2= test_scan, Input, test_scan_in[31]- (Test Sc
		n Port) = test_scan, Output, test_scan_out[31]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (CSU Ext Tamper)
		PSU_IOU_SLCR_MIO_PIN_31_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[5]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[5]- (GPIO bank 1) 1= can
		, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA signa
		) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal) 4= spi0, Input, spi
		_si- (MOSI signal) 5= ttc0, Output, ttc0_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial out
		ut) 7= trace, Output, tracedq[9]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_31_L3_SEL                                                  0

		Configures MIO Pin 31 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18007C, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_31_L0_SEL_MASK | IOU_SLCR_MIO_PIN_31_L1_SEL_MASK | IOU_SLCR_MIO_PIN_31_L2_SEL_MASK | IOU_SLCR_MIO_PIN_31_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_31_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_31_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_31_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_31_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_31_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_32 @ 0XFF180080</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Input, gem0_rgmii_rx_clk- (RX RGMII clock)
		PSU_IOU_SLCR_MIO_PIN_32_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dqs_in- (NAND Strobe) 1= nand, Output, nfc_dqs_out- (NAND Strobe

		PSU_IOU_SLCR_MIO_PIN_32_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Output, pmu_gpo[0]- (PMU GPI) 2= test_scan, Input, test_scan_in[32]- (Test S
		an Port) = test_scan, Output, test_scan_out[32]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (CSU Ext Tamper)
		PSU_IOU_SLCR_MIO_PIN_32_L2_SEL                                                  1

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[6]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[6]- (GPIO bank 1) 1= can
		, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL signa
		) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4= spi1, Output, spi
		_sclk_out- (SPI Clock) 5= ttc3, Input, ttc3_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7=
		race, Output, tracedq[10]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_32_L3_SEL                                                  0

		Configures MIO Pin 32 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180080, 0x000000FEU ,0x00000008U)
		RegMask = (IOU_SLCR_MIO_PIN_32_L0_SEL_MASK | IOU_SLCR_MIO_PIN_32_L1_SEL_MASK | IOU_SLCR_MIO_PIN_32_L2_SEL_MASK | IOU_SLCR_MIO_PIN_32_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_32_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_32_L1_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_32_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_32_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_32_OFFSET ,0x000000FEU ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_33 @ 0XFF180084</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Input, gem0_rgmii_rxd[0]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_33_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (PCIE Reset signal)
		PSU_IOU_SLCR_MIO_PIN_33_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Output, pmu_gpo[1]- (PMU GPI) 2= test_scan, Input, test_scan_in[33]- (Test S
		an Port) = test_scan, Output, test_scan_out[33]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (CSU Ext Tamper)
		PSU_IOU_SLCR_MIO_PIN_33_L2_SEL                                                  1

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[7]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[7]- (GPIO bank 1) 1= can
		, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA signal
		 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi1, Output, spi1_n_ss_out[2]- (SPI Master Selects) 5= t
		c3, Output, ttc3_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7= trace, Output, traced
		[11]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_33_L3_SEL                                                  0

		Configures MIO Pin 33 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180084, 0x000000FEU ,0x00000008U)
		RegMask = (IOU_SLCR_MIO_PIN_33_L0_SEL_MASK | IOU_SLCR_MIO_PIN_33_L1_SEL_MASK | IOU_SLCR_MIO_PIN_33_L2_SEL_MASK | IOU_SLCR_MIO_PIN_33_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_33_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_33_L1_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_33_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_33_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_33_OFFSET ,0x000000FEU ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_34 @ 0XFF180088</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Input, gem0_rgmii_rxd[1]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_34_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (PCIE Reset signal)
		PSU_IOU_SLCR_MIO_PIN_34_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Output, pmu_gpo[2]- (PMU GPI) 2= test_scan, Input, test_scan_in[34]- (Test S
		an Port) = test_scan, Output, test_scan_out[34]- (Test Scan Port) 3= dpaux, Input, dp_aux_data_in- (Dp Aux Data) = dpaux, Out
		ut, dp_aux_data_out- (Dp Aux Data)
		PSU_IOU_SLCR_MIO_PIN_34_L2_SEL                                                  3

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[8]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[8]- (GPIO bank 1) 1= can
		, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL signal
		 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi1, Output, spi1_n_ss_out[1]- (SPI Master Selects) 5= ttc2
		 Input, ttc2_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Output, tracedq[12]- (Trace P
		rt Databus)
		PSU_IOU_SLCR_MIO_PIN_34_L3_SEL                                                  0

		Configures MIO Pin 34 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180088, 0x000000FEU ,0x00000018U)
		RegMask = (IOU_SLCR_MIO_PIN_34_L0_SEL_MASK | IOU_SLCR_MIO_PIN_34_L1_SEL_MASK | IOU_SLCR_MIO_PIN_34_L2_SEL_MASK | IOU_SLCR_MIO_PIN_34_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_34_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_34_L1_SEL_SHIFT
			| 0x00000003U << IOU_SLCR_MIO_PIN_34_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_34_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_34_OFFSET ,0x000000FEU ,0x00000018U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_35 @ 0XFF18008C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Input, gem0_rgmii_rxd[2]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_35_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (PCIE Reset signal)
		PSU_IOU_SLCR_MIO_PIN_35_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Output, pmu_gpo[3]- (PMU GPI) 2= test_scan, Input, test_scan_in[35]- (Test S
		an Port) = test_scan, Output, test_scan_out[35]- (Test Scan Port) 3= dpaux, Input, dp_hot_plug_detect- (Dp Aux Hot Plug)
		PSU_IOU_SLCR_MIO_PIN_35_L2_SEL                                                  3

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[9]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[9]- (GPIO bank 1) 1= can
		, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA signa
		) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi1, Input, spi1_n_ss_in- (SPI Master Selects) 4= spi1,
		Output, spi1_n_ss_out[0]- (SPI Master Selects) 5= ttc2, Output, ttc2_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd-
		UART transmitter serial output) 7= trace, Output, tracedq[13]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_35_L3_SEL                                                  0

		Configures MIO Pin 35 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18008C, 0x000000FEU ,0x00000018U)
		RegMask = (IOU_SLCR_MIO_PIN_35_L0_SEL_MASK | IOU_SLCR_MIO_PIN_35_L1_SEL_MASK | IOU_SLCR_MIO_PIN_35_L2_SEL_MASK | IOU_SLCR_MIO_PIN_35_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_35_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_35_L1_SEL_SHIFT
			| 0x00000003U << IOU_SLCR_MIO_PIN_35_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_35_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_35_OFFSET ,0x000000FEU ,0x00000018U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_36 @ 0XFF180090</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Input, gem0_rgmii_rxd[3]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_36_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (PCIE Reset signal)
		PSU_IOU_SLCR_MIO_PIN_36_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Output, pmu_gpo[4]- (PMU GPI) 2= test_scan, Input, test_scan_in[36]- (Test S
		an Port) = test_scan, Output, test_scan_out[36]- (Test Scan Port) 3= dpaux, Input, dp_aux_data_in- (Dp Aux Data) = dpaux, Out
		ut, dp_aux_data_out- (Dp Aux Data)
		PSU_IOU_SLCR_MIO_PIN_36_L2_SEL                                                  3

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[10]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[10]- (GPIO bank 1) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= spi1, Output, spi1
		so- (MISO signal) 5= ttc1, Input, ttc1_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace
		 Output, tracedq[14]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_36_L3_SEL                                                  0

		Configures MIO Pin 36 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180090, 0x000000FEU ,0x00000018U)
		RegMask = (IOU_SLCR_MIO_PIN_36_L0_SEL_MASK | IOU_SLCR_MIO_PIN_36_L1_SEL_MASK | IOU_SLCR_MIO_PIN_36_L2_SEL_MASK | IOU_SLCR_MIO_PIN_36_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_36_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_36_L1_SEL_SHIFT
			| 0x00000003U << IOU_SLCR_MIO_PIN_36_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_36_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_36_OFFSET ,0x000000FEU ,0x00000018U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_37 @ 0XFF180094</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Input, gem0_rgmii_rx_ctl- (RX RGMII control )
		PSU_IOU_SLCR_MIO_PIN_37_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (PCIE Reset signal)
		PSU_IOU_SLCR_MIO_PIN_37_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Output, pmu_gpo[5]- (PMU GPI) 2= test_scan, Input, test_scan_in[37]- (Test S
		an Port) = test_scan, Output, test_scan_out[37]- (Test Scan Port) 3= dpaux, Input, dp_hot_plug_detect- (Dp Aux Hot Plug)
		PSU_IOU_SLCR_MIO_PIN_37_L2_SEL                                                  3

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[11]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[11]- (GPIO bank 1) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal) 4= spi1, Input, sp
		1_si- (MOSI signal) 5= ttc1, Output, ttc1_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input)
		7= trace, Output, tracedq[15]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_37_L3_SEL                                                  0

		Configures MIO Pin 37 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180094, 0x000000FEU ,0x00000018U)
		RegMask = (IOU_SLCR_MIO_PIN_37_L0_SEL_MASK | IOU_SLCR_MIO_PIN_37_L1_SEL_MASK | IOU_SLCR_MIO_PIN_37_L2_SEL_MASK | IOU_SLCR_MIO_PIN_37_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_37_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_37_L1_SEL_SHIFT
			| 0x00000003U << IOU_SLCR_MIO_PIN_37_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_37_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_37_OFFSET ,0x000000FEU ,0x00000018U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_38 @ 0XFF180098</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_tx_clk- (TX RGMII clock)
		PSU_IOU_SLCR_MIO_PIN_38_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_38_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_clk_out- (SDSDIO clock) 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_38_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[12]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[12]- (GPIO bank 1) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= pjtag, Input, pjtag_tck- (PJTAG TCK) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4= spi0, Output, spi0_sclk_out- (SPI Clo
		k) 5= ttc0, Input, ttc0_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Output, trace_clk-
		(Trace Port Clock)
		PSU_IOU_SLCR_MIO_PIN_38_L3_SEL                                                  4

		Configures MIO Pin 38 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180098, 0x000000FEU ,0x00000080U)
		RegMask = (IOU_SLCR_MIO_PIN_38_L0_SEL_MASK | IOU_SLCR_MIO_PIN_38_L1_SEL_MASK | IOU_SLCR_MIO_PIN_38_L2_SEL_MASK | IOU_SLCR_MIO_PIN_38_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_38_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_38_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_38_L2_SEL_SHIFT
			| 0x00000004U << IOU_SLCR_MIO_PIN_38_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_38_OFFSET ,0x000000FEU ,0x00000080U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_39 @ 0XFF18009C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_txd[0]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_39_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_39_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_cd_n- (SD card detect from connector) 2= sd1, Input, sd1_data_i
		[4]- (8-bit Data bus) = sd1, Output, sdio1_data_out[4]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_39_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[13]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[13]- (GPIO bank 1) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= pjtag, Input, pjtag_tdi- (PJTAG TDI) 4= spi0, Output, spi0_n_ss_out[2]- (SPI Master Selects) 5= ttc0, Output, ttc0_wav
		_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial output) 7= trace, Output, trace_ctl- (Trace Port
		Control Signal)
		PSU_IOU_SLCR_MIO_PIN_39_L3_SEL                                                  0

		Configures MIO Pin 39 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18009C, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_39_L0_SEL_MASK | IOU_SLCR_MIO_PIN_39_L1_SEL_MASK | IOU_SLCR_MIO_PIN_39_L2_SEL_MASK | IOU_SLCR_MIO_PIN_39_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_39_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_39_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_39_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_39_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_39_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_40 @ 0XFF1800A0</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_txd[1]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_40_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_40_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_cmd_in- (Command Indicator) = sd0, Output, sdio0_cmd_out- (Comman
		 Indicator) 2= sd1, Input, sd1_data_in[5]- (8-bit Data bus) = sd1, Output, sdio1_data_out[5]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_40_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[14]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[14]- (GPIO bank 1) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= pjtag, Output, pjtag_tdo- (PJTAG TDO) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Master Selects) 5= ttc3, Input, ttc3_clk
		in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace, Output, tracedq[0]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_40_L3_SEL                                                  4

		Configures MIO Pin 40 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800A0, 0x000000FEU ,0x00000080U)
		RegMask = (IOU_SLCR_MIO_PIN_40_L0_SEL_MASK | IOU_SLCR_MIO_PIN_40_L1_SEL_MASK | IOU_SLCR_MIO_PIN_40_L2_SEL_MASK | IOU_SLCR_MIO_PIN_40_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_40_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_40_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_40_L2_SEL_SHIFT
			| 0x00000004U << IOU_SLCR_MIO_PIN_40_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_40_OFFSET ,0x000000FEU ,0x00000080U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_41 @ 0XFF1800A4</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_txd[2]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_41_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_41_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[0]- (8-bit Data bus) = sd0, Output, sdio0_data_out[0]- (8
		bit Data bus) 2= sd1, Input, sd1_data_in[6]- (8-bit Data bus) = sd1, Output, sdio1_data_out[6]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_41_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[15]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[15]- (GPIO bank 1) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= pjtag, Input, pjtag_tms- (PJTAG TMS) 4= spi0, Input, spi0_n_ss_in- (SPI Master Selects) 4= spi0, Output, spi0_n_ss_out[
		]- (SPI Master Selects) 5= ttc3, Output, ttc3_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial in
		ut) 7= trace, Output, tracedq[1]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_41_L3_SEL                                                  4

		Configures MIO Pin 41 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800A4, 0x000000FEU ,0x00000080U)
		RegMask = (IOU_SLCR_MIO_PIN_41_L0_SEL_MASK | IOU_SLCR_MIO_PIN_41_L1_SEL_MASK | IOU_SLCR_MIO_PIN_41_L2_SEL_MASK | IOU_SLCR_MIO_PIN_41_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_41_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_41_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_41_L2_SEL_SHIFT
			| 0x00000004U << IOU_SLCR_MIO_PIN_41_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_41_OFFSET ,0x000000FEU ,0x00000080U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_42 @ 0XFF1800A8</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_txd[3]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_42_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_42_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[1]- (8-bit Data bus) = sd0, Output, sdio0_data_out[1]- (8
		bit Data bus) 2= sd1, Input, sd1_data_in[7]- (8-bit Data bus) = sd1, Output, sdio1_data_out[7]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_42_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[16]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[16]- (GPIO bank 1) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= spi0, Output, spi0_
		o- (MISO signal) 5= ttc2, Input, ttc2_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Outp
		t, tracedq[2]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_42_L3_SEL                                                  4

		Configures MIO Pin 42 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800A8, 0x000000FEU ,0x00000080U)
		RegMask = (IOU_SLCR_MIO_PIN_42_L0_SEL_MASK | IOU_SLCR_MIO_PIN_42_L1_SEL_MASK | IOU_SLCR_MIO_PIN_42_L2_SEL_MASK | IOU_SLCR_MIO_PIN_42_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_42_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_42_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_42_L2_SEL_SHIFT
			| 0x00000004U << IOU_SLCR_MIO_PIN_42_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_42_OFFSET ,0x000000FEU ,0x00000080U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_43 @ 0XFF1800AC</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_tx_ctl- (TX RGMII control)
		PSU_IOU_SLCR_MIO_PIN_43_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_43_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[2]- (8-bit Data bus) = sd0, Output, sdio0_data_out[2]- (8
		bit Data bus) 2= sd1, Output, sdio1_bus_pow- (SD card bus power) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_43_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[17]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[17]- (GPIO bank 1) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal) 4= spi0, Input, s
		i0_si- (MOSI signal) 5= ttc2, Output, ttc2_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial o
		tput) 7= trace, Output, tracedq[3]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_43_L3_SEL                                                  4

		Configures MIO Pin 43 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800AC, 0x000000FEU ,0x00000080U)
		RegMask = (IOU_SLCR_MIO_PIN_43_L0_SEL_MASK | IOU_SLCR_MIO_PIN_43_L1_SEL_MASK | IOU_SLCR_MIO_PIN_43_L2_SEL_MASK | IOU_SLCR_MIO_PIN_43_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_43_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_43_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_43_L2_SEL_SHIFT
			| 0x00000004U << IOU_SLCR_MIO_PIN_43_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_43_OFFSET ,0x000000FEU ,0x00000080U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_44 @ 0XFF1800B0</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rx_clk- (RX RGMII clock)
		PSU_IOU_SLCR_MIO_PIN_44_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_44_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[3]- (8-bit Data bus) = sd0, Output, sdio0_data_out[3]- (8
		bit Data bus) 2= sd1, Input, sdio1_wp- (SD card write protect from connector) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_44_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[18]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[18]- (GPIO bank 1) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4= spi1, Output, s
		i1_sclk_out- (SPI Clock) 5= ttc1, Input, ttc1_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7
		 Not Used
		PSU_IOU_SLCR_MIO_PIN_44_L3_SEL                                                  0

		Configures MIO Pin 44 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800B0, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_44_L0_SEL_MASK | IOU_SLCR_MIO_PIN_44_L1_SEL_MASK | IOU_SLCR_MIO_PIN_44_L2_SEL_MASK | IOU_SLCR_MIO_PIN_44_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_44_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_44_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_44_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_44_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_44_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_45 @ 0XFF1800B4</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rxd[0]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_45_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_45_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[4]- (8-bit Data bus) = sd0, Output, sdio0_data_out[4]- (8
		bit Data bus) 2= sd1, Input, sdio1_cd_n- (SD card detect from connector) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_45_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[19]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[19]- (GPIO bank 1) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi1, Output, spi1_n_ss_out[2]- (SPI Master Selects) 5=
		ttc1, Output, ttc1_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_45_L3_SEL                                                  0

		Configures MIO Pin 45 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800B4, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_45_L0_SEL_MASK | IOU_SLCR_MIO_PIN_45_L1_SEL_MASK | IOU_SLCR_MIO_PIN_45_L2_SEL_MASK | IOU_SLCR_MIO_PIN_45_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_45_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_45_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_45_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_45_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_45_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_46 @ 0XFF1800B8</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rxd[1]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_46_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_46_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[5]- (8-bit Data bus) = sd0, Output, sdio0_data_out[5]- (8
		bit Data bus) 2= sd1, Input, sd1_data_in[0]- (8-bit Data bus) = sd1, Output, sdio1_data_out[0]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_46_L2_SEL                                                  2

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[20]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[20]- (GPIO bank 1) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi1, Output, spi1_n_ss_out[1]- (SPI Master Selects) 5= tt
		0, Input, ttc0_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_46_L3_SEL                                                  0

		Configures MIO Pin 46 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800B8, 0x000000FEU ,0x00000010U)
		RegMask = (IOU_SLCR_MIO_PIN_46_L0_SEL_MASK | IOU_SLCR_MIO_PIN_46_L1_SEL_MASK | IOU_SLCR_MIO_PIN_46_L2_SEL_MASK | IOU_SLCR_MIO_PIN_46_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_46_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_46_L1_SEL_SHIFT
			| 0x00000002U << IOU_SLCR_MIO_PIN_46_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_46_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_46_OFFSET ,0x000000FEU ,0x00000010U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_47 @ 0XFF1800BC</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rxd[2]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_47_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_47_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[6]- (8-bit Data bus) = sd0, Output, sdio0_data_out[6]- (8
		bit Data bus) 2= sd1, Input, sd1_data_in[1]- (8-bit Data bus) = sd1, Output, sdio1_data_out[1]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_47_L2_SEL                                                  2

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[21]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[21]- (GPIO bank 1) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi1, Input, spi1_n_ss_in- (SPI Master Selects) 4= spi
		, Output, spi1_n_ss_out[0]- (SPI Master Selects) 5= ttc0, Output, ttc0_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd
		 (UART transmitter serial output) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_47_L3_SEL                                                  0

		Configures MIO Pin 47 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800BC, 0x000000FEU ,0x00000010U)
		RegMask = (IOU_SLCR_MIO_PIN_47_L0_SEL_MASK | IOU_SLCR_MIO_PIN_47_L1_SEL_MASK | IOU_SLCR_MIO_PIN_47_L2_SEL_MASK | IOU_SLCR_MIO_PIN_47_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_47_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_47_L1_SEL_SHIFT
			| 0x00000002U << IOU_SLCR_MIO_PIN_47_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_47_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_47_OFFSET ,0x000000FEU ,0x00000010U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_48 @ 0XFF1800C0</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rxd[3]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_48_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_48_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[7]- (8-bit Data bus) = sd0, Output, sdio0_data_out[7]- (8
		bit Data bus) 2= sd1, Input, sd1_data_in[2]- (8-bit Data bus) = sd1, Output, sdio1_data_out[2]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_48_L2_SEL                                                  2

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[22]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[22]- (GPIO bank 1) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= spi1, Output, spi1
		so- (MISO signal) 5= ttc3, Input, ttc3_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= Not U
		ed
		PSU_IOU_SLCR_MIO_PIN_48_L3_SEL                                                  0

		Configures MIO Pin 48 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800C0, 0x000000FEU ,0x00000010U)
		RegMask = (IOU_SLCR_MIO_PIN_48_L0_SEL_MASK | IOU_SLCR_MIO_PIN_48_L1_SEL_MASK | IOU_SLCR_MIO_PIN_48_L2_SEL_MASK | IOU_SLCR_MIO_PIN_48_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_48_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_48_L1_SEL_SHIFT
			| 0x00000002U << IOU_SLCR_MIO_PIN_48_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_48_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_48_OFFSET ,0x000000FEU ,0x00000010U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_49 @ 0XFF1800C4</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rx_ctl- (RX RGMII control )
		PSU_IOU_SLCR_MIO_PIN_49_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_49_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_bus_pow- (SD card bus power) 2= sd1, Input, sd1_data_in[3]- (8
		bit Data bus) = sd1, Output, sdio1_data_out[3]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_49_L2_SEL                                                  2

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[23]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[23]- (GPIO bank 1) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal) 4= spi1, Input, sp
		1_si- (MOSI signal) 5= ttc3, Output, ttc3_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input)
		7= Not Used
		PSU_IOU_SLCR_MIO_PIN_49_L3_SEL                                                  0

		Configures MIO Pin 49 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800C4, 0x000000FEU ,0x00000010U)
		RegMask = (IOU_SLCR_MIO_PIN_49_L0_SEL_MASK | IOU_SLCR_MIO_PIN_49_L1_SEL_MASK | IOU_SLCR_MIO_PIN_49_L2_SEL_MASK | IOU_SLCR_MIO_PIN_49_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_49_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_49_L1_SEL_SHIFT
			| 0x00000002U << IOU_SLCR_MIO_PIN_49_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_49_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_49_OFFSET ,0x000000FEU ,0x00000010U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_50 @ 0XFF1800C8</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem_tsu, Input, gem_tsu_clk- (TSU clock)
		PSU_IOU_SLCR_MIO_PIN_50_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_50_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_wp- (SD card write protect from connector) 2= sd1, Input, sd1_c
		d_in- (Command Indicator) = sd1, Output, sdio1_cmd_out- (Command Indicator) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_50_L2_SEL                                                  2

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[24]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[24]- (GPIO bank 1) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= mdio1, Output, gem1_mdc- (MDIO Clock) 5= ttc2, Input, ttc2
		clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_50_L3_SEL                                                  0

		Configures MIO Pin 50 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800C8, 0x000000FEU ,0x00000010U)
		RegMask = (IOU_SLCR_MIO_PIN_50_L0_SEL_MASK | IOU_SLCR_MIO_PIN_50_L1_SEL_MASK | IOU_SLCR_MIO_PIN_50_L2_SEL_MASK | IOU_SLCR_MIO_PIN_50_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_50_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_50_L1_SEL_SHIFT
			| 0x00000002U << IOU_SLCR_MIO_PIN_50_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_50_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_50_OFFSET ,0x000000FEU ,0x00000010U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_51 @ 0XFF1800CC</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem_tsu, Input, gem_tsu_clk- (TSU clock)
		PSU_IOU_SLCR_MIO_PIN_51_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_51_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= sd1, Output, sdio1_clk_out- (SDSDIO clock) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_51_L2_SEL                                                  2

		Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[25]- (GPIO bank 1) 0= gpio1, Output, gpio_1_pin_out[25]- (GPIO bank 1) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= mdio1, Input, gem1_mdio_in- (MDIO Data) 4= mdio1, Outp
		t, gem1_mdio_out- (MDIO Data) 5= ttc2, Output, ttc2_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter
		serial output) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_51_L3_SEL                                                  0

		Configures MIO Pin 51 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800CC, 0x000000FEU ,0x00000010U)
		RegMask = (IOU_SLCR_MIO_PIN_51_L0_SEL_MASK | IOU_SLCR_MIO_PIN_51_L1_SEL_MASK | IOU_SLCR_MIO_PIN_51_L2_SEL_MASK | IOU_SLCR_MIO_PIN_51_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_51_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_51_L1_SEL_SHIFT
			| 0x00000002U << IOU_SLCR_MIO_PIN_51_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_51_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_51_OFFSET ,0x000000FEU ,0x00000010U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_52 @ 0XFF1800D0</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_tx_clk- (TX RGMII clock)
		PSU_IOU_SLCR_MIO_PIN_52_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_clk_in- (ULPI Clock)
		PSU_IOU_SLCR_MIO_PIN_52_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_52_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[0]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[0]- (GPIO bank 2) 1= can
		, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL signa
		) 3= pjtag, Input, pjtag_tck- (PJTAG TCK) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4= spi0, Output, spi0_sclk_out- (SPI Cloc
		) 5= ttc1, Input, ttc1_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace, Output, trace_
		lk- (Trace Port Clock)
		PSU_IOU_SLCR_MIO_PIN_52_L3_SEL                                                  0

		Configures MIO Pin 52 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800D0, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_52_L0_SEL_MASK | IOU_SLCR_MIO_PIN_52_L1_SEL_MASK | IOU_SLCR_MIO_PIN_52_L2_SEL_MASK | IOU_SLCR_MIO_PIN_52_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_52_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_52_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_52_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_52_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_52_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_53 @ 0XFF1800D4</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_txd[0]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_53_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_dir- (Data bus direction control)
		PSU_IOU_SLCR_MIO_PIN_53_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_53_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[1]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[1]- (GPIO bank 2) 1= can
		, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA signal
		 3= pjtag, Input, pjtag_tdi- (PJTAG TDI) 4= spi0, Output, spi0_n_ss_out[2]- (SPI Master Selects) 5= ttc1, Output, ttc1_wave_o
		t- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7= trace, Output, trace_ctl- (Trace Port Control
		Signal)
		PSU_IOU_SLCR_MIO_PIN_53_L3_SEL                                                  0

		Configures MIO Pin 53 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800D4, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_53_L0_SEL_MASK | IOU_SLCR_MIO_PIN_53_L1_SEL_MASK | IOU_SLCR_MIO_PIN_53_L2_SEL_MASK | IOU_SLCR_MIO_PIN_53_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_53_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_53_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_53_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_53_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_53_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_54 @ 0XFF1800D8</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_txd[1]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_54_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_data[2]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_
		ata[2]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_54_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_54_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[2]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[2]- (GPIO bank 2) 1= can
		, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL signal
		 3= pjtag, Output, pjtag_tdo- (PJTAG TDO) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Master Selects) 5= ttc0, Input, ttc0_clk_in
		 (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Output, tracedq[0]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_54_L3_SEL                                                  0

		Configures MIO Pin 54 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800D8, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_54_L0_SEL_MASK | IOU_SLCR_MIO_PIN_54_L1_SEL_MASK | IOU_SLCR_MIO_PIN_54_L2_SEL_MASK | IOU_SLCR_MIO_PIN_54_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_54_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_54_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_54_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_54_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_54_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_55 @ 0XFF1800DC</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_txd[2]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_55_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_nxt- (Data flow control signal from the PHY)
		PSU_IOU_SLCR_MIO_PIN_55_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_55_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[3]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[3]- (GPIO bank 2) 1= can
		, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA signa
		) 3= pjtag, Input, pjtag_tms- (PJTAG TMS) 4= spi0, Input, spi0_n_ss_in- (SPI Master Selects) 4= spi0, Output, spi0_n_ss_out[0
		- (SPI Master Selects) 5= ttc0, Output, ttc0_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial
		output) 7= trace, Output, tracedq[1]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_55_L3_SEL                                                  0

		Configures MIO Pin 55 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800DC, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_55_L0_SEL_MASK | IOU_SLCR_MIO_PIN_55_L1_SEL_MASK | IOU_SLCR_MIO_PIN_55_L2_SEL_MASK | IOU_SLCR_MIO_PIN_55_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_55_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_55_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_55_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_55_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_55_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_56 @ 0XFF1800E0</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_txd[3]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_56_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_data[0]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_
		ata[0]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_56_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_56_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[4]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[4]- (GPIO bank 2) 1= can
		, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL signa
		) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= spi0, Output, spi0_s
		- (MISO signal) 5= ttc3, Input, ttc3_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace,
		utput, tracedq[2]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_56_L3_SEL                                                  0

		Configures MIO Pin 56 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800E0, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_56_L0_SEL_MASK | IOU_SLCR_MIO_PIN_56_L1_SEL_MASK | IOU_SLCR_MIO_PIN_56_L2_SEL_MASK | IOU_SLCR_MIO_PIN_56_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_56_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_56_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_56_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_56_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_56_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_57 @ 0XFF1800E4</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_tx_ctl- (TX RGMII control)
		PSU_IOU_SLCR_MIO_PIN_57_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_data[1]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_
		ata[1]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_57_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_57_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[5]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[5]- (GPIO bank 2) 1= can
		, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA signal
		 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal) 4= spi0, Input, spi0
		si- (MOSI signal) 5= ttc3, Output, ttc3_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7
		 trace, Output, tracedq[3]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_57_L3_SEL                                                  0

		Configures MIO Pin 57 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800E4, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_57_L0_SEL_MASK | IOU_SLCR_MIO_PIN_57_L1_SEL_MASK | IOU_SLCR_MIO_PIN_57_L2_SEL_MASK | IOU_SLCR_MIO_PIN_57_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_57_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_57_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_57_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_57_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_57_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_58 @ 0XFF1800E8</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rx_clk- (RX RGMII clock)
		PSU_IOU_SLCR_MIO_PIN_58_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Output, usb0_ulpi_stp- (Asserted to end or interrupt transfers)
		PSU_IOU_SLCR_MIO_PIN_58_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_58_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[6]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[6]- (GPIO bank 2) 1= can
		, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL signal
		 3= pjtag, Input, pjtag_tck- (PJTAG TCK) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4= spi1, Output, spi1_sclk_out- (SPI Clock
		 5= ttc2, Input, ttc2_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Output, tracedq[4]-
		Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_58_L3_SEL                                                  0

		Configures MIO Pin 58 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800E8, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_58_L0_SEL_MASK | IOU_SLCR_MIO_PIN_58_L1_SEL_MASK | IOU_SLCR_MIO_PIN_58_L2_SEL_MASK | IOU_SLCR_MIO_PIN_58_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_58_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_58_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_58_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_58_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_58_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_59 @ 0XFF1800EC</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rxd[0]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_59_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_data[3]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_
		ata[3]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_59_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_59_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[7]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[7]- (GPIO bank 2) 1= can
		, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA signa
		) 3= pjtag, Input, pjtag_tdi- (PJTAG TDI) 4= spi1, Output, spi1_n_ss_out[2]- (SPI Master Selects) 5= ttc2, Output, ttc2_wave_
		ut- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial output) 7= trace, Output, tracedq[5]- (Trace Port
		atabus)
		PSU_IOU_SLCR_MIO_PIN_59_L3_SEL                                                  0

		Configures MIO Pin 59 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800EC, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_59_L0_SEL_MASK | IOU_SLCR_MIO_PIN_59_L1_SEL_MASK | IOU_SLCR_MIO_PIN_59_L2_SEL_MASK | IOU_SLCR_MIO_PIN_59_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_59_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_59_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_59_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_59_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_59_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_60 @ 0XFF1800F0</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rxd[1]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_60_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_data[4]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_
		ata[4]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_60_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_60_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[8]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[8]- (GPIO bank 2) 1= can
		, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL signa
		) 3= pjtag, Output, pjtag_tdo- (PJTAG TDO) 4= spi1, Output, spi1_n_ss_out[1]- (SPI Master Selects) 5= ttc1, Input, ttc1_clk_i
		- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace, Output, tracedq[6]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_60_L3_SEL                                                  0

		Configures MIO Pin 60 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800F0, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_60_L0_SEL_MASK | IOU_SLCR_MIO_PIN_60_L1_SEL_MASK | IOU_SLCR_MIO_PIN_60_L2_SEL_MASK | IOU_SLCR_MIO_PIN_60_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_60_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_60_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_60_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_60_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_60_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_61 @ 0XFF1800F4</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rxd[2]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_61_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_data[5]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_
		ata[5]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_61_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_61_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[9]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[9]- (GPIO bank 2) 1= can
		, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA signal
		 3= pjtag, Input, pjtag_tms- (PJTAG TMS) 4= spi1, Input, spi1_n_ss_in- (SPI Master Selects) 4= spi1, Output, spi1_n_ss_out[0]
		 (SPI Master Selects) 5= ttc1, Output, ttc1_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial inpu
		) 7= trace, Output, tracedq[7]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_61_L3_SEL                                                  0

		Configures MIO Pin 61 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800F4, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_61_L0_SEL_MASK | IOU_SLCR_MIO_PIN_61_L1_SEL_MASK | IOU_SLCR_MIO_PIN_61_L2_SEL_MASK | IOU_SLCR_MIO_PIN_61_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_61_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_61_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_61_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_61_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_61_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_62 @ 0XFF1800F8</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rxd[3]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_62_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_data[6]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_
		ata[6]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_62_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_62_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[10]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[10]- (GPIO bank 2) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= spi1, Output, spi1_
		o- (MISO signal) 5= ttc0, Input, ttc0_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Outp
		t, tracedq[8]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_62_L3_SEL                                                  0

		Configures MIO Pin 62 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800F8, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_62_L0_SEL_MASK | IOU_SLCR_MIO_PIN_62_L1_SEL_MASK | IOU_SLCR_MIO_PIN_62_L2_SEL_MASK | IOU_SLCR_MIO_PIN_62_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_62_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_62_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_62_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_62_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_62_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_63 @ 0XFF1800FC</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rx_ctl- (RX RGMII control )
		PSU_IOU_SLCR_MIO_PIN_63_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_data[7]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_
		ata[7]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_63_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_63_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[11]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[11]- (GPIO bank 2) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal) 4= spi1, Input, s
		i1_si- (MOSI signal) 5= ttc0, Output, ttc0_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial o
		tput) 7= trace, Output, tracedq[9]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_63_L3_SEL                                                  0

		Configures MIO Pin 63 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF1800FC, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_63_L0_SEL_MASK | IOU_SLCR_MIO_PIN_63_L1_SEL_MASK | IOU_SLCR_MIO_PIN_63_L2_SEL_MASK | IOU_SLCR_MIO_PIN_63_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_63_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_63_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_63_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_63_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_63_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_64 @ 0XFF180100</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_tx_clk- (TX RGMII clock)
		PSU_IOU_SLCR_MIO_PIN_64_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_clk_in- (ULPI Clock)
		PSU_IOU_SLCR_MIO_PIN_64_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_clk_out- (SDSDIO clock) 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_64_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[12]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[12]- (GPIO bank 2) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4= spi0, Output, s
		i0_sclk_out- (SPI Clock) 5= ttc3, Input, ttc3_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7
		 trace, Output, tracedq[10]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_64_L3_SEL                                                  0

		Configures MIO Pin 64 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180100, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_64_L0_SEL_MASK | IOU_SLCR_MIO_PIN_64_L1_SEL_MASK | IOU_SLCR_MIO_PIN_64_L2_SEL_MASK | IOU_SLCR_MIO_PIN_64_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_64_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_64_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_64_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_64_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_64_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_65 @ 0XFF180104</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_txd[0]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_65_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_dir- (Data bus direction control)
		PSU_IOU_SLCR_MIO_PIN_65_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_cd_n- (SD card detect from connector) 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_65_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[13]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[13]- (GPIO bank 2) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi0, Output, spi0_n_ss_out[2]- (SPI Master Selects) 5=
		ttc3, Output, ttc3_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7= trace, Output, trac
		dq[11]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_65_L3_SEL                                                  0

		Configures MIO Pin 65 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180104, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_65_L0_SEL_MASK | IOU_SLCR_MIO_PIN_65_L1_SEL_MASK | IOU_SLCR_MIO_PIN_65_L2_SEL_MASK | IOU_SLCR_MIO_PIN_65_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_65_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_65_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_65_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_65_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_65_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_66 @ 0XFF180108</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_txd[1]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_66_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_data[2]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_
		ata[2]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_66_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_cmd_in- (Command Indicator) = sd0, Output, sdio0_cmd_out- (Comman
		 Indicator) 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_66_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[14]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[14]- (GPIO bank 2) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Master Selects) 5= tt
		2, Input, ttc2_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Output, tracedq[12]- (Trace
		Port Databus)
		PSU_IOU_SLCR_MIO_PIN_66_L3_SEL                                                  0

		Configures MIO Pin 66 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180108, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_66_L0_SEL_MASK | IOU_SLCR_MIO_PIN_66_L1_SEL_MASK | IOU_SLCR_MIO_PIN_66_L2_SEL_MASK | IOU_SLCR_MIO_PIN_66_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_66_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_66_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_66_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_66_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_66_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_67 @ 0XFF18010C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_txd[2]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_67_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_nxt- (Data flow control signal from the PHY)
		PSU_IOU_SLCR_MIO_PIN_67_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[0]- (8-bit Data bus) = sd0, Output, sdio0_data_out[0]- (8
		bit Data bus) 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_67_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[15]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[15]- (GPIO bank 2) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi0, Input, spi0_n_ss_in- (SPI Master Selects) 4= spi
		, Output, spi0_n_ss_out[0]- (SPI Master Selects) 5= ttc2, Output, ttc2_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd
		 (UART transmitter serial output) 7= trace, Output, tracedq[13]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_67_L3_SEL                                                  0

		Configures MIO Pin 67 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18010C, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_67_L0_SEL_MASK | IOU_SLCR_MIO_PIN_67_L1_SEL_MASK | IOU_SLCR_MIO_PIN_67_L2_SEL_MASK | IOU_SLCR_MIO_PIN_67_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_67_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_67_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_67_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_67_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_67_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_68 @ 0XFF180110</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_txd[3]- (TX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_68_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_data[0]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_
		ata[0]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_68_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[1]- (8-bit Data bus) = sd0, Output, sdio0_data_out[1]- (8
		bit Data bus) 2= Not Used 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_68_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[16]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[16]- (GPIO bank 2) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= spi0, Output, spi0
		so- (MISO signal) 5= ttc1, Input, ttc1_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace
		 Output, tracedq[14]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_68_L3_SEL                                                  0

		Configures MIO Pin 68 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180110, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_68_L0_SEL_MASK | IOU_SLCR_MIO_PIN_68_L1_SEL_MASK | IOU_SLCR_MIO_PIN_68_L2_SEL_MASK | IOU_SLCR_MIO_PIN_68_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_68_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_68_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_68_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_68_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_68_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_69 @ 0XFF180114</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_tx_ctl- (TX RGMII control)
		PSU_IOU_SLCR_MIO_PIN_69_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_data[1]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_
		ata[1]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_69_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[2]- (8-bit Data bus) = sd0, Output, sdio0_data_out[2]- (8
		bit Data bus) 2= sd1, Input, sdio1_wp- (SD card write protect from connector) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_69_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[17]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[17]- (GPIO bank 2) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal) 4= spi0, Input, sp
		0_si- (MOSI signal) 5= ttc1, Output, ttc1_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input)
		7= trace, Output, tracedq[15]- (Trace Port Databus)
		PSU_IOU_SLCR_MIO_PIN_69_L3_SEL                                                  0

		Configures MIO Pin 69 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180114, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_69_L0_SEL_MASK | IOU_SLCR_MIO_PIN_69_L1_SEL_MASK | IOU_SLCR_MIO_PIN_69_L2_SEL_MASK | IOU_SLCR_MIO_PIN_69_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_69_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_69_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_69_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_69_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_69_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_70 @ 0XFF180118</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rx_clk- (RX RGMII clock)
		PSU_IOU_SLCR_MIO_PIN_70_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Output, usb1_ulpi_stp- (Asserted to end or interrupt transfers)
		PSU_IOU_SLCR_MIO_PIN_70_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[3]- (8-bit Data bus) = sd0, Output, sdio0_data_out[3]- (8
		bit Data bus) 2= sd1, Output, sdio1_bus_pow- (SD card bus power) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_70_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[18]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[18]- (GPIO bank 2) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4= spi1, Output, sp
		1_sclk_out- (SPI Clock) 5= ttc0, Input, ttc0_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= Not
		sed
		PSU_IOU_SLCR_MIO_PIN_70_L3_SEL                                                  0

		Configures MIO Pin 70 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180118, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_70_L0_SEL_MASK | IOU_SLCR_MIO_PIN_70_L1_SEL_MASK | IOU_SLCR_MIO_PIN_70_L2_SEL_MASK | IOU_SLCR_MIO_PIN_70_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_70_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_70_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_70_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_70_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_70_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_71 @ 0XFF18011C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rxd[0]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_71_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_data[3]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_
		ata[3]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_71_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[4]- (8-bit Data bus) = sd0, Output, sdio0_data_out[4]- (8
		bit Data bus) 2= sd1, Input, sd1_data_in[0]- (8-bit Data bus) = sd1, Output, sdio1_data_out[0]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_71_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[19]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[19]- (GPIO bank 2) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi1, Output, spi1_n_ss_out[2]- (SPI Master Selects) 5
		 ttc0, Output, ttc0_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial output) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_71_L3_SEL                                                  0

		Configures MIO Pin 71 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18011C, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_71_L0_SEL_MASK | IOU_SLCR_MIO_PIN_71_L1_SEL_MASK | IOU_SLCR_MIO_PIN_71_L2_SEL_MASK | IOU_SLCR_MIO_PIN_71_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_71_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_71_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_71_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_71_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_71_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_72 @ 0XFF180120</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rxd[1]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_72_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_data[4]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_
		ata[4]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_72_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[5]- (8-bit Data bus) = sd0, Output, sdio0_data_out[5]- (8
		bit Data bus) 2= sd1, Input, sd1_data_in[1]- (8-bit Data bus) = sd1, Output, sdio1_data_out[1]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_72_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[20]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[20]- (GPIO bank 2) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= swdt1, Input, swdt1_clk_in- (Watch Dog Timer Input clock) 4= spi1, Output, spi1_n_ss_out[1]- (SPI Master Selects) 5= N
		t Used 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_72_L3_SEL                                                  0

		Configures MIO Pin 72 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180120, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_72_L0_SEL_MASK | IOU_SLCR_MIO_PIN_72_L1_SEL_MASK | IOU_SLCR_MIO_PIN_72_L2_SEL_MASK | IOU_SLCR_MIO_PIN_72_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_72_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_72_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_72_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_72_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_72_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_73 @ 0XFF180124</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rxd[2]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_73_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_data[5]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_
		ata[5]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_73_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[6]- (8-bit Data bus) = sd0, Output, sdio0_data_out[6]- (8
		bit Data bus) 2= sd1, Input, sd1_data_in[2]- (8-bit Data bus) = sd1, Output, sdio1_data_out[2]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_73_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[21]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[21]- (GPIO bank 2) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= swdt1, Output, swdt1_rst_out- (Watch Dog Timer Output clock) 4= spi1, Input, spi1_n_ss_in- (SPI Master Selects) 4= spi1
		 Output, spi1_n_ss_out[0]- (SPI Master Selects) 5= Not Used 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_73_L3_SEL                                                  0

		Configures MIO Pin 73 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180124, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_73_L0_SEL_MASK | IOU_SLCR_MIO_PIN_73_L1_SEL_MASK | IOU_SLCR_MIO_PIN_73_L2_SEL_MASK | IOU_SLCR_MIO_PIN_73_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_73_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_73_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_73_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_73_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_73_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_74 @ 0XFF180128</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rxd[3]- (RX RGMII data)
		PSU_IOU_SLCR_MIO_PIN_74_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_data[6]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_
		ata[6]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_74_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[7]- (8-bit Data bus) = sd0, Output, sdio0_data_out[7]- (8
		bit Data bus) 2= sd1, Input, sd1_data_in[3]- (8-bit Data bus) = sd1, Output, sdio1_data_out[3]- (8-bit Data bus) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_74_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[22]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[22]- (GPIO bank 2) 1= c
		n0, Input, can0_phy_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0, Output, i2c0_scl_out- (SCL sign
		l) 3= swdt0, Input, swdt0_clk_in- (Watch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= spi1, Output, spi1_
		o- (MISO signal) 5= Not Used 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_74_L3_SEL                                                  0

		Configures MIO Pin 74 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180128, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_74_L0_SEL_MASK | IOU_SLCR_MIO_PIN_74_L1_SEL_MASK | IOU_SLCR_MIO_PIN_74_L2_SEL_MASK | IOU_SLCR_MIO_PIN_74_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_74_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_74_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_74_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_74_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_74_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_75 @ 0XFF18012C</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rx_ctl- (RX RGMII control )
		PSU_IOU_SLCR_MIO_PIN_75_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_data[7]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_
		ata[7]- (ULPI data bus)
		PSU_IOU_SLCR_MIO_PIN_75_L1_SEL                                                  1

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_bus_pow- (SD card bus power) 2= sd1, Input, sd1_cmd_in- (Comma
		d Indicator) = sd1, Output, sdio1_cmd_out- (Command Indicator) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_75_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[23]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[23]- (GPIO bank 2) 1= c
		n0, Output, can0_phy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c0, Output, i2c0_sda_out- (SDA sig
		al) 3= swdt0, Output, swdt0_rst_out- (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal) 4= spi1, Input, s
		i1_si- (MOSI signal) 5= Not Used 6= ua0, Output, ua0_txd- (UART transmitter serial output) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_75_L3_SEL                                                  0

		Configures MIO Pin 75 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF18012C, 0x000000FEU ,0x00000004U)
		RegMask = (IOU_SLCR_MIO_PIN_75_L0_SEL_MASK | IOU_SLCR_MIO_PIN_75_L1_SEL_MASK | IOU_SLCR_MIO_PIN_75_L2_SEL_MASK | IOU_SLCR_MIO_PIN_75_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_75_L0_SEL_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_PIN_75_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_75_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_75_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_75_OFFSET ,0x000000FEU ,0x00000004U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_76 @ 0XFF180130</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_76_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_76_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_wp- (SD card write protect from connector) 2= sd1, Output, sdio
		_clk_out- (SDSDIO clock) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_76_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[24]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[24]- (GPIO bank 2) 1= c
		n1, Output, can1_phy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c1, Output, i2c1_scl_out- (SCL sig
		al) 3= mdio0, Output, gem0_mdc- (MDIO Clock) 4= mdio1, Output, gem1_mdc- (MDIO Clock) 5= mdio2, Output, gem2_mdc- (MDIO Clock
		 6= mdio3, Output, gem3_mdc- (MDIO Clock) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_76_L3_SEL                                                  0

		Configures MIO Pin 76 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180130, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_76_L0_SEL_MASK | IOU_SLCR_MIO_PIN_76_L1_SEL_MASK | IOU_SLCR_MIO_PIN_76_L2_SEL_MASK | IOU_SLCR_MIO_PIN_76_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_76_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_76_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_76_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_76_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_76_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_PIN_77 @ 0XFF180134</p>

		Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_77_L0_SEL                                                  0

		Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
		PSU_IOU_SLCR_MIO_PIN_77_L1_SEL                                                  0

		Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= sd1, Input, sdio1_cd_n- (SD card detect from connector) 3= Not Used
		PSU_IOU_SLCR_MIO_PIN_77_L2_SEL                                                  0

		Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[25]- (GPIO bank 2) 0= gpio2, Output, gpio_2_pin_out[25]- (GPIO bank 2) 1= c
		n1, Input, can1_phy_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1, Output, i2c1_sda_out- (SDA sign
		l) 3= mdio0, Input, gem0_mdio_in- (MDIO Data) 3= mdio0, Output, gem0_mdio_out- (MDIO Data) 4= mdio1, Input, gem1_mdio_in- (MD
		O Data) 4= mdio1, Output, gem1_mdio_out- (MDIO Data) 5= mdio2, Input, gem2_mdio_in- (MDIO Data) 5= mdio2, Output, gem2_mdio_o
		t- (MDIO Data) 6= mdio3, Input, gem3_mdio_in- (MDIO Data) 6= mdio3, Output, gem3_mdio_out- (MDIO Data) 7= Not Used
		PSU_IOU_SLCR_MIO_PIN_77_L3_SEL                                                  0

		Configures MIO Pin 77 peripheral interface mapping
		(OFFSET, MASK, VALUE)      (0XFF180134, 0x000000FEU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_PIN_77_L0_SEL_MASK | IOU_SLCR_MIO_PIN_77_L1_SEL_MASK | IOU_SLCR_MIO_PIN_77_L2_SEL_MASK | IOU_SLCR_MIO_PIN_77_L3_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_PIN_77_L0_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_77_L1_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_77_L2_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_PIN_77_L3_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_PIN_77_OFFSET ,0x000000FEU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : MIO_MST_TRI0 @ 0XFF180204</p>

		Master Tri-state Enable for pin 0, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_00_TRI                                            0

		Master Tri-state Enable for pin 1, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_01_TRI                                            0

		Master Tri-state Enable for pin 2, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_02_TRI                                            0

		Master Tri-state Enable for pin 3, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_03_TRI                                            0

		Master Tri-state Enable for pin 4, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_04_TRI                                            0

		Master Tri-state Enable for pin 5, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_05_TRI                                            0

		Master Tri-state Enable for pin 6, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_06_TRI                                            0

		Master Tri-state Enable for pin 7, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_07_TRI                                            0

		Master Tri-state Enable for pin 8, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_08_TRI                                            0

		Master Tri-state Enable for pin 9, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_09_TRI                                            1

		Master Tri-state Enable for pin 10, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_10_TRI                                            0

		Master Tri-state Enable for pin 11, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_11_TRI                                            0

		Master Tri-state Enable for pin 12, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_12_TRI                                            0

		Master Tri-state Enable for pin 13, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_13_TRI                                            0

		Master Tri-state Enable for pin 14, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_14_TRI                                            0

		Master Tri-state Enable for pin 15, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_15_TRI                                            0

		Master Tri-state Enable for pin 16, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_16_TRI                                            0

		Master Tri-state Enable for pin 17, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_17_TRI                                            0

		Master Tri-state Enable for pin 18, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_18_TRI                                            0

		Master Tri-state Enable for pin 19, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_19_TRI                                            0

		Master Tri-state Enable for pin 20, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_20_TRI                                            0

		Master Tri-state Enable for pin 21, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_21_TRI                                            0

		Master Tri-state Enable for pin 22, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_22_TRI                                            0

		Master Tri-state Enable for pin 23, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_23_TRI                                            0

		Master Tri-state Enable for pin 24, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_24_TRI                                            1

		Master Tri-state Enable for pin 25, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_25_TRI                                            0

		Master Tri-state Enable for pin 26, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_26_TRI                                            0

		Master Tri-state Enable for pin 27, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_27_TRI                                            0

		Master Tri-state Enable for pin 28, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_28_TRI                                            0

		Master Tri-state Enable for pin 29, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_29_TRI                                            0

		Master Tri-state Enable for pin 30, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_30_TRI                                            0

		Master Tri-state Enable for pin 31, active high
		PSU_IOU_SLCR_MIO_MST_TRI0_PIN_31_TRI                                            0

		MIO pin Tri-state Enables, 31:0
		(OFFSET, MASK, VALUE)      (0XFF180204, 0xFFFFFFFFU ,0x01000200U)
		RegMask = (IOU_SLCR_MIO_MST_TRI0_PIN_00_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_01_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_02_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_03_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_04_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_05_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_06_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_07_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_08_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_09_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_10_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_11_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_12_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_13_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_14_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_15_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_16_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_17_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_18_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_19_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_20_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_21_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_22_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_23_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_24_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_25_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_26_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_27_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_28_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_29_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_30_TRI_MASK | IOU_SLCR_MIO_MST_TRI0_PIN_31_TRI_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_00_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_01_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_02_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_03_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_04_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_05_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_06_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_07_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_08_TRI_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_MST_TRI0_PIN_09_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_10_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_11_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_12_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_13_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_14_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_15_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_16_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_17_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_18_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_19_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_20_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_21_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_22_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_23_TRI_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_MST_TRI0_PIN_24_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_25_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_26_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_27_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_28_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_29_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_30_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI0_PIN_31_TRI_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_MST_TRI0_OFFSET ,0xFFFFFFFFU ,0x01000200U);
	/*############################################################################################################################ */

		/*Register : MIO_MST_TRI1 @ 0XFF180208</p>

		Master Tri-state Enable for pin 32, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_32_TRI                                            0

		Master Tri-state Enable for pin 33, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_33_TRI                                            0

		Master Tri-state Enable for pin 34, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_34_TRI                                            0

		Master Tri-state Enable for pin 35, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_35_TRI                                            1

		Master Tri-state Enable for pin 36, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_36_TRI                                            0

		Master Tri-state Enable for pin 37, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_37_TRI                                            1

		Master Tri-state Enable for pin 38, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_38_TRI                                            0

		Master Tri-state Enable for pin 39, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_39_TRI                                            0

		Master Tri-state Enable for pin 40, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_40_TRI                                            0

		Master Tri-state Enable for pin 41, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_41_TRI                                            0

		Master Tri-state Enable for pin 42, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_42_TRI                                            0

		Master Tri-state Enable for pin 43, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_43_TRI                                            0

		Master Tri-state Enable for pin 44, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_44_TRI                                            0

		Master Tri-state Enable for pin 45, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_45_TRI                                            0

		Master Tri-state Enable for pin 46, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_46_TRI                                            0

		Master Tri-state Enable for pin 47, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_47_TRI                                            0

		Master Tri-state Enable for pin 48, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_48_TRI                                            0

		Master Tri-state Enable for pin 49, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_49_TRI                                            0

		Master Tri-state Enable for pin 50, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_50_TRI                                            0

		Master Tri-state Enable for pin 51, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_51_TRI                                            0

		Master Tri-state Enable for pin 52, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_52_TRI                                            1

		Master Tri-state Enable for pin 53, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_53_TRI                                            1

		Master Tri-state Enable for pin 54, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_54_TRI                                            0

		Master Tri-state Enable for pin 55, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_55_TRI                                            1

		Master Tri-state Enable for pin 56, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_56_TRI                                            0

		Master Tri-state Enable for pin 57, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_57_TRI                                            0

		Master Tri-state Enable for pin 58, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_58_TRI                                            0

		Master Tri-state Enable for pin 59, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_59_TRI                                            0

		Master Tri-state Enable for pin 60, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_60_TRI                                            0

		Master Tri-state Enable for pin 61, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_61_TRI                                            0

		Master Tri-state Enable for pin 62, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_62_TRI                                            0

		Master Tri-state Enable for pin 63, active high
		PSU_IOU_SLCR_MIO_MST_TRI1_PIN_63_TRI                                            0

		MIO pin Tri-state Enables, 63:32
		(OFFSET, MASK, VALUE)      (0XFF180208, 0xFFFFFFFFU ,0x00B00028U)
		RegMask = (IOU_SLCR_MIO_MST_TRI1_PIN_32_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_33_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_34_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_35_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_36_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_37_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_38_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_39_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_40_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_41_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_42_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_43_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_44_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_45_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_46_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_47_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_48_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_49_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_50_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_51_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_52_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_53_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_54_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_55_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_56_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_57_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_58_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_59_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_60_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_61_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_62_TRI_MASK | IOU_SLCR_MIO_MST_TRI1_PIN_63_TRI_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_32_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_33_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_34_TRI_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_MST_TRI1_PIN_35_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_36_TRI_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_MST_TRI1_PIN_37_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_38_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_39_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_40_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_41_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_42_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_43_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_44_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_45_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_46_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_47_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_48_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_49_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_50_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_51_TRI_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_MST_TRI1_PIN_52_TRI_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_MST_TRI1_PIN_53_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_54_TRI_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_MST_TRI1_PIN_55_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_56_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_57_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_58_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_59_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_60_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_61_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_62_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI1_PIN_63_TRI_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_MST_TRI1_OFFSET ,0xFFFFFFFFU ,0x00B00028U);
	/*############################################################################################################################ */

		/*Register : MIO_MST_TRI2 @ 0XFF18020C</p>

		Master Tri-state Enable for pin 64, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_64_TRI                                            1

		Master Tri-state Enable for pin 65, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_65_TRI                                            1

		Master Tri-state Enable for pin 66, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_66_TRI                                            0

		Master Tri-state Enable for pin 67, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_67_TRI                                            1

		Master Tri-state Enable for pin 68, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_68_TRI                                            0

		Master Tri-state Enable for pin 69, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_69_TRI                                            0

		Master Tri-state Enable for pin 70, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_70_TRI                                            0

		Master Tri-state Enable for pin 71, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_71_TRI                                            0

		Master Tri-state Enable for pin 72, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_72_TRI                                            0

		Master Tri-state Enable for pin 73, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_73_TRI                                            0

		Master Tri-state Enable for pin 74, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_74_TRI                                            0

		Master Tri-state Enable for pin 75, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_75_TRI                                            0

		Master Tri-state Enable for pin 76, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_76_TRI                                            0

		Master Tri-state Enable for pin 77, active high
		PSU_IOU_SLCR_MIO_MST_TRI2_PIN_77_TRI                                            0

		MIO pin Tri-state Enables, 77:64
		(OFFSET, MASK, VALUE)      (0XFF18020C, 0x00003FFFU ,0x0000000BU)
		RegMask = (IOU_SLCR_MIO_MST_TRI2_PIN_64_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_65_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_66_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_67_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_68_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_69_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_70_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_71_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_72_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_73_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_74_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_75_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_76_TRI_MASK | IOU_SLCR_MIO_MST_TRI2_PIN_77_TRI_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_MIO_MST_TRI2_PIN_64_TRI_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_MST_TRI2_PIN_65_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI2_PIN_66_TRI_SHIFT
			| 0x00000001U << IOU_SLCR_MIO_MST_TRI2_PIN_67_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI2_PIN_68_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI2_PIN_69_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI2_PIN_70_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI2_PIN_71_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI2_PIN_72_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI2_PIN_73_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI2_PIN_74_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI2_PIN_75_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI2_PIN_76_TRI_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_MST_TRI2_PIN_77_TRI_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_MST_TRI2_OFFSET ,0x00003FFFU ,0x0000000BU);
	/*############################################################################################################################ */

		/*Register : bank0_ctrl0 @ 0XFF180138</p>

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_0                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_1                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_2                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_3                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_4                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_5                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_6                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_7                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_8                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_9                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_10                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_11                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_12                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_13                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_14                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_15                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_16                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_17                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_18                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_19                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_20                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_21                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_22                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_23                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_24                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_25                                          1

		Drive0 control to MIO Bank 0 - control MIO[25:0]
		(OFFSET, MASK, VALUE)      (0XFF180138, 0x03FFFFFFU ,0x03FFFFFFU)
		RegMask = (IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_0_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_1_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_2_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_3_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_4_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_5_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_6_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_7_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_8_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_9_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_10_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_11_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_12_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_13_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_14_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_15_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_16_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_17_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_18_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_19_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_20_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_21_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_22_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_23_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_24_MASK | IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_25_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_23_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_24_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK0_CTRL0_OFFSET ,0x03FFFFFFU ,0x03FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank0_ctrl1 @ 0XFF18013C</p>

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_0                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_1                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_2                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_3                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_4                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_5                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_6                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_7                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_8                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_9                                           1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_10                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_11                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_12                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_13                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_14                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_15                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_16                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_17                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_18                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_19                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_20                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_21                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_22                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_23                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_24                                          1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_25                                          1

		Drive1 control to MIO Bank 0 - control MIO[25:0]
		(OFFSET, MASK, VALUE)      (0XFF18013C, 0x03FFFFFFU ,0x03FFFFFFU)
		RegMask = (IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_0_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_1_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_2_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_3_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_4_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_5_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_6_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_7_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_8_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_9_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_10_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_11_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_12_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_13_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_14_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_15_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_16_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_17_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_18_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_19_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_20_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_21_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_22_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_23_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_24_MASK | IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_25_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_23_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_24_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK0_CTRL1_OFFSET ,0x03FFFFFFU ,0x03FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank0_ctrl3 @ 0XFF180140</p>

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_0                                   0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_1                                   0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_2                                   0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_3                                   0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_4                                   0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_5                                   0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_6                                   0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_7                                   0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_8                                   0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_9                                   0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_10                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_11                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_12                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_13                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_14                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_15                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_16                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_17                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_18                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_19                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_20                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_21                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_22                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_23                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_24                                  0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_25                                  0

		Selects either Schmitt or CMOS input for MIO Bank 0 - control MIO[25:0]
		(OFFSET, MASK, VALUE)      (0XFF180140, 0x03FFFFFFU ,0x00000000U)
		RegMask = (IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_0_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_1_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_2_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_3_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_4_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_5_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_6_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_7_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_8_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_9_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_10_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_11_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_12_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_13_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_14_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_15_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_16_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_17_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_18_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_19_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_20_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_21_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_22_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_23_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_24_MASK | IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_25_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_0_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_1_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_2_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_3_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_4_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_5_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_6_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_7_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_8_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_9_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_10_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_11_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_12_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_13_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_14_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_15_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_16_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_17_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_18_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_19_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_20_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_21_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_22_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_23_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_24_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK0_CTRL3_OFFSET ,0x03FFFFFFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : bank0_ctrl4 @ 0XFF180144</p>

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_0                                  1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_1                                  1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_2                                  1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_3                                  1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_4                                  1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_5                                  1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_6                                  1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_7                                  1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_8                                  1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_9                                  1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_10                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_11                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_12                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_13                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_14                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_15                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_16                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_17                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_18                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_19                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_20                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_21                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_22                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_23                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_24                                 1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_25                                 1

		When mio_bank0_pull_enable is set, this selects pull up or pull down for MIO Bank 0 - control MIO[25:0]
		(OFFSET, MASK, VALUE)      (0XFF180144, 0x03FFFFFFU ,0x03FFFFFFU)
		RegMask = (IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_0_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_1_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_2_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_3_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_4_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_5_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_6_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_7_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_8_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_9_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_10_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_11_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_12_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_13_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_14_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_15_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_16_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_17_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_18_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_19_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_20_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_21_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_22_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_23_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_24_MASK | IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_25_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_23_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_24_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK0_CTRL4_OFFSET ,0x03FFFFFFU ,0x03FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank0_ctrl5 @ 0XFF180148</p>

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_0                                      1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_1                                      1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_2                                      1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_3                                      1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_4                                      1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_5                                      1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_6                                      1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_7                                      1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_8                                      1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_9                                      1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_10                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_11                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_12                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_13                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_14                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_15                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_16                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_17                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_18                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_19                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_20                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_21                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_22                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_23                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_24                                     1

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_25                                     1

		When set, this enables mio_bank0_pullupdown to selects pull up or pull down for MIO Bank 0 - control MIO[25:0]
		(OFFSET, MASK, VALUE)      (0XFF180148, 0x03FFFFFFU ,0x03FFFFFFU)
		RegMask = (IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_0_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_1_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_2_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_3_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_4_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_5_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_6_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_7_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_8_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_9_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_10_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_11_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_12_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_13_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_14_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_15_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_16_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_17_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_18_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_19_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_20_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_21_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_22_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_23_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_24_MASK | IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_25_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_23_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_24_SHIFT
			| 0x00000001U << IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK0_CTRL5_OFFSET ,0x03FFFFFFU ,0x03FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank0_ctrl6 @ 0XFF18014C</p>

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_0                                 0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_1                                 0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_2                                 0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_3                                 0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_4                                 0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_5                                 0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_6                                 0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_7                                 0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_8                                 0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_9                                 0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_10                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_11                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_12                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_13                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_14                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_15                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_16                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_17                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_18                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_19                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_20                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_21                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_22                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_23                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_24                                0

		Each bit applies to a single IO. Bit 0 for MIO[0].
		PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_25                                0

		Slew rate control to MIO Bank 0 - control MIO[25:0]
		(OFFSET, MASK, VALUE)      (0XFF18014C, 0x03FFFFFFU ,0x00000000U)
		RegMask = (IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_0_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_1_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_2_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_3_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_4_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_5_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_6_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_7_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_8_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_9_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_10_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_11_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_12_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_13_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_14_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_15_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_16_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_17_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_18_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_19_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_20_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_21_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_22_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_23_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_24_MASK | IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_25_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_0_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_1_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_2_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_3_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_4_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_5_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_6_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_7_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_8_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_9_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_10_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_11_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_12_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_13_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_14_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_15_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_16_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_17_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_18_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_19_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_20_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_21_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_22_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_23_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_24_SHIFT
			| 0x00000000U << IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK0_CTRL6_OFFSET ,0x03FFFFFFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : bank1_ctrl0 @ 0XFF180154</p>

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_0                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_1                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_2                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_3                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_4                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_5                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_6                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_7                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_8                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_9                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_10                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_11                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_12                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_13                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_14                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_15                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_16                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_17                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_18                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_19                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_20                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_21                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_22                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_23                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_24                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_25                                          1

		Drive0 control to MIO Bank 1 - control MIO[51:26]
		(OFFSET, MASK, VALUE)      (0XFF180154, 0x03FFFFFFU ,0x03FFFFFFU)
		RegMask = (IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_0_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_1_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_2_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_3_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_4_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_5_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_6_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_7_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_8_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_9_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_10_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_11_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_12_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_13_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_14_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_15_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_16_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_17_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_18_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_19_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_20_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_21_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_22_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_23_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_24_MASK | IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_25_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_23_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_24_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK1_CTRL0_OFFSET ,0x03FFFFFFU ,0x03FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank1_ctrl1 @ 0XFF180158</p>

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_0                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_1                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_2                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_3                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_4                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_5                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_6                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_7                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_8                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_9                                           1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_10                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_11                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_12                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_13                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_14                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_15                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_16                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_17                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_18                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_19                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_20                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_21                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_22                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_23                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_24                                          1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_25                                          1

		Drive1 control to MIO Bank 1 - control MIO[51:26]
		(OFFSET, MASK, VALUE)      (0XFF180158, 0x03FFFFFFU ,0x03FFFFFFU)
		RegMask = (IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_0_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_1_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_2_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_3_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_4_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_5_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_6_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_7_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_8_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_9_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_10_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_11_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_12_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_13_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_14_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_15_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_16_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_17_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_18_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_19_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_20_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_21_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_22_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_23_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_24_MASK | IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_25_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_23_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_24_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK1_CTRL1_OFFSET ,0x03FFFFFFU ,0x03FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank1_ctrl3 @ 0XFF18015C</p>

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_0                                   0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_1                                   0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_2                                   0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_3                                   0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_4                                   0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_5                                   0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_6                                   0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_7                                   0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_8                                   0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_9                                   0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_10                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_11                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_12                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_13                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_14                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_15                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_16                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_17                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_18                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_19                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_20                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_21                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_22                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_23                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_24                                  0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_25                                  0

		Selects either Schmitt or CMOS input for MIO Bank 1 - control MIO[51:26]
		(OFFSET, MASK, VALUE)      (0XFF18015C, 0x03FFFFFFU ,0x00000000U)
		RegMask = (IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_0_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_1_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_2_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_3_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_4_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_5_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_6_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_7_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_8_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_9_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_10_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_11_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_12_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_13_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_14_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_15_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_16_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_17_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_18_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_19_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_20_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_21_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_22_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_23_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_24_MASK | IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_25_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_0_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_1_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_2_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_3_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_4_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_5_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_6_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_7_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_8_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_9_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_10_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_11_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_12_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_13_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_14_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_15_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_16_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_17_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_18_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_19_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_20_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_21_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_22_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_23_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_24_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK1_CTRL3_OFFSET ,0x03FFFFFFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : bank1_ctrl4 @ 0XFF180160</p>

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_0                                  1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_1                                  1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_2                                  1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_3                                  1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_4                                  1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_5                                  1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_6                                  1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_7                                  1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_8                                  1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_9                                  1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_10                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_11                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_12                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_13                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_14                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_15                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_16                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_17                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_18                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_19                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_20                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_21                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_22                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_23                                 1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_24                                 1

		When mio_bank1_pull_enable is set, this selects pull up or pull down for MIO Bank 1 - control MIO[51:26]
		(OFFSET, MASK, VALUE)      (0XFF180160, 0x01FFFFFFU ,0x01FFFFFFU)
		RegMask = (IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_0_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_1_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_2_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_3_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_4_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_5_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_6_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_7_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_8_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_9_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_10_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_11_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_12_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_13_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_14_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_15_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_16_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_17_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_18_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_19_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_20_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_21_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_22_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_23_MASK | IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_24_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_23_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_24_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK1_CTRL4_OFFSET ,0x01FFFFFFU ,0x01FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank1_ctrl5 @ 0XFF180164</p>

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_0                                      1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_1                                      1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_2                                      1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_3                                      1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_4                                      1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_5                                      1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_6                                      1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_7                                      1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_8                                      1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_9                                      1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_10                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_11                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_12                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_13                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_14                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_15                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_16                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_17                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_18                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_19                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_20                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_21                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_22                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_23                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_24                                     1

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_25                                     0

		When set, this enables mio_bank1_pullupdown to selects pull up or pull down for MIO Bank 1 - control MIO[51:26]
		(OFFSET, MASK, VALUE)      (0XFF180164, 0x03FFFFFFU ,0x01FFFFFFU)
		RegMask = (IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_0_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_1_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_2_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_3_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_4_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_5_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_6_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_7_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_8_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_9_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_10_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_11_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_12_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_13_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_14_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_15_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_16_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_17_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_18_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_19_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_20_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_21_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_22_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_23_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_24_MASK | IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_25_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_23_SHIFT
			| 0x00000001U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_24_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK1_CTRL5_OFFSET ,0x03FFFFFFU ,0x01FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank1_ctrl6 @ 0XFF180168</p>

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_0                                 0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_1                                 0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_2                                 0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_3                                 0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_4                                 0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_5                                 0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_6                                 0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_7                                 0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_8                                 0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_9                                 0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_10                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_11                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_12                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_13                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_14                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_15                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_16                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_17                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_18                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_19                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_20                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_21                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_22                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_23                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_24                                0

		Each bit applies to a single IO. Bit 0 for MIO[26].
		PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_25                                0

		Slew rate control to MIO Bank 1 - control MIO[51:26]
		(OFFSET, MASK, VALUE)      (0XFF180168, 0x03FFFFFFU ,0x00000000U)
		RegMask = (IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_0_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_1_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_2_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_3_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_4_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_5_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_6_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_7_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_8_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_9_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_10_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_11_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_12_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_13_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_14_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_15_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_16_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_17_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_18_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_19_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_20_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_21_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_22_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_23_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_24_MASK | IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_25_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_0_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_1_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_2_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_3_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_4_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_5_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_6_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_7_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_8_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_9_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_10_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_11_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_12_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_13_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_14_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_15_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_16_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_17_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_18_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_19_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_20_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_21_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_22_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_23_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_24_SHIFT
			| 0x00000000U << IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK1_CTRL6_OFFSET ,0x03FFFFFFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : bank2_ctrl0 @ 0XFF180170</p>

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_0                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_1                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_2                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_3                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_4                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_5                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_6                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_7                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_8                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_9                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_10                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_11                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_12                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_13                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_14                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_15                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_16                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_17                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_18                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_19                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_20                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_21                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_22                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_23                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_24                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_25                                          1

		Drive0 control to MIO Bank 2 - control MIO[77:52]
		(OFFSET, MASK, VALUE)      (0XFF180170, 0x03FFFFFFU ,0x03FFFFFFU)
		RegMask = (IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_0_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_1_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_2_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_3_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_4_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_5_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_6_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_7_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_8_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_9_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_10_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_11_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_12_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_13_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_14_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_15_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_16_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_17_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_18_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_19_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_20_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_21_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_22_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_23_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_24_MASK | IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_25_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_23_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_24_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK2_CTRL0_OFFSET ,0x03FFFFFFU ,0x03FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank2_ctrl1 @ 0XFF180174</p>

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_0                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_1                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_2                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_3                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_4                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_5                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_6                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_7                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_8                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_9                                           1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_10                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_11                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_12                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_13                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_14                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_15                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_16                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_17                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_18                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_19                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_20                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_21                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_22                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_23                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_24                                          1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_25                                          1

		Drive1 control to MIO Bank 2 - control MIO[77:52]
		(OFFSET, MASK, VALUE)      (0XFF180174, 0x03FFFFFFU ,0x03FFFFFFU)
		RegMask = (IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_0_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_1_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_2_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_3_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_4_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_5_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_6_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_7_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_8_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_9_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_10_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_11_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_12_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_13_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_14_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_15_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_16_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_17_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_18_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_19_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_20_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_21_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_22_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_23_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_24_MASK | IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_25_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_23_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_24_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK2_CTRL1_OFFSET ,0x03FFFFFFU ,0x03FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank2_ctrl3 @ 0XFF180178</p>

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_0                                   0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_1                                   0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_2                                   0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_3                                   0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_4                                   0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_5                                   0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_6                                   0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_7                                   0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_8                                   0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_9                                   0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_10                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_11                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_12                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_13                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_14                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_15                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_16                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_17                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_18                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_19                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_20                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_21                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_22                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_23                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_24                                  0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_25                                  0

		Selects either Schmitt or CMOS input for MIO Bank 2 - control MIO[77:52]
		(OFFSET, MASK, VALUE)      (0XFF180178, 0x03FFFFFFU ,0x00000000U)
		RegMask = (IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_0_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_1_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_2_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_3_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_4_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_5_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_6_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_7_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_8_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_9_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_10_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_11_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_12_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_13_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_14_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_15_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_16_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_17_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_18_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_19_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_20_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_21_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_22_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_23_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_24_MASK | IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_25_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_0_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_1_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_2_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_3_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_4_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_5_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_6_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_7_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_8_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_9_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_10_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_11_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_12_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_13_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_14_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_15_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_16_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_17_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_18_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_19_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_20_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_21_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_22_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_23_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_24_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK2_CTRL3_OFFSET ,0x03FFFFFFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : bank2_ctrl4 @ 0XFF18017C</p>

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_0                                  1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_1                                  1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_2                                  1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_3                                  1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_4                                  1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_5                                  1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_6                                  1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_7                                  1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_8                                  1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_9                                  1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_10                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_11                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_12                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_13                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_14                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_15                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_16                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_17                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_18                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_19                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_20                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_21                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_22                                 1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_23                                 1

		When mio_bank2_pull_enable is set, this selects pull up or pull down for MIO Bank 2 - control MIO[77:52]
		(OFFSET, MASK, VALUE)      (0XFF18017C, 0x00FFFFFFU ,0x00FFFFFFU)
		RegMask = (IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_0_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_1_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_2_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_3_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_4_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_5_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_6_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_7_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_8_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_9_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_10_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_11_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_12_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_13_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_14_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_15_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_16_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_17_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_18_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_19_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_20_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_21_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_22_MASK | IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_23_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_23_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK2_CTRL4_OFFSET ,0x00FFFFFFU ,0x00FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank2_ctrl5 @ 0XFF180180</p>

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_0                                      1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_1                                      1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_2                                      1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_3                                      1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_4                                      1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_5                                      1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_6                                      1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_7                                      1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_8                                      1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_9                                      1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_10                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_11                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_12                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_13                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_14                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_15                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_16                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_17                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_18                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_19                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_20                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_21                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_22                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_23                                     1

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_24                                     0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_25                                     0

		When set, this enables mio_bank2_pullupdown to selects pull up or pull down for MIO Bank 2 - control MIO[77:52]
		(OFFSET, MASK, VALUE)      (0XFF180180, 0x03FFFFFFU ,0x00FFFFFFU)
		RegMask = (IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_0_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_1_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_2_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_3_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_4_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_5_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_6_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_7_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_8_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_9_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_10_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_11_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_12_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_13_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_14_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_15_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_16_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_17_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_18_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_19_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_20_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_21_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_22_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_23_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_24_MASK | IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_25_MASK |  0 );

		RegVal = ((0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_0_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_1_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_2_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_3_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_4_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_5_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_6_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_7_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_8_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_9_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_10_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_11_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_12_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_13_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_14_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_15_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_16_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_17_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_18_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_19_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_20_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_21_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_22_SHIFT
			| 0x00000001U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_23_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_24_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK2_CTRL5_OFFSET ,0x03FFFFFFU ,0x00FFFFFFU);
	/*############################################################################################################################ */

		/*Register : bank2_ctrl6 @ 0XFF180184</p>

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_0                                 0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_1                                 0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_2                                 0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_3                                 0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_4                                 0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_5                                 0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_6                                 0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_7                                 0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_8                                 0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_9                                 0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_10                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_11                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_12                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_13                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_14                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_15                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_16                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_17                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_18                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_19                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_20                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_21                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_22                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_23                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_24                                0

		Each bit applies to a single IO. Bit 0 for MIO[52].
		PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_25                                0

		Slew rate control to MIO Bank 2 - control MIO[77:52]
		(OFFSET, MASK, VALUE)      (0XFF180184, 0x03FFFFFFU ,0x00000000U)
		RegMask = (IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_0_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_1_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_2_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_3_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_4_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_5_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_6_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_7_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_8_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_9_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_10_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_11_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_12_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_13_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_14_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_15_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_16_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_17_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_18_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_19_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_20_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_21_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_22_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_23_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_24_MASK | IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_25_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_0_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_1_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_2_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_3_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_4_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_5_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_6_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_7_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_8_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_9_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_10_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_11_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_12_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_13_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_14_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_15_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_16_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_17_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_18_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_19_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_20_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_21_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_22_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_23_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_24_SHIFT
			| 0x00000000U << IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_25_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_BANK2_CTRL6_OFFSET ,0x03FFFFFFU ,0x00000000U);
	/*############################################################################################################################ */

		// : LOOPBACK
		/*Register : MIO_LOOPBACK @ 0XFF180200</p>

		I2C Loopback Control. 0 = Connect I2C inputs according to MIO mapping. 1 = Loop I2C 0 outputs to I2C 1 inputs, and I2C 1 outp
		ts to I2C 0 inputs.
		PSU_IOU_SLCR_MIO_LOOPBACK_I2C0_LOOP_I2C1                                        0

		CAN Loopback Control. 0 = Connect CAN inputs according to MIO mapping. 1 = Loop CAN 0 Tx to CAN 1 Rx, and CAN 1 Tx to CAN 0 R
		.
		PSU_IOU_SLCR_MIO_LOOPBACK_CAN0_LOOP_CAN1                                        0

		UART Loopback Control. 0 = Connect UART inputs according to MIO mapping. 1 = Loop UART 0 outputs to UART 1 inputs, and UART 1
		outputs to UART 0 inputs. RXD/TXD cross-connected. RTS/CTS cross-connected. DSR, DTR, DCD and RI not used.
		PSU_IOU_SLCR_MIO_LOOPBACK_UA0_LOOP_UA1                                          0

		SPI Loopback Control. 0 = Connect SPI inputs according to MIO mapping. 1 = Loop SPI 0 outputs to SPI 1 inputs, and SPI 1 outp
		ts to SPI 0 inputs. The other SPI core will appear on the LS Slave Select.
		PSU_IOU_SLCR_MIO_LOOPBACK_SPI0_LOOP_SPI1                                        0

		Loopback function within MIO
		(OFFSET, MASK, VALUE)      (0XFF180200, 0x0000000FU ,0x00000000U)
		RegMask = (IOU_SLCR_MIO_LOOPBACK_I2C0_LOOP_I2C1_MASK | IOU_SLCR_MIO_LOOPBACK_CAN0_LOOP_CAN1_MASK | IOU_SLCR_MIO_LOOPBACK_UA0_LOOP_UA1_MASK | IOU_SLCR_MIO_LOOPBACK_SPI0_LOOP_SPI1_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_MIO_LOOPBACK_I2C0_LOOP_I2C1_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_LOOPBACK_CAN0_LOOP_CAN1_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_LOOPBACK_UA0_LOOP_UA1_SHIFT
			| 0x00000000U << IOU_SLCR_MIO_LOOPBACK_SPI0_LOOP_SPI1_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_MIO_LOOPBACK_OFFSET ,0x0000000FU ,0x00000000U);
	/*############################################################################################################################ */


  return 1;
}
unsigned long psu_peripherals_init_data() {
		// : RESET BLOCKS
		// : ENET
		// : QSPI
		/*Register : RST_LPD_IOU2 @ 0XFF5E0238</p>

		Block level reset
		PSU_CRL_APB_RST_LPD_IOU2_QSPI_RESET                                             0

		Software control register for the IOU block. Each bit will cause a singlerperipheral or part of the peripheral to be reset.
		(OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00000001U ,0x00000000U)
		RegMask = (CRL_APB_RST_LPD_IOU2_QSPI_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RST_LPD_IOU2_QSPI_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_IOU2_OFFSET ,0x00000001U ,0x00000000U);
	/*############################################################################################################################ */

		// : NAND
		// : USB
		/*Register : RST_LPD_TOP @ 0XFF5E023C</p>

		USB 0 reset for control registers
		PSU_CRL_APB_RST_LPD_TOP_USB0_APB_RESET                                          0

		USB 1 reset for control registers
		PSU_CRL_APB_RST_LPD_TOP_USB1_APB_RESET                                          0

		USB 0 sleep circuit reset
		PSU_CRL_APB_RST_LPD_TOP_USB0_HIBERRESET                                         0

		USB 1 sleep circuit reset
		PSU_CRL_APB_RST_LPD_TOP_USB1_HIBERRESET                                         0

		USB 0 reset
		PSU_CRL_APB_RST_LPD_TOP_USB0_CORERESET                                          0

		USB 1 reset
		PSU_CRL_APB_RST_LPD_TOP_USB1_CORERESET                                          0

		Software control register for the LPD block.
		(OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00000FC0U ,0x00000000U)
		RegMask = (CRL_APB_RST_LPD_TOP_USB0_APB_RESET_MASK | CRL_APB_RST_LPD_TOP_USB1_APB_RESET_MASK | CRL_APB_RST_LPD_TOP_USB0_HIBERRESET_MASK | CRL_APB_RST_LPD_TOP_USB1_HIBERRESET_MASK | CRL_APB_RST_LPD_TOP_USB0_CORERESET_MASK | CRL_APB_RST_LPD_TOP_USB1_CORERESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RST_LPD_TOP_USB0_APB_RESET_SHIFT
			| 0x00000000U << CRL_APB_RST_LPD_TOP_USB1_APB_RESET_SHIFT
			| 0x00000000U << CRL_APB_RST_LPD_TOP_USB0_HIBERRESET_SHIFT
			| 0x00000000U << CRL_APB_RST_LPD_TOP_USB1_HIBERRESET_SHIFT
			| 0x00000000U << CRL_APB_RST_LPD_TOP_USB0_CORERESET_SHIFT
			| 0x00000000U << CRL_APB_RST_LPD_TOP_USB1_CORERESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_TOP_OFFSET ,0x00000FC0U ,0x00000000U);
	/*############################################################################################################################ */

		// : FPD RESET
		/*Register : RST_FPD_TOP @ 0XFD1A0100</p>

		Display Port block level reset (includes DPDMA)
		PSU_CRF_APB_RST_FPD_TOP_DP_RESET                                                0

		GDMA block level reset
		PSU_CRF_APB_RST_FPD_TOP_GDMA_RESET                                              0

		Pixel Processor (submodule of GPU) block level reset
		PSU_CRF_APB_RST_FPD_TOP_GPU_PP0_RESET                                           0

		Pixel Processor (submodule of GPU) block level reset
		PSU_CRF_APB_RST_FPD_TOP_GPU_PP1_RESET                                           0

		GPU block level reset
		PSU_CRF_APB_RST_FPD_TOP_GPU_RESET                                               0

		GT block level reset
		PSU_CRF_APB_RST_FPD_TOP_GT_RESET                                                0

		FPD Block level software controlled reset
		(OFFSET, MASK, VALUE)      (0XFD1A0100, 0x0001007CU ,0x00000000U)
		RegMask = (CRF_APB_RST_FPD_TOP_DP_RESET_MASK | CRF_APB_RST_FPD_TOP_GDMA_RESET_MASK | CRF_APB_RST_FPD_TOP_GPU_PP0_RESET_MASK | CRF_APB_RST_FPD_TOP_GPU_PP1_RESET_MASK | CRF_APB_RST_FPD_TOP_GPU_RESET_MASK | CRF_APB_RST_FPD_TOP_GT_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_RST_FPD_TOP_DP_RESET_SHIFT
			| 0x00000000U << CRF_APB_RST_FPD_TOP_GDMA_RESET_SHIFT
			| 0x00000000U << CRF_APB_RST_FPD_TOP_GPU_PP0_RESET_SHIFT
			| 0x00000000U << CRF_APB_RST_FPD_TOP_GPU_PP1_RESET_SHIFT
			| 0x00000000U << CRF_APB_RST_FPD_TOP_GPU_RESET_SHIFT
			| 0x00000000U << CRF_APB_RST_FPD_TOP_GT_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_RST_FPD_TOP_OFFSET ,0x0001007CU ,0x00000000U);
	/*############################################################################################################################ */

		// : SD
		/*Register : RST_LPD_IOU2 @ 0XFF5E0238</p>

		Block level reset
		PSU_CRL_APB_RST_LPD_IOU2_SDIO0_RESET                                            0

		Block level reset
		PSU_CRL_APB_RST_LPD_IOU2_SDIO1_RESET                                            0

		Software control register for the IOU block. Each bit will cause a singlerperipheral or part of the peripheral to be reset.
		(OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00000060U ,0x00000000U)
		RegMask = (CRL_APB_RST_LPD_IOU2_SDIO0_RESET_MASK | CRL_APB_RST_LPD_IOU2_SDIO1_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RST_LPD_IOU2_SDIO0_RESET_SHIFT
			| 0x00000000U << CRL_APB_RST_LPD_IOU2_SDIO1_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_IOU2_OFFSET ,0x00000060U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : CTRL_REG_SD @ 0XFF180310</p>

		SD or eMMC selection on SDIO0 0: SD enabled 1: eMMC enabled
		PSU_IOU_SLCR_CTRL_REG_SD_SD0_EMMC_SEL                                           0

		SD or eMMC selection on SDIO1 0: SD enabled 1: eMMC enabled
		PSU_IOU_SLCR_CTRL_REG_SD_SD1_EMMC_SEL                                           0

		SD eMMC selection
		(OFFSET, MASK, VALUE)      (0XFF180310, 0x00008001U ,0x00000000U)
		RegMask = (IOU_SLCR_CTRL_REG_SD_SD0_EMMC_SEL_MASK | IOU_SLCR_CTRL_REG_SD_SD1_EMMC_SEL_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_CTRL_REG_SD_SD0_EMMC_SEL_SHIFT
			| 0x00000000U << IOU_SLCR_CTRL_REG_SD_SD1_EMMC_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_CTRL_REG_SD_OFFSET ,0x00008001U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : SD_CONFIG_REG2 @ 0XFF180320</p>

		Should be set based on the final product usage 00 - Removable SCard Slot 01 - Embedded Slot for One Device 10 - Shared Bus Sl
		t 11 - Reserved
		PSU_IOU_SLCR_SD_CONFIG_REG2_SD0_SLOTTYPE                                        0

		Should be set based on the final product usage 00 - Removable SCard Slot 01 - Embedded Slot for One Device 10 - Shared Bus Sl
		t 11 - Reserved
		PSU_IOU_SLCR_SD_CONFIG_REG2_SD1_SLOTTYPE                                        0

		1.8V Support 1: 1.8V supported 0: 1.8V not supported support
		PSU_IOU_SLCR_SD_CONFIG_REG2_SD0_1P8V                                            0

		3.0V Support 1: 3.0V supported 0: 3.0V not supported support
		PSU_IOU_SLCR_SD_CONFIG_REG2_SD0_3P0V                                            0

		3.3V Support 1: 3.3V supported 0: 3.3V not supported support
		PSU_IOU_SLCR_SD_CONFIG_REG2_SD0_3P3V                                            1

		1.8V Support 1: 1.8V supported 0: 1.8V not supported support
		PSU_IOU_SLCR_SD_CONFIG_REG2_SD1_1P8V                                            0

		3.0V Support 1: 3.0V supported 0: 3.0V not supported support
		PSU_IOU_SLCR_SD_CONFIG_REG2_SD1_3P0V                                            0

		3.3V Support 1: 3.3V supported 0: 3.3V not supported support
		PSU_IOU_SLCR_SD_CONFIG_REG2_SD1_3P3V                                            1

		SD Config Register 2
		(OFFSET, MASK, VALUE)      (0XFF180320, 0x33803380U ,0x00800080U)
		RegMask = (IOU_SLCR_SD_CONFIG_REG2_SD0_SLOTTYPE_MASK | IOU_SLCR_SD_CONFIG_REG2_SD1_SLOTTYPE_MASK | IOU_SLCR_SD_CONFIG_REG2_SD0_1P8V_MASK | IOU_SLCR_SD_CONFIG_REG2_SD0_3P0V_MASK | IOU_SLCR_SD_CONFIG_REG2_SD0_3P3V_MASK | IOU_SLCR_SD_CONFIG_REG2_SD1_1P8V_MASK | IOU_SLCR_SD_CONFIG_REG2_SD1_3P0V_MASK | IOU_SLCR_SD_CONFIG_REG2_SD1_3P3V_MASK |  0 );

		RegVal = ((0x00000000U << IOU_SLCR_SD_CONFIG_REG2_SD0_SLOTTYPE_SHIFT
			| 0x00000000U << IOU_SLCR_SD_CONFIG_REG2_SD1_SLOTTYPE_SHIFT
			| 0x00000000U << IOU_SLCR_SD_CONFIG_REG2_SD0_1P8V_SHIFT
			| 0x00000000U << IOU_SLCR_SD_CONFIG_REG2_SD0_3P0V_SHIFT
			| 0x00000001U << IOU_SLCR_SD_CONFIG_REG2_SD0_3P3V_SHIFT
			| 0x00000000U << IOU_SLCR_SD_CONFIG_REG2_SD1_1P8V_SHIFT
			| 0x00000000U << IOU_SLCR_SD_CONFIG_REG2_SD1_3P0V_SHIFT
			| 0x00000001U << IOU_SLCR_SD_CONFIG_REG2_SD1_3P3V_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_SD_CONFIG_REG2_OFFSET ,0x33803380U ,0x00800080U);
	/*############################################################################################################################ */

		// : SD0 BASE CLOCK
		/*Register : SD_CONFIG_REG1 @ 0XFF18031C</p>

		Base Clock Frequency for SD Clock. This is the frequency of the xin_clk.
		PSU_IOU_SLCR_SD_CONFIG_REG1_SD0_BASECLK                                         0xc7

		SD Config Register 1
		(OFFSET, MASK, VALUE)      (0XFF18031C, 0x00007F80U ,0x00006380U)
		RegMask = (IOU_SLCR_SD_CONFIG_REG1_SD0_BASECLK_MASK |  0 );

		RegVal = ((0x000000C7U << IOU_SLCR_SD_CONFIG_REG1_SD0_BASECLK_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_SD_CONFIG_REG1_OFFSET ,0x00007F80U ,0x00006380U);
	/*############################################################################################################################ */

		// : SD1 BASE CLOCK
		/*Register : SD_CONFIG_REG1 @ 0XFF18031C</p>

		Base Clock Frequency for SD Clock. This is the frequency of the xin_clk.
		PSU_IOU_SLCR_SD_CONFIG_REG1_SD1_BASECLK                                         0xc7

		SD Config Register 1
		(OFFSET, MASK, VALUE)      (0XFF18031C, 0x7F800000U ,0x63800000U)
		RegMask = (IOU_SLCR_SD_CONFIG_REG1_SD1_BASECLK_MASK |  0 );

		RegVal = ((0x000000C7U << IOU_SLCR_SD_CONFIG_REG1_SD1_BASECLK_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (IOU_SLCR_SD_CONFIG_REG1_OFFSET ,0x7F800000U ,0x63800000U);
	/*############################################################################################################################ */

		// : CAN
		// : I2C
		/*Register : RST_LPD_IOU2 @ 0XFF5E0238</p>

		Block level reset
		PSU_CRL_APB_RST_LPD_IOU2_I2C0_RESET                                             0

		Software control register for the IOU block. Each bit will cause a singlerperipheral or part of the peripheral to be reset.
		(OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00000200U ,0x00000000U)
		RegMask = (CRL_APB_RST_LPD_IOU2_I2C0_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RST_LPD_IOU2_I2C0_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_IOU2_OFFSET ,0x00000200U ,0x00000000U);
	/*############################################################################################################################ */

		// : SWDT
		// : SPI
		/*Register : RST_LPD_IOU2 @ 0XFF5E0238</p>

		Block level reset
		PSU_CRL_APB_RST_LPD_IOU2_SPI0_RESET                                             0

		Software control register for the IOU block. Each bit will cause a singlerperipheral or part of the peripheral to be reset.
		(OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00000008U ,0x00000000U)
		RegMask = (CRL_APB_RST_LPD_IOU2_SPI0_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RST_LPD_IOU2_SPI0_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_IOU2_OFFSET ,0x00000008U ,0x00000000U);
	/*############################################################################################################################ */

		// : TTC
		// : UART
		/*Register : RST_LPD_IOU2 @ 0XFF5E0238</p>

		Block level reset
		PSU_CRL_APB_RST_LPD_IOU2_UART1_RESET                                            0

		Software control register for the IOU block. Each bit will cause a singlerperipheral or part of the peripheral to be reset.
		(OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00000004U ,0x00000000U)
		RegMask = (CRL_APB_RST_LPD_IOU2_UART1_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RST_LPD_IOU2_UART1_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_IOU2_OFFSET ,0x00000004U ,0x00000000U);
	/*############################################################################################################################ */

		// : UART BAUD RATE
		/*Register : Baud_rate_divider_reg0 @ 0XFF010034</p>

		Baud rate divider value: 0 - 3: ignored 4 - 255: Baud rate
		PSU_UART1_BAUD_RATE_DIVIDER_REG0_BDIV                                           0x5

		Baud Rate Divider Register
		(OFFSET, MASK, VALUE)      (0XFF010034, 0x000000FFU ,0x00000005U)
		RegMask = (UART1_BAUD_RATE_DIVIDER_REG0_BDIV_MASK |  0 );

		RegVal = ((0x00000005U << UART1_BAUD_RATE_DIVIDER_REG0_BDIV_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (UART1_BAUD_RATE_DIVIDER_REG0_OFFSET ,0x000000FFU ,0x00000005U);
	/*############################################################################################################################ */

		/*Register : Baud_rate_gen_reg0 @ 0XFF010018</p>

		Baud Rate Clock Divisor Value: 0: Disables baud_sample 1: Clock divisor bypass (baud_sample = sel_clk) 2 - 65535: baud_sample
		PSU_UART1_BAUD_RATE_GEN_REG0_CD                                                 0x8f

		Baud Rate Generator Register.
		(OFFSET, MASK, VALUE)      (0XFF010018, 0x0000FFFFU ,0x0000008FU)
		RegMask = (UART1_BAUD_RATE_GEN_REG0_CD_MASK |  0 );

		RegVal = ((0x0000008FU << UART1_BAUD_RATE_GEN_REG0_CD_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (UART1_BAUD_RATE_GEN_REG0_OFFSET ,0x0000FFFFU ,0x0000008FU);
	/*############################################################################################################################ */

		/*Register : Control_reg0 @ 0XFF010000</p>

		Stop transmitter break: 0: no affect 1: stop transmission of the break after a minimum of one character length and transmit a
		high level during 12 bit periods. It can be set regardless of the value of STTBRK.
		PSU_UART1_CONTROL_REG0_STPBRK                                                   0x0

		Start transmitter break: 0: no affect 1: start to transmit a break after the characters currently present in the FIFO and the
		transmit shift register have been transmitted. It can only be set if STPBRK (Stop transmitter break) is not high.
		PSU_UART1_CONTROL_REG0_STTBRK                                                   0x0

		Restart receiver timeout counter: 1: receiver timeout counter is restarted. This bit is self clearing once the restart has co
		pleted.
		PSU_UART1_CONTROL_REG0_RSTTO                                                    0x0

		Transmit disable: 0: enable transmitter 1: disable transmitter
		PSU_UART1_CONTROL_REG0_TXDIS                                                    0x0

		Transmit enable: 0: disable transmitter 1: enable transmitter, provided the TXDIS field is set to 0.
		PSU_UART1_CONTROL_REG0_TXEN                                                     0x1

		Receive disable: 0: enable 1: disable, regardless of the value of RXEN
		PSU_UART1_CONTROL_REG0_RXDIS                                                    0x0

		Receive enable: 0: disable 1: enable When set to one, the receiver logic is enabled, provided the RXDIS field is set to zero.
		PSU_UART1_CONTROL_REG0_RXEN                                                     0x1

		Software reset for Tx data path: 0: no affect 1: transmitter logic is reset and all pending transmitter data is discarded Thi
		 bit is self clearing once the reset has completed.
		PSU_UART1_CONTROL_REG0_TXRES                                                    0x1

		Software reset for Rx data path: 0: no affect 1: receiver logic is reset and all pending receiver data is discarded. This bit
		is self clearing once the reset has completed.
		PSU_UART1_CONTROL_REG0_RXRES                                                    0x1

		UART Control Register
		(OFFSET, MASK, VALUE)      (0XFF010000, 0x000001FFU ,0x00000017U)
		RegMask = (UART1_CONTROL_REG0_STPBRK_MASK | UART1_CONTROL_REG0_STTBRK_MASK | UART1_CONTROL_REG0_RSTTO_MASK | UART1_CONTROL_REG0_TXDIS_MASK | UART1_CONTROL_REG0_TXEN_MASK | UART1_CONTROL_REG0_RXDIS_MASK | UART1_CONTROL_REG0_RXEN_MASK | UART1_CONTROL_REG0_TXRES_MASK | UART1_CONTROL_REG0_RXRES_MASK |  0 );

		RegVal = ((0x00000000U << UART1_CONTROL_REG0_STPBRK_SHIFT
			| 0x00000000U << UART1_CONTROL_REG0_STTBRK_SHIFT
			| 0x00000000U << UART1_CONTROL_REG0_RSTTO_SHIFT
			| 0x00000000U << UART1_CONTROL_REG0_TXDIS_SHIFT
			| 0x00000001U << UART1_CONTROL_REG0_TXEN_SHIFT
			| 0x00000000U << UART1_CONTROL_REG0_RXDIS_SHIFT
			| 0x00000001U << UART1_CONTROL_REG0_RXEN_SHIFT
			| 0x00000001U << UART1_CONTROL_REG0_TXRES_SHIFT
			| 0x00000001U << UART1_CONTROL_REG0_RXRES_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (UART1_CONTROL_REG0_OFFSET ,0x000001FFU ,0x00000017U);
	/*############################################################################################################################ */

		/*Register : mode_reg0 @ 0XFF010004</p>

		Channel mode: Defines the mode of operation of the UART. 00: normal 01: automatic echo 10: local loopback 11: remote loopback
		PSU_UART1_MODE_REG0_CHMODE                                                      0x0

		Number of stop bits: Defines the number of stop bits to detect on receive and to generate on transmit. 00: 1 stop bit 01: 1.5
		stop bits 10: 2 stop bits 11: reserved
		PSU_UART1_MODE_REG0_NBSTOP                                                      0x0

		Parity type select: Defines the expected parity to check on receive and the parity to generate on transmit. 000: even parity
		01: odd parity 010: forced to 0 parity (space) 011: forced to 1 parity (mark) 1xx: no parity
		PSU_UART1_MODE_REG0_PAR                                                         0x4

		Character length select: Defines the number of bits in each character. 11: 6 bits 10: 7 bits 0x: 8 bits
		PSU_UART1_MODE_REG0_CHRL                                                        0x0

		Clock source select: This field defines whether a pre-scalar of 8 is applied to the baud rate generator input clock. 0: clock
		source is uart_ref_clk 1: clock source is uart_ref_clk/8
		PSU_UART1_MODE_REG0_CLKS                                                        0x0

		UART Mode Register
		(OFFSET, MASK, VALUE)      (0XFF010004, 0x000003FFU ,0x00000020U)
		RegMask = (UART1_MODE_REG0_CHMODE_MASK | UART1_MODE_REG0_NBSTOP_MASK | UART1_MODE_REG0_PAR_MASK | UART1_MODE_REG0_CHRL_MASK | UART1_MODE_REG0_CLKS_MASK |  0 );

		RegVal = ((0x00000000U << UART1_MODE_REG0_CHMODE_SHIFT
			| 0x00000000U << UART1_MODE_REG0_NBSTOP_SHIFT
			| 0x00000004U << UART1_MODE_REG0_PAR_SHIFT
			| 0x00000000U << UART1_MODE_REG0_CHRL_SHIFT
			| 0x00000000U << UART1_MODE_REG0_CLKS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (UART1_MODE_REG0_OFFSET ,0x000003FFU ,0x00000020U);
	/*############################################################################################################################ */

		// : GPIO
		// : ADMA TZ
		/*Register : slcr_adma @ 0XFF4B0024</p>

		TrustZone Classification for ADMA
		PSU_LPD_SLCR_SECURE_SLCR_ADMA_TZ                                                0XFF

		RPU TrustZone settings
		(OFFSET, MASK, VALUE)      (0XFF4B0024, 0x000000FFU ,0x000000FFU)
		RegMask = (LPD_SLCR_SECURE_SLCR_ADMA_TZ_MASK |  0 );

		RegVal = ((0x000000FFU << LPD_SLCR_SECURE_SLCR_ADMA_TZ_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (LPD_SLCR_SECURE_SLCR_ADMA_OFFSET ,0x000000FFU ,0x000000FFU);
	/*############################################################################################################################ */

		// : CSU TAMPERING
		// : CSU TAMPER STATUS
		/*Register : tamper_status @ 0XFFCA5000</p>

		CSU regsiter
		PSU_CSU_TAMPER_STATUS_TAMPER_0                                                  0

		External MIO
		PSU_CSU_TAMPER_STATUS_TAMPER_1                                                  0

		JTAG toggle detect
		PSU_CSU_TAMPER_STATUS_TAMPER_2                                                  0

		PL SEU error
		PSU_CSU_TAMPER_STATUS_TAMPER_3                                                  0

		AMS over temperature alarm for LPD
		PSU_CSU_TAMPER_STATUS_TAMPER_4                                                  0

		AMS over temperature alarm for APU
		PSU_CSU_TAMPER_STATUS_TAMPER_5                                                  0

		AMS voltage alarm for VCCPINT_FPD
		PSU_CSU_TAMPER_STATUS_TAMPER_6                                                  0

		AMS voltage alarm for VCCPINT_LPD
		PSU_CSU_TAMPER_STATUS_TAMPER_7                                                  0

		AMS voltage alarm for VCCPAUX
		PSU_CSU_TAMPER_STATUS_TAMPER_8                                                  0

		AMS voltage alarm for DDRPHY
		PSU_CSU_TAMPER_STATUS_TAMPER_9                                                  0

		AMS voltage alarm for PSIO bank 0/1/2
		PSU_CSU_TAMPER_STATUS_TAMPER_10                                                 0

		AMS voltage alarm for PSIO bank 3 (dedicated pins)
		PSU_CSU_TAMPER_STATUS_TAMPER_11                                                 0

		AMS voltaage alarm for GT
		PSU_CSU_TAMPER_STATUS_TAMPER_12                                                 0

		Tamper Response Status
		(OFFSET, MASK, VALUE)      (0XFFCA5000, 0x00001FFFU ,0x00000000U)
		RegMask = (CSU_TAMPER_STATUS_TAMPER_0_MASK | CSU_TAMPER_STATUS_TAMPER_1_MASK | CSU_TAMPER_STATUS_TAMPER_2_MASK | CSU_TAMPER_STATUS_TAMPER_3_MASK | CSU_TAMPER_STATUS_TAMPER_4_MASK | CSU_TAMPER_STATUS_TAMPER_5_MASK | CSU_TAMPER_STATUS_TAMPER_6_MASK | CSU_TAMPER_STATUS_TAMPER_7_MASK | CSU_TAMPER_STATUS_TAMPER_8_MASK | CSU_TAMPER_STATUS_TAMPER_9_MASK | CSU_TAMPER_STATUS_TAMPER_10_MASK | CSU_TAMPER_STATUS_TAMPER_11_MASK | CSU_TAMPER_STATUS_TAMPER_12_MASK |  0 );

		RegVal = ((0x00000000U << CSU_TAMPER_STATUS_TAMPER_0_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_1_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_2_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_3_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_4_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_5_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_6_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_7_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_8_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_9_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_10_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_11_SHIFT
			| 0x00000000U << CSU_TAMPER_STATUS_TAMPER_12_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CSU_TAMPER_STATUS_OFFSET ,0x00001FFFU ,0x00000000U);
	/*############################################################################################################################ */

		// : CSU TAMPER RESPONSE
		// : AFIFM INTERFACE WIDTH
		// : CPU QOS DEFAULT
		/*Register : ACE_CTRL @ 0XFD5C0060</p>

		Set ACE outgoing AWQOS value
		PSU_APU_ACE_CTRL_AWQOS                                                          0X0

		Set ACE outgoing ARQOS value
		PSU_APU_ACE_CTRL_ARQOS                                                          0X0

		ACE Control Register
		(OFFSET, MASK, VALUE)      (0XFD5C0060, 0x000F000FU ,0x00000000U)
		RegMask = (APU_ACE_CTRL_AWQOS_MASK | APU_ACE_CTRL_ARQOS_MASK |  0 );

		RegVal = ((0x00000000U << APU_ACE_CTRL_AWQOS_SHIFT
			| 0x00000000U << APU_ACE_CTRL_ARQOS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (APU_ACE_CTRL_OFFSET ,0x000F000FU ,0x00000000U);
	/*############################################################################################################################ */

		// : ENABLES RTC SWITCH TO BATTERY WHEN VCC_PSAUX IS NOT AVAILABLE
		/*Register : CONTROL @ 0XFFA60040</p>

		Enables the RTC. By writing a 0 to this bit, RTC will be powered off and the only module that potentially draws current from
		he battery will be BBRAM. The value read through this bit does not necessarily reflect whether RTC is enabled or not. It is e
		pected that RTC is enabled every time it is being configured. If RTC is not used in the design, FSBL will disable it by writi
		g a 0 to this bit.
		PSU_RTC_CONTROL_BATTERY_DISABLE                                                 0X1

		This register controls various functionalities within the RTC
		(OFFSET, MASK, VALUE)      (0XFFA60040, 0x80000000U ,0x80000000U)
		RegMask = (RTC_CONTROL_BATTERY_DISABLE_MASK |  0 );

		RegVal = ((0x00000001U << RTC_CONTROL_BATTERY_DISABLE_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (RTC_CONTROL_OFFSET ,0x80000000U ,0x80000000U);
	/*############################################################################################################################ */


  return 1;
}
unsigned long psu_post_config_data() {
		// : POST_CONFIG

  return 1;
}
unsigned long psu_peripherals_powerdwn_data() {
		// : POWER DOWN REQUEST INTERRUPT ENABLE
		// : POWER DOWN TRIGGER

  return 1;
}
unsigned long psu_serdes_init_data() {
		// : SERDES INITIALIZATION
		// : GT REFERENCE CLOCK SOURCE SELECTION
		/*Register : PLL_REF_SEL0 @ 0XFD410000</p>

		PLL0 Reference Selection. 0x0 - 5MHz, 0x1 - 9.6MHz, 0x2 - 10MHz, 0x3 - 12MHz, 0x4 - 13MHz, 0x5 - 19.2MHz, 0x6 - 20MHz, 0x7 -
		4MHz, 0x8 - 26MHz, 0x9 - 27MHz, 0xA - 38.4MHz, 0xB - 40MHz, 0xC - 52MHz, 0xD - 100MHz, 0xE - 108MHz, 0xF - 125MHz, 0x10 - 135
		Hz, 0x11 - 150 MHz. 0x12 to 0x1F - Reserved
		PSU_SERDES_PLL_REF_SEL0_PLLREFSEL0                                              0x8

		PLL0 Reference Selection Register
		(OFFSET, MASK, VALUE)      (0XFD410000, 0x0000001FU ,0x00000008U)
		RegMask = (SERDES_PLL_REF_SEL0_PLLREFSEL0_MASK |  0 );

		RegVal = ((0x00000008U << SERDES_PLL_REF_SEL0_PLLREFSEL0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_PLL_REF_SEL0_OFFSET ,0x0000001FU ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : PLL_REF_SEL1 @ 0XFD410004</p>

		PLL1 Reference Selection. 0x0 - 5MHz, 0x1 - 9.6MHz, 0x2 - 10MHz, 0x3 - 12MHz, 0x4 - 13MHz, 0x5 - 19.2MHz, 0x6 - 20MHz, 0x7 -
		4MHz, 0x8 - 26MHz, 0x9 - 27MHz, 0xA - 38.4MHz, 0xB - 40MHz, 0xC - 52MHz, 0xD - 100MHz, 0xE - 108MHz, 0xF - 125MHz, 0x10 - 135
		Hz, 0x11 - 150 MHz. 0x12 to 0x1F - Reserved
		PSU_SERDES_PLL_REF_SEL1_PLLREFSEL1                                              0x9

		PLL1 Reference Selection Register
		(OFFSET, MASK, VALUE)      (0XFD410004, 0x0000001FU ,0x00000009U)
		RegMask = (SERDES_PLL_REF_SEL1_PLLREFSEL1_MASK |  0 );

		RegVal = ((0x00000009U << SERDES_PLL_REF_SEL1_PLLREFSEL1_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_PLL_REF_SEL1_OFFSET ,0x0000001FU ,0x00000009U);
	/*############################################################################################################################ */

		/*Register : PLL_REF_SEL2 @ 0XFD410008</p>

		PLL2 Reference Selection. 0x0 - 5MHz, 0x1 - 9.6MHz, 0x2 - 10MHz, 0x3 - 12MHz, 0x4 - 13MHz, 0x5 - 19.2MHz, 0x6 - 20MHz, 0x7 -
		4MHz, 0x8 - 26MHz, 0x9 - 27MHz, 0xA - 38.4MHz, 0xB - 40MHz, 0xC - 52MHz, 0xD - 100MHz, 0xE - 108MHz, 0xF - 125MHz, 0x10 - 135
		Hz, 0x11 - 150 MHz. 0x12 to 0x1F - Reserved
		PSU_SERDES_PLL_REF_SEL2_PLLREFSEL2                                              0x8

		PLL2 Reference Selection Register
		(OFFSET, MASK, VALUE)      (0XFD410008, 0x0000001FU ,0x00000008U)
		RegMask = (SERDES_PLL_REF_SEL2_PLLREFSEL2_MASK |  0 );

		RegVal = ((0x00000008U << SERDES_PLL_REF_SEL2_PLLREFSEL2_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_PLL_REF_SEL2_OFFSET ,0x0000001FU ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : PLL_REF_SEL3 @ 0XFD41000C</p>

		PLL3 Reference Selection. 0x0 - 5MHz, 0x1 - 9.6MHz, 0x2 - 10MHz, 0x3 - 12MHz, 0x4 - 13MHz, 0x5 - 19.2MHz, 0x6 - 20MHz, 0x7 -
		4MHz, 0x8 - 26MHz, 0x9 - 27MHz, 0xA - 38.4MHz, 0xB - 40MHz, 0xC - 52MHz, 0xD - 100MHz, 0xE - 108MHz, 0xF - 125MHz, 0x10 - 135
		Hz, 0x11 - 150 MHz. 0x12 to 0x1F - Reserved
		PSU_SERDES_PLL_REF_SEL3_PLLREFSEL3                                              0xF

		PLL3 Reference Selection Register
		(OFFSET, MASK, VALUE)      (0XFD41000C, 0x0000001FU ,0x0000000FU)
		RegMask = (SERDES_PLL_REF_SEL3_PLLREFSEL3_MASK |  0 );

		RegVal = ((0x0000000FU << SERDES_PLL_REF_SEL3_PLLREFSEL3_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_PLL_REF_SEL3_OFFSET ,0x0000001FU ,0x0000000FU);
	/*############################################################################################################################ */

		// : GT REFERENCE CLOCK FREQUENCY SELECTION
		/*Register : L0_L0_REF_CLK_SEL @ 0XFD402860</p>

		Sel of lane 0 ref clock local mux. Set to 1 to select lane 0 slicer output. Set to 0 to select lane0 ref clock mux output.
		PSU_SERDES_L0_L0_REF_CLK_SEL_L0_REF_CLK_LCL_SEL                                 0x1

		Lane0 Ref Clock Selection Register
		(OFFSET, MASK, VALUE)      (0XFD402860, 0x00000080U ,0x00000080U)
		RegMask = (SERDES_L0_L0_REF_CLK_SEL_L0_REF_CLK_LCL_SEL_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L0_L0_REF_CLK_SEL_L0_REF_CLK_LCL_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_L0_REF_CLK_SEL_OFFSET ,0x00000080U ,0x00000080U);
	/*############################################################################################################################ */

		/*Register : L0_L1_REF_CLK_SEL @ 0XFD402864</p>

		Sel of lane 1 ref clock local mux. Set to 1 to select lane 1 slicer output. Set to 0 to select lane1 ref clock mux output.
		PSU_SERDES_L0_L1_REF_CLK_SEL_L1_REF_CLK_LCL_SEL                                 0x0

		Bit 3 of lane 1 ref clock mux one hot sel. Set to 1 to select lane 3 slicer output from ref clock network
		PSU_SERDES_L0_L1_REF_CLK_SEL_L1_REF_CLK_SEL_3                                   0x1

		Lane1 Ref Clock Selection Register
		(OFFSET, MASK, VALUE)      (0XFD402864, 0x00000088U ,0x00000008U)
		RegMask = (SERDES_L0_L1_REF_CLK_SEL_L1_REF_CLK_LCL_SEL_MASK | SERDES_L0_L1_REF_CLK_SEL_L1_REF_CLK_SEL_3_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L0_L1_REF_CLK_SEL_L1_REF_CLK_LCL_SEL_SHIFT
			| 0x00000001U << SERDES_L0_L1_REF_CLK_SEL_L1_REF_CLK_SEL_3_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_L1_REF_CLK_SEL_OFFSET ,0x00000088U ,0x00000008U);
	/*############################################################################################################################ */

		/*Register : L0_L2_REF_CLK_SEL @ 0XFD402868</p>

		Sel of lane 2 ref clock local mux. Set to 1 to select lane 1 slicer output. Set to 0 to select lane2 ref clock mux output.
		PSU_SERDES_L0_L2_REF_CLK_SEL_L2_REF_CLK_LCL_SEL                                 0x1

		Lane2 Ref Clock Selection Register
		(OFFSET, MASK, VALUE)      (0XFD402868, 0x00000080U ,0x00000080U)
		RegMask = (SERDES_L0_L2_REF_CLK_SEL_L2_REF_CLK_LCL_SEL_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L0_L2_REF_CLK_SEL_L2_REF_CLK_LCL_SEL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_L2_REF_CLK_SEL_OFFSET ,0x00000080U ,0x00000080U);
	/*############################################################################################################################ */

		/*Register : L0_L3_REF_CLK_SEL @ 0XFD40286C</p>

		Sel of lane 3 ref clock local mux. Set to 1 to select lane 3 slicer output. Set to 0 to select lane3 ref clock mux output.
		PSU_SERDES_L0_L3_REF_CLK_SEL_L3_REF_CLK_LCL_SEL                                 0x0

		Bit 1 of lane 3 ref clock mux one hot sel. Set to 1 to select lane 1 slicer output from ref clock network
		PSU_SERDES_L0_L3_REF_CLK_SEL_L3_REF_CLK_SEL_1                                   0x1

		Lane3 Ref Clock Selection Register
		(OFFSET, MASK, VALUE)      (0XFD40286C, 0x00000082U ,0x00000002U)
		RegMask = (SERDES_L0_L3_REF_CLK_SEL_L3_REF_CLK_LCL_SEL_MASK | SERDES_L0_L3_REF_CLK_SEL_L3_REF_CLK_SEL_1_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L0_L3_REF_CLK_SEL_L3_REF_CLK_LCL_SEL_SHIFT
			| 0x00000001U << SERDES_L0_L3_REF_CLK_SEL_L3_REF_CLK_SEL_1_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_L3_REF_CLK_SEL_OFFSET ,0x00000082U ,0x00000002U);
	/*############################################################################################################################ */

		// : ENABLE SPREAD SPECTRUM
		/*Register : L2_TM_PLL_DIG_37 @ 0XFD40A094</p>

		Enable/Disable coarse code satureation limiting logic
		PSU_SERDES_L2_TM_PLL_DIG_37_TM_ENABLE_COARSE_SATURATION                         0x1

		Test mode register 37
		(OFFSET, MASK, VALUE)      (0XFD40A094, 0x00000010U ,0x00000010U)
		RegMask = (SERDES_L2_TM_PLL_DIG_37_TM_ENABLE_COARSE_SATURATION_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L2_TM_PLL_DIG_37_TM_ENABLE_COARSE_SATURATION_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L2_TM_PLL_DIG_37_OFFSET ,0x00000010U ,0x00000010U);
	/*############################################################################################################################ */

		/*Register : L3_TM_PLL_DIG_37 @ 0XFD40E094</p>

		Enable/Disable coarse code satureation limiting logic
		PSU_SERDES_L3_TM_PLL_DIG_37_TM_ENABLE_COARSE_SATURATION                         0x1

		Test mode register 37
		(OFFSET, MASK, VALUE)      (0XFD40E094, 0x00000010U ,0x00000010U)
		RegMask = (SERDES_L3_TM_PLL_DIG_37_TM_ENABLE_COARSE_SATURATION_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L3_TM_PLL_DIG_37_TM_ENABLE_COARSE_SATURATION_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L3_TM_PLL_DIG_37_OFFSET ,0x00000010U ,0x00000010U);
	/*############################################################################################################################ */

		/*Register : L2_PLL_SS_STEPS_0_LSB @ 0XFD40A368</p>

		Spread Spectrum No of Steps [7:0]
		PSU_SERDES_L2_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB                          0x38

		Spread Spectrum No of Steps bits 7:0
		(OFFSET, MASK, VALUE)      (0XFD40A368, 0x000000FFU ,0x00000038U)
		RegMask = (SERDES_L2_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB_MASK |  0 );

		RegVal = ((0x00000038U << SERDES_L2_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L2_PLL_SS_STEPS_0_LSB_OFFSET ,0x000000FFU ,0x00000038U);
	/*############################################################################################################################ */

		/*Register : L2_PLL_SS_STEPS_1_MSB @ 0XFD40A36C</p>

		Spread Spectrum No of Steps [10:8]
		PSU_SERDES_L2_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB                          0x03

		Spread Spectrum No of Steps bits 10:8
		(OFFSET, MASK, VALUE)      (0XFD40A36C, 0x00000007U ,0x00000003U)
		RegMask = (SERDES_L2_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB_MASK |  0 );

		RegVal = ((0x00000003U << SERDES_L2_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L2_PLL_SS_STEPS_1_MSB_OFFSET ,0x00000007U ,0x00000003U);
	/*############################################################################################################################ */

		/*Register : L3_PLL_SS_STEPS_0_LSB @ 0XFD40E368</p>

		Spread Spectrum No of Steps [7:0]
		PSU_SERDES_L3_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB                          0x0

		Spread Spectrum No of Steps bits 7:0
		(OFFSET, MASK, VALUE)      (0XFD40E368, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L3_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L3_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L3_PLL_SS_STEPS_0_LSB_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L3_PLL_SS_STEPS_1_MSB @ 0XFD40E36C</p>

		Spread Spectrum No of Steps [10:8]
		PSU_SERDES_L3_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB                          0x0

		Spread Spectrum No of Steps bits 10:8
		(OFFSET, MASK, VALUE)      (0XFD40E36C, 0x00000007U ,0x00000000U)
		RegMask = (SERDES_L3_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L3_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L3_PLL_SS_STEPS_1_MSB_OFFSET ,0x00000007U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L0_PLL_SS_STEPS_0_LSB @ 0XFD402368</p>

		Spread Spectrum No of Steps [7:0]
		PSU_SERDES_L0_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB                          0x0

		Spread Spectrum No of Steps bits 7:0
		(OFFSET, MASK, VALUE)      (0XFD402368, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L0_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L0_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_PLL_SS_STEPS_0_LSB_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L0_PLL_SS_STEPS_1_MSB @ 0XFD40236C</p>

		Spread Spectrum No of Steps [10:8]
		PSU_SERDES_L0_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB                          0x0

		Spread Spectrum No of Steps bits 10:8
		(OFFSET, MASK, VALUE)      (0XFD40236C, 0x00000007U ,0x00000000U)
		RegMask = (SERDES_L0_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L0_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_PLL_SS_STEPS_1_MSB_OFFSET ,0x00000007U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L1_PLL_SS_STEPS_0_LSB @ 0XFD406368</p>

		Spread Spectrum No of Steps [7:0]
		PSU_SERDES_L1_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB                          0x0

		Spread Spectrum No of Steps bits 7:0
		(OFFSET, MASK, VALUE)      (0XFD406368, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L1_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L1_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L1_PLL_SS_STEPS_0_LSB_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L1_PLL_SS_STEPS_1_MSB @ 0XFD40636C</p>

		Spread Spectrum No of Steps [10:8]
		PSU_SERDES_L1_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB                          0x0

		Spread Spectrum No of Steps bits 10:8
		(OFFSET, MASK, VALUE)      (0XFD40636C, 0x00000007U ,0x00000000U)
		RegMask = (SERDES_L1_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L1_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L1_PLL_SS_STEPS_1_MSB_OFFSET ,0x00000007U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L0_PLL_SS_STEP_SIZE_0_LSB @ 0XFD402370</p>

		Step Size for Spread Spectrum [7:0]
		PSU_SERDES_L0_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB                         0x0

		Step Size for Spread Spectrum LSB
		(OFFSET, MASK, VALUE)      (0XFD402370, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L0_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L0_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_PLL_SS_STEP_SIZE_0_LSB_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L0_PLL_SS_STEP_SIZE_1 @ 0XFD402374</p>

		Step Size for Spread Spectrum [15:8]
		PSU_SERDES_L0_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1                                 0x0

		Step Size for Spread Spectrum 1
		(OFFSET, MASK, VALUE)      (0XFD402374, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L0_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L0_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_PLL_SS_STEP_SIZE_1_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L0_PLL_SS_STEP_SIZE_2 @ 0XFD402378</p>

		Step Size for Spread Spectrum [23:16]
		PSU_SERDES_L0_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2                                 0x0

		Step Size for Spread Spectrum 2
		(OFFSET, MASK, VALUE)      (0XFD402378, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L0_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L0_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_PLL_SS_STEP_SIZE_2_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L0_PLL_SS_STEP_SIZE_3_MSB @ 0XFD40237C</p>

		Step Size for Spread Spectrum [25:24]
		PSU_SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB                         0x0

		Enable/Disable test mode force on SS step size
		PSU_SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE                         0x1

		Enable/Disable test mode force on SS no of steps
		PSU_SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS                      0x1

		Enable force on enable Spread Spectrum
		(OFFSET, MASK, VALUE)      (0XFD40237C, 0x00000033U ,0x00000030U)
		RegMask = (SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB_MASK | SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE_MASK | SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB_SHIFT
			| 0x00000001U << SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE_SHIFT
			| 0x00000001U << SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_OFFSET ,0x00000033U ,0x00000030U);
	/*############################################################################################################################ */

		/*Register : L1_PLL_SS_STEP_SIZE_0_LSB @ 0XFD406370</p>

		Step Size for Spread Spectrum [7:0]
		PSU_SERDES_L1_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB                         0x0

		Step Size for Spread Spectrum LSB
		(OFFSET, MASK, VALUE)      (0XFD406370, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L1_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L1_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L1_PLL_SS_STEP_SIZE_0_LSB_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L1_PLL_SS_STEP_SIZE_1 @ 0XFD406374</p>

		Step Size for Spread Spectrum [15:8]
		PSU_SERDES_L1_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1                                 0x0

		Step Size for Spread Spectrum 1
		(OFFSET, MASK, VALUE)      (0XFD406374, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L1_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L1_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L1_PLL_SS_STEP_SIZE_1_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L1_PLL_SS_STEP_SIZE_2 @ 0XFD406378</p>

		Step Size for Spread Spectrum [23:16]
		PSU_SERDES_L1_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2                                 0x0

		Step Size for Spread Spectrum 2
		(OFFSET, MASK, VALUE)      (0XFD406378, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L1_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L1_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L1_PLL_SS_STEP_SIZE_2_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L1_PLL_SS_STEP_SIZE_3_MSB @ 0XFD40637C</p>

		Step Size for Spread Spectrum [25:24]
		PSU_SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB                         0x0

		Enable/Disable test mode force on SS step size
		PSU_SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE                         0x1

		Enable/Disable test mode force on SS no of steps
		PSU_SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS                      0x1

		Enable force on enable Spread Spectrum
		(OFFSET, MASK, VALUE)      (0XFD40637C, 0x00000033U ,0x00000030U)
		RegMask = (SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB_MASK | SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE_MASK | SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB_SHIFT
			| 0x00000001U << SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE_SHIFT
			| 0x00000001U << SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_OFFSET ,0x00000033U ,0x00000030U);
	/*############################################################################################################################ */

		/*Register : L2_PLL_SS_STEP_SIZE_0_LSB @ 0XFD40A370</p>

		Step Size for Spread Spectrum [7:0]
		PSU_SERDES_L2_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB                         0xF4

		Step Size for Spread Spectrum LSB
		(OFFSET, MASK, VALUE)      (0XFD40A370, 0x000000FFU ,0x000000F4U)
		RegMask = (SERDES_L2_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB_MASK |  0 );

		RegVal = ((0x000000F4U << SERDES_L2_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L2_PLL_SS_STEP_SIZE_0_LSB_OFFSET ,0x000000FFU ,0x000000F4U);
	/*############################################################################################################################ */

		/*Register : L2_PLL_SS_STEP_SIZE_1 @ 0XFD40A374</p>

		Step Size for Spread Spectrum [15:8]
		PSU_SERDES_L2_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1                                 0x31

		Step Size for Spread Spectrum 1
		(OFFSET, MASK, VALUE)      (0XFD40A374, 0x000000FFU ,0x00000031U)
		RegMask = (SERDES_L2_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1_MASK |  0 );

		RegVal = ((0x00000031U << SERDES_L2_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L2_PLL_SS_STEP_SIZE_1_OFFSET ,0x000000FFU ,0x00000031U);
	/*############################################################################################################################ */

		/*Register : L2_PLL_SS_STEP_SIZE_2 @ 0XFD40A378</p>

		Step Size for Spread Spectrum [23:16]
		PSU_SERDES_L2_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2                                 0x2

		Step Size for Spread Spectrum 2
		(OFFSET, MASK, VALUE)      (0XFD40A378, 0x000000FFU ,0x00000002U)
		RegMask = (SERDES_L2_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2_MASK |  0 );

		RegVal = ((0x00000002U << SERDES_L2_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L2_PLL_SS_STEP_SIZE_2_OFFSET ,0x000000FFU ,0x00000002U);
	/*############################################################################################################################ */

		/*Register : L2_PLL_SS_STEP_SIZE_3_MSB @ 0XFD40A37C</p>

		Step Size for Spread Spectrum [25:24]
		PSU_SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB                         0x0

		Enable/Disable test mode force on SS step size
		PSU_SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE                         0x1

		Enable/Disable test mode force on SS no of steps
		PSU_SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS                      0x1

		Enable force on enable Spread Spectrum
		(OFFSET, MASK, VALUE)      (0XFD40A37C, 0x00000033U ,0x00000030U)
		RegMask = (SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB_MASK | SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE_MASK | SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB_SHIFT
			| 0x00000001U << SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE_SHIFT
			| 0x00000001U << SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_OFFSET ,0x00000033U ,0x00000030U);
	/*############################################################################################################################ */

		/*Register : L3_PLL_SS_STEP_SIZE_0_LSB @ 0XFD40E370</p>

		Step Size for Spread Spectrum [7:0]
		PSU_SERDES_L3_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB                         0x0

		Step Size for Spread Spectrum LSB
		(OFFSET, MASK, VALUE)      (0XFD40E370, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L3_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L3_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L3_PLL_SS_STEP_SIZE_0_LSB_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L3_PLL_SS_STEP_SIZE_1 @ 0XFD40E374</p>

		Step Size for Spread Spectrum [15:8]
		PSU_SERDES_L3_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1                                 0x0

		Step Size for Spread Spectrum 1
		(OFFSET, MASK, VALUE)      (0XFD40E374, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L3_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L3_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L3_PLL_SS_STEP_SIZE_1_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L3_PLL_SS_STEP_SIZE_2 @ 0XFD40E378</p>

		Step Size for Spread Spectrum [23:16]
		PSU_SERDES_L3_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2                                 0x0

		Step Size for Spread Spectrum 2
		(OFFSET, MASK, VALUE)      (0XFD40E378, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L3_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L3_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L3_PLL_SS_STEP_SIZE_2_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L3_PLL_SS_STEP_SIZE_3_MSB @ 0XFD40E37C</p>

		Step Size for Spread Spectrum [25:24]
		PSU_SERDES_L3_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB                         0x0

		Enable/Disable test mode force on SS step size
		PSU_SERDES_L3_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE                         0x1

		Enable/Disable test mode force on SS no of steps
		PSU_SERDES_L3_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS                      0x1

		Enable force on enable Spread Spectrum
		(OFFSET, MASK, VALUE)      (0XFD40E37C, 0x00000033U ,0x00000030U)
		RegMask = (SERDES_L3_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB_MASK | SERDES_L3_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE_MASK | SERDES_L3_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L3_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB_SHIFT
			| 0x00000001U << SERDES_L3_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE_SHIFT
			| 0x00000001U << SERDES_L3_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L3_PLL_SS_STEP_SIZE_3_MSB_OFFSET ,0x00000033U ,0x00000030U);
	/*############################################################################################################################ */

		/*Register : L2_TM_DIG_6 @ 0XFD40906C</p>

		Bypass Descrambler
		PSU_SERDES_L2_TM_DIG_6_BYPASS_DESCRAM                                           0x1

		Enable Bypass for <1> TM_DIG_CTRL_6
		PSU_SERDES_L2_TM_DIG_6_FORCE_BYPASS_DESCRAM                                     0x1

		Data path test modes in decoder and descram
		(OFFSET, MASK, VALUE)      (0XFD40906C, 0x00000003U ,0x00000003U)
		RegMask = (SERDES_L2_TM_DIG_6_BYPASS_DESCRAM_MASK | SERDES_L2_TM_DIG_6_FORCE_BYPASS_DESCRAM_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L2_TM_DIG_6_BYPASS_DESCRAM_SHIFT
			| 0x00000001U << SERDES_L2_TM_DIG_6_FORCE_BYPASS_DESCRAM_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L2_TM_DIG_6_OFFSET ,0x00000003U ,0x00000003U);
	/*############################################################################################################################ */

		/*Register : L2_TX_DIG_TM_61 @ 0XFD4080F4</p>

		Bypass scrambler signal
		PSU_SERDES_L2_TX_DIG_TM_61_BYPASS_SCRAM                                         0x1

		Enable/disable scrambler bypass signal
		PSU_SERDES_L2_TX_DIG_TM_61_FORCE_BYPASS_SCRAM                                   0x1

		MPHY PLL Gear and bypass scrambler
		(OFFSET, MASK, VALUE)      (0XFD4080F4, 0x00000003U ,0x00000003U)
		RegMask = (SERDES_L2_TX_DIG_TM_61_BYPASS_SCRAM_MASK | SERDES_L2_TX_DIG_TM_61_FORCE_BYPASS_SCRAM_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L2_TX_DIG_TM_61_BYPASS_SCRAM_SHIFT
			| 0x00000001U << SERDES_L2_TX_DIG_TM_61_FORCE_BYPASS_SCRAM_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L2_TX_DIG_TM_61_OFFSET ,0x00000003U ,0x00000003U);
	/*############################################################################################################################ */

		/*Register : L3_TM_DIG_6 @ 0XFD40D06C</p>

		Bypass Descrambler
		PSU_SERDES_L3_TM_DIG_6_BYPASS_DESCRAM                                           0x1

		Enable Bypass for <1> TM_DIG_CTRL_6
		PSU_SERDES_L3_TM_DIG_6_FORCE_BYPASS_DESCRAM                                     0x1

		Data path test modes in decoder and descram
		(OFFSET, MASK, VALUE)      (0XFD40D06C, 0x00000003U ,0x00000003U)
		RegMask = (SERDES_L3_TM_DIG_6_BYPASS_DESCRAM_MASK | SERDES_L3_TM_DIG_6_FORCE_BYPASS_DESCRAM_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L3_TM_DIG_6_BYPASS_DESCRAM_SHIFT
			| 0x00000001U << SERDES_L3_TM_DIG_6_FORCE_BYPASS_DESCRAM_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L3_TM_DIG_6_OFFSET ,0x00000003U ,0x00000003U);
	/*############################################################################################################################ */

		/*Register : L3_TX_DIG_TM_61 @ 0XFD40C0F4</p>

		Bypass scrambler signal
		PSU_SERDES_L3_TX_DIG_TM_61_BYPASS_SCRAM                                         0x1

		Enable/disable scrambler bypass signal
		PSU_SERDES_L3_TX_DIG_TM_61_FORCE_BYPASS_SCRAM                                   0x1

		MPHY PLL Gear and bypass scrambler
		(OFFSET, MASK, VALUE)      (0XFD40C0F4, 0x00000003U ,0x00000003U)
		RegMask = (SERDES_L3_TX_DIG_TM_61_BYPASS_SCRAM_MASK | SERDES_L3_TX_DIG_TM_61_FORCE_BYPASS_SCRAM_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L3_TX_DIG_TM_61_BYPASS_SCRAM_SHIFT
			| 0x00000001U << SERDES_L3_TX_DIG_TM_61_FORCE_BYPASS_SCRAM_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L3_TX_DIG_TM_61_OFFSET ,0x00000003U ,0x00000003U);
	/*############################################################################################################################ */

		// : GT LANE SETTINGS
		/*Register : ICM_CFG0 @ 0XFD410010</p>

		Controls UPHY Lane 0 protocol configuration. 0 - PowerDown, 1 - PCIe .0, 2 - Sata0, 3 - USB0, 4 - DP.1, 5 - SGMII0, 6 - Unuse
		, 7 - Unused
		PSU_SERDES_ICM_CFG0_L0_ICM_CFG                                                  4

		Controls UPHY Lane 1 protocol configuration. 0 - PowerDown, 1 - PCIe.1, 2 - Sata1, 3 - USB0, 4 - DP.0, 5 - SGMII1, 6 - Unused
		 7 - Unused
		PSU_SERDES_ICM_CFG0_L1_ICM_CFG                                                  4

		ICM Configuration Register 0
		(OFFSET, MASK, VALUE)      (0XFD410010, 0x00000077U ,0x00000044U)
		RegMask = (SERDES_ICM_CFG0_L0_ICM_CFG_MASK | SERDES_ICM_CFG0_L1_ICM_CFG_MASK |  0 );

		RegVal = ((0x00000004U << SERDES_ICM_CFG0_L0_ICM_CFG_SHIFT
			| 0x00000004U << SERDES_ICM_CFG0_L1_ICM_CFG_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_ICM_CFG0_OFFSET ,0x00000077U ,0x00000044U);
	/*############################################################################################################################ */

		/*Register : ICM_CFG1 @ 0XFD410014</p>

		Controls UPHY Lane 2 protocol configuration. 0 - PowerDown, 1 - PCIe.1, 2 - Sata0, 3 - USB0, 4 - DP.1, 5 - SGMII2, 6 - Unused
		 7 - Unused
		PSU_SERDES_ICM_CFG1_L2_ICM_CFG                                                  3

		Controls UPHY Lane 3 protocol configuration. 0 - PowerDown, 1 - PCIe.3, 2 - Sata1, 3 - USB1, 4 - DP.0, 5 - SGMII3, 6 - Unused
		 7 - Unused
		PSU_SERDES_ICM_CFG1_L3_ICM_CFG                                                  3

		ICM Configuration Register 1
		(OFFSET, MASK, VALUE)      (0XFD410014, 0x00000077U ,0x00000033U)
		RegMask = (SERDES_ICM_CFG1_L2_ICM_CFG_MASK | SERDES_ICM_CFG1_L3_ICM_CFG_MASK |  0 );

		RegVal = ((0x00000003U << SERDES_ICM_CFG1_L2_ICM_CFG_SHIFT
			| 0x00000003U << SERDES_ICM_CFG1_L3_ICM_CFG_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_ICM_CFG1_OFFSET ,0x00000077U ,0x00000033U);
	/*############################################################################################################################ */

		// : CHECKING PLL LOCK
		// : ENABLE SERIAL DATA MUX DEEMPH
		/*Register : L0_TXPMD_TM_45 @ 0XFD400CB4</p>

		Enable/disable DP post2 path
		PSU_SERDES_L0_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_POST2_PATH                         0x1

		Override enable/disable of DP post2 path
		PSU_SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST2_PATH                    0x1

		Override enable/disable of DP post1 path
		PSU_SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST1_PATH                    0x1

		Enable/disable DP main path
		PSU_SERDES_L0_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_MAIN_PATH                          0x1

		Override enable/disable of DP main path
		PSU_SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_MAIN_PATH                     0x1

		Post or pre or main DP path selection
		(OFFSET, MASK, VALUE)      (0XFD400CB4, 0x00000037U ,0x00000037U)
		RegMask = (SERDES_L0_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_POST2_PATH_MASK | SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST2_PATH_MASK | SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST1_PATH_MASK | SERDES_L0_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_MAIN_PATH_MASK | SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_MAIN_PATH_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L0_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_POST2_PATH_SHIFT
			| 0x00000001U << SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST2_PATH_SHIFT
			| 0x00000001U << SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST1_PATH_SHIFT
			| 0x00000001U << SERDES_L0_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_MAIN_PATH_SHIFT
			| 0x00000001U << SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_MAIN_PATH_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_TXPMD_TM_45_OFFSET ,0x00000037U ,0x00000037U);
	/*############################################################################################################################ */

		/*Register : L1_TXPMD_TM_45 @ 0XFD404CB4</p>

		Enable/disable DP post2 path
		PSU_SERDES_L1_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_POST2_PATH                         0x1

		Override enable/disable of DP post2 path
		PSU_SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST2_PATH                    0x1

		Override enable/disable of DP post1 path
		PSU_SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST1_PATH                    0x1

		Enable/disable DP main path
		PSU_SERDES_L1_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_MAIN_PATH                          0x1

		Override enable/disable of DP main path
		PSU_SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_MAIN_PATH                     0x1

		Post or pre or main DP path selection
		(OFFSET, MASK, VALUE)      (0XFD404CB4, 0x00000037U ,0x00000037U)
		RegMask = (SERDES_L1_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_POST2_PATH_MASK | SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST2_PATH_MASK | SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST1_PATH_MASK | SERDES_L1_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_MAIN_PATH_MASK | SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_MAIN_PATH_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L1_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_POST2_PATH_SHIFT
			| 0x00000001U << SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST2_PATH_SHIFT
			| 0x00000001U << SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST1_PATH_SHIFT
			| 0x00000001U << SERDES_L1_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_MAIN_PATH_SHIFT
			| 0x00000001U << SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_MAIN_PATH_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L1_TXPMD_TM_45_OFFSET ,0x00000037U ,0x00000037U);
	/*############################################################################################################################ */

		/*Register : L0_TX_ANA_TM_118 @ 0XFD4001D8</p>

		Test register force for enabling/disablign TX deemphasis bits <17:0>
		PSU_SERDES_L0_TX_ANA_TM_118_FORCE_TX_DEEMPH_17_0                                0x1

		Enable Override of TX deemphasis
		(OFFSET, MASK, VALUE)      (0XFD4001D8, 0x00000001U ,0x00000001U)
		RegMask = (SERDES_L0_TX_ANA_TM_118_FORCE_TX_DEEMPH_17_0_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L0_TX_ANA_TM_118_FORCE_TX_DEEMPH_17_0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_TX_ANA_TM_118_OFFSET ,0x00000001U ,0x00000001U);
	/*############################################################################################################################ */

		/*Register : L1_TX_ANA_TM_118 @ 0XFD4041D8</p>

		Test register force for enabling/disablign TX deemphasis bits <17:0>
		PSU_SERDES_L1_TX_ANA_TM_118_FORCE_TX_DEEMPH_17_0                                0x1

		Enable Override of TX deemphasis
		(OFFSET, MASK, VALUE)      (0XFD4041D8, 0x00000001U ,0x00000001U)
		RegMask = (SERDES_L1_TX_ANA_TM_118_FORCE_TX_DEEMPH_17_0_MASK |  0 );

		RegVal = ((0x00000001U << SERDES_L1_TX_ANA_TM_118_FORCE_TX_DEEMPH_17_0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L1_TX_ANA_TM_118_OFFSET ,0x00000001U ,0x00000001U);
	/*############################################################################################################################ */

		// : ENABLE PRE EMPHAIS AND VOLTAGE SWING
		/*Register : L1_TXPMD_TM_48 @ 0XFD404CC0</p>

		Margining factor value
		PSU_SERDES_L1_TXPMD_TM_48_TM_RESULTANT_MARGINING_FACTOR                         0

		Margining factor
		(OFFSET, MASK, VALUE)      (0XFD404CC0, 0x0000001FU ,0x00000000U)
		RegMask = (SERDES_L1_TXPMD_TM_48_TM_RESULTANT_MARGINING_FACTOR_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L1_TXPMD_TM_48_TM_RESULTANT_MARGINING_FACTOR_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L1_TXPMD_TM_48_OFFSET ,0x0000001FU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L0_TXPMD_TM_48 @ 0XFD400CC0</p>

		Margining factor value
		PSU_SERDES_L0_TXPMD_TM_48_TM_RESULTANT_MARGINING_FACTOR                         0

		Margining factor
		(OFFSET, MASK, VALUE)      (0XFD400CC0, 0x0000001FU ,0x00000000U)
		RegMask = (SERDES_L0_TXPMD_TM_48_TM_RESULTANT_MARGINING_FACTOR_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L0_TXPMD_TM_48_TM_RESULTANT_MARGINING_FACTOR_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_TXPMD_TM_48_OFFSET ,0x0000001FU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L1_TX_ANA_TM_18 @ 0XFD404048</p>

		pipe_TX_Deemph. 0: -6dB de-emphasis, 1: -3.5dB de-emphasis, 2 : No de-emphasis, Others: reserved
		PSU_SERDES_L1_TX_ANA_TM_18_PIPE_TX_DEEMPH_7_0                                   0

		Override for PIPE TX de-emphasis
		(OFFSET, MASK, VALUE)      (0XFD404048, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L1_TX_ANA_TM_18_PIPE_TX_DEEMPH_7_0_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L1_TX_ANA_TM_18_PIPE_TX_DEEMPH_7_0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L1_TX_ANA_TM_18_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : L0_TX_ANA_TM_18 @ 0XFD400048</p>

		pipe_TX_Deemph. 0: -6dB de-emphasis, 1: -3.5dB de-emphasis, 2 : No de-emphasis, Others: reserved
		PSU_SERDES_L0_TX_ANA_TM_18_PIPE_TX_DEEMPH_7_0                                   0

		Override for PIPE TX de-emphasis
		(OFFSET, MASK, VALUE)      (0XFD400048, 0x000000FFU ,0x00000000U)
		RegMask = (SERDES_L0_TX_ANA_TM_18_PIPE_TX_DEEMPH_7_0_MASK |  0 );

		RegVal = ((0x00000000U << SERDES_L0_TX_ANA_TM_18_PIPE_TX_DEEMPH_7_0_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (SERDES_L0_TX_ANA_TM_18_OFFSET ,0x000000FFU ,0x00000000U);
	/*############################################################################################################################ */


  return 1;
}
unsigned long psu_resetout_init_data() {
		// : TAKING SERDES PERIPHERAL OUT OF RESET RESET
		// : PUTTING USB0 IN RESET
		/*Register : RST_LPD_TOP @ 0XFF5E023C</p>

		USB 0 reset for control registers
		PSU_CRL_APB_RST_LPD_TOP_USB0_APB_RESET                                          0X0

		Software control register for the LPD block.
		(OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00000400U ,0x00000000U)
		RegMask = (CRL_APB_RST_LPD_TOP_USB0_APB_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RST_LPD_TOP_USB0_APB_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_TOP_OFFSET ,0x00000400U ,0x00000000U);
	/*############################################################################################################################ */

		// : USB0 PIPE POWER PRESENT
		/*Register : fpd_power_prsnt @ 0XFF9D0080</p>

		This bit is used to choose between PIPE power present and 1'b1
		PSU_USB3_0_FPD_POWER_PRSNT_OPTION                                               0X1

		fpd_power_prsnt
		(OFFSET, MASK, VALUE)      (0XFF9D0080, 0x00000001U ,0x00000001U)
		RegMask = (USB3_0_FPD_POWER_PRSNT_OPTION_MASK |  0 );

		RegVal = ((0x00000001U << USB3_0_FPD_POWER_PRSNT_OPTION_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (USB3_0_FPD_POWER_PRSNT_OFFSET ,0x00000001U ,0x00000001U);
	/*############################################################################################################################ */

		// :
		/*Register : RST_LPD_TOP @ 0XFF5E023C</p>

		USB 0 sleep circuit reset
		PSU_CRL_APB_RST_LPD_TOP_USB0_HIBERRESET                                         0X0

		USB 0 reset
		PSU_CRL_APB_RST_LPD_TOP_USB0_CORERESET                                          0X0

		Software control register for the LPD block.
		(OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00000140U ,0x00000000U)
		RegMask = (CRL_APB_RST_LPD_TOP_USB0_HIBERRESET_MASK | CRL_APB_RST_LPD_TOP_USB0_CORERESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RST_LPD_TOP_USB0_HIBERRESET_SHIFT
			| 0x00000000U << CRL_APB_RST_LPD_TOP_USB0_CORERESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_TOP_OFFSET ,0x00000140U ,0x00000000U);
	/*############################################################################################################################ */

		// : PUTTING USB1 IN RESET
		/*Register : RST_LPD_TOP @ 0XFF5E023C</p>

		USB 1 reset for control registers
		PSU_CRL_APB_RST_LPD_TOP_USB1_APB_RESET                                          0X0

		Software control register for the LPD block.
		(OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00000800U ,0x00000000U)
		RegMask = (CRL_APB_RST_LPD_TOP_USB1_APB_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RST_LPD_TOP_USB1_APB_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_TOP_OFFSET ,0x00000800U ,0x00000000U);
	/*############################################################################################################################ */

		// : USB1 PIPE POWER PRESENT
		/*Register : fpd_power_prsnt @ 0XFF9E0080</p>

		This bit is used to choose between PIPE power present and 1'b1
		PSU_USB3_1_FPD_POWER_PRSNT_OPTION                                               0X1

		fpd_power_prsnt
		(OFFSET, MASK, VALUE)      (0XFF9E0080, 0x00000001U ,0x00000001U)
		RegMask = (USB3_1_FPD_POWER_PRSNT_OPTION_MASK |  0 );

		RegVal = ((0x00000001U << USB3_1_FPD_POWER_PRSNT_OPTION_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (USB3_1_FPD_POWER_PRSNT_OFFSET ,0x00000001U ,0x00000001U);
	/*############################################################################################################################ */

		// :
		/*Register : RST_LPD_TOP @ 0XFF5E023C</p>

		USB 1 sleep circuit reset
		PSU_CRL_APB_RST_LPD_TOP_USB1_HIBERRESET                                         0X0

		USB 1 reset
		PSU_CRL_APB_RST_LPD_TOP_USB1_CORERESET                                          0X0

		Software control register for the LPD block.
		(OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00000280U ,0x00000000U)
		RegMask = (CRL_APB_RST_LPD_TOP_USB1_HIBERRESET_MASK | CRL_APB_RST_LPD_TOP_USB1_CORERESET_MASK |  0 );

		RegVal = ((0x00000000U << CRL_APB_RST_LPD_TOP_USB1_HIBERRESET_SHIFT
			| 0x00000000U << CRL_APB_RST_LPD_TOP_USB1_CORERESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_TOP_OFFSET ,0x00000280U ,0x00000000U);
	/*############################################################################################################################ */

		// : PUTTING DP IN RESET
		/*Register : RST_FPD_TOP @ 0XFD1A0100</p>

		Display Port block level reset (includes DPDMA)
		PSU_CRF_APB_RST_FPD_TOP_DP_RESET                                                0X0

		FPD Block level software controlled reset
		(OFFSET, MASK, VALUE)      (0XFD1A0100, 0x00010000U ,0x00000000U)
		RegMask = (CRF_APB_RST_FPD_TOP_DP_RESET_MASK |  0 );

		RegVal = ((0x00000000U << CRF_APB_RST_FPD_TOP_DP_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_RST_FPD_TOP_OFFSET ,0x00010000U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : DP_PHY_RESET @ 0XFD4A0200</p>

		Set to '1' to hold the GT in reset. Clear to release.
		PSU_DP_DP_PHY_RESET_GT_RESET                                                    0X0

		Reset the transmitter PHY.
		(OFFSET, MASK, VALUE)      (0XFD4A0200, 0x00000002U ,0x00000000U)
		RegMask = (DP_DP_PHY_RESET_GT_RESET_MASK |  0 );

		RegVal = ((0x00000000U << DP_DP_PHY_RESET_GT_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (DP_DP_PHY_RESET_OFFSET ,0x00000002U ,0x00000000U);
	/*############################################################################################################################ */

		/*Register : DP_TX_PHY_POWER_DOWN @ 0XFD4A0238</p>

		Two bits per lane. When set to 11, moves the GT to power down mode. When set to 00, GT will be in active state. bits [1:0] -
		ane0 Bits [3:2] - lane 1
		PSU_DP_DP_TX_PHY_POWER_DOWN_POWER_DWN                                           0X0

		Control PHY Power down
		(OFFSET, MASK, VALUE)      (0XFD4A0238, 0x0000000FU ,0x00000000U)
		RegMask = (DP_DP_TX_PHY_POWER_DOWN_POWER_DWN_MASK |  0 );

		RegVal = ((0x00000000U << DP_DP_TX_PHY_POWER_DOWN_POWER_DWN_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (DP_DP_TX_PHY_POWER_DOWN_OFFSET ,0x0000000FU ,0x00000000U);
	/*############################################################################################################################ */

		// : USB0 GFLADJ
		/*Register : GUSB2PHYCFG @ 0XFE20C200</p>

		USB 2.0 Turnaround Time (USBTrdTim) Sets the turnaround time in PHY clocks. Specifies the response time for a MAC request to
		he Packet FIFO Controller (PFC) to fetch data from the DFIFO (SPRAM). The following are the required values for the minimum S
		C bus frequency of 60 MHz. USB turnaround time is a critical certification criteria when using long cables and five hub level
		. The required values for this field: - 4'h5: When the MAC interface is 16-bit UTMI+. - 4'h9: When the MAC interface is 8-bit
		UTMI+/ULPI. If SoC bus clock is less than 60 MHz, and USB turnaround time is not critical, this field can be set to a larger
		alue. Note: This field is valid only in device mode.
		PSU_USB3_0_XHCI_GUSB2PHYCFG_USBTRDTIM                                           0X9

		Transceiver Delay: Enables a delay between the assertion of the UTMI/ULPI Transceiver Select signal (for HS) and the assertio
		 of the TxValid signal during a HS Chirp. When this bit is set to 1, a delay (of approximately 2.5 us) is introduced from the
		time when the Transceiver Select is set to 2'b00 (HS) to the time the TxValid is driven to 0 for sending the chirp-K. This de
		ay is required for some UTMI/ULPI PHYs. Note: - If you enable the hibernation feature when the device core comes out of power
		off, you must re-initialize this bit with the appropriate value because the core does not save and restore this bit value dur
		ng hibernation. - This bit is valid only in device mode.
		PSU_USB3_0_XHCI_GUSB2PHYCFG_XCVRDLY                                             0X0

		Enable utmi_sleep_n and utmi_l1_suspend_n (EnblSlpM) The application uses this bit to control utmi_sleep_n and utmi_l1_suspen
		_n assertion to the PHY in the L1 state. - 1'b0: utmi_sleep_n and utmi_l1_suspend_n assertion from the core is not transferre
		 to the external PHY. - 1'b1: utmi_sleep_n and utmi_l1_suspend_n assertion from the core is transferred to the external PHY.
		ote: This bit must be set high for Port0 if PHY is used. Note: In Device mode - Before issuing any device endpoint command wh
		n operating in 2.0 speeds, disable this bit and enable it after the command completes. Without disabling this bit, if a comma
		d is issued when the device is in L1 state and if mac2_clk (utmi_clk/ulpi_clk) is gated off, the command will not get complet
		d.
		PSU_USB3_0_XHCI_GUSB2PHYCFG_ENBLSLPM                                            0X0

		USB 2.0 High-Speed PHY or USB 1.1 Full-Speed Serial Transceiver Select The application uses this bit to select a high-speed P
		Y or a full-speed transceiver. - 1'b0: USB 2.0 high-speed UTMI+ or ULPI PHY. This bit is always 0, with Write Only access. -
		'b1: USB 1.1 full-speed serial transceiver. This bit is always 1, with Write Only access. If both interface types are selecte
		 in coreConsultant (that is, parameters' values are not zero), the application uses this bit to select the active interface i
		 active, with Read-Write bit access. Note: USB 1.1 full-serial transceiver is not supported. This bit always reads as 1'b0.
		PSU_USB3_0_XHCI_GUSB2PHYCFG_PHYSEL                                              0X0

		Suspend USB2.0 HS/FS/LS PHY (SusPHY) When set, USB2.0 PHY enters Suspend mode if Suspend conditions are valid. For DRD/OTG co
		figurations, it is recommended that this bit is set to 0 during coreConsultant configuration. If it is set to 1, then the app
		ication must clear this bit after power-on reset. Application needs to set it to 1 after the core initialization completes. F
		r all other configurations, this bit can be set to 1 during core configuration. Note: - In host mode, on reset, this bit is s
		t to 1. Software can override this bit after reset. - In device mode, before issuing any device endpoint command when operati
		g in 2.0 speeds, disable this bit and enable it after the command completes. If you issue a command without disabling this bi
		 when the device is in L2 state and if mac2_clk (utmi_clk/ulpi_clk) is gated off, the command will not get completed.
		PSU_USB3_0_XHCI_GUSB2PHYCFG_SUSPENDUSB20                                        0X1

		Full-Speed Serial Interface Select (FSIntf) The application uses this bit to select a unidirectional or bidirectional USB 1.1
		full-speed serial transceiver interface. - 1'b0: 6-pin unidirectional full-speed serial interface. This bit is set to 0 with
		ead Only access. - 1'b1: 3-pin bidirectional full-speed serial interface. This bit is set to 0 with Read Only access. Note: U
		B 1.1 full-speed serial interface is not supported. This bit always reads as 1'b0.
		PSU_USB3_0_XHCI_GUSB2PHYCFG_FSINTF                                              0X0

		ULPI or UTMI+ Select (ULPI_UTMI_Sel) The application uses this bit to select a UTMI+ or ULPI Interface. - 1'b0: UTMI+ Interfa
		e - 1'b1: ULPI Interface This bit is writable only if UTMI+ and ULPI is specified for High-Speed PHY Interface(s) in coreCons
		ltant configuration (DWC_USB3_HSPHY_INTERFACE = 3). Otherwise, this bit is read-only and the value depends on the interface s
		lected through DWC_USB3_HSPHY_INTERFACE.
		PSU_USB3_0_XHCI_GUSB2PHYCFG_ULPI_UTMI_SEL                                       0X1

		PHY Interface (PHYIf) If UTMI+ is selected, the application uses this bit to configure the core to support a UTMI+ PHY with a
		 8- or 16-bit interface. - 1'b0: 8 bits - 1'b1: 16 bits ULPI Mode: 1'b0 Note: - All the enabled 2.0 ports must have the same
		lock frequency as Port0 clock frequency (utmi_clk[0]). - The UTMI 8-bit and 16-bit modes cannot be used together for differen
		 ports at the same time (that is, all the ports must be in 8-bit mode, or all of them must be in 16-bit mode, at a time). - I
		 any of the USB 2.0 ports is selected as ULPI port for operation, then all the USB 2.0 ports must be operating at 60 MHz.
		PSU_USB3_0_XHCI_GUSB2PHYCFG_PHYIF                                               0X0

		HS/FS Timeout Calibration (TOutCal) The number of PHY clocks, as indicated by the application in this field, is multiplied by
		a bit-time factor; this factor is added to the high-speed/full-speed interpacket timeout duration in the core to account for
		dditional delays introduced by the PHY. This may be required, since the delay introduced by the PHY in generating the linesta
		e condition may vary among PHYs. The USB standard timeout value for high-speed operation is 736 to 816 (inclusive) bit times.
		The USB standard timeout value for full-speed operation is 16 to 18 (inclusive) bit times. The application must program this
		ield based on the speed of connection. The number of bit times added per PHY clock are: High-speed operation: - One 30-MHz PH
		 clock = 16 bit times - One 60-MHz PHY clock = 8 bit times Full-speed operation: - One 30-MHz PHY clock = 0.4 bit times - One
		60-MHz PHY clock = 0.2 bit times - One 48-MHz PHY clock = 0.25 bit times
		PSU_USB3_0_XHCI_GUSB2PHYCFG_TOUTCAL                                             0X7

		Global USB2 PHY Configuration Register The application must program this register before starting any transactions on either
		he SoC bus or the USB. In Device-only configurations, only one register is needed. In Host mode, per-port registers are imple
		ented.
		(OFFSET, MASK, VALUE)      (0XFE20C200, 0x00003FFFU ,0x00002457U)
		RegMask = (USB3_0_XHCI_GUSB2PHYCFG_USBTRDTIM_MASK | USB3_0_XHCI_GUSB2PHYCFG_XCVRDLY_MASK | USB3_0_XHCI_GUSB2PHYCFG_ENBLSLPM_MASK | USB3_0_XHCI_GUSB2PHYCFG_PHYSEL_MASK | USB3_0_XHCI_GUSB2PHYCFG_SUSPENDUSB20_MASK | USB3_0_XHCI_GUSB2PHYCFG_FSINTF_MASK | USB3_0_XHCI_GUSB2PHYCFG_ULPI_UTMI_SEL_MASK | USB3_0_XHCI_GUSB2PHYCFG_PHYIF_MASK | USB3_0_XHCI_GUSB2PHYCFG_TOUTCAL_MASK |  0 );

		RegVal = ((0x00000009U << USB3_0_XHCI_GUSB2PHYCFG_USBTRDTIM_SHIFT
			| 0x00000000U << USB3_0_XHCI_GUSB2PHYCFG_XCVRDLY_SHIFT
			| 0x00000000U << USB3_0_XHCI_GUSB2PHYCFG_ENBLSLPM_SHIFT
			| 0x00000000U << USB3_0_XHCI_GUSB2PHYCFG_PHYSEL_SHIFT
			| 0x00000001U << USB3_0_XHCI_GUSB2PHYCFG_SUSPENDUSB20_SHIFT
			| 0x00000000U << USB3_0_XHCI_GUSB2PHYCFG_FSINTF_SHIFT
			| 0x00000001U << USB3_0_XHCI_GUSB2PHYCFG_ULPI_UTMI_SEL_SHIFT
			| 0x00000000U << USB3_0_XHCI_GUSB2PHYCFG_PHYIF_SHIFT
			| 0x00000007U << USB3_0_XHCI_GUSB2PHYCFG_TOUTCAL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (USB3_0_XHCI_GUSB2PHYCFG_OFFSET ,0x00003FFFU ,0x00002457U);
	/*############################################################################################################################ */

		/*Register : GFLADJ @ 0XFE20C630</p>

		This field indicates the frame length adjustment to be applied when SOF/ITP counter is running on the ref_clk. This register
		alue is used to adjust the ITP interval when GCTL[SOFITPSYNC] is set to '1'; SOF and ITP interval when GLADJ.GFLADJ_REFCLK_LP
		_SEL is set to '1'. This field must be programmed to a non-zero value only if GFLADJ_REFCLK_LPM_SEL is set to '1' or GCTL.SOF
		TPSYNC is set to '1'. The value is derived as follows: FLADJ_REF_CLK_FLADJ=((125000/ref_clk_period_integer)-(125000/ref_clk_p
		riod)) * ref_clk_period where - the ref_clk_period_integer is the integer value of the ref_clk period got by truncating the d
		cimal (fractional) value that is programmed in the GUCTL.REF_CLK_PERIOD field. - the ref_clk_period is the ref_clk period inc
		uding the fractional value. Examples: If the ref_clk is 24 MHz then - GUCTL.REF_CLK_PERIOD = 41 - GFLADJ.GLADJ_REFCLK_FLADJ =
		((125000/41)-(125000/41.6666))*41.6666 = 2032 (ignoring the fractional value) If the ref_clk is 48 MHz then - GUCTL.REF_CLK_P
		RIOD = 20 - GFLADJ.GLADJ_REFCLK_FLADJ = ((125000/20)-(125000/20.8333))*20.8333 = 5208 (ignoring the fractional value)
		PSU_USB3_0_XHCI_GFLADJ_GFLADJ_REFCLK_FLADJ                                      0X0

		Global Frame Length Adjustment Register This register provides options for the software to control the core behavior with res
		ect to SOF (Start of Frame) and ITP (Isochronous Timestamp Packet) timers and frame timer functionality. It provides an optio
		 to override the fladj_30mhz_reg sideband signal. In addition, it enables running SOF or ITP frame timer counters completely
		rom the ref_clk. This facilitates hardware LPM in host mode with the SOF or ITP counters being run from the ref_clk signal.
		(OFFSET, MASK, VALUE)      (0XFE20C630, 0x003FFF00U ,0x00000000U)
		RegMask = (USB3_0_XHCI_GFLADJ_GFLADJ_REFCLK_FLADJ_MASK |  0 );

		RegVal = ((0x00000000U << USB3_0_XHCI_GFLADJ_GFLADJ_REFCLK_FLADJ_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (USB3_0_XHCI_GFLADJ_OFFSET ,0x003FFF00U ,0x00000000U);
	/*############################################################################################################################ */

		// : USB1 GFLADJ
		/*Register : GUSB2PHYCFG @ 0XFE30C200</p>

		USB 2.0 Turnaround Time (USBTrdTim) Sets the turnaround time in PHY clocks. Specifies the response time for a MAC request to
		he Packet FIFO Controller (PFC) to fetch data from the DFIFO (SPRAM). The following are the required values for the minimum S
		C bus frequency of 60 MHz. USB turnaround time is a critical certification criteria when using long cables and five hub level
		. The required values for this field: - 4'h5: When the MAC interface is 16-bit UTMI+. - 4'h9: When the MAC interface is 8-bit
		UTMI+/ULPI. If SoC bus clock is less than 60 MHz, and USB turnaround time is not critical, this field can be set to a larger
		alue. Note: This field is valid only in device mode.
		PSU_USB3_1_XHCI_GUSB2PHYCFG_USBTRDTIM                                           0X9

		Transceiver Delay: Enables a delay between the assertion of the UTMI/ULPI Transceiver Select signal (for HS) and the assertio
		 of the TxValid signal during a HS Chirp. When this bit is set to 1, a delay (of approximately 2.5 us) is introduced from the
		time when the Transceiver Select is set to 2'b00 (HS) to the time the TxValid is driven to 0 for sending the chirp-K. This de
		ay is required for some UTMI/ULPI PHYs. Note: - If you enable the hibernation feature when the device core comes out of power
		off, you must re-initialize this bit with the appropriate value because the core does not save and restore this bit value dur
		ng hibernation. - This bit is valid only in device mode.
		PSU_USB3_1_XHCI_GUSB2PHYCFG_XCVRDLY                                             0X0

		Enable utmi_sleep_n and utmi_l1_suspend_n (EnblSlpM) The application uses this bit to control utmi_sleep_n and utmi_l1_suspen
		_n assertion to the PHY in the L1 state. - 1'b0: utmi_sleep_n and utmi_l1_suspend_n assertion from the core is not transferre
		 to the external PHY. - 1'b1: utmi_sleep_n and utmi_l1_suspend_n assertion from the core is transferred to the external PHY.
		ote: This bit must be set high for Port0 if PHY is used. Note: In Device mode - Before issuing any device endpoint command wh
		n operating in 2.0 speeds, disable this bit and enable it after the command completes. Without disabling this bit, if a comma
		d is issued when the device is in L1 state and if mac2_clk (utmi_clk/ulpi_clk) is gated off, the command will not get complet
		d.
		PSU_USB3_1_XHCI_GUSB2PHYCFG_ENBLSLPM                                            0X0

		USB 2.0 High-Speed PHY or USB 1.1 Full-Speed Serial Transceiver Select The application uses this bit to select a high-speed P
		Y or a full-speed transceiver. - 1'b0: USB 2.0 high-speed UTMI+ or ULPI PHY. This bit is always 0, with Write Only access. -
		'b1: USB 1.1 full-speed serial transceiver. This bit is always 1, with Write Only access. If both interface types are selecte
		 in coreConsultant (that is, parameters' values are not zero), the application uses this bit to select the active interface i
		 active, with Read-Write bit access. Note: USB 1.1 full-serial transceiver is not supported. This bit always reads as 1'b0.
		PSU_USB3_1_XHCI_GUSB2PHYCFG_PHYSEL                                              0X0

		Suspend USB2.0 HS/FS/LS PHY (SusPHY) When set, USB2.0 PHY enters Suspend mode if Suspend conditions are valid. For DRD/OTG co
		figurations, it is recommended that this bit is set to 0 during coreConsultant configuration. If it is set to 1, then the app
		ication must clear this bit after power-on reset. Application needs to set it to 1 after the core initialization completes. F
		r all other configurations, this bit can be set to 1 during core configuration. Note: - In host mode, on reset, this bit is s
		t to 1. Software can override this bit after reset. - In device mode, before issuing any device endpoint command when operati
		g in 2.0 speeds, disable this bit and enable it after the command completes. If you issue a command without disabling this bi
		 when the device is in L2 state and if mac2_clk (utmi_clk/ulpi_clk) is gated off, the command will not get completed.
		PSU_USB3_1_XHCI_GUSB2PHYCFG_SUSPENDUSB20                                        0X1

		Full-Speed Serial Interface Select (FSIntf) The application uses this bit to select a unidirectional or bidirectional USB 1.1
		full-speed serial transceiver interface. - 1'b0: 6-pin unidirectional full-speed serial interface. This bit is set to 0 with
		ead Only access. - 1'b1: 3-pin bidirectional full-speed serial interface. This bit is set to 0 with Read Only access. Note: U
		B 1.1 full-speed serial interface is not supported. This bit always reads as 1'b0.
		PSU_USB3_1_XHCI_GUSB2PHYCFG_FSINTF                                              0X0

		ULPI or UTMI+ Select (ULPI_UTMI_Sel) The application uses this bit to select a UTMI+ or ULPI Interface. - 1'b0: UTMI+ Interfa
		e - 1'b1: ULPI Interface This bit is writable only if UTMI+ and ULPI is specified for High-Speed PHY Interface(s) in coreCons
		ltant configuration (DWC_USB3_HSPHY_INTERFACE = 3). Otherwise, this bit is read-only and the value depends on the interface s
		lected through DWC_USB3_HSPHY_INTERFACE.
		PSU_USB3_1_XHCI_GUSB2PHYCFG_ULPI_UTMI_SEL                                       0X1

		PHY Interface (PHYIf) If UTMI+ is selected, the application uses this bit to configure the core to support a UTMI+ PHY with a
		 8- or 16-bit interface. - 1'b0: 8 bits - 1'b1: 16 bits ULPI Mode: 1'b0 Note: - All the enabled 2.0 ports must have the same
		lock frequency as Port0 clock frequency (utmi_clk[0]). - The UTMI 8-bit and 16-bit modes cannot be used together for differen
		 ports at the same time (that is, all the ports must be in 8-bit mode, or all of them must be in 16-bit mode, at a time). - I
		 any of the USB 2.0 ports is selected as ULPI port for operation, then all the USB 2.0 ports must be operating at 60 MHz.
		PSU_USB3_1_XHCI_GUSB2PHYCFG_PHYIF                                               0X0

		HS/FS Timeout Calibration (TOutCal) The number of PHY clocks, as indicated by the application in this field, is multiplied by
		a bit-time factor; this factor is added to the high-speed/full-speed interpacket timeout duration in the core to account for
		dditional delays introduced by the PHY. This may be required, since the delay introduced by the PHY in generating the linesta
		e condition may vary among PHYs. The USB standard timeout value for high-speed operation is 736 to 816 (inclusive) bit times.
		The USB standard timeout value for full-speed operation is 16 to 18 (inclusive) bit times. The application must program this
		ield based on the speed of connection. The number of bit times added per PHY clock are: High-speed operation: - One 30-MHz PH
		 clock = 16 bit times - One 60-MHz PHY clock = 8 bit times Full-speed operation: - One 30-MHz PHY clock = 0.4 bit times - One
		60-MHz PHY clock = 0.2 bit times - One 48-MHz PHY clock = 0.25 bit times
		PSU_USB3_1_XHCI_GUSB2PHYCFG_TOUTCAL                                             0X7

		Global USB2 PHY Configuration Register The application must program this register before starting any transactions on either
		he SoC bus or the USB. In Device-only configurations, only one register is needed. In Host mode, per-port registers are imple
		ented.
		(OFFSET, MASK, VALUE)      (0XFE30C200, 0x00003FFFU ,0x00002457U)
		RegMask = (USB3_1_XHCI_GUSB2PHYCFG_USBTRDTIM_MASK | USB3_1_XHCI_GUSB2PHYCFG_XCVRDLY_MASK | USB3_1_XHCI_GUSB2PHYCFG_ENBLSLPM_MASK | USB3_1_XHCI_GUSB2PHYCFG_PHYSEL_MASK | USB3_1_XHCI_GUSB2PHYCFG_SUSPENDUSB20_MASK | USB3_1_XHCI_GUSB2PHYCFG_FSINTF_MASK | USB3_1_XHCI_GUSB2PHYCFG_ULPI_UTMI_SEL_MASK | USB3_1_XHCI_GUSB2PHYCFG_PHYIF_MASK | USB3_1_XHCI_GUSB2PHYCFG_TOUTCAL_MASK |  0 );

		RegVal = ((0x00000009U << USB3_1_XHCI_GUSB2PHYCFG_USBTRDTIM_SHIFT
			| 0x00000000U << USB3_1_XHCI_GUSB2PHYCFG_XCVRDLY_SHIFT
			| 0x00000000U << USB3_1_XHCI_GUSB2PHYCFG_ENBLSLPM_SHIFT
			| 0x00000000U << USB3_1_XHCI_GUSB2PHYCFG_PHYSEL_SHIFT
			| 0x00000001U << USB3_1_XHCI_GUSB2PHYCFG_SUSPENDUSB20_SHIFT
			| 0x00000000U << USB3_1_XHCI_GUSB2PHYCFG_FSINTF_SHIFT
			| 0x00000001U << USB3_1_XHCI_GUSB2PHYCFG_ULPI_UTMI_SEL_SHIFT
			| 0x00000000U << USB3_1_XHCI_GUSB2PHYCFG_PHYIF_SHIFT
			| 0x00000007U << USB3_1_XHCI_GUSB2PHYCFG_TOUTCAL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (USB3_1_XHCI_GUSB2PHYCFG_OFFSET ,0x00003FFFU ,0x00002457U);
	/*############################################################################################################################ */

		/*Register : GFLADJ @ 0XFE30C630</p>

		This field indicates the frame length adjustment to be applied when SOF/ITP counter is running on the ref_clk. This register
		alue is used to adjust the ITP interval when GCTL[SOFITPSYNC] is set to '1'; SOF and ITP interval when GLADJ.GFLADJ_REFCLK_LP
		_SEL is set to '1'. This field must be programmed to a non-zero value only if GFLADJ_REFCLK_LPM_SEL is set to '1' or GCTL.SOF
		TPSYNC is set to '1'. The value is derived as follows: FLADJ_REF_CLK_FLADJ=((125000/ref_clk_period_integer)-(125000/ref_clk_p
		riod)) * ref_clk_period where - the ref_clk_period_integer is the integer value of the ref_clk period got by truncating the d
		cimal (fractional) value that is programmed in the GUCTL.REF_CLK_PERIOD field. - the ref_clk_period is the ref_clk period inc
		uding the fractional value. Examples: If the ref_clk is 24 MHz then - GUCTL.REF_CLK_PERIOD = 41 - GFLADJ.GLADJ_REFCLK_FLADJ =
		((125000/41)-(125000/41.6666))*41.6666 = 2032 (ignoring the fractional value) If the ref_clk is 48 MHz then - GUCTL.REF_CLK_P
		RIOD = 20 - GFLADJ.GLADJ_REFCLK_FLADJ = ((125000/20)-(125000/20.8333))*20.8333 = 5208 (ignoring the fractional value)
		PSU_USB3_1_XHCI_GFLADJ_GFLADJ_REFCLK_FLADJ                                      0X0

		Global Frame Length Adjustment Register This register provides options for the software to control the core behavior with res
		ect to SOF (Start of Frame) and ITP (Isochronous Timestamp Packet) timers and frame timer functionality. It provides an optio
		 to override the fladj_30mhz_reg sideband signal. In addition, it enables running SOF or ITP frame timer counters completely
		rom the ref_clk. This facilitates hardware LPM in host mode with the SOF or ITP counters being run from the ref_clk signal.
		(OFFSET, MASK, VALUE)      (0XFE30C630, 0x003FFF00U ,0x00000000U)
		RegMask = (USB3_1_XHCI_GFLADJ_GFLADJ_REFCLK_FLADJ_MASK |  0 );

		RegVal = ((0x00000000U << USB3_1_XHCI_GFLADJ_GFLADJ_REFCLK_FLADJ_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (USB3_1_XHCI_GFLADJ_OFFSET ,0x003FFF00U ,0x00000000U);
	/*############################################################################################################################ */

		// : CHECK PLL LOCK FOR LANE0
		/*Register : L0_PLL_STATUS_READ_1 @ 0XFD4023E4</p>

		Status Read value of PLL Lock
		PSU_SERDES_L0_PLL_STATUS_READ_1_PLL_LOCK_STATUS_READ                            1
		(OFFSET, MASK, VALUE)      (0XFD4023E4, 0x00000010U ,0x00000010U)  */
		mask_poll(SERDES_L0_PLL_STATUS_READ_1_OFFSET,0x00000010U);

	/*############################################################################################################################ */

		// : CHECK PLL LOCK FOR LANE1
		/*Register : L1_PLL_STATUS_READ_1 @ 0XFD4063E4</p>

		Status Read value of PLL Lock
		PSU_SERDES_L1_PLL_STATUS_READ_1_PLL_LOCK_STATUS_READ                            1
		(OFFSET, MASK, VALUE)      (0XFD4063E4, 0x00000010U ,0x00000010U)  */
		mask_poll(SERDES_L1_PLL_STATUS_READ_1_OFFSET,0x00000010U);

	/*############################################################################################################################ */

		// : CHECK PLL LOCK FOR LANE2
		/*Register : L2_PLL_STATUS_READ_1 @ 0XFD40A3E4</p>

		Status Read value of PLL Lock
		PSU_SERDES_L2_PLL_STATUS_READ_1_PLL_LOCK_STATUS_READ                            1
		(OFFSET, MASK, VALUE)      (0XFD40A3E4, 0x00000010U ,0x00000010U)  */
		mask_poll(SERDES_L2_PLL_STATUS_READ_1_OFFSET,0x00000010U);

	/*############################################################################################################################ */

		// : CHECK PLL LOCK FOR LANE3
		/*Register : L3_PLL_STATUS_READ_1 @ 0XFD40E3E4</p>

		Status Read value of PLL Lock
		PSU_SERDES_L3_PLL_STATUS_READ_1_PLL_LOCK_STATUS_READ                            1
		(OFFSET, MASK, VALUE)      (0XFD40E3E4, 0x00000010U ,0x00000010U)  */
		mask_poll(SERDES_L3_PLL_STATUS_READ_1_OFFSET,0x00000010U);

	/*############################################################################################################################ */

		// : UPDATING TWO PCIE REGISTERS DEFAULT VALUES, AS THESE REGISTERS HAVE INCORRECT RESET VALUES IN SILICON.
		/*Register : ATTR_25 @ 0XFD480064</p>

		If TRUE Completion Timeout Disable is supported. This is required to be TRUE for Endpoint and either setting allowed for Root
		ports. Drives Device Capability 2 [4]; EP=0x0001; RP=0x0001
		PSU_PCIE_ATTRIB_ATTR_25_ATTR_CPL_TIMEOUT_DISABLE_SUPPORTED                      0X1

		ATTR_25
		(OFFSET, MASK, VALUE)      (0XFD480064, 0x00000200U ,0x00000200U)
		RegMask = (PCIE_ATTRIB_ATTR_25_ATTR_CPL_TIMEOUT_DISABLE_SUPPORTED_MASK |  0 );

		RegVal = ((0x00000001U << PCIE_ATTRIB_ATTR_25_ATTR_CPL_TIMEOUT_DISABLE_SUPPORTED_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (PCIE_ATTRIB_ATTR_25_OFFSET ,0x00000200U ,0x00000200U);
	/*############################################################################################################################ */


  return 1;
}
unsigned long psu_resetin_init_data() {
		// : PUTTING SERDES PERIPHERAL IN RESET
		// : PUTTING USB0 IN RESET
		/*Register : RST_LPD_TOP @ 0XFF5E023C</p>

		USB 0 reset for control registers
		PSU_CRL_APB_RST_LPD_TOP_USB0_APB_RESET                                          0X1

		USB 0 sleep circuit reset
		PSU_CRL_APB_RST_LPD_TOP_USB0_HIBERRESET                                         0X1

		USB 0 reset
		PSU_CRL_APB_RST_LPD_TOP_USB0_CORERESET                                          0X1

		Software control register for the LPD block.
		(OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00000540U ,0x00000540U)
		RegMask = (CRL_APB_RST_LPD_TOP_USB0_APB_RESET_MASK | CRL_APB_RST_LPD_TOP_USB0_HIBERRESET_MASK | CRL_APB_RST_LPD_TOP_USB0_CORERESET_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_RST_LPD_TOP_USB0_APB_RESET_SHIFT
			| 0x00000001U << CRL_APB_RST_LPD_TOP_USB0_HIBERRESET_SHIFT
			| 0x00000001U << CRL_APB_RST_LPD_TOP_USB0_CORERESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_TOP_OFFSET ,0x00000540U ,0x00000540U);
	/*############################################################################################################################ */

		// : PUTTING USB1 IN RESET
		/*Register : RST_LPD_TOP @ 0XFF5E023C</p>

		USB 1 reset for control registers
		PSU_CRL_APB_RST_LPD_TOP_USB1_APB_RESET                                          0X1

		USB 1 sleep circuit reset
		PSU_CRL_APB_RST_LPD_TOP_USB1_HIBERRESET                                         0X1

		USB 1 reset
		PSU_CRL_APB_RST_LPD_TOP_USB1_CORERESET                                          0X1

		Software control register for the LPD block.
		(OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00000A80U ,0x00000A80U)
		RegMask = (CRL_APB_RST_LPD_TOP_USB1_APB_RESET_MASK | CRL_APB_RST_LPD_TOP_USB1_HIBERRESET_MASK | CRL_APB_RST_LPD_TOP_USB1_CORERESET_MASK |  0 );

		RegVal = ((0x00000001U << CRL_APB_RST_LPD_TOP_USB1_APB_RESET_SHIFT
			| 0x00000001U << CRL_APB_RST_LPD_TOP_USB1_HIBERRESET_SHIFT
			| 0x00000001U << CRL_APB_RST_LPD_TOP_USB1_CORERESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRL_APB_RST_LPD_TOP_OFFSET ,0x00000A80U ,0x00000A80U);
	/*############################################################################################################################ */

		// : PUTTING DP IN RESET
		/*Register : DP_TX_PHY_POWER_DOWN @ 0XFD4A0238</p>

		Two bits per lane. When set to 11, moves the GT to power down mode. When set to 00, GT will be in active state. bits [1:0] -
		ane0 Bits [3:2] - lane 1
		PSU_DP_DP_TX_PHY_POWER_DOWN_POWER_DWN                                           0XA

		Control PHY Power down
		(OFFSET, MASK, VALUE)      (0XFD4A0238, 0x0000000FU ,0x0000000AU)
		RegMask = (DP_DP_TX_PHY_POWER_DOWN_POWER_DWN_MASK |  0 );

		RegVal = ((0x0000000AU << DP_DP_TX_PHY_POWER_DOWN_POWER_DWN_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (DP_DP_TX_PHY_POWER_DOWN_OFFSET ,0x0000000FU ,0x0000000AU);
	/*############################################################################################################################ */

		/*Register : DP_PHY_RESET @ 0XFD4A0200</p>

		Set to '1' to hold the GT in reset. Clear to release.
		PSU_DP_DP_PHY_RESET_GT_RESET                                                    0X1

		Reset the transmitter PHY.
		(OFFSET, MASK, VALUE)      (0XFD4A0200, 0x00000002U ,0x00000002U)
		RegMask = (DP_DP_PHY_RESET_GT_RESET_MASK |  0 );

		RegVal = ((0x00000001U << DP_DP_PHY_RESET_GT_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (DP_DP_PHY_RESET_OFFSET ,0x00000002U ,0x00000002U);
	/*############################################################################################################################ */

		/*Register : RST_FPD_TOP @ 0XFD1A0100</p>

		Display Port block level reset (includes DPDMA)
		PSU_CRF_APB_RST_FPD_TOP_DP_RESET                                                0X1

		FPD Block level software controlled reset
		(OFFSET, MASK, VALUE)      (0XFD1A0100, 0x00010000U ,0x00010000U)
		RegMask = (CRF_APB_RST_FPD_TOP_DP_RESET_MASK |  0 );

		RegVal = ((0x00000001U << CRF_APB_RST_FPD_TOP_DP_RESET_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (CRF_APB_RST_FPD_TOP_OFFSET ,0x00010000U ,0x00010000U);
	/*############################################################################################################################ */


  return 1;
}
unsigned long psu_ps_pl_isolation_removal_data() {
		// : PS-PL POWER UP REQUEST
		/*Register : REQ_PWRUP_INT_EN @ 0XFFD80118</p>

		Power-up Request Interrupt Enable for PL
		PSU_PMU_GLOBAL_REQ_PWRUP_INT_EN_PL                                              1

		Power-up Request Interrupt Enable Register. Writing a 1 to this location will unmask the interrupt.
		(OFFSET, MASK, VALUE)      (0XFFD80118, 0x00800000U ,0x00800000U)
		RegMask = (PMU_GLOBAL_REQ_PWRUP_INT_EN_PL_MASK |  0 );

		RegVal = ((0x00000001U << PMU_GLOBAL_REQ_PWRUP_INT_EN_PL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (PMU_GLOBAL_REQ_PWRUP_INT_EN_OFFSET ,0x00800000U ,0x00800000U);
	/*############################################################################################################################ */

		/*Register : REQ_PWRUP_TRIG @ 0XFFD80120</p>

		Power-up Request Trigger for PL
		PSU_PMU_GLOBAL_REQ_PWRUP_TRIG_PL                                                1

		Power-up Request Trigger Register. A write of one to this location will generate a power-up request to the PMU.
		(OFFSET, MASK, VALUE)      (0XFFD80120, 0x00800000U ,0x00800000U)
		RegMask = (PMU_GLOBAL_REQ_PWRUP_TRIG_PL_MASK |  0 );

		RegVal = ((0x00000001U << PMU_GLOBAL_REQ_PWRUP_TRIG_PL_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (PMU_GLOBAL_REQ_PWRUP_TRIG_OFFSET ,0x00800000U ,0x00800000U);
	/*############################################################################################################################ */

		// : POLL ON PL POWER STATUS
		/*Register : REQ_PWRUP_STATUS @ 0XFFD80110</p>

		Power-up Request Status for PL
		PSU_PMU_GLOBAL_REQ_PWRUP_STATUS_PL                                              1
		(OFFSET, MASK, VALUE)      (0XFFD80110, 0x00800000U ,0x00000000U)  */
		mask_pollOnValue(PMU_GLOBAL_REQ_PWRUP_STATUS_OFFSET,0x00800000U,0x00000000U);

	/*############################################################################################################################ */


  return 1;
}
unsigned long psu_ps_pl_reset_config_data() {
		// : PS PL RESET SEQUENCE
		// : FABRIC RESET USING EMIO
		/*Register : MASK_DATA_5_MSW @ 0XFF0A002C</p>

		Operation is the same as MASK_DATA_0_LSW[MASK_0_LSW]
		PSU_GPIO_MASK_DATA_5_MSW_MASK_5_MSW                                             0x8000

		Maskable Output Data (GPIO Bank5, EMIO, Upper 16bits)
		(OFFSET, MASK, VALUE)      (0XFF0A002C, 0xFFFF0000U ,0x80000000U)
		RegMask = (GPIO_MASK_DATA_5_MSW_MASK_5_MSW_MASK |  0 );

		RegVal = ((0x00008000U << GPIO_MASK_DATA_5_MSW_MASK_5_MSW_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (GPIO_MASK_DATA_5_MSW_OFFSET ,0xFFFF0000U ,0x80000000U);
	/*############################################################################################################################ */

		/*Register : DIRM_5 @ 0XFF0A0344</p>

		Operation is the same as DIRM_0[DIRECTION_0]
		PSU_GPIO_DIRM_5_DIRECTION_5                                                     0x80000000

		Direction mode (GPIO Bank5, EMIO)
		(OFFSET, MASK, VALUE)      (0XFF0A0344, 0xFFFFFFFFU ,0x80000000U)
		RegMask = (GPIO_DIRM_5_DIRECTION_5_MASK |  0 );

		RegVal = ((0x80000000U << GPIO_DIRM_5_DIRECTION_5_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (GPIO_DIRM_5_OFFSET ,0xFFFFFFFFU ,0x80000000U);
	/*############################################################################################################################ */

		/*Register : OEN_5 @ 0XFF0A0348</p>

		Operation is the same as OEN_0[OP_ENABLE_0]
		PSU_GPIO_OEN_5_OP_ENABLE_5                                                      0x80000000

		Output enable (GPIO Bank5, EMIO)
		(OFFSET, MASK, VALUE)      (0XFF0A0348, 0xFFFFFFFFU ,0x80000000U)
		RegMask = (GPIO_OEN_5_OP_ENABLE_5_MASK |  0 );

		RegVal = ((0x80000000U << GPIO_OEN_5_OP_ENABLE_5_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (GPIO_OEN_5_OFFSET ,0xFFFFFFFFU ,0x80000000U);
	/*############################################################################################################################ */

		/*Register : DATA_5 @ 0XFF0A0054</p>

		Output Data
		PSU_GPIO_DATA_5_DATA_5                                                          0x80000000

		Output Data (GPIO Bank5, EMIO)
		(OFFSET, MASK, VALUE)      (0XFF0A0054, 0xFFFFFFFFU ,0x80000000U)
		RegMask = (GPIO_DATA_5_DATA_5_MASK |  0 );

		RegVal = ((0x80000000U << GPIO_DATA_5_DATA_5_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (GPIO_DATA_5_OFFSET ,0xFFFFFFFFU ,0x80000000U);
	/*############################################################################################################################ */

		mask_delay(1);

	/*############################################################################################################################ */

		// : FABRIC RESET USING DATA_5 TOGGLE
		/*Register : DATA_5 @ 0XFF0A0054</p>

		Output Data
		PSU_GPIO_DATA_5_DATA_5                                                          0X00000000

		Output Data (GPIO Bank5, EMIO)
		(OFFSET, MASK, VALUE)      (0XFF0A0054, 0xFFFFFFFFU ,0x00000000U)
		RegMask = (GPIO_DATA_5_DATA_5_MASK |  0 );

		RegVal = ((0x00000000U << GPIO_DATA_5_DATA_5_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (GPIO_DATA_5_OFFSET ,0xFFFFFFFFU ,0x00000000U);
	/*############################################################################################################################ */

		mask_delay(1);

	/*############################################################################################################################ */

		// : FABRIC RESET USING DATA_5 TOGGLE
		/*Register : DATA_5 @ 0XFF0A0054</p>

		Output Data
		PSU_GPIO_DATA_5_DATA_5                                                          0x80000000

		Output Data (GPIO Bank5, EMIO)
		(OFFSET, MASK, VALUE)      (0XFF0A0054, 0xFFFFFFFFU ,0x80000000U)
		RegMask = (GPIO_DATA_5_DATA_5_MASK |  0 );

		RegVal = ((0x80000000U << GPIO_DATA_5_DATA_5_SHIFT
			|  0 ) & RegMask); */
		PSU_Mask_Write (GPIO_DATA_5_OFFSET ,0xFFFFFFFFU ,0x80000000U);
	/*############################################################################################################################ */


  return 1;
}
unsigned long psu_ddr_phybringup_data() {return 1;}

#define CRF_APB_BASEADDR      0XFD1A0000U
#define CRF_APB_RST_DDR_SS_OFFSET                                                  0XFD1A0108
#define CRF_APB_RST_DDR_SS    ( ( CRF_APB_BASEADDR ) + 0X00000108U )

void noinline prog_reg_ddr(unsigned long addr, unsigned long shift,
		     unsigned long mask, unsigned long value)
{
	int rdata = 0;

	rdata = Xil_In32(addr);
	rdata = rdata & (~mask);
	rdata = rdata | (value << shift);
	Xil_Out32(addr,rdata);
}

unsigned long psu_ddr_init_data() {
   // ***** zcu100 is set to 1 *****
 // ***** lp4_wrk_around is overwritten to 1 *****



unsigned int regval = 0;
unsigned int tmp_regval;
int tp;


 tmp_regval = Xil_In32(CRF_APB_RST_DDR_SS);
 tmp_regval = tmp_regval & 0xFFFFFFF7;
 tmp_regval = tmp_regval | 0x00000008;
 Xil_Out32(CRF_APB_RST_DDR_SS, tmp_regval);


Xil_Out32(0xFD070000, 0x81081020); //MSTR
Xil_Out32(0xFD070010, 0x00000030); //MRCTRL0
Xil_Out32(0xFD070020, 0x00000102); //DERATEEN
Xil_Out32(0xFD070024, 0x0028AA28); //DERATEINT
Xil_Out32(0xFD070030, 0x00000000); //PWRCTL
Xil_Out32(0xFD070034, 0x00404310); //PWRTMG
Xil_Out32(0xFD070050, 0x00210000); //RFSHCTL0
Xil_Out32(0xFD070060, 0x00000000); //RFSHCTL3
Xil_Out32(0xFD070064, 0x00208030); //RFSHTMG
Xil_Out32(0xFD070070, 0x00000010); //ECCCFG0
Xil_Out32(0xFD070074, 0x00000000); //ECCCFG1
Xil_Out32(0xFD0700C4, 0x10000200); //CRCPARCTL1
Xil_Out32(0xFD0700C8, 0x0030051F); //CRCPARCTL2
Xil_Out32(0xFD0700D0, 0x0002020A); //INIT0
Xil_Out32(0xFD0700D4, 0x00360000); //INIT1
Xil_Out32(0xFD0700D8, 0x00001205); //INIT2
Xil_Out32(0xFD0700DC, 0x00140009); //INIT3
Xil_Out32(0xFD0700E0, 0x00310008); //INIT4
Xil_Out32(0xFD0700E4, 0x00210004); //INIT5
Xil_Out32(0xFD0700E8, 0x00000000); //INIT6
Xil_Out32(0xFD0700EC, 0x00000000); //INIT7
Xil_Out32(0xFD0700F0, 0x00000010); //DIMMCTL
Xil_Out32(0xFD0700F4, 0x0000077F); //RANKCTL
Xil_Out32(0xFD070100, 0x0D0B010C); //DRAMTMG0
Xil_Out32(0xFD070104, 0x00030411); //DRAMTMG1
Xil_Out32(0xFD070108, 0x03050D0C); //DRAMTMG2
Xil_Out32(0xFD07010C, 0x00A05000); //DRAMTMG3
Xil_Out32(0xFD070110, 0x05040306); //DRAMTMG4
Xil_Out32(0xFD070114, 0x01020404); //DRAMTMG5
Xil_Out32(0xFD070118, 0x01010004); //DRAMTMG6
Xil_Out32(0xFD07011C, 0x00000201); //DRAMTMG7
Xil_Out32(0xFD070120, 0x03030303); //DRAMTMG8
Xil_Out32(0xFD070124, 0x0004040D); //DRAMTMG9
Xil_Out32(0xFD07012C, 0x440C011C); //DRAMTMG11
Xil_Out32(0xFD070130, 0x00020608); //DRAMTMG12
Xil_Out32(0xFD070180, 0x010B0008); //ZQCTL0
Xil_Out32(0xFD070184, 0x00E32D4B); //ZQCTL1
Xil_Out32(0xFD070190, 0x04878204); //DFITMG0
Xil_Out32(0xFD070194, 0x00030304); //DFITMG1
Xil_Out32(0xFD070198, 0x07000101); //DFILPCFG0
Xil_Out32(0xFD07019C, 0x00000021); //DFILPCFG1
Xil_Out32(0xFD0701A0, 0x03FF0003); //DFIUPD0
Xil_Out32(0xFD0701A4, 0x00A00070); //DFIUPD1
Xil_Out32(0xFD0701B0, 0x00000004); //DFIMISC
Xil_Out32(0xFD0701B4, 0x0000053F); //DFITMG2
Xil_Out32(0xFD0701C0, 0x00000001); //DBICTL
Xil_Out32(0xFD070200, 0x0000001F); //ADDRMAP0
Xil_Out32(0xFD070204, 0x00070707); //ADDRMAP1
Xil_Out32(0xFD070208, 0x00000000); //ADDRMAP2
Xil_Out32(0xFD07020C, 0x0F000000); //ADDRMAP3
Xil_Out32(0xFD070210, 0x00000F0F); //ADDRMAP4
Xil_Out32(0xFD070214, 0x060F0606); //ADDRMAP5
Xil_Out32(0xFD070218, 0x0F060606); //ADDRMAP6
Xil_Out32(0xFD07021C, 0x00000F0F); //ADDRMAP7
Xil_Out32(0xFD070220, 0x00000000); //ADDRMAP8
Xil_Out32(0xFD070224, 0x06060606); //ADDRMAP9
Xil_Out32(0xFD070228, 0x06060606); //ADDRMAP10
Xil_Out32(0xFD07022C, 0x00000006); //ADDRMAP11
Xil_Out32(0xFD070240, 0x04000400); //ODTCFG
Xil_Out32(0xFD070244, 0x00000000); //ODTMAP
Xil_Out32(0xFD070250, 0x01002001); //SCHED
Xil_Out32(0xFD070264, 0x08000040); //PERFLPR1
Xil_Out32(0xFD07026C, 0x08000040); //PERFWR1
Xil_Out32(0xFD070294, 0x00000001); //DQMAP5
Xil_Out32(0xFD070300, 0x00000000); //DBG0
Xil_Out32(0xFD07030C, 0x00000000); //DBGCMD
Xil_Out32(0xFD070320, 0x00000000); //SWCTL
Xil_Out32(0xFD070400, 0x00000001); //PCCFG
Xil_Out32(0xFD070404, 0x0000200F); //PCFGR_0
Xil_Out32(0xFD070408, 0x0000200F); //PCFGW_0
Xil_Out32(0xFD070490, 0x00000001); //PCTRL_0
Xil_Out32(0xFD070494, 0x0020000B); //PCFGQOS0_0
Xil_Out32(0xFD070498, 0x00000000); //PCFGQOS1_0
Xil_Out32(0xFD0704B4, 0x0000200F); //PCFGR_1
Xil_Out32(0xFD0704B8, 0x0000200F); //PCFGW_1
Xil_Out32(0xFD070540, 0x00000001); //PCTRL_1
Xil_Out32(0xFD070544, 0x02000B03); //PCFGQOS0_1
Xil_Out32(0xFD070548, 0x00000000); //PCFGQOS1_1
Xil_Out32(0xFD070564, 0x0000200F); //PCFGR_2
Xil_Out32(0xFD070568, 0x0000200F); //PCFGW_2
Xil_Out32(0xFD0705F0, 0x00000001); //PCTRL_2
Xil_Out32(0xFD0705F4, 0x02000B03); //PCFGQOS0_2
Xil_Out32(0xFD0705F8, 0x00000000); //PCFGQOS1_2
Xil_Out32(0xFD070614, 0x0000200F); //PCFGR_3
Xil_Out32(0xFD070618, 0x0000200F); //PCFGW_3
Xil_Out32(0xFD0706A0, 0x00000001); //PCTRL_3
Xil_Out32(0xFD0706A4, 0x00100003); //PCFGQOS0_3
Xil_Out32(0xFD0706A8, 0x0000004F); //PCFGQOS1_3
Xil_Out32(0xFD0706AC, 0x00100003); //PCFGWQOS0_3
Xil_Out32(0xFD0706B0, 0x0000004F); //PCFGWQOS1_3
Xil_Out32(0xFD0706C4, 0x0000200F); //PCFGR_4
Xil_Out32(0xFD0706C8, 0x0000200F); //PCFGW_4
Xil_Out32(0xFD070750, 0x00000001); //PCTRL_4
Xil_Out32(0xFD070754, 0x00100003); //PCFGQOS0_4
Xil_Out32(0xFD070758, 0x0000004F); //PCFGQOS1_4
Xil_Out32(0xFD07075C, 0x00100003); //PCFGWQOS0_4
Xil_Out32(0xFD070760, 0x0000004F); //PCFGWQOS1_4
Xil_Out32(0xFD070774, 0x0000200F); //PCFGR_5
Xil_Out32(0xFD070778, 0x0000200F); //PCFGW_5
Xil_Out32(0xFD070800, 0x00000001); //PCTRL_5
Xil_Out32(0xFD070804, 0x00100003); //PCFGQOS0_5
Xil_Out32(0xFD070808, 0x0000004F); //PCFGQOS1_5
Xil_Out32(0xFD07080C, 0x00100003); //PCFGWQOS0_5
Xil_Out32(0xFD070810, 0x0000004F); //PCFGWQOS1_5
Xil_Out32(0xFD070F04, 0x00000000); //SARBASE0
Xil_Out32(0xFD070F08, 0x00000000); //SARSIZE0
Xil_Out32(0xFD070F0C, 0x00000010); //SARBASE1
Xil_Out32(0xFD070F10, 0x0000000F); //SARSIZE1
Xil_Out32(0xFD072190, 0x07828002); //DFITMG0_SHADOW
 for(tp=0;tp<10;tp++) {
    tmp_regval = Xil_In32(CRF_APB_RST_DDR_SS);
    tmp_regval = tmp_regval & 0xFFFFFFF7;
    tmp_regval = tmp_regval | 0x00000000;
    Xil_Out32(CRF_APB_RST_DDR_SS, tmp_regval);
 }
Xil_Out32(0xFD080010, 0x87001E00); //PGCR0
Xil_Out32(0xFD080018, 0x00F03D18); //PGCR2
Xil_Out32(0xFD08001C, 0x55AA5480); //PGCR3
Xil_Out32(0xFD080024, 0x010100F4); //PGCR5
Xil_Out32(0xFD080040, 0x5E001810); //PTR0
Xil_Out32(0xFD080044, 0xA068057A); //PTR1
Xil_Out32(0xFD080090, 0x02A040A1); //DSGCR
Xil_Out32(0xFD080100, 0x0000040D); //DCR
Xil_Out32(0xFD080110, 0x06180C08); //DTPR0
Xil_Out32(0xFD080114, 0x2816050A); //DTPR1
Xil_Out32(0xFD080118, 0x00080064); //DTPR2
Xil_Out32(0xFD08011C, 0x82000501); //DTPR3
Xil_Out32(0xFD080120, 0x00602B08); //DTPR4
Xil_Out32(0xFD080124, 0x00221008); //DTPR5
Xil_Out32(0xFD080128, 0x0000060A); //DTPR6
Xil_Out32(0xFD080140, 0x08400020); //RDIMMGCR0
Xil_Out32(0xFD080144, 0x00000C80); //RDIMMGCR1
Xil_Out32(0xFD080150, 0x00000000); //RDIMMCR0
Xil_Out32(0xFD080154, 0x00000000); //RDIMMCR1
Xil_Out32(0xFD080180, 0x00000000); //MR0
Xil_Out32(0xFD080184, 0x00000014); //MR1
Xil_Out32(0xFD080188, 0x00000009); //MR2
Xil_Out32(0xFD08018C, 0x00000031); //MR3
Xil_Out32(0xFD080190, 0x00000008); //MR4
Xil_Out32(0xFD080194, 0x00000000); //MR5
Xil_Out32(0xFD080198, 0x00000000); //MR6
Xil_Out32(0xFD0801AC, 0x00000056); //MR11
Xil_Out32(0xFD0801B0, 0x00000021); //MR12
Xil_Out32(0xFD0801B4, 0x00000008); //MR13
Xil_Out32(0xFD0801B8, 0x00000019); //MR14
Xil_Out32(0xFD0801D8, 0x00000016); //MR22
Xil_Out32(0xFD080200, 0x800091C7); //DTCR0
Xil_Out32(0xFD080204, 0x00010236); //DTCR1
Xil_Out32(0xFD080240, 0x00141054); //CATR0
Xil_Out32(0xFD080250, 0x00088000); //DQSDR0
Xil_Out32(0xFD080414, 0x12340400); //BISTLSR
Xil_Out32(0xFD0804F4, 0x0000000A); //RIOCR5
Xil_Out32(0xFD080500, 0x30000028); //ACIOCR0
Xil_Out32(0xFD080508, 0x00000000); //ACIOCR2
Xil_Out32(0xFD08050C, 0x00000005); //ACIOCR3
Xil_Out32(0xFD080510, 0x00000000); //ACIOCR4
Xil_Out32(0xFD080520, 0x0300BD99); //IOVCR0
Xil_Out32(0xFD080528, 0xF1032019); //VTCR0
Xil_Out32(0xFD08052C, 0x07F001E3); //VTCR1
Xil_Out32(0xFD080544, 0x00000000); //ACBDLR1
Xil_Out32(0xFD080548, 0x00000000); //ACBDLR2
Xil_Out32(0xFD080558, 0x00000000); //ACBDLR6
Xil_Out32(0xFD08055C, 0x00000000); //ACBDLR7
Xil_Out32(0xFD080560, 0x00000000); //ACBDLR8
Xil_Out32(0xFD080564, 0x00000000); //ACBDLR9
Xil_Out32(0xFD080680, 0x00894C58); //ZQCR
Xil_Out32(0xFD080684, 0x0001B39B); //ZQ0PR0
Xil_Out32(0xFD080694, 0x01E10210); //ZQ0OR0
Xil_Out32(0xFD080698, 0x01E10000); //ZQ0OR1
Xil_Out32(0xFD0806A4, 0x0001BB9B); //ZQ1PR0
Xil_Out32(0xFD080700, 0x40800604); //DX0GCR0
Xil_Out32(0xFD080710, 0x0E00F50C); //DX0GCR4
Xil_Out32(0xFD080714, 0x09091616); //DX0GCR5
Xil_Out32(0xFD080718, 0x09092B2B); //DX0GCR6
Xil_Out32(0xFD080800, 0x40800604); //DX1GCR0
Xil_Out32(0xFD080810, 0x0E00F50C); //DX1GCR4
Xil_Out32(0xFD080814, 0x09091616); //DX1GCR5
Xil_Out32(0xFD080818, 0x09092B2B); //DX1GCR6
Xil_Out32(0xFD080900, 0x40800604); //DX2GCR0
Xil_Out32(0xFD080904, 0x00007FFF); //DX2GCR1
Xil_Out32(0xFD080910, 0x0E00F50C); //DX2GCR4
Xil_Out32(0xFD080914, 0x09091616); //DX2GCR5
Xil_Out32(0xFD080918, 0x09092B2B); //DX2GCR6
Xil_Out32(0xFD080A00, 0x40800604); //DX3GCR0
Xil_Out32(0xFD080A04, 0x00007FFF); //DX3GCR1
Xil_Out32(0xFD080A10, 0x0E00F50C); //DX3GCR4
Xil_Out32(0xFD080A14, 0x09091616); //DX3GCR5
Xil_Out32(0xFD080A18, 0x09092B2B); //DX3GCR6
Xil_Out32(0xFD080B00, 0x40800604); //DX4GCR0
Xil_Out32(0xFD080B04, 0x00007F00); //DX4GCR1
Xil_Out32(0xFD080B10, 0x0E00BD0C); //DX4GCR4
Xil_Out32(0xFD080B14, 0x09091616); //DX4GCR5
Xil_Out32(0xFD080B18, 0x09092B2B); //DX4GCR6
Xil_Out32(0xFD080C00, 0x40800604); //DX5GCR0
Xil_Out32(0xFD080C04, 0x00007F00); //DX5GCR1
Xil_Out32(0xFD080C10, 0x0E00BD0C); //DX5GCR4
Xil_Out32(0xFD080C14, 0x09091616); //DX5GCR5
Xil_Out32(0xFD080C18, 0x09092B2B); //DX5GCR6
Xil_Out32(0xFD080D00, 0x40800604); //DX6GCR0
Xil_Out32(0xFD080D04, 0x00007F00); //DX6GCR1
Xil_Out32(0xFD080D10, 0x0E00BD0C); //DX6GCR4
Xil_Out32(0xFD080D14, 0x09091616); //DX6GCR5
Xil_Out32(0xFD080D18, 0x09092B2B); //DX6GCR6
Xil_Out32(0xFD080E00, 0x40800604); //DX7GCR0
Xil_Out32(0xFD080E04, 0x00007F00); //DX7GCR1
Xil_Out32(0xFD080E10, 0x0E00BD0C); //DX7GCR4
Xil_Out32(0xFD080E14, 0x09091616); //DX7GCR5
Xil_Out32(0xFD080E18, 0x09092B2B); //DX7GCR6
Xil_Out32(0xFD080F00, 0x40800624); //DX8GCR0
Xil_Out32(0xFD080F04, 0x00007F00); //DX8GCR1
Xil_Out32(0xFD080F10, 0x0E00BD0C); //DX8GCR4
Xil_Out32(0xFD080F14, 0x09091616); //DX8GCR5
Xil_Out32(0xFD080F18, 0x09092B2B); //DX8GCR6
Xil_Out32(0xFD081400, 0x2A019FFE); //DX8SL0OSC
Xil_Out32(0xFD08141C, 0x01264300); //DX8SL0DQSCTL
Xil_Out32(0xFD08142C, 0x000C1800); //DX8SL0DXCTL2
Xil_Out32(0xFD081430, 0x71000000); //DX8SL0IOCR
Xil_Out32(0xFD081440, 0x2A019FFE); //DX8SL1OSC
Xil_Out32(0xFD08145C, 0x01264300); //DX8SL1DQSCTL
Xil_Out32(0xFD08146C, 0x000C1800); //DX8SL1DXCTL2
Xil_Out32(0xFD081470, 0x71000000); //DX8SL1IOCR
Xil_Out32(0xFD081480, 0x2A019FFE); //DX8SL2OSC
Xil_Out32(0xFD08149C, 0x01264300); //DX8SL2DQSCTL
Xil_Out32(0xFD0814AC, 0x000C1800); //DX8SL2DXCTL2
Xil_Out32(0xFD0814B0, 0x71000000); //DX8SL2IOCR
Xil_Out32(0xFD0814C0, 0x2A019FFE); //DX8SL3OSC
Xil_Out32(0xFD0814DC, 0x01264300); //DX8SL3DQSCTL
Xil_Out32(0xFD0814EC, 0x000C1800); //DX8SL3DXCTL2
Xil_Out32(0xFD0814F0, 0x71000000); //DX8SL3IOCR
Xil_Out32(0xFD081500, 0x2A019FFE); //DX8SL4OSC
Xil_Out32(0xFD08151C, 0x01264300); //DX8SL4DQSCTL
Xil_Out32(0xFD08152C, 0x000C1800); //DX8SL4DXCTL2
Xil_Out32(0xFD081530, 0x71000000); //DX8SL4IOCR
Xil_Out32(0xFD0817DC, 0x012643C4); //DX8SLBDQSCTL
Xil_Out32(0xFD080004, 0x00040073); //PIR

// PHY BRINGUP SEQ
while ((Xil_In32(0xFD080030) & 0x0000000F) != 0x0000000F);

prog_reg_ddr (0xFD080004, 0x0, 0x1, 0x1);
//poll for PHY initialization to complete
while ((Xil_In32(0xFD080030) & 0x000000FF) != 0x0000001F);

Xil_Out32(0xFD070010, 0x00000038); //MRCTRL0
Xil_Out32(0xFD0701B0, 0x00000005); //DFIMISC
Xil_Out32(0xFD070014, 0x00000331); //MRCTRL1
Xil_Out32(0xFD070010, 0x80000018); //MRCTRL0
  regval = Xil_In32(0xFD070018); //MRSTAT
  while((regval & 0x1) != 0x0){
     regval = Xil_In32(0xFD070018); //MRSTAT
  }

  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
Xil_Out32(0xFD070014, 0x00000B36); //MRCTRL1
Xil_Out32(0xFD070010, 0x80000018); //MRCTRL0
  regval = Xil_In32(0xFD070018); //MRSTAT
  while((regval & 0x1) != 0x0){
     regval = Xil_In32(0xFD070018); //MRSTAT
  }

  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
Xil_Out32(0xFD070014, 0x00000C21); //MRCTRL1
Xil_Out32(0xFD070010, 0x80000018); //MRCTRL0
  regval = Xil_In32(0xFD070018); //MRSTAT
  while((regval & 0x1) != 0x0){
     regval = Xil_In32(0xFD070018); //MRSTAT
  }

  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
Xil_Out32(0xFD070014, 0x00000E19); //MRCTRL1
Xil_Out32(0xFD070010, 0x80000018); //MRCTRL0
  regval = Xil_In32(0xFD070018); //MRSTAT
  while((regval & 0x1) != 0x0){
     regval = Xil_In32(0xFD070018); //MRSTAT
  }

  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
Xil_Out32(0xFD070014, 0x00001616); //MRCTRL1
Xil_Out32(0xFD070010, 0x80000018); //MRCTRL0
  regval = Xil_In32(0xFD070018); //MRSTAT
  while((regval & 0x1) != 0x0){
     regval = Xil_In32(0xFD070018); //MRSTAT
  }

  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
  regval = Xil_In32(0xFD070018); //MRSTAT
Xil_Out32(0xFD070010, 0x80000010); //MRCTRL0
Xil_Out32(0xFD0701B0, 0x00000005); //DFIMISC
Xil_Out32(0xFD070320, 0x00000001); //SWCTL
while ((Xil_In32(0xFD070004) & 0x0000000F) != 0x00000001);
////////////////////////////////////////////////////////////////////////////////
// LPDDR4 training work around code
////////////////////////////////////////////////////////////////////////////////
prog_reg_ddr (0xFD080014, 0x6, 0x40, 0x1);
prog_reg_ddr (0xFD080680, 0x5, 0xE0, 0x0);
prog_reg_ddr (0xFD080028, 0x0, 0x1, 0x1);
prog_reg_ddr (0xFD070320, 0x0, 0x1, 0x0);
prog_reg_ddr (0xFD0701A0, 0x1F, 0x80000000, 0x1);
prog_reg_ddr (0xFD080004, 0xA, 0x400, 0x1);
prog_reg_ddr (0xFD080004, 0x9, 0x200, 0x1);
prog_reg_ddr (0xFD080004, 0x0, 0x1, 0x1);
while ((Xil_In32(0xFD080030) & 0x00000001) != 0x00000001);
int wdqsl_a0;
wdqsl_a0 = (Xil_In32(0xFD0807C0) & 0x07000000) >> 24;
int wdqsl_a1;
wdqsl_a1 = (Xil_In32(0xFD0808C0) & 0x07000000) >> 24;
int wdqsl_a2;
wdqsl_a2 = (Xil_In32(0xFD0809C0) & 0x07000000) >> 24;
int wdqsl_a3;
wdqsl_a3 = (Xil_In32(0xFD080AC0) & 0x07000000) >> 24;
int lcdl_a0;
lcdl_a0 = (Xil_In32(0xFD080784) & 0x000001FF) >> 0;
int lcdl_a1;
lcdl_a1 = (Xil_In32(0xFD080884) & 0x000001FF) >> 0;
int lcdl_a2;
lcdl_a2 = (Xil_In32(0xFD080984) & 0x000001FF) >> 0;
int lcdl_a3;
lcdl_a3 = (Xil_In32(0xFD080A84) & 0x000001FF) >> 0;
int iprd0;
iprd0 = (Xil_In32(0xFD0807A0) & 0x000001FF) >> 0;
int iprd1;
iprd1 = (Xil_In32(0xFD0808A0) & 0x000001FF) >> 0;
int iprd2;
iprd2 = (Xil_In32(0xFD0809A0) & 0x000001FF) >> 0;
int iprd3;
iprd3 = (Xil_In32(0xFD080AA0) & 0x000001FF) >> 0;
prog_reg_ddr (0xFD080004, 0x14, 0x100000, 0x1);
prog_reg_ddr (0xFD080004, 0x0, 0x1, 0x1);
while ((Xil_In32(0xFD080030) & 0x00008001) != 0x00008001)
  ;

int wdqsl_b0;
wdqsl_b0 = (Xil_In32(0xFD0807C0) & 0x07000000) >> 24;
int wdqsl_b1;
wdqsl_b1 = (Xil_In32(0xFD0808C0) & 0x07000000) >> 24;
int lcdl_b0;
lcdl_b0 = (Xil_In32(0xFD080784) & 0x000001FF) >> 0;
int lcdl_b1;
lcdl_b1 = (Xil_In32(0xFD080884) & 0x000001FF) >> 0;

// ** declare variables used for calculation **

int calc10, calc11, calc20, calc21, calc22, calc23;

calc10 = ((wdqsl_b0 * iprd0) + lcdl_b0) - ((wdqsl_a0 * iprd0) + lcdl_a0);
calc11 = ((wdqsl_b1 * iprd1) + lcdl_b1) - ((wdqsl_a1 * iprd1) + lcdl_a1);

calc20 = (calc10 / 2) + ((wdqsl_a0 * iprd0) + lcdl_a0);
calc21 = (calc11 / 2) + ((wdqsl_a1 * iprd1) + lcdl_a1);
calc22 = (calc10 / 2) + ((wdqsl_a2 * iprd2) + lcdl_a2);
calc23 = (calc11 / 2) + ((wdqsl_a3 * iprd3) + lcdl_a3);

prog_reg_ddr (0xFD0807C0, 0x18, 0x7000000, 0x0);
prog_reg_ddr (0xFD0808C0, 0x18, 0x7000000, 0x0);
prog_reg_ddr (0xFD0809C0, 0x18, 0x7000000, 0x0);
prog_reg_ddr (0xFD080AC0, 0x18, 0x7000000, 0x0);
prog_reg_ddr (0xFD080784, 0, 0x000001FF, calc20);
prog_reg_ddr (0xFD080884, 0, 0x000001FF, calc21);
prog_reg_ddr (0xFD080984, 0, 0x000001FF, calc22);
prog_reg_ddr (0xFD080A84, 0, 0x000001FF, calc23);
prog_reg_ddr (0xFD080004, 0xE, 0x4000, 0x1);
prog_reg_ddr (0xFD080004, 0x0, 0x1, 0x1);
while ((Xil_In32(0xFD080030) & 0x00000401) != 0x00000401);
prog_reg_ddr (0xFD080004, 0xB, 0x800, 0x1);
prog_reg_ddr (0xFD080004, 0xC, 0x1000, 0x1);
prog_reg_ddr (0xFD080004, 0xD, 0x2000, 0x1);
prog_reg_ddr (0xFD080004, 0xE, 0x4000, 0x1);
prog_reg_ddr (0xFD080004, 0xF, 0x8000, 0x1);
prog_reg_ddr (0xFD080004, 0x0, 0x1, 0x1);
while ((Xil_In32(0xFD080030) & 0x00000001) != 0x00000001);
prog_reg_ddr (0xFD080018, 0x0, 0x3FFFF, 0xEAD);
prog_reg_ddr (0xFD08001C, 0x3, 0x18, 0x3);
prog_reg_ddr (0xFD08142C, 0x4, 0x30, 0x3);
prog_reg_ddr (0xFD08146C, 0x4, 0x30, 0x3);
prog_reg_ddr (0xFD0814AC, 0x4, 0x30, 0x3);
prog_reg_ddr (0xFD0814EC, 0x4, 0x30, 0x3);
prog_reg_ddr (0xFD08152C, 0x4, 0x30, 0x3);
prog_reg_ddr (0xFD080004, 0x11, 0x20000, 0x1);
prog_reg_ddr (0xFD080004, 0x0, 0x1, 0x1);
while ((Xil_In32(0xFD080030) & 0x00000001) != 0x00000001);
prog_reg_ddr (0xFD080004, 0xE, 0x4000, 0x1);
prog_reg_ddr (0xFD080004, 0xF, 0x8000, 0x1);
prog_reg_ddr (0xFD080004, 0x0, 0x1, 0x1);
while ((Xil_In32(0xFD080030) & 0x00000001) != 0x00000001);
prog_reg_ddr (0xFD080200, 0x1C, 0xF0000000, 0x8);
prog_reg_ddr (0xFD08001C, 0x3, 0x18, 0x0);
prog_reg_ddr (0xFD08142C, 0x4, 0x30, 0x0);
prog_reg_ddr (0xFD08146C, 0x4, 0x30, 0x0);
prog_reg_ddr (0xFD0814AC, 0x4, 0x30, 0x0);
prog_reg_ddr (0xFD0814EC, 0x4, 0x30, 0x0);
prog_reg_ddr (0xFD08152C, 0x4, 0x30, 0x0);
prog_reg_ddr (0xFD080680, 0x5, 0xE0, 0x2);
prog_reg_ddr (0xFD080028, 0x0, 0x1, 0x0);
prog_reg_ddr (0xFD0701A0, 0x1F, 0x80000000, 0x0);
prog_reg_ddr (0xFD070320, 0x0, 0x1, 0x1);
prog_reg_ddr (0xFD070020, 0x0, 0x1, 0x1);
prog_reg_ddr (0xFD080014, 0x6, 0x40, 0x0);
prog_reg_ddr (0xFD080090, 0x6, 0xFC0, 0x4);
prog_reg_ddr (0xFD080090, 0x2, 0x4, 0x1);
prog_reg_ddr (0xFD08070C, 0x19, 0x2000000, 0x0);
prog_reg_ddr (0xFD08080C, 0x19, 0x2000000, 0x0);
prog_reg_ddr (0xFD08090C, 0x19, 0x2000000, 0x0);
prog_reg_ddr (0xFD080A0C, 0x19, 0x2000000, 0x0);
prog_reg_ddr (0xFD080F0C, 0x19, 0x2000000, 0x0);
prog_reg_ddr (0xFD080200, 0x4, 0x10, 0x1);
prog_reg_ddr (0xFD080250, 0x1, 0x2, 0x0);
prog_reg_ddr (0xFD080250, 0x2, 0xC, 0x1);
prog_reg_ddr (0xFD080250, 0x4, 0xF0, 0x0);
prog_reg_ddr (0xFD080250, 0x14, 0x300000, 0x1);
prog_reg_ddr (0xFD080250, 0x1C, 0xF0000000, 0x2);
prog_reg_ddr (0xFD08070C, 0x1B, 0x8000000, 0x0);
prog_reg_ddr (0xFD08080C, 0x1B, 0x8000000, 0x0);
prog_reg_ddr (0xFD08090C, 0x1B, 0x8000000, 0x0);
prog_reg_ddr (0xFD080A0C, 0x1B, 0x8000000, 0x0);
prog_reg_ddr (0xFD080B0C, 0x1B, 0x8000000, 0x0);
prog_reg_ddr (0xFD080C0C, 0x1B, 0x8000000, 0x0);
prog_reg_ddr (0xFD080D0C, 0x1B, 0x8000000, 0x0);
prog_reg_ddr (0xFD080E0C, 0x1B, 0x8000000, 0x0);
prog_reg_ddr (0xFD080F0C, 0x1B, 0x8000000, 0x0);
prog_reg_ddr (0xFD080254, 0x0, 0xFF, 0x1);
prog_reg_ddr (0xFD080254, 0x10, 0xF0000, 0xA);
prog_reg_ddr (0xFD080250, 0x0, 0x1, 0x1);
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// zddr_driver.c ver. 1.97, Oct  6 2016 , 23:36:35 ********
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ---- INPUT PARAMETERS ----
// sim mode    = 0
// train       = 0
// gls       = 0
// Vref       = 0
// slowboot     = 0
// ecc         = 0
// bus width   = 32
// dram width  = 16
// display_port(dp) = 0
// memory type = LPDDR4
// speed bin   = 1066
// frequency   = 533.000 MHz
// device capacity  = 8192 Mbit
// rank   addr cnt  = 0
// lp4catrain   = 0
// no_derate  = 0
// enable 2nd clk  = 0
// bank group  addr cnt  = 0
// bank   addr cnt  = 3
// row    addr cnt  = 15
// column addr cnt  = 10
// video buffer size = 0
// decoder stride size = 0
// decoder tile width = 0
// decoder tile height = 0
// temp ctrl ref mode = 0
// temp ctrl ref range = 0
// CL   = 6 cycles
// CWL  = 0 cycles
// tRCD = 10 cycles
// tRP  = 12 cycles
// AL   = 0 cycles
// BL   = 16 (burst length)
// tRC  = 63.000 nsec
// tRAS = 42.000 nsec
// tFAW = 40.000 nsec
// si_ver = 0.000000
// clock_stop_en = 0
// wr_drift  = 1
// rd_drift  = 1
// rd_dbi = 0
// wr_dbi = 0
// phy_dbi_mode = 0
// rdbi_wrk_around = 0
// zcal_wrk_around = 0
// lp4_wrk_around = 1
// data_mask = 1
// ecc_scrub = 0
// dis_dfi_lp_sr = 0
// dis_dfi_lp_pd = 0
// dis_dfi_lp_mpsm = 0
// ecc_poison = 0
// derate_int_d = 10000
// parity = 0
// ca_parity_latency = 0
// en_2t_timing_mode = 0
// geardown = 0
// max_pwr_sav_en = 0
// cal_mode_en = 0
// self_ref_abort = 0
// lp_asr = 0
// udimm = 0
// rdimm = 0
// gate_ext = 0
// no_gate_ext_no_train = 0
// addr_mirror = 1
// dimm_addr_mirror = 0
// dis_op_inv = 0
// phy_clk_gate = 0
// no_retry = 0
// en_op_inv_after_train = 0
// pll_bypass = 0
// freq_b = 0
// deep_pwr_dn_en = 0
// pwr_dn_en = 0
// crc = 0
// fgrm = 0
// ddr4_addr_mapping = 0
// brc_mapping = 0
// wr_preamble = 0
// rd_preamble = 0
// wr_postamble = 0
// rd_postamble = 0
// lpddr4_hynix = 0
// lpddr4_samsung = 0
// per_bank_refresh = 0
// static_rd_mode = 0
// zcu100 = 1


  return 1;
}
/**
 * CRL_APB Base Address
 */
#define CRL_APB_BASEADDR      0XFF5E0000U
#define CRL_APB_RST_LPD_IOU0    ( ( CRL_APB_BASEADDR ) + 0X00000230U )
#define CRL_APB_RST_LPD_IOU1    ( ( CRL_APB_BASEADDR ) + 0X00000234U )
#define CRL_APB_RST_LPD_IOU2    ( ( CRL_APB_BASEADDR ) + 0X00000238U )
#define CRL_APB_RST_LPD_TOP    ( ( CRL_APB_BASEADDR ) + 0X0000023CU )
#define CRL_APB_IOU_SWITCH_CTRL    ( ( CRL_APB_BASEADDR ) + 0X0000009CU )

/**
 * CRF_APB Base Address
 */
#define CRF_APB_BASEADDR      0XFD1A0000U

#define CRF_APB_RST_FPD_TOP    ( ( CRF_APB_BASEADDR ) + 0X00000100U )
#define CRF_APB_GPU_REF_CTRL    ( ( CRF_APB_BASEADDR ) + 0X00000084U )
#define CRF_APB_RST_DDR_SS    ( ( CRF_APB_BASEADDR ) + 0X00000108U )
#define PSU_MASK_POLL_TIME 1100000


int mask_pollOnValue(u32 add , u32 mask, u32 value ) {
	volatile u32 *addr = (volatile u32*) add;
	int i = 0;
	while ((*addr & mask)!= value) {
		if (i == PSU_MASK_POLL_TIME) {
			return 0;
		}
		i++;
	}
	return 1;
	//xil_printf("MaskPoll : 0x%x --> 0x%x \n \r" , add, *addr);
}

int mask_poll(u32 add , u32 mask) {
	volatile u32 *addr = (volatile u32*) add;
	int i = 0;
	while (!(*addr & mask)) {
		if (i == PSU_MASK_POLL_TIME) {
			return 0;
		}
		i++;
	}
	return 1;
	//xil_printf("MaskPoll : 0x%x --> 0x%x \n \r" , add, *addr);
}

void mask_delay(u32 delay) {
    usleep (delay);
}

u32 mask_read(u32 add , u32 mask ) {
        volatile u32 *addr = (volatile u32*) add;
        u32 val = (*addr & mask);
        //xil_printf("MaskRead : 0x%x --> 0x%x \n \r" , add, val);
        return val;
}


//Following SERDES programming sequences that a user need to follow to work around the known limitation with SERDES.
//These sequences should done before STEP 1 and STEP 2 as described in previous section. These programming steps are
//required for current silicon version and are likely to undergo further changes with subsequent silicon versions.



int serdes_fixcal_code() {
	int MaskStatus = 1;

   // L3_TM_CALIB_DIG19
   Xil_Out32(0xFD40EC4C,0x00000020);
   //ICM_CFG0
   Xil_Out32(0xFD410010,0x00000001);

   //is calibration done, polling on L3_CALIB_DONE_STATUS
   MaskStatus = mask_poll(0xFD40EF14, 0x2);

   if (MaskStatus == 0)
   {
        xil_printf("SERDES initialization timed out\n\r");
   }

   unsigned int tmp_0_1;
   tmp_0_1 = mask_read(0xFD400B0C, 0x3F);

   unsigned int tmp_0_2 = tmp_0_1 & (0x7);
   unsigned int tmp_0_3 = tmp_0_1 & (0x38);
   //Configure ICM for de-asserting CMN_Resetn
   Xil_Out32(0xFD410010,0x00000000);
   Xil_Out32(0xFD410014,0x00000000);

   unsigned int tmp_0_2_mod = (tmp_0_2 <<1) | (0x1);
   tmp_0_2_mod = (tmp_0_2_mod <<4);

   tmp_0_3 = tmp_0_3 >>3;
   Xil_Out32(0xFD40EC4C,tmp_0_3);

   //L3_TM_CALIB_DIG18
   Xil_Out32(0xFD40EC48,tmp_0_2_mod);
   return MaskStatus;


}

int serdes_enb_coarse_saturation() {
  //Enable PLL Coarse Code saturation Logic
   Xil_Out32(0xFD402094,0x00000010);
   Xil_Out32(0xFD406094,0x00000010);
   Xil_Out32(0xFD40A094,0x00000010);
   Xil_Out32(0xFD40E094,0x00000010);
   return 1;
}

int init_serdes() {
	int status = 1;
	status &=  psu_resetin_init_data();

	status &= serdes_fixcal_code();
	status &= serdes_enb_coarse_saturation();

    status &=  psu_serdes_init_data();
	status &=  psu_resetout_init_data();

	return status;
}






void init_peripheral()
{
	unsigned int RegValue;

	/* Turn on IOU Clock */
	//Xil_Out32( CRL_APB_IOU_SWITCH_CTRL, 0x01001500);

	/* Release all resets in the IOU */
	Xil_Out32( CRL_APB_RST_LPD_IOU0, 0x00000000);
	Xil_Out32( CRL_APB_RST_LPD_IOU1, 0x00000000);
	Xil_Out32( CRL_APB_RST_LPD_IOU2, 0x00000000);

	/* Activate GPU clocks */
	//Xil_Out32(CRF_APB_GPU_REF_CTRL, 0x07001500);

	/* Take LPD out of reset except R5 */
	RegValue = Xil_In32(CRL_APB_RST_LPD_TOP);
	RegValue &= 0x7;
	Xil_Out32( CRL_APB_RST_LPD_TOP, RegValue);

	/* Take most of FPD out of reset */
	Xil_Out32( CRF_APB_RST_FPD_TOP, 0x00000000);

	/* Making DPDMA as secure */
  unsigned int tmp_regval;
  tmp_regval = Xil_In32(0xFD690040);
  tmp_regval &= ~0x00000001;
  Xil_Out32(0xFD690040, tmp_regval);

	/* Making PCIe as secure */
  tmp_regval = Xil_In32(0xFD690030);
  tmp_regval &= ~0x00000001;
  Xil_Out32(0xFD690030, tmp_regval);
}
int
psu_init()
{
	int status = 1;
	status &= psu_mio_init_data ();
	status &=   psu_pll_init_data ();
	status &=   psu_clock_init_data ();

	status &=  psu_ddr_init_data ();
	status &=  psu_ddr_phybringup_data ();
	status &=  psu_peripherals_init_data ();

	status &=  init_serdes();
	init_peripheral ();

	status &=  psu_peripherals_powerdwn_data ();

	if (status == 0) {
		return 1;
	}
	return 0;
}
