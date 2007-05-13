#ifndef _ASM_CPU_SH4_H_
#define _ASM_CPU_SH4_H_
#if defined (CONFIG_CPU_SH7750)
#include <asm/cpu_sh7750.h>
#elif defined (CONFIG_CPU_SH7780)
#include <asm/cpu_sh7780.h>
#else
#error "Unknown SH4 variant"
#endif
#endif
