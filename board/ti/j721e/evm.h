/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board detected logic initialization abstraction
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

#ifndef __J72E_EVM_H
#define __J72E_EVM_H

#ifdef CONFIG_TI_I2C_BOARD_DETECT

int variant_board_late_init(void);
void variant_spl_board_init(void);

#else /* CONFIG_TI_I2C_BOARD_DETECT */

static inline int variant_board_late_init(void) { return 0; }
static inline void variant_spl_board_init(void) { }

#endif /* CONFIG_TI_I2C_BOARD_DETECT */

#endif /* __J72E_EVM_H */
