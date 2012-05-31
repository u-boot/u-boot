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

/* This test performs testing of FPGA SCRATCH register,
 * gets FPGA version and run get_ram_size() on FPGA memory
 */

#include <post.h>
#include <watchdog.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define FPGA_SCRATCH_REG	0xC4000050
#define FPGA_VERSION_REG	0xC4000040
#define FPGA_RAM_START		0xC4200000
#define FPGA_RAM_END		0xC4203FFF
#define FPGA_STAT		0xC400000C
#define FPGA_BUFFER		0x00800000
#define FPGA_RAM_SIZE		(FPGA_RAM_END - FPGA_RAM_START + 1)

#if CONFIG_POST & CONFIG_SYS_POST_BSPEC3

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
	0x00000000,
};

const static unsigned long otherpattern = 0x01234567;

static int one_scratch_test(uint value)
{
	uint read_value;
	int ret = 0;

	out_be32((void *)FPGA_SCRATCH_REG, value);
	/* read other location (protect against data lines capacity) */
	ret = in_be16((void *)FPGA_VERSION_REG);
	/* verify test pattern */
	read_value = in_be32((void *)FPGA_SCRATCH_REG);
	if (read_value != value) {
		post_log("FPGA SCRATCH test failed write %08X, read %08X\n",
			 value, read_value);
		ret = -1;
	}

	return ret;
}

static int fpga_post_test1(ulong *start, ulong size, ulong val)
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
			post_log("FPGA Memory error at %08x, "
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

static int fpga_post_test2(ulong *start, ulong size)
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
			post_log("FPGA Memory error at %08x, "
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

static int fpga_post_test3(ulong *start, ulong size)
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
			post_log("FPGA Memory error at %08x, "
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

static int fpga_post_test4(ulong *start, ulong size)
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
			post_log("FPGA Memory error at %08x, "
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

/* FPGA Memory-pattern-test */
static int fpga_mem_test(void)
{
	int ret = 0;
	ulong* start = (ulong *)FPGA_RAM_START;
	ulong  size  = FPGA_RAM_SIZE;

	if (ret == 0)
		ret = fpga_post_test1(start, size, 0x00000000);

	if (ret == 0)
		ret = fpga_post_test1(start, size, 0xffffffff);

	if (ret == 0)
		ret = fpga_post_test1(start, size, 0x55555555);

	if (ret == 0)
		ret = fpga_post_test1(start, size, 0xaaaaaaaa);

	WATCHDOG_RESET();

	if (ret == 0)
		ret = fpga_post_test2(start, size);

	if (ret == 0)
		ret = fpga_post_test3(start, size);

	if (ret == 0)
		ret = fpga_post_test4(start, size);

	return ret;
}

/* Verify FPGA addresslines */
static int fpga_post_addrline(ulong *address, ulong *base, ulong size)
{
	unsigned long *target;
	unsigned long *end;
	unsigned long readback;
	unsigned long xor;
	int ret = 0;

	end = (ulong *)((ulong)base + size);
	xor = 0;

	for (xor = sizeof(ulong); xor > 0; xor <<= 1) {
		target = (ulong*)((ulong)address ^ xor);
		if ((target >= base) && (target < end)) {
			*address = ~*target;
			readback = *target;

			if (readback == *address) {
				post_log("Memory (address line) error at %08x"
					 "XOR value %08x !\n",
					 address, target, xor);
				ret = -1;
				break;
			}
		}
	}

	return ret;
}

/* Verify FPGA addresslines */
static int fpga_post_dataline(ulong *address)
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
			post_log("Memory (date line) error at %08x, "
				 "wrote %08x, read %08x !\n",
				 address, pattern[i], temp32);
			ret = 1;
		}
	}

	return ret;
}

/* Verify FPGA, get version & memory size */
int fpga_post_test(int flags)
{
	uint   old_value;
	uint   version;
	uint   read_value;
	int    ret = 0;

	post_log("\n");
	old_value = in_be32((void *)FPGA_SCRATCH_REG);

	if (one_scratch_test(0x55555555))
		ret = 1;
	if (one_scratch_test(0xAAAAAAAA))
		ret = 1;

	out_be32((void *)FPGA_SCRATCH_REG, old_value);

	version = in_be32((void *)FPGA_VERSION_REG);
	post_log("FPGA version %u.%u\n",
		 (version >> 8) & 0xFF, version & 0xFF);

	/* Enable write to FPGA RAM */
	out_be32((void *)FPGA_STAT, in_be32((void *)FPGA_STAT) | 0x1000);

	/* get RAM size */
	read_value = get_ram_size((void *)CONFIG_SYS_FPGA_BASE_1, FPGA_RAM_SIZE);
	post_log("FPGA RAM size %d bytes\n", read_value);
	WATCHDOG_RESET();

	/* copy fpga memory to DDR2 RAM*/
	memcpy((void *)FPGA_BUFFER,(void *)FPGA_RAM_START, FPGA_RAM_SIZE);
	WATCHDOG_RESET();

	/* Test datalines */
	if (fpga_post_dataline((ulong *)FPGA_RAM_START)) {
		ret = 1;
		goto out;
	}
	WATCHDOG_RESET();

	/* Test addresslines */
	if (fpga_post_addrline((ulong *)FPGA_RAM_START,
			       (ulong *)FPGA_RAM_START, FPGA_RAM_SIZE)) {
		ret = 1;
		goto out;
	}
	WATCHDOG_RESET();
	if (fpga_post_addrline((ulong *)FPGA_RAM_END - sizeof(long),
			       (ulong *)FPGA_RAM_START, FPGA_RAM_SIZE)) {
		ret = 1;
		goto out;
	}
	WATCHDOG_RESET();

	/* Memory Pattern Test */
	if (fpga_mem_test()) {
		ret = 1;
		goto out;
	}
	WATCHDOG_RESET();

	/* restore memory */
	memcpy((void *)FPGA_RAM_START,(void *)FPGA_BUFFER, FPGA_RAM_SIZE);
	WATCHDOG_RESET();

out:
	/* Disable write to RAM */
	out_be32((void *)FPGA_STAT, in_be32((void *)FPGA_STAT) & 0xEFFF);
	return ret;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_BSPEC3 */
