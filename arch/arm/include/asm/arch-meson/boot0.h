/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 Brivo Systems LLC
 */
b reset
/*
 * BL2 copies some keys to the start of DRAM, clobbering anything in the first
 * 4k aside from the initial instruction. I measured the actual data clobbered
 * as 1076 bytes, but I suspect the amount depends on the configuration, so
 * just go with what amlogic does.
 */
.skip 4096
