#include <stddef.h>
#include <stdio.h>
#include <string.h>

void *func[8], **pfunc;

typedef struct xxx xxx_t;
struct xxx {
	int dummy;
	void **pfunc;
} q;

#define XF_strcpy 3
#define XF_printf 4

#define LABEL(x)					\
asm volatile (						\

#if defined(__i386__)
#define EXPORT_FUNC(x)					\
asm volatile (						\
"	.globl mon_" #x "\n"				\
"mon_" #x ":\n"						\
"	movl	%0, %%eax\n"				\
"	movl	pfunc, %%ecx\n"				\
"	jmp	*(%%ecx,%%eax)\n"			\
	: : "i"(XF_ ## x * sizeof(void *)) : "eax", "ecx");
#elif defined(__powerpc__)
#define EXPORT_FUNC(x)					\
asm volatile (						\
"	.globl mon_" #x "\n"				\
"mon_" #x ":\n"						\
"	lwz	%%r11, %0(%%r2)\n"			\
"	lwz	%%r11, %1(%%r11)\n"			\
"	mtctr	%%r11\n"				\
"	bctr\n"					\
	: : "i"(offsetof(xxx_t, pfunc)), "i"(XF_ ## x * sizeof(void *)) : "r11", "r2");
#elif defined(__arm__)
#define EXPORT_FUNC(x)					\
asm volatile (						\
"	.globl mon_" #x "\n"				\
"mon_" #x ":\n"						\
"	ldr	ip, [r8, %0]\n"				\
"	ldr	pc, [ip, %1]\n"				\
	: : "i"(offsetof(xxx_t, pfunc)), "i"(XF_ ## x * sizeof(void *)) : "ip");
#elif defined(__mips__)
#define EXPORT_FUNC(x)					\
asm volatile (						\
"	.globl mon_" #x "\n"				\
"mon_" #x ":\n"						\
"	lw	$25, %0($26)\n"				\
"	lw	$25, %1($25)\n"				\
"	jr	$25\n"					\
	: : "i"(offsetof(xxx_t, pfunc)), "i"(XF_ ## x * sizeof(void *)) : "t9");
#elif defined(__nds32__)
#define EXPORT_FUNC(x)					\
asm volatile (						\
"	.globl mon_" #x "\n"				\
"mon_" #x ":\n"						\
"	lwi	$r16, [$gp + (%0)]\n"			\
"	lwi	$r16, [$r16 + (%1)]\n"			\
"	jr	$r16\n"					\
: : "i"(offsetof(xxx_t, pfunc)),			\
"i"(XF_ ## x * sizeof(void *)) : "$r16");

#else
#error [No stub code for this arch]
#endif

void dummy(void)
{
EXPORT_FUNC(printf)
EXPORT_FUNC(strcpy)
}

int main(void)
{
#if defined(__i386__)
	xxx_t *pq;
#elif defined(__powerpc__)
	register volatile xxx_t *pq asm("r2");
#elif defined(__arm__)
	register volatile xxx_t *pq asm("r8");
#elif defined(__mips__)
	register volatile xxx_t *pq asm("k0");
#elif defined(__nds32__)
	register volatile xxx_t *pq asm("$r16");
#endif
	char buf[32];

	func[XF_strcpy] = strcpy;
	func[XF_printf] = printf;
	pq = &q;
	pq->pfunc = pfunc = func;

	mon_strcpy(buf, "test");
	mon_printf("hi %s %d z\n", buf, 444);

	return 0;
}
