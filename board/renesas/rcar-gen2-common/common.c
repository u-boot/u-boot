/*
 * board/renesas/rcar-gen2-common/common.c
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
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

#define TMU0_MSTP125	(1 << 25)
void arch_preboot_os(void)
{
	/* stop TMU0 */
	mstp_clrbits_le32(TMU_BASE + TSTR0, TMU_BASE + TSTR0, TSTR0_STR0);

	/* Disable TMU0 */
	mstp_setbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125);
}
