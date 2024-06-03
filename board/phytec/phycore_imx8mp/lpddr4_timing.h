/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 */

#ifndef __LPDDR4_TIMING_H__
#define __LPDDR4_TIMING_H__

void set_dram_timings_2ghz_2gb(void);
void set_dram_timings_2ghz_1gb(void);
void set_dram_timings_2ghz_4gb(void);
void set_dram_timings_1_5ghz_1gb(void);
void set_dram_timings_1_5ghz_4gb(void);
void set_dram_timings_2ghz_8gb(void);

#endif /* __LPDDR4_TIMING_H__ */
