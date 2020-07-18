// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#include <cpu_func.h>

/*
 * The Octeon platform is cache coherent and cache flushes and invalidates
 * are not needed. Define some platform specific empty flush_foo()
 * functions here to overwrite the _weak common function as a no-op.
 * This effectively disables all cache operations.
 */
void flush_dcache_range(ulong start_addr, ulong stop)
{
}

void flush_cache(ulong start_addr, ulong size)
{
}

void invalidate_dcache_range(ulong start_addr, ulong stop)
{
}
