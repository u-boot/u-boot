// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 */
// [dwc_ddrphy_phyinit_main] Start of dwc_ddrphy_phyinit_main()
// [dwc_ddrphy_phyinit_sequence] Start of dwc_ddrphy_phyinit_sequence()
// [dwc_ddrphy_phyinit_initStruct] Start of dwc_ddrphy_phyinit_initStruct()
// [dwc_ddrphy_phyinit_initStruct] End of dwc_ddrphy_phyinit_initStruct()
// [dwc_ddrphy_phyinit_setDefault] Start of dwc_ddrphy_phyinit_setDefault()
// [dwc_ddrphy_phyinit_setDefault] End of dwc_ddrphy_phyinit_setDefault()

////##############################################################
//
//// dwc_ddrphy_phyinit_userCustom_overrideUserInput is a user-editable function.  User can edit this function according to their needs.
////
//// The purpose of dwc_ddrphy_phyinit_userCustom_overrideUserInput() is to override any
//// any field in Phyinit data structure set by dwc_ddrphy_phyinit_setDefault()
//// User should only override values in userInputBasic and userInputAdvanced.
//// IMPORTANT: in this function, user shall not override any values in the
//// messageblock directly on the data structue as the might be overwritten by
//// dwc_ddrphy_phyinit_calcMb().  Use dwc_ddrphy_phyinit_setMb() to set
//// messageblock parameters for override values to remain pervasive if
//// desired
//
////##############################################################

