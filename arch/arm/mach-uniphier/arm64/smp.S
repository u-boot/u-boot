/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/linkage.h>

ENTRY(uniphier_smp_setup)
	mrs	x0, s3_1_c15_c2_1	/* CPUECTLR_EL1 */
	orr	x0, x0, #(1 << 6)	/* SMPEN */
	msr	s3_1_c15_c2_1, x0
	ret
ENDPROC(uniphier_smp_setup)

ENTRY(uniphier_secondary_startup)
	bl	uniphier_smp_setup
	b	_start
ENDPROC(uniphier_secondary_startup)
