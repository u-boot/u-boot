/*
 * (C) Copyright 2016
 * Alexander Graf <agraf@suse.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SETJMP_H_
#define _SETJMP_H_	1

struct jmp_buf_data {
	ulong target;
	ulong regs[5];
	int ret;
};

typedef struct jmp_buf_data jmp_buf[1];

static inline int setjmp(jmp_buf jmp)
{
	jmp->ret = 0;

#ifdef CONFIG_ARM64
	asm volatile(
		"adr x1, jmp_target\n"
		"str x1, %0\n"
		"stp x26, x27, %1\n"
		"stp x28, x29, %2\n"
		"mov x1, sp\n"
		"str x1, %3\n"
		"jmp_target: "
		: "=m" (jmp->target), "=m" (jmp->regs[0]),
		  "=m" (jmp->regs[2]), "=m" (jmp->regs[4])
		:
		: "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
		  "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
		  "x16", "x17", "x18", "x19", "x20", "x21", "x22",
		  "x23", "x24", "x25", /* x26, x27, x28, x29, sp */
		  "x30", "cc", "memory");
#else
	asm volatile(
#ifdef CONFIG_SYS_THUMB_BUILD
		".align 2\n"
		"adr r0, jmp_target\n"
		"add r0, r0, $1\n"
#else
		"adr r0, jmp_target\n"
#endif
		"mov r1, %0\n"
		"mov r2, sp\n"
		"stm r1!, {r0, r2, r4, r5, r6, r7}\n"
		".align 2\n"
		"jmp_target: \n"
		:
		: "l" (&jmp->target)
		: "r0", "r1", "r2", "r3", /* "r4", "r5", "r6", "r7", */
		  "r8", "r9", "r10", "r11", /* sp, */ "ip", "lr",
		  "cc", "memory");
#endif

	return jmp->ret;
}

static inline __noreturn void longjmp(jmp_buf jmp, int ret)
{
	jmp->ret = ret;

#ifdef CONFIG_ARM64
	asm volatile(
		"ldr x0, %0\n"
		"ldr x1, %3\n"
		"mov sp, x1\n"
		"ldp x26, x27, %1\n"
		"ldp x28, x25, %2\n"
		"mov x29, x25\n"
		"br x0\n"
		:
		: "m" (jmp->target), "m" (jmp->regs[0]), "m" (jmp->regs[2]),
		  "m" (jmp->regs[4])
		: "x0", "x1", "x25", "x26", "x27", "x28");
#else
	asm volatile(
		"mov r1, %0\n"
		"ldm r1!, {r0, r2, r4, r5, r6, r7}\n"
		"mov sp, r2\n"
		"bx r0\n"
		:
		: "l" (&jmp->target)
		: "r1");
#endif

	while (1) { }
}


#endif /* _SETJMP_H_ */
