/*
 * Copyright (c) 2014 Google, Inc
 *
 * From Coreboot file of the same name
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_MTRR_H
#define _ASM_MTRR_H

/*  These are the region types  */
#define MTRR_TYPE_UNCACHEABLE 0
#define MTRR_TYPE_WRCOMB     1
/*#define MTRR_TYPE_         2*/
/*#define MTRR_TYPE_         3*/
#define MTRR_TYPE_WRTHROUGH  4
#define MTRR_TYPE_WRPROT     5
#define MTRR_TYPE_WRBACK     6
#define MTRR_NUM_TYPES       7

#define MTRRcap_MSR     0x0fe
#define MTRRdefType_MSR 0x2ff

#define MTRRdefTypeEn		(1 << 11)
#define MTRRdefTypeFixEn	(1 << 10)

#define SMRRphysBase_MSR 0x1f2
#define SMRRphysMask_MSR 0x1f3

#define MTRRphysBase_MSR(reg) (0x200 + 2 * (reg))
#define MTRRphysMask_MSR(reg) (0x200 + 2 * (reg) + 1)

#define MTRRphysMaskValid	(1 << 11)

#define NUM_FIXED_RANGES 88
#define RANGES_PER_FIXED_MTRR 8
#define MTRRfix64K_00000_MSR 0x250
#define MTRRfix16K_80000_MSR 0x258
#define MTRRfix16K_A0000_MSR 0x259
#define MTRRfix4K_C0000_MSR 0x268
#define MTRRfix4K_C8000_MSR 0x269
#define MTRRfix4K_D0000_MSR 0x26a
#define MTRRfix4K_D8000_MSR 0x26b
#define MTRRfix4K_E0000_MSR 0x26c
#define MTRRfix4K_E8000_MSR 0x26d
#define MTRRfix4K_F0000_MSR 0x26e
#define MTRRfix4K_F8000_MSR 0x26f

#if !defined(__ASSEMBLER__)

/*
 * The MTRR code has some side effects that the callers should be aware for.
 * 1. The call sequence matters. x86_setup_mtrrs() calls
 *    x86_setup_fixed_mtrrs_no_enable() then enable_fixed_mtrrs() (equivalent
 *    of x86_setup_fixed_mtrrs()) then x86_setup_var_mtrrs(). If the callers
 *    want to call the components of x86_setup_mtrrs() because of other
 *    rquirements the ordering should still preserved.
 * 2. enable_fixed_mtrr() will enable both variable and fixed MTRRs because
 *    of the nature of the global MTRR enable flag. Therefore, all direct
 *    or indirect callers of enable_fixed_mtrr() should ensure that the
 *    variable MTRR MSRs do not contain bad ranges.
 * 3. If CONFIG_CACHE_ROM is selected an MTRR is allocated for enabling
 *    the caching of the ROM. However, it is set to uncacheable (UC). It
 *    is the responsiblity of the caller to enable it by calling
 *    x86_mtrr_enable_rom_caching().
 */
void x86_setup_mtrrs(void);
/*
 * x86_setup_var_mtrrs() parameters:
 * address_bits - number of physical address bits supported by cpu
 * above4gb - 2 means dynamically detect number of variable MTRRs available.
 *            non-zero means handle memory ranges above 4GiB.
 *            0 means ignore memory ranges above 4GiB
 */
void x86_setup_var_mtrrs(unsigned int address_bits, unsigned int above4gb);
void enable_fixed_mtrr(void);
void x86_setup_fixed_mtrrs(void);
/* Set up fixed MTRRs but do not enable them. */
void x86_setup_fixed_mtrrs_no_enable(void);
int x86_mtrr_check(void);
/* ROM caching can be used after variable MTRRs are set up. Beware that
 * enabling CONFIG_CACHE_ROM will eat through quite a few MTRRs based on
 * one's IO hole size and WRCOMB resources. Be sure to check the console
 * log when enabling CONFIG_CACHE_ROM or adding WRCOMB resources. Beware that
 * on CPUs with core-scoped MTRR registers such as hyperthreaded CPUs the
 * rom caching will be disabled if all threads run the MTRR code. Therefore,
 * one needs to call x86_mtrr_enable_rom_caching() after all threads of the
 * same core have run the MTRR code. */
#if CONFIG_CACHE_ROM
void x86_mtrr_enable_rom_caching(void);
void x86_mtrr_disable_rom_caching(void);
/* Return the variable range MTRR index of the ROM cache. */
long x86_mtrr_rom_cache_var_index(void);
#else
static inline void x86_mtrr_enable_rom_caching(void) {}
static inline void x86_mtrr_disable_rom_caching(void) {}
static inline long x86_mtrr_rom_cache_var_index(void) { return -1; }
#endif /* CONFIG_CACHE_ROM */

#endif

#if !defined(CONFIG_RAMTOP)
# error "CONFIG_RAMTOP not defined"
#endif

#if ((CONFIG_XIP_ROM_SIZE & (CONFIG_XIP_ROM_SIZE - 1)) != 0)
# error "CONFIG_XIP_ROM_SIZE is not a power of 2"
#endif

#if ((CONFIG_CACHE_ROM_SIZE & (CONFIG_CACHE_ROM_SIZE - 1)) != 0)
# error "CONFIG_CACHE_ROM_SIZE is not a power of 2"
#endif

#define CACHE_ROM_BASE	(((1 << 20) - (CONFIG_CACHE_ROM_SIZE >> 12)) << 12)

#if (CONFIG_RAMTOP & (CONFIG_RAMTOP - 1)) != 0
# error "CONFIG_RAMTOP must be a power of 2"
#endif

#endif
