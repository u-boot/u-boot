/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#ifndef __CVMX_REGS_H__
#define __CVMX_REGS_H__

#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <mach/cvmx-address.h>

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

#define CASTPTR(type, v)	((type *)(long)(v))
#define CAST64(v)		((long long)(long)(v))

/* Regs */
#define CVMX_CIU3_NMI		0x0001010000000160ULL

#define CVMX_MIO_BOOT_LOC_CFGX(x) (0x0001180000000080ULL + ((x) & 1) * 8)
#define MIO_BOOT_LOC_CFG_BASE	GENMASK_ULL(27, 3)
#define MIO_BOOT_LOC_CFG_EN	BIT_ULL(31)

#define CVMX_MIO_BOOT_LOC_ADR	0x0001180000000090ULL
#define MIO_BOOT_LOC_ADR_ADR	GENMASK_ULL(7, 3)

#define CVMX_MIO_BOOT_LOC_DAT	0x0001180000000098ULL

#define CVMX_MIO_FUS_DAT2	0x0001180000001410ULL
#define MIO_FUS_DAT2_NOCRYPTO	BIT_ULL(26)
#define MIO_FUS_DAT2_NOMUL	BIT_ULL(27)
#define MIO_FUS_DAT2_DORM_CRYPTO BIT_ULL(34)

#define CVMX_MIO_FUS_RCMD	0x0001180000001500ULL
#define MIO_FUS_RCMD_ADDR	GENMASK_ULL(7, 0)
#define MIO_FUS_RCMD_PEND	BIT_ULL(12)
#define MIO_FUS_RCMD_DAT	GENMASK_ULL(23, 16)

#define CVMX_RNM_CTL_STATUS	0x0001180040000000ULL
#define RNM_CTL_STATUS_EER_VAL	BIT_ULL(9)

#define CVMX_IOBDMA_ORDERED_IO_ADDR 0xffffffffffffa200ull

/* turn the variable name into a string */
#define CVMX_TMP_STR(x)		CVMX_TMP_STR2(x)
#define CVMX_TMP_STR2(x)	#x

#define CVMX_RDHWR(result, regstr)					\
	asm volatile("rdhwr %[rt],$" CVMX_TMP_STR(regstr) : [rt] "=d"(result))
#define CVMX_RDHWRNV(result, regstr)					\
	asm("rdhwr %[rt],$" CVMX_TMP_STR(regstr) : [rt] "=d"(result))
#define CVMX_POP(result, input)						\
	asm("pop %[rd],%[rs]" : [rd] "=d"(result) : [rs] "d"(input))

#define CVMX_SYNC   asm volatile("sync\n" : : : "memory")
#define CVMX_SYNCW  asm volatile("syncw\nsyncw\n" : : : "memory")
#define CVMX_SYNCS  asm volatile("syncs\n" : : : "memory")
#define CVMX_SYNCWS asm volatile("syncws\n" : : : "memory")

#define CVMX_CACHE_LINE_SIZE	128			   // In bytes
#define CVMX_CACHE_LINE_MASK	(CVMX_CACHE_LINE_SIZE - 1) // In bytes
#define CVMX_CACHE_LINE_ALIGNED __aligned(CVMX_CACHE_LINE_SIZE)

#define CVMX_SYNCIOBDMA		asm volatile("synciobdma" : : : "memory")

#define CVMX_MF_CHORD(dest)	CVMX_RDHWR(dest, 30)

/*
 * The macros cvmx_likely and cvmx_unlikely use the
 * __builtin_expect GCC operation to control branch
 * probabilities for a conditional. For example, an "if"
 * statement in the code that will almost always be
 * executed should be written as "if (cvmx_likely(...))".
 * If the "else" section of an if statement is more
 * probable, use "if (cvmx_unlikey(...))".
 */
#define cvmx_likely(x)	 __builtin_expect(!!(x), 1)
#define cvmx_unlikely(x) __builtin_expect(!!(x), 0)

