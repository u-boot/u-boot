#ifndef _ASM_SH_PROCESSOR_H_
#define _ASM_SH_PROCESSOR_H_
#if defined CONFIG_SH3
# include <asm/cpu_sh3.h>
#elif defined (CONFIG_SH4) || \
	defined (CONFIG_SH4A)
# include <asm/cpu_sh4.h>
#endif
#endif
