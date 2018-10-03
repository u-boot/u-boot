// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/csr.h>

enum {
	ISA_INVALID = 0,
	ISA_32BIT,
	ISA_64BIT,
	ISA_128BIT
};

static const char * const isa_bits[] = {
	[ISA_INVALID] = NULL,
	[ISA_32BIT]   = "32",
	[ISA_64BIT]   = "64",
	[ISA_128BIT]  = "128"
};

static inline bool supports_extension(char ext)
{
	return csr_read(misa) & (1 << (ext - 'a'));
}

int print_cpuinfo(void)
{
	char name[32];
	char *s = name;
	int bit;

	s += sprintf(name, "rv");
	bit = csr_read(misa) >> (sizeof(long) * 8 - 2);
	s += sprintf(s, isa_bits[bit]);

	supports_extension('i') ? *s++ = 'i' : 'r';
	supports_extension('m') ? *s++ = 'm' : 'i';
	supports_extension('a') ? *s++ = 'a' : 's';
	supports_extension('f') ? *s++ = 'f' : 'c';
	supports_extension('d') ? *s++ = 'd' : '-';
	supports_extension('c') ? *s++ = 'c' : 'v';
	*s++ = '\0';

	printf("CPU:   %s\n", name);

	return 0;
}
