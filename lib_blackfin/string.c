/*
 * U-boot - string.c Contains library routines.
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <config.h>
#include <asm/blackfin.h>
#include <asm/io.h>
#include <asm/mach-common/bits/dma.h>

char *strcpy(char *dest, const char *src)
{
	char *xdest = dest;
	char temp = 0;

	__asm__ __volatile__ (
		"1:\t%2 = B [%1++] (Z);\n\t"
		"B [%0++] = %2;\n\t"
		"CC = %2;\n\t"
		"if cc jump 1b (bp);\n"
		: "=a"(dest), "=a"(src), "=d"(temp)
		: "0"(dest), "1"(src), "2"(temp)
		: "memory");

	return xdest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	char *xdest = dest;
	char temp = 0;

	if (n == 0)
		return xdest;

	__asm__ __volatile__ (
		"1:\t%3 = B [%1++] (Z);\n\t"
		"B [%0++] = %3;\n\t"
		"CC = %3;\n\t"
		"if ! cc jump 2f;\n\t"
		"%2 += -1;\n\t"
		"CC = %2 == 0;\n\t"
		"if ! cc jump 1b (bp);\n"
		"2:\n"
		: "=a"(dest), "=a"(src), "=da"(n), "=d"(temp)
		: "0"(dest), "1"(src), "2"(n), "3"(temp)
		: "memory");

	return xdest;
}

int strcmp(const char *cs, const char *ct)
{
	char __res1, __res2;

	__asm__ (
		"1:\t%2 = B[%0++] (Z);\n\t"	/* get *cs */
		"%3 = B[%1++] (Z);\n\t"	/* get *ct */
		"CC = %2 == %3;\n\t"	/* compare a byte */
		"if ! cc jump 2f;\n\t"	/* not equal, break out */
		"CC = %2;\n\t"	/* at end of cs? */
		"if cc jump 1b (bp);\n\t"	/* no, keep going */
		"jump.s 3f;\n"	/* strings are equal */
		"2:\t%2 = %2 - %3;\n"	/* *cs - *ct */
		"3:\n"
		: "=a"(cs), "=a"(ct), "=d"(__res1), "=d"(__res2)
		: "0"(cs), "1"(ct));

	return __res1;
}

int strncmp(const char *cs, const char *ct, size_t count)
{
	char __res1, __res2;

	if (!count)
		return 0;

	__asm__(
		"1:\t%3 = B[%0++] (Z);\n\t"	/* get *cs */
		"%4 = B[%1++] (Z);\n\t"	/* get *ct */
		"CC = %3 == %4;\n\t"	/* compare a byte */
		"if ! cc jump 3f;\n\t"	/* not equal, break out */
		"CC = %3;\n\t"	/* at end of cs? */
		"if ! cc jump 4f;\n\t"	/* yes, all done */
		"%2 += -1;\n\t"	/* no, adjust count */
		"CC = %2 == 0;\n\t" "if ! cc jump 1b;\n"	/* more to do, keep going */
		"2:\t%3 = 0;\n\t"	/* strings are equal */
		"jump.s    4f;\n" "3:\t%3 = %3 - %4;\n"	/* *cs - *ct */
		"4:"
		: "=a"(cs), "=a"(ct), "=da"(count), "=d"(__res1), "=d"(__res2)
		: "0"(cs), "1"(ct), "2"(count));

	return __res1;
}

#ifdef bfin_write_MDMA1_D0_IRQ_STATUS
# define bfin_write_MDMA_D0_IRQ_STATUS bfin_write_MDMA1_D0_IRQ_STATUS
# define bfin_write_MDMA_D0_START_ADDR bfin_write_MDMA1_D0_START_ADDR
# define bfin_write_MDMA_D0_X_COUNT    bfin_write_MDMA1_D0_X_COUNT
# define bfin_write_MDMA_D0_X_MODIFY   bfin_write_MDMA1_D0_X_MODIFY
# define bfin_write_MDMA_D0_CONFIG     bfin_write_MDMA1_D0_CONFIG
# define bfin_write_MDMA_S0_START_ADDR bfin_write_MDMA1_S0_START_ADDR
# define bfin_write_MDMA_S0_X_COUNT    bfin_write_MDMA1_S0_X_COUNT
# define bfin_write_MDMA_S0_X_MODIFY   bfin_write_MDMA1_S0_X_MODIFY
# define bfin_write_MDMA_S0_CONFIG     bfin_write_MDMA1_S0_CONFIG
# define bfin_write_MDMA_D0_IRQ_STATUS bfin_write_MDMA1_D0_IRQ_STATUS
# define bfin_read_MDMA_D0_IRQ_STATUS  bfin_read_MDMA1_D0_IRQ_STATUS
#endif
static void *dma_memcpy(void *dst, const void *src, size_t count)
{
	if (dcache_status())
		blackfin_dcache_flush_range(src, src + count);

	bfin_write_MDMA_D0_IRQ_STATUS(DMA_DONE | DMA_ERR);

	/* Copy sram functions from sdram to sram */
	/* Setup destination start address */
	bfin_write_MDMA_D0_START_ADDR(dst);
	/* Setup destination xcount */
	bfin_write_MDMA_D0_X_COUNT(count);
	/* Setup destination xmodify */
	bfin_write_MDMA_D0_X_MODIFY(1);

	/* Setup Source start address */
	bfin_write_MDMA_S0_START_ADDR(src);
	/* Setup Source xcount */
	bfin_write_MDMA_S0_X_COUNT(count);
	/* Setup Source xmodify */
	bfin_write_MDMA_S0_X_MODIFY(1);

	/* Enable source DMA */
	bfin_write_MDMA_S0_CONFIG(DMAEN);
	SSYNC();

	bfin_write_MDMA_D0_CONFIG(WNR | DMAEN);

	while (bfin_read_MDMA_D0_IRQ_STATUS() & DMA_RUN)
		bfin_write_MDMA_D0_IRQ_STATUS(bfin_read_MDMA_D0_IRQ_STATUS() | DMA_DONE | DMA_ERR);
	bfin_write_MDMA_D0_IRQ_STATUS(bfin_read_MDMA_D0_IRQ_STATUS() | DMA_DONE | DMA_ERR);

	if (icache_status())
		blackfin_icache_flush_range(dst, dst + count);

	if (dcache_status())
		blackfin_dcache_invalidate_range(dst, dst + count);

	return dst;
}

/*
 * memcpy - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * We need to have this wrapper in memcpy() as common code may call memcpy()
 * to load up L1 regions.  Consider loading an ELF which has sections with
 * LMA's pointing to L1.  The common code ELF loader will simply use memcpy()
 * to move the ELF's sections into the right place.  We need to catch that
 * here and redirect to dma_memcpy().
 */
extern void *memcpy_ASM(void *dst, const void *src, size_t count);
void *memcpy(void *dst, const void *src, size_t count)
{
	if (!count)
		return dst;

	if (addr_bfin_on_chip_mem(dst)) {
		/* L1 is the destination */
		return dma_memcpy(dst, src, count);

	} else if (addr_bfin_on_chip_mem(src)) {
		/* L1 is the source */
		return dma_memcpy(dst, src, count);

	} else
		/* No L1 is involved, so just call regular memcpy */
		return memcpy_ASM(dst, src, count);
}
