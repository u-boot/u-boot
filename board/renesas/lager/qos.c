/*
 * board/renesas/lager/qos.c
 *
 * Copyright (C) 2013,2014 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/arch/rmobile.h>

/* QoS version 0.955 */

enum {
	DBSC3_R00, DBSC3_R01, DBSC3_R02, DBSC3_R03, DBSC3_R04,
	DBSC3_R05, DBSC3_R06, DBSC3_R07, DBSC3_R08, DBSC3_R09,
	DBSC3_R10, DBSC3_R11, DBSC3_R12, DBSC3_R13, DBSC3_R14,
	DBSC3_R15,
	DBSC3_W00, DBSC3_W01, DBSC3_W02, DBSC3_W03, DBSC3_W04,
	DBSC3_W05, DBSC3_W06, DBSC3_W07, DBSC3_W08, DBSC3_W09,
	DBSC3_W10, DBSC3_W11, DBSC3_W12, DBSC3_W13, DBSC3_W14,
	DBSC3_W15,
	DBSC3_NR,
};

static const u32 dbsc3_qos_addr[DBSC3_NR] = {
	[DBSC3_R00] = DBSC3_0_QOS_R0_BASE,
	[DBSC3_R01] = DBSC3_0_QOS_R1_BASE,
	[DBSC3_R02] = DBSC3_0_QOS_R2_BASE,
	[DBSC3_R03] = DBSC3_0_QOS_R3_BASE,
	[DBSC3_R04] = DBSC3_0_QOS_R4_BASE,
	[DBSC3_R05] = DBSC3_0_QOS_R5_BASE,
	[DBSC3_R06] = DBSC3_0_QOS_R6_BASE,
	[DBSC3_R07] = DBSC3_0_QOS_R7_BASE,
	[DBSC3_R08] = DBSC3_0_QOS_R8_BASE,
	[DBSC3_R09] = DBSC3_0_QOS_R9_BASE,
	[DBSC3_R10] = DBSC3_0_QOS_R10_BASE,
	[DBSC3_R11] = DBSC3_0_QOS_R11_BASE,
	[DBSC3_R12] = DBSC3_0_QOS_R12_BASE,
	[DBSC3_R13] = DBSC3_0_QOS_R13_BASE,
	[DBSC3_R14] = DBSC3_0_QOS_R14_BASE,
	[DBSC3_R15] = DBSC3_0_QOS_R15_BASE,
	[DBSC3_W00] = DBSC3_0_QOS_W0_BASE,
	[DBSC3_W01] = DBSC3_0_QOS_W1_BASE,
	[DBSC3_W02] = DBSC3_0_QOS_W2_BASE,
	[DBSC3_W03] = DBSC3_0_QOS_W3_BASE,
	[DBSC3_W04] = DBSC3_0_QOS_W4_BASE,
	[DBSC3_W05] = DBSC3_0_QOS_W5_BASE,
	[DBSC3_W06] = DBSC3_0_QOS_W6_BASE,
	[DBSC3_W07] = DBSC3_0_QOS_W7_BASE,
	[DBSC3_W08] = DBSC3_0_QOS_W8_BASE,
	[DBSC3_W09] = DBSC3_0_QOS_W9_BASE,
	[DBSC3_W10] = DBSC3_0_QOS_W10_BASE,
	[DBSC3_W11] = DBSC3_0_QOS_W11_BASE,
	[DBSC3_W12] = DBSC3_0_QOS_W12_BASE,
	[DBSC3_W13] = DBSC3_0_QOS_W13_BASE,
	[DBSC3_W14] = DBSC3_0_QOS_W14_BASE,
	[DBSC3_W15] = DBSC3_0_QOS_W15_BASE,
};

