/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023 Intel Corporation <www.intel.com>
 */

#ifndef _RSU_S10_SPL_H_
#define _RSU_S10_SPL_H_

#include <asm/arch/rsu_s10.h>

u32 rsu_spl_ssbl_address(bool is_qspi_imge_check);
u32 rsu_spl_ssbl_size(bool is_qspi_imge_check);

#endif /* _RSU_S10_SPL_H_ */
