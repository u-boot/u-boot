/*
 * (C) Copyright 2010
 * David Mueller <d.mueller@elsoft.ch>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/s3c24x0_cpu.h>

typedef ulong (*getfreq)(void);

static const getfreq freq_f[] = {
	get_FCLK,
	get_HCLK,
	get_PCLK,
};

static const char freq_c[] = { 'F', 'H', 'P' };

int print_cpuinfo(void)
{
	int i;
	char buf[32];
/* the S3C2400 seems to be lacking a CHIP ID register */
#ifndef CONFIG_S3C2400
	ulong cpuid;
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	cpuid = readl(&gpio->gstatus1);
	printf("CPUID: %8lX\n", cpuid);
#endif
	for (i = 0; i < ARRAY_SIZE(freq_f); i++)
		printf("%cCLK: %8s MHz\n", freq_c[i], strmhz(buf, freq_f[i]()));

	return 0;
}