#define CVMX_WAIT_FOR_FIELD64(address, type, field, op, value, to_us)	\
	({								\
		int result;						\
		do {							\
			u64 done = get_timer(0);			\
			type c;						\
			while (1) {					\
				c.u64 = csr_rd(address);		\
				if ((c.s.field)op(value)) {		\
					result = 0;			\
					break;				\
				} else if (get_timer(done) > ((to_us) / 1000)) { \
					result = -1;			\
					break;				\
				} else					\
					udelay(100);			\
			}						\
		} while (0);						\
		result;							\
	})

#define CVMX_WAIT_FOR_FIELD64_NODE(node, address, type, field, op, value, to_us) \
	({								\
		int result;						\
		do {							\
			u64 done = get_timer(0);			\
			type c;						\
			while (1) {					\
				c.u64 = csr_rd(address);		\
				if ((c.s.field)op(value)) {		\
					result = 0;			\
					break;				\
				} else if (get_timer(done) > ((to_us) / 1000)) { \
					result = -1;			\
					break;				\
				} else					\
					udelay(100);			\
			}						\
		} while (0);						\
		result;							\
	})

/* ToDo: Currently only node = 0 supported */
#define cvmx_get_node_num()	0

static inline u64 csr_rd_node(int node, u64 addr)
{
	void __iomem *base;

	base = ioremap_nocache(addr, 0x100);
	return ioread64(base);
}

static inline u32 csr_rd32_node(int node, u64 addr)
{
	void __iomem *base;

	base = ioremap_nocache(addr, 0x100);
	return ioread32(base);
}

static inline u64 csr_rd(u64 addr)
{
	return csr_rd_node(0, addr);
}

static inline u32 csr_rd32(u64 addr)
{
	return csr_rd32_node(0, addr);
}

static inline void csr_wr_node(int node, u64 addr, u64 val)
{
	void __iomem *base;

	base = ioremap_nocache(addr, 0x100);
	iowrite64(val, base);
}

static inline void csr_wr32_node(int node, u64 addr, u32 val)
{
	void __iomem *base;

	base = ioremap_nocache(addr, 0x100);
	iowrite32(val, base);
}

static inline void csr_wr(u64 addr, u64 val)
{
	csr_wr_node(0, addr, val);
}

static inline void csr_wr32(u64 addr, u32 val)
{
	csr_wr32_node(0, addr, val);
}

/*
 * We need to use the volatile access here, otherwise the IO accessor
 * functions might swap the bytes
 */
static inline u64 cvmx_read64_uint64(u64 addr)
{
	return *(volatile u64 *)addr;
}

static inline s64 cvmx_read64_int64(u64 addr)
{
	return *(volatile s64 *)addr;
}

static inline void cvmx_write64_uint64(u64 addr, u64 val)
{
	*(volatile u64 *)addr = val;
}

static inline void cvmx_write64_int64(u64 addr, s64 val)
{
	*(volatile s64 *)addr = val;
}

static inline u32 cvmx_read64_uint32(u64 addr)
{
	return *(volatile u32 *)addr;
}

static inline s32 cvmx_read64_int32(u64 addr)
{
	return *(volatile s32 *)addr;
}

static inline void cvmx_write64_uint32(u64 addr, u32 val)
{
	*(volatile u32 *)addr = val;
}

static inline void cvmx_write64_int32(u64 addr, s32 val)
{
	*(volatile s32 *)addr = val;
}

static inline void cvmx_write64_int16(u64 addr, s16 val)
{
	*(volatile s16 *)addr = val;
}

static inline void cvmx_write64_uint16(u64 addr, u16 val)
{
	*(volatile u16 *)addr = val;
}

static inline void cvmx_write64_int8(u64 addr, int8_t val)
{
	*(volatile int8_t *)addr = val;
}

static inline void cvmx_write64_uint8(u64 addr, u8 val)
{
	*(volatile u8 *)addr = val;
}

static inline s16 cvmx_read64_int16(u64 addr)
{
	return *(volatile s16 *)addr;
}

static inline u16 cvmx_read64_uint16(u64 addr)
{
	return *(volatile u16 *)addr;
}

static inline int8_t cvmx_read64_int8(u64 addr)
{
	return *(volatile int8_t *)addr;
}

