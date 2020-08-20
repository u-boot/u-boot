/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#ifndef __CVMX_REGS_H__
#define __CVMX_REGS_H__

#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/io.h>

/* General defines */
#define CVMX_MAX_CORES		48
/* Maximum # of bits to define core in node */
#define CVMX_NODE_NO_SHIFT	7
#define CVMX_NODE_BITS		2	/* Number of bits to define a node */
#define CVMX_MAX_NODES		(1 << CVMX_NODE_BITS)
#define CVMX_NODE_MASK		(CVMX_MAX_NODES - 1)
#define CVMX_NODE_IO_SHIFT	36
#define CVMX_NODE_MEM_SHIFT	40
#define CVMX_NODE_IO_MASK	((u64)CVMX_NODE_MASK << CVMX_NODE_IO_SHIFT)

#define CVMX_MIPS_MAX_CORE_BITS	10	/* Maximum # of bits to define cores */
#define CVMX_MIPS_MAX_CORES	(1 << CVMX_MIPS_MAX_CORE_BITS)

#define MAX_CORE_TADS		8

#define CAST_ULL(v)		((unsigned long long)(v))
#define CASTPTR(type, v)	((type *)(long)(v))

/* Regs */
#define CVMX_CIU_PP_RST		0x0001010000000100ULL
#define CVMX_CIU3_NMI		0x0001010000000160ULL
#define CVMX_CIU_FUSE		0x00010100000001a0ULL
#define CVMX_CIU_NMI		0x0001070000000718ULL

#define CVMX_MIO_BOOT_LOC_CFGX(x) (0x0001180000000080ULL + ((x) & 1) * 8)
#define MIO_BOOT_LOC_CFG_BASE		GENMASK_ULL(27, 3)
#define MIO_BOOT_LOC_CFG_EN		BIT_ULL(31)

#define CVMX_MIO_BOOT_LOC_ADR	0x0001180000000090ULL
#define MIO_BOOT_LOC_ADR_ADR		GENMASK_ULL(7, 3)

#define CVMX_MIO_BOOT_LOC_DAT	0x0001180000000098ULL

#define CVMX_MIO_FUS_DAT2	0x0001180000001410ULL
#define MIO_FUS_DAT2_NOCRYPTO		BIT_ULL(26)
#define MIO_FUS_DAT2_NOMUL		BIT_ULL(27)
#define MIO_FUS_DAT2_DORM_CRYPTO	BIT_ULL(34)

#define CVMX_MIO_FUS_RCMD	0x0001180000001500ULL
#define MIO_FUS_RCMD_ADDR		GENMASK_ULL(7, 0)
#define MIO_FUS_RCMD_PEND		BIT_ULL(12)
#define MIO_FUS_RCMD_DAT		GENMASK_ULL(23, 16)

#define CVMX_RNM_CTL_STATUS	0x0001180040000000ULL
#define RNM_CTL_STATUS_EER_VAL		BIT_ULL(9)

/* turn the variable name into a string */
#define CVMX_TMP_STR(x)		CVMX_TMP_STR2(x)
#define CVMX_TMP_STR2(x)	#x

#define CVMX_RDHWRNV(result, regstr)					\
	asm volatile ("rdhwr %[rt],$" CVMX_TMP_STR(regstr) : [rt] "=d" (result))

#define CVMX_SYNCW					\
	asm volatile ("syncw\nsyncw\n" : : : "memory")

/* ToDo: Currently only node = 0 supported */
static inline u64 csr_rd_node(int node, u64 addr)
{
	void __iomem *base;

	base = ioremap_nocache(addr, 0x100);
	return ioread64(base);
}

static inline u64 csr_rd(u64 addr)
{
	return csr_rd_node(0, addr);
}

static inline void csr_wr_node(int node, u64 addr, u64 val)
{
	void __iomem *base;

	base = ioremap_nocache(addr, 0x100);
	iowrite64(val, base);
}

static inline void csr_wr(u64 addr, u64 val)
{
	csr_wr_node(0, addr, val);
}

/*
 * We need to use the volatile access here, otherwise the IO accessor
 * functions might swap the bytes
 */
static inline u64 cvmx_read64_uint64(u64 addr)
{
	return *(volatile u64 *)addr;
}

static inline void cvmx_write64_uint64(u64 addr, u64 val)
{
	*(volatile u64 *)addr = val;
}

static inline u32 cvmx_read64_uint32(u64 addr)
{
	return *(volatile u32 *)addr;
}

static inline void cvmx_write64_uint32(u64 addr, u32 val)
{
	*(volatile u32 *)addr = val;
}

static inline void *cvmx_phys_to_ptr(u64 addr)
{
	return (void *)CKSEG0ADDR(addr);
}

static inline u64 cvmx_ptr_to_phys(void *ptr)
{
	return virt_to_phys(ptr);
}

/**
 * Number of the Core on which the program is currently running.
 *
 * @return core number
 */
static inline unsigned int cvmx_get_core_num(void)
{
	unsigned int core_num;

	CVMX_RDHWRNV(core_num, 0);
	return core_num;
}

#endif /* __CVMX_REGS_H__ */
