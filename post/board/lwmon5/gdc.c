/*
 * (C) Copyright 2008 Dmitry Rakhchev, EmCraft Systems, rda@emcraft.com
 *
 * Developed for DENX Software Engineering GmbH
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

/* This test attempts to verify board GDC. A scratch register tested, then
 * simple memory test (get_ram_size()) run over GDC memory.
 */

#include <post.h>
#include <watchdog.h>
#include <asm/io.h>
#include <video.h>

DECLARE_GLOBAL_DATA_PTR;

#define GDC_SCRATCH_REG	0xC1FF8044
#define GDC_VERSION_REG	0xC1FF8084
#define GDC_HOST_BASE	0xC1FC0000
#define GDC_RAM_START	0xC0000000
#define GDC_RAM_END	(GDC_HOST_BASE - 1)
#define GDC_RAM_SIZE	(GDC_RAM_END - GDC_RAM_START)

#if CONFIG_POST & CONFIG_SYS_POST_BSPEC4

const static unsigned long pattern[] = {
	0xffffffff,
	0xaaaaaaaa,
	0xcccccccc,
	0xf0f0f0f0,
	0xff00ff00,
	0xffff0000,
	0x0000ffff,
	0x00ff00ff,
	0x0f0f0f0f,
	0x33333333,
	0x55555555,
	0x00000000
};

const static unsigned long otherpattern = 0x01234567;

/* test write/read og a given LIME Register */
static int gdc_test_reg_one(uint value)
{
	uint read_value;

	/* write test pattern */
	out_be32((void *)GDC_SCRATCH_REG, value);
	/* read other location (protect against data lines capacity) */
	in_be32((void *)GDC_RAM_START);
	/* verify test pattern */
	read_value = in_be32((void *)GDC_SCRATCH_REG);
	if (read_value != value) {
		post_log("GDC SCRATCH test failed write %08X, read %08X\n",
			 value, read_value);
	}

	return (read_value != value);
}

