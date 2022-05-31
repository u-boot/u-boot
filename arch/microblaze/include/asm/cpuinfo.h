/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022, Ovidiu Panait <ovpanait@gmail.com>
 */

#ifndef __ASM_MICROBLAZE_CPUINFO_H
#define __ASM_MICROBLAZE_CPUINFO_H

/**
 * struct microblaze_cpuinfo - CPU info for microblaze processor core.
 *
 * @icache_size: Size of instruction cache memory in bytes.
 * @icache_line_length: Instruction cache line length in bytes.
 * @dcache_size: Size of data cache memory in bytes.
 * @dcache_line_length: Data cache line length in bytes.
 */
struct microblaze_cpuinfo {
	u32 icache_size;
	u32 icache_line_length;

	u32 dcache_size;
	u32 dcache_line_length;
};

/**
 * microblaze_early_cpuinfo_init() - Initialize cpuinfo with default values.
 *
 * Initializes the global data cpuinfo structure with default values (cache
 * size, cache line size, etc.). It is called very early in the boot process
 * (start.S codepath right before the first cache flush call) to ensure that
 * cache related operations are properly handled.
 */
void microblaze_early_cpuinfo_init(void);

#endif	/* __ASM_MICROBLAZE_CPUINFO_H */