dwc_ddrphy_phyinit_userCustom_overrideUserInput();
//
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'DramType' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'DimmType' to 0x4
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'NumDbyte' to 0x2
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'NumActiveDbyteDfi0' to 0x2
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'NumActiveDbyteDfi1' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'NumAnib' to 0xa
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'NumRank_dfi0' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'NumRank_dfi1' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'DramDataWidth[0]' to 0x10
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'DramDataWidth[1]' to 0x10
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'DramDataWidth[2]' to 0x10
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'DramDataWidth[3]' to 0x10
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'NumPStates' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'Frequency[0]' to 0x640
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'PllBypass[0]' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'DfiFreqRatio[0]' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'Dfi1Exists' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'ExtCalResVal' to 0xf0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'ODTImpedance[0]' to 0x78
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'TxImpedance[0]' to 0x3c
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'MemAlertEn' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'MtestPUImp' to 0xf0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'DisDynAdrTri[0]' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'PhyMstrTrainInterval[0]' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'PhyMstrMaxReqToAck[0]' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'PhyMstrCtrlMode[0]' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'WDQSExt' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'CalInterval' to 0x9
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'CalOnce' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'RxEnBackOff' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'TrainSequenceCtrl' to 0x837f
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'SnpsUmctlOpt' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'SnpsUmctlF0RC5x[0]' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'TxSlewRiseDQ[0]' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'TxSlewFallDQ[0]' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'TxSlewRiseAC' to 0x66
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'TxSlewFallAC' to 0x26
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'IsHighVDD' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'TxSlewRiseCK' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'TxSlewFallCK' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'EnTdqs2dqTrackingTg0[0]' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'EnTdqs2dqTrackingTg1[0]' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'EnTdqs2dqTrackingTg2[0]' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'EnTdqs2dqTrackingTg3[0]' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'DqsOscRunTimeSel[0]' to 0x100
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'EnRxDqsTracking[0]' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'D5TxDqPreambleCtrl[0]' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'D5DisableRetraining' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'DisablePmuEcc' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'EnableMAlertAsync' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'Apb32BitMode' to 0x1
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'tDQS2DQ' to 0x2ee
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'tDQSCK' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'tCASL_override' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'tCASL_add[0][0]' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'tCASL_add[0][1]' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'tCASL_add[0][2]' to 0x0
//// [dwc_ddrphy_phyinit_setUserInput] Setting PHYINIT field 'tCASL_add[0][3]' to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MsgMisc to 0x7
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].Pstate to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].PllBypassEn to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].DRAMFreq to 0xc80
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].PhyVref to 0x40
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].D5Misc to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].WL_ADJ to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].SequenceCtrl to 0x837f
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].HdtCtrl to 0xc8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].PhyCfg to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].DFIMRLMargin to 0x2
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].X16Present to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].UseBroadcastMR to 0x1
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].DisabledDbyte to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].CATrainOpt to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].PhyConfigOverride to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].EnabledDQsChA to 0x10
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].CsPresentChA to 0x1
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR0_A0 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR2_A0 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR3_A0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR4_A0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR5_A0 to 0x20
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR6_A0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR8_A0 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR10_A0 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR11_A0 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR12_A0 to 0xd6
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR13_A0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR14_A0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR15_A0 to 0x3
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR111_A0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR32_A0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR33_A0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR34_A0 to 0x11
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR35_A0 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR37_A0 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR38_A0 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR39_A0 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR50_A0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR51_A0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR52_A0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR0_A1 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR2_A1 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR3_A1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR4_A1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR5_A1 to 0x20
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR6_A1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR8_A1 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR10_A1 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR11_A1 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR12_A1 to 0xd6
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR13_A1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR14_A1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR15_A1 to 0x3
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR111_A1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR32_A1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR33_A1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR34_A1 to 0x11
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR35_A1 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR37_A1 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR38_A1 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR39_A1 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR50_A1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR51_A1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR52_A1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR0_A2 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR2_A2 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR3_A2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR4_A2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR5_A2 to 0x20
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR6_A2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR8_A2 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR10_A2 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR11_A2 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR12_A2 to 0xd6
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR13_A2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR14_A2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR15_A2 to 0x3
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR111_A2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR32_A2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR33_A2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR34_A2 to 0x11
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR35_A2 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR37_A2 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR38_A2 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR39_A2 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR50_A2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR51_A2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR52_A2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR0_A3 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR2_A3 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR3_A3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR4_A3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR5_A3 to 0x20
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR6_A3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR8_A3 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR10_A3 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR11_A3 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR12_A3 to 0xd6
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR13_A3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR14_A3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR15_A3 to 0x3
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR111_A3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR32_A3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR33_A3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR34_A3 to 0x11
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR35_A3 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR37_A3 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR38_A3 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR39_A3 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR50_A3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR51_A3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR52_A3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].EnabledDQsChB to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].CsPresentChB to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR0_B0 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR2_B0 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR3_B0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR4_B0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR5_B0 to 0x20
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR6_B0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR8_B0 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR10_B0 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR11_B0 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR12_B0 to 0xd6
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR13_B0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR14_B0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR15_B0 to 0x3
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR111_B0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR32_B0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR33_B0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR34_B0 to 0x11
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR35_B0 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR37_B0 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR38_B0 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR39_B0 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR50_B0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR51_B0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR52_B0 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR0_B1 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR2_B1 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR3_B1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR4_B1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR5_B1 to 0x20
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR6_B1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR8_B1 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR10_B1 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR11_B1 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR12_B1 to 0xd6
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR13_B1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR14_B1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR15_B1 to 0x3
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR111_B1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR32_B1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR33_B1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR34_B1 to 0x11
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR35_B1 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR37_B1 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR38_B1 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR39_B1 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR50_B1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR51_B1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR52_B1 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR0_B2 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR2_B2 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR3_B2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR4_B2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR5_B2 to 0x20
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR6_B2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR8_B2 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR10_B2 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR11_B2 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR12_B2 to 0xd6
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR13_B2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR14_B2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR15_B2 to 0x3
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR111_B2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR32_B2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR33_B2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR34_B2 to 0x11
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR35_B2 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR37_B2 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR38_B2 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR39_B2 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR50_B2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR51_B2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR52_B2 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR0_B3 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR2_B3 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR3_B3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR4_B3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR5_B3 to 0x20
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR6_B3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR8_B3 to 0x8
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR10_B3 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR11_B3 to 0x2d
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR12_B3 to 0xd6
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR13_B3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR14_B3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR15_B3 to 0x3
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR111_B3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR32_B3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR33_B3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR34_B3 to 0x11
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR35_B3 to 0x4
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR37_B3 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR38_B3 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR39_B3 to 0x2c
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR50_B3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR51_B3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].MR52_B3 to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].WL_ADJ_START to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].WL_ADJ_END to 0x0
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].RCW00_ChA_D0 to 0x1
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].RCW00_ChA_D1 to 0x1
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].RCW00_ChB_D0 to 0x1
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].RCW00_ChB_D1 to 0x1
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib0 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib1 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib2 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib3 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib4 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib5 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib6 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib7 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib8 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib9 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib10 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib11 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib12 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib13 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib14 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib15 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib16 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib17 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib18 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR0Nib19 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib0 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib1 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib2 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib3 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib4 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib5 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib6 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib7 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib8 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib9 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib10 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib11 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib12 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib13 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib14 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib15 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib16 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib17 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib18 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR1Nib19 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib0 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib1 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib2 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib3 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib4 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib5 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib6 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib7 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib8 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib9 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib10 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib11 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib12 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib13 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib14 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib15 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib16 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib17 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib18 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR2Nib19 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib0 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib1 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib2 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib3 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib4 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib5 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib6 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib7 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib8 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib9 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib10 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib11 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib12 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib13 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib14 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib15 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib16 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib17 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib18 to 0x17
// [dwc_ddrphy_phyinit_setMb] Setting mb_DDR5U_1D[0].VrefDqR3Nib19 to 0x17
// [dwc_ddrphy_phyinit_userCustom_overrideUserInput] End of dwc_ddrphy_phyinit_userCustom_overrideUserInput()
//[dwc_ddrphy_phyinit_calcMb] Start of dwc_ddrphy_phyinit_calcMb()
//// [dwc_ddrphy_phyinit_softSetMb] mb_DDR5U_1D[0].Pstate override to 0x0
//// [dwc_ddrphy_phyinit_softSetMb] mb_DDR5U_1D[0].DRAMFreq override to 0xc80
//// [dwc_ddrphy_phyinit_softSetMb] mb_DDR5U_1D[0].PllBypassEn override to 0x0
//// [dwc_ddrphy_phyinit_softSetMb] mb_DDR5U_1D[0].X16Present override to 0x0
//// [dwc_ddrphy_phyinit_softSetMb] mb_DDR5U_1D[0].EnabledDQsChA override to 0x10
//// [dwc_ddrphy_phyinit_softSetMb] mb_DDR5U_1D[0].EnabledDQsChB override to 0x0
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[1].Pstate to 0x1
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[1].DRAMFreq to 0x856
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[1].PllBypassEn to 0x0
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[1].X16Present to 0x1
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[1].EnabledDQsChA to 0x10
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[1].EnabledDQsChB to 0x0
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[2].Pstate to 0x2
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[2].DRAMFreq to 0x74a
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[2].PllBypassEn to 0x0
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[2].X16Present to 0x1
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[2].EnabledDQsChA to 0x10
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[2].EnabledDQsChB to 0x0
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[3].Pstate to 0x3
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[3].DRAMFreq to 0x640
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[3].PllBypassEn to 0x0
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[3].X16Present to 0x1
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[3].EnabledDQsChA to 0x10
//// [dwc_ddrphy_phyinit_softSetMb] Setting mb_DDR5U_1D[3].EnabledDQsChB to 0x0
////[dwc_ddrphy_phyinit_calcMb] TG_active[0] = 1
////[dwc_ddrphy_phyinit_calcMb] TG_active[1] = 0
////[dwc_ddrphy_phyinit_calcMb] TG_active[2] = 0
////[dwc_ddrphy_phyinit_calcMb] TG_active[3] = 0
////[dwc_ddrphy_phyinit_calcMb] tDIMM_CK [pstate=0][tg=0] = 0 ps
////[dwc_ddrphy_phyinit_calcMb] tDIMM_DQ [pstate=0][tg=0] = 0 ps
////[dwc_ddrphy_phyinit_calcMb] userInputSim.tCASL_add[pstate=0][tg=0] = 0 ps
////[dwc_ddrphy_phyinit_calcMb] tDIMM_CK [pstate=0][tg=1] = 0 ps
////[dwc_ddrphy_phyinit_calcMb] tDIMM_DQ [pstate=0][tg=1] = 0 ps
////[dwc_ddrphy_phyinit_calcMb] userInputSim.tCASL_add[pstate=0][tg=1] = 0 ps
////[dwc_ddrphy_phyinit_calcMb] tDIMM_CK [pstate=0][tg=2] = 0 ps
////[dwc_ddrphy_phyinit_calcMb] tDIMM_DQ [pstate=0][tg=2] = 0 ps
////[dwc_ddrphy_phyinit_calcMb] userInputSim.tCASL_add[pstate=0][tg=2] = 0 ps
////[dwc_ddrphy_phyinit_calcMb] tDIMM_CK [pstate=0][tg=3] = 0 ps
////[dwc_ddrphy_phyinit_calcMb] tDIMM_DQ [pstate=0][tg=3] = 0 ps
////[dwc_ddrphy_phyinit_calcMb] userInputSim.tCASL_add[pstate=0][tg=3] = 0 ps
//[dwc_ddrphy_phyinit_calcMb] End of dwc_ddrphy_phyinit_calcMb()
//// [phyinit_print_dat] // ####################################################
//// [phyinit_print_dat] //
//// [phyinit_print_dat] // Printing values in user input structure
//// [phyinit_print_dat] //
//// [phyinit_print_dat] // ####################################################
//// [phyinit_print_dat] pUserInputBasic->DramDataWidth[0] = 16
//// [phyinit_print_dat] pUserInputBasic->DramDataWidth[1] = 16
//// [phyinit_print_dat] pUserInputBasic->DramDataWidth[2] = 16
//// [phyinit_print_dat] pUserInputBasic->DramDataWidth[3] = 16
//// [phyinit_print_dat] pUserInputBasic->NumActiveDbyteDfi1 = 0
//// [phyinit_print_dat] pUserInputBasic->DramType = 1
//// [phyinit_print_dat] pUserInputBasic->ARdPtrInitValOvr = 0
//// [phyinit_print_dat] pUserInputBasic->ARdPtrInitVal[0] = 3
//// [phyinit_print_dat] pUserInputBasic->ARdPtrInitVal[1] = 3
//// [phyinit_print_dat] pUserInputBasic->ARdPtrInitVal[2] = 3
//// [phyinit_print_dat] pUserInputBasic->ARdPtrInitVal[3] = 3
//// [phyinit_print_dat] pUserInputBasic->Dfi1Exists = 0
//// [phyinit_print_dat] pUserInputBasic->Frequency[0] = 1600
//// [phyinit_print_dat] pUserInputBasic->Frequency[1] = 1067
//// [phyinit_print_dat] pUserInputBasic->Frequency[2] = 933
//// [phyinit_print_dat] pUserInputBasic->Frequency[3] = 800
//// [phyinit_print_dat] pUserInputBasic->NumActiveDbyteDfi0 = 2
//// [phyinit_print_dat] pUserInputBasic->DisPtrInitClrTxTracking[0] = 0
//// [phyinit_print_dat] pUserInputBasic->DisPtrInitClrTxTracking[1] = 0
//// [phyinit_print_dat] pUserInputBasic->DisPtrInitClrTxTracking[2] = 0
//// [phyinit_print_dat] pUserInputBasic->DisPtrInitClrTxTracking[3] = 0
//// [phyinit_print_dat] pUserInputBasic->NumRank_dfi0 = 1
//// [phyinit_print_dat] pUserInputBasic->NumPStates = 1
//// [phyinit_print_dat] pUserInputBasic->PllBypass[0] = 0
//// [phyinit_print_dat] pUserInputBasic->PllBypass[1] = 0
//// [phyinit_print_dat] pUserInputBasic->PllBypass[2] = 0
//// [phyinit_print_dat] pUserInputBasic->PllBypass[3] = 0
//// [phyinit_print_dat] pUserInputBasic->DfiFreqRatio[0] = 1
//// [phyinit_print_dat] pUserInputBasic->DfiFreqRatio[1] = 1
//// [phyinit_print_dat] pUserInputBasic->DfiFreqRatio[2] = 1
//// [phyinit_print_dat] pUserInputBasic->DfiFreqRatio[3] = 1
//// [phyinit_print_dat] pUserInputBasic->NumAnib = 10
//// [phyinit_print_dat] pUserInputBasic->DimmType = 4
//// [phyinit_print_dat] pUserInputBasic->NumRank_dfi1 = 0
//// [phyinit_print_dat] pUserInputBasic->NumDbyte = 2
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg1[0] = 1
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg1[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg1[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg1[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg0[0] = 1
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg0[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg0[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg0[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->SnpsUmctlF0RC5x[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->SnpsUmctlF0RC5x[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->SnpsUmctlF0RC5x[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->SnpsUmctlF0RC5x[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedanceCtrl2[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedanceCtrl2[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedanceCtrl2[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedanceCtrl2[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->IsHighVDD = 0
//// [phyinit_print_dat] pUserInputAdvanced->DramByteSwap[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->DramByteSwap[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->DramByteSwap[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->DramByteSwap[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->ExtCalResVal = 240
//// [phyinit_print_dat] pUserInputAdvanced->D4TxPreambleLength[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->D4TxPreambleLength[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->D4TxPreambleLength[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->D4TxPreambleLength[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->RxEnBackOff = 1
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvLaneSel[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvLaneSel[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvLaneSel[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvLaneSel[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvLaneSel[4] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvLaneSel[5] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvLaneSel[6] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvLaneSel[7] = 0
//// [phyinit_print_dat] pUserInputAdvanced->CalOnce = 0
//// [phyinit_print_dat] pUserInputAdvanced->Apb32BitMode = 1
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewRiseCK = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewFallAC = 38
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewFallCK = 0
//// [phyinit_print_dat] pUserInputAdvanced->DisablePmuEcc = 1
//// [phyinit_print_dat] pUserInputAdvanced->SnpsUmctlOpt = 0
//// [phyinit_print_dat] pUserInputAdvanced->WDQSExt = 0
//// [phyinit_print_dat] pUserInputAdvanced->VREGCtrl_LP2_PwrSavings_En = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewRiseAC = 102
//// [phyinit_print_dat] pUserInputAdvanced->DisDynAdrTri[0] = 1
//// [phyinit_print_dat] pUserInputAdvanced->DisDynAdrTri[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->DisDynAdrTri[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->DisDynAdrTri[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvEn[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvEn[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvEn[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvEn[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvEn[4] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvEn[5] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvEn[6] = 0
//// [phyinit_print_dat] pUserInputAdvanced->AnibRcvEn[7] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg3[0] = 1
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg3[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg3[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg3[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrTrainInterval[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrTrainInterval[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrTrainInterval[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrTrainInterval[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->rtt_term_en = 0
//// [phyinit_print_dat] pUserInputAdvanced->AlertRecoveryEnable = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewRiseDQ[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewRiseDQ[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewRiseDQ[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewRiseDQ[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->en_16LogicalRanks_3DS = 0
//// [phyinit_print_dat] pUserInputAdvanced->D4RxPreambleLength[0] = 1
//// [phyinit_print_dat] pUserInputAdvanced->D4RxPreambleLength[1] = 1
//// [phyinit_print_dat] pUserInputAdvanced->D4RxPreambleLength[2] = 1
//// [phyinit_print_dat] pUserInputAdvanced->D4RxPreambleLength[3] = 1
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrCtrlMode[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrCtrlMode[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrCtrlMode[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrCtrlMode[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewFallDQ[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewFallDQ[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewFallDQ[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxSlewFallDQ[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->NvAnibRcvSel[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->NvAnibRcvSel[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->NvAnibRcvSel[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->NvAnibRcvSel[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->NvAnibRcvSel[4] = 0
//// [phyinit_print_dat] pUserInputAdvanced->NvAnibRcvSel[5] = 0
//// [phyinit_print_dat] pUserInputAdvanced->NvAnibRcvSel[6] = 0
//// [phyinit_print_dat] pUserInputAdvanced->NvAnibRcvSel[7] = 0
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedanceCtrl1[0] = 25
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedanceCtrl1[1] = 25
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedanceCtrl1[2] = 25
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedanceCtrl1[3] = 25
//// [phyinit_print_dat] pUserInputAdvanced->Nibble_ECC = 15
//// [phyinit_print_dat] pUserInputAdvanced->D5DisableRetraining = 0
//// [phyinit_print_dat] pUserInputAdvanced->DqsOscRunTimeSel[0] = 256
//// [phyinit_print_dat] pUserInputAdvanced->DqsOscRunTimeSel[1] = 256
//// [phyinit_print_dat] pUserInputAdvanced->DqsOscRunTimeSel[2] = 256
//// [phyinit_print_dat] pUserInputAdvanced->DqsOscRunTimeSel[3] = 256
//// [phyinit_print_dat] pUserInputAdvanced->ODTImpedance[0] = 120
//// [phyinit_print_dat] pUserInputAdvanced->ODTImpedance[1] = 60
//// [phyinit_print_dat] pUserInputAdvanced->ODTImpedance[2] = 60
//// [phyinit_print_dat] pUserInputAdvanced->ODTImpedance[3] = 60
//// [phyinit_print_dat] pUserInputAdvanced->MtestPUImp = 240
//// [phyinit_print_dat] pUserInputAdvanced->EnableMAlertAsync = 0
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrMaxReqToAck[0] = 0
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrMaxReqToAck[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrMaxReqToAck[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->PhyMstrMaxReqToAck[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->RedundantCs_en = 0
//// [phyinit_print_dat] pUserInputAdvanced->CalInterval = 9
//// [phyinit_print_dat] pUserInputAdvanced->D5TxDqPreambleCtrl[0] = 1
//// [phyinit_print_dat] pUserInputAdvanced->D5TxDqPreambleCtrl[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->D5TxDqPreambleCtrl[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->D5TxDqPreambleCtrl[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->MemAlertEn = 0
//// [phyinit_print_dat] pUserInputAdvanced->ATxImpedance = 53247
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg2[0] = 1
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg2[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg2[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnTdqs2dqTrackingTg2[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->en_3DS = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnRxDqsTracking[0] = 1
//// [phyinit_print_dat] pUserInputAdvanced->EnRxDqsTracking[1] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnRxDqsTracking[2] = 0
//// [phyinit_print_dat] pUserInputAdvanced->EnRxDqsTracking[3] = 0
//// [phyinit_print_dat] pUserInputAdvanced->RstRxTrkState = 0
//// [phyinit_print_dat] pUserInputAdvanced->TrainSequenceCtrl = 33663
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedance[0] = 60
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedance[1] = 25
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedance[2] = 25
//// [phyinit_print_dat] pUserInputAdvanced->TxImpedance[3] = 25
//// [phyinit_print_dat] pUserInputSim->tDQS2DQ    = 750
//// [phyinit_print_dat] pUserInputSim->tDQSCK     = 0
//// [phyinit_print_dat] pUserInputSim->tSTAOFF[0] = 0
//// [phyinit_print_dat] pUserInputSim->tSTAOFF[1] = 0
//// [phyinit_print_dat] pUserInputSim->tSTAOFF[2] = 0
//// [phyinit_print_dat] pUserInputSim->tSTAOFF[3] = 0
//// [phyinit_print_dat] // ####################################################
//// [phyinit_print_dat] //
//// [phyinit_print_dat] // Printing values of 1D message block input/inout fields, PState=0
//// [phyinit_print_dat] //
//// [phyinit_print_dat] // ####################################################
//// [phyinit_print_dat] mb_DDR5U_1D[0].AdvTrainOpt = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MsgMisc = 0x7
//// [phyinit_print_dat] mb_DDR5U_1D[0].Pstate = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].PllBypassEn = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DRAMFreq = 0xc80
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW05_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW06_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RXEN_ADJ = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RX2D_DFE_Misc = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].PhyVref = 0x40
//// [phyinit_print_dat] mb_DDR5U_1D[0].D5Misc = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WL_ADJ = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].SequenceCtrl = 0x837f
//// [phyinit_print_dat] mb_DDR5U_1D[0].HdtCtrl = 0xc8
//// [phyinit_print_dat] mb_DDR5U_1D[0].PhyCfg = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFIMRLMargin = 0x2
//// [phyinit_print_dat] mb_DDR5U_1D[0].X16Present = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].UseBroadcastMR = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].D5Quickboot = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDbyte = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].CATrainOpt = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].TX2D_DFE_Misc = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RX2D_TrainOpt = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].TX2D_TrainOpt = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].Share2DVrefResult = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MRE_MIN_PULSE = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DWL_MIN_PULSE = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].PhyConfigOverride = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].EnabledDQsChA = 0x10
//// [phyinit_print_dat] mb_DDR5U_1D[0].CsPresentChA = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_A0 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_A0 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_A0 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_A0 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_A0 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A0 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A0 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_A0 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_A0 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_A0 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_A0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_A0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_A0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_A1 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_A1 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_A1 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_A1 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_A1 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A1 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A1 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_A1 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_A1 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_A1 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_A1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_A1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_A1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_A2 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_A2 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_A2 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_A2 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_A2 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A2 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A2 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_A2 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_A2 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_A2 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_A2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_A2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_A2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_A3 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_A3 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_A3 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_A3 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_A3 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A3 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A3 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_A3 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_A3 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_A3 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_A3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_A3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_A3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW04_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW05_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].EnabledDQsChB = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].CsPresentChB = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_B0 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_B0 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_B0 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_B0 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_B0 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B0 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B0 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_B0 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_B0 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_B0 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_B0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_B0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_B0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_B1 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_B1 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_B1 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_B1 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_B1 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B1 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B1 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_B1 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_B1 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_B1 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_B1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_B1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_B1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_B2 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_B2 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_B2 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_B2 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_B2 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B2 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B2 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_B2 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_B2 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_B2 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_B2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_B2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_B2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_B3 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_B3 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_B3 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_B3 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_B3 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B3 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B3 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_B3 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_B3 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_B3 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_B3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_B3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_B3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WL_ADJ_START = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WL_ADJ_END = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW00_ChA_D0 = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW01_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW02_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW03_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW04_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW05_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW06_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW07_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW08_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW09_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW10_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW11_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW12_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW13_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW14_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW15_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW16_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW17_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW18_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW19_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW20_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW21_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW22_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW23_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW24_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW25_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW26_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW27_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW28_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW29_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW30_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW31_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW32_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW33_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW34_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW35_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW36_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW37_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW38_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW39_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW40_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW41_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW42_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW43_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW44_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW45_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW46_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW47_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW48_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW49_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW50_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW51_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW52_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW53_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW54_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW55_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW56_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW57_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW58_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW59_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW60_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW61_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW62_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW63_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW64_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW65_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW66_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW67_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW68_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW69_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW70_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW71_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW72_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW73_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW74_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW75_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW76_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW77_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW78_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW79_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW00_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW01_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW02_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW03_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW04_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW05_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW06_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW07_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW08_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW09_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW10_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW11_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW12_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW13_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW14_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW15_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW16_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW17_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW18_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW19_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW20_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW21_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW22_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW23_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW24_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW25_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW26_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW27_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW28_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW29_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW30_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW31_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW32_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW33_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW34_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW35_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW36_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW37_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW38_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW39_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW40_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW41_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW42_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW43_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW44_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW45_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW46_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW47_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW48_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW49_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW50_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW51_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW52_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW53_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW54_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW55_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW56_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW57_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW58_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW59_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW60_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW61_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW62_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW63_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW64_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW65_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW66_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW67_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW68_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW69_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW70_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW71_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW72_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW73_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW74_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW75_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW76_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW77_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW78_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW79_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW00_ChA_D1 = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW01_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW02_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW03_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW04_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW05_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW06_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW07_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW08_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW09_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW10_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW11_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW12_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW13_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW14_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW15_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW16_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW17_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW18_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW19_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW20_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW21_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW22_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW23_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW24_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW25_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW26_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW27_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW28_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW29_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW30_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW31_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW32_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW33_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW34_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW35_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW36_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW37_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW38_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW39_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW40_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW41_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW42_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW43_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW44_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW45_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW46_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW47_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW48_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW49_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW50_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW51_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW52_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW53_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW54_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW55_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW56_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW57_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW58_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW59_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW60_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW61_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW62_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW63_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW64_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW65_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW66_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW67_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW68_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW69_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW70_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW71_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW72_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW73_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW74_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW75_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW76_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW77_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW78_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW79_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW00_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW01_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW02_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW03_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW04_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW05_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW06_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW07_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW08_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW09_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW10_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW11_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW12_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW13_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW14_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW15_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW16_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW17_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW18_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW19_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW20_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW21_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW22_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW23_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW24_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW25_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW26_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW27_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW28_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW29_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW30_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW31_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW32_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW33_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW34_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW35_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW36_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW37_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW38_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW39_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW40_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW41_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW42_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW43_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW44_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW45_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW46_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW47_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW48_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW49_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW50_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW51_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW52_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW53_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW54_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW55_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW56_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW57_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW58_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW59_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW60_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW61_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW62_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW63_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW64_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW65_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW66_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW67_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW68_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW69_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW70_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW71_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW72_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW73_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW74_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW75_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW76_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW77_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW78_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW79_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW00_ChB_D0 = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW01_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW02_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW03_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW04_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW05_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW06_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW07_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW08_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW09_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW10_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW11_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW12_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW13_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW14_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW15_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW16_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW17_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW18_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW19_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW20_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW21_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW22_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW23_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW24_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW25_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW26_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW27_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW28_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW29_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW30_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW31_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW32_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW33_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW34_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW35_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW36_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW37_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW38_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW39_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW40_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW41_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW42_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW43_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW44_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW45_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW46_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW47_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW48_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW49_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW50_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW51_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW52_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW53_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW54_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW55_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW56_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW57_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW58_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW59_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW60_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW61_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW62_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW63_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW64_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW65_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW66_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW67_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW68_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW69_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW70_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW71_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW72_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW73_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW74_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW75_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW76_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW77_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW78_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW79_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW00_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW01_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW02_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW03_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW04_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW05_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW06_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW07_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW08_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW09_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW10_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW11_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW12_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW13_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW14_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW15_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW16_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW17_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW18_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW19_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW20_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW21_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW22_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW23_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW24_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW25_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW26_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW27_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW28_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW29_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW30_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW31_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW32_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW33_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW34_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW35_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW36_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW37_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW38_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW39_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW40_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW41_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW42_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW43_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW44_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW45_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW46_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW47_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW48_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW49_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW50_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW51_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW52_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW53_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW54_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW55_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW56_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW57_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW58_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW59_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW60_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW61_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW62_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW63_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW64_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW65_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW66_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW67_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW68_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW69_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW70_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW71_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW72_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW73_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW74_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW75_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW76_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW77_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW78_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW79_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW00_ChB_D1 = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW01_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW02_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW03_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW04_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW05_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW06_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW07_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW08_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW09_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW10_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW11_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW12_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW13_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW14_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW15_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW16_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW17_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW18_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW19_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW20_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW21_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW22_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW23_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW24_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW25_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW26_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW27_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW28_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW29_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW30_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW31_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW32_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW33_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW34_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW35_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW36_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW37_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW38_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW39_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW40_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW41_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW42_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW43_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW44_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW45_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW46_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW47_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW48_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW49_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW50_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW51_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW52_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW53_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW54_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW55_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW56_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW57_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW58_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW59_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW60_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW61_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW62_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW63_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW64_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW65_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW66_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW67_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW68_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW69_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW70_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW71_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW72_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW73_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW74_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW75_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW76_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW77_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW78_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW79_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW00_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW01_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW02_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW03_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW04_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW05_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW06_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW07_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW08_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW09_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW10_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW11_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW12_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW13_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW14_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW15_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW16_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW17_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW18_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW19_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW20_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW21_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW22_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW23_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW24_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW25_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW26_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW27_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW28_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW29_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW30_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW31_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW32_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW33_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW34_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW35_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW36_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW37_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW38_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW39_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW40_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW41_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW42_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW43_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW44_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW45_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW46_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW47_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW48_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW49_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW50_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW51_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW52_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW53_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW54_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW55_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW56_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW57_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW58_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW59_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW60_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW61_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW62_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW63_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW64_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW65_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW66_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW67_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW68_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW69_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW70_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW71_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW72_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW73_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW74_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW75_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW76_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW77_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW78_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW79_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib0 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib1 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib2 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib3 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib4 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib5 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib6 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib7 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib8 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib9 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib10 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib11 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib12 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib13 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib14 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib15 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib16 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib17 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib18 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib19 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib0 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib1 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib2 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib3 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib4 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib5 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib6 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib7 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib8 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib9 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib10 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib11 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib12 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib13 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib14 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib15 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib16 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib17 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib18 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib19 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib0 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib1 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib2 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib3 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib4 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib5 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib6 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib7 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib8 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib9 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib10 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib11 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib12 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib13 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib14 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib15 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib16 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib17 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib18 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib19 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib0 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib1 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib2 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib3 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib4 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib5 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib6 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib7 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib8 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib9 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib10 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib11 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib12 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib13 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib14 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib15 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib16 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib17 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib18 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib19 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB0LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB1LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB2LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB3LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB4LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB5LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB6LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB7LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB8LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB9LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB0LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB1LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB2LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB3LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB4LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB5LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB6LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB7LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB8LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB9LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB0LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB1LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB2LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB3LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB4LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB5LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB6LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB7LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB8LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB9LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB0LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB1LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB2LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB3LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB4LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB5LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB6LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB7LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB8LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB9LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].AdvTrainOpt = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MsgMisc = 0x7
//// [phyinit_print_dat] mb_DDR5U_1D[0].Pstate = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].PllBypassEn = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DRAMFreq = 0xc80
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW05_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW06_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RXEN_ADJ = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RX2D_DFE_Misc = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].PhyVref = 0x40
//// [phyinit_print_dat] mb_DDR5U_1D[0].D5Misc = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WL_ADJ = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].SequenceCtrl = 0x837f
//// [phyinit_print_dat] mb_DDR5U_1D[0].HdtCtrl = 0xc8
//// [phyinit_print_dat] mb_DDR5U_1D[0].PhyCfg = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFIMRLMargin = 0x2
//// [phyinit_print_dat] mb_DDR5U_1D[0].X16Present = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].UseBroadcastMR = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].D5Quickboot = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDbyte = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].CATrainOpt = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].TX2D_DFE_Misc = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RX2D_TrainOpt = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].TX2D_TrainOpt = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].Share2DVrefResult = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MRE_MIN_PULSE = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DWL_MIN_PULSE = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].PhyConfigOverride = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].EnabledDQsChA = 0x10
//// [phyinit_print_dat] mb_DDR5U_1D[0].CsPresentChA = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_A0 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_A0 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_A0 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_A0 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_A0 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A0 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A0 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_A0 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_A0 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_A0 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_A0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_A0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_A0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_A1 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_A1 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_A1 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_A1 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_A1 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A1 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A1 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_A1 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_A1 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_A1 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_A1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_A1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_A1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_A2 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_A2 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_A2 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_A2 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_A2 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A2 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A2 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_A2 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_A2 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_A2 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_A2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_A2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_A2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_A3 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_A3 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_A3 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_A3 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_A3 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A3 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A3 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_A3 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_A3 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_A3 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_A3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_A3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_A3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_A3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW04_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW05_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_A0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_A1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_A2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_A3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].EnabledDQsChB = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].CsPresentChB = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_B0 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_B0 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_B0 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_B0 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_B0 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B0 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B0 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_B0 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_B0 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_B0 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_B0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_B0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_B0 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B0_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_B1 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_B1 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_B1 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_B1 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_B1 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B1 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B1 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_B1 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_B1 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_B1 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_B1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_B1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_B1 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B1_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_B2 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_B2 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_B2 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_B2 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_B2 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B2 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B2 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_B2 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_B2 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_B2 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_B2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_B2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_B2 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B2_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR0_B3 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR2_B3 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR4_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR5_B3 = 0x20
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR6_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR8_B3 = 0x8
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR10_B3 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B3 = 0x2d
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B3 = 0xd6
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR14_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR15_B3 = 0x3
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR111_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR34_B3 = 0x11
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR35_B3 = 0x4
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR32_ORG_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR37_B3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR38_B3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR39_B3 = 0x2c
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR11_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR12_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR13_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_ORG_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR33_B3_next = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR50_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR51_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR52_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DFE_GainBias_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_B0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_B1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_B2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WR_RD_RTT_PARK_B3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WL_ADJ_START = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].WL_ADJ_END = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW00_ChA_D0 = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW01_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW02_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW03_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW04_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW05_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW06_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW07_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW08_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW09_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW10_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW11_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW12_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW13_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW14_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW15_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW16_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW17_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW18_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW19_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW20_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW21_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW22_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW23_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW24_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW25_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW26_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW27_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW28_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW29_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW30_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW31_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW32_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW33_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW34_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW35_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW36_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW37_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW38_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW39_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW40_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW41_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW42_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW43_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW44_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW45_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW46_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW47_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW48_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW49_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW50_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW51_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW52_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW53_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW54_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW55_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW56_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW57_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW58_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW59_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW60_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW61_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW62_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW63_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW64_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW65_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW66_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW67_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW68_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW69_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW70_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW71_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW72_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW73_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW74_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW75_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW76_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW77_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW78_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW79_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW00_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW01_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW02_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW03_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW04_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW05_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW06_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW07_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW08_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW09_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW10_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW11_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW12_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW13_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW14_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW15_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW16_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW17_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW18_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW19_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW20_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW21_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW22_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW23_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW24_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW25_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW26_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW27_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW28_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW29_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW30_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW31_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW32_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW33_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW34_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW35_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW36_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW37_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW38_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW39_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW40_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW41_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW42_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW43_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW44_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW45_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW46_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW47_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW48_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW49_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW50_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW51_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW52_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW53_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW54_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW55_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW56_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW57_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW58_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW59_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW60_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW61_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW62_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW63_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW64_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW65_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW66_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW67_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW68_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW69_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW70_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW71_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW72_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW73_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW74_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW75_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW76_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW77_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW78_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW79_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7A_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7B_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7C_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7D_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7E_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7F_ChA_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW00_ChA_D1 = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW01_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW02_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW03_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW04_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW05_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW06_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW07_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW08_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW09_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW10_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW11_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW12_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW13_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW14_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW15_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW16_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW17_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW18_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW19_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW20_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW21_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW22_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW23_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW24_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW25_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW26_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW27_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW28_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW29_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW30_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW31_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW32_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW33_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW34_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW35_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW36_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW37_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW38_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW39_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW40_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW41_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW42_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW43_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW44_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW45_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW46_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW47_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW48_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW49_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW50_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW51_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW52_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW53_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW54_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW55_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW56_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW57_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW58_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW59_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW60_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW61_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW62_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW63_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW64_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW65_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW66_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW67_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW68_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW69_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW70_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW71_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW72_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW73_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW74_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW75_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW76_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW77_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW78_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW79_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW00_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW01_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW02_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW03_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW04_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW05_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW06_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW07_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW08_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW09_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW10_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW11_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW12_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW13_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW14_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW15_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW16_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW17_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW18_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW19_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW20_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW21_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW22_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW23_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW24_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW25_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW26_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW27_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW28_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW29_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW30_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW31_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW32_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW33_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW34_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW35_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW36_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW37_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW38_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW39_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW40_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW41_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW42_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW43_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW44_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW45_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW46_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW47_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW48_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW49_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW50_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW51_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW52_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW53_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW54_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW55_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW56_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW57_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW58_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW59_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW60_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW61_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW62_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW63_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW64_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW65_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW66_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW67_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW68_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW69_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW70_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW71_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW72_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW73_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW74_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW75_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW76_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW77_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW78_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW79_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7A_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7B_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7C_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7D_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7E_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7F_ChA_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW00_ChB_D0 = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW01_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW02_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW03_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW04_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW05_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW06_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW07_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW08_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW09_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW10_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW11_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW12_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW13_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW14_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW15_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW16_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW17_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW18_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW19_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW20_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW21_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW22_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW23_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW24_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW25_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW26_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW27_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW28_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW29_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW30_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW31_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW32_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW33_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW34_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW35_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW36_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW37_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW38_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW39_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW40_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW41_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW42_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW43_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW44_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW45_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW46_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW47_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW48_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW49_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW50_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW51_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW52_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW53_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW54_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW55_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW56_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW57_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW58_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW59_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW60_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW61_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW62_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW63_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW64_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW65_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW66_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW67_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW68_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW69_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW70_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW71_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW72_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW73_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW74_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW75_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW76_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW77_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW78_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW79_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW00_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW01_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW02_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW03_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW04_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW05_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW06_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW07_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW08_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW09_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW10_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW11_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW12_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW13_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW14_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW15_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW16_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW17_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW18_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW19_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW20_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW21_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW22_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW23_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW24_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW25_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW26_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW27_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW28_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW29_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW30_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW31_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW32_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW33_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW34_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW35_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW36_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW37_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW38_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW39_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW40_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW41_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW42_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW43_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW44_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW45_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW46_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW47_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW48_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW49_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW50_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW51_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW52_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW53_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW54_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW55_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW56_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW57_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW58_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW59_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW60_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW61_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW62_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW63_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW64_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW65_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW66_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW67_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW68_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW69_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW70_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW71_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW72_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW73_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW74_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW75_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW76_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW77_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW78_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW79_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7A_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7B_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7C_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7D_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7E_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7F_ChB_D0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW00_ChB_D1 = 0x1
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW01_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW02_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW03_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW04_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW05_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW06_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW07_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW08_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW09_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW0F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW10_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW11_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW12_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW13_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW14_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW15_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW16_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW17_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW18_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW19_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW1F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW20_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW21_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW22_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW23_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW24_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW25_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW26_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW27_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW28_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW29_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW2F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW30_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW31_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW32_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW33_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW34_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW35_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW36_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW37_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW38_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW39_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW3F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW40_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW41_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW42_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW43_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW44_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW45_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW46_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW47_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW48_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW49_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW4F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW50_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW51_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW52_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW53_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW54_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW55_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW56_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW57_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW58_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW59_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW5F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW60_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW61_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW62_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW63_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW64_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW65_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW66_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW67_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW68_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW69_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW6F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW70_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW71_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW72_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW73_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW74_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW75_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW76_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW77_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW78_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW79_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].RCW7F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW00_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW01_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW02_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW03_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW04_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW05_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW06_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW07_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW08_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW09_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW0F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW10_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW11_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW12_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW13_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW14_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW15_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW16_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW17_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW18_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW19_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW1F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW20_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW21_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW22_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW23_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW24_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW25_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW26_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW27_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW28_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW29_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW2F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW30_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW31_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW32_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW33_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW34_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW35_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW36_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW37_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW38_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW39_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW3F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW40_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW41_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW42_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW43_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW44_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW45_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW46_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW47_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW48_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW49_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW4F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW50_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW51_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW52_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW53_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW54_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW55_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW56_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW57_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW58_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW59_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW5F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW60_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW61_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW62_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW63_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW64_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW65_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW66_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW67_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW68_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW69_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW6F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW70_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW71_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW72_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW73_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW74_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW75_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW76_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW77_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW78_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW79_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7A_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7B_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7C_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7D_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7E_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].BCW7F_ChB_D1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib0 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib1 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib2 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib3 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib4 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib5 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib6 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib7 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib8 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib9 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib10 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib11 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib12 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib13 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib14 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib15 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib16 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib17 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib18 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR0Nib19 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib0 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib1 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib2 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib3 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib4 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib5 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib6 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib7 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib8 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib9 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib10 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib11 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib12 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib13 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib14 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib15 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib16 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib17 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib18 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR1Nib19 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib0 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib1 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib2 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib3 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib4 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib5 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib6 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib7 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib8 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib9 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib10 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib11 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib12 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib13 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib14 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib15 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib16 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib17 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib18 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR2Nib19 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib0 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib1 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib2 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib3 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib4 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib5 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib6 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib7 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib8 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib9 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib10 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib11 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib12 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib13 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib14 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib15 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib16 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib17 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib18 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefDqR3Nib19 = 0x17
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R0Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R1Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R2Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].MR3R3Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR0Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR1Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR2Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCSR3Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR0Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR1Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR2Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib4 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib5 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib6 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib7 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib8 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib9 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib10 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib11 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib12 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib13 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib14 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib15 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib16 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib17 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib18 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].VrefCAR3Nib19 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB0LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB1LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB2LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB3LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB4LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB5LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB6LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB7LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB8LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB9LaneR0 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB0LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB1LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB2LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB3LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB4LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB5LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB6LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB7LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB8LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB9LaneR1 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB0LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB1LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB2LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB3LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB4LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB5LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB6LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB7LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB8LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB9LaneR2 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB0LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB1LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB2LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB3LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB4LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB5LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB6LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB7LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB8LaneR3 = 0x0
//// [phyinit_print_dat] mb_DDR5U_1D[0].DisabledDB9LaneR3 = 0x0

////##############################################################
////
//// Step (A) : Bring up VDD, VDDQ, and VAA
////
//// The power supplies can come up and stabilize in any order.
//// While the power supplies are coming up, all outputs will be unknown and
//// the values of the inputs are don't cares.
////
////##############################################################

dwc_ddrphy_phyinit_userCustom_A_bringupPower();

//[dwc_ddrphy_phyinit_userCustom_A_bringupPower] End of dwc_ddrphy_phyinit_userCustom_A_bringupPower()
//
//
////##############################################################
////
//// 4.3.2(B) Start Clocks and Reset the PHY
////
//// Following is one possbile sequence to reset the PHY. Other sequences are also possible.
//// See section 5.2.2 of the PUB for other possible reset sequences.
////
//// 1. Drive PwrOkIn to 0. Note: Reset, DfiClk, and APBCLK can be X.
//// 2. Start DfiClk and APBCLK
//// 3. Drive Reset to 1 and PRESETn_APB to 0.
////    Note: The combination of PwrOkIn=0 and Reset=1 signals a cold reset to the PHY.
//// 4. Wait a minimum of 8 cycles.
//// 5. Drive PwrOkIn to 1. Once the PwrOkIn is asserted (and Reset is still asserted),
////    DfiClk synchronously switches to any legal input frequency.
//// 6. Wait a minimum of 64 cycles. Note: This is the reset period for the PHY.
//// 7. Drive Reset to 0. Note: All DFI and APB inputs must be driven at valid reset states before the deassertion of Reset.
//// 8. Wait a minimum of 1 Cycle.
//// 9. Drive PRESETn_APB to 1 to de-assert reset on the ABP bus.
////10. The PHY is now in the reset state and is ready to accept APB transactions.
////
////##############################################################
//
//
dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy(sdrammc);

//// [dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy] End of dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy()
//

////##############################################################
////
//// Step (C) Initialize PHY Configuration
////
//// Load the required PHY configuration registers for the appropriate mode and memory configuration
////
////##############################################################
//

//// [phyinit_C_initPhyConfig] Start of dwc_ddrphy_phyinit_C_initPhyConfig()
//// [phyinit_C_initPhyConfig] Programming ForceClkGaterEnables::ForcePubDxClkEnLow to 0x1
dwc_ddrphy_apb_wr(0x200a6, 0x2); // DWC_DDRPHYA_MASTER0_base0_ForceClkGaterEnables
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl3::VshCtrlUpdate to 0x0 for MASTER
dwc_ddrphy_apb_wr(0x20066, 0x0); // DWC_DDRPHYA_MASTER0_base0_VREGCtrl3
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl3::VshCtrlUpdate to 0x0 for all DBYTEs
dwc_ddrphy_apb_wr(0x10066, 0x0); // DWC_DDRPHYA_DBYTE0_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x11066, 0x0); // DWC_DDRPHYA_DBYTE1_base0_VREGCtrl3
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl3::VshCtrlUpdate to 0x0 for all ANIBs
dwc_ddrphy_apb_wr(0x66, 0x0); // DWC_DDRPHYA_ANIB0_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x1066, 0x0); // DWC_DDRPHYA_ANIB1_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x2066, 0x0); // DWC_DDRPHYA_ANIB2_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x3066, 0x0); // DWC_DDRPHYA_ANIB3_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x4066, 0x0); // DWC_DDRPHYA_ANIB4_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x5066, 0x0); // DWC_DDRPHYA_ANIB5_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x6066, 0x0); // DWC_DDRPHYA_ANIB6_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x7066, 0x0); // DWC_DDRPHYA_ANIB7_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x8066, 0x0); // DWC_DDRPHYA_ANIB8_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x9066, 0x0); // DWC_DDRPHYA_ANIB9_base0_VREGCtrl3
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl1::VshDAC to 0x16 for MASTER
dwc_ddrphy_apb_wr(0x20029, 0x58); // DWC_DDRPHYA_MASTER0_base0_VREGCtrl1_p0
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl1::VshDAC to 0x16 for all DBYTEs
dwc_ddrphy_apb_wr(0x10029, 0x58); // DWC_DDRPHYA_DBYTE0_base0_VREGCtrl1_p0
dwc_ddrphy_apb_wr(0x11029, 0x58); // DWC_DDRPHYA_DBYTE1_base0_VREGCtrl1_p0
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl1::VshDAC to 0x16 for all ANIBs
dwc_ddrphy_apb_wr(0x29, 0x58); // DWC_DDRPHYA_ANIB0_base0_VREGCtrl1_p0
dwc_ddrphy_apb_wr(0x1029, 0x58); // DWC_DDRPHYA_ANIB1_base0_VREGCtrl1_p0
dwc_ddrphy_apb_wr(0x2029, 0x58); // DWC_DDRPHYA_ANIB2_base0_VREGCtrl1_p0
dwc_ddrphy_apb_wr(0x3029, 0x58); // DWC_DDRPHYA_ANIB3_base0_VREGCtrl1_p0
dwc_ddrphy_apb_wr(0x4029, 0x58); // DWC_DDRPHYA_ANIB4_base0_VREGCtrl1_p0
dwc_ddrphy_apb_wr(0x5029, 0x58); // DWC_DDRPHYA_ANIB5_base0_VREGCtrl1_p0
dwc_ddrphy_apb_wr(0x6029, 0x58); // DWC_DDRPHYA_ANIB6_base0_VREGCtrl1_p0
dwc_ddrphy_apb_wr(0x7029, 0x58); // DWC_DDRPHYA_ANIB7_base0_VREGCtrl1_p0
dwc_ddrphy_apb_wr(0x8029, 0x58); // DWC_DDRPHYA_ANIB8_base0_VREGCtrl1_p0
dwc_ddrphy_apb_wr(0x9029, 0x58); // DWC_DDRPHYA_ANIB9_base0_VREGCtrl1_p0
dwc_ddrphy_apb_wr(0x90301, 0x59); // DWC_DDRPHYA_INITENG0_base0_Seq0BGPR1_p0
dwc_ddrphy_apb_wr(0x90302, 0x58); // DWC_DDRPHYA_INITENG0_base0_Seq0BGPR2_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TxSlewRate::CsrTxSrc to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TxSlewRate::TxPreDrvMode to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TxSlewRate to 0x0
//// [phyinit_C_initPhyConfig] ### NOTE ### Optimal setting for TxSlewRate::CsrTxSrc are technology specific.
//// [phyinit_C_initPhyConfig] ### NOTE ### Please consult the "Output Slew Rate" section of HSpice Model App Note in specific technology for recommended settings

dwc_ddrphy_apb_wr(0x1005f, 0x0); // DWC_DDRPHYA_DBYTE0_base0_TxSlewRate_b0_p0
dwc_ddrphy_apb_wr(0x1015f, 0x0); // DWC_DDRPHYA_DBYTE0_base0_TxSlewRate_b1_p0
dwc_ddrphy_apb_wr(0x1105f, 0x0); // DWC_DDRPHYA_DBYTE1_base0_TxSlewRate_b0_p0
dwc_ddrphy_apb_wr(0x1115f, 0x0); // DWC_DDRPHYA_DBYTE1_base0_TxSlewRate_b1_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate::ATxPreDrvMode to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate::CsrATxSrc ANIB 0 to 0x1be
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate ANIB 0 to 0x1be
dwc_ddrphy_apb_wr(0x55, 0x1be); // DWC_DDRPHYA_ANIB0_base0_ATxSlewRate_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate::CsrATxSrc ANIB 1 to 0x1be
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate ANIB 1 to 0x1be
dwc_ddrphy_apb_wr(0x1055, 0x1be); // DWC_DDRPHYA_ANIB1_base0_ATxSlewRate_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate::CsrATxSrc ANIB 2 to 0x1be
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate ANIB 2 to 0x1be
dwc_ddrphy_apb_wr(0x2055, 0x1be); // DWC_DDRPHYA_ANIB2_base0_ATxSlewRate_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate::CsrATxSrc ANIB 3 to 0x1be
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate ANIB 3 to 0x1be
dwc_ddrphy_apb_wr(0x3055, 0x1be); // DWC_DDRPHYA_ANIB3_base0_ATxSlewRate_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate::CsrATxSrc ANIB 4 to 0x1be
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate ANIB 4 to 0x1be
dwc_ddrphy_apb_wr(0x4055, 0x1be); // DWC_DDRPHYA_ANIB4_base0_ATxSlewRate_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate::CsrATxSrc ANIB 5 to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate ANIB 5 to 0x0
dwc_ddrphy_apb_wr(0x5055, 0x0); // DWC_DDRPHYA_ANIB5_base0_ATxSlewRate_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate::CsrATxSrc ANIB 6 to 0x1be
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate ANIB 6 to 0x1be
dwc_ddrphy_apb_wr(0x6055, 0x1be); // DWC_DDRPHYA_ANIB6_base0_ATxSlewRate_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate::CsrATxSrc ANIB 7 to 0x1be
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate ANIB 7 to 0x1be
dwc_ddrphy_apb_wr(0x7055, 0x1be); // DWC_DDRPHYA_ANIB7_base0_ATxSlewRate_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate::CsrATxSrc ANIB 8 to 0x1be
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate ANIB 8 to 0x1be
dwc_ddrphy_apb_wr(0x8055, 0x1be); // DWC_DDRPHYA_ANIB8_base0_ATxSlewRate_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate::CsrATxSrc ANIB 9 to 0x1be
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxSlewRate ANIB 9 to 0x1be
dwc_ddrphy_apb_wr(0x9055, 0x1be); // DWC_DDRPHYA_ANIB9_base0_ATxSlewRate_p0
//// [phyinit_C_initPhyConfig] ### NOTE ### Optimal setting for ATxSlewRate::CsrATxSrc are technology specific.
//// [phyinit_C_initPhyConfig] ### NOTE ### Please consult the "Output Slew Rate" section of HSpice Model App Note in specific technology for recommended settings

//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming CalPreDriverOverride::CsrTxOvSrc to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming CalPreDriverOverride::TxCalBaseN to 0x1
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming CalPreDriverOverride::TxCalBaseP to 0x1
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming CalPreDriverOverride to 0x300
//// [phyinit_C_initPhyConfig] ### NOTE ### Optimal setting for CalPreDriverOverride::CsrTxOvSrc are technology specific.
//// [phyinit_C_initPhyConfig] ### NOTE ### Please consult the "Output Slew Rate" section of HSpice Model App Note in specific technology for recommended settings

dwc_ddrphy_apb_wr(0x2008c, 0x300); // DWC_DDRPHYA_MASTER0_base0_CalPreDriverOverride
//// [phyinit_C_initPhyConfig] PUB revision is 0x0350.
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming PllCtrl2::PllFreqSel to 0x19 based on DfiClk frequency = 800.
dwc_ddrphy_apb_wr(0x200c5, 0x19); // DWC_DDRPHYA_MASTER0_base0_PllCtrl2_p0
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming PllCtrl1::PllCpPropCtrl to 0x3 based on DfiClk frequency = 800.
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming PllCtrl1::PllCpIntCtrl to 0x1 based on DfiClk frequency = 800.
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming PllCtrl1 to 0x61 based on DfiClk frequency = 800.
dwc_ddrphy_apb_wr(0x200c7, 0x21); // DWC_DDRPHYA_MASTER0_base0_PllCtrl1_p0
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming PllTestMode to 0x400f based on DfiClk frequency = 800.
dwc_ddrphy_apb_wr(0x200ca, 0x402f); // DWC_DDRPHYA_MASTER0_base0_PllTestMode_p0
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming PllCtrl4::PllCpPropGsCtrl to 0x6 based on DfiClk frequency = 800.
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming PllCtrl4::PllCpIntGsCtrl to 0x12 based on DfiClk frequency = 800.
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming PllCtrl4 to 0xd2 based on DfiClk frequency = 800.
dwc_ddrphy_apb_wr(0x200cc, 0x17f); // DWC_DDRPHYA_MASTER0_base0_PllCtrl4_p0
//// [phyinit_C_initPhyConfig] ### NOTE ### Optimal setting for PllCtrl1 and PllTestMode are technology specific.
//// [phyinit_C_initPhyConfig] ### NOTE ### Please consult technology specific PHY Databook for recommended settings

//
////##############################################################
////
//// Program ARdPtrInitVal based on Frequency and PLL Bypass inputs
//// The values programmed here assume ideal properties of DfiClk
//// and Pclk including:
//// - DfiClk skew
//// - DfiClk jitter
//// - DfiClk PVT variations
//// - Pclk skew
//// - Pclk jitter
////
//// PLL Bypassed mode:
////     For MemClk frequency > 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 2-5
////     For MemClk frequency < 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 1-5
////
//// PLL Enabled mode:
////     For MemClk frequency > 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 1-5
////     For MemClk frequency < 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 0-5
////
////##############################################################
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ARdPtrInitVal to 0x1
dwc_ddrphy_apb_wr(0x2002e, 0x1); // DWC_DDRPHYA_MASTER0_base0_ARdPtrInitVal_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DisPtrInitClrTxTracking to 0x0
dwc_ddrphy_apb_wr(0x20051, 0x0); // DWC_DDRPHYA_MASTER0_base0_PtrInitTrackingModeCntrl_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPreambleControl::TwoTckRxDqsPre  to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPreambleControl::TwoTckTxDqsPre  to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPreambleControl::PositionDfeInit to 0x2
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPreambleControl::DDR5RxPreamble  to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPreambleControl::DDR5RxPostamble to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPreambleControl                  to 0x88
dwc_ddrphy_apb_wr(0x20024, 0x88); // DWC_DDRPHYA_MASTER0_base0_DqsPreambleControl_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPreamblePattern::EnTxDqsPreamblePattern to 0x7
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPreamblePattern::TxDqsPreamblePattern to 0x1
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPreamblePattern to 0x701
dwc_ddrphy_apb_wr(0x200a1, 0x701); // DWC_DDRPHYA_MASTER0_base0_DqsPreamblePattern_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPostamblePattern::EnTxDqsPostamblePattern to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPostamblePattern::TxDqsPostamblePattern to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqsPostamblePattern to 0x0
dwc_ddrphy_apb_wr(0x200a2, 0x0); // DWC_DDRPHYA_MASTER0_base0_DqsPostamblePattern_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DmPreamblePattern::EnTxDmPreamblePattern to 0xf
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DmPreamblePattern::TxDmPreamblePattern to 0xf
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DmPreamblePattern to 0xf5
dwc_ddrphy_apb_wr(0x200fe, 0xf5); // DWC_DDRPHYA_MASTER0_base0_DmPreamblePattern_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqPreamblePatternU0::EnTxDqPreamblePatternU0 to 0xf
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqPreamblePatternU0::TxDqPreamblePatternU0 to 0xf
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqPreamblePatternU0 to 0xf5
dwc_ddrphy_apb_wr(0x200fc, 0xf5); // DWC_DDRPHYA_MASTER0_base0_DqPreamblePatternU0_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqPreamblePatternU1::EnTxDqPreamblePatternU1 to 0xf
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqPreamblePatternU1::TxDqPreamblePatternU1 to 0xf
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DqPreamblePatternU1 to 0xf5
dwc_ddrphy_apb_wr(0x200fd, 0xf5); // DWC_DDRPHYA_MASTER0_base0_DqPreamblePatternU1_p0
//// [phyinit_C_initPhyConfig] Programming DbyteDllModeCntrl::DllRxPreambleMode to 0x1
//// [phyinit_C_initPhyConfig] Programming DbyteDllModeCntrl::DllRxBurstLengthMode to 0x0
//// [phyinit_C_initPhyConfig] Programming DbyteDllModeCntrl to 0x2
dwc_ddrphy_apb_wr(0x2003a, 0x2); // DWC_DDRPHYA_MASTER0_base0_DbyteDllModeCntrl
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TxOdtDrvStren::TxOdtStrenPu to 0x4
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TxOdtDrvStren::TxOdtStrenPd to 0x0
dwc_ddrphy_apb_wr(0x1004d, 0x104); // DWC_DDRPHYA_DBYTE0_base0_TxOdtDrvStren_b0_p0
dwc_ddrphy_apb_wr(0x1014d, 0x104); // DWC_DDRPHYA_DBYTE0_base0_TxOdtDrvStren_b1_p0
dwc_ddrphy_apb_wr(0x1104d, 0x104); // DWC_DDRPHYA_DBYTE1_base0_TxOdtDrvStren_b0_p0
dwc_ddrphy_apb_wr(0x1114d, 0x104); // DWC_DDRPHYA_DBYTE1_base0_TxOdtDrvStren_b1_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxImpedance::ADrvStrenP to 0x3f
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxImpedance::ADrvStrenN to 0x3f
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxImpedance::ATxReserved13x12 to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxImpedance::ATxCalBaseN to 0x1
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming ATxImpedance::ATxCalBaseP to 0x1
dwc_ddrphy_apb_wr(0x43, 0xcfff); // DWC_DDRPHYA_ANIB0_base0_ATxImpedance
dwc_ddrphy_apb_wr(0x1043, 0xcfff); // DWC_DDRPHYA_ANIB1_base0_ATxImpedance
dwc_ddrphy_apb_wr(0x2043, 0xcfff); // DWC_DDRPHYA_ANIB2_base0_ATxImpedance
dwc_ddrphy_apb_wr(0x3043, 0xcfff); // DWC_DDRPHYA_ANIB3_base0_ATxImpedance
dwc_ddrphy_apb_wr(0x4043, 0xcfff); // DWC_DDRPHYA_ANIB4_base0_ATxImpedance
dwc_ddrphy_apb_wr(0x5043, 0xcfff); // DWC_DDRPHYA_ANIB5_base0_ATxImpedance
dwc_ddrphy_apb_wr(0x6043, 0xcfff); // DWC_DDRPHYA_ANIB6_base0_ATxImpedance
dwc_ddrphy_apb_wr(0x7043, 0xcfff); // DWC_DDRPHYA_ANIB7_base0_ATxImpedance
dwc_ddrphy_apb_wr(0x8043, 0xcfff); // DWC_DDRPHYA_ANIB8_base0_ATxImpedance
dwc_ddrphy_apb_wr(0x9043, 0xcfff); // DWC_DDRPHYA_ANIB9_base0_ATxImpedance
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TxImpedanceCtrl0::TxStrenEqHiPu to 0xc
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TxImpedanceCtrl0::TxStrenEqLoPd to 0xc
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TxImpedanceCtrl1::TxStrenPu to 0x3f
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TxImpedanceCtrl1::TxStrenPd to 0x3f
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TxImpedanceCtrl2::TxStrenEqLoPu to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TxImpedanceCtrl2::TxStrenEqHiPd to 0x0
dwc_ddrphy_apb_wr(0x10041, 0x30c); // DWC_DDRPHYA_DBYTE0_base0_TxImpedanceCtrl0_b0_p0
dwc_ddrphy_apb_wr(0x10049, 0x79e); // DWC_DDRPHYA_DBYTE0_base0_TxImpedanceCtrl1_b0_p0
dwc_ddrphy_apb_wr(0x1004b, 0x0); // DWC_DDRPHYA_DBYTE0_base0_TxImpedanceCtrl2_b0_p0
dwc_ddrphy_apb_wr(0x10141, 0x30c); // DWC_DDRPHYA_DBYTE0_base0_TxImpedanceCtrl0_b1_p0
dwc_ddrphy_apb_wr(0x10149, 0x79e); // DWC_DDRPHYA_DBYTE0_base0_TxImpedanceCtrl1_b1_p0
dwc_ddrphy_apb_wr(0x1014b, 0x0); // DWC_DDRPHYA_DBYTE0_base0_TxImpedanceCtrl2_b1_p0
dwc_ddrphy_apb_wr(0x11041, 0x30c); // DWC_DDRPHYA_DBYTE1_base0_TxImpedanceCtrl0_b0_p0
dwc_ddrphy_apb_wr(0x11049, 0x79e); // DWC_DDRPHYA_DBYTE1_base0_TxImpedanceCtrl1_b0_p0
dwc_ddrphy_apb_wr(0x1104b, 0x0); // DWC_DDRPHYA_DBYTE1_base0_TxImpedanceCtrl2_b0_p0
dwc_ddrphy_apb_wr(0x11141, 0x30c); // DWC_DDRPHYA_DBYTE1_base0_TxImpedanceCtrl0_b1_p0
dwc_ddrphy_apb_wr(0x11149, 0x79e); // DWC_DDRPHYA_DBYTE1_base0_TxImpedanceCtrl1_b1_p0
dwc_ddrphy_apb_wr(0x1114b, 0x0); // DWC_DDRPHYA_DBYTE1_base0_TxImpedanceCtrl2_b1_p0
//// [phyinit_C_initPhyConfig] Programming DfiMode to 0x1
dwc_ddrphy_apb_wr(0x20018, 0x1); // DWC_DDRPHYA_MASTER0_base0_DfiMode
//// [phyinit_C_initPhyConfig] Programming DfiCAMode to 0x10
dwc_ddrphy_apb_wr(0x20075, 0x10); // DWC_DDRPHYA_MASTER0_base0_DfiCAMode
//// [phyinit_C_initPhyConfig] Programming CalDrvStr0::CalDrvStrPd50 to 0x2
//// [phyinit_C_initPhyConfig] Programming CalDrvStr0::CalDrvStrPu50 to 0x2
dwc_ddrphy_apb_wr(0x20050, 0x82); // DWC_DDRPHYA_MASTER0_base0_CalDrvStr0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming CalUclkInfo::CalUClkTicksPer1uS to 0x320
dwc_ddrphy_apb_wr(0x20008, 0x320); // DWC_DDRPHYA_MASTER0_base0_CalUclkInfo_p0
//// [phyinit_C_initPhyConfig] Programming CalRate::CalInterval to 0x9
//// [phyinit_C_initPhyConfig] Programming CalRate::CalOnce to 0x0
dwc_ddrphy_apb_wr(0x20088, 0x9); // DWC_DDRPHYA_MASTER0_base0_CalRate
//// [phyinit_C_initPhyConfig] Pstate=0, Programming VrefInGlobal::GlobalVrefInSel to 0x0
//// [phyinit_C_initPhyConfig] Pstate=0, Programming VrefInGlobal::GlobalVrefInDAC to 0x1f
//// [phyinit_C_initPhyConfig] Pstate=0, Programming VrefInGlobal to 0xf8
dwc_ddrphy_apb_wr(0x200b2, 0xf8); // DWC_DDRPHYA_MASTER0_base0_VrefInGlobal_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Programming DqDqsRcvCntrl (Byte=0, Upper/Lower=0) to 0x2900
dwc_ddrphy_apb_wr(0x10043, 0x2900); // DWC_DDRPHYA_DBYTE0_base0_DqDqsRcvCntrl_b0_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Programming DqDqsRcvCntrl (Byte=0, Upper/Lower=1) to 0x2900
dwc_ddrphy_apb_wr(0x10143, 0x2900); // DWC_DDRPHYA_DBYTE0_base0_DqDqsRcvCntrl_b1_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Programming DqDqsRcvCntrl (Byte=1, Upper/Lower=0) to 0x2900
dwc_ddrphy_apb_wr(0x11043, 0x2900); // DWC_DDRPHYA_DBYTE1_base0_DqDqsRcvCntrl_b0_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Programming DqDqsRcvCntrl (Byte=1, Upper/Lower=1) to 0x2900
dwc_ddrphy_apb_wr(0x11143, 0x2900); // DWC_DDRPHYA_DBYTE1_base0_DqDqsRcvCntrl_b1_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Programming DqDqsRcvCntrl2 to 0x1c
dwc_ddrphy_apb_wr(0x1004c, 0x1c); // DWC_DDRPHYA_DBYTE0_base0_DqDqsRcvCntrl2_p0
dwc_ddrphy_apb_wr(0x1104c, 0x1c); // DWC_DDRPHYA_DBYTE1_base0_DqDqsRcvCntrl2_p0
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TristateModeCA::DisDynAdrTri_p0 to 0x1
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming TristateModeCA::DDR2TMode_p0 to 0x0
dwc_ddrphy_apb_wr(0x20019, 0x5); // DWC_DDRPHYA_MASTER0_base0_TristateModeCA_p0
//// [phyinit_C_initPhyConfig] Programming DfiFreqXlat*
dwc_ddrphy_apb_wr(0x200f0, 0x0); // DWC_DDRPHYA_MASTER0_base0_DfiFreqXlat0
dwc_ddrphy_apb_wr(0x200f1, 0x0); // DWC_DDRPHYA_MASTER0_base0_DfiFreqXlat1
dwc_ddrphy_apb_wr(0x200f2, 0x4444); // DWC_DDRPHYA_MASTER0_base0_DfiFreqXlat2
dwc_ddrphy_apb_wr(0x200f3, 0x8888); // DWC_DDRPHYA_MASTER0_base0_DfiFreqXlat3
dwc_ddrphy_apb_wr(0x200f4, 0x5555); // DWC_DDRPHYA_MASTER0_base0_DfiFreqXlat4
dwc_ddrphy_apb_wr(0x200f5, 0x0); // DWC_DDRPHYA_MASTER0_base0_DfiFreqXlat5
dwc_ddrphy_apb_wr(0x200f6, 0x0); // DWC_DDRPHYA_MASTER0_base0_DfiFreqXlat6
dwc_ddrphy_apb_wr(0x200f7, 0xf000); // DWC_DDRPHYA_MASTER0_base0_DfiFreqXlat7
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming Seq0BDLY0 to 0x64
dwc_ddrphy_apb_wr(0x2000b, 0x64); // DWC_DDRPHYA_MASTER0_base0_Seq0BDLY0_p0
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming Seq0BDLY1 to 0xc8
dwc_ddrphy_apb_wr(0x2000c, 0xc8); // DWC_DDRPHYA_MASTER0_base0_Seq0BDLY1_p0
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming Seq0BDLY2 to 0x2bc
dwc_ddrphy_apb_wr(0x2000d, 0x2bc); // DWC_DDRPHYA_MASTER0_base0_Seq0BDLY2_p0
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming Seq0BDLY3 to 0x2c
dwc_ddrphy_apb_wr(0x2000e, 0x2c); // DWC_DDRPHYA_MASTER0_base0_Seq0BDLY3_p0
//// [phyinit_C_initPhyConfig] Disabling DBYTE 0 Lane 8 (DBI) Receiver to save power.
dwc_ddrphy_apb_wr(0x1004a, 0x500); // DWC_DDRPHYA_DBYTE0_base0_DqDqsRcvCntrl1
//// [phyinit_C_initPhyConfig] Disabling DBYTE 1 Lane 8 (DBI) Receiver to save power.
dwc_ddrphy_apb_wr(0x1104a, 0x500); // DWC_DDRPHYA_DBYTE1_base0_DqDqsRcvCntrl1
//// [phyinit_C_initPhyConfig] Programming MasterX4Config::X4TG to 0x0
dwc_ddrphy_apb_wr(0x20025, 0x0); // DWC_DDRPHYA_MASTER0_base0_MasterX4Config
// [phyinit_C_initPhyConfig] Programming DfiDataEnLatency::WLm13 and RLm13
dwc_ddrphy_apb_wr(0x2019a, 0x18); // DWC_DDRPHYA_MASTER0_base0_DfiDataEnLatency
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, rd_Crc = 0 cwl= 24 , cl = 26 mr_cl =2 MR0_A0 = 0x8
dwc_ddrphy_apb_wr(0x400f5, 0x1200); // DWC_DDRPHYA_ACSM0_base0_AcsmCtrl5_p0
dwc_ddrphy_apb_wr(0x400f6, 0x10); // DWC_DDRPHYA_ACSM0_base0_AcsmCtrl6_p0
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM0RxEnPulse to 2062
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM0RxValPulse to 2062
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM0RdcsPulse to 2062
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM0TxEnPulse to 2060
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM0WrcsPulse to 2060
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM0SnoopPulse to 2062
dwc_ddrphy_apb_wr(0x20120, 0x80e); // DWC_DDRPHYA_MASTER0_base0_D5ACSM0RxEnPulse_p0
dwc_ddrphy_apb_wr(0x20121, 0x80e); // DWC_DDRPHYA_MASTER0_base0_D5ACSM0RxValPulse_p0
dwc_ddrphy_apb_wr(0x20124, 0x80e); // DWC_DDRPHYA_MASTER0_base0_D5ACSM0RdcsPulse_p0
dwc_ddrphy_apb_wr(0x20122, 0x80c); // DWC_DDRPHYA_MASTER0_base0_D5ACSM0TxEnPulse_p0
dwc_ddrphy_apb_wr(0x20123, 0x80c); // DWC_DDRPHYA_MASTER0_base0_D5ACSM0WrcsPulse_p0
dwc_ddrphy_apb_wr(0x20125, 0x80e); // DWC_DDRPHYA_MASTER0_base0_D5ACSM0SnoopPulse_p0
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM0SnoopVal to 801
dwc_ddrphy_apb_wr(0x2012e, 0x321); // DWC_DDRPHYA_MASTER0_base0_D5ACSM0SnoopVal
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM1RxEnPulse to 2062
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM1RxValPulse to 2062
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM1RdcsPulse to 2062
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM1TxEnPulse to 2060
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM1WrcsPulse to 2060
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM1SnoopPulse to 2062
dwc_ddrphy_apb_wr(0x20140, 0x80e); // DWC_DDRPHYA_MASTER0_base0_D5ACSM1RxEnPulse_p0
dwc_ddrphy_apb_wr(0x20141, 0x80e); // DWC_DDRPHYA_MASTER0_base0_D5ACSM1RxValPulse_p0
dwc_ddrphy_apb_wr(0x20144, 0x80e); // DWC_DDRPHYA_MASTER0_base0_D5ACSM1RdcsPulse_p0
dwc_ddrphy_apb_wr(0x20142, 0x80c); // DWC_DDRPHYA_MASTER0_base0_D5ACSM1TxEnPulse_p0
dwc_ddrphy_apb_wr(0x20143, 0x80c); // DWC_DDRPHYA_MASTER0_base0_D5ACSM1WrcsPulse_p0
dwc_ddrphy_apb_wr(0x20145, 0x80e); // DWC_DDRPHYA_MASTER0_base0_D5ACSM1SnoopPulse_p0
//// [phyinit_C_initPhyConfig] Pstate=0,  Memclk=1600MHz, Programming D5ACSM1SnoopVal to 801
dwc_ddrphy_apb_wr(0x2014e, 0x321); // DWC_DDRPHYA_MASTER0_base0_D5ACSM1SnoopVal
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming GPR7(csrAlertRecovery) to 0x0
dwc_ddrphy_apb_wr(0x90307, 0x0); // DWC_DDRPHYA_INITENG0_base0_Seq0BGPR7_p0
// [phyinit_C_initPhyConfig] Programming TimingModeCntrl::Dly64Prec to 0x1
dwc_ddrphy_apb_wr(0x20040, 0x1); // DWC_DDRPHYA_MASTER0_base0_TimingModeCntrl
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl3::VshCtrlUpdate to 0x1 for MASTER
dwc_ddrphy_apb_wr(0x20066, 0x1); // DWC_DDRPHYA_MASTER0_base0_VREGCtrl3
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl3::VshCtrlUpdate to 0x1 for all DBYTEs
dwc_ddrphy_apb_wr(0x10066, 0x1); // DWC_DDRPHYA_DBYTE0_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x11066, 0x1); // DWC_DDRPHYA_DBYTE1_base0_VREGCtrl3
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl3::VshCtrlUpdate to 0x1 for all ANIBs
dwc_ddrphy_apb_wr(0x66, 0x1); // DWC_DDRPHYA_ANIB0_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x1066, 0x1); // DWC_DDRPHYA_ANIB1_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x2066, 0x1); // DWC_DDRPHYA_ANIB2_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x3066, 0x1); // DWC_DDRPHYA_ANIB3_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x4066, 0x1); // DWC_DDRPHYA_ANIB4_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x5066, 0x1); // DWC_DDRPHYA_ANIB5_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x6066, 0x1); // DWC_DDRPHYA_ANIB6_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x7066, 0x1); // DWC_DDRPHYA_ANIB7_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x8066, 0x1); // DWC_DDRPHYA_ANIB8_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x9066, 0x1); // DWC_DDRPHYA_ANIB9_base0_VREGCtrl3
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming AcClkDLLControl to 0x1080
dwc_ddrphy_apb_wr(0x200ea, 0x1080); // DWC_DDRPHYA_MASTER0_base0_AcClkDLLControl_p0
// [phyinit_C_initPhyConfig] Programming ArcPmuEccCtl to 0x1
dwc_ddrphy_apb_wr(0xc0086, 0x1); // DWC_DDRPHYA_DRTUB0_ArcPmuEccCtl
// [phyinit_C_initPhyConfig] Programming VREGCtrl2 to 0x9820 for MASTER
dwc_ddrphy_apb_wr(0x2002b, 0x9820); // DWC_DDRPHYA_MASTER0_base0_VREGCtrl2
// [phyinit_C_initPhyConfig] Programming VREGCtrl2 to 0x8020 for all DBYTEs
dwc_ddrphy_apb_wr(0x1002b, 0x8020); // DWC_DDRPHYA_DBYTE0_base0_VREGCtrl2
dwc_ddrphy_apb_wr(0x1102b, 0x8020); // DWC_DDRPHYA_DBYTE1_base0_VREGCtrl2
// [phyinit_C_initPhyConfig] Programming VREGCtrl2 to 0x8020 for all ANIBs
dwc_ddrphy_apb_wr(0x2b, 0x8020); // DWC_DDRPHYA_ANIB0_base0_VREGCtrl2
dwc_ddrphy_apb_wr(0x102b, 0x8020); // DWC_DDRPHYA_ANIB1_base0_VREGCtrl2
dwc_ddrphy_apb_wr(0x202b, 0x8020); // DWC_DDRPHYA_ANIB2_base0_VREGCtrl2
dwc_ddrphy_apb_wr(0x302b, 0x8020); // DWC_DDRPHYA_ANIB3_base0_VREGCtrl2
dwc_ddrphy_apb_wr(0x402b, 0x8020); // DWC_DDRPHYA_ANIB4_base0_VREGCtrl2
dwc_ddrphy_apb_wr(0x502b, 0x8020); // DWC_DDRPHYA_ANIB5_base0_VREGCtrl2
dwc_ddrphy_apb_wr(0x602b, 0x8020); // DWC_DDRPHYA_ANIB6_base0_VREGCtrl2
dwc_ddrphy_apb_wr(0x702b, 0x8020); // DWC_DDRPHYA_ANIB7_base0_VREGCtrl2
dwc_ddrphy_apb_wr(0x802b, 0x8020); // DWC_DDRPHYA_ANIB8_base0_VREGCtrl2
dwc_ddrphy_apb_wr(0x902b, 0x8020); // DWC_DDRPHYA_ANIB9_base0_VREGCtrl2
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl3::VshCtrlUpdate to 0x0 for MASTER
dwc_ddrphy_apb_wr(0x20066, 0x0); // DWC_DDRPHYA_MASTER0_base0_VREGCtrl3
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl3::VshCtrlUpdate to 0x0 for all DBYTEs
dwc_ddrphy_apb_wr(0x10066, 0x0); // DWC_DDRPHYA_DBYTE0_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x11066, 0x0); // DWC_DDRPHYA_DBYTE1_base0_VREGCtrl3
// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming VREGCtrl3::VshCtrlUpdate to 0x0 for all ANIBs
dwc_ddrphy_apb_wr(0x66, 0x0); // DWC_DDRPHYA_ANIB0_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x1066, 0x0); // DWC_DDRPHYA_ANIB1_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x2066, 0x0); // DWC_DDRPHYA_ANIB2_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x3066, 0x0); // DWC_DDRPHYA_ANIB3_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x4066, 0x0); // DWC_DDRPHYA_ANIB4_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x5066, 0x0); // DWC_DDRPHYA_ANIB5_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x6066, 0x0); // DWC_DDRPHYA_ANIB6_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x7066, 0x0); // DWC_DDRPHYA_ANIB7_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x8066, 0x0); // DWC_DDRPHYA_ANIB8_base0_VREGCtrl3
dwc_ddrphy_apb_wr(0x9066, 0x0); // DWC_DDRPHYA_ANIB9_base0_VREGCtrl3
// [phyinit_C_initPhyConfig] Programming VrefDAC0 to 0x3f for all DBYTEs and lanes
// [phyinit_C_initPhyConfig] Programming VrefDAC1 to 0x3f for all DBYTEs and lanes
// [phyinit_C_initPhyConfig] Programming VrefDAC2 to 0x3f for all DBYTEs and lanes
// [phyinit_C_initPhyConfig] Programming VrefDAC3 to 0x3f for all DBYTEs and lanes
dwc_ddrphy_apb_wr(0x10040, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC0_r0_p0
dwc_ddrphy_apb_wr(0x10030, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC1_r0
dwc_ddrphy_apb_wr(0x10050, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC2_r0
dwc_ddrphy_apb_wr(0x10060, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC3_r0
dwc_ddrphy_apb_wr(0x10140, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC0_r1_p0
dwc_ddrphy_apb_wr(0x10130, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC1_r1
dwc_ddrphy_apb_wr(0x10150, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC2_r1
dwc_ddrphy_apb_wr(0x10160, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC3_r1
dwc_ddrphy_apb_wr(0x10240, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC0_r2_p0
dwc_ddrphy_apb_wr(0x10230, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC1_r2
dwc_ddrphy_apb_wr(0x10250, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC2_r2
dwc_ddrphy_apb_wr(0x10260, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC3_r2
dwc_ddrphy_apb_wr(0x10340, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC0_r3_p0
dwc_ddrphy_apb_wr(0x10330, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC1_r3
dwc_ddrphy_apb_wr(0x10350, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC2_r3
dwc_ddrphy_apb_wr(0x10360, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC3_r3
dwc_ddrphy_apb_wr(0x10440, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC0_r4_p0
dwc_ddrphy_apb_wr(0x10430, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC1_r4
dwc_ddrphy_apb_wr(0x10450, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC2_r4
dwc_ddrphy_apb_wr(0x10460, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC3_r4
dwc_ddrphy_apb_wr(0x10540, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC0_r5_p0
dwc_ddrphy_apb_wr(0x10530, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC1_r5
dwc_ddrphy_apb_wr(0x10550, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC2_r5
dwc_ddrphy_apb_wr(0x10560, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC3_r5
dwc_ddrphy_apb_wr(0x10640, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC0_r6_p0
dwc_ddrphy_apb_wr(0x10630, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC1_r6
dwc_ddrphy_apb_wr(0x10650, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC2_r6
dwc_ddrphy_apb_wr(0x10660, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC3_r6
dwc_ddrphy_apb_wr(0x10740, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC0_r7_p0
dwc_ddrphy_apb_wr(0x10730, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC1_r7
dwc_ddrphy_apb_wr(0x10750, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC2_r7
dwc_ddrphy_apb_wr(0x10760, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC3_r7
dwc_ddrphy_apb_wr(0x10840, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC0_r8_p0
dwc_ddrphy_apb_wr(0x10830, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC1_r8
dwc_ddrphy_apb_wr(0x10850, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC2_r8
dwc_ddrphy_apb_wr(0x10860, 0x3f); // DWC_DDRPHYA_DBYTE0_base0_VrefDAC3_r8
dwc_ddrphy_apb_wr(0x11040, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC0_r0_p0
dwc_ddrphy_apb_wr(0x11030, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC1_r0
dwc_ddrphy_apb_wr(0x11050, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC2_r0
dwc_ddrphy_apb_wr(0x11060, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC3_r0
dwc_ddrphy_apb_wr(0x11140, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC0_r1_p0
dwc_ddrphy_apb_wr(0x11130, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC1_r1
dwc_ddrphy_apb_wr(0x11150, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC2_r1
dwc_ddrphy_apb_wr(0x11160, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC3_r1
dwc_ddrphy_apb_wr(0x11240, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC0_r2_p0
dwc_ddrphy_apb_wr(0x11230, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC1_r2
dwc_ddrphy_apb_wr(0x11250, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC2_r2
dwc_ddrphy_apb_wr(0x11260, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC3_r2
dwc_ddrphy_apb_wr(0x11340, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC0_r3_p0
dwc_ddrphy_apb_wr(0x11330, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC1_r3
dwc_ddrphy_apb_wr(0x11350, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC2_r3
dwc_ddrphy_apb_wr(0x11360, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC3_r3
dwc_ddrphy_apb_wr(0x11440, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC0_r4_p0
dwc_ddrphy_apb_wr(0x11430, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC1_r4
dwc_ddrphy_apb_wr(0x11450, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC2_r4
dwc_ddrphy_apb_wr(0x11460, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC3_r4
dwc_ddrphy_apb_wr(0x11540, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC0_r5_p0
dwc_ddrphy_apb_wr(0x11530, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC1_r5
dwc_ddrphy_apb_wr(0x11550, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC2_r5
dwc_ddrphy_apb_wr(0x11560, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC3_r5
dwc_ddrphy_apb_wr(0x11640, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC0_r6_p0
dwc_ddrphy_apb_wr(0x11630, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC1_r6
dwc_ddrphy_apb_wr(0x11650, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC2_r6
dwc_ddrphy_apb_wr(0x11660, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC3_r6
dwc_ddrphy_apb_wr(0x11740, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC0_r7_p0
dwc_ddrphy_apb_wr(0x11730, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC1_r7
dwc_ddrphy_apb_wr(0x11750, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC2_r7
dwc_ddrphy_apb_wr(0x11760, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC3_r7
dwc_ddrphy_apb_wr(0x11840, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC0_r8_p0
dwc_ddrphy_apb_wr(0x11830, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC1_r8
dwc_ddrphy_apb_wr(0x11850, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC2_r8
dwc_ddrphy_apb_wr(0x11860, 0x3f); // DWC_DDRPHYA_DBYTE1_base0_VrefDAC3_r8
//// [phyinit_C_initPhyConfig] Pstate=0, Memclk=1600MHz, Programming DfiFreqRatio_p0 to 0x1
dwc_ddrphy_apb_wr(0x200fa, 0x1); // DWC_DDRPHYA_MASTER0_base0_DfiFreqRatio_p0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::DOCByteSelTg0 (dbyte=0) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::DOCByteSelTg1 (dbyte=0) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::DOCByteSelTg2 (dbyte=0) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::DOCByteSelTg3 (dbyte=0) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::NoX4onUpperNibbleTg0 (dbyte=0) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::NoX4onUpperNibbleTg1 (dbyte=0) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::NoX4onUpperNibbleTg2 (dbyte=0) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::NoX4onUpperNibbleTg3 (dbyte=0) to 0x0
dwc_ddrphy_apb_wr(0x100aa, 0x0); // DWC_DDRPHYA_DBYTE0_base0_PptCtlStatic
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::DOCByteSelTg0 (dbyte=1) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::DOCByteSelTg1 (dbyte=1) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::DOCByteSelTg2 (dbyte=1) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::DOCByteSelTg3 (dbyte=1) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::NoX4onUpperNibbleTg0 (dbyte=1) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::NoX4onUpperNibbleTg1 (dbyte=1) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::NoX4onUpperNibbleTg2 (dbyte=1) to 0x0
//// [phyinit_C_initPhyConfig] Programming PptCtlStatic::NoX4onUpperNibbleTg3 (dbyte=1) to 0x0
dwc_ddrphy_apb_wr(0x110aa, 0x0); // DWC_DDRPHYA_DBYTE1_base0_PptCtlStatic
//// [phyinit_C_initPhyConfig] Programming ForceClkGaterEnables::ForcePubDxClkEnLow to 0x0
dwc_ddrphy_apb_wr(0x200a6, 0x0); // DWC_DDRPHYA_MASTER0_base0_ForceClkGaterEnables
//// [phyinit_C_initPhyConfig] Programming AForceTriCont (anib=0) to 0xc
//// [phyinit_C_initPhyConfig] Programming AForceTriCont (anib=4) to 0x8
//// [phyinit_C_initPhyConfig] Programming AForceTriCont (anib=6) to 0xf
//// [phyinit_C_initPhyConfig] Programming AForceTriCont (anib=7) to 0xf
//// [phyinit_C_initPhyConfig] Programming AForceTriCont (anib=8) to 0xf
//// [phyinit_C_initPhyConfig] Programming AForceTriCont (anib=9) to 0xf
dwc_ddrphy_apb_wr(0x28, 0xc); // DWC_DDRPHYA_ANIB0_base0_AForceTriCont
dwc_ddrphy_apb_wr(0x4028, 0x8); // DWC_DDRPHYA_ANIB4_base0_AForceTriCont
dwc_ddrphy_apb_wr(0x6028, 0xf); // DWC_DDRPHYA_ANIB6_base0_AForceTriCont
dwc_ddrphy_apb_wr(0x7028, 0xf); // DWC_DDRPHYA_ANIB7_base0_AForceTriCont
dwc_ddrphy_apb_wr(0x8028, 0xf); // DWC_DDRPHYA_ANIB8_base0_AForceTriCont
dwc_ddrphy_apb_wr(0x9028, 0xf); // DWC_DDRPHYA_ANIB9_base0_AForceTriCont
//// [phyinit_C_initPhyConfig] End of dwc_ddrphy_phyinit_C_initPhyConfig()
//
//
////##############################################################
////
//// dwc_ddrphy_phyinit_userCustom_customPreTrain is a user-editable function.
////
//// The purpose of dwc_ddrphy_phyinit_userCustom_customPreTrain() is to override any
//// any message block fields calculated by Phyinit in dwc_ddrphy_phyinit_calcMb() or to
//// override any CSR values programmed by Phyinit in dwc_ddrphy_phyinit_C_initPhyConfig().
//// This function is executed before training and thus any override here might affect
//// training result.
////
//// IMPORTANT: in this function, user shall not override any values in userInputBasic and
//// userInputAdvanced data structures. Use dwc_ddrphy_phyinit_userCustom_overrideUserInput()
//// to modify values in those data structures.
////
////##############################################################
//
//// [phyinit_userCustom_customPreTrain] Start of dwc_ddrphy_phyinit_userCustom_customPreTrain()
//// [phyinit_userCustom_customPreTrain] End of dwc_ddrphy_phyinit_userCustom_customPreTrain()
//// [dwc_ddrphy_phyinit_D_loadIMEM, 1D] Start of dwc_ddrphy_phyinit_D_loadIMEM (Train2D=0)
//
//
////##############################################################
////
//// (D) Load the 1D IMEM image
////
//// This function loads the training firmware IMEM image into the SRAM.
//// See PhyInit App Note for detailed description and function usage
////
////##############################################################
//
//
//// [dwc_ddrphy_phyinit_D_loadIMEM, 1D] Programming MemResetL to 0x2
dwc_ddrphy_apb_wr(0x20060, 0x2); // DWC_DDRPHYA_MASTER0_base0_MemResetL
// [dwc_ddrphy_phyinit_storeIncvFile] Reading input file: /home/jerry_ku/Project/Development/ast2700dev/ddr45phy_tsmc12/coreConsultant/config3_3.50a/2022-12-12-17-01-49/firmware/Latest/training/ddr5/ddr5_pmu_train_imem.incv

//// 1.	Enable access to the internal CSRs by setting the MicroContMuxSel CSR to 0.
////       This allows the memory controller unrestricted access to the configuration CSRs.
dwc_ddrphy_apb_wr(0xd0000, 0x0); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
//// [phyinit_userCustom_wait] Wait 40 DfiClks
//// [dwc_ddrphy_phyinit_WriteOutMem] STARTING 32bit write. offset 0x50000 size 0x8000
dwc_ddrphy_phyinit_userCustom_D_loadIMEM(sdrammc, 0);

//// [dwc_ddrphy_phyinit_WriteOutMem] DONE.  Index 0x8000
//// 2.	Isolate the APB access from the internal CSRs by setting the MicroContMuxSel CSR to 1.
////      This allows the firmware unrestricted access to the configuration CSRs.
dwc_ddrphy_apb_wr(0xd0000, 0x1); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
//// [phyinit_userCustom_wait] Wait 40 DfiClks
//// [dwc_ddrphy_phyinit_D_loadIMEM, 1D] End of dwc_ddrphy_phyinit_D_loadIMEM()
//
//
////##############################################################
////
//// 4.3.5(E) Set the PHY input clocks to the desired frequency for pstate 0
////
//// Set the PHY input Dfi Clk to the desired operating frequency associated with the given Pstate. Before proceeding to the next step,
//// the clock should be stable at the new frequency. For more information on clocking requirements, see "Clocks" on page <XXX>.
////
////##############################################################
//
dwc_ddrphy_phyinit_userCustom_E_setDfiClk(sdrammc);

//
//// [dwc_ddrphy_phyinit_userCustom_E_setDfiClk] End of dwc_ddrphy_phyinit_userCustom_E_setDfiClk()
//// [phyinit_F_loadDMEM, 1D] Start of dwc_ddrphy_phyinit_F_loadDMEM (pstate=0, Train2D=0)
//
//
////##############################################################
////
//// 4.3.5(F) Load the 1D DMEM image and write the 1D Message Block parameters for the training firmware
////
//// The procedure is as follows:
////
////##############################################################
//
//
//
//// 1.    Load the firmware DMEM segment to initialize the data structures.
//
//// 2.    Write the Firmware Message Block with the required contents detailing the training parameters.
//
// [dwc_ddrphy_phyinit_storeIncvFile] Reading input file: /home/jerry_ku/Project/Development/ast2700dev/ddr45phy_tsmc12/coreConsultant/config3_3.50a/2022-12-12-17-01-49/firmware/Latest/training/ddr5/ddr5_pmu_train_dmem.incv

//// 1.	Enable access to the internal CSRs by setting the MicroContMuxSel CSR to 0.
////       This allows the memory controller unrestricted access to the configuration CSRs.
dwc_ddrphy_apb_wr(0xd0000, 0x0); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
//// [phyinit_userCustom_wait] Wait 40 DfiClks
//// [dwc_ddrphy_phyinit_WriteOutMem] STARTING 32bit write. offset 0x58000 size 0x8000

dwc_ddrphy_phyinit_userCustom_F_loadDMEM(sdrammc, 0, 0);

dwc_ddrphy_apb_wr_32b(0x58000, 0x100);
dwc_ddrphy_apb_wr_32b(0x58002, 0xc800000);
dwc_ddrphy_apb_wr_32b(0x58004, 0x0);
dwc_ddrphy_apb_wr_32b(0x58006, 0x40);
if (IS_ENABLED(CONFIG_ASPEED_PHY_TRAINING_MESSAGE))
	dwc_ddrphy_apb_wr_32b(0x58008, 0x04827f);
else
	dwc_ddrphy_apb_wr_32b(0x58008, 0xc8827f);
// Redmine 1392: Set X16Present=1 by Synopsys's comment
// 0x5800b[7:0]=DFIMRLMargin, 0x5800b[15:8]=X16Present
dwc_ddrphy_apb_wr_32b(0x5800a, 0x01020000);
// Redmine 1456: Skip_CA13_during_CAtraining during DDR5
dwc_ddrphy_apb_wr_32b(0x5800c, 0x10000001);
// Redmine 1392: To speed up data collection, set the voltage and delay step size in Rx2D_TrainOpt and Tx2D_TrainOpt to its maximum value.
//   uint8_t  RX2D_TrainOpt;    // Byte offset 0x1d, CSR Addr 0x5800e, Direction=In
//   uint8_t  TX2D_TrainOpt;    // Byte offset 0x1e, CSR Addr 0x5800f, Direction=In
//dwc_ddrphy_apb_wr_32b(0x5800e, 0x001e1e00);
//#elif defined(TRAIN_1D)
//printf("- <DWC_DDRPHY TRAIN>: Enable RdDQS1D, WrDQ1D for 1D training");
//  #ifdef DWC_DEBUG
//printf("- <DWC_DDRPHY TRAIN>: Debug level = 0x05: Detailed debug messages (e.g. Eye delays)");
//dwc_ddrphy_apb_wr_32b(0x58008, 0x05821f);
//  #else
//printf("- <DWC_DDRPHY TRAIN>: Debug level = 0xC8: Stage completion");
//dwc_ddrphy_apb_wr_32b(0x58008, 0xc8821f);
//  #endif
//// Redmine 1392: Set X16Present=1 by Synopsys's comment
//dwc_ddrphy_apb_wr_32b(0x5800a, 0x01020000);
//// Redmine 1456: Skip_CA13_during_CAtraining during DDR5
//dwc_ddrphy_apb_wr_32b(0x5800c, 0x18000001);
//// Redmine 1392: To speed up data collection, set the voltage and delay step size in Rx2D_TrainOpt and Tx2D_TrainOpt to its maximum value.
////   uint8_t  RX2D_TrainOpt;    // Byte offset 0x1d, CSR Addr 0x5800e, Direction=In
////   uint8_t  TX2D_TrainOpt;    // Byte offset 0x1e, CSR Addr 0x5800f, Direction=In
//dwc_ddrphy_apb_wr_32b(0x5800e, 0x001e1e00);
//#else
//dwc_ddrphy_apb_wr_32b(0x58008, 0xc8837f);
//dwc_ddrphy_apb_wr_32b(0x5800a, 0x20000);
//dwc_ddrphy_apb_wr_32b(0x5800c, 0x8000001);
//dwc_ddrphy_apb_wr_32b(0x5800e, 0x0);
//#endif
dwc_ddrphy_apb_wr_32b(0x58010, 0x0);
dwc_ddrphy_apb_wr_32b(0x58012, 0x110);
dwc_ddrphy_apb_wr_32b(0x58014, 0x0);
dwc_ddrphy_apb_wr_32b(0x58016, 0x0);
dwc_ddrphy_apb_wr_32b(0x58018, 0x0);
dwc_ddrphy_apb_wr_32b(0x5801a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5801c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5801e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58020, 0x0);
dwc_ddrphy_apb_wr_32b(0x58022, 0x0);
dwc_ddrphy_apb_wr_32b(0x58024, 0x0);
dwc_ddrphy_apb_wr_32b(0x58026, 0x0);
dwc_ddrphy_apb_wr_32b(0x58028, 0x0);
dwc_ddrphy_apb_wr_32b(0x5802a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5802c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5802e, 0x84080000); //MR0 0x5802f=0x8(CL=26), MR2 0x5802f=0x84(OP[2]=1 2N mode, OP[7]=enable internal WL training)
dwc_ddrphy_apb_wr_32b(0x58030, 0x200000); //MR5 0x58031=0x20(OP[5]=1 DM enable, OP[2:1]=0 pu 34ohm, 1=40ohm, 2=48ohm, OP7:6]=pd)
dwc_ddrphy_apb_wr_32b(0x58032, 0x2d000800); //MR8 0x58032=0x08(OP[4:3]=1 Write preamble 2 tCK) MR10 0x58033=0x2d(Vref 75%)
dwc_ddrphy_apb_wr_32b(0x58034, 0xd62d);
dwc_ddrphy_apb_wr_32b(0x58036, 0x04240003); //MR32 0x58037=0x24(OP[2:0]=4 CK ODT 80, OP[5:3]=4 CS ODT 80ohm), MR33 0x58037=0x4(OP[2:0]=4 CA ODTt 80ohm)
dwc_ddrphy_apb_wr_32b(0x58038, 0x2c000499); //MR34 0x58038(OP[5:3]=3 RTT_WR 80)
dwc_ddrphy_apb_wr_32b(0x5803a, 0x2c2c);
dwc_ddrphy_apb_wr_32b(0x5803c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5803e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58040, 0x0);
dwc_ddrphy_apb_wr_32b(0x58042, 0x408);
dwc_ddrphy_apb_wr_32b(0x58044, 0x8000020);
dwc_ddrphy_apb_wr_32b(0x58046, 0xd62d2d00);
dwc_ddrphy_apb_wr_32b(0x58048, 0x30000);
dwc_ddrphy_apb_wr_32b(0x5804a, 0x4110000);
dwc_ddrphy_apb_wr_32b(0x5804c, 0x2c2c2c00);
dwc_ddrphy_apb_wr_32b(0x5804e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58050, 0x0);
dwc_ddrphy_apb_wr_32b(0x58052, 0x0);
dwc_ddrphy_apb_wr_32b(0x58054, 0x4080000);
dwc_ddrphy_apb_wr_32b(0x58056, 0x200000);
dwc_ddrphy_apb_wr_32b(0x58058, 0x2d000800);
dwc_ddrphy_apb_wr_32b(0x5805a, 0xd62d);
dwc_ddrphy_apb_wr_32b(0x5805c, 0x3);
dwc_ddrphy_apb_wr_32b(0x5805e, 0x2c000411);
dwc_ddrphy_apb_wr_32b(0x58060, 0x2c2c);
dwc_ddrphy_apb_wr_32b(0x58062, 0x0);
dwc_ddrphy_apb_wr_32b(0x58064, 0x0);
dwc_ddrphy_apb_wr_32b(0x58066, 0x0);
dwc_ddrphy_apb_wr_32b(0x58068, 0x408);
dwc_ddrphy_apb_wr_32b(0x5806a, 0x8000020);
dwc_ddrphy_apb_wr_32b(0x5806c, 0xd62d2d00);
dwc_ddrphy_apb_wr_32b(0x5806e, 0x30000);
dwc_ddrphy_apb_wr_32b(0x58070, 0x4110000);
dwc_ddrphy_apb_wr_32b(0x58072, 0x2c2c2c00);
dwc_ddrphy_apb_wr_32b(0x58074, 0x0);
dwc_ddrphy_apb_wr_32b(0x58076, 0x0);
dwc_ddrphy_apb_wr_32b(0x58078, 0x0);
dwc_ddrphy_apb_wr_32b(0x5807a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5807c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5807e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58080, 0x0);
dwc_ddrphy_apb_wr_32b(0x58082, 0x0);
dwc_ddrphy_apb_wr_32b(0x58084, 0x0);
dwc_ddrphy_apb_wr_32b(0x58086, 0x0);
dwc_ddrphy_apb_wr_32b(0x58088, 0x0);
dwc_ddrphy_apb_wr_32b(0x5808a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5808c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5808e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58090, 0x0);
dwc_ddrphy_apb_wr_32b(0x58092, 0x0);
dwc_ddrphy_apb_wr_32b(0x58094, 0x0);
dwc_ddrphy_apb_wr_32b(0x58096, 0x0);
dwc_ddrphy_apb_wr_32b(0x58098, 0x0);
dwc_ddrphy_apb_wr_32b(0x5809a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5809c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5809e, 0x0);
dwc_ddrphy_apb_wr_32b(0x580a0, 0x0);
dwc_ddrphy_apb_wr_32b(0x580a2, 0x0);
dwc_ddrphy_apb_wr_32b(0x580a4, 0x4080000);
dwc_ddrphy_apb_wr_32b(0x580a6, 0x200000);
dwc_ddrphy_apb_wr_32b(0x580a8, 0x2d000800);
dwc_ddrphy_apb_wr_32b(0x580aa, 0xd62d);
dwc_ddrphy_apb_wr_32b(0x580ac, 0x3);
dwc_ddrphy_apb_wr_32b(0x580ae, 0x2c000411);
dwc_ddrphy_apb_wr_32b(0x580b0, 0x2c2c);
dwc_ddrphy_apb_wr_32b(0x580b2, 0x0);
dwc_ddrphy_apb_wr_32b(0x580b4, 0x0);
dwc_ddrphy_apb_wr_32b(0x580b6, 0x0);
dwc_ddrphy_apb_wr_32b(0x580b8, 0x408);
dwc_ddrphy_apb_wr_32b(0x580ba, 0x8000020);
dwc_ddrphy_apb_wr_32b(0x580bc, 0xd62d2d00);
dwc_ddrphy_apb_wr_32b(0x580be, 0x30000);
dwc_ddrphy_apb_wr_32b(0x580c0, 0x4110000);
dwc_ddrphy_apb_wr_32b(0x580c2, 0x2c2c2c00);
dwc_ddrphy_apb_wr_32b(0x580c4, 0x0);
dwc_ddrphy_apb_wr_32b(0x580c6, 0x0);
dwc_ddrphy_apb_wr_32b(0x580c8, 0x0);
dwc_ddrphy_apb_wr_32b(0x580ca, 0x4080000);
dwc_ddrphy_apb_wr_32b(0x580cc, 0x200000);
dwc_ddrphy_apb_wr_32b(0x580ce, 0x2d000800);
dwc_ddrphy_apb_wr_32b(0x580d0, 0xd62d);
dwc_ddrphy_apb_wr_32b(0x580d2, 0x3);
dwc_ddrphy_apb_wr_32b(0x580d4, 0x2c000411);
dwc_ddrphy_apb_wr_32b(0x580d6, 0x2c2c);
dwc_ddrphy_apb_wr_32b(0x580d8, 0x0);
dwc_ddrphy_apb_wr_32b(0x580da, 0x0);
dwc_ddrphy_apb_wr_32b(0x580dc, 0x0);
dwc_ddrphy_apb_wr_32b(0x580de, 0x408);
dwc_ddrphy_apb_wr_32b(0x580e0, 0x8000020);
dwc_ddrphy_apb_wr_32b(0x580e2, 0xd62d2d00);
dwc_ddrphy_apb_wr_32b(0x580e4, 0x30000);
dwc_ddrphy_apb_wr_32b(0x580e6, 0x4110000);
dwc_ddrphy_apb_wr_32b(0x580e8, 0x2c2c2c00);
dwc_ddrphy_apb_wr_32b(0x580ea, 0x0);
dwc_ddrphy_apb_wr_32b(0x580ec, 0x0);
dwc_ddrphy_apb_wr_32b(0x580ee, 0x0);
dwc_ddrphy_apb_wr_32b(0x580f0, 0x0);
dwc_ddrphy_apb_wr_32b(0x580f2, 0x0);
dwc_ddrphy_apb_wr_32b(0x580f4, 0x0);
dwc_ddrphy_apb_wr_32b(0x580f6, 0x0);
dwc_ddrphy_apb_wr_32b(0x580f8, 0x0);
dwc_ddrphy_apb_wr_32b(0x580fa, 0x0);
dwc_ddrphy_apb_wr_32b(0x580fc, 0x0);
dwc_ddrphy_apb_wr_32b(0x580fe, 0xa00060); // WL_ADJ_START, WL_ADJ_END
dwc_ddrphy_apb_wr_32b(0x58100, 0x1);
dwc_ddrphy_apb_wr_32b(0x58102, 0x0);
dwc_ddrphy_apb_wr_32b(0x58104, 0x0);
dwc_ddrphy_apb_wr_32b(0x58106, 0x0);
dwc_ddrphy_apb_wr_32b(0x58108, 0x0);
dwc_ddrphy_apb_wr_32b(0x5810a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5810c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5810e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58110, 0x0);
dwc_ddrphy_apb_wr_32b(0x58112, 0x0);
dwc_ddrphy_apb_wr_32b(0x58114, 0x0);
dwc_ddrphy_apb_wr_32b(0x58116, 0x0);
dwc_ddrphy_apb_wr_32b(0x58118, 0x0);
dwc_ddrphy_apb_wr_32b(0x5811a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5811c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5811e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58120, 0x0);
dwc_ddrphy_apb_wr_32b(0x58122, 0x0);
dwc_ddrphy_apb_wr_32b(0x58124, 0x0);
dwc_ddrphy_apb_wr_32b(0x58126, 0x0);
dwc_ddrphy_apb_wr_32b(0x58128, 0x0);
dwc_ddrphy_apb_wr_32b(0x5812a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5812c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5812e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58130, 0x0);
dwc_ddrphy_apb_wr_32b(0x58132, 0x0);
dwc_ddrphy_apb_wr_32b(0x58134, 0x0);
dwc_ddrphy_apb_wr_32b(0x58136, 0x0);
dwc_ddrphy_apb_wr_32b(0x58138, 0x0);
dwc_ddrphy_apb_wr_32b(0x5813a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5813c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5813e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58140, 0x0);
dwc_ddrphy_apb_wr_32b(0x58142, 0x0);
dwc_ddrphy_apb_wr_32b(0x58144, 0x0);
dwc_ddrphy_apb_wr_32b(0x58146, 0x0);
dwc_ddrphy_apb_wr_32b(0x58148, 0x0);
dwc_ddrphy_apb_wr_32b(0x5814a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5814c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5814e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58150, 0x0);
dwc_ddrphy_apb_wr_32b(0x58152, 0x0);
dwc_ddrphy_apb_wr_32b(0x58154, 0x0);
dwc_ddrphy_apb_wr_32b(0x58156, 0x0);
dwc_ddrphy_apb_wr_32b(0x58158, 0x0);
dwc_ddrphy_apb_wr_32b(0x5815a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5815c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5815e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58160, 0x0);
dwc_ddrphy_apb_wr_32b(0x58162, 0x0);
dwc_ddrphy_apb_wr_32b(0x58164, 0x0);
dwc_ddrphy_apb_wr_32b(0x58166, 0x0);
dwc_ddrphy_apb_wr_32b(0x58168, 0x0);
dwc_ddrphy_apb_wr_32b(0x5816a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5816c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5816e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58170, 0x0);
dwc_ddrphy_apb_wr_32b(0x58172, 0x0);
dwc_ddrphy_apb_wr_32b(0x58174, 0x0);
dwc_ddrphy_apb_wr_32b(0x58176, 0x0);
dwc_ddrphy_apb_wr_32b(0x58178, 0x0);
dwc_ddrphy_apb_wr_32b(0x5817a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5817c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5817e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58180, 0x1);
dwc_ddrphy_apb_wr_32b(0x58182, 0x0);
dwc_ddrphy_apb_wr_32b(0x58184, 0x0);
dwc_ddrphy_apb_wr_32b(0x58186, 0x0);
dwc_ddrphy_apb_wr_32b(0x58188, 0x0);
dwc_ddrphy_apb_wr_32b(0x5818a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5818c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5818e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58190, 0x0);
dwc_ddrphy_apb_wr_32b(0x58192, 0x0);
dwc_ddrphy_apb_wr_32b(0x58194, 0x0);
dwc_ddrphy_apb_wr_32b(0x58196, 0x0);
dwc_ddrphy_apb_wr_32b(0x58198, 0x0);
dwc_ddrphy_apb_wr_32b(0x5819a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5819c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5819e, 0x0);
dwc_ddrphy_apb_wr_32b(0x581a0, 0x0);
dwc_ddrphy_apb_wr_32b(0x581a2, 0x0);
dwc_ddrphy_apb_wr_32b(0x581a4, 0x0);
dwc_ddrphy_apb_wr_32b(0x581a6, 0x0);
dwc_ddrphy_apb_wr_32b(0x581a8, 0x0);
dwc_ddrphy_apb_wr_32b(0x581aa, 0x0);
dwc_ddrphy_apb_wr_32b(0x581ac, 0x0);
dwc_ddrphy_apb_wr_32b(0x581ae, 0x0);
dwc_ddrphy_apb_wr_32b(0x581b0, 0x0);
dwc_ddrphy_apb_wr_32b(0x581b2, 0x0);
dwc_ddrphy_apb_wr_32b(0x581b4, 0x0);
dwc_ddrphy_apb_wr_32b(0x581b6, 0x0);
dwc_ddrphy_apb_wr_32b(0x581b8, 0x0);
dwc_ddrphy_apb_wr_32b(0x581ba, 0x0);
dwc_ddrphy_apb_wr_32b(0x581bc, 0x0);
dwc_ddrphy_apb_wr_32b(0x581be, 0x0);
dwc_ddrphy_apb_wr_32b(0x581c0, 0x0);
dwc_ddrphy_apb_wr_32b(0x581c2, 0x0);
dwc_ddrphy_apb_wr_32b(0x581c4, 0x0);
dwc_ddrphy_apb_wr_32b(0x581c6, 0x0);
dwc_ddrphy_apb_wr_32b(0x581c8, 0x0);
dwc_ddrphy_apb_wr_32b(0x581ca, 0x0);
dwc_ddrphy_apb_wr_32b(0x581cc, 0x0);
dwc_ddrphy_apb_wr_32b(0x581ce, 0x0);
dwc_ddrphy_apb_wr_32b(0x581d0, 0x0);
dwc_ddrphy_apb_wr_32b(0x581d2, 0x0);
dwc_ddrphy_apb_wr_32b(0x581d4, 0x0);
dwc_ddrphy_apb_wr_32b(0x581d6, 0x0);
dwc_ddrphy_apb_wr_32b(0x581d8, 0x0);
dwc_ddrphy_apb_wr_32b(0x581da, 0x0);
dwc_ddrphy_apb_wr_32b(0x581dc, 0x0);
dwc_ddrphy_apb_wr_32b(0x581de, 0x0);
dwc_ddrphy_apb_wr_32b(0x581e0, 0x0);
dwc_ddrphy_apb_wr_32b(0x581e2, 0x0);
dwc_ddrphy_apb_wr_32b(0x581e4, 0x0);
dwc_ddrphy_apb_wr_32b(0x581e6, 0x0);
dwc_ddrphy_apb_wr_32b(0x581e8, 0x0);
dwc_ddrphy_apb_wr_32b(0x581ea, 0x0);
dwc_ddrphy_apb_wr_32b(0x581ec, 0x0);
dwc_ddrphy_apb_wr_32b(0x581ee, 0x0);
dwc_ddrphy_apb_wr_32b(0x581f0, 0x0);
dwc_ddrphy_apb_wr_32b(0x581f2, 0x0);
dwc_ddrphy_apb_wr_32b(0x581f4, 0x0);
dwc_ddrphy_apb_wr_32b(0x581f6, 0x0);
dwc_ddrphy_apb_wr_32b(0x581f8, 0x0);
dwc_ddrphy_apb_wr_32b(0x581fa, 0x0);
dwc_ddrphy_apb_wr_32b(0x581fc, 0x0);
dwc_ddrphy_apb_wr_32b(0x581fe, 0x0);
dwc_ddrphy_apb_wr_32b(0x58200, 0x1);
dwc_ddrphy_apb_wr_32b(0x58202, 0x0);
dwc_ddrphy_apb_wr_32b(0x58204, 0x0);
dwc_ddrphy_apb_wr_32b(0x58206, 0x0);
dwc_ddrphy_apb_wr_32b(0x58208, 0x0);
dwc_ddrphy_apb_wr_32b(0x5820a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5820c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5820e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58210, 0x0);
dwc_ddrphy_apb_wr_32b(0x58212, 0x0);
dwc_ddrphy_apb_wr_32b(0x58214, 0x0);
dwc_ddrphy_apb_wr_32b(0x58216, 0x0);
dwc_ddrphy_apb_wr_32b(0x58218, 0x0);
dwc_ddrphy_apb_wr_32b(0x5821a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5821c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5821e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58220, 0x0);
dwc_ddrphy_apb_wr_32b(0x58222, 0x0);
dwc_ddrphy_apb_wr_32b(0x58224, 0x0);
dwc_ddrphy_apb_wr_32b(0x58226, 0x0);
dwc_ddrphy_apb_wr_32b(0x58228, 0x0);
dwc_ddrphy_apb_wr_32b(0x5822a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5822c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5822e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58230, 0x0);
dwc_ddrphy_apb_wr_32b(0x58232, 0x0);
dwc_ddrphy_apb_wr_32b(0x58234, 0x0);
dwc_ddrphy_apb_wr_32b(0x58236, 0x0);
dwc_ddrphy_apb_wr_32b(0x58238, 0x0);
dwc_ddrphy_apb_wr_32b(0x5823a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5823c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5823e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58240, 0x0);
dwc_ddrphy_apb_wr_32b(0x58242, 0x0);
dwc_ddrphy_apb_wr_32b(0x58244, 0x0);
dwc_ddrphy_apb_wr_32b(0x58246, 0x0);
dwc_ddrphy_apb_wr_32b(0x58248, 0x0);
dwc_ddrphy_apb_wr_32b(0x5824a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5824c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5824e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58250, 0x0);
dwc_ddrphy_apb_wr_32b(0x58252, 0x0);
dwc_ddrphy_apb_wr_32b(0x58254, 0x0);
dwc_ddrphy_apb_wr_32b(0x58256, 0x0);
dwc_ddrphy_apb_wr_32b(0x58258, 0x0);
dwc_ddrphy_apb_wr_32b(0x5825a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5825c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5825e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58260, 0x0);
dwc_ddrphy_apb_wr_32b(0x58262, 0x0);
dwc_ddrphy_apb_wr_32b(0x58264, 0x0);
dwc_ddrphy_apb_wr_32b(0x58266, 0x0);
dwc_ddrphy_apb_wr_32b(0x58268, 0x0);
dwc_ddrphy_apb_wr_32b(0x5826a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5826c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5826e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58270, 0x0);
dwc_ddrphy_apb_wr_32b(0x58272, 0x0);
dwc_ddrphy_apb_wr_32b(0x58274, 0x0);
dwc_ddrphy_apb_wr_32b(0x58276, 0x0);
dwc_ddrphy_apb_wr_32b(0x58278, 0x0);
dwc_ddrphy_apb_wr_32b(0x5827a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5827c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5827e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58280, 0x1);
dwc_ddrphy_apb_wr_32b(0x58282, 0x0);
dwc_ddrphy_apb_wr_32b(0x58284, 0x0);
dwc_ddrphy_apb_wr_32b(0x58286, 0x0);
dwc_ddrphy_apb_wr_32b(0x58288, 0x0);
dwc_ddrphy_apb_wr_32b(0x5828a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5828c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5828e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58290, 0x0);
dwc_ddrphy_apb_wr_32b(0x58292, 0x0);
dwc_ddrphy_apb_wr_32b(0x58294, 0x0);
dwc_ddrphy_apb_wr_32b(0x58296, 0x0);
dwc_ddrphy_apb_wr_32b(0x58298, 0x0);
dwc_ddrphy_apb_wr_32b(0x5829a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5829c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5829e, 0x0);
dwc_ddrphy_apb_wr_32b(0x582a0, 0x0);
dwc_ddrphy_apb_wr_32b(0x582a2, 0x0);
dwc_ddrphy_apb_wr_32b(0x582a4, 0x0);
dwc_ddrphy_apb_wr_32b(0x582a6, 0x0);
dwc_ddrphy_apb_wr_32b(0x582a8, 0x0);
dwc_ddrphy_apb_wr_32b(0x582aa, 0x0);
dwc_ddrphy_apb_wr_32b(0x582ac, 0x0);
dwc_ddrphy_apb_wr_32b(0x582ae, 0x0);
dwc_ddrphy_apb_wr_32b(0x582b0, 0x0);
dwc_ddrphy_apb_wr_32b(0x582b2, 0x0);
dwc_ddrphy_apb_wr_32b(0x582b4, 0x0);
dwc_ddrphy_apb_wr_32b(0x582b6, 0x0);
dwc_ddrphy_apb_wr_32b(0x582b8, 0x0);
dwc_ddrphy_apb_wr_32b(0x582ba, 0x0);
dwc_ddrphy_apb_wr_32b(0x582bc, 0x0);
dwc_ddrphy_apb_wr_32b(0x582be, 0x0);
dwc_ddrphy_apb_wr_32b(0x582c0, 0x0);
dwc_ddrphy_apb_wr_32b(0x582c2, 0x0);
dwc_ddrphy_apb_wr_32b(0x582c4, 0x0);
dwc_ddrphy_apb_wr_32b(0x582c6, 0x0);
dwc_ddrphy_apb_wr_32b(0x582c8, 0x0);
dwc_ddrphy_apb_wr_32b(0x582ca, 0x0);
dwc_ddrphy_apb_wr_32b(0x582cc, 0x0);
dwc_ddrphy_apb_wr_32b(0x582ce, 0x0);
dwc_ddrphy_apb_wr_32b(0x582d0, 0x0);
dwc_ddrphy_apb_wr_32b(0x582d2, 0x0);
dwc_ddrphy_apb_wr_32b(0x582d4, 0x0);
dwc_ddrphy_apb_wr_32b(0x582d6, 0x0);
dwc_ddrphy_apb_wr_32b(0x582d8, 0x0);
dwc_ddrphy_apb_wr_32b(0x582da, 0x0);
dwc_ddrphy_apb_wr_32b(0x582dc, 0x0);
dwc_ddrphy_apb_wr_32b(0x582de, 0x0);
dwc_ddrphy_apb_wr_32b(0x582e0, 0x0);
dwc_ddrphy_apb_wr_32b(0x582e2, 0x0);
dwc_ddrphy_apb_wr_32b(0x582e4, 0x0);
dwc_ddrphy_apb_wr_32b(0x582e6, 0x0);
dwc_ddrphy_apb_wr_32b(0x582e8, 0x0);
dwc_ddrphy_apb_wr_32b(0x582ea, 0x0);
dwc_ddrphy_apb_wr_32b(0x582ec, 0x0);
dwc_ddrphy_apb_wr_32b(0x582ee, 0x0);
dwc_ddrphy_apb_wr_32b(0x582f0, 0x0);
dwc_ddrphy_apb_wr_32b(0x582f2, 0x0);
dwc_ddrphy_apb_wr_32b(0x582f4, 0x0);
dwc_ddrphy_apb_wr_32b(0x582f6, 0x0);
dwc_ddrphy_apb_wr_32b(0x582f8, 0x0);
dwc_ddrphy_apb_wr_32b(0x582fa, 0x0);
dwc_ddrphy_apb_wr_32b(0x582fc, 0x0);
dwc_ddrphy_apb_wr_32b(0x582fe, 0x0);
dwc_ddrphy_apb_wr_32b(0x58300, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58302, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58304, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58306, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58308, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x5830a, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x5830c, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x5830e, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58310, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58312, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58314, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58316, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58318, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x5831a, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x5831c, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x5831e, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58320, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58322, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58324, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58326, 0x17171717);
dwc_ddrphy_apb_wr_32b(0x58328, 0x0);
dwc_ddrphy_apb_wr_32b(0x5832a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5832c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5832e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58330, 0x0);
dwc_ddrphy_apb_wr_32b(0x58332, 0x0);
dwc_ddrphy_apb_wr_32b(0x58334, 0x0);
dwc_ddrphy_apb_wr_32b(0x58336, 0x0);
dwc_ddrphy_apb_wr_32b(0x58338, 0x0);
dwc_ddrphy_apb_wr_32b(0x5833a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5833c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5833e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58340, 0x0);
dwc_ddrphy_apb_wr_32b(0x58342, 0x0);
dwc_ddrphy_apb_wr_32b(0x58344, 0x0);
dwc_ddrphy_apb_wr_32b(0x58346, 0x0);
dwc_ddrphy_apb_wr_32b(0x58348, 0x0);
dwc_ddrphy_apb_wr_32b(0x5834a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5834c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5834e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58350, 0x0);
dwc_ddrphy_apb_wr_32b(0x58352, 0x0);
dwc_ddrphy_apb_wr_32b(0x58354, 0x0);
dwc_ddrphy_apb_wr_32b(0x58356, 0x0);
dwc_ddrphy_apb_wr_32b(0x58358, 0x0);
dwc_ddrphy_apb_wr_32b(0x5835a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5835c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5835e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58360, 0x0);
dwc_ddrphy_apb_wr_32b(0x58362, 0x0);
dwc_ddrphy_apb_wr_32b(0x58364, 0x0);
dwc_ddrphy_apb_wr_32b(0x58366, 0x0);
dwc_ddrphy_apb_wr_32b(0x58368, 0x0);
dwc_ddrphy_apb_wr_32b(0x5836a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5836c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5836e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58370, 0x0);
dwc_ddrphy_apb_wr_32b(0x58372, 0x0);
dwc_ddrphy_apb_wr_32b(0x58374, 0x0);
dwc_ddrphy_apb_wr_32b(0x58376, 0x0);
dwc_ddrphy_apb_wr_32b(0x58378, 0x0);
dwc_ddrphy_apb_wr_32b(0x5837a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5837c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5837e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58380, 0x0);
dwc_ddrphy_apb_wr_32b(0x58382, 0x0);
dwc_ddrphy_apb_wr_32b(0x58384, 0x0);
dwc_ddrphy_apb_wr_32b(0x58386, 0x0);
dwc_ddrphy_apb_wr_32b(0x58388, 0x0);
dwc_ddrphy_apb_wr_32b(0x5838a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5838c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5838e, 0x0);
dwc_ddrphy_apb_wr_32b(0x58390, 0x0);
dwc_ddrphy_apb_wr_32b(0x58392, 0x0);
dwc_ddrphy_apb_wr_32b(0x58394, 0x0);
dwc_ddrphy_apb_wr_32b(0x58396, 0x0);
dwc_ddrphy_apb_wr_32b(0x58398, 0x0);
dwc_ddrphy_apb_wr_32b(0x5839a, 0x0);
dwc_ddrphy_apb_wr_32b(0x5839c, 0x0);
dwc_ddrphy_apb_wr_32b(0x5839e, 0x0);
dwc_ddrphy_apb_wr_32b(0x583a0, 0x0);
dwc_ddrphy_apb_wr_32b(0x583a2, 0x0);
dwc_ddrphy_apb_wr_32b(0x583a4, 0x0);
dwc_ddrphy_apb_wr_32b(0x583a6, 0x0);
dwc_ddrphy_apb_wr_32b(0x583a8, 0x0);
dwc_ddrphy_apb_wr_32b(0x583aa, 0x0);
dwc_ddrphy_apb_wr_32b(0x583ac, 0x0);
dwc_ddrphy_apb_wr_32b(0x583ae, 0x0);
dwc_ddrphy_apb_wr_32b(0x583b0, 0x0);
dwc_ddrphy_apb_wr_32b(0x583b2, 0x0);
dwc_ddrphy_apb_wr_32b(0x583b4, 0x0);
dwc_ddrphy_apb_wr_32b(0x583b6, 0x0);
dwc_ddrphy_apb_wr_32b(0x583b8, 0x0);
dwc_ddrphy_apb_wr_32b(0x583ba, 0x0);
dwc_ddrphy_apb_wr_32b(0x583bc, 0x0);
dwc_ddrphy_apb_wr_32b(0x583be, 0x0);
dwc_ddrphy_apb_wr_32b(0x583c0, 0x0);
dwc_ddrphy_apb_wr_32b(0x583c2, 0x0);
dwc_ddrphy_apb_wr_32b(0x583c4, 0x0);
dwc_ddrphy_apb_wr_32b(0x583c6, 0x0);
dwc_ddrphy_apb_wr_32b(0x583c8, 0x0);
dwc_ddrphy_apb_wr_32b(0x583ca, 0x0);
dwc_ddrphy_apb_wr_32b(0x583cc, 0x0);
dwc_ddrphy_apb_wr_32b(0x583ce, 0x0);
dwc_ddrphy_apb_wr_32b(0x583d0, 0x0);
dwc_ddrphy_apb_wr_32b(0x583d2, 0x0);
dwc_ddrphy_apb_wr_32b(0x583d4, 0x0);
dwc_ddrphy_apb_wr_32b(0x583d6, 0x0);
dwc_ddrphy_apb_wr_32b(0x583d8, 0x0);
dwc_ddrphy_apb_wr_32b(0x583da, 0x0);
dwc_ddrphy_apb_wr_32b(0x583dc, 0x0);
dwc_ddrphy_apb_wr_32b(0x583de, 0x0);
dwc_ddrphy_apb_wr_32b(0x583e0, 0x0);
dwc_ddrphy_apb_wr_32b(0x583e2, 0x0);
dwc_ddrphy_apb_wr_32b(0x583e4, 0x0);
dwc_ddrphy_apb_wr_32b(0x583e6, 0x0);
dwc_ddrphy_apb_wr_32b(0x583e8, 0x0);
dwc_ddrphy_apb_wr_32b(0x583ea, 0x0);
dwc_ddrphy_apb_wr_32b(0x583ec, 0x0);
dwc_ddrphy_apb_wr_32b(0x583ee, 0x0);
dwc_ddrphy_apb_wr_32b(0x583f0, 0x0);
dwc_ddrphy_apb_wr_32b(0x583f2, 0x0);
dwc_ddrphy_apb_wr_32b(0x583f4, 0x0);
dwc_ddrphy_apb_wr_32b(0x583f6, 0x0);
dwc_ddrphy_apb_wr_32b(0x583f8, 0x0);
dwc_ddrphy_apb_wr_32b(0x583fa, 0x0);
dwc_ddrphy_apb_wr_32b(0x583fc, 0x0);
dwc_ddrphy_apb_wr_32b(0x583fe, 0x0);
//// [dwc_ddrphy_phyinit_WriteOutMem] DONE.  Index 0x8000
//// 2.	Isolate the APB access from the internal CSRs by setting the MicroContMuxSel CSR to 1.
////      This allows the firmware unrestricted access to the configuration CSRs.
dwc_ddrphy_apb_wr(0xd0000, 0x1); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
//// [phyinit_userCustom_wait] Wait 40 DfiClks
//// [phyinit_F_loadDMEM, 1D] End of dwc_ddrphy_phyinit_F_loadDMEM()
//
//
////##############################################################
////
//// 4.3.7(G) Execute the Training Firmware
////
//// The training firmware is executed with the following procedure:
////
////##############################################################
//
//
//// 1.  Reset the firmware microcontroller by writing the MicroReset CSR to set the StallToMicro and
////     ResetToMicro fields to 1 (all other fields should be zero).
////     Then rewrite the CSR so that only the StallToMicro remains set (all other fields should be zero).
dwc_ddrphy_apb_wr(0xd0000, 0x1); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
//// [phyinit_userCustom_wait] Wait 40 DfiClks
dwc_ddrphy_apb_wr(0xd0099, 0x9); // DWC_DDRPHYA_APBONLY0_MicroReset
dwc_ddrphy_apb_wr(0xd0099, 0x1); // DWC_DDRPHYA_APBONLY0_MicroReset
//
//// 2. Begin execution of the training firmware by setting the MicroReset CSR to 4'b0000.
dwc_ddrphy_apb_wr(0xd0099, 0x0); // DWC_DDRPHYA_APBONLY0_MicroReset
//
//// 3.   Wait for the training firmware to complete by following the procedure in "uCtrl Initialization and Mailbox Messaging"
//// 4.3.7  3.   Wait for the training firmware to complete.  Implement timeout function or follow the procedure in "3.4 Running the firmware" of the Training Firmware Application Note to poll the Mailbox message.
dwc_ddrphy_phyinit_userCustom_G_waitFwDone(sdrammc);

//// [dwc_ddrphy_phyinit_userCustom_G_waitFwDone] End of dwc_ddrphy_phyinit_userCustom_G_waitFwDone()
//// 4.   Halt the microcontroller."
dwc_ddrphy_apb_wr(0xd0099, 0x1); // DWC_DDRPHYA_APBONLY0_MicroReset
dwc_ddrphy_apb_wr(0x20089, 0x0); // DWC_DDRPHYA_MASTER0_base0_CalZap
//// [dwc_ddrphy_phyinit_G_execFW] End of dwc_ddrphy_phyinit_G_execFW()
//
//
////##############################################################
////
//// 4.3.8(H) Read the Message Block results
////
//// The procedure is as follows:
////
////##############################################################
//
//
//// 1.	Enable access to the internal CSRs by setting the MicroContMuxSel CSR to 0.
dwc_ddrphy_apb_wr(0xd0000, 0x0); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
//// [phyinit_userCustom_wait] Wait 40 DfiClks
//
//2. Read the Firmware Message Block to obtain the results from the training.
//This can be accomplished by issuing APB read commands to the DMEM addresses.
//Example:
//if (Train2D)
//{
//  _read_2d_message_block_outputs_
//}
//else
//{
//  _read_1d_message_block_outputs_
//}
//This can be accomplished by issuing APB read commands to the DMEM addresses.
dwc_ddrphy_phyinit_userCustom_H_readMsgBlock(sdrammc, 0);

//[dwc_ddrphy_phyinit_userCustom_H_readMsgBlock] End of dwc_ddrphy_phyinit_userCustom_H_readMsgBlock()
//// 3.	Isolate the APB access from the internal CSRs by setting the MicroContMuxSel CSR to 1.
dwc_ddrphy_apb_wr(0xd0000, 0x1); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
//// [phyinit_userCustom_wait] Wait 40 DfiClks
//// 4.	If training is required at another frequency, repeat the operations starting at step (E).
//// [dwc_ddrphy_phyinit_H_readMsgBlock] End of dwc_ddrphy_phyinit_H_readMsgBlock()
//// [initRuntimeConfigEnableBits] Start of initRuntimeConfigEnableBits()
//// [initRuntimeConfigEnableBits] enableBits[0] = 0x00000009
//// [initRuntimeConfigEnableBits] WR_RD_RTT_PARK_A0 = 0x00000000, rtt_required = 0x00000001
//// [initRuntimeConfigEnableBits] WR_RD_RTT_PARK_A1 = 0x00000000, rtt_required = 0x00000002
//// [initRuntimeConfigEnableBits] WR_RD_RTT_PARK_A2 = 0x00000000, rtt_required = 0x00000004
//// [initRuntimeConfigEnableBits] WR_RD_RTT_PARK_A3 = 0x00000000, rtt_required = 0x00000008
//// [initRuntimeConfigEnableBits] enableBits[1] = 0x00000000
//// [initRuntimeConfigEnableBits] WR_RD_RTT_PARK_B0 = 0x00000000, rtt_required = 0x00000001
//// [initRuntimeConfigEnableBits] WR_RD_RTT_PARK_B1 = 0x00000000, rtt_required = 0x00000002
//// [initRuntimeConfigEnableBits] WR_RD_RTT_PARK_B2 = 0x00000000, rtt_required = 0x00000004
//// [initRuntimeConfigEnableBits] WR_RD_RTT_PARK_B3 = 0x00000000, rtt_required = 0x00000008
//// [initRuntimeConfigEnableBits] enableBits[2] = 0x00000000
//// [initRuntimeConfigEnableBits] End of initRuntimeConfigEnableBits()
//// [phyinit_I_loadPIEImage] Start of dwc_ddrphy_phyinit_I_loadPIEImage()
//
//
////##############################################################
////
//// 4.3.9(I) Load PHY Init Engine Image
////
//// Load the PHY Initialization Engine memory with the provided initialization sequence.
////
////##############################################################
//
//
//// Enable access to the internal CSRs by setting the MicroContMuxSel CSR to 0.
//// This allows the memory controller unrestricted access to the configuration CSRs.
dwc_ddrphy_apb_wr(0xd0000, 0x0); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
//// [phyinit_userCustom_wait] Wait 40 DfiClks
//// [phyinit_I_loadPIEImage] Programming ForceClkGaterEnables::ForcePubDxClkEnLow to 0x1
dwc_ddrphy_apb_wr(0x200a6, 0x2); // DWC_DDRPHYA_MASTER0_base0_ForceClkGaterEnables
//// [phyinit_I_loadPIEImage] Programming PIE Production Code
//// [phyinit_LoadPIECodeSections] Start of dwc_ddrphy_phyinit_LoadPIECodeSections()
//// [phyinit_LoadPIECodeSections] Moving start address from 0 to 90000
dwc_ddrphy_apb_wr(0x90000, 0x10); // DWC_DDRPHYA_INITENG0_base0_PreSequenceReg0b0s0
dwc_ddrphy_apb_wr(0x90001, 0x400); // DWC_DDRPHYA_INITENG0_base0_PreSequenceReg0b0s1
dwc_ddrphy_apb_wr(0x90002, 0x10e); // DWC_DDRPHYA_INITENG0_base0_PreSequenceReg0b0s2
dwc_ddrphy_apb_wr(0x90003, 0x0); // DWC_DDRPHYA_INITENG0_base0_PreSequenceReg0b1s0
dwc_ddrphy_apb_wr(0x90004, 0x0); // DWC_DDRPHYA_INITENG0_base0_PreSequenceReg0b1s1
dwc_ddrphy_apb_wr(0x90005, 0x8); // DWC_DDRPHYA_INITENG0_base0_PreSequenceReg0b1s2
//// [phyinit_LoadPIECodeSections] Moving start address from 90006 to 41000
dwc_ddrphy_apb_wr(0x41000, 0x3fff);
dwc_ddrphy_apb_wr(0x41001, 0xff00);
dwc_ddrphy_apb_wr(0x41002, 0x3f);
dwc_ddrphy_apb_wr(0x41003, 0x2c1);
dwc_ddrphy_apb_wr(0x41004, 0x3fff);
dwc_ddrphy_apb_wr(0x41005, 0xff00);
dwc_ddrphy_apb_wr(0x41006, 0x3f);
dwc_ddrphy_apb_wr(0x41007, 0xa01);
dwc_ddrphy_apb_wr(0x41008, 0x3fff);
dwc_ddrphy_apb_wr(0x41009, 0xff00);
dwc_ddrphy_apb_wr(0x4100a, 0x3f);
dwc_ddrphy_apb_wr(0x4100b, 0x1);
dwc_ddrphy_apb_wr(0x4100c, 0xffff);
dwc_ddrphy_apb_wr(0x4100d, 0xff03);
dwc_ddrphy_apb_wr(0x4100e, 0x3ff);
dwc_ddrphy_apb_wr(0x4100f, 0x0);
dwc_ddrphy_apb_wr(0x41010, 0xffff);
dwc_ddrphy_apb_wr(0x41011, 0xff03);
dwc_ddrphy_apb_wr(0x41012, 0x3ff);
dwc_ddrphy_apb_wr(0x41013, 0x1c1);
dwc_ddrphy_apb_wr(0x41014, 0xffff);
dwc_ddrphy_apb_wr(0x41015, 0xff03);
dwc_ddrphy_apb_wr(0x41016, 0x3ff);
dwc_ddrphy_apb_wr(0x41017, 0x1);
dwc_ddrphy_apb_wr(0x41018, 0xffff);
dwc_ddrphy_apb_wr(0x41019, 0xff03);
dwc_ddrphy_apb_wr(0x4101a, 0x3ff);
dwc_ddrphy_apb_wr(0x4101b, 0x2c1);
dwc_ddrphy_apb_wr(0x4101c, 0xffff);
dwc_ddrphy_apb_wr(0x4101d, 0xff03);
dwc_ddrphy_apb_wr(0x4101e, 0x3ff);
dwc_ddrphy_apb_wr(0x4101f, 0x101);
dwc_ddrphy_apb_wr(0x41020, 0x3fff);
dwc_ddrphy_apb_wr(0x41021, 0xff00);
dwc_ddrphy_apb_wr(0x41022, 0x3f);
dwc_ddrphy_apb_wr(0x41023, 0x1);
dwc_ddrphy_apb_wr(0x41024, 0x3fff);
dwc_ddrphy_apb_wr(0x41025, 0xff00);
dwc_ddrphy_apb_wr(0x41026, 0x3ff);
dwc_ddrphy_apb_wr(0x41027, 0x1);
dwc_ddrphy_apb_wr(0x41028, 0xffff);
dwc_ddrphy_apb_wr(0x41029, 0xff03);
dwc_ddrphy_apb_wr(0x4102a, 0x3ff);
dwc_ddrphy_apb_wr(0x4102b, 0x2c1);
dwc_ddrphy_apb_wr(0x4102c, 0xffff);
dwc_ddrphy_apb_wr(0x4102d, 0xff03);
dwc_ddrphy_apb_wr(0x4102e, 0x3ff);
dwc_ddrphy_apb_wr(0x4102f, 0xf901);
dwc_ddrphy_apb_wr(0x41030, 0xffff);
dwc_ddrphy_apb_wr(0x41031, 0xff03);
dwc_ddrphy_apb_wr(0x41032, 0x3ff);
dwc_ddrphy_apb_wr(0x41033, 0x2c1);
dwc_ddrphy_apb_wr(0x41034, 0xffff);
dwc_ddrphy_apb_wr(0x41035, 0xff03);
dwc_ddrphy_apb_wr(0x41036, 0x3ff);
dwc_ddrphy_apb_wr(0x41037, 0x5901);
dwc_ddrphy_apb_wr(0x41038, 0x5a5);
dwc_ddrphy_apb_wr(0x41039, 0x4000);
dwc_ddrphy_apb_wr(0x4103a, 0x3c0);
dwc_ddrphy_apb_wr(0x4103b, 0x1);
dwc_ddrphy_apb_wr(0x4103c, 0xc000);
dwc_ddrphy_apb_wr(0x4103d, 0x3);
dwc_ddrphy_apb_wr(0x4103e, 0x3c0);
dwc_ddrphy_apb_wr(0x4103f, 0x0);
dwc_ddrphy_apb_wr(0x41040, 0xc000);
dwc_ddrphy_apb_wr(0x41041, 0x3);
dwc_ddrphy_apb_wr(0x41042, 0x3c0);
dwc_ddrphy_apb_wr(0x41043, 0x2c1);
dwc_ddrphy_apb_wr(0x41044, 0xc000);
dwc_ddrphy_apb_wr(0x41045, 0x3);
dwc_ddrphy_apb_wr(0x41046, 0x3c0);
dwc_ddrphy_apb_wr(0x41047, 0xa01);
dwc_ddrphy_apb_wr(0x41048, 0xef);
dwc_ddrphy_apb_wr(0x41049, 0xef00);
dwc_ddrphy_apb_wr(0x4104a, 0x3c0);
dwc_ddrphy_apb_wr(0x4104b, 0x1);
dwc_ddrphy_apb_wr(0x4104c, 0xc000);
dwc_ddrphy_apb_wr(0x4104d, 0x3);
dwc_ddrphy_apb_wr(0x4104e, 0x3c0);
dwc_ddrphy_apb_wr(0x4104f, 0x0);
dwc_ddrphy_apb_wr(0x41050, 0xc000);
dwc_ddrphy_apb_wr(0x41051, 0x3);
dwc_ddrphy_apb_wr(0x41052, 0x3c0);
dwc_ddrphy_apb_wr(0x41053, 0x2c1);
dwc_ddrphy_apb_wr(0x41054, 0xc000);
dwc_ddrphy_apb_wr(0x41055, 0x3);
dwc_ddrphy_apb_wr(0x41056, 0x3c0);
dwc_ddrphy_apb_wr(0x41057, 0xff01);
dwc_ddrphy_apb_wr(0x41058, 0xc000);
dwc_ddrphy_apb_wr(0x41059, 0x3);
dwc_ddrphy_apb_wr(0x4105a, 0x3c0);
dwc_ddrphy_apb_wr(0x4105b, 0x2c1);
dwc_ddrphy_apb_wr(0x4105c, 0xc000);
dwc_ddrphy_apb_wr(0x4105d, 0x3);
dwc_ddrphy_apb_wr(0x4105e, 0x3c0);
dwc_ddrphy_apb_wr(0x4105f, 0xff01);
dwc_ddrphy_apb_wr(0x41060, 0xc000);
dwc_ddrphy_apb_wr(0x41061, 0x3);
dwc_ddrphy_apb_wr(0x41062, 0x3c0);
dwc_ddrphy_apb_wr(0x41063, 0x2c1);
dwc_ddrphy_apb_wr(0x41064, 0xc000);
dwc_ddrphy_apb_wr(0x41065, 0x3);
dwc_ddrphy_apb_wr(0x41066, 0x3c0);
dwc_ddrphy_apb_wr(0x41067, 0xa01);
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 200, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 400, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 800, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 200000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 400000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 800000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10, type = 0
//// [phyinit_LoadPIECodeSections] Matched NO enable_bits = 4, type = 0
dwc_ddrphy_apb_wr(0x41068, 0x85d5);
dwc_ddrphy_apb_wr(0x41069, 0x63);
dwc_ddrphy_apb_wr(0x4106a, 0x3c0);
dwc_ddrphy_apb_wr(0x4106b, 0x400);
dwc_ddrphy_apb_wr(0x4106c, 0xc000);
dwc_ddrphy_apb_wr(0x4106d, 0x3);
dwc_ddrphy_apb_wr(0x4106e, 0x3c0);
dwc_ddrphy_apb_wr(0x4106f, 0x0);
dwc_ddrphy_apb_wr(0x41070, 0xc000);
dwc_ddrphy_apb_wr(0x41071, 0x3);
dwc_ddrphy_apb_wr(0x41072, 0x3c0);
dwc_ddrphy_apb_wr(0x41073, 0x2c1);
dwc_ddrphy_apb_wr(0x41074, 0xc000);
dwc_ddrphy_apb_wr(0x41075, 0x3);
dwc_ddrphy_apb_wr(0x41076, 0x3c0);
dwc_ddrphy_apb_wr(0x41077, 0x1001);
dwc_ddrphy_apb_wr(0x41078, 0x85f5);
dwc_ddrphy_apb_wr(0x41079, 0x63);
dwc_ddrphy_apb_wr(0x4107a, 0x3c0);
dwc_ddrphy_apb_wr(0x4107b, 0x800);
dwc_ddrphy_apb_wr(0x4107c, 0xc000);
dwc_ddrphy_apb_wr(0x4107d, 0x3);
dwc_ddrphy_apb_wr(0x4107e, 0x3c0);
dwc_ddrphy_apb_wr(0x4107f, 0x0);
dwc_ddrphy_apb_wr(0x41080, 0xc000);
dwc_ddrphy_apb_wr(0x41081, 0x3);
dwc_ddrphy_apb_wr(0x41082, 0x3c0);
dwc_ddrphy_apb_wr(0x41083, 0x2c1);
dwc_ddrphy_apb_wr(0x41084, 0xc000);
dwc_ddrphy_apb_wr(0x41085, 0x3);
dwc_ddrphy_apb_wr(0x41086, 0x3c0);
dwc_ddrphy_apb_wr(0x41087, 0x1001);
dwc_ddrphy_apb_wr(0x41088, 0x45d5);
dwc_ddrphy_apb_wr(0x41089, 0x63);
dwc_ddrphy_apb_wr(0x4108a, 0x3c0);
dwc_ddrphy_apb_wr(0x4108b, 0x401);
dwc_ddrphy_apb_wr(0x4108c, 0xc000);
dwc_ddrphy_apb_wr(0x4108d, 0x3);
dwc_ddrphy_apb_wr(0x4108e, 0x3c0);
dwc_ddrphy_apb_wr(0x4108f, 0x1);
dwc_ddrphy_apb_wr(0x41090, 0xc000);
dwc_ddrphy_apb_wr(0x41091, 0x3);
dwc_ddrphy_apb_wr(0x41092, 0x3c0);
dwc_ddrphy_apb_wr(0x41093, 0x2c1);
dwc_ddrphy_apb_wr(0x41094, 0xc000);
dwc_ddrphy_apb_wr(0x41095, 0x3);
dwc_ddrphy_apb_wr(0x41096, 0x3c0);
dwc_ddrphy_apb_wr(0x41097, 0x1001);
dwc_ddrphy_apb_wr(0x41098, 0x45f5);
dwc_ddrphy_apb_wr(0x41099, 0x63);
dwc_ddrphy_apb_wr(0x4109a, 0x3c0);
dwc_ddrphy_apb_wr(0x4109b, 0x801);
dwc_ddrphy_apb_wr(0x4109c, 0xc000);
dwc_ddrphy_apb_wr(0x4109d, 0x3);
dwc_ddrphy_apb_wr(0x4109e, 0x3c0);
dwc_ddrphy_apb_wr(0x4109f, 0x1);
dwc_ddrphy_apb_wr(0x410a0, 0xc000);
dwc_ddrphy_apb_wr(0x410a1, 0x3);
dwc_ddrphy_apb_wr(0x410a2, 0x3c0);
dwc_ddrphy_apb_wr(0x410a3, 0x2c1);
dwc_ddrphy_apb_wr(0x410a4, 0xc000);
dwc_ddrphy_apb_wr(0x410a5, 0x3);
dwc_ddrphy_apb_wr(0x410a6, 0x3c0);
dwc_ddrphy_apb_wr(0x410a7, 0x1001);
dwc_ddrphy_apb_wr(0x410a8, 0xc5d5);
dwc_ddrphy_apb_wr(0x410a9, 0x62);
dwc_ddrphy_apb_wr(0x410aa, 0x3c0);
dwc_ddrphy_apb_wr(0x410ab, 0x402);
dwc_ddrphy_apb_wr(0x410ac, 0xc000);
dwc_ddrphy_apb_wr(0x410ad, 0x3);
dwc_ddrphy_apb_wr(0x410ae, 0x3c0);
dwc_ddrphy_apb_wr(0x410af, 0x2);
dwc_ddrphy_apb_wr(0x410b0, 0xc000);
dwc_ddrphy_apb_wr(0x410b1, 0x3);
dwc_ddrphy_apb_wr(0x410b2, 0x3c0);
dwc_ddrphy_apb_wr(0x410b3, 0x2c1);
dwc_ddrphy_apb_wr(0x410b4, 0xc000);
dwc_ddrphy_apb_wr(0x410b5, 0x3);
dwc_ddrphy_apb_wr(0x410b6, 0x3c0);
dwc_ddrphy_apb_wr(0x410b7, 0x1001);
dwc_ddrphy_apb_wr(0x410b8, 0xc5f5);
dwc_ddrphy_apb_wr(0x410b9, 0x62);
dwc_ddrphy_apb_wr(0x410ba, 0x3c0);
dwc_ddrphy_apb_wr(0x410bb, 0x802);
dwc_ddrphy_apb_wr(0x410bc, 0xc000);
dwc_ddrphy_apb_wr(0x410bd, 0x3);
dwc_ddrphy_apb_wr(0x410be, 0x3c0);
dwc_ddrphy_apb_wr(0x410bf, 0x2);
dwc_ddrphy_apb_wr(0x410c0, 0xc000);
dwc_ddrphy_apb_wr(0x410c1, 0x3);
dwc_ddrphy_apb_wr(0x410c2, 0x3c0);
dwc_ddrphy_apb_wr(0x410c3, 0x2c1);
dwc_ddrphy_apb_wr(0x410c4, 0xc000);
dwc_ddrphy_apb_wr(0x410c5, 0x3);
dwc_ddrphy_apb_wr(0x410c6, 0x3c0);
dwc_ddrphy_apb_wr(0x410c7, 0x1001);
dwc_ddrphy_apb_wr(0x410c8, 0xc5d5);
dwc_ddrphy_apb_wr(0x410c9, 0x61);
dwc_ddrphy_apb_wr(0x410ca, 0x3c0);
dwc_ddrphy_apb_wr(0x410cb, 0x403);
dwc_ddrphy_apb_wr(0x410cc, 0xc000);
dwc_ddrphy_apb_wr(0x410cd, 0x3);
dwc_ddrphy_apb_wr(0x410ce, 0x3c0);
dwc_ddrphy_apb_wr(0x410cf, 0x3);
dwc_ddrphy_apb_wr(0x410d0, 0xc000);
dwc_ddrphy_apb_wr(0x410d1, 0x3);
dwc_ddrphy_apb_wr(0x410d2, 0x3c0);
dwc_ddrphy_apb_wr(0x410d3, 0x2c1);
dwc_ddrphy_apb_wr(0x410d4, 0xc000);
dwc_ddrphy_apb_wr(0x410d5, 0x3);
dwc_ddrphy_apb_wr(0x410d6, 0x3c0);
dwc_ddrphy_apb_wr(0x410d7, 0x1001);
dwc_ddrphy_apb_wr(0x410d8, 0xc5f5);
dwc_ddrphy_apb_wr(0x410d9, 0x61);
dwc_ddrphy_apb_wr(0x410da, 0x3c0);
dwc_ddrphy_apb_wr(0x410db, 0x803);
dwc_ddrphy_apb_wr(0x410dc, 0xc000);
dwc_ddrphy_apb_wr(0x410dd, 0x3);
dwc_ddrphy_apb_wr(0x410de, 0x3c0);
dwc_ddrphy_apb_wr(0x410df, 0x3);
dwc_ddrphy_apb_wr(0x410e0, 0xc000);
dwc_ddrphy_apb_wr(0x410e1, 0x3);
dwc_ddrphy_apb_wr(0x410e2, 0x3c0);
dwc_ddrphy_apb_wr(0x410e3, 0x2c1);
dwc_ddrphy_apb_wr(0x410e4, 0xc000);
dwc_ddrphy_apb_wr(0x410e5, 0x3);
dwc_ddrphy_apb_wr(0x410e6, 0x3c0);
dwc_ddrphy_apb_wr(0x410e7, 0x1d01);
//// [phyinit_LoadPIECodeSections] Matched NO enable_bits = 2, type = 0
dwc_ddrphy_apb_wr(0x410e8, 0x213);
dwc_ddrphy_apb_wr(0x410e9, 0x0);
dwc_ddrphy_apb_wr(0x410ea, 0x3c0);
dwc_ddrphy_apb_wr(0x410eb, 0x1);
dwc_ddrphy_apb_wr(0x410ec, 0xc000);
dwc_ddrphy_apb_wr(0x410ed, 0x3);
dwc_ddrphy_apb_wr(0x410ee, 0x3c0);
dwc_ddrphy_apb_wr(0x410ef, 0x0);
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100, type = 0
dwc_ddrphy_apb_wr(0x410f0, 0xc000);
dwc_ddrphy_apb_wr(0x410f1, 0x3);
dwc_ddrphy_apb_wr(0x410f2, 0x3c0);
dwc_ddrphy_apb_wr(0x410f3, 0x2c1);
dwc_ddrphy_apb_wr(0x410f4, 0xc000);
dwc_ddrphy_apb_wr(0x410f5, 0x3);
dwc_ddrphy_apb_wr(0x410f6, 0x3c0);
dwc_ddrphy_apb_wr(0x410f7, 0xef00);
dwc_ddrphy_apb_wr(0x410f8, 0xc000);
dwc_ddrphy_apb_wr(0x410f9, 0x3);
dwc_ddrphy_apb_wr(0x410fa, 0x3c0);
dwc_ddrphy_apb_wr(0x410fb, 0x2c1);
dwc_ddrphy_apb_wr(0x410fc, 0xc000);
dwc_ddrphy_apb_wr(0x410fd, 0x3);
dwc_ddrphy_apb_wr(0x410fe, 0x3c0);
dwc_ddrphy_apb_wr(0x410ff, 0x5900);
dwc_ddrphy_apb_wr(0x41100, 0x217);
dwc_ddrphy_apb_wr(0x41101, 0x1700);
dwc_ddrphy_apb_wr(0x41102, 0x3c2);
dwc_ddrphy_apb_wr(0x41103, 0x1);
dwc_ddrphy_apb_wr(0x41104, 0xc000);
dwc_ddrphy_apb_wr(0x41105, 0x3);
dwc_ddrphy_apb_wr(0x41106, 0x3c0);
dwc_ddrphy_apb_wr(0x41107, 0x0);
dwc_ddrphy_apb_wr(0x41108, 0xc000);
dwc_ddrphy_apb_wr(0x41109, 0x3);
dwc_ddrphy_apb_wr(0x4110a, 0x3c0);
dwc_ddrphy_apb_wr(0x4110b, 0x2c1);
dwc_ddrphy_apb_wr(0x4110c, 0xc000);
dwc_ddrphy_apb_wr(0x4110d, 0x3);
dwc_ddrphy_apb_wr(0x4110e, 0x3c0);
dwc_ddrphy_apb_wr(0x4110f, 0x400);
dwc_ddrphy_apb_wr(0x41110, 0x3fff);
dwc_ddrphy_apb_wr(0x41111, 0xff00);
dwc_ddrphy_apb_wr(0x41112, 0x3f);
dwc_ddrphy_apb_wr(0x41113, 0x2e1);
dwc_ddrphy_apb_wr(0x41114, 0x3fff);
dwc_ddrphy_apb_wr(0x41115, 0xff00);
dwc_ddrphy_apb_wr(0x41116, 0x3f);
dwc_ddrphy_apb_wr(0x41117, 0xa21);
dwc_ddrphy_apb_wr(0x41118, 0x3fff);
dwc_ddrphy_apb_wr(0x41119, 0xff00);
dwc_ddrphy_apb_wr(0x4111a, 0x3f);
dwc_ddrphy_apb_wr(0x4111b, 0x21);
dwc_ddrphy_apb_wr(0x4111c, 0xffff);
dwc_ddrphy_apb_wr(0x4111d, 0xff03);
dwc_ddrphy_apb_wr(0x4111e, 0x3ff);
dwc_ddrphy_apb_wr(0x4111f, 0x20);
dwc_ddrphy_apb_wr(0x41120, 0xffff);
dwc_ddrphy_apb_wr(0x41121, 0xff03);
dwc_ddrphy_apb_wr(0x41122, 0x3ff);
dwc_ddrphy_apb_wr(0x41123, 0x1e1);
dwc_ddrphy_apb_wr(0x41124, 0xffff);
dwc_ddrphy_apb_wr(0x41125, 0xff03);
dwc_ddrphy_apb_wr(0x41126, 0x3ff);
dwc_ddrphy_apb_wr(0x41127, 0x21);
dwc_ddrphy_apb_wr(0x41128, 0xffff);
dwc_ddrphy_apb_wr(0x41129, 0xff03);
dwc_ddrphy_apb_wr(0x4112a, 0x3ff);
dwc_ddrphy_apb_wr(0x4112b, 0x2e1);
dwc_ddrphy_apb_wr(0x4112c, 0xffff);
dwc_ddrphy_apb_wr(0x4112d, 0xff03);
dwc_ddrphy_apb_wr(0x4112e, 0x3ff);
dwc_ddrphy_apb_wr(0x4112f, 0x121);
dwc_ddrphy_apb_wr(0x41130, 0x3fff);
dwc_ddrphy_apb_wr(0x41131, 0xff00);
dwc_ddrphy_apb_wr(0x41132, 0x3ff);
dwc_ddrphy_apb_wr(0x41133, 0x21);
dwc_ddrphy_apb_wr(0x41134, 0x3fff);
dwc_ddrphy_apb_wr(0x41135, 0xff00);
dwc_ddrphy_apb_wr(0x41136, 0x3ff);
dwc_ddrphy_apb_wr(0x41137, 0x21);
dwc_ddrphy_apb_wr(0x41138, 0x3fff);
dwc_ddrphy_apb_wr(0x41139, 0xff00);
dwc_ddrphy_apb_wr(0x4113a, 0x3ff);
dwc_ddrphy_apb_wr(0x4113b, 0x21);
dwc_ddrphy_apb_wr(0x4113c, 0xffff);
dwc_ddrphy_apb_wr(0x4113d, 0xff03);
dwc_ddrphy_apb_wr(0x4113e, 0x3ff);
dwc_ddrphy_apb_wr(0x4113f, 0x21);
dwc_ddrphy_apb_wr(0x41140, 0xffff);
dwc_ddrphy_apb_wr(0x41141, 0xff03);
dwc_ddrphy_apb_wr(0x41142, 0x3ff);
dwc_ddrphy_apb_wr(0x41143, 0x2e1);
dwc_ddrphy_apb_wr(0x41144, 0xffff);
dwc_ddrphy_apb_wr(0x41145, 0xff03);
dwc_ddrphy_apb_wr(0x41146, 0x3ff);
dwc_ddrphy_apb_wr(0x41147, 0xf921);
dwc_ddrphy_apb_wr(0x41148, 0xffff);
dwc_ddrphy_apb_wr(0x41149, 0xff03);
dwc_ddrphy_apb_wr(0x4114a, 0x3ff);
dwc_ddrphy_apb_wr(0x4114b, 0x2e1);
dwc_ddrphy_apb_wr(0x4114c, 0xffff);
dwc_ddrphy_apb_wr(0x4114d, 0xff03);
dwc_ddrphy_apb_wr(0x4114e, 0x3ff);
dwc_ddrphy_apb_wr(0x4114f, 0x5921);
dwc_ddrphy_apb_wr(0x41150, 0x5a5);
dwc_ddrphy_apb_wr(0x41151, 0xa500);
dwc_ddrphy_apb_wr(0x41152, 0x3c5);
dwc_ddrphy_apb_wr(0x41153, 0x21);
dwc_ddrphy_apb_wr(0x41154, 0xc040);
dwc_ddrphy_apb_wr(0x41155, 0x4003);
dwc_ddrphy_apb_wr(0x41156, 0x3c0);
dwc_ddrphy_apb_wr(0x41157, 0x20);
dwc_ddrphy_apb_wr(0x41158, 0xc000);
dwc_ddrphy_apb_wr(0x41159, 0x3);
dwc_ddrphy_apb_wr(0x4115a, 0x3c0);
dwc_ddrphy_apb_wr(0x4115b, 0x2e1);
dwc_ddrphy_apb_wr(0x4115c, 0xc000);
dwc_ddrphy_apb_wr(0x4115d, 0x3);
dwc_ddrphy_apb_wr(0x4115e, 0x3c0);
dwc_ddrphy_apb_wr(0x4115f, 0xa21);
dwc_ddrphy_apb_wr(0x41160, 0xef);
dwc_ddrphy_apb_wr(0x41161, 0xef00);
dwc_ddrphy_apb_wr(0x41162, 0x3c0);
dwc_ddrphy_apb_wr(0x41163, 0x21);
dwc_ddrphy_apb_wr(0x41164, 0xc000);
dwc_ddrphy_apb_wr(0x41165, 0x3);
dwc_ddrphy_apb_wr(0x41166, 0x3c0);
dwc_ddrphy_apb_wr(0x41167, 0x20);
dwc_ddrphy_apb_wr(0x41168, 0xc000);
dwc_ddrphy_apb_wr(0x41169, 0x3);
dwc_ddrphy_apb_wr(0x4116a, 0x3c0);
dwc_ddrphy_apb_wr(0x4116b, 0x2e1);
dwc_ddrphy_apb_wr(0x4116c, 0xc000);
dwc_ddrphy_apb_wr(0x4116d, 0x3);
dwc_ddrphy_apb_wr(0x4116e, 0x3c0);
dwc_ddrphy_apb_wr(0x4116f, 0xff21);
dwc_ddrphy_apb_wr(0x41170, 0xc000);
dwc_ddrphy_apb_wr(0x41171, 0x3);
dwc_ddrphy_apb_wr(0x41172, 0x3c0);
dwc_ddrphy_apb_wr(0x41173, 0x2e1);
dwc_ddrphy_apb_wr(0x41174, 0xc000);
dwc_ddrphy_apb_wr(0x41175, 0x3);
dwc_ddrphy_apb_wr(0x41176, 0x3c0);
dwc_ddrphy_apb_wr(0x41177, 0xff21);
dwc_ddrphy_apb_wr(0x41178, 0xc000);
dwc_ddrphy_apb_wr(0x41179, 0x3);
dwc_ddrphy_apb_wr(0x4117a, 0x3c0);
dwc_ddrphy_apb_wr(0x4117b, 0x2e1);
dwc_ddrphy_apb_wr(0x4117c, 0xc000);
dwc_ddrphy_apb_wr(0x4117d, 0x3);
dwc_ddrphy_apb_wr(0x4117e, 0x3c0);
dwc_ddrphy_apb_wr(0x4117f, 0xa21);
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 200, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 400, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 800, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 200000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 400000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 800000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80000000, type = 1
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10, type = 0
//// [phyinit_LoadPIECodeSections] Matched NO enable_bits = 4, type = 0
dwc_ddrphy_apb_wr(0x41180, 0x85d5);
dwc_ddrphy_apb_wr(0x41181, 0xd563);
dwc_ddrphy_apb_wr(0x41182, 0x3c5);
dwc_ddrphy_apb_wr(0x41183, 0x420);
dwc_ddrphy_apb_wr(0x41184, 0xc000);
dwc_ddrphy_apb_wr(0x41185, 0x3);
dwc_ddrphy_apb_wr(0x41186, 0x3c0);
dwc_ddrphy_apb_wr(0x41187, 0x20);
dwc_ddrphy_apb_wr(0x41188, 0xc000);
dwc_ddrphy_apb_wr(0x41189, 0x3);
dwc_ddrphy_apb_wr(0x4118a, 0x3c0);
dwc_ddrphy_apb_wr(0x4118b, 0x2c1);
dwc_ddrphy_apb_wr(0x4118c, 0xc000);
dwc_ddrphy_apb_wr(0x4118d, 0x3);
dwc_ddrphy_apb_wr(0x4118e, 0x3c0);
dwc_ddrphy_apb_wr(0x4118f, 0x1001);
dwc_ddrphy_apb_wr(0x41190, 0x85f5);
dwc_ddrphy_apb_wr(0x41191, 0xf563);
dwc_ddrphy_apb_wr(0x41192, 0x3c5);
dwc_ddrphy_apb_wr(0x41193, 0x820);
dwc_ddrphy_apb_wr(0x41194, 0xc000);
dwc_ddrphy_apb_wr(0x41195, 0x3);
dwc_ddrphy_apb_wr(0x41196, 0x3c0);
dwc_ddrphy_apb_wr(0x41197, 0x20);
dwc_ddrphy_apb_wr(0x41198, 0xc000);
dwc_ddrphy_apb_wr(0x41199, 0x3);
dwc_ddrphy_apb_wr(0x4119a, 0x3c0);
dwc_ddrphy_apb_wr(0x4119b, 0x2c1);
dwc_ddrphy_apb_wr(0x4119c, 0xc000);
dwc_ddrphy_apb_wr(0x4119d, 0x3);
dwc_ddrphy_apb_wr(0x4119e, 0x3c0);
dwc_ddrphy_apb_wr(0x4119f, 0x1001);
dwc_ddrphy_apb_wr(0x411a0, 0x45d5);
dwc_ddrphy_apb_wr(0x411a1, 0xd563);
dwc_ddrphy_apb_wr(0x411a2, 0x3c5);
dwc_ddrphy_apb_wr(0x411a3, 0x421);
dwc_ddrphy_apb_wr(0x411a4, 0xc000);
dwc_ddrphy_apb_wr(0x411a5, 0x3);
dwc_ddrphy_apb_wr(0x411a6, 0x3c0);
dwc_ddrphy_apb_wr(0x411a7, 0x21);
dwc_ddrphy_apb_wr(0x411a8, 0xc000);
dwc_ddrphy_apb_wr(0x411a9, 0x3);
dwc_ddrphy_apb_wr(0x411aa, 0x3c0);
dwc_ddrphy_apb_wr(0x411ab, 0x2c1);
dwc_ddrphy_apb_wr(0x411ac, 0xc000);
dwc_ddrphy_apb_wr(0x411ad, 0x3);
dwc_ddrphy_apb_wr(0x411ae, 0x3c0);
dwc_ddrphy_apb_wr(0x411af, 0x1001);
dwc_ddrphy_apb_wr(0x411b0, 0x45f5);
dwc_ddrphy_apb_wr(0x411b1, 0xf563);
dwc_ddrphy_apb_wr(0x411b2, 0x3c5);
dwc_ddrphy_apb_wr(0x411b3, 0x821);
dwc_ddrphy_apb_wr(0x411b4, 0xc000);
dwc_ddrphy_apb_wr(0x411b5, 0x3);
dwc_ddrphy_apb_wr(0x411b6, 0x3c0);
dwc_ddrphy_apb_wr(0x411b7, 0x21);
dwc_ddrphy_apb_wr(0x411b8, 0xc000);
dwc_ddrphy_apb_wr(0x411b9, 0x3);
dwc_ddrphy_apb_wr(0x411ba, 0x3c0);
dwc_ddrphy_apb_wr(0x411bb, 0x2c1);
dwc_ddrphy_apb_wr(0x411bc, 0xc000);
dwc_ddrphy_apb_wr(0x411bd, 0x3);
dwc_ddrphy_apb_wr(0x411be, 0x3c0);
dwc_ddrphy_apb_wr(0x411bf, 0x1001);
dwc_ddrphy_apb_wr(0x411c0, 0xc5d5);
dwc_ddrphy_apb_wr(0x411c1, 0xd562);
dwc_ddrphy_apb_wr(0x411c2, 0x3c5);
dwc_ddrphy_apb_wr(0x411c3, 0x422);
dwc_ddrphy_apb_wr(0x411c4, 0xc000);
dwc_ddrphy_apb_wr(0x411c5, 0x3);
dwc_ddrphy_apb_wr(0x411c6, 0x3c0);
dwc_ddrphy_apb_wr(0x411c7, 0x22);
dwc_ddrphy_apb_wr(0x411c8, 0xc000);
dwc_ddrphy_apb_wr(0x411c9, 0x3);
dwc_ddrphy_apb_wr(0x411ca, 0x3c0);
dwc_ddrphy_apb_wr(0x411cb, 0x2c1);
dwc_ddrphy_apb_wr(0x411cc, 0xc000);
dwc_ddrphy_apb_wr(0x411cd, 0x3);
dwc_ddrphy_apb_wr(0x411ce, 0x3c0);
dwc_ddrphy_apb_wr(0x411cf, 0x1001);
dwc_ddrphy_apb_wr(0x411d0, 0xc5f5);
dwc_ddrphy_apb_wr(0x411d1, 0xf562);
dwc_ddrphy_apb_wr(0x411d2, 0x3c5);
dwc_ddrphy_apb_wr(0x411d3, 0x822);
dwc_ddrphy_apb_wr(0x411d4, 0xc000);
dwc_ddrphy_apb_wr(0x411d5, 0x3);
dwc_ddrphy_apb_wr(0x411d6, 0x3c0);
dwc_ddrphy_apb_wr(0x411d7, 0x22);
dwc_ddrphy_apb_wr(0x411d8, 0xc000);
dwc_ddrphy_apb_wr(0x411d9, 0x3);
dwc_ddrphy_apb_wr(0x411da, 0x3c0);
dwc_ddrphy_apb_wr(0x411db, 0x2c1);
dwc_ddrphy_apb_wr(0x411dc, 0xc000);
dwc_ddrphy_apb_wr(0x411dd, 0x3);
dwc_ddrphy_apb_wr(0x411de, 0x3c0);
dwc_ddrphy_apb_wr(0x411df, 0x1001);
dwc_ddrphy_apb_wr(0x411e0, 0xc5d5);
dwc_ddrphy_apb_wr(0x411e1, 0xd561);
dwc_ddrphy_apb_wr(0x411e2, 0x3c5);
dwc_ddrphy_apb_wr(0x411e3, 0x423);
dwc_ddrphy_apb_wr(0x411e4, 0xc000);
dwc_ddrphy_apb_wr(0x411e5, 0x3);
dwc_ddrphy_apb_wr(0x411e6, 0x3c0);
dwc_ddrphy_apb_wr(0x411e7, 0x23);
dwc_ddrphy_apb_wr(0x411e8, 0xc000);
dwc_ddrphy_apb_wr(0x411e9, 0x3);
dwc_ddrphy_apb_wr(0x411ea, 0x3c0);
dwc_ddrphy_apb_wr(0x411eb, 0x2c1);
dwc_ddrphy_apb_wr(0x411ec, 0xc000);
dwc_ddrphy_apb_wr(0x411ed, 0x3);
dwc_ddrphy_apb_wr(0x411ee, 0x3c0);
dwc_ddrphy_apb_wr(0x411ef, 0x1001);
dwc_ddrphy_apb_wr(0x411f0, 0xc5f5);
dwc_ddrphy_apb_wr(0x411f1, 0xf561);
dwc_ddrphy_apb_wr(0x411f2, 0x3c5);
dwc_ddrphy_apb_wr(0x411f3, 0x823);
dwc_ddrphy_apb_wr(0x411f4, 0xc000);
dwc_ddrphy_apb_wr(0x411f5, 0x3);
dwc_ddrphy_apb_wr(0x411f6, 0x3c0);
dwc_ddrphy_apb_wr(0x411f7, 0x23);
dwc_ddrphy_apb_wr(0x411f8, 0xc000);
dwc_ddrphy_apb_wr(0x411f9, 0x3);
dwc_ddrphy_apb_wr(0x411fa, 0x3c0);
dwc_ddrphy_apb_wr(0x411fb, 0x2c1);
dwc_ddrphy_apb_wr(0x411fc, 0xc000);
dwc_ddrphy_apb_wr(0x411fd, 0x3);
dwc_ddrphy_apb_wr(0x411fe, 0x3c0);
dwc_ddrphy_apb_wr(0x411ff, 0x1d01);
//// [phyinit_LoadPIECodeSections] Matched NO enable_bits = 2, type = 0
dwc_ddrphy_apb_wr(0x41200, 0x213);
dwc_ddrphy_apb_wr(0x41201, 0x1300);
dwc_ddrphy_apb_wr(0x41202, 0x3c2);
dwc_ddrphy_apb_wr(0x41203, 0x21);
dwc_ddrphy_apb_wr(0x41204, 0xc000);
dwc_ddrphy_apb_wr(0x41205, 0x3);
dwc_ddrphy_apb_wr(0x41206, 0x3c0);
dwc_ddrphy_apb_wr(0x41207, 0x20);
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100, type = 0
dwc_ddrphy_apb_wr(0x41208, 0xc000);
dwc_ddrphy_apb_wr(0x41209, 0x3);
dwc_ddrphy_apb_wr(0x4120a, 0x3c0);
dwc_ddrphy_apb_wr(0x4120b, 0x2e1);
dwc_ddrphy_apb_wr(0x4120c, 0xc000);
dwc_ddrphy_apb_wr(0x4120d, 0x3);
dwc_ddrphy_apb_wr(0x4120e, 0x3c0);
dwc_ddrphy_apb_wr(0x4120f, 0xef20);
dwc_ddrphy_apb_wr(0x41210, 0xc000);
dwc_ddrphy_apb_wr(0x41211, 0x3);
dwc_ddrphy_apb_wr(0x41212, 0x3c0);
dwc_ddrphy_apb_wr(0x41213, 0x2e1);
dwc_ddrphy_apb_wr(0x41214, 0xc000);
dwc_ddrphy_apb_wr(0x41215, 0x3);
dwc_ddrphy_apb_wr(0x41216, 0x3c0);
dwc_ddrphy_apb_wr(0x41217, 0x5920);
dwc_ddrphy_apb_wr(0x41218, 0x217);
dwc_ddrphy_apb_wr(0x41219, 0x1700);
dwc_ddrphy_apb_wr(0x4121a, 0x3c2);
dwc_ddrphy_apb_wr(0x4121b, 0x21);
dwc_ddrphy_apb_wr(0x4121c, 0xc000);
dwc_ddrphy_apb_wr(0x4121d, 0x3);
dwc_ddrphy_apb_wr(0x4121e, 0x3c0);
dwc_ddrphy_apb_wr(0x4121f, 0x20);
dwc_ddrphy_apb_wr(0x41220, 0xc000);
dwc_ddrphy_apb_wr(0x41221, 0x3);
dwc_ddrphy_apb_wr(0x41222, 0x3c0);
dwc_ddrphy_apb_wr(0x41223, 0x2e1);
dwc_ddrphy_apb_wr(0x41224, 0xc000);
dwc_ddrphy_apb_wr(0x41225, 0x3);
dwc_ddrphy_apb_wr(0x41226, 0x3c0);
dwc_ddrphy_apb_wr(0x41227, 0x420);
//// [phyinit_LoadPIECodeSections] Moving start address from 41228 to 42000
dwc_ddrphy_apb_wr(0x42000, 0x3fff);
dwc_ddrphy_apb_wr(0x42001, 0xff00);
dwc_ddrphy_apb_wr(0x42002, 0x3f);
dwc_ddrphy_apb_wr(0x42003, 0x2c1);
dwc_ddrphy_apb_wr(0x42004, 0x3fff);
dwc_ddrphy_apb_wr(0x42005, 0xff00);
dwc_ddrphy_apb_wr(0x42006, 0x3f);
dwc_ddrphy_apb_wr(0x42007, 0xa01);
dwc_ddrphy_apb_wr(0x42008, 0x3fff);
dwc_ddrphy_apb_wr(0x42009, 0xff00);
dwc_ddrphy_apb_wr(0x4200a, 0x3f);
dwc_ddrphy_apb_wr(0x4200b, 0x1);
dwc_ddrphy_apb_wr(0x4200c, 0xffff);
dwc_ddrphy_apb_wr(0x4200d, 0xff03);
dwc_ddrphy_apb_wr(0x4200e, 0x3ff);
dwc_ddrphy_apb_wr(0x4200f, 0x0);
dwc_ddrphy_apb_wr(0x42010, 0xffff);
dwc_ddrphy_apb_wr(0x42011, 0xff03);
dwc_ddrphy_apb_wr(0x42012, 0x3ff);
dwc_ddrphy_apb_wr(0x42013, 0x1c1);
dwc_ddrphy_apb_wr(0x42014, 0xffff);
dwc_ddrphy_apb_wr(0x42015, 0xff03);
dwc_ddrphy_apb_wr(0x42016, 0x3ff);
dwc_ddrphy_apb_wr(0x42017, 0x1);
dwc_ddrphy_apb_wr(0x42018, 0xffff);
dwc_ddrphy_apb_wr(0x42019, 0xff03);
dwc_ddrphy_apb_wr(0x4201a, 0x3ff);
dwc_ddrphy_apb_wr(0x4201b, 0x2c1);
dwc_ddrphy_apb_wr(0x4201c, 0xffff);
dwc_ddrphy_apb_wr(0x4201d, 0xff03);
dwc_ddrphy_apb_wr(0x4201e, 0x3ff);
dwc_ddrphy_apb_wr(0x4201f, 0x101);
dwc_ddrphy_apb_wr(0x42020, 0x3fff);
dwc_ddrphy_apb_wr(0x42021, 0xff00);
dwc_ddrphy_apb_wr(0x42022, 0x3f);
dwc_ddrphy_apb_wr(0x42023, 0x1);
dwc_ddrphy_apb_wr(0x42024, 0x3fff);
dwc_ddrphy_apb_wr(0x42025, 0xff00);
dwc_ddrphy_apb_wr(0x42026, 0x3ff);
dwc_ddrphy_apb_wr(0x42027, 0x1);
dwc_ddrphy_apb_wr(0x42028, 0xffff);
dwc_ddrphy_apb_wr(0x42029, 0xff03);
dwc_ddrphy_apb_wr(0x4202a, 0x3ff);
dwc_ddrphy_apb_wr(0x4202b, 0x2c1);
dwc_ddrphy_apb_wr(0x4202c, 0xffff);
dwc_ddrphy_apb_wr(0x4202d, 0xff03);
dwc_ddrphy_apb_wr(0x4202e, 0x3ff);
dwc_ddrphy_apb_wr(0x4202f, 0xf901);
dwc_ddrphy_apb_wr(0x42030, 0xffff);
dwc_ddrphy_apb_wr(0x42031, 0xff03);
dwc_ddrphy_apb_wr(0x42032, 0x3ff);
dwc_ddrphy_apb_wr(0x42033, 0x2c1);
dwc_ddrphy_apb_wr(0x42034, 0xffff);
dwc_ddrphy_apb_wr(0x42035, 0xff03);
dwc_ddrphy_apb_wr(0x42036, 0x3ff);
dwc_ddrphy_apb_wr(0x42037, 0x5901);
dwc_ddrphy_apb_wr(0x42038, 0x5a5);
dwc_ddrphy_apb_wr(0x42039, 0x4000);
dwc_ddrphy_apb_wr(0x4203a, 0x3c0);
dwc_ddrphy_apb_wr(0x4203b, 0x1);
dwc_ddrphy_apb_wr(0x4203c, 0xc000);
dwc_ddrphy_apb_wr(0x4203d, 0x3);
dwc_ddrphy_apb_wr(0x4203e, 0x3c0);
dwc_ddrphy_apb_wr(0x4203f, 0x0);
dwc_ddrphy_apb_wr(0x42040, 0xc000);
dwc_ddrphy_apb_wr(0x42041, 0x3);
dwc_ddrphy_apb_wr(0x42042, 0x3c0);
dwc_ddrphy_apb_wr(0x42043, 0x2c1);
dwc_ddrphy_apb_wr(0x42044, 0xc000);
dwc_ddrphy_apb_wr(0x42045, 0x3);
dwc_ddrphy_apb_wr(0x42046, 0x3c0);
dwc_ddrphy_apb_wr(0x42047, 0xa01);
dwc_ddrphy_apb_wr(0x42048, 0xef);
dwc_ddrphy_apb_wr(0x42049, 0xef00);
dwc_ddrphy_apb_wr(0x4204a, 0x3c0);
dwc_ddrphy_apb_wr(0x4204b, 0x1);
dwc_ddrphy_apb_wr(0x4204c, 0xc000);
dwc_ddrphy_apb_wr(0x4204d, 0x3);
dwc_ddrphy_apb_wr(0x4204e, 0x3c0);
dwc_ddrphy_apb_wr(0x4204f, 0x0);
dwc_ddrphy_apb_wr(0x42050, 0xc000);
dwc_ddrphy_apb_wr(0x42051, 0x3);
dwc_ddrphy_apb_wr(0x42052, 0x3c0);
dwc_ddrphy_apb_wr(0x42053, 0x2c1);
dwc_ddrphy_apb_wr(0x42054, 0xc000);
dwc_ddrphy_apb_wr(0x42055, 0x3);
dwc_ddrphy_apb_wr(0x42056, 0x3c0);
dwc_ddrphy_apb_wr(0x42057, 0xff01);
dwc_ddrphy_apb_wr(0x42058, 0xc000);
dwc_ddrphy_apb_wr(0x42059, 0x3);
dwc_ddrphy_apb_wr(0x4205a, 0x3c0);
dwc_ddrphy_apb_wr(0x4205b, 0x2c1);
dwc_ddrphy_apb_wr(0x4205c, 0xc000);
dwc_ddrphy_apb_wr(0x4205d, 0x3);
dwc_ddrphy_apb_wr(0x4205e, 0x3c0);
dwc_ddrphy_apb_wr(0x4205f, 0xff01);
dwc_ddrphy_apb_wr(0x42060, 0xc000);
dwc_ddrphy_apb_wr(0x42061, 0x3);
dwc_ddrphy_apb_wr(0x42062, 0x3c0);
dwc_ddrphy_apb_wr(0x42063, 0x2c1);
dwc_ddrphy_apb_wr(0x42064, 0xc000);
dwc_ddrphy_apb_wr(0x42065, 0x3);
dwc_ddrphy_apb_wr(0x42066, 0x3c0);
dwc_ddrphy_apb_wr(0x42067, 0xa01);
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 200, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 400, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 800, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 200000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 400000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 800000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10, type = 0
//// [phyinit_LoadPIECodeSections] Matched NO enable_bits = 4, type = 0
dwc_ddrphy_apb_wr(0x42068, 0x85d5);
dwc_ddrphy_apb_wr(0x42069, 0x63);
dwc_ddrphy_apb_wr(0x4206a, 0x3c0);
dwc_ddrphy_apb_wr(0x4206b, 0x400);
dwc_ddrphy_apb_wr(0x4206c, 0xc000);
dwc_ddrphy_apb_wr(0x4206d, 0x3);
dwc_ddrphy_apb_wr(0x4206e, 0x3c0);
dwc_ddrphy_apb_wr(0x4206f, 0x0);
dwc_ddrphy_apb_wr(0x42070, 0xc000);
dwc_ddrphy_apb_wr(0x42071, 0x3);
dwc_ddrphy_apb_wr(0x42072, 0x3c0);
dwc_ddrphy_apb_wr(0x42073, 0x2c1);
dwc_ddrphy_apb_wr(0x42074, 0xc000);
dwc_ddrphy_apb_wr(0x42075, 0x3);
dwc_ddrphy_apb_wr(0x42076, 0x3c0);
dwc_ddrphy_apb_wr(0x42077, 0x1001);
dwc_ddrphy_apb_wr(0x42078, 0x85f5);
dwc_ddrphy_apb_wr(0x42079, 0x63);
dwc_ddrphy_apb_wr(0x4207a, 0x3c0);
dwc_ddrphy_apb_wr(0x4207b, 0x800);
dwc_ddrphy_apb_wr(0x4207c, 0xc000);
dwc_ddrphy_apb_wr(0x4207d, 0x3);
dwc_ddrphy_apb_wr(0x4207e, 0x3c0);
dwc_ddrphy_apb_wr(0x4207f, 0x0);
dwc_ddrphy_apb_wr(0x42080, 0xc000);
dwc_ddrphy_apb_wr(0x42081, 0x3);
dwc_ddrphy_apb_wr(0x42082, 0x3c0);
dwc_ddrphy_apb_wr(0x42083, 0x2c1);
dwc_ddrphy_apb_wr(0x42084, 0xc000);
dwc_ddrphy_apb_wr(0x42085, 0x3);
dwc_ddrphy_apb_wr(0x42086, 0x3c0);
dwc_ddrphy_apb_wr(0x42087, 0x1001);
dwc_ddrphy_apb_wr(0x42088, 0x45d5);
dwc_ddrphy_apb_wr(0x42089, 0x63);
dwc_ddrphy_apb_wr(0x4208a, 0x3c0);
dwc_ddrphy_apb_wr(0x4208b, 0x401);
dwc_ddrphy_apb_wr(0x4208c, 0xc000);
dwc_ddrphy_apb_wr(0x4208d, 0x3);
dwc_ddrphy_apb_wr(0x4208e, 0x3c0);
dwc_ddrphy_apb_wr(0x4208f, 0x1);
dwc_ddrphy_apb_wr(0x42090, 0xc000);
dwc_ddrphy_apb_wr(0x42091, 0x3);
dwc_ddrphy_apb_wr(0x42092, 0x3c0);
dwc_ddrphy_apb_wr(0x42093, 0x2c1);
dwc_ddrphy_apb_wr(0x42094, 0xc000);
dwc_ddrphy_apb_wr(0x42095, 0x3);
dwc_ddrphy_apb_wr(0x42096, 0x3c0);
dwc_ddrphy_apb_wr(0x42097, 0x1001);
dwc_ddrphy_apb_wr(0x42098, 0x45f5);
dwc_ddrphy_apb_wr(0x42099, 0x63);
dwc_ddrphy_apb_wr(0x4209a, 0x3c0);
dwc_ddrphy_apb_wr(0x4209b, 0x801);
dwc_ddrphy_apb_wr(0x4209c, 0xc000);
dwc_ddrphy_apb_wr(0x4209d, 0x3);
dwc_ddrphy_apb_wr(0x4209e, 0x3c0);
dwc_ddrphy_apb_wr(0x4209f, 0x1);
dwc_ddrphy_apb_wr(0x420a0, 0xc000);
dwc_ddrphy_apb_wr(0x420a1, 0x3);
dwc_ddrphy_apb_wr(0x420a2, 0x3c0);
dwc_ddrphy_apb_wr(0x420a3, 0x2c1);
dwc_ddrphy_apb_wr(0x420a4, 0xc000);
dwc_ddrphy_apb_wr(0x420a5, 0x3);
dwc_ddrphy_apb_wr(0x420a6, 0x3c0);
dwc_ddrphy_apb_wr(0x420a7, 0x1001);
dwc_ddrphy_apb_wr(0x420a8, 0xc5d5);
dwc_ddrphy_apb_wr(0x420a9, 0x62);
dwc_ddrphy_apb_wr(0x420aa, 0x3c0);
dwc_ddrphy_apb_wr(0x420ab, 0x402);
dwc_ddrphy_apb_wr(0x420ac, 0xc000);
dwc_ddrphy_apb_wr(0x420ad, 0x3);
dwc_ddrphy_apb_wr(0x420ae, 0x3c0);
dwc_ddrphy_apb_wr(0x420af, 0x2);
dwc_ddrphy_apb_wr(0x420b0, 0xc000);
dwc_ddrphy_apb_wr(0x420b1, 0x3);
dwc_ddrphy_apb_wr(0x420b2, 0x3c0);
dwc_ddrphy_apb_wr(0x420b3, 0x2c1);
dwc_ddrphy_apb_wr(0x420b4, 0xc000);
dwc_ddrphy_apb_wr(0x420b5, 0x3);
dwc_ddrphy_apb_wr(0x420b6, 0x3c0);
dwc_ddrphy_apb_wr(0x420b7, 0x1001);
dwc_ddrphy_apb_wr(0x420b8, 0xc5f5);
dwc_ddrphy_apb_wr(0x420b9, 0x62);
dwc_ddrphy_apb_wr(0x420ba, 0x3c0);
dwc_ddrphy_apb_wr(0x420bb, 0x802);
dwc_ddrphy_apb_wr(0x420bc, 0xc000);
dwc_ddrphy_apb_wr(0x420bd, 0x3);
dwc_ddrphy_apb_wr(0x420be, 0x3c0);
dwc_ddrphy_apb_wr(0x420bf, 0x2);
dwc_ddrphy_apb_wr(0x420c0, 0xc000);
dwc_ddrphy_apb_wr(0x420c1, 0x3);
dwc_ddrphy_apb_wr(0x420c2, 0x3c0);
dwc_ddrphy_apb_wr(0x420c3, 0x2c1);
dwc_ddrphy_apb_wr(0x420c4, 0xc000);
dwc_ddrphy_apb_wr(0x420c5, 0x3);
dwc_ddrphy_apb_wr(0x420c6, 0x3c0);
dwc_ddrphy_apb_wr(0x420c7, 0x1001);
dwc_ddrphy_apb_wr(0x420c8, 0xc5d5);
dwc_ddrphy_apb_wr(0x420c9, 0x61);
dwc_ddrphy_apb_wr(0x420ca, 0x3c0);
dwc_ddrphy_apb_wr(0x420cb, 0x403);
dwc_ddrphy_apb_wr(0x420cc, 0xc000);
dwc_ddrphy_apb_wr(0x420cd, 0x3);
dwc_ddrphy_apb_wr(0x420ce, 0x3c0);
dwc_ddrphy_apb_wr(0x420cf, 0x3);
dwc_ddrphy_apb_wr(0x420d0, 0xc000);
dwc_ddrphy_apb_wr(0x420d1, 0x3);
dwc_ddrphy_apb_wr(0x420d2, 0x3c0);
dwc_ddrphy_apb_wr(0x420d3, 0x2c1);
dwc_ddrphy_apb_wr(0x420d4, 0xc000);
dwc_ddrphy_apb_wr(0x420d5, 0x3);
dwc_ddrphy_apb_wr(0x420d6, 0x3c0);
dwc_ddrphy_apb_wr(0x420d7, 0x1001);
dwc_ddrphy_apb_wr(0x420d8, 0xc5f5);
dwc_ddrphy_apb_wr(0x420d9, 0x61);
dwc_ddrphy_apb_wr(0x420da, 0x3c0);
dwc_ddrphy_apb_wr(0x420db, 0x803);
dwc_ddrphy_apb_wr(0x420dc, 0xc000);
dwc_ddrphy_apb_wr(0x420dd, 0x3);
dwc_ddrphy_apb_wr(0x420de, 0x3c0);
dwc_ddrphy_apb_wr(0x420df, 0x3);
dwc_ddrphy_apb_wr(0x420e0, 0xc000);
dwc_ddrphy_apb_wr(0x420e1, 0x3);
dwc_ddrphy_apb_wr(0x420e2, 0x3c0);
dwc_ddrphy_apb_wr(0x420e3, 0x2c1);
dwc_ddrphy_apb_wr(0x420e4, 0xc000);
dwc_ddrphy_apb_wr(0x420e5, 0x3);
dwc_ddrphy_apb_wr(0x420e6, 0x3c0);
dwc_ddrphy_apb_wr(0x420e7, 0x1d01);
//// [phyinit_LoadPIECodeSections] Matched NO enable_bits = 2, type = 0
dwc_ddrphy_apb_wr(0x420e8, 0x213);
dwc_ddrphy_apb_wr(0x420e9, 0x0);
dwc_ddrphy_apb_wr(0x420ea, 0x3c0);
dwc_ddrphy_apb_wr(0x420eb, 0x1);
dwc_ddrphy_apb_wr(0x420ec, 0xc000);
dwc_ddrphy_apb_wr(0x420ed, 0x3);
dwc_ddrphy_apb_wr(0x420ee, 0x3c0);
dwc_ddrphy_apb_wr(0x420ef, 0x0);
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100, type = 0
dwc_ddrphy_apb_wr(0x420f0, 0xc000);
dwc_ddrphy_apb_wr(0x420f1, 0x3);
dwc_ddrphy_apb_wr(0x420f2, 0x3c0);
dwc_ddrphy_apb_wr(0x420f3, 0x2c1);
dwc_ddrphy_apb_wr(0x420f4, 0xc000);
dwc_ddrphy_apb_wr(0x420f5, 0x3);
dwc_ddrphy_apb_wr(0x420f6, 0x3c0);
dwc_ddrphy_apb_wr(0x420f7, 0xef00);
dwc_ddrphy_apb_wr(0x420f8, 0xc000);
dwc_ddrphy_apb_wr(0x420f9, 0x3);
dwc_ddrphy_apb_wr(0x420fa, 0x3c0);
dwc_ddrphy_apb_wr(0x420fb, 0x2c1);
dwc_ddrphy_apb_wr(0x420fc, 0xc000);
dwc_ddrphy_apb_wr(0x420fd, 0x3);
dwc_ddrphy_apb_wr(0x420fe, 0x3c0);
dwc_ddrphy_apb_wr(0x420ff, 0x5900);
dwc_ddrphy_apb_wr(0x42100, 0x217);
dwc_ddrphy_apb_wr(0x42101, 0x1700);
dwc_ddrphy_apb_wr(0x42102, 0x3c2);
dwc_ddrphy_apb_wr(0x42103, 0x1);
dwc_ddrphy_apb_wr(0x42104, 0xc000);
dwc_ddrphy_apb_wr(0x42105, 0x3);
dwc_ddrphy_apb_wr(0x42106, 0x3c0);
dwc_ddrphy_apb_wr(0x42107, 0x0);
dwc_ddrphy_apb_wr(0x42108, 0xc000);
dwc_ddrphy_apb_wr(0x42109, 0x3);
dwc_ddrphy_apb_wr(0x4210a, 0x3c0);
dwc_ddrphy_apb_wr(0x4210b, 0x2c1);
dwc_ddrphy_apb_wr(0x4210c, 0xc000);
dwc_ddrphy_apb_wr(0x4210d, 0x3);
dwc_ddrphy_apb_wr(0x4210e, 0x3c0);
dwc_ddrphy_apb_wr(0x4210f, 0x400);
dwc_ddrphy_apb_wr(0x42110, 0x3fff);
dwc_ddrphy_apb_wr(0x42111, 0xff00);
dwc_ddrphy_apb_wr(0x42112, 0x3f);
dwc_ddrphy_apb_wr(0x42113, 0x2e1);
dwc_ddrphy_apb_wr(0x42114, 0x3fff);
dwc_ddrphy_apb_wr(0x42115, 0xff00);
dwc_ddrphy_apb_wr(0x42116, 0x3f);
dwc_ddrphy_apb_wr(0x42117, 0xa21);
dwc_ddrphy_apb_wr(0x42118, 0x3fff);
dwc_ddrphy_apb_wr(0x42119, 0xff00);
dwc_ddrphy_apb_wr(0x4211a, 0x3f);
dwc_ddrphy_apb_wr(0x4211b, 0x21);
dwc_ddrphy_apb_wr(0x4211c, 0xffff);
dwc_ddrphy_apb_wr(0x4211d, 0xff03);
dwc_ddrphy_apb_wr(0x4211e, 0x3ff);
dwc_ddrphy_apb_wr(0x4211f, 0x20);
dwc_ddrphy_apb_wr(0x42120, 0xffff);
dwc_ddrphy_apb_wr(0x42121, 0xff03);
dwc_ddrphy_apb_wr(0x42122, 0x3ff);
dwc_ddrphy_apb_wr(0x42123, 0x1e1);
dwc_ddrphy_apb_wr(0x42124, 0xffff);
dwc_ddrphy_apb_wr(0x42125, 0xff03);
dwc_ddrphy_apb_wr(0x42126, 0x3ff);
dwc_ddrphy_apb_wr(0x42127, 0x21);
dwc_ddrphy_apb_wr(0x42128, 0xffff);
dwc_ddrphy_apb_wr(0x42129, 0xff03);
dwc_ddrphy_apb_wr(0x4212a, 0x3ff);
dwc_ddrphy_apb_wr(0x4212b, 0x2e1);
dwc_ddrphy_apb_wr(0x4212c, 0xffff);
dwc_ddrphy_apb_wr(0x4212d, 0xff03);
dwc_ddrphy_apb_wr(0x4212e, 0x3ff);
dwc_ddrphy_apb_wr(0x4212f, 0x121);
dwc_ddrphy_apb_wr(0x42130, 0x3fff);
dwc_ddrphy_apb_wr(0x42131, 0xff00);
dwc_ddrphy_apb_wr(0x42132, 0x3ff);
dwc_ddrphy_apb_wr(0x42133, 0x21);
dwc_ddrphy_apb_wr(0x42134, 0x3fff);
dwc_ddrphy_apb_wr(0x42135, 0xff00);
dwc_ddrphy_apb_wr(0x42136, 0x3ff);
dwc_ddrphy_apb_wr(0x42137, 0x21);
dwc_ddrphy_apb_wr(0x42138, 0x3fff);
dwc_ddrphy_apb_wr(0x42139, 0xff00);
dwc_ddrphy_apb_wr(0x4213a, 0x3ff);
dwc_ddrphy_apb_wr(0x4213b, 0x21);
dwc_ddrphy_apb_wr(0x4213c, 0xffff);
dwc_ddrphy_apb_wr(0x4213d, 0xff03);
dwc_ddrphy_apb_wr(0x4213e, 0x3ff);
dwc_ddrphy_apb_wr(0x4213f, 0x21);
dwc_ddrphy_apb_wr(0x42140, 0xffff);
dwc_ddrphy_apb_wr(0x42141, 0xff03);
dwc_ddrphy_apb_wr(0x42142, 0x3ff);
dwc_ddrphy_apb_wr(0x42143, 0x2e1);
dwc_ddrphy_apb_wr(0x42144, 0xffff);
dwc_ddrphy_apb_wr(0x42145, 0xff03);
dwc_ddrphy_apb_wr(0x42146, 0x3ff);
dwc_ddrphy_apb_wr(0x42147, 0xf921);
dwc_ddrphy_apb_wr(0x42148, 0xffff);
dwc_ddrphy_apb_wr(0x42149, 0xff03);
dwc_ddrphy_apb_wr(0x4214a, 0x3ff);
dwc_ddrphy_apb_wr(0x4214b, 0x2e1);
dwc_ddrphy_apb_wr(0x4214c, 0xffff);
dwc_ddrphy_apb_wr(0x4214d, 0xff03);
dwc_ddrphy_apb_wr(0x4214e, 0x3ff);
dwc_ddrphy_apb_wr(0x4214f, 0x5921);
dwc_ddrphy_apb_wr(0x42150, 0x5a5);
dwc_ddrphy_apb_wr(0x42151, 0xa500);
dwc_ddrphy_apb_wr(0x42152, 0x3c5);
dwc_ddrphy_apb_wr(0x42153, 0x21);
dwc_ddrphy_apb_wr(0x42154, 0xc040);
dwc_ddrphy_apb_wr(0x42155, 0x4003);
dwc_ddrphy_apb_wr(0x42156, 0x3c0);
dwc_ddrphy_apb_wr(0x42157, 0x20);
dwc_ddrphy_apb_wr(0x42158, 0xc000);
dwc_ddrphy_apb_wr(0x42159, 0x3);
dwc_ddrphy_apb_wr(0x4215a, 0x3c0);
dwc_ddrphy_apb_wr(0x4215b, 0x2e1);
dwc_ddrphy_apb_wr(0x4215c, 0xc000);
dwc_ddrphy_apb_wr(0x4215d, 0x3);
dwc_ddrphy_apb_wr(0x4215e, 0x3c0);
dwc_ddrphy_apb_wr(0x4215f, 0xa21);
dwc_ddrphy_apb_wr(0x42160, 0xef);
dwc_ddrphy_apb_wr(0x42161, 0xef00);
dwc_ddrphy_apb_wr(0x42162, 0x3c0);
dwc_ddrphy_apb_wr(0x42163, 0x21);
dwc_ddrphy_apb_wr(0x42164, 0xc000);
dwc_ddrphy_apb_wr(0x42165, 0x3);
dwc_ddrphy_apb_wr(0x42166, 0x3c0);
dwc_ddrphy_apb_wr(0x42167, 0x20);
dwc_ddrphy_apb_wr(0x42168, 0xc000);
dwc_ddrphy_apb_wr(0x42169, 0x3);
dwc_ddrphy_apb_wr(0x4216a, 0x3c0);
dwc_ddrphy_apb_wr(0x4216b, 0x2e1);
dwc_ddrphy_apb_wr(0x4216c, 0xc000);
dwc_ddrphy_apb_wr(0x4216d, 0x3);
dwc_ddrphy_apb_wr(0x4216e, 0x3c0);
dwc_ddrphy_apb_wr(0x4216f, 0xff21);
dwc_ddrphy_apb_wr(0x42170, 0xc000);
dwc_ddrphy_apb_wr(0x42171, 0x3);
dwc_ddrphy_apb_wr(0x42172, 0x3c0);
dwc_ddrphy_apb_wr(0x42173, 0x2e1);
dwc_ddrphy_apb_wr(0x42174, 0xc000);
dwc_ddrphy_apb_wr(0x42175, 0x3);
dwc_ddrphy_apb_wr(0x42176, 0x3c0);
dwc_ddrphy_apb_wr(0x42177, 0xff21);
dwc_ddrphy_apb_wr(0x42178, 0xc000);
dwc_ddrphy_apb_wr(0x42179, 0x3);
dwc_ddrphy_apb_wr(0x4217a, 0x3c0);
dwc_ddrphy_apb_wr(0x4217b, 0x2e1);
dwc_ddrphy_apb_wr(0x4217c, 0xc000);
dwc_ddrphy_apb_wr(0x4217d, 0x3);
dwc_ddrphy_apb_wr(0x4217e, 0x3c0);
dwc_ddrphy_apb_wr(0x4217f, 0xa21);
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 200, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 400, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 800, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 200000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 400000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 800000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 1000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 2000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 4000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 8000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80000000, type = 2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 10, type = 0
//// [phyinit_LoadPIECodeSections] Matched NO enable_bits = 4, type = 0
dwc_ddrphy_apb_wr(0x42180, 0x85d5);
dwc_ddrphy_apb_wr(0x42181, 0xd563);
dwc_ddrphy_apb_wr(0x42182, 0x3c5);
dwc_ddrphy_apb_wr(0x42183, 0x420);
dwc_ddrphy_apb_wr(0x42184, 0xc000);
dwc_ddrphy_apb_wr(0x42185, 0x3);
dwc_ddrphy_apb_wr(0x42186, 0x3c0);
dwc_ddrphy_apb_wr(0x42187, 0x20);
dwc_ddrphy_apb_wr(0x42188, 0xc000);
dwc_ddrphy_apb_wr(0x42189, 0x3);
dwc_ddrphy_apb_wr(0x4218a, 0x3c0);
dwc_ddrphy_apb_wr(0x4218b, 0x2c1);
dwc_ddrphy_apb_wr(0x4218c, 0xc000);
dwc_ddrphy_apb_wr(0x4218d, 0x3);
dwc_ddrphy_apb_wr(0x4218e, 0x3c0);
dwc_ddrphy_apb_wr(0x4218f, 0x1001);
dwc_ddrphy_apb_wr(0x42190, 0x85f5);
dwc_ddrphy_apb_wr(0x42191, 0xf563);
dwc_ddrphy_apb_wr(0x42192, 0x3c5);
dwc_ddrphy_apb_wr(0x42193, 0x820);
dwc_ddrphy_apb_wr(0x42194, 0xc000);
dwc_ddrphy_apb_wr(0x42195, 0x3);
dwc_ddrphy_apb_wr(0x42196, 0x3c0);
dwc_ddrphy_apb_wr(0x42197, 0x20);
dwc_ddrphy_apb_wr(0x42198, 0xc000);
dwc_ddrphy_apb_wr(0x42199, 0x3);
dwc_ddrphy_apb_wr(0x4219a, 0x3c0);
dwc_ddrphy_apb_wr(0x4219b, 0x2c1);
dwc_ddrphy_apb_wr(0x4219c, 0xc000);
dwc_ddrphy_apb_wr(0x4219d, 0x3);
dwc_ddrphy_apb_wr(0x4219e, 0x3c0);
dwc_ddrphy_apb_wr(0x4219f, 0x1001);
dwc_ddrphy_apb_wr(0x421a0, 0x45d5);
dwc_ddrphy_apb_wr(0x421a1, 0xd563);
dwc_ddrphy_apb_wr(0x421a2, 0x3c5);
dwc_ddrphy_apb_wr(0x421a3, 0x421);
dwc_ddrphy_apb_wr(0x421a4, 0xc000);
dwc_ddrphy_apb_wr(0x421a5, 0x3);
dwc_ddrphy_apb_wr(0x421a6, 0x3c0);
dwc_ddrphy_apb_wr(0x421a7, 0x21);
dwc_ddrphy_apb_wr(0x421a8, 0xc000);
dwc_ddrphy_apb_wr(0x421a9, 0x3);
dwc_ddrphy_apb_wr(0x421aa, 0x3c0);
dwc_ddrphy_apb_wr(0x421ab, 0x2c1);
dwc_ddrphy_apb_wr(0x421ac, 0xc000);
dwc_ddrphy_apb_wr(0x421ad, 0x3);
dwc_ddrphy_apb_wr(0x421ae, 0x3c0);
dwc_ddrphy_apb_wr(0x421af, 0x1001);
dwc_ddrphy_apb_wr(0x421b0, 0x45f5);
dwc_ddrphy_apb_wr(0x421b1, 0xf563);
dwc_ddrphy_apb_wr(0x421b2, 0x3c5);
dwc_ddrphy_apb_wr(0x421b3, 0x821);
dwc_ddrphy_apb_wr(0x421b4, 0xc000);
dwc_ddrphy_apb_wr(0x421b5, 0x3);
dwc_ddrphy_apb_wr(0x421b6, 0x3c0);
dwc_ddrphy_apb_wr(0x421b7, 0x21);
dwc_ddrphy_apb_wr(0x421b8, 0xc000);
dwc_ddrphy_apb_wr(0x421b9, 0x3);
dwc_ddrphy_apb_wr(0x421ba, 0x3c0);
dwc_ddrphy_apb_wr(0x421bb, 0x2c1);
dwc_ddrphy_apb_wr(0x421bc, 0xc000);
dwc_ddrphy_apb_wr(0x421bd, 0x3);
dwc_ddrphy_apb_wr(0x421be, 0x3c0);
dwc_ddrphy_apb_wr(0x421bf, 0x1001);
dwc_ddrphy_apb_wr(0x421c0, 0xc5d5);
dwc_ddrphy_apb_wr(0x421c1, 0xd562);
dwc_ddrphy_apb_wr(0x421c2, 0x3c5);
dwc_ddrphy_apb_wr(0x421c3, 0x422);
dwc_ddrphy_apb_wr(0x421c4, 0xc000);
dwc_ddrphy_apb_wr(0x421c5, 0x3);
dwc_ddrphy_apb_wr(0x421c6, 0x3c0);
dwc_ddrphy_apb_wr(0x421c7, 0x22);
dwc_ddrphy_apb_wr(0x421c8, 0xc000);
dwc_ddrphy_apb_wr(0x421c9, 0x3);
dwc_ddrphy_apb_wr(0x421ca, 0x3c0);
dwc_ddrphy_apb_wr(0x421cb, 0x2c1);
dwc_ddrphy_apb_wr(0x421cc, 0xc000);
dwc_ddrphy_apb_wr(0x421cd, 0x3);
dwc_ddrphy_apb_wr(0x421ce, 0x3c0);
dwc_ddrphy_apb_wr(0x421cf, 0x1001);
dwc_ddrphy_apb_wr(0x421d0, 0xc5f5);
dwc_ddrphy_apb_wr(0x421d1, 0xf562);
dwc_ddrphy_apb_wr(0x421d2, 0x3c5);
dwc_ddrphy_apb_wr(0x421d3, 0x822);
dwc_ddrphy_apb_wr(0x421d4, 0xc000);
dwc_ddrphy_apb_wr(0x421d5, 0x3);
dwc_ddrphy_apb_wr(0x421d6, 0x3c0);
dwc_ddrphy_apb_wr(0x421d7, 0x22);
dwc_ddrphy_apb_wr(0x421d8, 0xc000);
dwc_ddrphy_apb_wr(0x421d9, 0x3);
dwc_ddrphy_apb_wr(0x421da, 0x3c0);
dwc_ddrphy_apb_wr(0x421db, 0x2c1);
dwc_ddrphy_apb_wr(0x421dc, 0xc000);
dwc_ddrphy_apb_wr(0x421dd, 0x3);
dwc_ddrphy_apb_wr(0x421de, 0x3c0);
dwc_ddrphy_apb_wr(0x421df, 0x1001);
dwc_ddrphy_apb_wr(0x421e0, 0xc5d5);
dwc_ddrphy_apb_wr(0x421e1, 0xd561);
dwc_ddrphy_apb_wr(0x421e2, 0x3c5);
dwc_ddrphy_apb_wr(0x421e3, 0x423);
dwc_ddrphy_apb_wr(0x421e4, 0xc000);
dwc_ddrphy_apb_wr(0x421e5, 0x3);
dwc_ddrphy_apb_wr(0x421e6, 0x3c0);
dwc_ddrphy_apb_wr(0x421e7, 0x23);
dwc_ddrphy_apb_wr(0x421e8, 0xc000);
dwc_ddrphy_apb_wr(0x421e9, 0x3);
dwc_ddrphy_apb_wr(0x421ea, 0x3c0);
dwc_ddrphy_apb_wr(0x421eb, 0x2c1);
dwc_ddrphy_apb_wr(0x421ec, 0xc000);
dwc_ddrphy_apb_wr(0x421ed, 0x3);
dwc_ddrphy_apb_wr(0x421ee, 0x3c0);
dwc_ddrphy_apb_wr(0x421ef, 0x1001);
dwc_ddrphy_apb_wr(0x421f0, 0xc5f5);
dwc_ddrphy_apb_wr(0x421f1, 0xf561);
dwc_ddrphy_apb_wr(0x421f2, 0x3c5);
dwc_ddrphy_apb_wr(0x421f3, 0x823);
dwc_ddrphy_apb_wr(0x421f4, 0xc000);
dwc_ddrphy_apb_wr(0x421f5, 0x3);
dwc_ddrphy_apb_wr(0x421f6, 0x3c0);
dwc_ddrphy_apb_wr(0x421f7, 0x23);
dwc_ddrphy_apb_wr(0x421f8, 0xc000);
dwc_ddrphy_apb_wr(0x421f9, 0x3);
dwc_ddrphy_apb_wr(0x421fa, 0x3c0);
dwc_ddrphy_apb_wr(0x421fb, 0x2c1);
dwc_ddrphy_apb_wr(0x421fc, 0xc000);
dwc_ddrphy_apb_wr(0x421fd, 0x3);
dwc_ddrphy_apb_wr(0x421fe, 0x3c0);
dwc_ddrphy_apb_wr(0x421ff, 0x1d01);
//// [phyinit_LoadPIECodeSections] Matched NO enable_bits = 2, type = 0
dwc_ddrphy_apb_wr(0x42200, 0x213);
dwc_ddrphy_apb_wr(0x42201, 0x1300);
dwc_ddrphy_apb_wr(0x42202, 0x3c2);
dwc_ddrphy_apb_wr(0x42203, 0x21);
dwc_ddrphy_apb_wr(0x42204, 0xc000);
dwc_ddrphy_apb_wr(0x42205, 0x3);
dwc_ddrphy_apb_wr(0x42206, 0x3c0);
dwc_ddrphy_apb_wr(0x42207, 0x20);
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100, type = 0
dwc_ddrphy_apb_wr(0x42208, 0xc000);
dwc_ddrphy_apb_wr(0x42209, 0x3);
dwc_ddrphy_apb_wr(0x4220a, 0x3c0);
dwc_ddrphy_apb_wr(0x4220b, 0x2e1);
dwc_ddrphy_apb_wr(0x4220c, 0xc000);
dwc_ddrphy_apb_wr(0x4220d, 0x3);
dwc_ddrphy_apb_wr(0x4220e, 0x3c0);
dwc_ddrphy_apb_wr(0x4220f, 0xef20);
dwc_ddrphy_apb_wr(0x42210, 0xc000);
dwc_ddrphy_apb_wr(0x42211, 0x3);
dwc_ddrphy_apb_wr(0x42212, 0x3c0);
dwc_ddrphy_apb_wr(0x42213, 0x2e1);
dwc_ddrphy_apb_wr(0x42214, 0xc000);
dwc_ddrphy_apb_wr(0x42215, 0x3);
dwc_ddrphy_apb_wr(0x42216, 0x3c0);
dwc_ddrphy_apb_wr(0x42217, 0x5920);
dwc_ddrphy_apb_wr(0x42218, 0x217);
dwc_ddrphy_apb_wr(0x42219, 0x1700);
dwc_ddrphy_apb_wr(0x4221a, 0x3c2);
dwc_ddrphy_apb_wr(0x4221b, 0x21);
dwc_ddrphy_apb_wr(0x4221c, 0xc000);
dwc_ddrphy_apb_wr(0x4221d, 0x3);
dwc_ddrphy_apb_wr(0x4221e, 0x3c0);
dwc_ddrphy_apb_wr(0x4221f, 0x20);
dwc_ddrphy_apb_wr(0x42220, 0xc000);
dwc_ddrphy_apb_wr(0x42221, 0x3);
dwc_ddrphy_apb_wr(0x42222, 0x3c0);
dwc_ddrphy_apb_wr(0x42223, 0x2e1);
dwc_ddrphy_apb_wr(0x42224, 0xc000);
dwc_ddrphy_apb_wr(0x42225, 0x3);
dwc_ddrphy_apb_wr(0x42226, 0x3c0);
dwc_ddrphy_apb_wr(0x42227, 0x420);
//// [phyinit_LoadPIECodeSections] Moving start address from 42228 to 90029
dwc_ddrphy_apb_wr(0x90029, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b0s0
dwc_ddrphy_apb_wr(0x9002a, 0x4); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b0s1
dwc_ddrphy_apb_wr(0x9002b, 0x18); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b0s2
dwc_ddrphy_apb_wr(0x9002c, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b1s0
dwc_ddrphy_apb_wr(0x9002d, 0x4); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b1s1
dwc_ddrphy_apb_wr(0x9002e, 0x18); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b1s2
dwc_ddrphy_apb_wr(0x9002f, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b2s0
dwc_ddrphy_apb_wr(0x90030, 0x4); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b2s1
dwc_ddrphy_apb_wr(0x90031, 0x18); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b2s2
dwc_ddrphy_apb_wr(0x90032, 0xb); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b3s0
dwc_ddrphy_apb_wr(0x90033, 0x480); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b3s1
dwc_ddrphy_apb_wr(0x90034, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b3s2
dwc_ddrphy_apb_wr(0x90035, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b4s0
dwc_ddrphy_apb_wr(0x90036, 0x448); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b4s1
dwc_ddrphy_apb_wr(0x90037, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b4s2
dwc_ddrphy_apb_wr(0x90038, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b5s0
dwc_ddrphy_apb_wr(0x90039, 0x478); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b5s1
dwc_ddrphy_apb_wr(0x9003a, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b5s2
dwc_ddrphy_apb_wr(0x9003b, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b6s0
dwc_ddrphy_apb_wr(0x9003c, 0xe8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b6s1
dwc_ddrphy_apb_wr(0x9003d, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b6s2
dwc_ddrphy_apb_wr(0x9003e, 0x2); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b7s0
dwc_ddrphy_apb_wr(0x9003f, 0x10); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b7s1
dwc_ddrphy_apb_wr(0x90040, 0x139); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b7s2
dwc_ddrphy_apb_wr(0x90041, 0xf); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b8s0
dwc_ddrphy_apb_wr(0x90042, 0x7c0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b8s1
dwc_ddrphy_apb_wr(0x90043, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b8s2
dwc_ddrphy_apb_wr(0x90044, 0x107); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b9s0
dwc_ddrphy_apb_wr(0x90045, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b9s1
dwc_ddrphy_apb_wr(0x90046, 0x159); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b9s2
dwc_ddrphy_apb_wr(0x90047, 0x18); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b10s0
dwc_ddrphy_apb_wr(0x90048, 0xe0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b10s1
dwc_ddrphy_apb_wr(0x90049, 0x139); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b10s2
dwc_ddrphy_apb_wr(0x9004a, 0x147); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b11s0
dwc_ddrphy_apb_wr(0x9004b, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b11s1
dwc_ddrphy_apb_wr(0x9004c, 0x159); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b11s2
dwc_ddrphy_apb_wr(0x9004d, 0x14f); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b12s0
dwc_ddrphy_apb_wr(0x9004e, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b12s1
dwc_ddrphy_apb_wr(0x9004f, 0x159); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b12s2
dwc_ddrphy_apb_wr(0x90050, 0x7); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b13s0
dwc_ddrphy_apb_wr(0x90051, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b13s1
dwc_ddrphy_apb_wr(0x90052, 0x149); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b13s2
dwc_ddrphy_apb_wr(0x90053, 0x47); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b14s0
dwc_ddrphy_apb_wr(0x90054, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b14s1
dwc_ddrphy_apb_wr(0x90055, 0x149); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b14s2
dwc_ddrphy_apb_wr(0x90056, 0x4f); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b15s0
dwc_ddrphy_apb_wr(0x90057, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b15s1
dwc_ddrphy_apb_wr(0x90058, 0x179); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b15s2
//// [phyinit_LoadPIECodeSections] Matched NO enable_bits = 800, type = 0
dwc_ddrphy_apb_wr(0x90059, 0x100); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b16s0
dwc_ddrphy_apb_wr(0x9005a, 0x15c); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b16s1
dwc_ddrphy_apb_wr(0x9005b, 0x139); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b16s2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 800, type = 0
dwc_ddrphy_apb_wr(0x9005c, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b17s0
dwc_ddrphy_apb_wr(0x9005d, 0x7c8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b17s1
dwc_ddrphy_apb_wr(0x9005e, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b17s2
dwc_ddrphy_apb_wr(0x9005f, 0x11); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b18s0
dwc_ddrphy_apb_wr(0x90060, 0x530); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b18s1
dwc_ddrphy_apb_wr(0x90061, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b18s2
dwc_ddrphy_apb_wr(0x90062, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b19s0
dwc_ddrphy_apb_wr(0x90063, 0x1); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b19s1
dwc_ddrphy_apb_wr(0x90064, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b19s2
dwc_ddrphy_apb_wr(0x90065, 0x14f); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b20s0
dwc_ddrphy_apb_wr(0x90066, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b20s1
dwc_ddrphy_apb_wr(0x90067, 0x159); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b20s2
dwc_ddrphy_apb_wr(0x90068, 0x2); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b21s0
dwc_ddrphy_apb_wr(0x90069, 0x45a); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b21s1
dwc_ddrphy_apb_wr(0x9006a, 0x9); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b21s2
dwc_ddrphy_apb_wr(0x9006b, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b22s0
dwc_ddrphy_apb_wr(0x9006c, 0x530); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b22s1
dwc_ddrphy_apb_wr(0x9006d, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b22s2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 800, type = 0
dwc_ddrphy_apb_wr(0x9006e, 0xc100); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b23s0
dwc_ddrphy_apb_wr(0x9006f, 0x15c); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b23s1
dwc_ddrphy_apb_wr(0x90070, 0x139); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b23s2
dwc_ddrphy_apb_wr(0x90071, 0x18); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b24s0
dwc_ddrphy_apb_wr(0x90072, 0x65a); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b24s1
dwc_ddrphy_apb_wr(0x90073, 0x9); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b24s2
dwc_ddrphy_apb_wr(0x90074, 0x41); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b25s0
dwc_ddrphy_apb_wr(0x90075, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b25s1
dwc_ddrphy_apb_wr(0x90076, 0x179); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b25s2
dwc_ddrphy_apb_wr(0x90077, 0x1); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b26s0
dwc_ddrphy_apb_wr(0x90078, 0x618); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b26s1
dwc_ddrphy_apb_wr(0x90079, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b26s2
dwc_ddrphy_apb_wr(0x9007a, 0x40c0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b27s0
dwc_ddrphy_apb_wr(0x9007b, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b27s1
dwc_ddrphy_apb_wr(0x9007c, 0x149); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b27s2
dwc_ddrphy_apb_wr(0x9007d, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b28s0
dwc_ddrphy_apb_wr(0x9007e, 0x4); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b28s1
dwc_ddrphy_apb_wr(0x9007f, 0x48); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b28s2
dwc_ddrphy_apb_wr(0x90080, 0x4040); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b29s0
dwc_ddrphy_apb_wr(0x90081, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b29s1
dwc_ddrphy_apb_wr(0x90082, 0x149); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b29s2
dwc_ddrphy_apb_wr(0x90083, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b30s0
dwc_ddrphy_apb_wr(0x90084, 0x4); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b30s1
dwc_ddrphy_apb_wr(0x90085, 0x48); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b30s2
dwc_ddrphy_apb_wr(0x90086, 0x40); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b31s0
dwc_ddrphy_apb_wr(0x90087, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b31s1
dwc_ddrphy_apb_wr(0x90088, 0x149); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b31s2
dwc_ddrphy_apb_wr(0x90089, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b32s0
dwc_ddrphy_apb_wr(0x9008a, 0x658); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b32s1
dwc_ddrphy_apb_wr(0x9008b, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b32s2
dwc_ddrphy_apb_wr(0x9008c, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b33s0
dwc_ddrphy_apb_wr(0x9008d, 0x4); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b33s1
dwc_ddrphy_apb_wr(0x9008e, 0x18); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b33s2
dwc_ddrphy_apb_wr(0x9008f, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b34s0
dwc_ddrphy_apb_wr(0x90090, 0x4); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b34s1
dwc_ddrphy_apb_wr(0x90091, 0x78); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b34s2
dwc_ddrphy_apb_wr(0x90092, 0x549); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b35s0
dwc_ddrphy_apb_wr(0x90093, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b35s1
dwc_ddrphy_apb_wr(0x90094, 0x159); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b35s2
dwc_ddrphy_apb_wr(0x90095, 0xd49); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b36s0
dwc_ddrphy_apb_wr(0x90096, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b36s1
dwc_ddrphy_apb_wr(0x90097, 0x159); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b36s2
dwc_ddrphy_apb_wr(0x90098, 0x94c); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b37s0
dwc_ddrphy_apb_wr(0x90099, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b37s1
dwc_ddrphy_apb_wr(0x9009a, 0x159); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b37s2
dwc_ddrphy_apb_wr(0x9009b, 0x94c); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b38s0
dwc_ddrphy_apb_wr(0x9009c, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b38s1
dwc_ddrphy_apb_wr(0x9009d, 0x159); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b38s2
dwc_ddrphy_apb_wr(0x9009e, 0x442); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b39s0
dwc_ddrphy_apb_wr(0x9009f, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b39s1
dwc_ddrphy_apb_wr(0x900a0, 0x149); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b39s2
dwc_ddrphy_apb_wr(0x900a1, 0x42); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b40s0
dwc_ddrphy_apb_wr(0x900a2, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b40s1
dwc_ddrphy_apb_wr(0x900a3, 0x149); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b40s2
dwc_ddrphy_apb_wr(0x900a4, 0x1); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b41s0
dwc_ddrphy_apb_wr(0x900a5, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b41s1
dwc_ddrphy_apb_wr(0x900a6, 0x149); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b41s2
dwc_ddrphy_apb_wr(0x900a7, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b42s0
dwc_ddrphy_apb_wr(0x900a8, 0xe0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b42s1
dwc_ddrphy_apb_wr(0x900a9, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b42s2
dwc_ddrphy_apb_wr(0x900aa, 0xa); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b43s0
dwc_ddrphy_apb_wr(0x900ab, 0x10); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b43s1
dwc_ddrphy_apb_wr(0x900ac, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b43s2
dwc_ddrphy_apb_wr(0x900ad, 0x9); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b44s0
dwc_ddrphy_apb_wr(0x900ae, 0x3c0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b44s1
dwc_ddrphy_apb_wr(0x900af, 0x149); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b44s2
dwc_ddrphy_apb_wr(0x900b0, 0x9); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b45s0
dwc_ddrphy_apb_wr(0x900b1, 0x3c0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b45s1
dwc_ddrphy_apb_wr(0x900b2, 0x159); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b45s2
dwc_ddrphy_apb_wr(0x900b3, 0x18); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b46s0
dwc_ddrphy_apb_wr(0x900b4, 0x10); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b46s1
dwc_ddrphy_apb_wr(0x900b5, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b46s2
dwc_ddrphy_apb_wr(0x900b6, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b47s0
dwc_ddrphy_apb_wr(0x900b7, 0x3c0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b47s1
dwc_ddrphy_apb_wr(0x900b8, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b47s2
dwc_ddrphy_apb_wr(0x900b9, 0x18); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b48s0
dwc_ddrphy_apb_wr(0x900ba, 0x4); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b48s1
dwc_ddrphy_apb_wr(0x900bb, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b48s2
dwc_ddrphy_apb_wr(0x900bc, 0xc); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b49s0
dwc_ddrphy_apb_wr(0x900bd, 0x10); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b49s1
dwc_ddrphy_apb_wr(0x900be, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b49s2
dwc_ddrphy_apb_wr(0x900bf, 0x3); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b50s0
dwc_ddrphy_apb_wr(0x900c0, 0x10); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b50s1
dwc_ddrphy_apb_wr(0x900c1, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b50s2
dwc_ddrphy_apb_wr(0x900c2, 0x7); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b51s0
dwc_ddrphy_apb_wr(0x900c3, 0x7c0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b51s1
dwc_ddrphy_apb_wr(0x900c4, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b51s2
//// [phyinit_LoadPIECodeSections] Matched ANY enable_bits = 8, type = 0
dwc_ddrphy_apb_wr(0x900c5, 0x3a); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b52s0
dwc_ddrphy_apb_wr(0x900c6, 0x1e2); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b52s1
dwc_ddrphy_apb_wr(0x900c7, 0x9); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b52s2
dwc_ddrphy_apb_wr(0x900c8, 0x10); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b53s0
dwc_ddrphy_apb_wr(0x900c9, 0x10); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b53s1
dwc_ddrphy_apb_wr(0x900ca, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b53s2
dwc_ddrphy_apb_wr(0x900cb, 0x10); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b54s0
dwc_ddrphy_apb_wr(0x900cc, 0x400); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b54s1
dwc_ddrphy_apb_wr(0x900cd, 0x16e); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b54s2
dwc_ddrphy_apb_wr(0x900ce, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b55s0
dwc_ddrphy_apb_wr(0x900cf, 0x7c8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b55s1
dwc_ddrphy_apb_wr(0x900d0, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b55s2
dwc_ddrphy_apb_wr(0x900d1, 0x10); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b56s0
dwc_ddrphy_apb_wr(0x900d2, 0x10); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b56s1
dwc_ddrphy_apb_wr(0x900d3, 0x169); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b56s2
dwc_ddrphy_apb_wr(0x900d4, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b57s0
dwc_ddrphy_apb_wr(0x900d5, 0x978); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b57s1
dwc_ddrphy_apb_wr(0x900d6, 0x169); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b57s2
dwc_ddrphy_apb_wr(0x900d7, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b58s0
dwc_ddrphy_apb_wr(0x900d8, 0xa78); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b58s1
dwc_ddrphy_apb_wr(0x900d9, 0x169); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b58s2
dwc_ddrphy_apb_wr(0x900da, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b59s0
dwc_ddrphy_apb_wr(0x900db, 0x980); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b59s1
dwc_ddrphy_apb_wr(0x900dc, 0x169); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b59s2
dwc_ddrphy_apb_wr(0x900dd, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b60s0
dwc_ddrphy_apb_wr(0x900de, 0xa80); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b60s1
dwc_ddrphy_apb_wr(0x900df, 0x169); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b60s2
dwc_ddrphy_apb_wr(0x900e0, 0x32); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b61s0
dwc_ddrphy_apb_wr(0x900e1, 0x952); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b61s1
dwc_ddrphy_apb_wr(0x900e2, 0x69); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b61s2
dwc_ddrphy_apb_wr(0x900e3, 0x32); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b62s0
dwc_ddrphy_apb_wr(0x900e4, 0xa52); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b62s1
dwc_ddrphy_apb_wr(0x900e5, 0x69); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b62s2
dwc_ddrphy_apb_wr(0x900e6, 0x2); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b63s0
dwc_ddrphy_apb_wr(0x900e7, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b63s1
dwc_ddrphy_apb_wr(0x900e8, 0x68); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b63s2
dwc_ddrphy_apb_wr(0x900e9, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b64s0
dwc_ddrphy_apb_wr(0x900ea, 0x370); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b64s1
dwc_ddrphy_apb_wr(0x900eb, 0x169); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b64s2
dwc_ddrphy_apb_wr(0x900ec, 0x1); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b65s0
dwc_ddrphy_apb_wr(0x900ed, 0x1400); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b65s1
dwc_ddrphy_apb_wr(0x900ee, 0x169); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b65s2
dwc_ddrphy_apb_wr(0x900ef, 0x18); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b66s0
dwc_ddrphy_apb_wr(0x900f0, 0x8e8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b66s1
dwc_ddrphy_apb_wr(0x900f1, 0x169); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b66s2
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 20, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 80, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 40, type = 0
//// [phyinit_LoadPIECodeSections] No match for ANY enable_bits = 100, type = 0
//// [phyinit_LoadPIECodeSections] Matched NO enable_bits = 2, type = 0
dwc_ddrphy_apb_wr(0x900f2, 0x2cd); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b67s0
dwc_ddrphy_apb_wr(0x900f3, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b67s1
dwc_ddrphy_apb_wr(0x900f4, 0x68); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b67s2
dwc_ddrphy_apb_wr(0x900f5, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b68s0
dwc_ddrphy_apb_wr(0x900f6, 0x8e8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b68s1
dwc_ddrphy_apb_wr(0x900f7, 0x169); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b68s2
dwc_ddrphy_apb_wr(0x900f8, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b69s0
dwc_ddrphy_apb_wr(0x900f9, 0x3c8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b69s1
dwc_ddrphy_apb_wr(0x900fa, 0x1e9); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b69s2
dwc_ddrphy_apb_wr(0x900fb, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b70s0
dwc_ddrphy_apb_wr(0x900fc, 0x370); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b70s1
dwc_ddrphy_apb_wr(0x900fd, 0x169); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b70s2
dwc_ddrphy_apb_wr(0x900fe, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b71s0
dwc_ddrphy_apb_wr(0x900ff, 0xe8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b71s1
dwc_ddrphy_apb_wr(0x90100, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b71s2
dwc_ddrphy_apb_wr(0x90101, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b72s0
dwc_ddrphy_apb_wr(0x90102, 0x8140); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b72s1
dwc_ddrphy_apb_wr(0x90103, 0x10c); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b72s2
dwc_ddrphy_apb_wr(0x90104, 0x10); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b73s0
dwc_ddrphy_apb_wr(0x90105, 0x8138); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b73s1
dwc_ddrphy_apb_wr(0x90106, 0x10c); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b73s2
//// [phyinit_LoadPIECodeSections] Matched ANY enable_bits = 1, type = 0
dwc_ddrphy_apb_wr(0x90107, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b74s0
dwc_ddrphy_apb_wr(0x90108, 0x400); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b74s1
dwc_ddrphy_apb_wr(0x90109, 0x10e); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b74s2
dwc_ddrphy_apb_wr(0x9010a, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b75s0
dwc_ddrphy_apb_wr(0x9010b, 0x448); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b75s1
dwc_ddrphy_apb_wr(0x9010c, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b75s2
dwc_ddrphy_apb_wr(0x9010d, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b76s0
dwc_ddrphy_apb_wr(0x9010e, 0x7c8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b76s1
dwc_ddrphy_apb_wr(0x9010f, 0x101); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b76s2
dwc_ddrphy_apb_wr(0x90110, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b77s0
dwc_ddrphy_apb_wr(0x90111, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b77s1
dwc_ddrphy_apb_wr(0x90112, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b77s2
dwc_ddrphy_apb_wr(0x90113, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b78s0
dwc_ddrphy_apb_wr(0x90114, 0x448); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b78s1
dwc_ddrphy_apb_wr(0x90115, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b78s2
dwc_ddrphy_apb_wr(0x90116, 0xf); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b79s0
dwc_ddrphy_apb_wr(0x90117, 0x7c0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b79s1
dwc_ddrphy_apb_wr(0x90118, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b79s2
dwc_ddrphy_apb_wr(0x90119, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b80s0
dwc_ddrphy_apb_wr(0x9011a, 0xe8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b80s1
dwc_ddrphy_apb_wr(0x9011b, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b80s2
dwc_ddrphy_apb_wr(0x9011c, 0x7); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b81s0
dwc_ddrphy_apb_wr(0x9011d, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b81s1
dwc_ddrphy_apb_wr(0x9011e, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b81s2
dwc_ddrphy_apb_wr(0x9011f, 0x47); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b82s0
dwc_ddrphy_apb_wr(0x90120, 0x630); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b82s1
dwc_ddrphy_apb_wr(0x90121, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b82s2
dwc_ddrphy_apb_wr(0x90122, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b83s0
dwc_ddrphy_apb_wr(0x90123, 0x618); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b83s1
dwc_ddrphy_apb_wr(0x90124, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b83s2
dwc_ddrphy_apb_wr(0x90125, 0x18); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b84s0
dwc_ddrphy_apb_wr(0x90126, 0xe0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b84s1
dwc_ddrphy_apb_wr(0x90127, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b84s2
dwc_ddrphy_apb_wr(0x90128, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b85s0
dwc_ddrphy_apb_wr(0x90129, 0x7c8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b85s1
dwc_ddrphy_apb_wr(0x9012a, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b85s2
dwc_ddrphy_apb_wr(0x9012b, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b86s0
dwc_ddrphy_apb_wr(0x9012c, 0x8140); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b86s1
dwc_ddrphy_apb_wr(0x9012d, 0x10c); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b86s2
dwc_ddrphy_apb_wr(0x9012e, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b87s0
dwc_ddrphy_apb_wr(0x9012f, 0x478); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b87s1
dwc_ddrphy_apb_wr(0x90130, 0x109); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b87s2
dwc_ddrphy_apb_wr(0x90131, 0x0); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b88s0
dwc_ddrphy_apb_wr(0x90132, 0x1); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b88s1
dwc_ddrphy_apb_wr(0x90133, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b88s2
dwc_ddrphy_apb_wr(0x90134, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b89s0
dwc_ddrphy_apb_wr(0x90135, 0x4); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b89s1
dwc_ddrphy_apb_wr(0x90136, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b89s2
dwc_ddrphy_apb_wr(0x90137, 0x8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b90s0
dwc_ddrphy_apb_wr(0x90138, 0x7c8); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b90s1
dwc_ddrphy_apb_wr(0x90139, 0x101); // DWC_DDRPHYA_INITENG0_base0_SequenceReg0b90s2
//// [phyinit_LoadPIECodeSections] Moving start address from 9013a to 90006
dwc_ddrphy_apb_wr(0x90006, 0x0); // DWC_DDRPHYA_INITENG0_base0_PostSequenceReg0b0s0
dwc_ddrphy_apb_wr(0x90007, 0x0); // DWC_DDRPHYA_INITENG0_base0_PostSequenceReg0b0s1
dwc_ddrphy_apb_wr(0x90008, 0x8); // DWC_DDRPHYA_INITENG0_base0_PostSequenceReg0b0s2
dwc_ddrphy_apb_wr(0x90009, 0x0); // DWC_DDRPHYA_INITENG0_base0_PostSequenceReg0b1s0
dwc_ddrphy_apb_wr(0x9000a, 0x0); // DWC_DDRPHYA_INITENG0_base0_PostSequenceReg0b1s1
dwc_ddrphy_apb_wr(0x9000b, 0x0); // DWC_DDRPHYA_INITENG0_base0_PostSequenceReg0b1s2
//// [phyinit_LoadPIECodeSections] Moving start address from 9000c to d00e7
dwc_ddrphy_apb_wr(0xd00e7, 0x400); // DWC_DDRPHYA_APBONLY0_SequencerOverride
//// [phyinit_LoadPIECodeSections] End of dwc_ddrphy_phyinit_LoadPIECodeSections()
dwc_ddrphy_apb_wr(0x20240, 0x4300); // DWC_DDRPHYA_MASTER0_base0_D5ACSMPtrXlat0
dwc_ddrphy_apb_wr(0x20242, 0x8944); // DWC_DDRPHYA_MASTER0_base0_D5ACSMPtrXlat2
dwc_ddrphy_apb_wr(0x20241, 0x4300); // DWC_DDRPHYA_MASTER0_base0_D5ACSMPtrXlat1
dwc_ddrphy_apb_wr(0x20243, 0x8944); // DWC_DDRPHYA_MASTER0_base0_D5ACSMPtrXlat3
//seq0b_LoadPstateSeqProductionCode(): ---------------------------------------------------------------------------------------------------
//seq0b_LoadPstateSeqProductionCode(): Programming the 0B sequencer 0b0000 start vector registers with 0.
//seq0b_LoadPstateSeqProductionCode(): Programming the 0B sequencer 0b1000 start vector register with 54.
//seq0b_LoadPstateSeqProductionCode(): Programming the 0B sequencer 0b1111 start vector register with 77.
//seq0b_LoadPstateSeqProductionCode(): ---------------------------------------------------------------------------------------------------
dwc_ddrphy_apb_wr(0x90017, 0x0); // DWC_DDRPHYA_INITENG0_base0_StartVector0b0
dwc_ddrphy_apb_wr(0x9001f, 0x36); // DWC_DDRPHYA_INITENG0_base0_StartVector0b8
dwc_ddrphy_apb_wr(0x90026, 0x4d); // DWC_DDRPHYA_INITENG0_base0_StartVector0b15
dwc_ddrphy_apb_wr(0x9000c, 0x0); // DWC_DDRPHYA_INITENG0_base0_Seq0BDisableFlag0
dwc_ddrphy_apb_wr(0x9000d, 0x173); // DWC_DDRPHYA_INITENG0_base0_Seq0BDisableFlag1
dwc_ddrphy_apb_wr(0x9000e, 0x60); // DWC_DDRPHYA_INITENG0_base0_Seq0BDisableFlag2
dwc_ddrphy_apb_wr(0x9000f, 0x6110); // DWC_DDRPHYA_INITENG0_base0_Seq0BDisableFlag3
dwc_ddrphy_apb_wr(0x90010, 0x2152); // DWC_DDRPHYA_INITENG0_base0_Seq0BDisableFlag4
dwc_ddrphy_apb_wr(0x90011, 0xdfbd); // DWC_DDRPHYA_INITENG0_base0_Seq0BDisableFlag5
dwc_ddrphy_apb_wr(0x90012, 0x8060); // DWC_DDRPHYA_INITENG0_base0_Seq0BDisableFlag6
dwc_ddrphy_apb_wr(0x90013, 0x6152); // DWC_DDRPHYA_INITENG0_base0_Seq0BDisableFlag7
//// [phyinit_I_loadPIEImage] Enabling Phy Master Interface for DRAM drift compensation
//// [phyinit_I_loadPIEImage] Pstate=0, Memclk=1600MHz, Programming PPTTrainSetup::PhyMstrTrainInterval to 0x0
//// [phyinit_I_loadPIEImage] Pstate=0, Memclk=1600MHz, Programming PPTTrainSetup::PhyMstrMaxReqToAck to 0x0
dwc_ddrphy_apb_wr(0x20010, 0x0); // DWC_DDRPHYA_MASTER0_base0_PPTTrainSetup_p0
//// [phyinit_I_loadPIEImage] Pstate=0, Memclk=1600MHz, Programming PPTTrainSetup2::PhyMstrFreqOverride to 0x3
dwc_ddrphy_apb_wr(0x20011, 0x3); // DWC_DDRPHYA_MASTER0_base0_PPTTrainSetup2_p0
//// [phyinit_I_loadPIEImage] Programming D5ACSMXlatSelect to 0x1
dwc_ddrphy_apb_wr(0x20281, 0x1); // DWC_DDRPHYA_MASTER0_base0_D5ACSMXlatSelect
//// [phyinit_I_loadPIEImage] Programming DbyteRxEnTrain::EnDqsSampNegRxEn to 0x1
dwc_ddrphy_apb_wr(0x2003b, 0x2); // DWC_DDRPHYA_MASTER0_base0_DbyteRxEnTrain
//// [phyinit_I_loadPIEImage] Pstate=0, Memclk=1600MHz, Programming TrackingModeCntrl to 0x131f
dwc_ddrphy_apb_wr(0x20041, 0x131f); // DWC_DDRPHYA_MASTER0_base0_TrackingModeCntrl_p0
//// [phyinit_I_loadPIEImage] Programming D5ACSM0MaskCs to 0xe
dwc_ddrphy_apb_wr(0x20131, 0xe); // DWC_DDRPHYA_MASTER0_base0_D5ACSM0MaskCs
//// [phyinit_I_loadPIEImage] Programming D5ACSM1MaskCs to 0xf
dwc_ddrphy_apb_wr(0x20151, 0xf); // DWC_DDRPHYA_MASTER0_base0_D5ACSM1MaskCs
//// [phyinit_I_loadPIEImage] Pstate=0, Memclk=1600MHz, Programming Seq0BGPR6[0] with OuterLoopRepeatCnt values to 0x2
dwc_ddrphy_apb_wr(0x90306, 0x2); // DWC_DDRPHYA_INITENG0_base0_Seq0BGPR6_p0
//// [phyinit_I_loadPIEImage] Programming D5ACSM<0/1>OuterLoopRepeatCnt=2
dwc_ddrphy_apb_wr(0x2012a, 0x2); // DWC_DDRPHYA_MASTER0_base0_D5ACSM0OuterLoopRepeatCnt
dwc_ddrphy_apb_wr(0x2014a, 0x2); // DWC_DDRPHYA_MASTER0_base0_D5ACSM1OuterLoopRepeatCnt
//// [phyinit_I_loadPIEImage] Programming D5ACSM<0/1>AddressMask=7ff
dwc_ddrphy_apb_wr(0x20126, 0x7ff); // DWC_DDRPHYA_MASTER0_base0_D5ACSM0AddressMask
dwc_ddrphy_apb_wr(0x20146, 0x7ff); // DWC_DDRPHYA_MASTER0_base0_D5ACSM1AddressMask
//// [phyinit_I_loadPIEImage] Programming D5ACSM<0/1>AlgaIncVal=1
dwc_ddrphy_apb_wr(0x20127, 0x1); // DWC_DDRPHYA_MASTER0_base0_D5ACSM0AlgaIncVal
dwc_ddrphy_apb_wr(0x20147, 0x1); // DWC_DDRPHYA_MASTER0_base0_D5ACSM1AlgaIncVal
//// [phyinit_I_loadPIEImage] Turn on calibration and hold idle until dfi_init_start is asserted sequence is triggered.
//// [phyinit_I_loadPIEImage] Programming CalZap to 0x1
//// [phyinit_I_loadPIEImage] Programming CalRate::CalRun to 0x1
//// [phyinit_I_loadPIEImage] Programming CalRate to 0x19
dwc_ddrphy_apb_wr(0x20089, 0x1); // DWC_DDRPHYA_MASTER0_base0_CalZap
dwc_ddrphy_apb_wr(0x20088, 0x19); // DWC_DDRPHYA_MASTER0_base0_CalRate
//// [phyinit_I_loadPIEImage] Programming ForceClkGaterEnables::ForcePubDxClkEnLow to 0x0
dwc_ddrphy_apb_wr(0x200a6, 0x0); // DWC_DDRPHYA_MASTER0_base0_ForceClkGaterEnables
//// Disabling Ucclk (PMU) and Hclk (training hardware)
dwc_ddrphy_apb_wr(0xc0080, 0x0); // DWC_DDRPHYA_DRTUB0_UcclkHclkEnables
//// Isolate the APB access from the internal CSRs by setting the MicroContMuxSel CSR to 1.
dwc_ddrphy_apb_wr(0xd0000, 0x1); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
//// [phyinit_userCustom_wait] Wait 40 DfiClks
//// [phyinit_I_loadPIEImage] End of dwc_ddrphy_phyinit_I_loadPIEImage()
//
//
////##############################################################
////
//// dwc_ddrphy_phyinit_userCustom_customPostTrain is a user-editable function.
////
//// The purpose of dwc_ddrphy_phyinit_userCustom_customPostTrain() is to override any
//// CSR values programmed by the training firmware or dwc_ddrphy_phyinit_progCsrSkipTrain()
//// This function is executed after training
////
//// IMPORTANT: in this function, user shall not override any values in userInputBasic and
//// userInputAdvanced data structures. Only CSR programming should be done in this function.
////
//// Sequence of Events in this function are:
//// 1. Enable APB access.
//// 2. Issue register writes
//// 3. Isolate APB access.
//
////##############################################################
//
dwc_ddrphy_phyinit_userCustom_customPostTrain();

//// [dwc_ddrphy_phyinit_userCustom_customPostTrain] End of dwc_ddrphy_phyinit_userCustom_customPostTrain()
//// [dwc_ddrphy_phyinit_userCustom_J_enterMissionMode] Start of dwc_ddrphy_phyinit_userCustom_J_enterMissionMode()
//
//
////##############################################################
////
//// 4.3.10(J) Initialize the PHY to Mission Mode through DFI Initialization
////
//// Initialize the PHY to mission mode as follows:
////
//// 1. Set the PHY input clocks to the desired frequency.
//// 2. Initialize the PHY to mission mode by performing DFI Initialization.
////    Please see the DFI specification for more information. See the DFI frequency bus encoding in section <XXX>.
//// Note: The PHY training firmware initializes the DRAM state. if skip
//// training is used, the DRAM state is not initialized.
////
////##############################################################
//
dwc_ddrphy_phyinit_userCustom_J_enterMissionMode(sdrammc);

//
//// [dwc_ddrphy_phyinit_userCustom_J_enterMissionMode] End of dwc_ddrphy_phyinit_userCustom_J_enterMissionMode()
// [dwc_ddrphy_phyinit_sequence] End of dwc_ddrphy_phyinit_sequence()
// [dwc_ddrphy_phyinit_main] End of dwc_ddrphy_phyinit_main()
