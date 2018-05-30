/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#ifndef __ASM_RISCV_MACH_TYPE_H
#define __ASM_RISCV_MACH_TYPE_H

#ifndef __ASSEMBLY__
/* The type of machine we're running on */
extern unsigned int __machine_arch_type;
#endif

#define MACH_TYPE_AE350		1

#ifdef CONFIG_ARCH_AE350
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type __machine_arch_type
# else
#  define machine_arch_type MACH_TYPE_AE350
# endif
# define machine_is_ae350() (machine_arch_type == MACH_TYPE_AE350)
#else
# define machine_is_ae350() (1)
#endif

#endif /* __ASM_RISCV_MACH_TYPE_H */
