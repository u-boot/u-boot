/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Ported from Intel released Quark UEFI BIOS
 * QuarkSocPkg/QuarkNorthCluster/MemoryInit/Pei
 *
 * SPDX-License-Identifier:	Intel
 */

#ifndef _SMC_H_
#define _SMC_H_

/* System Memory Controller Register Defines */

/* Memory Controller Message Bus Registers Offsets */
#define DRP			0x00
#define DTR0			0x01
#define DTR1			0x02
#define DTR2			0x03
#define DTR3			0x04
#define DTR4			0x05
#define DPMC0			0x06
#define DPMC1			0x07
#define DRFC			0x08
#define DSCH			0x09
#define DCAL			0x0A
#define DRMC			0x0B
#define PMSTS			0x0C
#define DCO			0x0F
#define DSTAT			0x20
#define SSKPD0			0x4A
#define SSKPD1			0x4B
#define DECCCTRL		0x60
#define DECCSTAT		0x61
#define DECCSBECNT		0x62
#define DECCSBECA		0x68
#define DECCSBECS		0x69
#define DECCDBECA		0x6A
#define DECCDBECS		0x6B
#define DFUSESTAT		0x70
#define SCRMSEED		0x80
#define SCRMLO			0x81
#define SCRMHI			0x82

/* DRAM init command */
#define DCMD_MRS1(rnk, dat)	(0 | ((rnk) << 22) | (1 << 3) | ((dat) << 6))
#define DCMD_REF(rnk)		(1 | ((rnk) << 22))
#define DCMD_PRE(rnk)		(2 | ((rnk) << 22))
#define DCMD_PREA(rnk)		(2 | ((rnk) << 22) | (BIT10 << 6))
#define DCMD_ACT(rnk, row)	(3 | ((rnk) << 22) | ((row) << 6))
#define DCMD_WR(rnk, col)	(4 | ((rnk) << 22) | ((col) << 6))
#define DCMD_RD(rnk, col)	(5 | ((rnk) << 22) | ((col) << 6))
#define DCMD_ZQCS(rnk)		(6 | ((rnk) << 22))
#define DCMD_ZQCL(rnk)		(6 | ((rnk) << 22) | (BIT10 << 6))
#define DCMD_NOP(rnk)		(7 | ((rnk) << 22))

#define DDR3_EMRS1_DIC_40	(0)
#define DDR3_EMRS1_DIC_34	(1)

#define DDR3_EMRS1_RTTNOM_0	(0)
#define DDR3_EMRS1_RTTNOM_60	(0x04)
#define DDR3_EMRS1_RTTNOM_120	(0x40)
#define DDR3_EMRS1_RTTNOM_40	(0x44)
#define DDR3_EMRS1_RTTNOM_20	(0x200)
#define DDR3_EMRS1_RTTNOM_30	(0x204)

#define DDR3_EMRS2_RTTWR_60	(1 << 9)
#define DDR3_EMRS2_RTTWR_120	(1 << 10)

/* BEGIN DDRIO Registers */

/* DDR IOs & COMPs */
#define DDRIODQ_BL_OFFSET	0x0800
#define DDRIODQ_CH_OFFSET	((NUM_BYTE_LANES / 2) * DDRIODQ_BL_OFFSET)
#define DDRIOCCC_CH_OFFSET	0x0800
#define DDRCOMP_CH_OFFSET	0x0100

