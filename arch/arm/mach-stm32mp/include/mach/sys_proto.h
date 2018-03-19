/*
 * Copyright (C) 2015-2017, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier:	GPL-2.0+	BSD-3-Clause
 */

#define CPU_STMP32MP15x	0x500

/* return CPU_STMP32MPxx constants */
u32 get_cpu_type(void);

#define CPU_REVA	0x1000
#define CPU_REVB	0x2000

/* return CPU_REV constants */
u32 get_cpu_rev(void);
