/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_RISCV_MACH_TYPE_H
#define __ASM_RISCV_MACH_TYPE_H

#ifndef __ASSEMBLY__
/* The type of machine we're running on */
extern unsigned int __machine_arch_type;
#endif

#define MACH_TYPE_AE250		1

#ifdef CONFIG_ARCH_AE250
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type __machine_arch_type
# else
#  define machine_arch_type MACH_TYPE_AE250
# endif
# define machine_is_ae250() (machine_arch_type == MACH_TYPE_AE250)
#else
# define machine_is_ae250() (1)
#endif

#endif /* __ASM_RISCV_MACH_TYPE_H */