/* CH0-BL01-DQ */
#define DQOBSCKEBBCTL		0x0000
#define DQDLLTXCTL		0x0004
#define DQDLLRXCTL		0x0008
#define DQMDLLCTL		0x000C
#define B0RXIOBUFCTL		0x0010
#define B0VREFCTL		0x0014
#define B0RXOFFSET1		0x0018
#define B0RXOFFSET0		0x001C
#define B1RXIOBUFCTL		0x0020
#define B1VREFCTL		0x0024
#define B1RXOFFSET1		0x0028
#define B1RXOFFSET0		0x002C
#define DQDFTCTL		0x0030
#define DQTRAINSTS		0x0034
#define B1DLLPICODER0		0x0038
#define B0DLLPICODER0		0x003C
#define B1DLLPICODER1		0x0040
#define B0DLLPICODER1		0x0044
#define B1DLLPICODER2		0x0048
#define B0DLLPICODER2		0x004C
#define B1DLLPICODER3		0x0050
#define B0DLLPICODER3		0x0054
#define B1RXDQSPICODE		0x0058
#define B0RXDQSPICODE		0x005C
#define B1RXDQPICODER32		0x0060
#define B1RXDQPICODER10		0x0064
#define B0RXDQPICODER32		0x0068
#define B0RXDQPICODER10		0x006C
#define B01PTRCTL0		0x0070
#define B01PTRCTL1		0x0074
#define B01DBCTL0		0x0078
#define B01DBCTL1		0x007C
#define B0LATCTL0		0x0080
#define B1LATCTL0		0x0084
#define B01LATCTL1		0x0088
#define B0ONDURCTL		0x008C
#define B1ONDURCTL		0x0090
#define B0OVRCTL		0x0094
#define B1OVRCTL		0x0098
#define DQCTL			0x009C
#define B0RK2RKCHGPTRCTRL	0x00A0
#define B1RK2RKCHGPTRCTRL	0x00A4
#define DQRK2RKCTL		0x00A8
#define DQRK2RKPTRCTL		0x00AC
#define B0RK2RKLAT		0x00B0
#define B1RK2RKLAT		0x00B4
#define DQCLKALIGNREG0		0x00B8
#define DQCLKALIGNREG1		0x00BC
#define DQCLKALIGNREG2		0x00C0
#define DQCLKALIGNSTS0		0x00C4
#define DQCLKALIGNSTS1		0x00C8
#define DQCLKGATE		0x00CC
#define B0COMPSLV1		0x00D0
#define B1COMPSLV1		0x00D4
#define B0COMPSLV2		0x00D8
#define B1COMPSLV2		0x00DC
#define B0COMPSLV3		0x00E0
#define B1COMPSLV3		0x00E4
#define DQVISALANECR0TOP	0x00E8
#define DQVISALANECR1TOP	0x00EC
#define DQVISACONTROLCRTOP	0x00F0
#define DQVISALANECR0BL		0x00F4
#define DQVISALANECR1BL		0x00F8
#define DQVISACONTROLCRBL	0x00FC
#define DQTIMINGCTRL		0x010C

/* CH0-ECC */
#define ECCDLLTXCTL		0x2004
#define ECCDLLRXCTL		0x2008
#define ECCMDLLCTL		0x200C
#define ECCB1DLLPICODER0	0x2038
#define ECCB1DLLPICODER1	0x2040
#define ECCB1DLLPICODER2	0x2048
#define ECCB1DLLPICODER3	0x2050
#define ECCB01DBCTL0		0x2078
#define ECCB01DBCTL1		0x207C
#define ECCCLKALIGNREG0		0x20B8
#define ECCCLKALIGNREG1		0x20BC
#define ECCCLKALIGNREG2		0x20C0

/* CH0-CMD */
#define CMDOBSCKEBBCTL		0x4800
#define CMDDLLTXCTL		0x4808
#define CMDDLLRXCTL		0x480C
#define CMDMDLLCTL		0x4810
#define CMDRCOMPODT		0x4814
#define CMDDLLPICODER0		0x4820
#define CMDDLLPICODER1		0x4824
#define CMDCFGREG0		0x4840
#define CMDPTRREG		0x4844
#define CMDCLKALIGNREG0		0x4850
#define CMDCLKALIGNREG1		0x4854
#define CMDCLKALIGNREG2		0x4858
#define CMDPMCONFIG0		0x485C
#define CMDPMDLYREG0		0x4860
#define CMDPMDLYREG1		0x4864
#define CMDPMDLYREG2		0x4868
#define CMDPMDLYREG3		0x486C
#define CMDPMDLYREG4		0x4870
#define CMDCLKALIGNSTS0		0x4874
#define CMDCLKALIGNSTS1		0x4878
#define CMDPMSTS0		0x487C
#define CMDPMSTS1		0x4880
#define CMDCOMPSLV		0x4884
#define CMDBONUS0		0x488C
#define CMDBONUS1		0x4890
#define CMDVISALANECR0		0x4894
#define CMDVISALANECR1		0x4898
#define CMDVISACONTROLCR	0x489C
#define CMDCLKGATE		0x48A0
#define CMDTIMINGCTRL		0x48A4

