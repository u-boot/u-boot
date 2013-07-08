/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Taken from Linux kernel, commit f56c3196 */

#ifndef _ASM_GENERIC_SECTIONS_H_
#define _ASM_GENERIC_SECTIONS_H_

/* References to section boundaries */

extern char _text[], _stext[], _etext[];
extern char _data[], _sdata[], _edata[];
extern char __bss_start[], __bss_stop[];
extern char __init_begin[], __init_end[];
extern char _sinittext[], _einittext[];
extern char _end[], _init[];
extern char __per_cpu_load[], __per_cpu_start[], __per_cpu_end[];
extern char __kprobes_text_start[], __kprobes_text_end[];
extern char __entry_text_start[], __entry_text_end[];
extern char __initdata_begin[], __initdata_end[];
extern char __start_rodata[], __end_rodata[];

/* Start and end of .ctors section - used for constructor calls. */
extern char __ctors_start[], __ctors_end[];

/* function descriptor handling (if any).  Override
 * in asm/sections.h */
#ifndef dereference_function_descriptor
#define dereference_function_descriptor(p) (p)
#endif

/* random extra sections (if any).  Override
 * in asm/sections.h */
#ifndef arch_is_kernel_text
static inline int arch_is_kernel_text(unsigned long addr)
{
	return 0;
}
#endif

#ifndef arch_is_kernel_data
static inline int arch_is_kernel_data(unsigned long addr)
{
	return 0;
}
#endif

/* U-Boot-specific things begin here */

/* Start of U-Boot text region */
extern char __text_start[];

/* This marks the end of the text region which must be relocated */
extern char __image_copy_end[];

/*
 * This is the U-Boot entry point - prior to relocation it should be same
 * as __text_start
 */
extern void _start(void);

/*
 * ARM needs to use offsets for symbols, since the values of some symbols
 * are not resolved prior to relocation (and are just 0). Maybe this can be
 * resolved, or maybe other architectures are similar, iwc this should be
 * promoted to an architecture option.
 */
#ifdef CONFIG_ARM
#define CONFIG_SYS_SYM_OFFSETS
#endif

#ifdef CONFIG_SYS_SYM_OFFSETS
/* Start/end of the relocation entries, as an offset from _start */
extern ulong _rel_dyn_start_ofs;
extern ulong _rel_dyn_end_ofs;

/* End of the region to be relocated, as an offset form _start */
extern ulong _image_copy_end_ofs;

extern ulong _bss_start_ofs;	/* BSS start relative to _start */
extern ulong _bss_end_ofs;		/* BSS end relative to _start */
extern ulong _end_ofs;		/* end of image relative to _start */

extern ulong _TEXT_BASE;	/* code start */

#else /* don't use offsets: */

/* Exports from the Linker Script */
extern ulong __data_end;
extern ulong __rel_dyn_start;
extern ulong __rel_dyn_end;
extern ulong __bss_end;

extern ulong _TEXT_BASE;	/* code start */

#endif

#endif /* _ASM_GENERIC_SECTIONS_H_ */
