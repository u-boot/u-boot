/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 */

#ifndef __ASM_ARCH_MX8M_DDR_H
#define __ASM_ARCH_MX8M_DDR_H

#define DDRC_DDR_SS_GPR0		0x3d000000
#define DDRC_IPS_BASE_ADDR_0		0x3f400000
#define IP2APB_DDRPHY_IPS_BASE_ADDR(X)	(0x3c000000 + (X * 0x2000000))
#define DDRPHY_MEM(X)			(0x3c000000 + (X * 0x2000000) + 0x50000)

struct ddrc_freq {
	u32 res0[8];
	u32 derateen;
	u32 derateint;
	u32 res1[10];
	u32 rfshctl0;
	u32 res2[4];
	u32 rfshtmg;
	u32 rfshtmg1;
	u32 res3[28];
	u32 init3;
	u32 init4;
	u32 res;
	u32 init6;
	u32 init7;
	u32 res4[4];
	u32 dramtmg0;
	u32 dramtmg1;
	u32 dramtmg2;
	u32 dramtmg3;
	u32 dramtmg4;
	u32 dramtmg5;
	u32 dramtmg6;
	u32 dramtmg7;
	u32 dramtmg8;
	u32 dramtmg9;
	u32 dramtmg10;
	u32 dramtmg11;
	u32 dramtmg12;
	u32 dramtmg13;
	u32 dramtmg14;
	u32 dramtmg15;
	u32 dramtmg16;
	u32 dramtmg17;
	u32 res5[10];
	u32 mramtmg0;
	u32 mramtmg1;
	u32 mramtmg4;
	u32 mramtmg9;
	u32 zqctl0;
	u32 res6[3];
	u32 dfitmg0;
	u32 dfitmg1;
	u32 res7[7];
	u32 dfitmg2;
	u32 dfitmg3;
	u32 res8[33];
	u32 odtcfg;
};

