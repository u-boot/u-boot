/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#ifndef	_SDRAM_SOC32_H_
#define	_SDRAM_SOC32_H_

#if IS_ENABLED(CONFIG_SOCFPGA_ECC_SCRUB_SUPPORT)
void sdram_init_ecc_bits(void);
#else
static inline void sdram_init_ecc_bits(void) { }
#endif

#endif /* _SDRAM_SOC32_H_ */
