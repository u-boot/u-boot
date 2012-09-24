/*
 * This was automagically generated from arch/nds/tools/mach-types!
 * Do NOT edit
 */

#ifndef __ASM_NDS32_MACH_TYPE_H
#define __ASM_NDS32_MACH_TYPE_H

#ifndef __ASSEMBLY__
/* The type of machine we're running on */
extern unsigned int __machine_arch_type;
#endif

/* see arch/arm/kernel/arch.c for a description of these */
#define MACH_TYPE_ADPAG101             0

#ifdef CONFIG_ARCH_ADPAG101
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ADPAG101
# endif
# define machine_is_adpag101()	(machine_arch_type == MACH_TYPE_ADPAG101)
#else
# define machine_is_adpag101()	(0)
#endif

#define MACH_TYPE_ADPAG101P            1

#ifdef CONFIG_ARCH_ADPAG101P
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ADPAG101P
# endif
# define machine_is_adpag101p()	(machine_arch_type == MACH_TYPE_ADPAG101P)
#else
# define machine_is_adpag101p()	(1)
#endif

#define MACH_TYPE_ADPAG102             2

#ifdef CONFIG_ARCH_ADPAG102
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ADPAG102
# endif
# define machine_is_adpag102()	(machine_arch_type == MACH_TYPE_ADPAG102)
#else
# define machine_is_adpag102()	(2)
#endif

#endif /* __ASM_NDS32_MACH_TYPE_H */
