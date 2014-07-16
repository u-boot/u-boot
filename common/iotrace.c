/*
 * Copyright (c) 2014 Google, Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#define IOTRACE_IMPL

#include <common.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* Support up to the machine word length for now */
typedef ulong iovalue_t;

enum iotrace_flags {
	IOT_8 = 0,
	IOT_16,
	IOT_32,

	IOT_READ = 0 << 3,
	IOT_WRITE = 1 << 3,
};

/**
 * struct iotrace_record - Holds a single I/O trace record
 *
 * @flags: I/O access type
 * @addr: Address of access
 * @value: Value written or read
 */
struct iotrace_record {
	enum iotrace_flags flags;
	phys_addr_t addr;
	iovalue_t value;
};

/**
 * struct iotrace - current trace status and checksum
 *
 * @start:	Start address of iotrace buffer
 * @size:	Size of iotrace buffer in bytes
 * @offset:	Current write offset into iotrace buffer
 * @crc32:	Current value of CRC chceksum of trace records
 * @enabled:	true if enabled, false if disabled
 */
static struct iotrace {
	ulong start;
	ulong size;
	ulong offset;
	u32 crc32;
	bool enabled;
} iotrace;

static void add_record(int flags, const void *ptr, ulong value)
{
	struct iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	if (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		return;

	/* Store it if there is room */
	if (iotrace.offset + sizeof(*rec) < iotrace.size) {
		rec = (struct iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					sizeof(value));
	}

	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->value = value;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (unsigned char *)rec,
			      sizeof(*rec));

	iotrace.offset += sizeof(struct iotrace_record);
}

u32 iotrace_readl(const void *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	return v;
}

void iotrace_writel(ulong value, const void *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, value);
	writel(value, ptr);
}

u16 iotrace_readw(const void *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	return v;
}

void iotrace_writew(ulong value, const void *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, value);
	writew(value, ptr);
}

u8 iotrace_readb(const void *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	return v;
}

void iotrace_writeb(ulong value, const void *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, value);
	writeb(value, ptr);
}

void iotrace_reset_checksum(void)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(void)
{
	return iotrace.crc32;
}

void iotrace_set_enabled(int enable)
{
	iotrace.enabled = enable;
}

int iotrace_get_enabled(void)
{
	return iotrace.enabled;
}

void iotrace_set_buffer(ulong start, ulong size)
{
	iotrace.start = start;
	iotrace.size = size;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

void iotrace_get_buffer(ulong *start, ulong *size, ulong *offset, ulong *count)
{
	*start = iotrace.start;
	*size = iotrace.size;
	*offset = iotrace.offset;
	*count = iotrace.offset / sizeof(struct iotrace_record);
}
