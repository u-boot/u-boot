#ifndef _ASM_SH_PROCESSOR_H_
#define _ASM_SH_PROCESSOR_H_
#if defined(CONFIG_CPU_SH2)
# include <asm/cpu_sh2.h>
#elif defined(CONFIG_CPU_SH3)
# include <asm/cpu_sh3.h>
#elif defined(CONFIG_CPU_SH4)
# include <asm/cpu_sh4.h>
#endif
#endif