static inline u8 cvmx_read64_uint8(u64 addr)
{
	return *(volatile u8 *)addr;
}

static inline void cvmx_send_single(u64 data)
{
	cvmx_write64_uint64(CVMX_IOBDMA_ORDERED_IO_ADDR, data);
}

/**
 * Perform a 64-bit write to an IO address
 *
 * @param io_addr	I/O address to write to
 * @param val		64-bit value to write
 */
static inline void cvmx_write_io(u64 io_addr, u64 val)
{
	cvmx_write64_uint64(io_addr, val);
}

/**
 * Builds a memory address for I/O based on the Major and Sub DID.
 *
 * @param major_did 5 bit major did
 * @param sub_did   3 bit sub did
 * @return I/O base address
 */
static inline u64 cvmx_build_io_address(u64 major_did, u64 sub_did)
{
	return ((0x1ull << 48) | (major_did << 43) | (sub_did << 40));
}

/**
 * Builds a bit mask given the required size in bits.
 *
 * @param bits   Number of bits in the mask
 * @return The mask
 */
static inline u64 cvmx_build_mask(u64 bits)
{
	if (bits == 64)
		return -1;

	return ~((~0x0ull) << bits);
}

/**
 * Extract bits out of a number
 *
 * @param input  Number to extract from
 * @param lsb    Starting bit, least significant (0-63)
 * @param width  Width in bits (1-64)
 *
 * @return Extracted number
 */
static inline u64 cvmx_bit_extract(u64 input, int lsb, int width)
{
	u64 result = input >> lsb;

	result &= cvmx_build_mask(width);

	return result;
}

/**
 * Perform mask and shift to place the supplied value into
 * the supplied bit rage.
 *
 * Example: cvmx_build_bits(39,24,value)
 * <pre>
 * 6       5       4       3       3       2       1
 * 3       5       7       9       1       3       5       7      0
 * +-------+-------+-------+-------+-------+-------+-------+------+
 * 000000000000000000000000___________value000000000000000000000000
 * </pre>
 *
 * @param high_bit Highest bit value can occupy (inclusive) 0-63
 * @param low_bit  Lowest bit value can occupy inclusive 0-high_bit
 * @param value    Value to use
 * @return Value masked and shifted
 */
static inline u64 cvmx_build_bits(u64 high_bit, u64 low_bit, u64 value)
{
	return ((value & cvmx_build_mask(high_bit - low_bit + 1)) << low_bit);
}

static inline u64 cvmx_mask_to_localaddr(u64 addr)
{
	return (addr & 0xffffffffff);
}

static inline u64 cvmx_addr_on_node(u64 node, u64 addr)
{
	return (node << 40) | cvmx_mask_to_localaddr(addr);
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

/**
 * Node-local number of the core on which the program is currently running.
 *
 * @return core number on local node
 */
static inline unsigned int cvmx_get_local_core_num(void)
{
	unsigned int core_num, core_mask;

	CVMX_RDHWRNV(core_num, 0);
	/* note that MAX_CORES may not be power of 2 */
	core_mask = (1 << CVMX_NODE_NO_SHIFT) - 1;

	return core_num & core_mask;
}

/**
 * Returns the number of bits set in the provided value.
 * Simple wrapper for POP instruction.
 *
 * @param val    32 bit value to count set bits in
 *
 * @return Number of bits set
 */
static inline u32 cvmx_pop(u32 val)
{
	u32 pop;

	CVMX_POP(pop, val);

	return pop;
}

#define cvmx_read_csr_node(node, addr)	     csr_rd(addr)
#define cvmx_write_csr_node(node, addr, val) csr_wr(addr, val)

#define cvmx_printf  printf
#define cvmx_vprintf vprintf

#if defined(DEBUG)
void cvmx_warn(const char *format, ...) __printf(1, 2);
#else
void cvmx_warn(const char *format, ...);
#endif

#define cvmx_warn_if(expression, format, ...)				\
	if (expression)							\
		cvmx_warn(format, ##__VA_ARGS__)

#endif /* __CVMX_REGS_H__ */
