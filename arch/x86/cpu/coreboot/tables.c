/*
 * This file is part of the libpayload project.
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 * Copyright (C) 2009 coresystems GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <asm/arch-coreboot/ipchecksum.h>
#include <asm/arch-coreboot/sysinfo.h>
#include <asm/arch-coreboot/tables.h>

/*
 * Some of this is x86 specific, and the rest of it is generic. Right now,
 * since we only support x86, we'll avoid trying to make lots of infrastructure
 * we don't need. If in the future, we want to use coreboot on some other
 * architecture, then take out the generic parsing code and move it elsewhere.
 */

/* === Parsing code === */
/* This is the generic parsing code. */

static void cb_parse_memory(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_memory *mem = (struct cb_memory *)ptr;
	int count = MEM_RANGE_COUNT(mem);
	int i;

	if (count > SYSINFO_MAX_MEM_RANGES)
		count = SYSINFO_MAX_MEM_RANGES;

	info->n_memranges = 0;

	for (i = 0; i < count; i++) {
		struct cb_memory_range *range =
		    (struct cb_memory_range *)MEM_RANGE_PTR(mem, i);

		info->memrange[info->n_memranges].base =
		    UNPACK_CB64(range->start);

		info->memrange[info->n_memranges].size =
		    UNPACK_CB64(range->size);

		info->memrange[info->n_memranges].type = range->type;

		info->n_memranges++;
	}
}

static void cb_parse_serial(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_serial *ser = (struct cb_serial *)ptr;
	if (ser->type != CB_SERIAL_TYPE_IO_MAPPED)
		return;
	info->ser_ioport = ser->baseaddr;
}

static void cb_parse_optiontable(unsigned char *ptr, struct sysinfo_t *info)
{
	info->option_table = (struct cb_cmos_option_table *)ptr;
}

static void cb_parse_checksum(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_cmos_checksum *cmos_cksum = (struct cb_cmos_checksum *)ptr;
	info->cmos_range_start = cmos_cksum->range_start;
	info->cmos_range_end = cmos_cksum->range_end;
	info->cmos_checksum_location = cmos_cksum->location;
}

static void cb_parse_framebuffer(unsigned char *ptr, struct sysinfo_t *info)
{
	info->framebuffer = (struct cb_framebuffer *)ptr;
}

static int cb_parse_header(void *addr, int len, struct sysinfo_t *info)
{
	struct cb_header *header;
	unsigned char *ptr = (unsigned char *)addr;
	int i;

	for (i = 0; i < len; i += 16, ptr += 16) {
		header = (struct cb_header *)ptr;
		if (!strncmp((const char *)header->signature, "LBIO", 4))
			break;
	}

	/* We walked the entire space and didn't find anything. */
	if (i >= len)
		return -1;

	if (!header->table_bytes)
		return 0;

	/* Make sure the checksums match. */
	if (ipchksum((u16 *) header, sizeof(*header)) != 0)
		return -1;

	if (ipchksum((u16 *) (ptr + sizeof(*header)),
		     header->table_bytes) != header->table_checksum)
		return -1;

	/* Now, walk the tables. */
	ptr += header->header_bytes;

	for (i = 0; i < header->table_entries; i++) {
		struct cb_record *rec = (struct cb_record *)ptr;

		/* We only care about a few tags here (maybe more later). */
		switch (rec->tag) {
		case CB_TAG_FORWARD:
			return cb_parse_header(
				(void *)(unsigned long)
				((struct cb_forward *)rec)->forward,
				len, info);
			continue;
		case CB_TAG_MEMORY:
			cb_parse_memory(ptr, info);
			break;
		case CB_TAG_SERIAL:
			cb_parse_serial(ptr, info);
			break;
		case CB_TAG_CMOS_OPTION_TABLE:
			cb_parse_optiontable(ptr, info);
			break;
		case CB_TAG_OPTION_CHECKSUM:
			cb_parse_checksum(ptr, info);
			break;
		/*
		 * FIXME we should warn on serial if coreboot set up a
		 * framebuffer buf the payload does not know about it.
		 */
		case CB_TAG_FRAMEBUFFER:
			cb_parse_framebuffer(ptr, info);
			break;
		}

		ptr += rec->size;
	}

	return 1;
}

/* == Architecture specific == */
/* This is the x86 specific stuff. */

/* Assume no translation or that memory is identity mapped. */
static void *phys_to_virt(unsigned long virt)
{
	return (void *)(uintptr_t)virt;
}

int get_coreboot_info(struct sysinfo_t *info)
{
	int ret = cb_parse_header(phys_to_virt(0x00000000), 0x1000, info);

	if (ret != 1)
		ret = cb_parse_header(phys_to_virt(0x000f0000), 0x1000, info);

	return (ret == 1) ? 0 : -1;
}
