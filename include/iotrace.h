/*
 * Copyright (c) 2014 Google, Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __IOTRACE_H
#define __IOTRACE_H

#include <linux/types.h>

/*
 * This file is designed to be included in arch/<arch>/include/asm/io.h.
 * It redirects all IO access through a tracing/checksumming feature for
 * testing purposes.
 */

#if defined(CONFIG_IO_TRACE) && !defined(IOTRACE_IMPL) && \
	!defined(CONFIG_SPL_BUILD)

#undef readl
#define readl(addr)	iotrace_readl((const void *)(addr))

#undef writel
#define writel(val, addr)	iotrace_writel(val, (const void *)(addr))

#undef readw
#define readw(addr)	iotrace_readw((const void *)(addr))

#undef writew
#define writew(val, addr)	iotrace_writew(val, (const void *)(addr))

#undef readb
#define readb(addr)	iotrace_readb((const void *)(uintptr_t)addr)

#undef writeb
#define writeb(val, addr) \
	iotrace_writeb(val, (const void *)(uintptr_t)addr)

#endif

/* Tracing functions which mirror their io.h counterparts */
u32 iotrace_readl(const void *ptr);
void iotrace_writel(ulong value, const void *ptr);
u16 iotrace_readw(const void *ptr);
void iotrace_writew(ulong value, const void *ptr);
u8 iotrace_readb(const void *ptr);
void iotrace_writeb(ulong value, const void *ptr);

/**
 * iotrace_reset_checksum() - Reset the iotrace checksum
 */
void iotrace_reset_checksum(void);

/**
 * iotrace_get_checksum() - Get the current checksum value
 *
 * @return currect checksum value
 */
u32 iotrace_get_checksum(void);

/**
 * iotrace_set_enabled() - Set whether iotracing is enabled or not
 *
 * This controls whether the checksum is updated and a trace record added
 * for each I/O access.
 *
 * @enable: true to enable iotracing, false to disable
 */
void iotrace_set_enabled(int enable);

/**
 * iotrace_get_enabled() - Get whether iotracing is enabled or not
 *
 * @return true if enabled, false if disabled
 */
int iotrace_get_enabled(void);

/**
 * iotrace_set_buffer() - Set position and size of iotrace buffer
 *
 * Defines where the iotrace buffer goes, and resets the output pointer to
 * the start of the buffer.
 *
 * The buffer can be 0 size in which case the checksum is updated but no
 * trace records are writen. If the buffer is exhausted, the offset will
 * continue to increase but not new data will be written.
 *
 * @start: Start address of buffer
 * @size: Size of buffer in bytes
 */
void iotrace_set_buffer(ulong start, ulong size);

/**
 * iotrace_get_buffer() - Get buffer information
 *
 * @start: Returns start address of buffer
 * @size: Returns size of buffer in bytes
 * @offset: Returns the byte offset where the next output trace record will
 * @count: Returns the number of trace records recorded
 * be written (or would be if the buffer was large enough)
 */
void iotrace_get_buffer(ulong *start, ulong *size, ulong *offset, ulong *count);

#endif /* __IOTRACE_H */
