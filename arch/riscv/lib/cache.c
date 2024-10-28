// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <cpu_func.h>
#include <dm.h>
#include <asm/insn-def.h>
#include <linux/const.h>

#define CBO_INVAL(base)						\
	INSN_I(OPCODE_MISC_MEM, FUNC3(2), __RD(0),		\
	       RS1(base), SIMM12(0))
#define CBO_CLEAN(base)						\
	INSN_I(OPCODE_MISC_MEM, FUNC3(2), __RD(0),		\
	       RS1(base), SIMM12(1))
#define CBO_FLUSH(base)						\
	INSN_I(OPCODE_MISC_MEM, FUNC3(2), __RD(0),		\
	       RS1(base), SIMM12(2))
enum {
	CBO_CLEAN,
	CBO_FLUSH,
	CBO_INVAL
} riscv_cbo_ops;
static int zicbom_block_size;

static inline void do_cbo_clean(unsigned long base)
{
	asm volatile ("add a0, %0, zero\n" CBO_CLEAN(%0) ::
		      "r"(base) : "memory");
}

static inline void do_cbo_flush(unsigned long base)
{
	asm volatile ("add a0, %0, zero\n" CBO_FLUSH(%0) ::
		      "r"(base) : "memory");
}

static inline void do_cbo_inval(unsigned long base)
{
	asm volatile ("add a0, %0, zero\n" CBO_INVAL(%0) ::
		      "r"(base) : "memory");
}

static void cbo_op(int op_type, unsigned long start,
		   unsigned long end)
{
	unsigned long op_size = end - start, size = 0;
	void (*fn)(unsigned long base);

	switch (op_type) {
	case CBO_CLEAN:
		fn = do_cbo_clean;
		break;
	case CBO_FLUSH:
		fn = do_cbo_flush;
		break;
	case CBO_INVAL:
		fn = do_cbo_inval;
		break;
	}
	start &= ~(UL(zicbom_block_size - 1));
	while (size < op_size) {
		fn(start + size);
		size += zicbom_block_size;
	}
}

void cbo_flush(unsigned long start, unsigned long end)
{
	if (zicbom_block_size)
		cbo_op(CBO_FLUSH, start, end);
}

void cbo_inval(unsigned long start, unsigned long end)
{
	if (zicbom_block_size)
		cbo_op(CBO_INVAL, start, end);
}

static int riscv_zicbom_init(void)
{
	struct udevice *dev;

	if (!CONFIG_IS_ENABLED(RISCV_ISA_ZICBOM) || zicbom_block_size)
		return 1;

	uclass_first_device(UCLASS_CPU, &dev);
	if (!dev) {
		log_debug("Failed to get cpu device!\n");
		return 0;
	}

	if (dev_read_u32(dev, "riscv,cbom-block-size", &zicbom_block_size))
		log_debug("riscv,cbom-block-size DT property not present\n");

	return zicbom_block_size;
}

void invalidate_icache_all(void)
{
	asm volatile ("fence.i" ::: "memory");
}

__weak void flush_dcache_all(void)
{
}

__weak void flush_dcache_range(unsigned long start, unsigned long end)
{
	cbo_flush(start, end);
}

__weak void invalidate_icache_range(unsigned long start, unsigned long end)
{
	/*
	 * RISC-V does not have an instruction for invalidating parts of the
	 * instruction cache. Invalidate all of it instead.
	 */
	invalidate_icache_all();
}

__weak void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	cbo_inval(start, end);
}

void cache_flush(void)
{
	invalidate_icache_all();
	flush_dcache_all();
}

void flush_cache(unsigned long addr, unsigned long size)
{
	invalidate_icache_range(addr, addr + size);
	flush_dcache_range(addr, addr + size);
}

__weak void icache_enable(void)
{
}

__weak void icache_disable(void)
{
}

__weak int icache_status(void)
{
	return 0;
}

__weak void dcache_enable(void)
{
}

__weak void dcache_disable(void)
{
}

__weak int dcache_status(void)
{
	return 0;
}

__weak void enable_caches(void)
{
	if (!riscv_zicbom_init())
		log_info("Zicbom not initialized.\n");
}