/* CH0-CLK-CTL */
#define CCOBSCKEBBCTL		0x5800
#define CCRCOMPIO		0x5804
#define CCDLLTXCTL		0x5808
#define CCDLLRXCTL		0x580C
#define CCMDLLCTL		0x5810
#define CCRCOMPODT		0x5814
#define CCDLLPICODER0		0x5820
#define CCDLLPICODER1		0x5824
#define CCDDR3RESETCTL		0x5830
#define CCCFGREG0		0x5838
#define CCCFGREG1		0x5840
#define CCPTRREG		0x5844
#define CCCLKALIGNREG0		0x5850
#define CCCLKALIGNREG1		0x5854
#define CCCLKALIGNREG2		0x5858
#define CCPMCONFIG0		0x585C
#define CCPMDLYREG0		0x5860
#define CCPMDLYREG1		0x5864
#define CCPMDLYREG2		0x5868
#define CCPMDLYREG3		0x586C
#define CCPMDLYREG4		0x5870
#define CCCLKALIGNSTS0		0x5874
#define CCCLKALIGNSTS1		0x5878
#define CCPMSTS0		0x587C
#define CCPMSTS1		0x5880
#define CCCOMPSLV1		0x5884
#define CCCOMPSLV2		0x5888
#define CCCOMPSLV3		0x588C
#define CCBONUS0		0x5894
#define CCBONUS1		0x5898
#define CCVISALANECR0		0x589C
#define CCVISALANECR1		0x58A0
#define CCVISACONTROLCR		0x58A4
#define CCCLKGATE		0x58A8
#define CCTIMINGCTL		0x58AC

