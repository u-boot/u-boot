/*
 * This file is part of the libpayload project.
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
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

#ifndef _COREBOOT_SYSINFO_H
#define _COREBOOT_SYSINFO_H

#include <compiler.h>

/* Allow a maximum of 16 memory range definitions. */
#define SYSINFO_MAX_MEM_RANGES 16

struct sysinfo_t {
	unsigned int cpu_khz;
	unsigned short ser_ioport;
	unsigned long ser_base; /* for mmapped serial */

	int n_memranges;

	struct memrange {
		unsigned long long base;
		unsigned long long size;
		unsigned int type;
	} memrange[SYSINFO_MAX_MEM_RANGES];

	struct cb_cmos_option_table *option_table;
	u32 cmos_range_start;
	u32 cmos_range_end;
	u32 cmos_checksum_location;

	struct cb_framebuffer *framebuffer;

	unsigned long *mbtable; /** Pointer to the multiboot table */
};

extern struct sysinfo_t lib_sysinfo;

#endif
