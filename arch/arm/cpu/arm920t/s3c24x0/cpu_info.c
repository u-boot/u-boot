/*
 * (C) Copyright 2010
 * David Mueller <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