/* COMP */
#define CMPCTRL			0x6800
#define SOFTRSTCNTL		0x6804
#define MSCNTR			0x6808
#define NMSCNTRL		0x680C
#define LATCH1CTL		0x6814
#define COMPVISALANECR0		0x681C
#define COMPVISALANECR1		0x6820
#define COMPVISACONTROLCR	0x6824
#define COMPBONUS0		0x6830
#define TCOCNTCTRL		0x683C
#define DQANAODTPUCTL		0x6840
#define DQANAODTPDCTL		0x6844
#define DQANADRVPUCTL		0x6848
#define DQANADRVPDCTL		0x684C
#define DQANADLYPUCTL		0x6850
#define DQANADLYPDCTL		0x6854
#define DQANATCOPUCTL		0x6858
#define DQANATCOPDCTL		0x685C
#define CMDANADRVPUCTL		0x6868
#define CMDANADRVPDCTL		0x686C
#define CMDANADLYPUCTL		0x6870
#define CMDANADLYPDCTL		0x6874
#define CLKANAODTPUCTL		0x6880
#define CLKANAODTPDCTL		0x6884
#define CLKANADRVPUCTL		0x6888
#define CLKANADRVPDCTL		0x688C
#define CLKANADLYPUCTL		0x6890
#define CLKANADLYPDCTL		0x6894
#define CLKANATCOPUCTL		0x6898
#define CLKANATCOPDCTL		0x689C
#define DQSANAODTPUCTL		0x68A0
#define DQSANAODTPDCTL		0x68A4
#define DQSANADRVPUCTL		0x68A8
#define DQSANADRVPDCTL		0x68AC
#define DQSANADLYPUCTL		0x68B0
#define DQSANADLYPDCTL		0x68B4
#define DQSANATCOPUCTL		0x68B8
#define DQSANATCOPDCTL		0x68BC
#define CTLANADRVPUCTL		0x68C8
#define CTLANADRVPDCTL		0x68CC
#define CTLANADLYPUCTL		0x68D0
#define CTLANADLYPDCTL		0x68D4
#define CHNLBUFSTATIC		0x68F0
#define COMPOBSCNTRL		0x68F4
#define COMPBUFFDBG0		0x68F8
#define COMPBUFFDBG1		0x68FC
#define CFGMISCCH0		0x6900
#define COMPEN0CH0		0x6904
#define COMPEN1CH0		0x6908
#define COMPEN2CH0		0x690C
#define STATLEGEN0CH0		0x6910
#define STATLEGEN1CH0		0x6914
#define DQVREFCH0		0x6918
#define CMDVREFCH0		0x691C
#define CLKVREFCH0		0x6920
#define DQSVREFCH0		0x6924
#define CTLVREFCH0		0x6928
#define TCOVREFCH0		0x692C
#define DLYSELCH0		0x6930
#define TCODRAMBUFODTCH0	0x6934
#define CCBUFODTCH0		0x6938
#define RXOFFSETCH0		0x693C
#define DQODTPUCTLCH0		0x6940
#define DQODTPDCTLCH0		0x6944
#define DQDRVPUCTLCH0		0x6948
#define DQDRVPDCTLCH0		0x694C
#define DQDLYPUCTLCH0		0x6950
#define DQDLYPDCTLCH0		0x6954
#define DQTCOPUCTLCH0		0x6958
#define DQTCOPDCTLCH0		0x695C
#define CMDDRVPUCTLCH0		0x6968
#define CMDDRVPDCTLCH0		0x696C
#define CMDDLYPUCTLCH0		0x6970
#define CMDDLYPDCTLCH0		0x6974
#define CLKODTPUCTLCH0		0x6980
#define CLKODTPDCTLCH0		0x6984
#define CLKDRVPUCTLCH0		0x6988
#define CLKDRVPDCTLCH0		0x698C
#define CLKDLYPUCTLCH0		0x6990
#define CLKDLYPDCTLCH0		0x6994
#define CLKTCOPUCTLCH0		0x6998
#define CLKTCOPDCTLCH0		0x699C
#define DQSODTPUCTLCH0		0x69A0
#define DQSODTPDCTLCH0		0x69A4
#define DQSDRVPUCTLCH0		0x69A8
#define DQSDRVPDCTLCH0		0x69AC
#define DQSDLYPUCTLCH0		0x69B0
#define DQSDLYPDCTLCH0		0x69B4
#define DQSTCOPUCTLCH0		0x69B8
#define DQSTCOPDCTLCH0		0x69BC
#define CTLDRVPUCTLCH0		0x69C8
#define CTLDRVPDCTLCH0		0x69CC
#define CTLDLYPUCTLCH0		0x69D0
#define CTLDLYPDCTLCH0		0x69D4
#define FNLUPDTCTLCH0		0x69F0

/* PLL */
#define MPLLCTRL0		0x7800
#define MPLLCTRL1		0x7808
#define MPLLCSR0		0x7810
#define MPLLCSR1		0x7814
#define MPLLCSR2		0x7820
#define MPLLDFT			0x7828
#define MPLLMON0CTL		0x7830
#define MPLLMON1CTL		0x7838
#define MPLLMON2CTL		0x783C
#define SFRTRIM			0x7850
#define MPLLDFTOUT0		0x7858
#define MPLLDFTOUT1		0x785C
#define MASTERRSTN		0x7880
#define PLLLOCKDEL		0x7884
#define SFRDEL			0x7888
#define CRUVISALANECR0		0x78F0
#define CRUVISALANECR1		0x78F4
#define CRUVISACONTROLCR	0x78F8
#define IOSFVISALANECR0		0x78FC
#define IOSFVISALANECR1		0x7900
#define IOSFVISACONTROLCR	0x7904

/* END DDRIO Registers */

