/*
 * Copyright (c) 2017 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef RISCV_CSR_ENCODING_H
#define RISCV_CSR_ENCODING_H

#define MSTATUS_UIE	0x00000001
#define MSTATUS_SIE	0x00000002
#define MSTATUS_HIE	0x00000004
#define MSTATUS_MIE	0x00000008
#define MSTATUS_UPIE	0x00000010
#define MSTATUS_SPIE	0x00000020
#define MSTATUS_HPIE	0x00000040
#define MSTATUS_MPIE	0x00000080
#define MSTATUS_SPP	0x00000100
#define MSTATUS_HPP	0x00000600
#define MSTATUS_MPP	0x00001800
#define MSTATUS_FS	0x00006000
#define MSTATUS_XS	0x00018000
#define MSTATUS_MPRV	0x00020000
#define MSTATUS_PUM	0x00040000
#define MSTATUS_VM	0x1F000000
#define MSTATUS32_SD	0x80000000
#define MSTATUS64_SD	0x8000000000000000

#define MCAUSE32_CAUSE	0x7FFFFFFF
#define MCAUSE64_CAUSE	0x7FFFFFFFFFFFFFFF
#define MCAUSE32_INT	0x80000000
#define MCAUSE64_INT	0x8000000000000000

#define SSTATUS_UIE	0x00000001
#define SSTATUS_SIE	0x00000002
#define SSTATUS_UPIE	0x00000010
#define SSTATUS_SPIE	0x00000020
#define SSTATUS_SPP	0x00000100
#define SSTATUS_FS	0x00006000
#define SSTATUS_XS	0x00018000
#define SSTATUS_PUM	0x00040000
#define SSTATUS32_SD	0x80000000
#define SSTATUS64_SD	0x8000000000000000

#define MIP_SSIP	BIT(IRQ_S_SOFT)
#define MIP_HSIP	BIT(IRQ_H_SOFT)
#define MIP_MSIP	BIT(IRQ_M_SOFT)
#define MIP_STIP	BIT(IRQ_S_TIMER)
#define MIP_HTIP	BIT(IRQ_H_TIMER)
#define MIP_MTIP	BIT(IRQ_M_TIMER)
#define MIP_SEIP	BIT(IRQ_S_EXT)
#define MIP_HEIP	BIT(IRQ_H_EXT)
#define MIP_MEIP	BIT(IRQ_M_EXT)

#define SIP_SSIP	MIP_SSIP
#define SIP_STIP	MIP_STIP

#define PRV_U	0
#define PRV_S	1
#define PRV_H	2
#define PRV_M	3

#define VM_MBARE	0
#define VM_MBB		1
#define VM_MBBID	2
#define VM_SV32		8
#define VM_SV39		9
#define VM_SV48		10

#define IRQ_S_SOFT	1
#define IRQ_H_SOFT	2
#define IRQ_M_SOFT	3
#define IRQ_S_TIMER	5
#define IRQ_H_TIMER	6
#define IRQ_M_TIMER	7
#define IRQ_S_EXT	9
#define IRQ_H_EXT	10
#define IRQ_M_EXT	11
#define IRQ_COP		12
#define IRQ_HOST	13

#define DEFAULT_RSTVEC		0x00001000
#define DEFAULT_NMIVEC		0x00001004
#define DEFAULT_MTVEC		0x00001010
#define CONFIG_STRING_ADDR	0x0000100C
#define EXT_IO_BASE		0x40000000
#define DRAM_BASE		0x80000000

// page table entry (PTE) fields
#define PTE_V		0x001 // Valid
#define PTE_TYPE	0x01E // Type
#define PTE_R		0x020 // Referenced
#define PTE_D		0x040 // Dirty
#define PTE_SOFT	0x380 // Reserved for Software

#define PTE_TYPE_TABLE		0x00
#define PTE_TYPE_TABLE_GLOBAL	0x02
#define PTE_TYPE_URX_SR		0x04
#define PTE_TYPE_URWX_SRW	0x06
#define PTE_TYPE_UR_SR		0x08
#define PTE_TYPE_URW_SRW	0x0A
#define PTE_TYPE_URX_SRX	0x0C
#define PTE_TYPE_URWX_SRWX0x0E
#define PTE_TYPE_SR		0x10
#define PTE_TYPE_SRW		0x12
#define PTE_TYPE_SRX		0x14
#define PTE_TYPE_SRWX		0x16
#define PTE_TYPE_SR_GLOBAL	0x18
#define PTE_TYPE_SRW_GLOBAL	0x1A
#define PTE_TYPE_SRX_GLOBAL	0x1C
#define PTE_TYPE_SRWX_GLOBAL	0x1E

#define PTE_PPN_SHIFT	10

#define PTE_TABLE(PTE)	((0x0000000AU >> ((PTE) & 0x1F)) & 1)
#define PTE_UR(PTE)	((0x0000AAA0U >> ((PTE) & 0x1F)) & 1)
#define PTE_UW(PTE)	((0x00008880U >> ((PTE) & 0x1F)) & 1)
#define PTE_UX(PTE)	((0x0000A0A0U >> ((PTE) & 0x1F)) & 1)
#define PTE_SR(PTE)	((0xAAAAAAA0U >> ((PTE) & 0x1F)) & 1)
#define PTE_SW(PTE)	((0x88888880U >> ((PTE) & 0x1F)) & 1)
#define PTE_SX(PTE)	((0xA0A0A000U >> ((PTE) & 0x1F)) & 1)

#define PTE_CHECK_PERM(PTE, SUPERVISOR, STORE, FETCH) \
	((STORE) ? ((SUPERVISOR) ? PTE_SW(PTE) : PTE_UW(PTE)) : \
	(FETCH) ? ((SUPERVISOR) ? PTE_SX(PTE) : PTE_UX(PTE)) : \
	((SUPERVISOR) ? PTE_SR(PTE) : PTE_UR(PTE)))

#ifdef __riscv
#ifdef CONFIG_64BIT
# define MSTATUS_SD MSTATUS64_SD
# define SSTATUS_SD SSTATUS64_SD
# define MCAUSE_INT MCAUSE64_INT
# define MCAUSE_CAUSE MCAUSE64_CAUSE
# define RISCV_PGLEVEL_BITS 9
#else
# define MSTATUS_SD MSTATUS32_SD
# define SSTATUS_SD SSTATUS32_SD
# define RISCV_PGLEVEL_BITS 10
# define MCAUSE_INT MCAUSE32_INT
# define MCAUSE_CAUSE MCAUSE32_CAUSE
#endif
#define RISCV_PGSHIFT 12
#define RISCV_PGSIZE BIT(RISCV_PGSHIFT)

#ifndef __ASSEMBLER__

#ifdef __GNUC__

#define read_csr(reg) ({ unsigned long __tmp; \
	asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
	__tmp; })

#define write_csr(reg, val) ({ \
if (__builtin_constant_p(val) && (unsigned long)(val) < 32) \
	asm volatile ("csrw " #reg ", %0" :: "i"(val)); \
else \
	asm volatile ("csrw " #reg ", %0" :: "r"(val)); })

#define swap_csr(reg, val) ({ unsigned long __tmp; \
if (__builtin_constant_p(val) && (unsigned long)(val) < 32) \
	asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "i"(val)); \
else \
	asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "r"(val)); \
	__tmp; })

#define set_csr(reg, bit) ({ unsigned long __tmp; \
if (__builtin_constant_p(bit) && (unsigned long)(bit) < 32) \
	asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "i"(bit)); \
else \
	asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit)); \
	__tmp; })

#define clear_csr(reg, bit) ({ unsigned long __tmp; \
if (__builtin_constant_p(bit) && (unsigned long)(bit) < 32) \
	asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "i"(bit)); \
else \
	asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit)); \
	__tmp; })

#define rdtime() read_csr(time)
#define rdcycle() read_csr(cycle)
#define rdinstret() read_csr(instret)

#endif
#endif
#endif
#endif
