/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/immap.h>
#include <asm/cache.h>

volatile int *cf_icache_status = (int *)ICACHE_STATUS;
volatile int *cf_dcache_status = (int *)DCACHE_STATUS;

void flush_cache(ulong start_addr, ulong size)
{
	/* Must be implemented for all M68k processors with copy-back data cache */
}

int icache_status(void)
{
	return *cf_icache_status;
}

int dcache_status(void)
{
	return *cf_dcache_status;
}

void icache_enable(void)
{
	icache_invalid();

	*cf_icache_status = 1;

#ifdef CONFIG_CF_V4
	__asm__ __volatile__("movec %0, %%acr2"::"r"(CONFIG_SYS_CACHE_ACR2));
	__asm__ __volatile__("movec %0, %%acr3"::"r"(CONFIG_SYS_CACHE_ACR3));
#elif defined(CONFIG_CF_V4e)
	__asm__ __volatile__("movec %0, %%acr6"::"r"(CONFIG_SYS_CACHE_ACR6));
	__asm__ __volatile__("movec %0, %%acr7"::"r"(CONFIG_SYS_CACHE_ACR7));
#else
	__asm__ __volatile__("movec %0, %%acr0"::"r"(CONFIG_SYS_CACHE_ACR0));
	__asm__ __volatile__("movec %0, %%acr1"::"r"(CONFIG_SYS_CACHE_ACR1));
#endif

	__asm__ __volatile__("movec %0, %%cacr"::"r"(CONFIG_SYS_CACHE_ICACR));
}

void icache_disable(void)
{
	u32 temp = 0;

	*cf_icache_status = 0;
	icache_invalid();

#ifdef CONFIG_CF_V4
	__asm__ __volatile__("movec %0, %%acr2"::"r"(temp));
	__asm__ __volatile__("movec %0, %%acr3"::"r"(temp));
#elif defined(CONFIG_CF_V4e)
	__asm__ __volatile__("movec %0, %%acr6"::"r"(temp));
	__asm__ __volatile__("movec %0, %%acr7"::"r"(temp));
#else
	__asm__ __volatile__("movec %0, %%acr0"::"r"(temp));
	__asm__ __volatile__("movec %0, %%acr1"::"r"(temp));

#endif
}

void icache_invalid(void)
{
	u32 temp;

	temp = CONFIG_SYS_ICACHE_INV;
	if (*cf_icache_status)
		temp |= CONFIG_SYS_CACHE_ICACR;

	__asm__ __volatile__("movec %0, %%cacr"::"r"(temp));
}

/*
 * data cache only for ColdFire V4 such as MCF547x_8x, MCF5445x
 * the dcache will be dummy in ColdFire V2 and V3
 */
void dcache_enable(void)
{
	dcache_invalid();
	*cf_dcache_status = 1;

#ifdef CONFIG_CF_V4
	__asm__ __volatile__("movec %0, %%acr0"::"r"(CONFIG_SYS_CACHE_ACR0));
	__asm__ __volatile__("movec %0, %%acr1"::"r"(CONFIG_SYS_CACHE_ACR1));
#elif defined(CONFIG_CF_V4e)
	__asm__ __volatile__("movec %0, %%acr4"::"r"(CONFIG_SYS_CACHE_ACR4));
	__asm__ __volatile__("movec %0, %%acr5"::"r"(CONFIG_SYS_CACHE_ACR5));

#endif

	__asm__ __volatile__("movec %0, %%cacr"::"r"(CONFIG_SYS_CACHE_DCACR));
}

void dcache_disable(void)
{
	u32 temp = 0;

	*cf_dcache_status = 0;
	dcache_invalid();

	__asm__ __volatile__("movec %0, %%cacr"::"r"(temp));

#ifdef CONFIG_CF_V4
	__asm__ __volatile__("movec %0, %%acr0"::"r"(temp));
	__asm__ __volatile__("movec %0, %%acr1"::"r"(temp));
#elif defined(CONFIG_CF_V4e)
	__asm__ __volatile__("movec %0, %%acr4"::"r"(temp));
	__asm__ __volatile__("movec %0, %%acr5"::"r"(temp));

#endif
}

void dcache_invalid(void)
{
#ifdef CONFIG_CF_V4
	u32 temp;

	temp = CONFIG_SYS_DCACHE_INV;
	if (*cf_dcache_status)
		temp |= CONFIG_SYS_CACHE_DCACR;
	if (*cf_icache_status)
		temp |= CONFIG_SYS_CACHE_ICACR;

	__asm__ __volatile__("movec %0, %%cacr"::"r"(temp));
#endif
}