/* DRAM Specific Message Bus OpCodes */
#define MSG_OP_DRAM_INIT	0x68
#define MSG_OP_DRAM_WAKE	0xCA

#define SAMPLE_SIZE		6

/* must be less than this number to enable early deadband */
#define EARLY_DB		0x12
/* must be greater than this number to enable late deadband */
#define LATE_DB			0x34

#define CHX_REGS		(11 * 4)
#define FULL_CLK		128
#define HALF_CLK		64
#define QRTR_CLK		32

#define MCEIL(num, den)		((uint8_t)((num + den - 1) / den))
#define MMAX(a, b)		((a) > (b) ? (a) : (b))
#define DEAD_LOOP()		for (;;);

#define MIN_RDQS_EYE		10	/* in PI Codes */
#define MIN_VREF_EYE		10	/* in VREF Codes */
/* how many RDQS codes to jump while margining */
#define RDQS_STEP		1
/* how many VREF codes to jump while margining */
#define VREF_STEP		1
/* offset into "vref_codes[]" for minimum allowed VREF setting */
#define VREF_MIN		0x00
/* offset into "vref_codes[]" for maximum allowed VREF setting */
#define VREF_MAX		0x3F
#define RDQS_MIN		0x00	/* minimum RDQS delay value */
#define RDQS_MAX		0x3F	/* maximum RDQS delay value */

/* how many WDQ codes to jump while margining */
#define WDQ_STEP		1

enum {
	B,	/* BOTTOM VREF */
	T	/* TOP VREF */
};

enum {
	L,	/* LEFT RDQS */
	R	/* RIGHT RDQS */
};

/* Memory Options */

/* enable STATIC timing settings for RCVN (BACKUP_MODE) */
#undef BACKUP_RCVN
/* enable STATIC timing settings for WDQS (BACKUP_MODE) */
#undef BACKUP_WDQS
/* enable STATIC timing settings for RDQS (BACKUP_MODE) */
#undef BACKUP_RDQS
/* enable STATIC timing settings for WDQ (BACKUP_MODE) */
#undef BACKUP_WDQ
/* enable *COMP overrides (BACKUP_MODE) */
#undef BACKUP_COMPS
/* enable the RD_TRAIN eye check */
#undef RX_EYE_CHECK

/* enable Host to Memory Clock Alignment */
#define HMC_TEST
/* enable multi-rank support via rank2rank sharing */
#define R2R_SHARING
/* disable signals not used in 16bit mode of DDRIO */
#define FORCE_16BIT_DDRIO

#define PLATFORM_ID		1

void clear_self_refresh(struct mrc_params *mrc_params);
void prog_ddr_timing_control(struct mrc_params *mrc_params);
void prog_decode_before_jedec(struct mrc_params *mrc_params);
void perform_ddr_reset(struct mrc_params *mrc_params);
void ddrphy_init(struct mrc_params *mrc_params);
void perform_jedec_init(struct mrc_params *mrc_params);
void set_ddr_init_complete(struct mrc_params *mrc_params);
void restore_timings(struct mrc_params *mrc_params);
void default_timings(struct mrc_params *mrc_params);
void rcvn_cal(struct mrc_params *mrc_params);
void wr_level(struct mrc_params *mrc_params);
void prog_page_ctrl(struct mrc_params *mrc_params);
void rd_train(struct mrc_params *mrc_params);
void wr_train(struct mrc_params *mrc_params);
void store_timings(struct mrc_params *mrc_params);
void enable_scrambling(struct mrc_params *mrc_params);
void prog_ddr_control(struct mrc_params *mrc_params);
void prog_dra_drb(struct mrc_params *mrc_params);
void perform_wake(struct mrc_params *mrc_params);
void change_refresh_period(struct mrc_params *mrc_params);
void set_auto_refresh(struct mrc_params *mrc_params);
void ecc_enable(struct mrc_params *mrc_params);
void memory_test(struct mrc_params *mrc_params);
void lock_registers(struct mrc_params *mrc_params);

#endif /* _SMC_H_ */