/* test with a given static 32 bit pattern in a given memory addressrange */
static int gdc_post_test1(ulong *start, ulong size, ulong val)
{
	int ret = 0;
	ulong i = 0;
	ulong *mem = start;
	ulong readback;

	for (i = 0; i < size / sizeof(ulong); i++) {
		mem[i] = val;
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof(ulong); i++) {
		readback = mem[i];
		if (readback != val) {
			post_log("GDC Memory error at %08x, "
				 "wrote %08x, read %08x !\n",
				 mem + i, val, readback);
			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

/* test with dynamic 32 bit pattern in a given memory addressrange */
static int gdc_post_test2(ulong *start, ulong size)
{
	int ret = 0;
	ulong i = 0;
	ulong *mem = start;
	ulong readback;

	for (i = 0; i < size / sizeof(ulong); i++) {
		mem[i] = 1 << (i % 32);
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof(ulong); i++) {
		readback = mem[i];
		if (readback != 1 << (i % 32)) {
			post_log("GDC Memory error at %08x, "
				 "wrote %08x, read %08x !\n",
				 mem + i, 1 << (i % 32), readback);
			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

/* test with dynamic 32 bit pattern in a given memory addressrange */
static int gdc_post_test3(ulong *start, ulong size)
{
	int ret = 0;
	ulong i = 0;
	ulong *mem = start;
	ulong readback;

	for (i = 0; i < size / sizeof(ulong); i++) {
		mem[i] = i;
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof(ulong); i++) {
		readback = mem[i];
		if (readback != i) {
			post_log("GDC Memory error at %08x, "
				 "wrote %08x, read %08x !\n",
				 mem + i, i, readback);
			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

/* test with dynamic 32 bit pattern in a given memory addressrange */
static int gdc_post_test4(ulong *start, ulong size)
{
	int ret = 0;
	ulong i = 0;
	ulong *mem = start;
	ulong readback;

	for (i = 0; i < size / sizeof(ulong); i++) {
		mem[i] = ~i;
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof(ulong); i++) {
		readback = mem[i];
		if (readback != ~i) {
			post_log("GDC Memory error at %08x, "
				 "wrote %08x, read %08x !\n",
				 mem + i, ~i, readback);
			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

/* do some patterntests in a given addressrange */
int gdc_mem_test(ulong *start, ulong size)
{
	int ret = 0;

	/*
	 * check addressrange and do different static and dynamic
	 * pattern tests with it.
	 */
	if (((void *)start) + size <= (void *)GDC_RAM_END) {
		if (ret == 0)
			ret = gdc_post_test1(start, size, 0x00000000);

		if (ret == 0)
			ret = gdc_post_test1(start, size, 0xffffffff);

		if (ret == 0)
			ret = gdc_post_test1(start, size, 0x55555555);

		if (ret == 0)
			ret = gdc_post_test1(start, size, 0xaaaaaaaa);

		if (ret == 0)
			ret = gdc_post_test2(start, size);

		if (ret == 0)
			ret = gdc_post_test3(start, size);

		if (ret == 0)
			ret = gdc_post_test4(start, size);
	}

	return ret;
}

/* test function of gdc memory addresslines*/
static int gdc_post_addrline(ulong *address, ulong *base, ulong size)
{
	ulong *target;
	ulong *end;
	ulong readback = 0;
	ulong xor = 0;
	int ret = 0;

	end = (ulong *)((ulong)base + size);

	for (xor = sizeof(long); xor > 0; xor <<= 1) {
		target = (ulong *)((ulong)address ^ xor);
		if ((target >= base) && (target < end)) {
			*address = ~*target;
			readback = *target;
		}

		if (readback == *address) {
			post_log("GDC Memory (address line) error at %08x"
				 "XOR value %08x !\n",
				 address, target , xor);
			ret = -1;
			break;
		}
	}

	return ret;
}

static int gdc_post_dataline(ulong *address)
{
	unsigned long temp32 = 0;
	int i = 0;
	int ret = 0;

	for (i = 0; i < ARRAY_SIZE(pattern); i++) {
		*address = pattern[i];
		/*
		 * Put a different pattern on the data lines: otherwise they
		 * may float long enough to read back what we wrote.
		 */
		*(address + 1) = otherpattern;
		temp32 = *address;

		if (temp32 != pattern[i]){
			post_log("GDC Memory (date line) error at %08x, "
				 "wrote %08x, read %08x !\n",
				 address, pattern[i], temp32);
			ret = 1;
		}
	}

	return ret;
}

/* Verify GDC, get memory size, verify GDC memory */
int gdc_post_test(int flags)
{
	uint   	old_value;
	int 	i = 0;
	int    	ret = 0;

	post_log("\n");
	old_value = in_be32((void *)GDC_SCRATCH_REG);

	/*
	 * GPIOC2 register behaviour: the LIME graphics processor has a
	 * maximum of 5 GPIO ports that can be used in this hardware
	 * configuration. Thus only the  bits  for these 5 GPIOs can be
	 * activated in the GPIOC2 register. All other bits will always be
	 * read as zero.
	 */
	if (gdc_test_reg_one(0x00150015))
		ret = 1;
	if (gdc_test_reg_one(0x000A000A))
		ret = 1;

	out_be32((void *)GDC_SCRATCH_REG, old_value);

	old_value = in_be32((void *)GDC_VERSION_REG);
	post_log("GDC chip version %u.%u, year %04X\n",
		 (old_value >> 8) & 0xFF, old_value & 0xFF,
		 (old_value >> 16) & 0xFFFF);

	old_value = get_ram_size((void *)GDC_RAM_START,
				 0x02000000);

	debug("GDC RAM size (ist):  %d bytes\n", old_value);
	debug("GDC RAM size (soll): %d bytes\n", GDC_RAM_SIZE);
	post_log("GDC RAM size: %d bytes\n", old_value);

	/* Test SDRAM datalines */
	if (gdc_post_dataline((ulong *)GDC_RAM_START)) {
		ret = 1;
		goto out;
	}
	WATCHDOG_RESET();

	/* Test SDRAM adresslines */
	if (gdc_post_addrline((ulong *)GDC_RAM_START,
			      (ulong *)GDC_RAM_START, GDC_RAM_SIZE)) {
		ret = 1;
		goto out;
	}
	WATCHDOG_RESET();
	if (gdc_post_addrline((ulong *)GDC_RAM_END - sizeof(long),
			      (ulong *)GDC_RAM_START, GDC_RAM_SIZE)) {
		ret = 1;
		goto out;
	}
	WATCHDOG_RESET();

	/* memory pattern test */
	debug("GDC Memory test (flags %8x:%8x)\n", flags,
	      POST_SLOWTEST | POST_MANUAL);

	if (flags & POST_MANUAL) {
		debug("Full memory test\n");
		if (gdc_mem_test((ulong *)GDC_RAM_START, GDC_RAM_SIZE)) {
			ret = 1;
			goto out;
		}
		/* load splashscreen again */
	} else {
		debug("smart memory test\n");
		for (i = 0; i < (GDC_RAM_SIZE >> 20) && ret == 0; i++) {
			if (ret == 0)
				ret = gdc_mem_test((ulong *)(GDC_RAM_START +
							     (i << 20)),
						   0x800);
			if (ret == 0)
				ret = gdc_mem_test((ulong *)(GDC_RAM_START +
							     (i << 20) + 0xff800),
						   0x800);
		}
	}
	WATCHDOG_RESET();

out:
	return ret;
}
#endif /* CONFIG_POST & CONFIG_SYS_POST_BSPEC4 */