void qos_init(void)
{
	int i;
	struct rcar_s3c *s3c;
	struct rcar_s3c_qos *s3c_qos;
	struct rcar_dbsc3_qos *qos_addr;
	struct rcar_mxi *mxi;
	struct rcar_mxi_qos *mxi_qos;
	struct rcar_axi_qos *axi_qos;

	/* DBSC DBADJ2 */
	writel(0x20042004, DBSC3_0_DBADJ2);

	/* S3C -QoS */
	s3c = (struct rcar_s3c *)S3C_BASE;
	writel(0x80FF1C1E, &s3c->s3cadsplcr);
	writel(0x1F060505, &s3c->s3crorr);
	writel(0x1F020100, &s3c->s3cworr);

	/* QoS Control Registers */
	s3c_qos = (struct rcar_s3c_qos *)S3C_QOS_CCI0_BASE;
	writel(0x00800080, &s3c_qos->s3cqos0);
	writel(0x22000010, &s3c_qos->s3cqos1);
	writel(0x22002200, &s3c_qos->s3cqos2);
	writel(0x2F002200, &s3c_qos->s3cqos3);
	writel(0x2F002F00, &s3c_qos->s3cqos4);
	writel(0x22000010, &s3c_qos->s3cqos5);
	writel(0x22002200, &s3c_qos->s3cqos6);
	writel(0x2F002200, &s3c_qos->s3cqos7);
	writel(0x2F002F00, &s3c_qos->s3cqos8);

	s3c_qos = (struct rcar_s3c_qos *)S3C_QOS_CCI1_BASE;
	writel(0x00800080, &s3c_qos->s3cqos0);
	writel(0x22000010, &s3c_qos->s3cqos1);
	writel(0x22002200, &s3c_qos->s3cqos2);
	writel(0x2F002200, &s3c_qos->s3cqos3);
	writel(0x2F002F00, &s3c_qos->s3cqos4);
	writel(0x22000010, &s3c_qos->s3cqos5);
	writel(0x22002200, &s3c_qos->s3cqos6);
	writel(0x2F002200, &s3c_qos->s3cqos7);
	writel(0x2F002F00, &s3c_qos->s3cqos8);

	s3c_qos = (struct rcar_s3c_qos *)S3C_QOS_MXI_BASE;
	writel(0x80918099, &s3c_qos->s3cqos0);
	writel(0x20410010, &s3c_qos->s3cqos1);
	writel(0x200A2023, &s3c_qos->s3cqos2);
	writel(0x20502001, &s3c_qos->s3cqos3);
	writel(0x00002032, &s3c_qos->s3cqos4);
	writel(0x20410FFF, &s3c_qos->s3cqos5);
	writel(0x200A2023, &s3c_qos->s3cqos6);
	writel(0x20502001, &s3c_qos->s3cqos7);
	writel(0x20142032, &s3c_qos->s3cqos8);

	s3c_qos = (struct rcar_s3c_qos *)S3C_QOS_AXI_BASE;

	writel(0x00810089, &s3c_qos->s3cqos0);
	writel(0x20410001, &s3c_qos->s3cqos1);
	writel(0x200A2023, &s3c_qos->s3cqos2);
	writel(0x20502001, &s3c_qos->s3cqos3);
	writel(0x00002032, &s3c_qos->s3cqos4);
	writel(0x20410FFF, &s3c_qos->s3cqos5);
	writel(0x200A2023, &s3c_qos->s3cqos6);
	writel(0x20502001, &s3c_qos->s3cqos7);
	writel(0x20142032, &s3c_qos->s3cqos8);

	writel(0x00200808, &s3c->s3carcr11);

	/* DBSC -QoS */
	/* DBSC0 - Read/Write */
	for (i = DBSC3_R00; i < DBSC3_NR; i++) {
		qos_addr = (struct rcar_dbsc3_qos *)dbsc3_qos_addr[i];
		writel(0x00000203, &qos_addr->dblgcnt);
		writel(0x00002064, &qos_addr->dbtmval0);
		writel(0x00002048, &qos_addr->dbtmval1);
		writel(0x00002032, &qos_addr->dbtmval2);
		writel(0x00002019, &qos_addr->dbtmval3);
		writel(0x00000001, &qos_addr->dbrqctr);
		writel(0x00002019, &qos_addr->dbthres0);
		writel(0x00002019, &qos_addr->dbthres1);
		writel(0x00002019, &qos_addr->dbthres2);
		writel(0x00000000, &qos_addr->dblgqon);
	}
	/* CCI-400 -QoS */
	writel(0x20001000, CCI_400_MAXOT_1);
	writel(0x20001000, CCI_400_MAXOT_2);
	writel(0x0000000C, CCI_400_QOSCNTL_1);
	writel(0x0000000C, CCI_400_QOSCNTL_2);

	/* MXI -QoS */
	/* Transaction Control (MXI) */
	mxi = (struct rcar_mxi *)MXI_BASE;
	writel(0x00000013, &mxi->mxrtcr);
	writel(0x00000013, &mxi->mxwtcr);
	writel(0x00B800C0, &mxi->mxsaar0);
	writel(0x02000800, &mxi->mxsaar1);
	writel(0x00200000, &mxi->mxs3cracr);
	writel(0x00200000, &mxi->mxs3cwacr);
	writel(0x00200000, &mxi->mxaxiracr);
	writel(0x00200000, &mxi->mxaxiwacr);

	/* QoS Control (MXI) */
	mxi_qos = (struct rcar_mxi_qos *)MXI_QOS_BASE;
	writel(0x0000000C, &mxi_qos->vspdu0);
	writel(0x0000000C, &mxi_qos->vspdu1);
	writel(0x0000000D, &mxi_qos->du0);
	writel(0x0000000D, &mxi_qos->du1);

	/* AXI -QoS */
	/* Transaction Control (MXI) */
	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SYX64TO128_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_AVB_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200A, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_G2D_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200A, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMP0_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002002, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMP1_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMUX0_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMUX1_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMUX2_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_LBS_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002014, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUDS_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUM_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUS0_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUS1_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MTSB0_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002002, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MTSB1_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002002, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_PCI_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002014, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_RTX_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDS0_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200A, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDS1_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200A, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_USB20_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002005, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_USB21_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002005, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_USB22_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002005, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_USB30_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002014, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* QoS Register (RT-AXI) */
	axi_qos = (struct rcar_axi_qos *)RT_AXI_SHX_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002005, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_RDS_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_RTX64TO128_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_STPRO_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002003, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* QoS Register (MP-AXI) */
	axi_qos = (struct rcar_axi_qos *)MP_AXI_ADSP_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_ASDS0_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002014, &axi_qos->qosctset0);
	writel(0x00000040, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_ASDS1_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002014, &axi_qos->qosctset0);
	writel(0x00000040, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_MLP_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00001FF0, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00002001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_MMUMP_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_SPU_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_SPUC_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200D, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* QoS Register (SYS-AXI256) */
	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_AXI128TO256_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_SYX_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_MPX_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_MXI_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* QoS Register (CCI-AXI) */
	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUS0_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_SYX2_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUDS_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUM_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MXI_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x0000200F, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUS1_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUMP_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00002009, &axi_qos->qosctset1);
	writel(0x00002003, &axi_qos->qosctset2);
	writel(0x00002003, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* QoS Register (Media-AXI) */
	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_JPR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_JPW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_GCU0R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_GCU0W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_GCU1R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_GCU1W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_TDMR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_TDMW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP0CR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP0CW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP1CR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP1CW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPDU0CR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPDU0CW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPDU1CR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPDU1CW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VIN0W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP0R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP0W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_FDP0R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_FDP0W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMSR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMSW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP1R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP1W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_FDP1R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_FDP1W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMRR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMRW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_FDP2R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_FDP2W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD0R_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD0W_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD1R_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD1W_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_DU0R_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_DU0W_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_DU1R_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_DU1W_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000200C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP0CR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP0CW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP0VR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP0VW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VPC0R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP1CR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP1CW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP1VR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP1VW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VPC1R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002007, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000000, &axi_qos->qosqon);
}