struct imx8m_ddrc_regs {
	u32 mstr;
	u32 stat;
	u32 mstr1;
	u32 res1;
	u32 mrctrl0;
	u32 mrctrl1;
	u32 mrstat;
	u32 mrctrl2;
	u32 derateen;
	u32 derateint;
	u32 mstr2;
	u32 res2;
	u32 pwrctl;
	u32 pwrtmg;
	u32 hwlpctl;
	u32 hwffcctl;
	u32 hwffcstat;
	u32 res3[3];
	u32 rfshctl0;
	u32 rfshctl1;
	u32 rfshctl2;
	u32 rfshctl4;
	u32 rfshctl3;
	u32 rfshtmg;
	u32 rfshtmg1;
	u32 res4;
	u32 ecccfg0;
	u32 ecccfg1;
	u32 eccstat;
	u32 eccclr;
	u32 eccerrcnt;
	u32 ecccaddr0;
	u32 ecccaddr1;
	u32 ecccsyn0;
	u32 ecccsyn1;
	u32 ecccsyn2;
	u32 eccbitmask0;
	u32 eccbitmask1;
	u32 eccbitmask2;
	u32 eccuaddr0;
	u32 eccuaddr1;
	u32 eccusyn0;
	u32 eccusyn1;
	u32 eccusyn2;
	u32 eccpoisonaddr0;
	u32 eccpoisonaddr1;
	u32 crcparctl0;
	u32 crcparctl1;
	u32 crcparctl2;
	u32 crcparstat;
	u32 init0;
	u32 init1;
	u32 init2;
	u32 init3;
	u32 init4;
	u32 init5;
	u32 init6;
	u32 init7;
	u32 dimmctl;
	u32 rankctl;
	u32 res5;
	u32 chctl;
	u32 dramtmg0;
	u32 dramtmg1;
	u32 dramtmg2;
	u32 dramtmg3;
	u32 dramtmg4;
	u32 dramtmg5;
	u32 dramtmg6;
	u32 dramtmg7;
	u32 dramtmg8;
	u32 dramtmg9;
	u32 dramtmg10;
	u32 dramtmg11;
	u32 dramtmg12;
	u32 dramtmg13;
	u32 dramtmg14;
	u32 dramtmg15;
	u32 dramtmg16;
	u32 dramtmg17;
	u32 res6[10];
	u32 mramtmg0;
	u32 mramtmg1;
	u32 mramtmg4;
	u32 mramtmg9;
	u32 zqctl0;
	u32 zqctl1;
	u32 zqctl2;
	u32 zqstat;
	u32 dfitmg0;
	u32 dfitmg1;
	u32 dfilpcfg0;
	u32 dfilpcfg1;
	u32 dfiupd0;
	u32 dfiupd1;
	u32 dfiupd2;
	u32 res7;
	u32 dfimisc;
	u32 dfitmg2;
	u32 dfitmg3;
	u32 dfistat;
	u32 dbictl;
	u32 dfiphymstr;
	u32 res8[14];
	u32 addrmap0;
	u32 addrmap1;
	u32 addrmap2;
	u32 addrmap3;
	u32 addrmap4;
	u32 addrmap5;
	u32 addrmap6;
	u32 addrmap7;
	u32 addrmap8;
	u32 addrmap9;
	u32 addrmap10;
	u32 addrmap11;
	u32 res9[4];
	u32 odtcfg;
	u32 odtmap;
	u32 res10[2];
	u32 sched;
	u32 sched1;
	u32 sched2;
	u32 perfhpr1;
	u32 res11;
	u32 perflpr1;
	u32 res12;
	u32 perfwr1;
	u32 res13[4];
	u32 dqmap0;
	u32 dqmap1;
	u32 dqmap2;
	u32 dqmap3;
	u32 dqmap4;
	u32 dqmap5;
	u32 res14[26];
	u32 dbg0;
	u32 dbg1;
	u32 dbgcam;
	u32 dbgcmd;
	u32 dbgstat;
	u32 res15[3];
	u32 swctl;
	u32 swstat;
	u32 res16[2];
	u32 ocparcfg0;
	u32 ocparcfg1;
	u32 ocparcfg2;
	u32 ocparcfg3;
	u32 ocparstat0;
	u32 ocparstat1;
	u32 ocparwlog0;
	u32 ocparwlog1;
	u32 ocparwlog2;
	u32 ocparawlog0;
	u32 ocparawlog1;
	u32 ocparrlog0;
	u32 ocparrlog1;
	u32 ocpararlog0;
	u32 ocpararlog1;
	u32 poisoncfg;
	u32 poisonstat;
	u32 adveccindex;
	union  {
		u32 adveccstat;
		u32 eccapstat;
	};
	u32 eccpoisonpat0;
	u32 eccpoisonpat1;
	u32 eccpoisonpat2;
	u32 res17[6];
	u32 caparpoisonctl;
	u32 caparpoisonstat;
	u32 res18[2];
	u32 dynbsmstat;
	u32 res19[18];
	u32 pstat;
	u32 pccfg;
	struct {
		u32 pcfgr;
		u32 pcfgw;
		u32 pcfgc;
		struct {
			u32 pcfgidmaskch0;
			u32 pcfidvaluech0;
		} pcfgid[16];
		u32 pctrl;
		u32 pcfgqos0;
		u32 pcfgqos1;
		u32 pcfgwqos0;
		u32 pcfgwqos1;
		u32 res[4];
	} pcfg[16];
	struct {
		u32 sarbase;
		u32 sarsize;
	} sar[4];
	u32 sbrctl;
	u32 sbrstat;
	u32 sbrwdata0;
	u32 sbrwdata1;
	u32 pdch;
	u32 res20[755];
	/* umctl2_regs_dch1 */
	u32 ch1_stat;
	u32 res21[2];
	u32 ch1_mrctrl0;
	u32 ch1_mrctrl1;
	u32 ch1_mrstat;
	u32 ch1_mrctrl2;
	u32 res22[4];
	u32 ch1_pwrctl;
	u32 ch1_pwrtmg;
	u32 ch1_hwlpctl;
	u32 res23[15];
	u32 ch1_eccstat;
	u32 ch1_eccclr;
	u32 ch1_eccerrcnt;
	u32 ch1_ecccaddr0;
	u32 ch1_ecccaddr1;
	u32 ch1_ecccsyn0;
	u32 ch1_ecccsyn1;
	u32 ch1_ecccsyn2;
	u32 ch1_eccbitmask0;
	u32 ch1_eccbitmask1;
	u32 ch1_eccbitmask2;
	u32 ch1_eccuaddr0;
	u32 ch1_eccuaddr1;
	u32 ch1_eccusyn0;
	u32 ch1_eccusyn1;
	u32 ch1_eccusyn2;
	u32 res24[2];
	u32 ch1_crcparctl0;
	u32 res25[2];
	u32 ch1_crcparstat;
	u32 res26[46];
	u32 ch1_zqctl2;
	u32 ch1_zqstat;
	u32 res27[11];
	u32 ch1_dfistat;
	u32 res28[33];
	u32 ch1_odtmap;
	u32 res29[47];
	u32 ch1_dbg1;
	u32 ch1_dbgcam;
	u32 ch1_dbgcmd;
	u32 ch1_dbgstat;
	u32 res30[123];
	/* umctl2_regs_freq1 */
	struct ddrc_freq freq1;
	u32 res31[109];
	/* umctl2_regs_addrmap_alt */
	u32 addrmap0_alt;
	u32 addrmap1_alt;
	u32 addrmap2_alt;
	u32 addrmap3_alt;
	u32 addrmap4_alt;
	u32 addrmap5_alt;
	u32 addrmap6_alt;
	u32 addrmap7_alt;
	u32 addrmap8_alt;
	u32 addrmap9_alt;
	u32 addrmap10_alt;
	u32 addrmap11_alt;
	u32 res32[758];
	/* umctl2_regs_freq2 */
	struct ddrc_freq freq2;
	u32 res33[879];
	/* umctl2_regs_freq3 */
	struct ddrc_freq freq3;
};

struct imx8m_ddrphy_regs {
	u32 reg[0xf0000];
};

/* PHY State */
enum pstate {
	PS0,
	PS1,
	PS2,
	PS3,
};

enum msg_response {
	TRAIN_SUCCESS = 0x7,
	TRAIN_STREAM_START = 0x8,
	TRAIN_FAIL = 0xff,
};

#endif
