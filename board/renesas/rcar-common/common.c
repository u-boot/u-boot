/*
 * board/renesas/rcar-common/common.c
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2015 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>

#define TSTR0		0x04
#define TSTR0_STR0	0x01

static struct mstp_ctl mstptbl[] = {
	{ SMSTPCR0, MSTP0_BITS, CONFIG_SMSTP0_ENA,
		RMSTPCR0, MSTP0_BITS, CONFIG_RMSTP0_ENA },
	{ SMSTPCR1, MSTP1_BITS, CONFIG_SMSTP1_ENA,
		RMSTPCR1, MSTP1_BITS, CONFIG_RMSTP1_ENA },
	{ SMSTPCR2, MSTP2_BITS, CONFIG_SMSTP2_ENA,
		RMSTPCR2, MSTP2_BITS, CONFIG_RMSTP2_ENA },
	{ SMSTPCR3, MSTP3_BITS, CONFIG_SMSTP3_ENA,
		RMSTPCR3, MSTP3_BITS, CONFIG_RMSTP3_ENA },
	{ SMSTPCR4, MSTP4_BITS, CONFIG_SMSTP4_ENA,
		RMSTPCR4, MSTP4_BITS, CONFIG_RMSTP4_ENA },
	{ SMSTPCR5, MSTP5_BITS, CONFIG_SMSTP5_ENA,
		RMSTPCR5, MSTP5_BITS, CONFIG_RMSTP5_ENA },
#ifdef CONFIG_RCAR_GEN3
	{ SMSTPCR6, MSTP6_BITS, CONFIG_SMSTP6_ENA,
		RMSTPCR6, MSTP6_BITS, CONFIG_RMSTP6_ENA },
#endif
	{ SMSTPCR7, MSTP7_BITS, CONFIG_SMSTP7_ENA,
		RMSTPCR7, MSTP7_BITS, CONFIG_RMSTP7_ENA },
	{ SMSTPCR8, MSTP8_BITS, CONFIG_SMSTP8_ENA,
		RMSTPCR8, MSTP8_BITS, CONFIG_RMSTP8_ENA },
	{ SMSTPCR9, MSTP9_BITS, CONFIG_SMSTP9_ENA,
		RMSTPCR9, MSTP9_BITS, CONFIG_RMSTP9_ENA },
	{ SMSTPCR10, MSTP10_BITS, CONFIG_SMSTP10_ENA,
		 RMSTPCR10, MSTP10_BITS, CONFIG_RMSTP10_ENA },
	{ SMSTPCR11, MSTP11_BITS, CONFIG_SMSTP1_ENA,
		 RMSTPCR11, MSTP11_BITS, CONFIG_RMSTP11_ENA },
};

void arch_preboot_os(void)
{
	int i;

	/* stop TMU0 */
	mstp_clrbits_le32(TMU_BASE + TSTR0, TMU_BASE + TSTR0, TSTR0_STR0);

	/* Stop module clock */
	for (i = 0; i < ARRAY_SIZE(mstptbl); i++) {
		mstp_setclrbits_le32((uintptr_t)mstptbl[i].s_addr,
				     mstptbl[i].s_dis,
				     mstptbl[i].s_ena);
		mstp_setclrbits_le32((uintptr_t)mstptbl[i].r_addr,
				     mstptbl[i].r_dis,
				     mstptbl[i].r_ena);
	}
}
